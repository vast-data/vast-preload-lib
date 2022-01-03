#include <boost/filesystem.hpp>
#include <fcntl.h>
#include "InjectionTk.h"

/**
 * Used after an open()-style operation to check for a path match to inject O_DIRECT and to add the
 * fd to the path store. The fd will be added to the store either way, because it might refer to a
 * directory, in which case we might later need it to resolve the full path in the context of an
 * openat()-style operation.
 *
 * @flags as in "open(const char *pathname, int flags, mode_t mode)"
 */
void InjectionTk::injectAfterOpen(int fd, std::string path, int flags)
{
	if( (path.length() > 1) && (path[0] != '/') )
	{ // path is relative to current working dir, so make it absolute
		char* cwdBuf = getcwd(NULL, 0);

		std::string concatPathStr(cwdBuf);

		free(cwdBuf);

		if(concatPathStr != "/")
			concatPathStr += "/";

		concatPathStr += path;

		path = concatPathStr;
	}

	if(!(libOpts & ENV_LIB_OPT_RAWPATHS) )
	{ // normalize path (e.g. remove ".." and "//")
		boost::filesystem::path pathObj(path);

		std::string pathObjNormalizedStr = pathObj.normalize().string();

		// .normalize() leaves a trailing slashdot ("/.") when path ended with "/", so remove that
		if( (pathObjNormalizedStr.length() > 2) &&
			('/' == pathObjNormalizedStr[pathObjNormalizedStr.length() -2] ) &&
			('.' == pathObjNormalizedStr[pathObjNormalizedStr.length() -1] ) )
			pathObjNormalizedStr.resize(pathObjNormalizedStr.length() -2);

		if( (libLogTopics & ENV_LOG_TOPIC_NORMALIZE) && (pathObj.string() != path) )
			log_fprintf(stderr, LOG_PREFIX "Normalized path. fd: %d; old: %s; new: %s\n",
				fd, path.c_str(), pathObjNormalizedStr.c_str() );

		path = pathObjNormalizedStr;
	}

	/* always add to store because this could be a dir, which might be be needed for openat() later
		even though the dir path does not match user-given paths */
	fdStore->addFD(fd, path);

	if(!pathMatchStore->checkPathMatch(path) )
	{
		if(libLogTopics & ENV_LOG_TOPIC_INJECT_SKIP)
			log_fprintf(stderr, LOG_PREFIX "Skipping inject due to path mismatch. "
				"fd: %d; path: %s\n", fd, path.c_str() );
		return;
	}

	if(flags & O_DIRECTORY)
	{
		if(libLogTopics & ENV_LOG_TOPIC_INJECT_SKIP)
			log_fprintf(stderr, LOG_PREFIX "Skipping inject due to O_DIRECTORY. "
				"fd: %d; path: %s\n", fd, path.c_str() );
		return;
	}

	if(flags & O_DIRECT)
	{
		if(libLogTopics & ENV_LOG_TOPIC_INJECT_SKIP)
			log_fprintf(stderr, LOG_PREFIX "Skipping inject due to O_DIRECT set already. "
				"fd: %d; path: %s\n", fd, path.c_str() );
		return;
	}

	if(libLogTopics & ENV_LOG_TOPIC_INJECT)
		log_fprintf(stderr, LOG_PREFIX "Injecting O_DIRECT. fd: %d; path: %s\n",
			fd, path.c_str() );

	int fcntlRes = fcntl(fd, F_SETFL, flags | O_DIRECT);
	if(fcntlRes == -1)
	{
		if(libLogTopics & ENV_LOG_TOPIC_INJECT)
			log_fprintf(stderr, LOG_PREFIX "Adding O_DIRECT failed. fd: %d; path: %s\n",
				fd, path.c_str() );
	}
}

/**
 * Add to fdStore and inject O_DIRECT if appropriate.
 */
void InjectionTk::injectAfterOpenat(int dirfd, int fd, std::string path, int flags)
{
	if(path[0] == '/')
		injectAfterOpen(fd, path, flags); // abs path ignores dirfd
	else
	{
		if(dirfd == AT_FDCWD)
		{ // path is relative to current working dir
			char* cwdBuf = getcwd(NULL, 0);

			std::string concatPathStr(cwdBuf);

			free(cwdBuf);

			if(concatPathStr != "/")
				concatPathStr += "/";

			concatPathStr += path;

			path = concatPathStr;
		}
		else
		{
			std::string dirPathStr;

			bool fdFound = fdStore->getFD(dirfd, dirPathStr);

			if(!fdFound & (libLogTopics & ENV_LOG_TOPIC_INJECT_SKIP) )
			{
				log_fprintf(stderr,
					LOG_PREFIX "Skipping open inject due to openat dirfd not found in store: "
					"dirfd: %d; fd: %d; path: %s\n", dirfd, fd, path);

				return;
			}

			std::string concatPathStr(dirPathStr + "/" + path);

			path = concatPathStr;
		}


		injectAfterOpen(fd, path, flags);
	}
}

/**
 * Remove O_DIRECT after read or write returned EINVAL (which can indicate invalid parameters for
 * O_DIRECT among others).
 */
void InjectionTk::ejectAfterEinval(int fd)
{
	std::string removalPath;

	/* note: we don't remove from fdStore here because this might be the fd of a dir, which is later
		needed to resolve an openat() path */

	bool fdFound = fdStore->getFD(fd, removalPath);

	if(!fdFound)
		return; // fd not in store, so not tracked by us and thus nothing to do

	int flags = fcntl(fd, F_GETFL);

	if(!(flags & O_DIRECT) )
		return; // O_DIRECT not set, so nothing to do

	if(libLogTopics & ENV_LOG_TOPIC_INJECT)
		log_fprintf(stderr, LOG_PREFIX "Removing injected O_DIRECT after EINVAL error. "
			"fd: %d; path: %s\n",
			fd, removalPath.c_str() );

	fcntl(fd, F_SETFL, flags & ~O_DIRECT);
}

/**
 * Remove FD from O_DIRECT list before file close.
 */
void InjectionTk::ejectBeforeClose(int fd)
{
	if(libLogTopics & ENV_LOG_TOPIC_CLOSE)
	{
		std::string pathStr;
		bool pathFound = fdStore->getFD(fd, pathStr);
		log_fprintf(stderr, LOG_PREFIX "Closing. fd: %d; path: %s\n", fd,
			pathFound ? pathStr.c_str() : "<not found>");
	}

	fdStore->removeFD(fd);
}


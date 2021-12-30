#include "PathMatchStore.h"

PathMatchStore* pathMatchStore = NULL;

/**
 * Read newline-separated paths from given file.
 *
 * @return number of loaded paths, -1 on error (e.g. file not found or not readable).
 */
bool PathMatchStore::loadPathFile(std::string path)
{
	std::string lineStr;
	unsigned lineNum = 0;

	std::ifstream fileStream(path.c_str() );
	if(!fileStream)
	{
		log_fprintf(stderr, LOG_PREFIX "ERROR: Open of path file failed: %s\n",
			path.c_str() );
		return false;
	}

	// process each line in input file
	for( ; std::getline(fileStream, lineStr); lineNum++)
	{
		boost::trim(lineStr);

		if(lineStr.empty() )
			continue; // nothing to do for empty lines

		if(lineStr[0] != '/')
		{
			log_fprintf(stderr, LOG_PREFIX
				"ERROR: Path file contains a path that is not absolute: "
				"%s (Line: %d; File: %s)\n",
				lineStr.c_str(), lineNum+1, path.c_str() );

			return false;
		}

		PathMatchStoreElem newElem(wildcardStrToRegex(lineStr), lineStr);

		pathMap.insert(newElem);
	}

	return true;
}

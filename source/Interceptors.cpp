/**
 * Find used function names of a certain binary via
 * - readelf, e.g.:
 * 		readelf -a /usr/bin/tar | grep open
 * - LD_DEBUG by running the executable, e.g.:
 * 		$ man ld-linux # see section LD_DEBUG
 * 		$ export LD_DEBUG=symbols
 * 		$ export LD_DEBUG_OUTPUT=myoutput.txt
 * 		$ ls -lh /some/dir
 * 		$ unset LD_DEBUG
 * 		Now myoutput.txt.PID contains symbol lookups.
 * - function signatures in system includes, e.g.:
 * 		grep -r openat /usr/include
 */


#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <mntent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/vfs.h>

#include "Common.h"
#include "Config.h"
#include "InjectionTk.h"
#include "Logger.h"


FUNC_FORWARD_DECL(opendir,
	DIR*, (const char *filename));
FUNC_FORWARD_DECL(fopen, FILE*, (const char *path, const char *mode));
FUNC_FORWARD_DECL(fopen64, FILE*, (const char *path, const char *mode));
FUNC_FORWARD_DECL(open, int, (const char *path, int flags, ...));
FUNC_FORWARD_DECL(open64, int, (const char *path, int flags, ...));
FUNC_FORWARD_DECL(__open_2, int, (const char *path, int oflag));
FUNC_FORWARD_DECL(__open64_2, int, (const char *path, int oflag));
FUNC_FORWARD_DECL(openat, int, (int dirfd, const char *path, int flags, ...));
FUNC_FORWARD_DECL(openat64, int, (int dirfd, const char *path, int flags, ...));
FUNC_FORWARD_DECL(__openat_2, int, (int dirfd, const char *path, int flags));
FUNC_FORWARD_DECL(__openat64_2, int, (int dirfd, const char *path, int flags));
FUNC_FORWARD_DECL(creat, int, (const char* path, mode_t mode));
FUNC_FORWARD_DECL(creat64, int, (const char* path, mode_t mode));
FUNC_FORWARD_DECL(fread, size_t, (void *ptr, size_t size, size_t nmemb, FILE *stream));
FUNC_FORWARD_DECL(read, ssize_t, (int fd, void *buf, size_t count));
FUNC_FORWARD_DECL(fwrite, size_t, (const void *ptr, size_t size, size_t nmemb, FILE *stream));
FUNC_FORWARD_DECL(write, ssize_t, (int fd, const void *buf, size_t count));
FUNC_FORWARD_DECL(pread, ssize_t, (int fd, void *buf, size_t count, off_t offset));
FUNC_FORWARD_DECL(pwrite, ssize_t, (int fd, const void *buf, size_t count, off_t offset));
FUNC_FORWARD_DECL(pread64, ssize_t, (int fd, void *buf, size_t count, off64_t offset));
FUNC_FORWARD_DECL(pwrite64, ssize_t, (int fd, const void *buf, size_t count, off64_t offset));
FUNC_FORWARD_DECL(readv, ssize_t, (int fd, const struct iovec *iov, int iovcnt));
FUNC_FORWARD_DECL(preadv, ssize_t, (int fd, const struct iovec *iov, int iovcnt, off_t offset));
FUNC_FORWARD_DECL(preadv64, ssize_t, (int fd, const struct iovec *iov, int iovcnt, off64_t offset));
FUNC_FORWARD_DECL(preadv2, ssize_t, (int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags));
FUNC_FORWARD_DECL(preadv64v2, ssize_t, (int fd, const struct iovec *iov, int iovcnt, off64_t offset, int flags));
FUNC_FORWARD_DECL(writev, ssize_t, (int fd, const struct iovec *iov, int iovcnt));
FUNC_FORWARD_DECL(pwritev, ssize_t, (int fd, const struct iovec *iov, int iovcnt, off_t offset));
FUNC_FORWARD_DECL(pwritev64, ssize_t, (int fd, const struct iovec *iov, int iovcnt, off64_t offset));
FUNC_FORWARD_DECL(pwritev2, ssize_t, (int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags));
FUNC_FORWARD_DECL(pwritev64v2, ssize_t, (int fd, const struct iovec *iov, int iovcnt, off64_t offset, int flags));
FUNC_FORWARD_DECL(fsync, int, (int fd));
FUNC_FORWARD_DECL(fdatasync, int, (int fd));
FUNC_FORWARD_DECL(closedir, int, (DIR *dirp));
FUNC_FORWARD_DECL(fclose, int, (FILE* stream));
FUNC_FORWARD_DECL(close, int, (int fd));



extern "C" DIR* INTERCEPT_FUNC_DECL(opendir)(const char *path)
{
	DIR* ret;

	LOG_INTERCEPT_PATH(path);

	MAP_OR_FAIL(opendir);

	ret = __real_opendir(path);

	if(!initDone || (ret==NULL) )
		return(ret);

	int fd = dirfd(ret);

	InjectionTk::injectAfterOpen(fd, path, O_DIRECTORY);

	return(ret);
}

extern "C" FILE* INTERCEPT_FUNC_DECL(fopen)(const char *path, const char *mode)
{
	FILE* ret;

	LOG_INTERCEPT_PATH(path);

	MAP_OR_FAIL(fopen);

	ret = __real_fopen(path, mode);

	if(!initDone || (ret==NULL) )
		return(ret);

	int fd = fileno(ret);
	int flags = fcntl(fd, F_GETFL);

	InjectionTk::injectAfterOpen(fd, path, flags);

	return(ret);
}

extern "C" FILE* INTERCEPT_FUNC_DECL(fopen64)(const char *path, const char *mode)
{
	FILE* ret;

	LOG_INTERCEPT_PATH(path);

	MAP_OR_FAIL(fopen64);

	ret = __real_fopen64(path, mode);

	if(!initDone || (ret==NULL) )
		return(ret);

	int fd = fileno(ret);
	int flags = fcntl(fd, F_GETFL);

	InjectionTk::injectAfterOpen(fd, path, flags);

	return(ret);
}

extern "C" int INTERCEPT_FUNC_DECL(open)(const char *path, int flags, ...)
{
	int mode = 0;
	int ret;

	LOG_INTERCEPT_PATH(path);

	MAP_OR_FAIL(open);

	if(flags & O_CREAT)
	{
		va_list arg;
		va_start(arg, flags);
		mode = va_arg(arg, int);
		va_end(arg);

		ret = __real_open(path, flags, mode);
	}
	else
		ret = __real_open(path, flags);

	if(!initDone || (ret == -1) )
		return(ret);

	InjectionTk::injectAfterOpen(ret, path, flags);

	return(ret);
}

extern "C" int INTERCEPT_FUNC_DECL(open64)(const char *path, int flags, ...)
{
	int mode = 0;
	int ret;

	LOG_INTERCEPT_PATH(path);

	MAP_OR_FAIL(open64);

	if(flags & O_CREAT)
	{
		va_list arg;
		va_start(arg, flags);
		mode = va_arg(arg, int);
		va_end(arg);

		ret = __real_open64(path, flags, mode);
	}
	else
		ret = __real_open64(path, flags);

	if(!initDone || (ret == -1) )
		return(ret);

	InjectionTk::injectAfterOpen(ret, path, flags);

	return(ret);
}

extern "C" int INTERCEPT_FUNC_DECL(__open_2)(const char *path, int flags)
{
	int ret;

	LOG_INTERCEPT_PATH(path);

	MAP_OR_FAIL(__open_2);

	ret = __real___open_2(path, flags);

	if(!initDone || (ret == -1) )
		return(ret);

	InjectionTk::injectAfterOpen(ret, path, flags);

	return(ret);
}

extern "C" int INTERCEPT_FUNC_DECL(__open64_2)(const char *path, int flags)
{
	int ret;

	LOG_INTERCEPT_PATH(path);

	MAP_OR_FAIL(__open64_2);

	ret = __real___open64_2(path, flags);

	if(!initDone || (ret == -1) )
		return(ret);

	InjectionTk::injectAfterOpen(ret, path, flags);

	return(ret);
}

extern "C" int INTERCEPT_FUNC_DECL(openat)(int dirfd, const char *path, int flags, ...)
{
	int mode = 0;
	int ret;

	LOG_INTERCEPT_PATH(path);

	MAP_OR_FAIL(openat);

	if(flags & O_CREAT)
	{
		va_list arg;
		va_start(arg, flags);
		mode = va_arg(arg, int);
		va_end(arg);

		ret = __real_openat(dirfd, path, flags, mode);
	}
	else
		ret = __real_openat(dirfd, path, flags);

	if(!initDone || (ret == -1) )
		return(ret);

	InjectionTk::injectAfterOpenat(dirfd, ret, path, flags);

	return(ret);
}

extern "C" int INTERCEPT_FUNC_DECL(openat64)(int dirfd, const char *path, int flags, ...)
{
	int mode = 0;
	int ret;

	LOG_INTERCEPT_PATH(path);

	MAP_OR_FAIL(openat64);

	if(flags & O_CREAT)
	{
		va_list arg;
		va_start(arg, flags);
		mode = va_arg(arg, int);
		va_end(arg);

		ret = __real_openat64(dirfd, path, flags, mode);
	}
	else
		ret = __real_openat64(dirfd, path, flags);

	if(!initDone || (ret == -1) )
		return(ret);

	InjectionTk::injectAfterOpenat(dirfd, ret, path, flags);

	return(ret);
}

extern "C" int INTERCEPT_FUNC_DECL(__openat_2)(int dirfd, const char *path, int flags)
{
	int ret;

	LOG_INTERCEPT_PATH(path);

	MAP_OR_FAIL(__openat_2);

	ret = __real___openat_2(dirfd, path, flags);

	if(!initDone || (ret == -1) )
		return(ret);

	InjectionTk::injectAfterOpenat(dirfd, ret, path, flags);

	return(ret);
}

extern "C" int INTERCEPT_FUNC_DECL(__openat64_2)(int dirfd, const char *path, int flags)
{
	int ret;

	LOG_INTERCEPT_PATH(path);

	MAP_OR_FAIL(__openat64_2);

	ret = __real___openat64_2(dirfd, path, flags);

	if(!initDone || (ret == -1) )
		return(ret);

	InjectionTk::injectAfterOpenat(dirfd, ret, path, flags);

	return(ret);
}

extern "C" int INTERCEPT_FUNC_DECL(creat)(const char* path, mode_t mode)
{
	int ret;

	LOG_INTERCEPT_PATH(path);

	MAP_OR_FAIL(creat);

	ret = __real_creat(path, mode);

	if(!initDone || (ret == -1) )
		return(ret);

	/* note: "man 2 creat" says this is equivalent to "open() with flags equal to
		O_CREAT|O_WRONLY|O_TRUNC" */

	InjectionTk::injectAfterOpen(ret, path, O_CREAT | O_WRONLY | O_TRUNC);

	return(ret);
}

extern "C" int INTERCEPT_FUNC_DECL(creat64)(const char* path, mode_t mode)
{
	int ret;

	LOG_INTERCEPT_PATH(path);

	MAP_OR_FAIL(creat64);

	/* note: if we need to inject flags here, then "man 2 creat" says this is equivalent to
		"open() with flags equal to O_CREAT_|O_WRONLY|O_TRUNC"
		so we could forward to that or afterwards modify the fd to add O_DIRECT via fcntl */

	ret = __real_creat64(path, mode);

	if(!initDone || (ret == -1) )
		return(ret);

	/* note: "man 2 creat" says this is equivalent to "open() with flags equal to
		O_CREAT|O_WRONLY|O_TRUNC" */

	InjectionTk::injectAfterOpen(ret, path, O_CREAT | O_WRONLY | O_TRUNC);

	return(ret);
}

extern "C" size_t INTERCEPT_FUNC_DECL(fread)(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t ret;

	LOG_INTERCEPT_FD(fileno(stream) );

	MAP_OR_FAIL(fread);

	ret = __real_fread(ptr, size, nmemb, stream);

	// note: ferror does not provide a specific error code like EINVAL

	if(ret || !ferror(stream) )
		return(ret);

	InjectionTk::ejectAfterEinval(fileno(stream) );

	clearerr(stream);

	ret = __real_fread(ptr, size, nmemb, stream);

	return(ret);
}

extern "C" ssize_t INTERCEPT_FUNC_DECL(read)(int fd, void *buf, size_t count)
{
	ssize_t ret;

	LOG_INTERCEPT_FD(fd);

	MAP_OR_FAIL(read);

	ret = __real_read(fd, buf, count);

	if( (ret != -1) || (errno != EINVAL) )
		return(ret);

	InjectionTk::ejectAfterEinval(fd);

	ret = __real_read(fd, buf, count);

	return(ret);
}

extern "C" size_t INTERCEPT_FUNC_DECL(fwrite)(const void *ptr, size_t size, size_t nmemb,
	FILE *stream)
{
	size_t ret;

	LOG_INTERCEPT_FD(fileno(stream) );

	MAP_OR_FAIL(fwrite);

	ret = __real_fwrite(ptr, size, nmemb, stream);

	// note: ferror does not provide a specific error code like EINVAL

	if(ret || !ferror(stream) )
		return(ret);

	InjectionTk::ejectAfterEinval(fileno(stream) );

	clearerr(stream);

	ret = __real_fwrite(ptr, size, nmemb, stream);

	return(ret);
}

extern "C" ssize_t INTERCEPT_FUNC_DECL(write)(int fd, const void *buf, size_t count)
{
	ssize_t ret;

	LOG_INTERCEPT_FD(fd);

	MAP_OR_FAIL(write);

	ret = __real_write(fd, buf, count);

	if( (ret != -1) || (errno != EINVAL) )
		return(ret);

	InjectionTk::ejectAfterEinval(fd);

	ret = __real_write(fd, buf, count);

	return(ret);
}

extern "C" ssize_t INTERCEPT_FUNC_DECL(pread)(int fd, void *buf, size_t count, off_t offset)
{
	ssize_t ret;

	LOG_INTERCEPT_FD(fd);

	MAP_OR_FAIL(pread);

	ret = __real_pread(fd, buf, count, offset);

	if( (ret != -1) || (errno != EINVAL) )
		return(ret);

	InjectionTk::ejectAfterEinval(fd);

	ret = __real_pread(fd, buf, count, offset);

	return(ret);
}

extern "C" ssize_t INTERCEPT_FUNC_DECL(pwrite)(int fd, const void *buf, size_t count, off_t offset)
{
	ssize_t ret;

	LOG_INTERCEPT_FD(fd);

	MAP_OR_FAIL(pwrite);

	ret = __real_pwrite(fd, buf, count, offset);

	if( (ret != -1) || (errno != EINVAL) )
		return(ret);

	InjectionTk::ejectAfterEinval(fd);

	ret = __real_pwrite(fd, buf, count, offset);

	return(ret);
}

extern "C" ssize_t INTERCEPT_FUNC_DECL(pread64)(int fd, void *buf, size_t count, off64_t offset)
{
	ssize_t ret;

	LOG_INTERCEPT_FD(fd);

	MAP_OR_FAIL(pread64);

	ret = __real_pread64(fd, buf, count, offset);

	if( (ret != -1) || (errno != EINVAL) )
		return(ret);

	InjectionTk::ejectAfterEinval(fd);

	ret = __real_pread64(fd, buf, count, offset);

	return(ret);
}

extern "C" ssize_t INTERCEPT_FUNC_DECL(pwrite64)(int fd, const void *buf, size_t count, off64_t offset)
{
	ssize_t ret;

	LOG_INTERCEPT_FD(fd);

	MAP_OR_FAIL(pwrite64);

	ret = __real_pwrite64(fd, buf, count, offset);

	if( (ret != -1) || (errno != EINVAL) )
		return(ret);

	InjectionTk::ejectAfterEinval(fd);

	ret = __real_pwrite64(fd, buf, count, offset);

	return(ret);
}

extern "C" ssize_t INTERCEPT_FUNC_DECL(readv)(int fd, const struct iovec *iov, int iovcnt)
{
	ssize_t ret;

	LOG_INTERCEPT_FD(fd);

	MAP_OR_FAIL(readv);

	ret = __real_readv(fd, iov, iovcnt);

	if( (ret != -1) || (errno != EINVAL) )
		return(ret);

	InjectionTk::ejectAfterEinval(fd);

	ret = __real_readv(fd, iov, iovcnt);

	return(ret);
}

extern "C" ssize_t INTERCEPT_FUNC_DECL(preadv)(int fd, const struct iovec *iov, int iovcnt, off_t offset)
{
	ssize_t ret;

	LOG_INTERCEPT_FD(fd);

	MAP_OR_FAIL(preadv);

	ret = __real_preadv(fd, iov, iovcnt, offset);

	if( (ret != -1) || (errno != EINVAL) )
		return(ret);

	InjectionTk::ejectAfterEinval(fd);

	ret = __real_preadv(fd, iov, iovcnt, offset);

	return(ret);
}

extern "C" ssize_t INTERCEPT_FUNC_DECL(preadv64)(int fd, const struct iovec *iov, int iovcnt, off64_t offset)
{
	ssize_t ret;

	LOG_INTERCEPT_FD(fd);

	MAP_OR_FAIL(preadv64);

	ret = __real_preadv64(fd, iov, iovcnt, offset);

	if( (ret != -1) || (errno != EINVAL) )
		return(ret);

	InjectionTk::ejectAfterEinval(fd);

	ret = __real_preadv64(fd, iov, iovcnt, offset);

	return(ret);
}

extern "C" ssize_t INTERCEPT_FUNC_DECL(preadv2)(int fd, const struct iovec *iov, int iovcnt,
	off_t offset, int flags)
{
	ssize_t ret;

	LOG_INTERCEPT_FD(fd);

	MAP_OR_FAIL(preadv2);

	ret = __real_preadv2(fd, iov, iovcnt, offset, flags);

	if( (ret != -1) || (errno != EINVAL) )
		return(ret);

	InjectionTk::ejectAfterEinval(fd);

	ret = __real_preadv2(fd, iov, iovcnt, offset, flags);

	return(ret);
}

extern "C" ssize_t INTERCEPT_FUNC_DECL(preadv64v2)(int fd, const struct iovec *iov, int iovcnt,
	off64_t offset, int flags)
{
	ssize_t ret;

	LOG_INTERCEPT_FD(fd);

	MAP_OR_FAIL(preadv64v2);

	ret = __real_preadv64v2(fd, iov, iovcnt, offset, flags);

	if( (ret != -1) || (errno != EINVAL) )
		return(ret);

	InjectionTk::ejectAfterEinval(fd);

	ret = __real_preadv64v2(fd, iov, iovcnt, offset, flags);

	return(ret);
}

extern "C" ssize_t INTERCEPT_FUNC_DECL(writev)(int fd, const struct iovec *iov, int iovcnt)
{
	ssize_t ret;

	LOG_INTERCEPT_FD(fd);

	MAP_OR_FAIL(writev);

	ret = __real_writev(fd, iov, iovcnt);

	if( (ret != -1) || (errno != EINVAL) )
		return(ret);

	InjectionTk::ejectAfterEinval(fd);

	ret = __real_writev(fd, iov, iovcnt);

	return(ret);
}

extern "C" ssize_t INTERCEPT_FUNC_DECL(pwritev)(int fd, const struct iovec *iov, int iovcnt,
	off_t offset)
{
	ssize_t ret;

	LOG_INTERCEPT_FD(fd);

	MAP_OR_FAIL(pwritev);

	ret = __real_pwritev(fd, iov, iovcnt, offset);

	if( (ret != -1) || (errno != EINVAL) )
		return(ret);

	InjectionTk::ejectAfterEinval(fd);

	ret = __real_pwritev(fd, iov, iovcnt, offset);

	return(ret);
}

extern "C" ssize_t INTERCEPT_FUNC_DECL(pwritev64)(int fd, const struct iovec *iov, int iovcnt,
	off64_t offset)
{
	ssize_t ret;

	LOG_INTERCEPT_FD(fd);

	MAP_OR_FAIL(pwritev64);

	ret = __real_pwritev64(fd, iov, iovcnt, offset);

	if( (ret != -1) || (errno != EINVAL) )
		return(ret);

	InjectionTk::ejectAfterEinval(fd);

	ret = __real_pwritev64(fd, iov, iovcnt, offset);

	return(ret);
}

extern "C" ssize_t INTERCEPT_FUNC_DECL(pwritev2)(int fd, const struct iovec *iov, int iovcnt,
	off_t offset, int flags)
{
	ssize_t ret;

	LOG_INTERCEPT_FD(fd);

	MAP_OR_FAIL(pwritev2);

	ret = __real_pwritev2(fd, iov, iovcnt, offset, flags);

	if( (ret != -1) || (errno != EINVAL) )
		return(ret);

	InjectionTk::ejectAfterEinval(fd);

	ret = __real_pwritev2(fd, iov, iovcnt, offset, flags);

	return(ret);
}

extern "C" ssize_t INTERCEPT_FUNC_DECL(pwritev64v2)(int fd, const struct iovec *iov, int iovcnt,
	off64_t offset, int flags)
{
	ssize_t ret;

	LOG_INTERCEPT_FD(fd);

	MAP_OR_FAIL(pwritev64v2);

	ret = __real_pwritev64v2(fd, iov, iovcnt, offset, flags);

	if( (ret != -1) || (errno != EINVAL) )
		return(ret);

	InjectionTk::ejectAfterEinval(fd);

	ret = __real_pwritev64v2(fd, iov, iovcnt, offset, flags);

	return(ret);
}


extern "C" int INTERCEPT_FUNC_DECL(fsync)(int fd)
{
	int ret;

	LOG_INTERCEPT_FD(fd);

	MAP_OR_FAIL(fsync);

	ret = __real_fsync(fd);

	if(ret < 0)
		return(ret);

	return(ret);
}

extern "C" int INTERCEPT_FUNC_DECL(fdatasync)(int fd)
{
	int ret;

	LOG_INTERCEPT_FD(fd);

	MAP_OR_FAIL(fdatasync);

	ret = __real_fdatasync(fd);

	if(ret < 0)
		return(ret);

	return(ret);
}

extern "C" int INTERCEPT_FUNC_DECL(closedir)(DIR *dirp)
{
	int ret;

	int fd = dirfd(dirp);

	LOG_INTERCEPT_FD(fd);

	if(initDone)
		InjectionTk::ejectBeforeClose(fd);

	MAP_OR_FAIL(closedir);

	ret = __real_closedir(dirp);

	return(ret);
}

extern "C" int INTERCEPT_FUNC_DECL(fclose)(FILE* stream)
{
	int ret;

	int fd = fileno(stream);

	LOG_INTERCEPT_FD(fd);

	if(initDone)
		InjectionTk::ejectBeforeClose(fd);

	MAP_OR_FAIL(fclose);

	ret = __real_fclose(stream);

	return(ret);
}

extern "C" int INTERCEPT_FUNC_DECL(close)(int fd)
{
	int ret;

	LOG_INTERCEPT_FD(fd);

	if(initDone)
		InjectionTk::ejectBeforeClose(fd);

	MAP_OR_FAIL(close);

	ret = __real_close(fd);

	return(ret);
}

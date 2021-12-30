/*
 * This provides a few simple test cases for the O_DIRECT injection. It is used with a single path
 * (file or dir) as argument.
 *
 * Example with file argument, called from the source root directory:
 * $ make test
 * $ echo abc > /tmp/test
 * $ echo /tmp/test > /tmp/pathfile
 * $ LD_VAST_PATHFILE=/tmp/pathfile LD_VAST_LOG_TOPICS=126 LD_PRELOAD=~/project/vast_preload_lib/bin/libvastpreload.so test/test /tmp/test
 *
 * The output of the example will show where O_DIRECT gets injected. If more details are required,
 * the whole command can also be used with strace like this: strace -f bash -c "<EXAMPLE_COMMAND>"
 */


#include <cstdlib>
#include <cstdio>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


void print_stat_buf(struct stat* stat_buf)
{
   std::cout << "size: " << stat_buf->st_size << " " <<
      "atime: " << stat_buf->st_atime << " "
      "mtime: " << stat_buf->st_mtime << " "
      "ctime: " << stat_buf->st_ctime <<
      std::endl;
}

int main(int argc, char** argv)
{
   if(argc < 2)
   {
      std::cerr << "Usage: " << argv[0] << " <path>" << std::endl;
      return 1;
   }

   const char* path = argv[1];

   std::cout << std::endl;

   {
	   std::cout << "* open 3-args: " << path << std::endl;

	   int fd = open(path, O_RDONLY, 0600);

	   std::cout << "* open 3-args res: " << fd << std::endl;
   }

   std::cout << std::endl;

   {
	   std::cout << "* open 2-args DIRECT: " << path << std::endl;

	   int fd = open(path, O_RDONLY | O_DIRECT);

	   std::cout << "* open 2-args res: " << fd << std::endl;
   }

   std::cout << std::endl;

   {
	   std::cout << "* openat 4-args: " << path << std::endl;

	   int fd = openat(3, path, O_RDONLY, 0600);

	   std::cout << "* openat 4-args res: " << fd << std::endl;
   }

   std::cout << std::endl;

   {
	   std::cout << "* open 2-args as DIR: " << path << std::endl;

	   int fd = open(path, O_RDONLY | O_DIRECTORY);

	   std::cout << "* open 3-args res: " << fd << std::endl;
   }

   std::cout << std::endl;

   {
	   std::cout << "* opendir 1-args: " << path << std::endl;

	   DIR* dir = opendir(path);

	   std::cout << "* opendir 1-args res: " << (!dir ? "NULL" : "SUCCESS") << std::endl;
   }

   std::cout << std::endl;

   {
	   std::cout << "* fopen 2-args: " << path << std::endl;

	   FILE* file = fopen(path, "r");

	   std::cout << "* fopen 2-args res: " << (!file ? "NULL" : "SUCCESS") << std::endl;

	   if(file)
	   {
		   std::cout << "* setting fread buf..." << std::endl;

		   size_t bufSize = 4*1024*1024;
		   char* readaheadBuf = (char*)malloc(bufSize);
		   char* ioBuf = (char*)malloc(bufSize);

		   int setBufRes = setvbuf(file, readaheadBuf, _IOFBF, bufSize);

		   std::cout << "* setting fread buf res: " << setBufRes << std::endl;

		   std::cout << "* doing fread..." << std::endl;

		   size_t readRes = fread(ioBuf, 1, 1, file);

		   std::cout << "* fread res: " << readRes << std::endl;

		   std::cout << "* closing fopen'ed file..." << std::endl;

		   int fcloseRes = fclose(file);

		   std::cout << "* fclose res: " << fcloseRes << std::endl;
	   }

	   std::cout << "* closing intentionally invalid fd \"-1\"..." << std::endl;

	   int closeRes = close(-1);

	   std::cout << "* close res: " << closeRes << std::endl;
   }

   std::cout << "* All done." << std::endl;
}

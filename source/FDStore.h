#ifndef FDSTORE_H_
#define FDSTORE_H_

#include <map>
#include <mutex>
#include <string>
#include "Common.h"


typedef std::map<int, std::string> FDMap;
typedef FDMap::iterator FDMapIter;
typedef FDMap::const_iterator FDMapConstIter;


class FDStore
{
	public:

	private:
		FDMap fdMap;
		std::mutex mutex;

		// inliners
	public:
		void addFD(int fd, std::string path)
		{
			std::unique_lock<std::mutex> lock(mutex); // L O C K (scoped)

			fdMap[fd] = path;
		}

		void removeFD(int fd)
		{
			std::unique_lock<std::mutex> lock(mutex); // L O C K (scoped)

			fdMap.erase(fd);
		}

		void removeFD(int fd, std::string& outRemovedPath)
		{
			std::unique_lock<std::mutex> lock(mutex); // L O C K (scoped)

			FDMapConstIter iter = fdMap.find(fd);

			if(iter == fdMap.end() )
				return;

			outRemovedPath = iter->second;

			fdMap.erase(iter);
		}

		bool getIsFDInStore(int fd)
		{
			std::unique_lock<std::mutex> lock(mutex); // L O C K (scoped)

			return (fdMap.find(fd) != fdMap.end() );
		}

		bool getFD(int fd, std::string& outString)
		{
			std::unique_lock<std::mutex> lock(mutex); // L O C K (scoped)

			FDMapConstIter iter = fdMap.find(fd);

			if(iter == fdMap.end() )
				return false;

			outString = iter->second;

			return true;
		}

};

extern FDStore* fdStore;

#endif /* FDSTORE_H_ */

#ifndef INJECTIONTK_H_
#define INJECTIONTK_H_

#include <string.h>
#include "Common.h"
#include "Config.h"
#include "FDStore.h"
#include "Logger.h"
#include "PathMatchStore.h"


/**
 * Toolkit to inject O_DIRECT on file open and to eject it on close or error.
 */
class InjectionTk
{
	public:
		static void injectAfterOpen(int fd, std::string path, int flags);
		static void injectAfterOpenat(int dirfd, int fd, std::string path, int flags);
		static void ejectAfterEinval(int fd);
		static void ejectBeforeClose(int fd);

	private:
		InjectionTk() {}
};



#endif /* INJECTIONTK_H_ */

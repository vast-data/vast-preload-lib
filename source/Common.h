#ifndef COMMON_H_
#define COMMON_H_

#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <dlfcn.h>
#include <iostream>
#include <list>
#include <set>
#include <string>
#include <unistd.h>
#include <vector>

extern bool initDone; // defined and set in Main.cpp when initialization is done

typedef std::list<std::string> StringList;
typedef std::set<std::string> StringSet;
typedef std::vector<std::string> StringVec;
typedef std::vector<int> IntVec;
typedef std::vector<char*> BufferVec;
typedef std::vector<size_t> SizeTVec;
typedef std::vector<uint64_t> UInt64Vec;

/**
 * Return length of a static C string (as in "mystring").
 */
#define STRINGIZE(value)		_STRINGIZE(value) // 2 levels necessary for macro expansion
#define _STRINGIZE(value)		#value

/**
 * Minimum of a and b.
 */
#define MIN(a, b) 			( (a < b) ? a : b)

/**
 * Call delete() on an object pointer if it's not NULL and afterwards set it to NULL.
 */
#define SAFE_DELETE(objectPointer) \
	do \
	{ \
		if(objectPointer) \
		{ \
			delete(objectPointer); \
			objectPointer = NULL; \
		}  \
	} while(0)

/**
 * Call free on a pointer if it's not NULL and afterwards set it to NULL.
 */
#define SAFE_FREE(pointer) \
	do \
	{ \
		if(pointer) \
		{ \
			free(pointer); \
			pointer = NULL; \
		}  \
	} while(0)

/**
 * Declare a function type and corresponding variable, which is initialized to NULL. This is for
 * dlsym (MAP_OR_FAIL).
 */
#define FUNC_FORWARD_DECL(__func,__ret,__args) \
  typedef __ret (*__real_ ## __func ## _TYPE)__args; \
  __real_ ## __func ## _TYPE __real_ ## __func = NULL;

/**
 * A no-op to visually mark functions that we want to intercept.
 */
#define INTERCEPT_FUNC_DECL(__func)	__func

/**
 * Assign the pointer to an overloaded function to the function pointer declared via FUNC_FORWARD_DECL.
 */
#define MAP_OR_FAIL(__func) \
	if (!(__real_ ## __func)) \
	{ \
		__real_ ## __func = (__real_ ## __func ## _TYPE)dlsym(RTLD_NEXT, #__func); \
		if(!(__real_ ## __func)) { \
			log_fprintf(stderr, LOG_PREFIX "Failed to find symbol: %s\n", #__func); \
			exit(1); \
	} \
}

#endif /* COMMON_H_ */

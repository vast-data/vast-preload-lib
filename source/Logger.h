#ifndef LOGGER_H_
#define LOGGER_H_

#include "Common.h"
#include "Config.h"

#define ENV_LOG_TOPICS				"LD_VAST_LOG_TOPICS"
#define ENV_LOG_TOPIC_INTERCEPT		1 // call intercepted
#define ENV_LOG_TOPIC_INJECT		2 // parameters changed/injected
#define ENV_LOG_TOPIC_INJECT_SKIP	4 // skipped injection reasons
#define ENV_LOG_TOPIC_INIT			8 // initialization info
#define ENV_LOG_TOPIC_CLOSE			16 // close ops with path
#define ENV_LOG_TOPIC_NORMALIZE		32 // normalized paths (if different from original)


#define LOG_PREFIX		LIB_NAME ": "

#define LOG_INTERCEPT_PATH(path) \
	do \
	{ \
		if(libLogTopics & ENV_LOG_TOPIC_INTERCEPT) \
			log_fprintf(stderr, LOG_PREFIX "%s: %s %s\n", \
			initDone ? "INTERCEPTING" : "not intercepting", __func__, path); \
	} while(0)

#define LOG_INTERCEPT_FD(fd) \
	do \
	{ \
		if(libLogTopics & ENV_LOG_TOPIC_INTERCEPT) \
			log_fprintf(stderr, LOG_PREFIX "%s: %s; fd: %d\n", \
			initDone ? "INTERCEPTING" : "not intercepting", __func__, fd); \
	} while(0)



void log_fprintf(FILE *stream, const char *format, ...);



#endif /* LOGGER_H_ */

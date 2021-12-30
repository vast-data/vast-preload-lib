#include "Common.h"
#include "FDStore.h"
#include "Logger.h"
#include "PathMatchStore.h"


bool initDone = false; // set to true at end of initlib() to avoid intercepts during init


/**
 * Called when the library is loaded to init basic data structures.
 *
 * Note: std::cout/cerr is not usable here.
 */
static __attribute__((constructor)) void initlib(void)
{
	const char* libLogTopicsStr = getenv(ENV_LOG_TOPICS);
	if(libLogTopicsStr)
		libLogTopics = atoi(libLogTopicsStr);

	if(libLogTopics & ENV_LOG_TOPIC_INIT)
	{
		log_fprintf(stderr, LOG_PREFIX "Initializer called (pid: %d)\n", (int)getpid() );
		log_fprintf(stderr, LOG_PREFIX "libLogTopics: %d\n", libLogTopics);
	}

	const char* libOptsStr = getenv(ENV_LIB_OPTS);
	if(libOptsStr)
	{
		libOpts = atoi(libOptsStr);

		if(libLogTopics & ENV_LOG_TOPIC_INIT)
			log_fprintf(stderr, LOG_PREFIX "libOpts: %d\n", libOpts);
	}

	const char* libPathFileStr = getenv(ENV_LIB_PATHFILE);
	if(libPathFileStr)
	{
		// needs to be alloc'ed here as it's otherwise not initialized as static var
		pathMatchStore = new PathMatchStore();

		bool loadRes = pathMatchStore->loadPathFile(libPathFileStr);

		if(!loadRes)
		{
			log_fprintf(stderr, LOG_PREFIX "ERROR: Loading of path file failed: %s\n",
				libPathFileStr);
			exit(1);
		}

	}

	if(libLogTopics & ENV_LOG_TOPIC_INIT)
		log_fprintf(stderr, LOG_PREFIX "Loaded paths: %d\n", pathMatchStore->size() );

	// needs to be alloc'ed here as it's otherwise not initialized as static var
	fdStore = new FDStore();

	initDone = true; // signal to interceptors that they can be effective now

	if(libLogTopics & ENV_LOG_TOPIC_INIT)
		log_fprintf(stderr, LOG_PREFIX "Init complete\n");
}

/**
 * Called when the library is unloaded to cleanup data structures.
 */
static __attribute__((destructor)) void uninitlib(void)
{
	if(libLogTopics & ENV_LOG_TOPIC_INIT)
		fprintf(stderr, LOG_PREFIX "Uninit called\n");

	SAFE_DELETE(pathMatchStore);
	SAFE_DELETE(fdStore);

	if(libLogTopics & ENV_LOG_TOPIC_INIT)
		fprintf(stderr, LOG_PREFIX "Uninit complete\n");
}

#ifndef CONFIG_H_
#define CONFIG_H_

#include "Common.h"

#define ENV_LIB_OPTS				"LD_VAST_OPTS" // no opts defined yet
#define ENV_LIB_OPT_RAWPATHS		1 // don't normalize paths (e.g. remove ".." and "//")

#define ENV_LIB_PATHFILE			"LD_VAST_PATHFILE" // paths for which injection is effective


extern int libLogTopics; // flags
extern int libOpts; // flags
#define ENV_PATHS_LEN	1023
extern char libInjectPaths[ENV_PATHS_LEN+1]; // +1 for trailing zero


#endif /* CONFIG_H_ */

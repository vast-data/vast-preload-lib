#include "Common.h"
#include "Logger.h"

FUNC_FORWARD_DECL(vfprintf, int, (FILE *stream, const char *format, va_list) );


/**
 * This fprintf wrapper exists to ensure that the real fprintf function gets called.
 */
void log_fprintf(FILE *stream, const char *format, ...)
{
	va_list ap;

	MAP_OR_FAIL(vfprintf);

	va_start(ap, format);
	__real_vfprintf(stream, format, ap);
	va_end(ap);

	return;
}

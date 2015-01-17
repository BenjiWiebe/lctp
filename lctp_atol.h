#ifndef __LCTP_ATOL_H__
#define __LCTP_ATOL_H__
#include <stdlib.h>
#include <errno.h>
#ifndef LCTP_ATOL_onerror
#warning	"LCTP_ATOL_onerror should be defined."
#endif
#define LCTP_ATOL(str, dst, min, max) do { \
	errno = 0; \
	dst = strtol(str, NULL, 10); \
	if(errno != 0) \
		{LCTP_ATOL_onerror} \
	} while(0)

#endif

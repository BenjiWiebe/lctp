#ifndef __LCTP_ATOL_H__
#define __LCTP_ATOL_H__
#include <stdlib.h>
#ifndef lctp_atol_error_code
#error	"lctp_atol_error_code must be defined."
#endif
#define lctp_atol(str, min, max) do { \
	errno = 0; \
	str = strtol(s ## str, NULL, 10); \
	if(errno != 0) \
		{int RANGE_EXCEEDED=0; lctp_atol_error_code;} \
	if((str > max) || (str < min)) \
		{int RANGE_EXCEEDED=1; lctp_atol_error_code;} \
	} while(0)

#endif

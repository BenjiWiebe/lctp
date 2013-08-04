#ifndef __DATETIM_H_INC__
#define __DATETIM_H_INC__

#include <time.h>
#define FMT_ERR		((time_t)(-1))
#define MAX_YEAR	2500
#define MIN_YEAR	1900

#ifdef __cplusplus
extern "C" {
#endif

time_t parse_date(char*,char);
time_t parse_datetime(char*,char,char);

#ifdef __cplusplus
}
#endif

#endif

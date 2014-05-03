#ifndef __DATETIME_H_INC__
#define __DATETIME_H_INC__

#include <time.h>
#define FMT_ERR		((time_t)(-1))
#define MAX_YEAR	5000
#define MIN_YEAR	1970
#define SECONDS(x)	(x)
#define MINUTES(x)	((x)*SECONDS(60))
#define HOURS(x)	((x)*MINUTES(60))
#define DAYS(x)		((x)*HOURS(24))

#ifdef __cplusplus
extern "C" {
#endif

time_t parse_date(char*,char);
time_t parse_datetime(char*,char,char);
char *basicdate(time_t*);

#ifdef __cplusplus
}
#endif

#endif

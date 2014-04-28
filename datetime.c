#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "datetime.h"

#define satio(str, max, min)	errno = 0; \
	str = strtol(s ## str, NULL, 10); \
	if(errno != 0) \
		return FMT_ERR; \
	if((str > max) || (str < min)) \
		return FMT_ERR

time_t parse_date(char *str, char sep)
{
	/* format (x is _sep_): mmxddxyyyy */

	if(strlen(str) != 10)
		return FMT_ERR;

	char syear[5];
	char smonth[3];
	char sday[3];
	int year = 0, month = 0, day = 0;

	strncpy(smonth, str, 2);
	smonth[2] = 0;
	satio(month, 12, 1);

	strncpy(sday, str+3, 2);
	sday[2] = 0;
	satio(day, 31, 1);

	strncpy(syear, str+6, 4);
	syear[4] = 0;
	satio(year, 3000, 1900);

	if((str[2] != sep) || (str[5] != sep))
		return FMT_ERR;

	struct tm *utm = calloc(1, sizeof(struct tm));
	utm->tm_mday = day;
	utm->tm_mon = month - 1;
	utm->tm_year = year - 1900;

	time_t t = mktime(utm);
	free(utm);
	if(t == -1)
		return FMT_ERR;
	return t;
}

time_t parse_datetime(char *str, char dsep, char tsep)
{
	/* format (2 is _tsep_, 1 is _dsep_): mm1dd1yyyyhh2mm */
	if(strlen(str) != 15)
		return FMT_ERR;

	if(str[2] != dsep || str[5] != dsep || str[12] != tsep)
		return FMT_ERR;

	char syear[5];
	char smonth[3];
	char sday[3];
	char shour[3];
	char sminute[3];
	int year = 0, month = 0, day = 0, hour = 0, minute = 0;

	strncpy(smonth, str, 2);
	smonth[2] = 0;
	satio(month, 12, 1);
	str += 3;

	strncpy(sday, str, 2);
	sday[2] = 0;
	satio(day, 31, 1);
	str += 3;

	strncpy(syear, str, 4);
	syear[4] = 0;
	satio(year, MAX_YEAR, MIN_YEAR);
	str += 4;

	strncpy(shour, str, 2);
	shour[2] = 0;
	satio(hour, 23, 0);
	str += 3;

	strncpy(sminute, str, 2);
	sminute[2] = 0;
	satio(minute, 59, 0);

		
	struct tm *utm = calloc(1, sizeof(struct tm));

	utm->tm_mday = day;
	utm->tm_mon = month - 1;
	utm->tm_year = year - 1900;
	utm->tm_min = minute;
	utm->tm_hour = hour;
	utm->tm_isdst = -1;

	time_t t = mktime(utm);
	free(utm);
	if(t == -1)
		return FMT_ERR;
	return t;
}

char *basicdate(time_t *t)
{
	char *tmp = malloc(11);
	struct tm *s = localtime(t);
	strftime(tmp, 11, "%m-%d-%Y", s);
	return tmp;
}

#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "datetime.h"
#define lctp_atol_error_code	return FMT_ERR;
#include "lctp_atol.h"

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
	lctp_atol(month, 1, 12);

	strncpy(sday, str+3, 2);
	sday[2] = 0;
	lctp_atol(day, 1, 31);

	strncpy(syear, str+6, 4);
	syear[4] = 0;
	lctp_atol(year, MIN_YEAR, MAX_YEAR);

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
	lctp_atol(month, 1, 12);
	str += 3;

	strncpy(sday, str, 2);
	sday[2] = 0;
	lctp_atol(day, 1, 31);
	str += 3;

	strncpy(syear, str, 4);
	syear[4] = 0;
	lctp_atol(year, MIN_YEAR, MAX_YEAR);
	str += 4;

	strncpy(shour, str, 2);
	shour[2] = 0;
	lctp_atol(hour, 0, 23);
	str += 3;

	strncpy(sminute, str, 2);
	sminute[2] = 0;
	lctp_atol(minute, 0, 59);

		
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

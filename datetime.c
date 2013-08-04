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
	/* format (assuming sep == '-'): mm-dd-yyyy */
	int i = strlen(str);

	if(i != 10)
	{
		return FMT_ERR;
	}

	char syear[5];
	char smonth[3];
	char sday[3];

	syear[4] = 0;
	smonth[2] = 0;
	sday[2] = 0;

	int year = 0, month = 0, day = 0;

	while(i--)
	{
		switch(i)
		{
			case 0:
			case 1:
				smonth[i] = str[i];
				break;
			case 3:
			case 4:
				sday[i - 3] = str[i];
				break;
			case 2:
			case 5:
				if(str[i] != sep)
				{
					return FMT_ERR;
				}
				break;
			case 6:
			case 7:
			case 8:
			case 9:
				syear[i - 6] = str[i];
				break;
			default:
				return FMT_ERR;
				break;
		}
	}

	satio(month, 12, 1);
	satio(day, 31, 1);
	satio(year, 3000, 1900);

	struct tm *utm = malloc(sizeof(struct tm));
	memset(utm, 0, sizeof(struct tm));

	utm->tm_mday = day;
	utm->tm_mon = month - 1;
	utm->tm_year = year - 1900;

	time_t t = mktime(utm);
	free(utm);
	if(!(t > 0))
	{
		return FMT_ERR;
	}
	return t;
}

time_t parse_datetime(char *str, char dsep, char tsep)
{
	/* format (assuming tsep == ':' and dsep == '-'): mm-dd-yyyyhh:mm */
	int i = strlen(str);

	if(i != 15)
	{
		return FMT_ERR;
	}

	char syear[5];
	char smonth[3];
	char sday[3];
	char shour[3];
	char sminute[3];

	syear[4] = 0;
	smonth[2] = 0;
	sday[2] = 0;
	shour[2] = 0;
	sminute[2] = 0;

	int year = 0, month = 0, day = 0, hour = 0, minute = 0;
	while(i--)
	{
		switch(i)
		{
			case 0:
			case 1:
				smonth[i] = str[i];
				break;
			case 3:
			case 4:
				sday[i - 3] = str[i];
				break;
			case 2:
			case 5:
				if(str[i] != dsep)
				{
					return FMT_ERR;
				}
				break;
			case 6:
			case 7:
			case 8:
			case 9:
				syear[i - 6] = str[i];
				break;
			case 10:
			case 11:
				shour[i - 10] = str[i];
				break;
			case 12:
				if(str[i] != tsep)
				{
					return FMT_ERR;
				}
				break;
			case 13:
			case 14:
				sminute[i - 13] = str[i];
				break;
			default:
				return FMT_ERR;
				break;
		}
	}

	satio(month, 12, 1);
	satio(day, 31, 1);
	satio(year, MAX_YEAR, MIN_YEAR);
	satio(hour, 23, 0);
	satio(minute, 59, 0);

	struct tm *utm = malloc(sizeof(struct tm));

	utm->tm_mday = day;
	utm->tm_mon = month - 1;
	utm->tm_year = year - 1900;
	utm->tm_min = minute;
	utm->tm_hour = hour;
	utm->tm_isdst = -1;

	time_t t = mktime(utm);
	//time_t t = (year * 12)
	free(utm);
	if(!(t > 0))
	{
		return FMT_ERR;
	}
	return t;
}

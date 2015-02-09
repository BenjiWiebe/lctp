#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include "lctp.h"

#define DAT_LINE_LEN	31
#define DAT_DATE_SEP	'/'
#define DAT_TIME_SEP	':'

int lctp_procline_atol(char *str, int *i, int min, int max)
{
	errno = 0;
	*i = strtol(str, NULL, 10);
	if(errno != 0)
	{
		return -1;
	}
	if((*i > max) || (*i < min))
	{
		errno = ERANGE;
		return -1;
	}
	return 0;
}

enum lctp_procline_errors { PLE_OK = 0, PLE_LEN, PLE_ARGS, PLE_IO, PLE_TIMESEP, PLE_DATESEP, PLE_SPACES, PLE_NEWLINE, PLE_CONVERR, PLE_RANGE, PLE_UNKNOWN }; 

static const char *ple_strs[] = {"Success", "Line length invalid", "Invalid argument", "Wrong value for IN/OUT field", "Invalid time separator", "Invalid date separator", "Invalid number of spaces", "Missing trailing newline", "Unknown number-parsing error", "Date or time value out of range", "Unknown error"};

struct data_entry {
	enum actions action; // ACTION_IN or ACTION_OUT
	uint8_t month; // 0-11
	uint8_t day; // 1-31
	uint16_t year; // years since 1900
	uint8_t hour; // 0-23
	uint8_t minute; // 0-59
	uint16_t comment; // 1000-9999
	uint8_t is_comment; // 0/1 (line is/not a comment)
};

#define convert2(str, var)	if(!isdigit(str[0]) || !isdigit(str[1])) \
	return PLE_CONVERR; \
	var = (str[0] - '0') * 10 + str[1] - '0';

#define convert4(str, var)	if(!isdigit(str[0]) || !isdigit(str[1]) || !isdigit(str[2]) || !isdigit(str[3])) \
	return PLE_CONVERR; \
	var = ((int)str[0] - '0') * 1000 + ((int)str[1] - '0') * 100 + ((int)str[2] - '0') * 10 + (int)str[3] - '0';

// Checks a line's syntax and creates a struct data_entry from it
// Also checks to make sure that numbers are in range
// Returns 0 on success, and an lctp_procline_errors member on error
static enum lctp_procline_errors mk_data_entry(char *line, struct data_entry *ret)
{
	if(line == NULL || ret == NULL)
		return PLE_ARGS;

	if(line[0] == '#')
	{
		ret->is_comment = 1;
		return PLE_OK;
	}

	if(!strncmp(line, "IN ", 3))
		ret->action = ACTION_IN;
	else if(!strncmp(line, "OUT", 3))
		ret->action = ACTION_OUT;
	else
		return PLE_IO;
	line += 3;

	if(strncmp(line, "   ", 3))
		return PLE_SPACES;
	line += 3;

	convert2(line, ret->month);
	ret->month -= 1;
	line += 2;

	if(line[0] != DAT_DATE_SEP)
		return PLE_DATESEP;
	line += 1;

	convert2(line, ret->day);
	line += 2;

	if(line[0] != DAT_DATE_SEP)
		return PLE_DATESEP;
	line += 1;

	convert4(line, ret->year);
	ret->year -= 1900;
	line += 4;

	if(strncmp(line, "  ", 2))
		return PLE_SPACES;
	line += 2;

	int has_comment = 0;
	if(strncmp(line, "    ", 4))
	{
		has_comment = 1;
		convert4(line, ret->comment);
	}
	line += 4;

	if(strncmp(line, "  ", 2))
		return PLE_SPACES;
	line += 2;

	convert2(line, ret->hour);
	line += 2;

	if(line[0] != DAT_TIME_SEP)
		return PLE_TIMESEP;
	line += 1;

	convert2(line, ret->minute);
	line += 2;

	if(line[0] == '\r')
	{
		line += 1;
		if(line[0] != '\n')
			return PLE_NEWLINE;
	}
	else if(line[0] == '\n')
	{
		line += 1;
	}
	else
	{
		return PLE_NEWLINE;
	}

	if(line[0] != 0)
		return PLE_LEN;

	// All values are unsigned int's; no need to check for < 0
	if(ret->year + 1900 > 3000 ||
			ret->month > 11 ||
			ret->day < 1 || ret->day > 31 ||
			ret->hour > 23 ||
			ret->minute > 59 ||
			(has_comment && (ret->comment < 1000 || ret->comment > 9999)))
		return PLE_RANGE;
	return 0;
}

// Gets a human-readable status message (statically allocated)
/*char *lctp_getstatus(char *line)
{
	*
Last action: OUT @ 12:30 PM 09/31
Last action: OUT @ 12:30 PM Wednesday, September 31
	 *
	struct text_data_entry e;
	//mktextdata(&e, line);
	static char st[52];
	st[0] = 0;
	uint8_t normhour = (e.hour[0] - '0') * 10 + e.hour[1] - '0';
	char ampm = 'A';
	if(normhour > 12)
	{
		normhour -= 12;
		ampm = 'P';
	}
	snprintf(st, 50, "Last action: %s @ %02d:%s %cM %s/%s", 
			e.io,
			normhour,
			e.minute,
			ampm,
			e.month,
			e.day);
	return st;
}*/

int lctp_procline(struct lctp_lineinfo *i, char *line)
{
	enum lctp_procline_errors err;
	struct data_entry ent = {0};
	err = mk_data_entry(line, &ent);
	i->error_message = ple_strs[err];
	if(err != PLE_OK)
		return -1;
	if(ent.is_comment)
	{
		i->is_comment = 1;
		return 0;
	}
	else
	{
		i->is_comment = 0;
	}
	i->action = ent.action;
	i->commentno = ent.comment;
	struct tm t = {0};
	t.tm_isdst = -1;
	t.tm_min = ent.minute;
	t.tm_hour = ent.hour;
	t.tm_mday = ent.day;
	t.tm_mon = ent.month;
	t.tm_year = ent.year;
	i->time = mktime(&t);
	return 0;
}

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#define LCTP_ATOL_onerror	i->error = errno == ERANGE ? PLE_RANGE : PLE_CONVERR; return -1;
#include "lctp_atol.h"
#include "lctp.h"

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

static const char *ple_strs[] = {"Success", "Line length invalid", "Invalid argument", "Wrong value for IN/OUT field", "Invalid time separator", "Invalid date separator", "Invalid number of spaces", "Missing trailing newline", "Unknown number-parsing error", "Date or time value out of range", "Unknown error"};

const char *lctp_procline_strerror(enum lctp_procline_errors err)
{
	return ple_strs[err];
}

struct text_data_entry {
	char *line; // The original pointer to the line
	char *io; // "IN" or "OUT"
	char *month; // the month
	char *day; // the day
	char *year; // the year
	char *hour; // the hour (24-hour)
	char *minute; // the minute
	char *comment; // the comment
};

struct data_entry {
	enum actions action;
	uint8_t month;
	uint8_t day;
	uint16_t year;
	uint8_t hour;
	uint8_t minute;
	uint16_t comment;
};

// Convert a line to a struct text_data_entry
// Returns 0 on success, and an lctp_procline_errors member on error
static int mktextdata(struct text_data_entry *e, char *line)
{
	if(e == NULL || line == NULL)
		return PLE_UNKNOWN;

	// Save the original pointer, in case we need to free() it or something
	e->line = line;

	// Save the action (IN or OUT) and NULL-terminate it. Uses 4 bytes of 'line' ("OUT\0" or "IN\0\0")
	e->io = line;
	if(e->io[2] == ' ')
		e->io[2] = 0;
	else
		e->io[3] = 0;
	line += 4;

	line += 2;

	e->month = line;
	e->month[2] = 0;
	line += 3;

	e->day = line;
	e->day[2] = 0;
	line += 3;

	e->year = line;
	e->year[4] = 0;
	line += 5;

	line += 1;

	e->comment = line;
	e->comment[4] = 0;
	line += 5;

	line += 1;

	e->hour = line;
	e->hour[2] = 0;
	line += 3;

	e->minute = line;
	e->minute[2] = 0;

	return 0;
}

// Checks a line's syntax (not values of numbers or anything)
// Returns 0 on success, and an lctp_procline_errors member on error
static enum lctp_procline_errors validate(char *line)
{
}

// Checks a struct data_entry's data for out-of-range values, etc
// Returns 0 on success, and an lctp_procline_errors member on error
static enum lctp_procline_errors logic_check(struct text_data_entry *e)
{
}

// Converts a struct text_data_entry to a struct data_entry
static enum lctp_procline_errors convert(struct text_data_entry *e)
{
}

// Gets a human-readable status message (statically allocated)
char *lctp_getstatus(char *line)
{
	/*
Last action: OUT @ 12:30 PM 09/31
Last action: OUT @ 12:30 PM Wednesday, September 31
	 */
	struct text_data_entry e;
	mktextdata(&e, line);
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
}

int lctp_procline(struct lctp_lineinfo *i, char *line)
{
	if(i == NULL)
		return -1;

	if(line == NULL || i->timesep == 0 || i->datesep == 0)
	{
		i->error = PLE_ARGS;
		return -1;
	}
	if(line[0] == '#')
	{
		i->error = PLE_UNIXCOMMENT;
		return -1;
	}
	size_t linelen = strlen(line);
	if(linelen != 30 && linelen != 31)
	{
		i->error = PLE_LEN;
		return -1;
	}
	char strio[4];
	strncpy(strio, line, 3);
	strio[3] = 0;
	if(!strncmp(line, "IN    ", 6))
	{
		i->action = ACTION_IN;
	}
	else if(!strncmp(line, "OUT   ", 6))
	{
		i->action = ACTION_OUT;
	}
	else
	{
		i->error = PLE_IO;
		return -1;
	}

	line += 6;

	struct tm t = {0};
	t.tm_isdst = -1;

	char strdt[5];
	strdt[0] = line[0];
	strdt[1] = line[1];
	strdt[2] = 0;


	LCTP_ATOL(strdt, t.tm_mon, 1, 12);
	t.tm_mon--;
	line += 2;
	if(line[0] != i->datesep)
	{
		i->error = PLE_DATESEP;
		return -1;
	}
	line++;
	strdt[0] = line[0];
	strdt[1] = line[1];
	strdt[2] = 0;
	LCTP_ATOL(strdt, t.tm_mday, 1, 31);
	line += 2;
	if(line[0] != i->datesep)
	{
		i->error = PLE_DATESEP;
		return -1;
	}
	line++;
	strncpy(strdt, line, 4);
	strdt[4] = 0;
	int tmp = 0;
	LCTP_ATOL(strdt, tmp, 1900, 2200);
	t.tm_year = tmp - 1900;
	line += 4;
	if(strncmp(line, "  ", 2))
	{
		i->error = PLE_SPACES;
		return -1;
	}
	line += 2;
	if(strncmp(line, "    ", 4))
	{
		strncpy(strdt, line, 4);
		strdt[4] = 0;
		LCTP_ATOL(strdt, i->commentno, 1000, 9999);
	}
	else
	{
		i->commentno = 0;
	}
	line += 4;
	if(strncmp(line, "  ", 2))
	{
		i->error = PLE_SPACES;
		return -1;
	}
	line += 2;
	strdt[0] = line[0];
	strdt[1] = line[1];
	strdt[2] = 0;
	LCTP_ATOL(strdt, t.tm_hour, 0, 23);
	line += 2;
	if(line[0] != i->timesep)
	{
		i->error = PLE_TIMESEP;
		return -1;
	}
	line += 1;
	strdt[0] = line[0];
	strdt[1] = line[1];
	strdt[2] = 0;
	LCTP_ATOL(strdt, t.tm_min, 0, 59);
	line += 2;
	if(strcmp(line, "\r\n") && strcmp(line, "\n"))
	{
		i->error = PLE_NEWLINE;		
		return -1;
	}
	i->error = PLE_OK;
	i->time = mktime(&t);
	return PLE_OK;
}

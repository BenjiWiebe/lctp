#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "lctp_procline.h"

int lctp_procline_atol(char *str, int *i, int min, int max)
{
	errno = 0;
	*i = strtol(str, NULL, 10);
	if(errno != 0)
	{
		return errno;
	}
	if((*i > max) || (*i < min))
	{
		return ERANGE;
	}
}

static const char *ple_strs[] = {"Success", "Line length invalid", "Invalid argument", "Wrong value for IN/OUT field", "Invalid time separator", "Invalid date separator", "Invalid number of spaces", "Missing trailing newline"};

const char *lctp_procline_strerror(enum lctp_procline_errors err)
{
	return ple_strs[err];
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
	lctp_procline_atol(strdt, &t.tm_mon, 1, 12);	// TODO Handle error
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
	lctp_procline_atol(strdt, &t.tm_mday, 1, 31); // TODO Handle error
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
	lctp_procline_atol(strdt, &tmp, 1900, 2200); // TODO Handle error
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
		lctp_procline_atol(strdt, &i->commentno, 1000, 9999); // TODO Handle error
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
	strdt[0] = line[0];
	strdt[1] = line[1];
	strdt[2] = 0;
	lctp_procline_atol(strdt, &t.tm_hour, 0, 23); // TODO Handle error
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
	lctp_procline_atol(strdt, &t.tm_min, 0, 59); // TODO Handle error
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

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>
#include <getopt.h>
#include "chomp.h"
#include "datetime.h"
#define lctp_atol_error_code	formaterr(lineno);
#include "lctp_atol.h"
//#define LINELEN			30
//#define check(x)		if(!(x))formaterr(lineno)
#define format_error(x)	_format_error(x, argv[optind], line_num, __LINE__)

void usage(char *progname, int ret)
{
	printf("Usage: %s [OPTIONS] FILE\n", progname);
	printf("Options:\n");
	printf("  -s, --start <date>\tStart date\n");
	printf("  -e, --end <date>\tEnd date\n");
	printf("  -q, --quiet\t\tDisplay only hours\n");
	printf("  -h, --help\t\tPrint this help message and exit\n");
	printf("  -v, --version\t\tPrint version information and exit\n");
	exit(ret);
}

void apperr(char *fmt, ...)
{
	va_list l;
	va_start(l, fmt);
	vfprintf(stderr, fmt, l);
	va_end(l);
	exit(1);
}

void err(char *msg)
{
	perror(msg);
	exit(1);
}

void _format_error(char *message, char *file, int lineno, int sourceline)
{
	apperr("%s:%d: Format error: %s (%d)\n", file, lineno, message, sourceline);
}

char *get_line(FILE *fp)
{
	size_t size = 256;
	char *ret = calloc(size, sizeof(char));
	if(ret == NULL)
		err("calloc");
	int x = 0, i = 0;
	while((x = fgetc(fp)) != EOF)
	{
		if(x == '\r' || x == '\n')
			break;
		ret[i++] = (char)x;
		if(i == size)
		{
			int oldsize = size;
			size *= 1.5;
			ret = realloc(ret, size);
			if(ret == NULL)
				err("realloc");
			memset(ret + oldsize, 0, size - oldsize);
		}
	}
	return ret;
}

typedef enum
{
	ACTION_NIL = 0,
	ACTION_IN = 1,
	ACTION_OUT = 2
} ACTION;

int main(int argc, char *argv[])
{
	static struct option long_options[] =
	{
		{"start", required_argument, 0, 's'},
		{"end", required_argument, 0, 'e'},
		{"quiet", no_argument, 0, 'q'},
		{"help", no_argument, 0, 'h'},
		{"version", no_argument, 0, 'v'}
	};
	int opti;
	char *sstart = NULL, *send = NULL;
	bool quiet = false;

	while(1)
	{
		int c = getopt_long(argc, argv, "s:e:qhv", long_options, &opti);
		if(c == -1)
		{
			break;
		}
		switch(c)
		{
			case 's':
				sstart = optarg;
				break;
			case 'e':
				send = optarg;
				break;
			case 'q':
				quiet = true;
				break;
			case 'h':
				usage(argv[0], 0);
				break;
			case 'v':
				apperr("Not implemented.\n");
				break;
		}
	}

	if(!sstart != !send)
	{
		if(sstart)
		{
			apperr("End-date is also needed.\n");
		}
		if(send)
		{
			apperr("Start-date is also needed.\n");
		}
		usage(argv[0], 1);
	}

	time_t start_time, stop_time;

	if(sstart)
	{
		start_time = parse_date(sstart, '-');
		stop_time = parse_date(send, '-');

		if(start_time == FMT_ERR || stop_time == FMT_ERR)
			apperr("Start-date and end-date must be in the format mm-dd-yyyy.\n");

		if(start_time > stop_time)
			apperr("End date must be larger than or equal to the start date.");
	}
	else
	{
		start_time = -1;
		stop_time = -1;
	}

	if(optind != (argc - 1))
	{
		usage(argv[0], 1);
	}

	FILE *fp = fopen(argv[optind], "r");
	if(fp == NULL)
	{
		err("fopen");
	}
	char *line = NULL;
	int comment_number = 0, line_num = 0, len = 0;
	time_t total_time = 0, this_time = 0, last_time = 0, now_time = time(NULL);
	ACTION this_action = ACTION_NIL, last_action = ACTION_NIL;

	//while((line = get_line(fp)) != NULL)
	while(getline(&line, &len, fp) != -1)
	{
		char datetimestring[16]; // 10 chars, 5 chars, plus NULL ("mm/dd/yyyyhh:mm\0")
		line_num++;
		int line_len = strlen(line);
/*
 * Format of line:
 * ^IN    04/25/2014        03:00$
 * ^IN<4spaces>mm/dd/yyyy<2>cccc<2>hh:mm$
 * ^OUT   04/25/2014        11:00$
 * ^OUT<3spaces>mm/dd/yyyy<2>cccc<2>hh:mm$
 */

		// Put stuff from the last line into last_*
		last_action = this_action;
		last_time = this_time;

		// Remove line-ending characters
		if(line[line_len - 1] == '\n') // check last character of line
			line[--line_len] = 0;      // if it is \n, replace it with NULL, and decrement line_len
		if(line[line_len - 1] == '\r') // repeat the process checking for \r
			line[--line_len] = 0;      // and replacing \r

		// Check line length, WITHOUT the line-ending characters
		fprintf(stderr, "linelen:%d\nline:'%s'\n", line_len, line);
		if(line_len != 29)
			format_error("Line is not the correct length.");

		// Check type of entry, whether it is IN or OUT
		if(strncmp(line, "IN", 2) == 0)
		{
			this_action = ACTION_IN;
			line += 2; // Move past the "IN"
		}
		else if(strncmp(line, "OUT", 3) == 0)
		{	this_action = ACTION_OUT;
			line += 3; // Move past the "OUT"
		}
		else
			format_error("Line does not start with IN or OUT.");

		// TODO Check for the spaces here!
		if(strncmp(line, "   ", 3) == 0) // If the first three chars of 'line' are spaces...
		{
			if(this_action == ACTION_IN)
			{
				if(line[3] != ' ') // If action==in, the fourth char needs to be a space too...
					format_error("Incorrect number and placement of spaces.");
				line += 1; // Make up for the extra space if action==in
			}
		}
		else
		{
			format_error("Incorrect number and placement of spaces.");
		}
		line += 3; // Move past the spaces


		///////HERE WE ARE, COPY DATETIME///////
		strncpy(datetimestring, line, 10);
		datetimestring[10] = 0;
		line += 10; // Move past the date string


		// Check that the next two chars are spaces
		if(line[0] == ' ' && line[1] == ' ')
			line += 2; // Move past the spaces
		else
			format_error("Incorrect number and placement of spaces.");

		// Make sure the comment number is valid/within range
		if(strncmp(line, "    ", 4) == 0)
		{
			comment_number = -1;  // No comment number on this line...
		}
		else
		{
			char tmp = line[4]; // Save char at 4
			line[4] = 0; // Make the string only 4 chars long
			errno = 0;
			comment_number = strtol(line, NULL, 10); // TODO Verify that this works
			if(errno != 0)
				format_error("Comment number seems to be present, but is not a number.");
			line[4] = tmp; // Restore char at 4
		}

		// Move pointer past comment field
		line += 4;

		// Check that the next two chars are spaces
		if(line[0] == ' ' && line[1] == ' ')
			line += 2; // Move past the spaces
		else
			format_error("Incorrect number and placement of spaces.");

		////////////copy time string//////////////
		strncat(datetimestring, line, 5);
		datetimestring[15] = 0;

		/*******************************************************************************
		 * Well, here we are... parsing the date & time are the only things left now. *
		*******************************************************************************/

		struct tm *timestruct = calloc(1, sizeof(struct tm));
		if(timestruct == NULL)
			err("calloc");
		char *result = strptime(datetimestring, "%m/%d/%Y%H:%M", timestruct);
		if(result == NULL || *result != 0)
			format_error("Date or time is not in correct format.");
		this_time = mktime(timestruct);
		free(timestruct);

///////////////////////////////////// Logic section starts here, by the Emperor's Decree! ///////////////////////////////////

		// Check whether the current and last action are the same
		if(this_action == last_action)
			format_error("The last line was of the same type as this one.");
	

		// If the IN and the OUT are close together, warn.
		if(last_action == ACTION_OUT && labs(this_time - last_time) / 60 <= 10)
		{
			char *tmp = basicdate(&this_time);
			if(tmp == NULL)
				err("malloc");
			printf("%s:%d ***WARNING*** Difference between IN and OUT time is too small (%d minutes) on %s.\n", argv[optind], line_num, labs(this_time - last_time) / 60, tmp);
			free(tmp);
		}
		// Make sure the entries come after each other, time-wise.
		if(this_time < last_time)
			format_error("This line's date and time was not after the last line's.");

		// Entries must not be in the future
		if(this_time > now_time)
			format_error("This line's date and time are in the future.");

		if(this_time >= start_time || start_time == -1) // If this entry falls between start_time and stop_time, OR start_time is not set
		{
			if(this_time < stop_time + DAYS(1) || stop_time == -1) // If this_time is LESS than stop_time + 1 full day OR stop_time is not set
			{
				if(this_action == ACTION_OUT)   // If we are processing an OUT, calculate the time
				{
					time_t to_add = this_time - last_time; // Amount to add to the total
					if(to_add > HOURS(12)) // If amount to add is greater than 12 hours, warn.
					{
						char *tmp = basicdate(&this_time);
						if(tmp == NULL)
							err("malloc");
						printf("%s:%d: ***WARNING*** Employee clocked in for more than 12 hours (%.2f hours) on %s.\n", argv[optind], line_num, (double)to_add / 60 / 60, tmp);
						free(tmp);
					}
					total_time += this_time - last_time;
				}
			}
		}
	}
	if(quiet)
	{
		printf("%.2f\n", (double)total_time / 60 / 60);
	}
	else
	{
		printf("Total time: %.2f hours.\n", (double)total_time / 60 / 60);
	}
	return 0;
}

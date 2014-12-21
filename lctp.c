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
#include "lctp_procline.h"
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
	printf("      --no-warnings\tSuppress warnings\n");
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

int main(int argc, char *argv[])
{
	static struct option long_options[] =
	{
		{"start", required_argument, 0, 's'},
		{"end", required_argument, 0, 'e'},
		{"no-warnings", no_argument, 0, 1},
		{"quiet", no_argument, 0, 'q'},
		{"help", no_argument, 0, 'h'},
		{"version", no_argument, 0, 'v'}
	};
	int opti;
	char *sstart = NULL, *send = NULL;
	bool quiet = false, warnings = true;

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
			case 1:
				warnings = false;
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

	char *line = NULL, *orig_line = NULL;
	int comment_number = 0, line_num = 0;
	size_t len = 0;
	time_t total_time = 0, last_time = 0, now_time = time(NULL);
	enum actions last_action = ACTION_NIL;
	struct lctp_lineinfo l = {0};
	l.timesep = ':';
	l.datesep = '/';

	while(getline(&line, &len, fp) != -1)
	{

		// Increment the line number
		line_num++;

		// Save the unmodified pointer from getline() for passing to free()
		orig_line = line;

		// Put stuff from the last line into last_*
		last_action = l.action;
		last_time = l.time;

		// Zero out 'l'...necessary??
		l.action = ACTION_NIL;
		l.error = PLE_OK;
		l.time = 0;

		lctp_procline(&l, line);
		if(l.error != PLE_OK)
			format_error((char*)lctp_procline_strerror(l.error));

		// Now it is time to make sure the dates and times make sense

		// Check whether the current and last action are the same
		if(l.action == last_action)
			format_error("The last line was of the same type as this one.");
	

		// Make sure the entries come after each other, time-wise.
		if(l.time < last_time)
			format_error("This line's date and time was not after the last line's.");

		// Entries must not be in the future
		if(l.time > now_time)
			format_error("This line's date and time are in the future.");

		if(l.time >= start_time || start_time == -1) // If this entry falls between start_time and stop_time, OR start_time is not set
		{
			if(l.time < stop_time + DAYS(1) || stop_time == -1) // If l.time is LESS than stop_time + 1 full day OR stop_time is not set
			{
				// If the IN and the OUT are close together, warn.
				if(last_action == ACTION_OUT && labs(l.time - last_time) / 60 <= 10)
				{
					char *tmp = basicdate(&l.time);
					if(tmp == NULL)
						err("malloc");
					if(warnings)
						printf("%s:%d ***WARNING*** Difference between IN and OUT time is too small (%ld minutes) on %s.\n", argv[optind], line_num, labs(l.time - last_time) / 60, tmp);
					free(tmp);
				}

				if(l.action == ACTION_OUT)   // If we are processing an OUT, calculate the time
				{
					time_t to_add = l.time - last_time; // Amount to add to the total
					if(to_add > HOURS(12)) // If amount to add is greater than 12 hours, warn.
					{
						char *tmp = basicdate(&l.time);
						if(tmp == NULL)
							err("malloc");
						if(warnings)
							printf("%s:%d: ***WARNING*** Employee clocked in for more than 12 hours (%.2f hours) on %s.\n", argv[optind], line_num, (double)to_add / 60 / 60, tmp);
						free(tmp);
					}
					total_time += l.time - last_time;
				}
			}
		}
		line = orig_line; // Restore the pointer, for free() or for getline()
	}
	free(line);
	fclose(fp);
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

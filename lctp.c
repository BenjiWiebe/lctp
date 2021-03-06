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
#include "datetime.h"
#include "lctp.h"
#define LCTP_ATOL_onerror	formaterr(lineno);
#include "lctp_atol.h"
//#define LINELEN			30
//#define check(x)		if(!(x))formaterr(lineno)
#define format_error(x)	_format_error(x, argv[optind], line_num, __LINE__, true)
#define format_warning(x) _format_error(x, argv[optind], line_num, __LINE__, false)
#define apperr(...)	do{apperr_noexit(__VA_ARGS__);exit(1);}while(0)

void usage(char *progname, int ret)
{
	printf("Usage: %s [OPTIONS] FILE\n", progname);
	printf("Options:\n");
	printf("  -s, --start <date>\tStart date\n");
	printf("  -e, --end <date>\tEnd date\n");
	printf("  -q, --quiet\t\tDisplay only hours\n");
	printf("      --no-warnings\tSuppress warnings\n");
	printf("      --comments\tShow comment numbers\n");
	printf("  -h, --help\t\tPrint this help message and exit\n");
	printf("  -v, --version\t\tPrint version information and exit\n");
	exit(ret);
}

void apperr_noexit(char *fmt, ...)
{
	va_list l;
	va_start(l, fmt);
	vfprintf(stderr, fmt, l);
	va_end(l);
}

void err(char *msg)
{
	perror(msg);
	exit(1);
}

void _format_error(char *message, char *file, int lineno, int sourceline, bool iserror)
{
	apperr_noexit("%s:%d: Format %s: %s (%d)\n", file, lineno, iserror ? "error" : "warning", message, sourceline);
	if(iserror)
		exit(1);
}

int main(int argc, char *argv[])
{
	static struct option long_options[] =
	{
		{"start", required_argument, 0, 's'},
		{"end", required_argument, 0, 'e'},
		{"no-warnings", no_argument, 0, 1},
		{"comments", no_argument, 0, 2},
		{"quiet", no_argument, 0, 'q'},
		{"help", no_argument, 0, 'h'},
		{"version", no_argument, 0, 'v'}
	};
	int opti;
	char *sstart = NULL, *send = NULL;
	bool quiet = false, warnings = true, comments = false;

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
			case 2:
				comments = true;
				break;
			case 'q':
				quiet = true;
				break;
			case 'h':
				usage(argv[0], 0);
				break;
			case 'v':
				printf("%s\n", PACKAGE_STRING);
				exit(EXIT_SUCCESS);
				break;
		}
	}

	time_t start_time, stop_time;

	if(sstart)
	{
		start_time = parse_date(sstart, '-');

		if(start_time == FMT_ERR)
			apperr("Start time must be in format mm-dd-yyyy.\n");
	}
	else
	{
		start_time = -1;
	}

	if(send)
	{
		stop_time = parse_date(send, '-');
		if(stop_time == FMT_ERR)
			apperr("End date must be in format mm-dd-yyyy.\n");
	}
	else
	{
		stop_time = -1;
	}

	if((start_time != -1 && stop_time != -1) && start_time > stop_time) // If start and end dates are being used, make sure start comes before end chronologically
	{
		apperr("End date must be larger than or equal to the start date.\n");
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
	int line_num = 0;
	size_t len = 0;
	time_t total_time = 0, last_time = 0, now_time = time(NULL);
	enum actions last_action = ACTION_NIL;
	struct lctp_lineinfo l = {0};

	while(getline(&line, &len, fp) != -1)
	{

		// Increment the line number
		line_num++;

		// Put stuff from the last line into last_*
		last_action = l.action;
		last_time = l.time;

		if(lctp_procline(&l, line) < 0)
			format_error((char*)l.error_message);

		if(l.is_comment)
			continue;

		// Now it is time to make sure the dates and times make sense

		// Check whether the current and last action are the same
		if(l.action == last_action)
			format_warning("The last line was of the same type as this one.");

		// Make sure the entries come after each other, time-wise.
		if(l.time < last_time)
			format_error("This line's date and time was not after the last line's.");

		// Entries must not be in the future
		if(l.time > now_time)
			format_error("This line's date and time are in the future.");

		if(l.time < start_time) // Don't tally up this line if it comes before start_time. Do include it if it IS start_time.
			continue;

		// End-date is inclusive.
		// We need to compare with end-date plus a full day, exclusive, right?
		if(l.time > stop_time + DAYS(1) && stop_time != -1) // If l.time is LESS than stop_time + 1 full day OR stop_time is not set
			continue;

		char *tmp = basicdate(&l.time);
		if(tmp == NULL)
			err("malloc");
		// If the IN and the OUT are close together, warn.
		if(last_action == ACTION_OUT && labs(l.time - last_time) / 60 <= 10)
		{
			if(warnings)
				printf("%s:%d WARNING: Difference between IN and OUT time is too small (%ld minutes) on %s.\n", argv[optind], line_num, labs(l.time - last_time) / 60, tmp);
		}

		if(l.action == ACTION_OUT)   // If we are processing an OUT, calculate the time
		{
			time_t to_add = l.time - last_time; // Amount to add to the total
			if(to_add > HOURS(12)) // If amount to add is greater than 12 hours, warn.
			{
				if(warnings)
					printf("%s:%d: WARNING: Employee clocked in for more than 12 hours (%.2f hours) on %s.\n", argv[optind], line_num, (double)to_add / 60 / 60, tmp);
			}
			total_time += l.time - last_time;
		}

		// If there is a comment number, and we are supposed to show it, then show it
		if(l.commentno && comments)
		{
			if(quiet)
				printf("comment:%d ", l.commentno);
			else
				printf("%s:%d Comment number %d on %s.\n", argv[optind], line_num, l.commentno, tmp);
		}
		free(tmp);
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

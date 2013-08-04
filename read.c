#define _GNU_SOURCE
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

#define satio(str, max, min)	errno = 0; \
	str = strtol(s ## str, NULL, 10); \
	if(errno != 0) \
		formaterr(lineno); \
	if((str > max) || (str < min)) \
		formaterr(lineno)

#define LINELEN			30
#define check(x)		if(!(x))formaterr(lineno)
#define formaterr(x)	_formaterr(__FILE__,__LINE__,x)

char *basictime(time_t *t)
{
	char *ret = asctime(localtime(t));
	char *n = &ret[strlen(ret) - 1];
	if(*n == '\n')
		*n = 0;
	return ret;
}

void usage(char *progname, int ret)
{
	printf("Usage: %s [OPTIONS] FILE\n", progname);
	printf("Options:\n");
	printf("  -s, --start <date>\tStart date\n");
	printf("  -e, --end <date>\tEnd date\n");
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

void _formaterr(char *file, int line, int lineno)
{
	apperr("%s:%d: Invalid format on line %d.\n", file, line, lineno);
}

typedef enum
{
	NIL = 0,
	IN = 1,
	OUT = 2
} IO;

int main(int argc, char *argv[])
{
	static struct option long_options[] =
	{
		{"start", required_argument, 0, 's'},
		{"end", required_argument, 0, 'e'},
		{"help", no_argument, 0, 'h'},
		{"version", no_argument, 0, 'v'}
	};
	int c;
	int opti;
	char *sstart = NULL, *send = NULL;

	while(1)
	{
		c = getopt_long(argc, argv, "s:e:hv", long_options, &opti);
		if(c == -1)
		{
			break;
		}
		switch(c)
		{
			case 's':
				sstart = malloc(strlen(optarg) + 1);
				strcpy(sstart, optarg);
				break;
			case 'e':
				send = malloc(strlen(optarg) + 1);
				strcpy(send, optarg);
				break;
			case 'h':
				if(sstart)
				{
					free(sstart);
				}
				if(send)
				{
					free(send);
				}
				usage(argv[0], 0);
				break;
			case 'v':
				if(sstart)
				{
					free(sstart);
				}
				if(send)
				{
					free(send);
				}
				apperr("Not implemented.\n");
				break;
		}
	}

	if((sstart && (!send)) || (send && (!sstart)))
	{
		fprintf(stderr, "Arguments --end/-e and --start/-s must be used together.\n");
		if(sstart)
		{
			free(sstart);
		}
		if(send)
		{
			free(send);
		}
		usage(argv[0], 1);
	}

	time_t start, end;

	if(sstart)
	{
		start = parse_date(sstart, '-');
		end = parse_date(send, '-');

		free(sstart);
		free(send);

		if(start == FMT_ERR)
		{
			apperr("Arguments to --end/-e and --start/-s must be in the format mm-dd-yyyy.\n");
		}
		if(end == FMT_ERR)
		{
			apperr("Arguments to --end/-e and --start/-s must be in the format mm-dd-yyyy.\n");
		}
		if(start > end)
		{
			apperr("End date must be larger than or equal to the start date.");
		}
	}
	else
	{
		start = 0;
		end = parse_date("01-01-2500", '-'); /* 01 Jan 2500 00:00:00 GMT */
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
	size_t len = 0;

	if(fseek(fp, 0, SEEK_SET) < 0)
	{
		err("fseek");
	}

	char *line = NULL;
	int cmnt = 0;
	int lineno = 0;
	time_t utm;
	int linei = 0;
	char sio[4], scmnt[5], sdatetime[16];
	time_t last_time = 0;
	float totaltime = 0.0;
	IO iocur = 0, iolast = 0;

	while(getline(&line, &len, fp) != -1)
	{
		lineno++;

		linei = strlen(line) - 1;
		if(linei != 29)
		{
			formaterr(lineno);
		}

		while(linei--)
		{
			switch(linei)
			{
				case 0:
				case 1:
				case 2:
					sio[linei] = line[linei];
					break;
				case 3:
				case 4:
				case 5:
					check(line[linei] == ' ');
					break;
				case 6:
				case 7:
					sdatetime[linei - 6] = line[linei];
					break;
				case 8:
				case 11:
					check(line[linei] == '/');
					break;
				case 9:
				case 10:
					sdatetime[linei - 9 + 3] = line[linei];
					break;
				case 12:
				case 13:
				case 14:
				case 15:
					sdatetime[linei - 12 + 6] = line[linei];
					break;
				case 16:
				case 17:
				case 22:
				case 23:
					check(line[linei] == ' ');
					break;
				case 18:
				case 19:
				case 20:
				case 21:
					scmnt[linei - 18] = line[linei];
					break;
				case 24:
				case 25:
					sdatetime[linei - 24 + 10] = line[linei];
					break;
				case 26:
					check(line[linei] == ':');
					break;
				case 27:
				case 28:
					sdatetime[linei - 27 + 13] = line[linei];
					break;
				case 29:
					check(line[linei] == '\n');
					break;
				default:
					apperr("***ERROR*** Unknown error.\n");
			}
		}

		sio[3] = 0;
		scmnt[4] = 0;

		// make sure that sio is IN or OUT and set iocur accordingly

		if(sio[2] == ' ')
		{
			sio[2] = 0;
		}
		if(strcmp(sio, "IN") == 0)
		{
			iocur = IN;
		}
		else if(strcmp(sio, "OUT") == 0)
		{
			iocur = OUT;
		}
		else
		{
			formaterr(lineno);
		}

		// an IN must not follow an IN, vice versa for OUT
		if(iocur == iolast)
		{
			formaterr(lineno);
		}

		// make sure the comment number is valid/within range
		if(strcmp(scmnt, "    ") != 0)
		{
			satio(cmnt, 9999, 1000);
		}

		// normalize and then parse sdatetime
		sdatetime[2] = sdatetime[5] = '-';
		sdatetime[12] = ':';
		sdatetime[15] = 0;

		utm = parse_datetime(sdatetime, '-', ':');

		// make sure the entries are in the right order
		if(utm < last_time)
			formaterr(lineno);

		time_t to_add;
		if(utm >= start && utm <= (end + (24 * 60 * 60 - 1)))
		{
			if(iocur == OUT)   // if we are processing an OUT, calculate the time
			{
				to_add = utm - last_time;
				if(to_add > (12 * 60 * 60))
				{
					printf("%s:%d: ***WARNING*** Employee logged in for more than 12 hours (%.2f hours): IN at '%s', OUT at '%s'.\n", argv[optind], lineno, ((float)to_add) / 60 / 60, basictime(&last_time), basictime(&utm));
					printf("to_add = %ju\n",to_add);
				}
				totaltime += ((float)to_add) / 60 / 60;
			}

			if(iocur == IN)
			{
				last_time = utm;
			}
		}

		iolast = iocur;
	}
	printf("Total time: %.2f hours.\n", totaltime);

	free(line);
	return 0;
}

#ifndef __LCTP_PROCLINE_H__
#define __LCTP_PROCLINE_H__
#include <time.h>
#define LCTP_DATA_LINE_LEN	31
enum actions { ACTION_NIL = 0, ACTION_IN, ACTION_OUT };
struct lctp_lineinfo {
	time_t time;
	enum actions action;
	int commentno;
	const char *error_message;
	int is_comment;
};
int lctp_procline(struct lctp_lineinfo*, char*);
time_t parse_date(char *str, char sep);
time_t parse_datetime(char *str, char dsep, char tsep);
char *lctp_getstatus(char*);
#endif

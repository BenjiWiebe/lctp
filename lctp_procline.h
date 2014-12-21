#ifndef __LCTP_PROCLINE_H__
#define __LCTP_PROCLINE_H__
enum lctp_procline_errors { PLE_OK = 0, PLE_LEN, PLE_ARGS, PLE_IO, PLE_TIMESEP, PLE_DATESEP, PLE_SPACES, PLE_NEWLINE }; 
enum actions { ACTION_NIL = 0, ACTION_IN, ACTION_OUT };
struct lctp_lineinfo {
	time_t time;
	enum actions action;
	char datesep;
	char timesep;
	enum lctp_procline_errors error;
};
const char *lctp_procline_strerror(enum lctp_procline_errors);
int lctp_procline(struct lctp_lineinfo*, char*);
#endif

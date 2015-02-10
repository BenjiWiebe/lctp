#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void fatalerror(char *s)
{
	perror(s);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{

	if(argc != 2)
	{
		printf("Usage: %s <timefile>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	time_t now = time(NULL);
	struct tm *bdt = localtime(&now);
	bdt->tm_sec = 0;
	bdt->tm_min = 0;
	bdt->tm_hour = 0;
	bdt->tm_isdst = -1;
	time_t ttoday = mktime(bdt);
	struct tm *today = localtime(&ttoday);
	today->tm_isdst = -1;
	int days_ago = 0;
	while(today->tm_wday != 4)
	{
		if(!today->tm_wday)
			today->tm_wday = 6;
		else
			today->tm_wday--;
		days_ago++;
	}
	days_ago++;
	size_t filename_len = strlen(argv[1]);
	char *begin_arg = malloc(11), *end_arg = malloc(11), *command = malloc(39+filename_len);
	if(!(begin_arg && end_arg))
		fatalerror("malloc");
	time_t end = mktime(today) - days_ago * 24 * 60 * 60;
	time_t begin = end - (6 * 24 * 60 * 60); //FIXME (in lctp) Use 6 instead of 7 since lctp is INCLUSIVE with the end date
	strftime(begin_arg, 11, "%m-%d-%Y", localtime(&begin));
	strftime(end_arg, 11, "%m-%d-%Y", localtime(&end));
	snprintf(command, 39+filename_len, "lctp -q -s %s -e %s %s", begin_arg, end_arg, argv[1]);
	char line[128];
	FILE *pipe = popen(command, "r");
	if(pipe == NULL)
		fatalerror("popen");
	while(fgets(line, 128, pipe))
		printf("%s", line);
	pclose(pipe);
	free(begin_arg);
	free(end_arg);
	free(command);
	return 0;
}

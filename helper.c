#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
void err(char *msg)
{
	fprintf(stderr, "%s: %s\n", msg, strerror(errno));
	exit(EXIT_FAILURE);
}
int main(int argc, char *argv[])
{
	if(argc!=2)
	{
		printf("Usage: %s <timefile>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	char *prog;
	time_t t = time(NULL);
	char *sbuf, *ebuf;
	sbuf = malloc(11);
	ebuf = malloc(11);
	if(sbuf == NULL || ebuf == NULL)err("malloc");
	strftime(sbuf, 11, "%m-%d-%Y", localtime(&t));
	t -= 60 * 60 * 24 * 7;
	strftime(ebuf, 11, "%m-%d-%Y", localtime(&t));
	int s = strlen(argv[1]) + 40;
	prog = malloc(s);
	if(prog == NULL)err("malloc");
	snprintf(prog, s-1, "" 
						#ifndef _WIN32
						"./"
						#endif
						   "lctp -s %10s -e %10s %s", ebuf, sbuf, argv[1]);
	FILE *p = popen(prog, "r");
	if(p == NULL)err("malloc");
	char *line = NULL;
	size_t len = 0;
	while(getline(&line, &len, p) != -1)
	{
		printf("%s",line);
	}
	pclose(p);
	free(sbuf);
	free(ebuf);
	return 0;
}

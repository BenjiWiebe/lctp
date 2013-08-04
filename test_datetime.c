#include <stdio.h>
#include "datetime.h"
int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("Usage: %s <mm/dd/yyyyhh:mm>\n", argv[0]);
		return 1;
	}
	time_t t = parse_datetime(argv[1], '/', ':');
	if(t == FMT_ERR)
		printf("Error.\n");
	else
		printf("%ju\n", t);
	return 0;
}

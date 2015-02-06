#include <stdio.h>
#include <string.h>
#include <lctp.h>
int main()
{
	char line[] = {"IN    08/24/2014        07:30"};
	if(strcmp(lctp_getstatus(line), "Last action: IN @ 07:30 AM 08/24"))
		return 1;
	return 0;
}

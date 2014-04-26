#include <string.h>
#include "chomp.h"
char *chomp(char *buf)
{
    char *c = buf + strlen(buf) - 1;
    if(*c == '\n')
        *c = 0;
    c--;
    if(*c == '\r')
        *c = 0;
    return buf;
}

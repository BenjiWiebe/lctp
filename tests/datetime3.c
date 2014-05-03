#include "datetime.h"
int main()
{
	return !(parse_datetime("01-01-197000:00", '-', ':') == 0);
}

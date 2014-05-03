#include "datetime.h"
int main()
{
	return !(parse_datetime("05-01-201422:05", '-', ':') == 1398981900);
}

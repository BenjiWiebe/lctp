#include "datetime.h"
int main()
{
	return !(parse_datetime("12/31/500024.59", '/', '.') == 95649123540);
}

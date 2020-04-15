#include <time.h>
#include <math.h>

#define SUNDAY		0
#define MONDAY		1
#define THURSDAY	4

enum holidays { NEWYEARS, EASTER, MEMORIALDAY, INDEPENDENCEDAY, LABORDAY, THANKSGIVING, CHRISTMAS };
struct holiday_factor {
	enum holidays holiday;
	int percent; // 200 = 200%, double pay. 150 for time-and-a-half.
};


// New Year's = Jan 1
// Easter = calculate_easter()
// Memorial Day = Last Monday of May
// Independence Day = July 4
// Labor Day = 1st Monday in September
// Thanksgiving = 4th Thursday in November
// Christmas = Dec 25

// Returns 100 for 100%, see holiday_factor.percent
// TODO Needs to indicate what holiday it was
int get_holiday_factor(struct tm date, struct holiday_factor *factors)
{
	struct tm *t;
	int this_year = date.tm_year + 1900;
	for(int i = 0; factors[i] != NULL; i++)
	{
		switch(factors[i].holiday) // Check which holiday is specified, and get the rate for it, if the date matches.
		{
			case NEWYEARS:
				if(date.tm_mon == 0 && date.tm_mday == 1)
					return factors[i].percent;
				break;
			case EASTER:
				t = calculate_easter(this_year);
				if(date.tm_mon == t->tm_mon && date.tm_mday == t->tm_mday)
					return factors[i].percent;
				break;
			case MEMORIALDAY:
				t = calculate_memorial(this_year);
				if(date.tm_mon == t->tm_mon && date.tm_mday == t->tm_mday)
					return factors[i].percent;
				break;
			case INDEPENDENCEDAY:
				if(date.tm_mon == 6 && date.tm_mday == 4)
					return factors[i].percent;
				break;
			case LABORDAY:
				t = calculate_labor(this_year);
				if(date.tm_mon == t->tm_mon && date.tm_mday == t->tm_mday)
					return factors[i].percent;
				break;
			case THANKSGIVING:
				t = calculate_thanksgiving(this_year);
				if(date.tm_mon == t->tm_mon && date.tm_mday == t->tm_mday)
					return factors[i].percent;
				break;
			case CHRISTMAS:
				if(date.tm_mon == 11 && date.tm_mday == 25)
					return factors[i].percent;
				break;
		}
	}
	// Must not be a holiday or not one that was listed in *factors
	return 100; // Pay 100%. No bonus today!
}

struct tm *calculate_easter(int year)
{
	static struct tm cached_date;
	if(cached_date.tm_year + 1900 == year)
		return &cached_date;
	int a = year % 19;
	int b = floor(year / 100);
	int c = year % 100;
	int d = floor(b / 4);
	int e = b % 4;
	int f = floor((b + 8) / 25);
	int g = floor((b - f + 1) / 3);
	int h = (19 * a + b - d - g + 15) % 30;
	int i = floor(c / 4);
	int k = c % 4;
	int l = (32 + 2 * e + 2 * i - h - k) % 7;
	int m = floor((a + 11 * h + 22 * l) / 451);
	int month = floor((h + l - 7 * m + 114) / 31);
	int day = ((h + l - 7 * m + 114) % 31) + 1;
	
	cached_date.tm_year = year - 1900; // tm_year is years since 1900.
	cached_date.tm_mon = month - 1; // tm_mon starts at 0. This algorithm gives month as an ordinal number, i.e. 1-12.
	cached_date.tm_mday = day; // tm_mday is ordinal, like our algorithm's day.

	return &cached_date;
}

struct tm *calculate_memorial(int year)
{
	static struct tm cached_date;
	if(cached_date.tm_year + 1900 == year)
		return &cached_date;

	struct tm d = {0};
	d.tm_mday = 31; // Last day...
	d.tm_mon = 4; // ..of May.
	d.tm_year = year - 1900;
	mktime(&d); // Populat tm_wday
	if(d.tm_wday == SUNDAY)
		d.tm_mday -= 6;
	else
		d.tm_mday -= d.tm_wday - 1;	
	
	cached_date.tm_year = year - 1900;
	cached_date.tm_mon = d.tm_mon;
	cached_date.tm_mday = d.tm_mday;

	return &cached_date;
}

struct tm *calculate_labor(int year)
{
	static struct tm cached_date;
	if(cached_date.tm_year + 1900 == year)
		return &cached_date;

	struct tm d = {0};
	d.tm_mday = 1; // The 1st of...
	d.tm_mon = 8; // September
	d.tm_year = year - 1900;
	mktime(&d); // Populate tm_wday
	if(d.tm_wday == SUNDAY)
		d.tm_mday += 1; // Make it Monday
	else if(d.tm_wday != MONDAY)
		d.tm_mday += 8 - d.tm_wday; // Wrap around a whole week and some to make it the next Monday.

	cached_date.tm_year = year - 1900;
	cached_date.tm_mon = d.tm_mon;
	cached_date.tm_mday = d.tm_mday;

	return &cached_date;
}

struct tm *calculate_thanksgiving(int year)
{
	static struct tm cached_date;
	if(cached_date.tm_year + 1900 == year)
		return &cached_date;

	struct tm d = {0};
	d.tm_mday = 1; // The 1st of...
	d.tm_mon = 10; // November
	d.tm_year = year - 1900;
	mktime(&d); // Populate tm_wday
	if(d.tm_wday <= THURSDAY)
		d.tm_mday += 25 - d.tm_wday;
	else
		d.tm_mday += 32 - d.tm_wday;

	cached_date.tm_year = year - 1900;
	cached_date.tm_mon = d.tm_mon;
	cached_date.tm_mday = d.tm_mday;

	return &cached_date;
}

#include <ep/ep.h>
#include <ep/ep_time.h>

#define _M	* 60		// minutes
#define _H	* 60 _M		// hours
#define _D	* 24 _H		// days

#define T(d, h, m, s)		d _D + h _H + m _M + s

#define TRY1(s, u)		try(0, 0, s, u * 1000)
#define TRY2(s1, u1, s2, u2)	try(s1, u1 * 1000, s2, u2 * 1000)

void
try(int64_t startsec, int32_t startnsec, int64_t endsec, int32_t endnsec)
{
	char tbuf[40];

	EP_TIME_SPEC start = { startsec, startnsec };
	EP_TIME_SPEC end = { endsec, endnsec };
	ep_time_format_interval(&start, &end, tbuf, sizeof tbuf);
	printf("%s\n", tbuf);
}

int
main(int argc, char **argv)
{
	TRY1(0, 0);
	TRY1(0, 1);
	TRY1(1, 2);
	TRY1(T(0, 0, 1, 0), 3);
	TRY1(T(0, 1, 0, 0), 4);
	TRY1(T(1, 0, 1, 0), 5);

	TRY2(T(365, 0, 0, 0), 100, T(365, 1, 2, 3), 200);
	TRY2(T(365, 0, 0, 0), 100, T(366, 1, 2, 3), 300);

	return 0;
}

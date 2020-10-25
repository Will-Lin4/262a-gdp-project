/* vim: set ai sw=4 sts=4 ts=4 :*/

/*
**  LLOGGER --- like logger(1), but to a local file
**
**  	Also allows for log rotation.
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#include <sys/errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>


const char		*OpenMode = "w";

void
reopen(const char *fname, FILE **fpp, ino_t *inop)
{
	FILE *fp = *fpp;
	struct stat st;

	if (fp != NULL)
		fclose(fp);

	fp = fopen(fname, OpenMode);
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot open output file \"%s\": %s\n",
				fname, strerror(errno));
		exit(EX_CANTCREAT);
	}
	setlinebuf(fp);
	*inop = -1;
	if (fstat(fileno(fp), &st) == 0)
		*inop = st.st_ino;
	*fpp = fp;
}

int
main(int argc, char **argv)
{
	int opt;
	char in_buf[10240];
	FILE *in_fp;
	FILE *out_fp;
	const char *out_fname;
	ino_t prev_ino = -1;
	bool tee = false;
	int sigfigs = 6;
	bool utc = false;

	while ((opt = getopt(argc, argv, "as:tu")) > 0)
	{
		switch (opt)
		{
		case 'a':
			OpenMode = "a";
			break;

		case 's':
			sigfigs = atoi(optarg);
			break;

		case 't':
			tee = true;
			break;

		case 'u':
			utc = true;
			break;
		}
	}

	argc -= optind;
	argv += optind;

	if (sigfigs < 0)
		sigfigs = 0;
	else if (sigfigs > 6)
		sigfigs = 6;

	if (argc < 1)
	{
		out_fname = NULL;
		out_fp = stdout;
		tee = false;
	}
	else
	{
		out_fname = argv[0];
		out_fp = NULL;
		reopen(out_fname, &out_fp, &prev_ino);
		argc--;
		argv++;
	}

	in_fp = stdin;

	while (fgets(in_buf, sizeof in_buf, in_fp) != NULL)
	{
		struct stat st;
		struct timeval tv;
		struct tm *tm;

		// if the file has been rotated, open the new version
		if (out_fname != NULL)
		{
			if (stat(out_fname, &st) != 0 || st.st_ino != prev_ino)
			{
				reopen(out_fname, &out_fp, &prev_ino);
			}
		}

		gettimeofday(&tv, NULL);
		if (utc)
			tm = gmtime(&tv.tv_sec);
		else
			tm = localtime(&tv.tv_sec);
		long usec = tv.tv_usec;
		char fractional_seconds[10];

		if (sigfigs <= 0)
			fractional_seconds[0] = '\0';
		else
			snprintf(fractional_seconds, sizeof fractional_seconds,
					".%06ld", usec);

		fprintf(out_fp, "%04d-%02d-%02d %02d:%02d:%02d%.*s %s",
				tm->tm_year + 1900,
				tm->tm_mon + 1,
				tm->tm_mday,
				tm->tm_hour,
				tm->tm_min,
				tm->tm_sec,
				sigfigs + 1,
				fractional_seconds,
				in_buf);

		if (tee)
			printf("%04d-%02d-%02d %02d:%02d:%02d%.*s %s",
					tm->tm_year + 1900,
					tm->tm_mon + 1,
					tm->tm_mday,
					tm->tm_hour,
					tm->tm_min,
					tm->tm_sec,
					sigfigs + 1,
					fractional_seconds,
					in_buf);
	}
}

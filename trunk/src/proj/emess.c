/* Error message processing */
#ifndef lint
static const char SCCSID[]="@(#)emess.c	4.6	94/05/24	GIE	REL";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <proj_api.h>
#define EMESS_ROUTINE
#include "emess.h"
	void
emess(int code, char *fmt, ...) {
	va_list args;

	va_start(args, fmt);
	/* prefix program name, if given */
	if (fmt != NULL)
		(void)fprintf(stderr,"%s\n<%s>: ",pj_release,emess_dat.Prog_name);
	/* print file name and line, if given */
	if (emess_dat.File_name != NULL && *emess_dat.File_name) {
		(void)fprintf(stderr,"while processing file: %s", emess_dat.File_name);
		if (emess_dat.File_line > 0)
			(void)fprintf(stderr,", line %d\n", emess_dat.File_line);
		else
			(void)fputc('\n', stderr);
	} else
		putc('\n', stderr);
	/* if |code|==2, print errno code data */
	if (code == 2 || code == -2)
		(void)fprintf(stderr, "Sys errno: %d: %s\n",
			errno,
#ifdef HAVE_STRERROR
			strerror(errno));
#else
			"<system mess. texts unavail.>");
#endif
	/* post remainder of call data */
	(void)vfprintf(stderr,fmt,args);
	va_end(args);
	/* die if code positive */
	if (code > 0) {
		(void)fputs("\nprogram abnormally terminated\n", stderr);
		exit(code);
	}
	else
		putc('\n', stderr);
}

/*--------------------------------------------------------------------
 *    The MB-system:  mb_xmutil.c
 *
 *    Copyright (c) 2026 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <Xm/Xm.h>
#include <Xm/Text.h>

#include "mb_xmutil.h"

void mb_get_text_string(Widget w, String str, size_t len) {
	if (len == 0)
		return;

	char *str_tmp = (char *)XmTextGetString(w);
	if (str_tmp == NULL) {
		str[0] = '\0';
		return;
	}

	strncpy(str, str_tmp, len - 1);
	str[len - 1] = '\0';
	XtFree(str_tmp);
}

void mb_file_open_error_message(const char *path, char *line1, char *line2, char *line3, size_t linesize) {
	if (linesize == 0)
		return;

	snprintf(line1, linesize, "Unable to open input file:");
	snprintf(line2, linesize, "%s", path);

	/* Diagnose the actual cause with stat()/access() rather than asserting
	 * one fixed explanation - mb_read_init() does not preserve errno from
	 * its failed fopen() through to this point, and a generic "file may not
	 * exist or you may not have read permission" message is shown for every
	 * failure including an overly long path (errno ENAMETOOLONG). */
	struct stat file_status;
	if (stat(path, &file_status) != 0) {
		const int stat_errno = errno;
		if (stat_errno == ENOENT)
			snprintf(line3, linesize, "File does not exist.");
		else if (stat_errno == ENAMETOOLONG)
			snprintf(line3, linesize, "Path is too long (%zu characters).", strlen(path));
		else if (stat_errno == EACCES)
			snprintf(line3, linesize, "Permission denied.");
		else if (stat_errno == ENOTDIR)
			snprintf(line3, linesize, "A component of the path is not a directory.");
		else
			snprintf(line3, linesize, "%s", strerror(stat_errno));
	}
	else if (access(path, R_OK) != 0) {
		snprintf(line3, linesize, "Permission denied.");
	}
	else {
		snprintf(line3, linesize, "File exists but could not be read (wrong format?).");
	}
}

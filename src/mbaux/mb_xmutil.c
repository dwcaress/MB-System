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

#include <string.h>
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

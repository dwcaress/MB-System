/*--------------------------------------------------------------------
 *    The MB-system:	mbformat.c	1/22/93
 *
 *    Copyright (c) 1993-2025 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBFORMAT provides a description of the swath data format
 * associated with a particular MBIO format identifier.  If
 * no format is specified, MBFORMAT will list descriptions of all
 * the currently supported formats.
 *
 * Author:	D. W. Caress
 * Date:	January 22, 1993
 *
 * GMT-module rewrite of mbformat.cc: wrapped as GMT_mbformat entry
 * so it can be invoked from the GMT API (Julia FFI / Matlab MEX).
 * Single-letter options (-F, -I, -L, -K, -V, -W, -H) map directly to
 * GMT_OPTION entries — no getopt_long lookup table required.
 */

#define THIS_MODULE_NAME		"mbformat"
#define THIS_MODULE_LIB			"mbsystem"
#define THIS_MODULE_PURPOSE		"Describe MBIO swath data formats (id, name, attributes) or list all supported formats"
#define THIS_MODULE_KEYS		">D}"
#define THIS_MODULE_NEEDS		""
#define THIS_MODULE_OPTIONS		"-:>Vh"

#include "gmt_dev.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_status.h"

enum MbformatList {
	MBFORMAT_LIST_LONG   = 0,
	MBFORMAT_LIST_SIMPLE = 1,
	MBFORMAT_LIST_ROOT   = 2
};

static const char help_message[] =
    "MBFORMAT is an utility which identifies the swath data formats\n"
    "associated with MBIO format id's.  If no format id is specified,\n"
    "MBFORMAT lists all of the currently supported formats.";
static const char usage_message[] = "mbformat [-Fformat -Ifile -L -K -V -W -H]";

/* --- Control structure ---------------------------------------------- */

struct MBFORMAT_CTRL {
	struct mbf_F { bool active; int  format; } F;
	struct mbf_H { bool active; } H;
	struct mbf_I { bool active; char file[MB_PATH_MAXLINE]; } I;
	struct mbf_L { bool active; } L;
	struct mbf_K { bool active; } K;
	struct mbf_W { bool active; } W;
};

static void *New_mbformat_Ctrl(struct GMT_CTRL *GMT) {
	struct MBFORMAT_CTRL *Ctrl = gmt_M_memory(GMT, NULL, 1, struct MBFORMAT_CTRL);
	return Ctrl;
}

static void Free_mbformat_Ctrl(struct GMT_CTRL *GMT, struct MBFORMAT_CTRL *Ctrl) {
	if (!Ctrl) return;
	gmt_M_free(GMT, Ctrl);
}

static int usage(struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return GMT_NOERROR;
	GMT_Message(API, GMT_TIME_NONE, "usage: %s\n\n", usage_message);
	if (level == GMT_SYNOPSIS) return GMT_PARSE_ERROR;
	GMT_Message(API, GMT_TIME_NONE, "\n%s\n\n", help_message);
	GMT_Message(API, GMT_TIME_NONE,
		"\t-F MBIO format id to describe.\n"
		"\t-I Swath data file; format is inferred from the filename.\n"
		"\t-L Simple listing: print the format id(s) only.\n"
		"\t-K Root listing: print '<root> <format>' (filename root + format id).\n"
		"\t-W Emit HTML page describing all supported formats.\n"
		"\t-H Print description and exit.\n");
	GMT_Option(API, "V,:");
	return GMT_PARSE_ERROR;
}

static int parse(struct GMT_CTRL *GMT, struct MBFORMAT_CTRL *Ctrl, struct GMT_OPTION *options) {
	unsigned int n_errors = 0;
	struct GMT_OPTION *opt;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {
		case 'F':
			if (opt->arg && opt->arg[0]) {
				if (sscanf(opt->arg, "%d", &Ctrl->F.format) == 1)
					Ctrl->F.active = true;
				else {
					GMT_Report(API, GMT_MSG_NORMAL, "Syntax -F option: expected integer format id\n");
					n_errors++;
				}
			} else n_errors++;
			break;
		case 'H':
			Ctrl->H.active = true;
			break;
		case 'I':
			if (opt->arg && opt->arg[0]) {
				strncpy(Ctrl->I.file, opt->arg, MB_PATH_MAXLINE - 1);
				Ctrl->I.file[MB_PATH_MAXLINE - 1] = '\0';
				Ctrl->I.active = true;
			} else n_errors++;
			break;
		case 'L':
			Ctrl->L.active = true;
			break;
		case 'K':
			Ctrl->K.active = true;
			break;
		case 'W':
			Ctrl->W.active = true;
			break;
		default:
			n_errors += gmt_default_error(GMT, opt->option);
			break;
		}
	}

	return n_errors ? GMT_PARSE_ERROR : GMT_OK;
}

#define bailout(code)  { gmt_M_free_options(mode); return code; }
#define Return(code)   { Free_mbformat_Ctrl(GMT, Ctrl); gmt_end_module(GMT, GMT_cpy); bailout(code); }

/*--------------------------------------------------------------------*/
int GMT_mbformat(void *V_API, int mode, void *args) {
	int	error = MB_ERROR_NO_ERROR;

	struct MBFORMAT_CTRL *Ctrl = NULL;
	struct GMT_CTRL      *GMT  = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION    *options = NULL;
	struct GMTAPI_CTRL   *API = gmt_get_api_ptr(V_API);

	if (API == NULL) return GMT_NOT_A_SESSION;
	if (mode == GMT_MODULE_PURPOSE) return usage(API, GMT_MODULE_PURPOSE);
	options = GMT_Create_Options(API, mode, args);
	if (API->error) return API->error;
	if (!options || options->option == GMT_OPT_USAGE)   bailout(usage(API, GMT_USAGE));
	if (options->option == GMT_OPT_SYNOPSIS)            bailout(usage(API, GMT_SYNOPSIS));

#if GMT_MAJOR_VERSION >= 6
	if ((GMT = gmt_init_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout(API->error);
#else
	GMT = gmt_begin_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy);
#endif
	if (GMT_Parse_Common(API, THIS_MODULE_OPTIONS, options)) Return(API->error);

	Ctrl = New_mbformat_Ctrl(GMT);
	if ((error = parse(GMT, Ctrl, options))) Return (error);

	int verbose = GMT->common.V.active;

	const bool file_specified   = Ctrl->I.active;
	const bool format_specified = Ctrl->F.active;
	const bool help             = Ctrl->H.active;
	const bool html             = Ctrl->W.active;
	int format = format_specified ? Ctrl->F.format : 0;
	char file[MB_PATH_MAXLINE];
	strncpy(file, Ctrl->I.file, MB_PATH_MAXLINE);

	enum MbformatList list_mode = MBFORMAT_LIST_LONG;
	if (Ctrl->L.active) list_mode = MBFORMAT_LIST_SIMPLE;
	if (Ctrl->K.active) list_mode = MBFORMAT_LIST_ROOT;

	if (verbose == 1 || help) {
		fprintf(stderr, "\nProgram %s\n", THIS_MODULE_NAME);
		fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
	}

	if (verbose >= 2) {
		GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Program <%s>\n", THIS_MODULE_NAME);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  MB-system Version %s\n", MB_VERSION);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  Control Parameters:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       verbose: %d\n", verbose);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       help:    %d\n", (int)help);
		if (format_specified)
			GMT_Report(API, GMT_MSG_NORMAL, "dbg2       format:  %d\n", format);
		if (file_specified)
			GMT_Report(API, GMT_MSG_NORMAL, "dbg2       file:    %s\n", file);
	}

	if (help) {
		fprintf(stderr, "\n%s\n", help_message);
		fprintf(stderr, "\nusage: %s\n", usage_message);
		Return(MB_ERROR_NO_ERROR);
	}

	int status = MB_SUCCESS;

	/* print out the info */
	int format_save = format;
	char root[MB_PATH_MAXLINE];
	if (file_specified) {
		status = mb_get_format(verbose, file, root, &format, &error);
	}
	else if (format_specified) {
		status = mb_format(verbose, &format, &error);
	}

	if (file_specified && format == 0) {
		if (list_mode == MBFORMAT_LIST_SIMPLE)
			printf("%d\n", format);
		else if (list_mode == MBFORMAT_LIST_ROOT)
			printf("%s %d\n", root, format);
		else
			printf("Program %s unable to infer format from filename %s\n", THIS_MODULE_NAME, file);
	}
	else if (format_specified && format == 0) {
		if (list_mode == MBFORMAT_LIST_SIMPLE)
			printf("%d\n", format);
		else if (list_mode == MBFORMAT_LIST_ROOT)
			printf("%s %d\n", root, format);
		else
			printf("Specified format %d invalid for MB-System\n", format_save);
	}
	else if (format != 0) {
		if (list_mode == MBFORMAT_LIST_SIMPLE) {
			printf("%d\n", format);
		}
		else if (list_mode == MBFORMAT_LIST_ROOT) {
			printf("%s %d\n", root, format);
		}
		else {
			char format_description[MB_DESCRIPTION_LENGTH];
			status = mb_format_description(verbose, &format, format_description, &error);
			if (status == MB_SUCCESS) {
				printf("\nMBIO data format id: %d\n", format);
				printf("%s", format_description);
			}
			else if (file_specified) {
				printf("Program %s unable to infer format from filename %s\n", THIS_MODULE_NAME, file);
			}
			else if (format_specified) {
				printf("Specified format %d invalid for MB-System\n", format_save);
			}
		}
	}
	else if (html) {
		printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\">\n");
		printf("<HTML>\n<HEAD>\n   <TITLE>MB-System Supported Data Formats</TITLE>\n");
		printf("</HEAD>\n<BODY TEXT=\"#000000\" BGCOLOR=\"#FFFFFF\" LINK=\"#336699\" VLINK=\"#997040\" ALINK=\"#CC9900\">\n\n");
		printf("<CENTER><P><B><FONT SIZE=+2>MB-System Supported Swath Data Formats</FONT></B></P></CENTER>\n\n");
		printf("<P>Each swath mapping sonar system outputs a data stream which includes\n");
		printf("some values or parameters unique to that system. In general, a number of\n");
		printf("different data formats have come into use for data from each of the sonar\n");
		printf("systems; many of these formats include only a subset of the original data\n");
		printf("stream. Internally, MBIO recognizes which sonar system each data format\n");
		printf("is associated with and uses a data structure including the complete data\n");
		printf("stream for that sonar. At present, formats associated with the following\n");
		printf("sonars are supported: </P>\n\n");
		printf("<UL>\n<LI>Sea Beam &quot;classic&quot; multibeam sonar </LI>\n\n");
		printf("<LI>Hydrosweep DS multibeam sonar </LI>\n\n");
		printf("<LI>Hydrosweep DS2 multibeam sonar </LI>\n\n");
		printf("<LI>Hydrosweep MD multibeam sonar </LI>\n\n");
		printf("<LI>Sea Beam 2000 multibeam sonar </LI>\n\n");
		printf("<LI>Sea Beam 2112 and 2136 multibeam sonars </LI>\n\n");
		printf("<LI>Sea Beam 2120 multibeam sonars </LI>\n\n");
		printf("<LI>Simrad EM12, EM121, EM950, and EM1000 multibeam sonars </LI>\n\n");
		printf("<LI>Simrad EM120, EM300, and EM3000 multibeam sonars</LI>\n\n");
		printf("<LI>Simrad EM122, EM302, EM710, and EM3002 multibeam sonars</LI>\n\n");
		printf("<LI>Simrad Mesotech SM2000 multibeam sonar</LI>\n\n");
		printf("<LI>Hawaii MR-1 shallow tow interferometric sonar </LI>\n\n");
		printf("<LI>ELAC Bottomchart and Bottomchart MkII shallow water multibeam sonars</LI>\n\n");
		printf("<LI>Reson Seabat multibeam sonars (e.g. 9001, 8081, 7125)</LI>\n\n");
		printf("<LI>WHOI DSL AMS-120 deep tow interferometric sonar </LI>\n\n");
		printf("<LI>Sea Scan sidescan sonar</LI>\n\n");
		printf("<LI>Furuno HS-1 multibeam sonar</LI>\n\n");
		printf("<LI>Edgetech sidescan and subbottom profiler sonars</LI>\n\n");
		printf("<LI>Imagenex DeltaT multibeam sonars</LI>\n\n");
		printf("<LI>Odom ES3 multibeam sonar</LI>\n\n");
		printf("</UL>\n\n");
		printf("<P>The following swath mapping sonar data formats are currently supported by MB-System:</P>\n\n");

		for (int i = 0; i <= 1000; i++) {
			format = i;
			char format_description[MB_DESCRIPTION_LENGTH];
			if ((status = mb_format_description(verbose, &format, format_description, &error)) == MB_SUCCESS && format == i) {
				const char *format_informal_ptr   = strstr(format_description, "Informal Description:");
				const char *format_attributes_ptr = strstr(format_description, "Attributes:");
				char format_name[MB_DESCRIPTION_LENGTH];
				size_t format_name_len = MIN(MB_DESCRIPTION_LENGTH, strlen(format_description) - strlen(format_informal_ptr));
				strncpy(format_name, format_description, MIN(format_name_len, sizeof(format_description)));
				format_name[format_name_len - 1] = '\0';
				char format_informal[MB_DESCRIPTION_LENGTH];
				size_t format_informal_len = MIN(MB_DESCRIPTION_LENGTH, strlen(format_informal_ptr) - strlen(format_attributes_ptr));
				strncpy(format_informal, format_informal_ptr, MIN(format_informal_len, sizeof(format_description)));
				format_informal[format_informal_len - 1] = '\0';
				char format_attributes[MB_DESCRIPTION_LENGTH];
				strcpy(format_attributes, format_attributes_ptr);
				format_attributes[strlen(format_attributes_ptr) - 1] = '\0';
				printf("\n<UL>\n<LI>MBIO Data Format ID:  %d </LI>\n", format);
				printf("\n<UL>\n<LI>%s</LI>\n", format_name);
				printf("\n<LI>%s</LI>\n", format_informal);
				printf("\n<LI>%s</LI>\n", format_attributes);
				printf("</UL>\n</UL>\n");
			}
		}

		printf("\n<CENTER><P><BR>\n");
		printf("\n<P>\n<HR WIDTH=\"67%%\"></P>\n\n");
		printf("<center>\n");
		printf("\t<a href=\"https://www.mbari.org/products/research-software/mb-system/\"><< MB-System website</a> ");
		printf("| <a href=\"https://www.mbari.org/technology/mb-system/documentation/\"> Online MB-System Documenation>></a> ");
		printf("| <a href=\"index.html\">MB-System Information in Local Installation</a></p>\n");
		printf("</center>\n");
		printf("\n</BODY>\n</HTML>\n");

		status = MB_SUCCESS;
	}
	else if (list_mode) {
		for (int i = 0; i <= 1000; i++) {
			format = i;
			if ((status = mb_format(verbose, &format, &error)) == MB_SUCCESS && format == i) {
				printf("%d\n", format);
			}
		}
		status = MB_SUCCESS;
		error  = MB_ERROR_NO_ERROR;
	}
	else {
		printf("\nSupported MBIO Formats:\n");
		for (int i = 0; i <= 1000; i++) {
			format = i;
			char format_description[MB_DESCRIPTION_LENGTH];
			if ((status = mb_format_description(verbose, &format, format_description, &error)) == MB_SUCCESS && format == i) {
				printf("\nMBIO Data Format ID:  %d\n", format);
				printf("%s", format_description);
			}
		}
		status = MB_SUCCESS;
		error  = MB_ERROR_NO_ERROR;
	}

	if (verbose >= 2) {
		GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Program <%s> completed\n", THIS_MODULE_NAME);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  Ending status:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       status:  %d\n", status);
	}

	Return(error);
}
/*--------------------------------------------------------------------*/

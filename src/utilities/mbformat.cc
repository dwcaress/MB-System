/*--------------------------------------------------------------------
 *    The MB-system:	mbformat.c	1/22/93
 *
 *    Copyright (c) 1993-2023 by
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
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_status.h"

enum MbformatList {
  MBFORMAT_LIST_LONG = 0,
  MBFORMAT_LIST_SIMPLE = 1,
  MBFORMAT_LIST_ROOT = 2
};

constexpr char program_name[] = "MBFORMAT";
constexpr char help_message[] =
    "MBFORMAT is an utility which identifies the swath data formats\n"
    "associated with MBIO format id's.  If no format id is specified,\n"
    "MBFORMAT lists all of the currently supported formats.";
constexpr char usage_message[] = "mbformat [-Fformat -Ifile -L -K -V -W -H]";

/*--------------------------------------------------------------------*/
int main(int argc, char **argv) {
	char file[MB_PATH_MAXLINE];
	int verbose = 0;
	bool file_specified = false;
	int format = 0;
	bool format_specified = false;
	bool html = false;
	enum MbformatList list_mode = MBFORMAT_LIST_LONG;

	/* process argument list */
	{
		bool errflg = false;
		bool help = false;
		int c;
		while ((c = getopt(argc, argv, "F:f:HhI:i:LlKkVvWw")) != -1)
			switch (c) {
			case 'F':
			case 'f':
				sscanf(optarg, "%d", &format);
				format_specified = true;
				break;
			case 'L':
			case 'l':
				list_mode = MBFORMAT_LIST_SIMPLE;
				break;
			case 'K':
			case 'k':
				list_mode = MBFORMAT_LIST_ROOT;
				break;
			case 'H':
			case 'h':
				help = true;
				break;
			case 'I':
			case 'i':
				sscanf(optarg, "%1023s", file);
				file_specified = true;
				break;
			case 'V':
			case 'v':
				verbose++;
				break;
			case 'W':
			case 'w':
				html = true;
				break;
			case '?':
				errflg = true;
			}

		if (errflg) {
			fprintf(stderr, "usage: %s\n", usage_message);
			exit(MB_ERROR_BAD_USAGE);
		}

		if (verbose == 1 || help) {
			fprintf(stderr, "\nProgram %s\n", program_name);
			fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
		}

		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
			fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
			fprintf(stderr, "dbg2  Control Parameters:\n");
			fprintf(stderr, "dbg2       verbose: %d\n", verbose);
			fprintf(stderr, "dbg2       help:    %d\n", help);
			if (format_specified)
				fprintf(stderr, "dbg2       format:  %d\n", format);
			if (file_specified)
				fprintf(stderr, "dbg2       file:    %s\n", file);
		}

		if (help) {
			fprintf(stderr, "\n%s\n", help_message);
			fprintf(stderr, "\nusage: %s\n", usage_message);
			exit(MB_ERROR_NO_ERROR);
		}
	} /* process argument list */

	int status = MB_SUCCESS;
	int error = MB_ERROR_NO_ERROR;

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
			printf("Program %s unable to infer format from filename %s\n", program_name, file);
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
				printf("Program %s unable to infer format from filename %s\n", program_name, file);
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
				const char *format_informal_ptr =
				    (char *)strstr(format_description, "Informal Description:");
				const char *format_attributes_ptr =
				    (char *)strstr(format_description, "Attributes:");
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
		printf("\n</BODY>\n</HTML>\n");

		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
	}
	else if (list_mode) {
		for (int i = 0; i <= 1000; i++) {
			format = i;
			if ((status = mb_format(verbose, &format, &error)) == MB_SUCCESS && format == i) {
				printf("%d\n", format);
			}
		}
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
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
		error = MB_ERROR_NO_ERROR;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
		fprintf(stderr, "dbg2  Ending status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	exit(error);
}
/*--------------------------------------------------------------------*/

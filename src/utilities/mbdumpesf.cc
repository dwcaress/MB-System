/*--------------------------------------------------------------------
 *    The MB-system:	mbdumpesf.c	3/20/2008
 *
 *    Copyright (c) 2008-2023 by
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
 * mbdumpesf reads an MB-System edit save file and dumps the contents
 * as an ascii table to stdout. This is primarily used for debugging
 * bathymetry editing tools such as mbedit and mbeditviz.
 *
 * Author:	D. W. Caress
 * Date:	March 20, 2008
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mb_swap.h"

typedef enum {
    OUTPUT_TEXT = 0,
    OUTPUT_ESF = 1,
} omode_t;

constexpr char program_name[] = "mbdumpesf";
constexpr char help_message[] =
    "mbdumpesf reads an MB-System edit save file and dumps the\n"
    "contents as an ascii table to stdout.";
constexpr char usage_message[] =
    "mbdumpesf --input=esffile\n"
    "\t[--output=esffile --ignore-unflag --ignore-flag\n"
    "\t--ignore-filter --ignore-zero\n"
    "\t--verbose --help]";

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	int verbose = 0;

	/* MBIO read and write control parameters */
	char iesffile[MB_PATH_MAXLINE];
	char oesffile[MB_PATH_MAXLINE];
	omode_t omode = OUTPUT_TEXT;
	FILE *iesffp = nullptr;
	FILE *oesffp = nullptr;

	bool ignore_unflag = false;
	bool ignore_flag = false;
	bool ignore_filter = false;
	bool ignore_zero = false;

	/* process argument list */
	{
		const struct option options[] = {
			{"verbose", no_argument, nullptr, 0},
			{"help", no_argument, nullptr, 0},
			{"input", required_argument, nullptr, 0},
			{"output", required_argument, nullptr, 0},
			{"ignore-unflag", no_argument, nullptr, 0},
			{"ignore-flag", no_argument, nullptr, 0},
			{"ignore-filter", no_argument, nullptr, 0},
			{"ignore-zero", no_argument, nullptr, 0},
			{nullptr, 0, nullptr, 0}};
		int option_index;
		bool errflg = false;
		int c;
		bool help = false;
		while ((c = getopt_long(argc, argv, "VvHhI:i:", options, &option_index)) != -1)
		{
			switch (c) {
			/* long options all return c=0 */
			case 0:
				if (strcmp("verbose", options[option_index].name) == 0) {
					verbose++;
				}
				else if (strcmp("help", options[option_index].name) == 0) {
					help = true;
				}
				else if (strcmp("input", options[option_index].name) == 0) {
					strcpy(iesffile, optarg);
				}
				else if (strcmp("output", options[option_index].name) == 0) {
					strcpy(oesffile, optarg);
					omode = OUTPUT_ESF;
				}
				else if (strcmp("ignore-unflag", options[option_index].name) == 0) {
					ignore_unflag = true;
				}
				else if (strcmp("ignore-flag", options[option_index].name) == 0) {
					ignore_flag = true;
				}
				else if (strcmp("ignore-filter", options[option_index].name) == 0) {
					ignore_filter = true;
				}
				else if (strcmp("ignore-zero", options[option_index].name) == 0) {
					ignore_zero = true;
				}

				break;
			case 'H':
			case 'h':
				help = true;
				break;
			case 'V':
			case 'v':
				verbose++;
				break;
			case 'I':
			case 'i':
				sscanf(optarg, "%1023s", iesffile);
				break;
			case '?':
				errflg = true;
			}
		}

		if (errflg) {
			fprintf(stderr, "usage: %s\n", usage_message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_BAD_USAGE);
		}

		if (help) {
			fprintf(stderr, "\n%s\n", help_message);
			fprintf(stderr, "\nusage: %s\n", usage_message);
			exit(MB_ERROR_NO_ERROR);
		}

		if (verbose == 1 || help) {
			fprintf(stderr, "\nProgram %s\n", program_name);
			fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
		}

		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
			fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
			fprintf(stderr, "dbg2  Control Parameters:\n");
			fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
			fprintf(stderr, "dbg2       help:             %d\n", help);
			fprintf(stderr, "dbg2       input esf file:   %s\n", iesffile);
			fprintf(stderr, "dbg2       omode:            %d\n", omode);
			if (omode == OUTPUT_ESF)
				fprintf(stderr, "dbg2       output esf file:  %s\n", oesffile);
			fprintf(stderr, "dbg2       ignore_unflag:    %d\n", ignore_unflag);
			fprintf(stderr, "dbg2       ignore_flag:      %d\n", ignore_flag);
			fprintf(stderr, "dbg2       ignore_filter:    %d\n", ignore_filter);
			fprintf(stderr, "dbg2       ignore_zero:      %d\n", ignore_zero);
		}
	}

	double time_d;
	int beam;
	int action;

	int beam_flag = 0;
	int beam_unflag = 0;
	int beam_zero = 0;
	int beam_filter = 0;
	int beam_flag_ignore = 0;
	int beam_unflag_ignore = 0;
	int beam_zero_ignore = 0;
	int beam_filter_ignore = 0;

	const int byteswapped = mb_swap_check();

	int error = MB_ERROR_NO_ERROR;

	/* check that esf file exists */
	struct stat file_status;
	const int fstat = stat(iesffile, &file_status);
	if (fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
		/* open the input esf file */
		if ((iesffp = fopen(iesffile, "r")) == nullptr) {
			fprintf(stderr, "\nUnable to edit save file <%s> for reading\n", iesffile);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_OPEN_FAIL);
		}

		/* open the output esf file */
		if (omode == OUTPUT_ESF && (oesffp = fopen(oesffile, "w")) == nullptr) {
			fprintf(stderr, "\nUnable to edit save file <%s> for reading\n", iesffile);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_OPEN_FAIL);
		}

		/* read file header to discern the format */
		int status = MB_SUCCESS;
		int nedit = 0;
		char esf_header[MB_PATH_MAXLINE * 3];  /* Need more space than an mb_path */
		if (fread(esf_header, MB_PATH_MAXLINE, 1, iesffp) == 1 && strncmp(esf_header, "ESFVERSION", 10) == 0) {
			nedit = (file_status.st_size - MB_PATH_MAXLINE) / (sizeof(double) + 2 * sizeof(int));

			if (omode == OUTPUT_ESF && oesffp != nullptr) {
				memset(esf_header, 0, MB_PATH_MAXLINE);
				const int esf_mode = MB_ESF_MODE_EXPLICIT;
        char user[256], host[256], date[32];
        status = mb_user_host_date(verbose, user, host, date, &error);
				snprintf(esf_header, sizeof(esf_header),
				        "ESFVERSION03\nESF Mode: %d\nMB-System Version %s\nProgram: %s\nUser: %s\nCPU: %s\nDate: %s\n",
				        esf_mode, MB_VERSION, program_name, user, host, date);
				if (fwrite(esf_header, MB_PATH_MAXLINE, 1, oesffp) != 1) {
					status = MB_FAILURE;
					error = MB_ERROR_WRITE_FAIL;
				}
			}
		}
		else {
			rewind(iesffp);
			nedit = file_status.st_size / (sizeof(double) + 2 * sizeof(int));
		}

		/* loop over reading edit events and printing them out */
		for (int i = 0; i < nedit && error == MB_ERROR_NO_ERROR; i++) {
			bool ignore = false;
			if (fread(&(time_d), sizeof(double), 1, iesffp) != 1 || fread(&(beam), sizeof(int), 1, iesffp) != 1 ||
			    fread(&(action), sizeof(int), 1, iesffp) != 1) {
				ignore = true;
				status = MB_FAILURE;
				error = MB_ERROR_EOF;
			}
			else if (strncmp(((char *)&time_d), "ESFVERSI", 8) == 0) {
				ignore = true;
				if (fread(esf_header, MB_PATH_MAXLINE-16, 1, iesffp) != 1) {
					status = MB_FAILURE;
					error = MB_ERROR_EOF;
				}
			}
			else if (byteswapped) {
				mb_swap_double(&(time_d));
				beam = mb_swap_int(beam);
				action = mb_swap_int(action);
			}

			if (!ignore) {
				if (action == MBP_EDIT_FLAG) {
					beam_flag++;
					if (ignore_flag) {
						ignore = true;
						beam_flag_ignore++;
					}
				}
				else if (action == MBP_EDIT_UNFLAG) {
					beam_unflag++;
					if (ignore_unflag) {
						ignore = true;
						beam_unflag_ignore++;
					}
				}
				else if (action == MBP_EDIT_ZERO) {
					beam_zero++;
					if (ignore_zero) {
						ignore = true;
						beam_zero_ignore++;
					}
				}
				else if (action == MBP_EDIT_FILTER) {
					beam_filter++;
					if (ignore_filter) {
						ignore = true;
						beam_filter_ignore++;
					}
				}
			}

			/* write out the edit if not ignored */
			if (!ignore) {
				if (omode == OUTPUT_TEXT) {
					int time_i[7];
					mb_get_date(verbose, time_d, time_i);
					fprintf(stdout, "EDITS READ: i:%d time: %f %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d beam:%d action:%d\n", i,
					        time_d, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], beam, action);
				}
				else {
					if (byteswapped) {
						mb_swap_double(&time_d);
						beam = mb_swap_int(beam);
						action = mb_swap_int(action);
					}
					if (fwrite(&time_d, sizeof(double), 1, oesffp) != 1) {
						status = MB_FAILURE;
						error = MB_ERROR_WRITE_FAIL;
					}
					if (status == MB_SUCCESS && fwrite(&beam, sizeof(int), 1, oesffp) != 1) {
						status = MB_FAILURE;
						error = MB_ERROR_WRITE_FAIL;
					}
					if (status == MB_SUCCESS && fwrite(&action, sizeof(int), 1, oesffp) != 1) {
						status = MB_FAILURE;
						error = MB_ERROR_WRITE_FAIL;
					}
				}
			}
      else {
				if (omode == OUTPUT_TEXT) {
					int time_i[7];
					mb_get_date(verbose, time_d, time_i);
					fprintf(stdout, "** EDITS READ BUT IGNORED **: i:%d time: %f %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d beam:%d action:%d\n", i,
					        time_d, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], beam, action);
				}
      }
		}

		fclose(iesffp);
		if (omode == OUTPUT_ESF)
			fclose(oesffp);
	}

	/* give the statistics */
	if (verbose >= 1) {
		fprintf(stderr, "\nBeam flag read totals:\n");
		fprintf(stderr, "\t%d beams flagged manually\n", beam_flag);
		fprintf(stderr, "\t%d beams unflagged\n", beam_unflag);
		fprintf(stderr, "\t%d beams zeroed\n", beam_zero);
		fprintf(stderr, "\t%d beams flagged by filter\n", beam_filter);
		if (ignore_flag || ignore_unflag || ignore_zero || ignore_filter) {
			fprintf(stderr, "\nBeam flag ignore totals:\n");
			fprintf(stderr, "\t%d beams flagged manually (ignored in output)\n", beam_flag_ignore);
			fprintf(stderr, "\t%d beams unflagged (ignored in output)\n", beam_unflag_ignore);
			fprintf(stderr, "\t%d beams zeroed (ignored in output)\n", beam_zero_ignore);
			fprintf(stderr, "\t%d beams flagged by filter (ignored in output)\n", beam_filter_ignore);
		}
	}

	exit(error);
}
/*--------------------------------------------------------------------*/

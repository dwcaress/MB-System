/*--------------------------------------------------------------------
 *    The MB-system:	mbdatalist.c	10/10/2001
 *
 *    Copyright (c) 2001-2019 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBdatalist parses recursive datalist files and outputs the
 * complete list of data files and formats.
 * The results are dumped to stdout.
 *
 * Author:	D. W. Caress
 * Date:	October 10, 2001
 */

#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_process.h"
#include "mb_status.h"

static const char program_name[] = "mbdatalist";
static const char help_message[] =
    "mbdatalist parses recursive datalist files and outputs the\n"
    "complete list of data files and formats. The results are dumped to stdout.";
static const char usage_message[] =
    "mbdatalist [-C -D -Fformat -Ifile -N -O -P -Q -Rw/e/s/n -S -U -Y -Z -V -H]";

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	int verbose = 0;
	int format;
	int pings;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double speedmin;
	double timegap;
	int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

	/* set default input to stdin */
	char read_file[MB_PATH_MAXLINE] = "datalist.mb-1";

	bool copyfiles = false;
	bool force_update = false;
	bool make_inf = false;
	int look_processed = MB_DATALIST_LOOK_UNSET;
	bool problem_report = false;
	bool look_bounds = false;
	bool status_report = false;
	bool remove_locks = false;
	bool make_datalistp = false;
	bool reportdatalists = false;
	FILE *output = NULL;

	{
		int option_index;

		const struct option options[] = {
			{"verbose", no_argument, NULL, 0},
	                {"help", no_argument, NULL, 0},
	                {"copy", no_argument, NULL, 0},
	                {"report", no_argument, NULL, 0},
	                {"format", required_argument, NULL, 0},
	                {"input", required_argument, NULL, 0},
	                {"make-ancilliary", no_argument, NULL, 0},
	                {"update-ancilliary", no_argument, NULL, 0},
	                {"processed", no_argument, NULL, 0},
	                {"problem", no_argument, NULL, 0},
	                {"bounds", required_argument, NULL, 0},
	                {"status", no_argument, NULL, 0},
	                {"raw", no_argument, NULL, 0},
	                {"unlock", no_argument, NULL, 0},
	                {"datalistp", no_argument, NULL, 0},
	                {NULL, 0, NULL, 0}};

		bool errflg = false;
		int c;
		bool help = false;
		while ((c = getopt_long(argc, argv, "VvHhCcDdF:f:I:i:NnOoPpQqR:r:SsUuYyZz", options, &option_index)) != -1)
		{
			switch (c) {
			/* long options */
			case 0:
				if (strcmp("verbose", options[option_index].name) == 0) {
					verbose++;
				}
				else if (strcmp("help", options[option_index].name) == 0) {
					help = true;
				}
				else if (strcmp("copy", options[option_index].name) == 0) {
					copyfiles = true;
				}
				else if (strcmp("report", options[option_index].name) == 0) {
					copyfiles = true;
				}
				else if (strcmp("format", options[option_index].name) == 0) {
					sscanf(optarg, "%d", &format);
				}
				else if (strcmp("input", options[option_index].name) == 0) {
					sscanf(optarg, "%1023s", read_file);
				}
				else if (strcmp("make-ancilliary", options[option_index].name) == 0) {
					force_update = true;
					make_inf = true;
				}
				else if (strcmp("update-ancilliary", options[option_index].name) == 0) {
					make_inf = true;
				}
				else if (strcmp("processed", options[option_index].name) == 0) {
					look_processed = MB_DATALIST_LOOK_YES;
				}
				else if (strcmp("problem", options[option_index].name) == 0) {
					problem_report = true;
				}
				else if (strcmp("bounds", options[option_index].name) == 0) {
					mb_get_bounds(optarg, bounds);
					look_bounds = true;
				}
				else if (strcmp("status", options[option_index].name) == 0) {
					status_report = true;
				}
				else if (strcmp("raw", options[option_index].name) == 0) {
					look_processed = MB_DATALIST_LOOK_NO;
				}
				else if (strcmp("unlock", options[option_index].name) == 0) {
					remove_locks = true;
				}
				else if (strcmp("datalistp", options[option_index].name) == 0) {
					make_datalistp = true;
				}

				break;

			/* short options (deprecated) */
			case 'C':
			case 'c':
				copyfiles = true;
				break;
			case 'D':
			case 'd':
				reportdatalists = true;
				break;
			case 'F':
			case 'f':
				sscanf(optarg, "%d", &format);
				break;
			case 'H':
			case 'h':
				help = true;
				break;
			case 'I':
			case 'i':
				sscanf(optarg, "%1023s", read_file);
				break;
			case 'N':
			case 'n':
				force_update = true;
				make_inf = true;
				break;
			case 'O':
			case 'o':
				make_inf = true;
				break;
			case 'P':
			case 'p':
				look_processed = MB_DATALIST_LOOK_YES;
				break;
			case 'Q':
			case 'q':
				problem_report = true;
				break;
			case 'R':
			case 'r':
				mb_get_bounds(optarg, bounds);
				look_bounds = true;
				break;
			case 'S':
			case 's':
				status_report = true;
				break;
			case 'U':
			case 'u':
				look_processed = MB_DATALIST_LOOK_NO;
				break;
			case 'V':
			case 'v':
				verbose++;
				break;
			case 'Y':
			case 'y':
				remove_locks = true;
				break;
			case 'Z':
			case 'z':
				make_datalistp = true;
				break;
			case '?':
				errflg = true;
			}
		}

		if (verbose <= 1)
			output = stdout;
		else
			output = stderr;

		if (errflg) {
			fprintf(output, "usage: %s\n", usage_message);
			fprintf(output, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_BAD_USAGE);
		}

		if (verbose == 1 || help) {
			fprintf(output, "\nProgram %s\n", program_name);
			fprintf(output, "MB-system Version %s\n", MB_VERSION);
		}

		if (verbose >= 2) {
			fprintf(output, "\ndbg2  Program <%s>\n", program_name);
			fprintf(output, "dbg2  MB-system Version %s\n", MB_VERSION);
			fprintf(output, "dbg2  Control Parameters:\n");
			fprintf(output, "dbg2       verbose:             %d\n", verbose);
			fprintf(output, "dbg2       help:                %d\n", help);
			fprintf(output, "dbg2       file:                %s\n", read_file);
			fprintf(output, "dbg2       format:              %d\n", format);
			fprintf(output, "dbg2       look_processed:      %d\n", look_processed);
			fprintf(output, "dbg2       copyfiles:           %d\n", copyfiles);
			fprintf(output, "dbg2       reportdatalists:     %d\n", reportdatalists);
			fprintf(output, "dbg2       make_inf:            %d\n", make_inf);
			fprintf(output, "dbg2       force_update:        %d\n", force_update);
			fprintf(output, "dbg2       status_report:       %d\n", status_report);
			fprintf(output, "dbg2       problem_report:      %d\n", problem_report);
			fprintf(output, "dbg2       make_datalistp:      %d\n", make_datalistp);
			fprintf(output, "dbg2       remove_locks:        %d\n", remove_locks);
			fprintf(output, "dbg2       pings:               %d\n", pings);
			fprintf(output, "dbg2       lonflip:             %d\n", lonflip);
			fprintf(output, "dbg2       bounds[0]:           %f\n", bounds[0]);
			fprintf(output, "dbg2       bounds[1]:           %f\n", bounds[1]);
			fprintf(output, "dbg2       bounds[2]:           %f\n", bounds[2]);
			fprintf(output, "dbg2       bounds[3]:           %f\n", bounds[3]);
			fprintf(output, "dbg2       btime_i[0]:          %d\n", btime_i[0]);
			fprintf(output, "dbg2       btime_i[1]:          %d\n", btime_i[1]);
			fprintf(output, "dbg2       btime_i[2]:          %d\n", btime_i[2]);
			fprintf(output, "dbg2       btime_i[3]:          %d\n", btime_i[3]);
			fprintf(output, "dbg2       btime_i[4]:          %d\n", btime_i[4]);
			fprintf(output, "dbg2       btime_i[5]:          %d\n", btime_i[5]);
			fprintf(output, "dbg2       btime_i[6]:          %d\n", btime_i[6]);
			fprintf(output, "dbg2       etime_i[0]:          %d\n", etime_i[0]);
			fprintf(output, "dbg2       etime_i[1]:          %d\n", etime_i[1]);
			fprintf(output, "dbg2       etime_i[2]:          %d\n", etime_i[2]);
			fprintf(output, "dbg2       etime_i[3]:          %d\n", etime_i[3]);
			fprintf(output, "dbg2       etime_i[4]:          %d\n", etime_i[4]);
			fprintf(output, "dbg2       etime_i[5]:          %d\n", etime_i[5]);
			fprintf(output, "dbg2       etime_i[6]:          %d\n", etime_i[6]);
			fprintf(output, "dbg2       speedmin:            %f\n", speedmin);
			fprintf(output, "dbg2       timegap:             %f\n", timegap);
		}

		if (help) {
			fprintf(output, "\n%s\n", help_message);
			fprintf(output, "\nusage: %s\n", usage_message);
			exit(MB_ERROR_NO_ERROR);
		}
	}

	int error = MB_ERROR_NO_ERROR;

	/* output stream for basic stuff (stdout if verbose <= 1,
	    output if verbose > 1) */

	if (make_datalistp) {
		/* figure out data format and fileroot if possible */
		char fileroot[MB_PATH_MAXLINE];
		status = mb_get_format(verbose, read_file, fileroot, &format, &error);
		char file[MB_PATH_MAXLINE];
		sprintf(file, "%sp.mb-1", fileroot);

		FILE *fp = fopen(file, "w");
		if (fp == NULL) {
			fprintf(stderr, "\nUnable to open output file %s\n", file);
			fprintf(stderr, "Program %s aborted!\n", program_name);
			exit(MB_ERROR_OPEN_FAIL);
		}
		fprintf(fp, "$PROCESSED\n%s %d\n", read_file, format);
		fclose(fp);
		if (verbose > 0)
			fprintf(output, "Convenience datalist file %s created...\n", file);

		/* exit unless building ancillary files has also been requested */
		if (!make_inf)
			exit(error);
	}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose, read_file, NULL, &format, &error);

	void *datalist;
	double file_weight = 1.0;
	char dfile[MB_PATH_MAXLINE];
	char dfilelast[MB_PATH_MAXLINE];
	char pwd[MB_PATH_MAXLINE];
	char command[MB_PATH_MAXLINE];
	char *filename;
	int nfile = 0;
	int nparproblem;
	int ndataproblem;
	int nparproblemtot = 0;
	int ndataproblemtot = 0;
	int nproblemfiles = 0;
	int recursion = -1;

	int prstatus = MB_PR_FILE_UP_TO_DATE;
	int lock_error = MB_ERROR_NO_ERROR;
	int lock_purpose = 0;
	mb_path lock_program = "";
	mb_path lock_cpu = "";
	mb_path lock_user = "";
	char lock_date[25] = "";
	mb_path lockfile = "";
	int file_in_bounds = false;  // TODO(schwehr): Convert mb_check_info to bool.
	int locked = false;  // TODO(schwehr): Convert mb_pr_lockinfo to bool.

	/* if not a datalist just output filename format and weight */
	if (format > 0) {
		nfile++;

		if (make_inf) {
			status = mb_make_info(verbose, force_update, read_file, format, &error);
		}
		else if (problem_report) {
			status = mb_pr_check(verbose, read_file, &nparproblem, &ndataproblem, &error);
			if (nparproblem + ndataproblem > 0)
				nproblemfiles++;
			nparproblemtot += nparproblem;
			ndataproblemtot += ndataproblem;
		}
		else {
			/* check for mbinfo file if bounds checking enabled */
			if (look_bounds) {
				status = mb_check_info(verbose, read_file, lonflip, bounds, &file_in_bounds, &error);
				if (status == MB_FAILURE) {
					file_in_bounds = true;
					status = MB_SUCCESS;
					error = MB_ERROR_NO_ERROR;
				}
			}

			/* output file if no bounds checking or in bounds */
			if (!look_bounds || file_in_bounds) {
				if (verbose > 0)
					fprintf(output, "%s %d %f\n", read_file, format, file_weight);
				else
					fprintf(output, "%s %d %f", read_file, format, file_weight);

				/* check status if desired */
				if (status_report) {
					status = mb_pr_checkstatus(verbose, read_file, &prstatus, &error);
					if (verbose > 0) {
						if (prstatus == MB_PR_FILE_UP_TO_DATE)
							fprintf(output, "\tStatus: up to date\n");
						else if (prstatus == MB_PR_FILE_NEEDS_PROCESSING)
							fprintf(output, "\tStatus: out of date - needs processing\n");
						else if (prstatus == MB_PR_FILE_NOT_EXIST)
							fprintf(output, "\tStatus: file does not exist\n");
						else if (prstatus == MB_PR_NO_PARAMETER_FILE)
							fprintf(output, "\tStatus: no parameter file - processing undefined\n");
					}
					else {
						if (prstatus == MB_PR_FILE_UP_TO_DATE)
							fprintf(output, "\t<Up-to-date>");
						else if (prstatus == MB_PR_FILE_NEEDS_PROCESSING)
							fprintf(output, "\t<Needs-processing>");
						else if (prstatus == MB_PR_FILE_NOT_EXIST)
							fprintf(output, "\t<Does-not-exist>");
						else if (prstatus == MB_PR_NO_PARAMETER_FILE)
							fprintf(output, "\t<No-parameter-file>");
					}
				}

				/* check locks if desired */
				if (status_report || remove_locks) {
					/* int lock_status = */ mb_pr_lockinfo(verbose, read_file, &locked, &lock_purpose, lock_program, lock_user, lock_cpu,
					                             lock_date, &lock_error);
					if (locked && status_report) {
						if (verbose > 0)
							fprintf(output, "\tLocked by program <%s> run by <%s> on <%s> at <%s>\n", lock_program, lock_user,
							        lock_cpu, lock_date);
						else
							fprintf(output, "\t<Locked>");
					}
					if (locked && remove_locks) {
						char file[MB_PATH_MAXLINE];
						sprintf(lockfile, "%s.lck", file);
						sprintf(command, "/bin/rm -f %s", lockfile);
					}
				}

				if (verbose == 0)
					fprintf(output, "\n");
			}
		}
	}

	/* else parse datalist */
	else {
		if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
			fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_OPEN_FAIL);
		}
		char file[MB_PATH_MAXLINE];
		while (mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS) {
			nfile++;
			/* char *bufptr = */ getcwd(pwd, MB_PATH_MAXLINE);
			mb_get_relative_path(verbose, file, pwd, &error);
			mb_get_relative_path(verbose, dfile, pwd, &error);

			/* generate inf fnv fbt files */
			if (make_inf) {
				status = mb_make_info(verbose, force_update, file, format, &error);
			}

			/* or generate problem reports */
			else if (problem_report) {
				status = mb_pr_check(verbose, file, &nparproblem, &ndataproblem, &error);
				if (nparproblem + ndataproblem > 0)
					nproblemfiles++;
				nparproblemtot += nparproblem;
				ndataproblemtot += ndataproblem;
			}

			/* or copy files */
			else if (copyfiles) {
				/* check for mbinfo file if bounds checking enabled */
				if (look_bounds) {
					status = mb_check_info(verbose, file, lonflip, bounds, &file_in_bounds, &error);
					if (status == MB_FAILURE) {
						file_in_bounds = true;
						status = MB_SUCCESS;
						error = MB_ERROR_NO_ERROR;
					}
				}

				/* copy file if no bounds checking or in bounds */
				if (!look_bounds || file_in_bounds) {
					fprintf(output, "Copying %s %d %f\n", file, format, file_weight);
					sprintf(command, "cp %s* .", file);
					/* shellstatus = */ system(command);
					if ((filename = strrchr(file, '/')) != NULL)
						filename++;
					else
						filename = file;
					if (nfile == 1)
						/* shellstatus = */ system("rm datalist.mb-1");
					sprintf(command, "echo %s %d %f >> datalist.mb-1", filename, format, file_weight);
					/* shellstatus = */ system(command);
				}
			}

			/* or list the datalists parsed through the recursive datalist structure */
			else if (reportdatalists) {
				if (strcmp(dfile, dfilelast) != 0)
					status = mb_datalist_recursion(verbose, datalist, true, &recursion, &error);
				strcpy(dfilelast, dfile);
			}

			/* or list the files returned from parsing the recursive datalist
			    structure, with bounds checking if desired */
			else {
				/* check for mbinfo file if bounds checking enabled */
				if (look_bounds) {
					status = mb_check_info(verbose, file, lonflip, bounds, &file_in_bounds, &error);
					if (status == MB_FAILURE) {
						file_in_bounds = true;
						status = MB_SUCCESS;
						error = MB_ERROR_NO_ERROR;
					}
				}

				/* output file if no bounds checking or in bounds */
				if (!look_bounds || file_in_bounds) {
					if (verbose > 0)
						fprintf(output, "%s %d %f\n", file, format, file_weight);
					else
						fprintf(output, "%s %d %f", file, format, file_weight);

					/* check status if desired */
					if (status_report) {
						status = mb_pr_checkstatus(verbose, file, &prstatus, &error);
						if (verbose > 0) {
							if (prstatus == MB_PR_FILE_UP_TO_DATE)
								fprintf(output, "\tStatus: up to date\n");
							else if (prstatus == MB_PR_FILE_NEEDS_PROCESSING)
								fprintf(output, "\tStatus: out of date - needs processing\n");
							else if (prstatus == MB_PR_FILE_NOT_EXIST)
								fprintf(output, "\tStatus: file does not exist\n");
							else if (prstatus == MB_PR_NO_PARAMETER_FILE)
								fprintf(output, "\tStatus: no parameter file - processing undefined\n");
						}
						else {
							if (prstatus == MB_PR_FILE_UP_TO_DATE)
								fprintf(output, "\t<Up-to-date>");
							else if (prstatus == MB_PR_FILE_NEEDS_PROCESSING)
								fprintf(output, "\t<Needs-processing>");
							else if (prstatus == MB_PR_FILE_NOT_EXIST)
								fprintf(output, "\t<Does-not-exist>");
							else if (prstatus == MB_PR_NO_PARAMETER_FILE)
								fprintf(output, "\t<No-parameter-file>");
						}
					}

					/* check locks if desired */
					if (status_report || remove_locks) {
						/* int lock_status = */ mb_pr_lockinfo(verbose, file, &locked, &lock_purpose, lock_program, lock_user, lock_cpu,
						                             lock_date, &lock_error);
						if (locked && status_report) {
							if (verbose > 0)
								fprintf(output, "\tLocked by program <%s> run by <%s> on <%s> at <%s>\n", lock_program, lock_user,
								        lock_cpu, lock_date);
							else
								fprintf(output, "\t<Locked>");
						}
						if (locked && remove_locks) {
							sprintf(lockfile, "%s.lck", file);
							fprintf(output, "\tRemoving lock file %s\n", lockfile);
							sprintf(command, "/bin/rm -f %s", lockfile);
							/* shellstatus = */ system(command);
						}
					}

					if (verbose == 0)
						fprintf(output, "\n");
				}
			}
		}
		mb_datalist_close(verbose, &datalist, &error);
	}

	/* set program status */
	status = MB_SUCCESS;

	/* output counts */
	if (verbose > 0) {
		fprintf(output, "\nTotal swath files:         %d\n", nfile);
		if (problem_report) {
			fprintf(output, "Total files with problems: %d\n", nproblemfiles);
			fprintf(output, "Total parameter problems:  %d\n", nparproblemtot);
			fprintf(output, "Total data problems:       %d\n", ndataproblemtot);
		}
	}

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose, &error);

	if (verbose >= 2) {
		fprintf(output, "\ndbg2  Program <%s> completed\n", program_name);
		fprintf(output, "dbg2  Ending status:\n");
		fprintf(output, "dbg2       status:  %d\n", status);
	}

	exit(error);
}
/*--------------------------------------------------------------------*/

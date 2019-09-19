/*--------------------------------------------------------------------
 *    The MB-system:	mbgetesf.c	6/15/93
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
 * mbgetesf reads a multibeam data file and writes out
 * an edit save file which can be applied to other data files
 * containing the same data (but presumably in a different
 * state of processing).  This allows editing of one data file to
 * be transferred to another with ease.  The programs mbedit and
 * mbprocess can be used to apply the edit events to another file.
 *
 * Author:	D. W. Caress
 * Date:	January 24, 2001
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mb_swap.h"

#define MBGETESF_FLAGONLY 1
#define MBGETESF_FLAGNULL 2
#define MBGETESF_ALL 3
#define MBGETESF_IMPLICITBEST 4
#define MBGETESF_IMPLICITNULL 5
#define MBGETESF_IMPLICITGOOD 6

static const char program_name[] = "mbgetesf";
static const char help_message[] =
    "mbgetesf reads a multibeam data file and writes out\nan edit save file which can be applied to other "
    "data files\ncontaining the same data (but presumably in a different\nstate of processing).  This "
    "allows editing of one data file to\nbe transferred to another with ease.  The programs mbedit "
    "and\nmbprocess can be used to apply the edit events to another file.";
static const char usage_message[] =
    "mbgetesf [-Fformat -Iinfile -Mmode -Oesffile -V -H]";

/*--------------------------------------------------------------------*/
int mbgetesf_save_edit(int verbose, FILE *sofp, double time_d, int beam, int action, int *error) {
	int status = MB_SUCCESS;
	//	int time_i[7];

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");

		fprintf(stderr, "dbg2       sofp:            %p\n", (void *)sofp);
		fprintf(stderr, "dbg2       time_d:          %f\n", time_d);
		fprintf(stderr, "dbg2       beam:            %d\n", beam);
		fprintf(stderr, "dbg2       action:          %d\n", action);
	}
	// mb_get_date(verbose,time_d,time_i);
	// fprintf(stderr,"MBGETESF: time: %f %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d beam:%d action:%d\n",
	// time_d,time_i[0],time_i[1],time_i[2],
	// time_i[3],time_i[4],time_i[5],time_i[6],
	// beam,action);

	/* write out the edit */
	if (sofp != NULL) {
#ifdef BYTESWAPPED
		mb_swap_double(&time_d);
		beam = mb_swap_int(beam);
		action = mb_swap_int(action);
#endif
		if (fwrite(&time_d, sizeof(double), 1, sofp) != 1) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		if (status == MB_SUCCESS && fwrite(&beam, sizeof(int), 1, sofp) != 1) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		if (status == MB_SUCCESS && fwrite(&action, sizeof(int), 1, sofp) != 1) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:       %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	int errflg = 0;
	int c;
	int help = 0;
	int flag = 0;

	/* MBIO status variables */
	int status;
	int verbose = 0;
	int error = MB_ERROR_NO_ERROR;
	char *message = NULL;

	/* MBIO read control parameters */
	int format;
	int pings;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double btime_d;
	double etime_d;
	double speedmin;
	double timegap;
	char ifile[MB_PATH_MAXLINE];
	int beams_bath;
	int beams_amp;
	int pixels_ss;
	void *imbio_ptr = NULL;

	/* mbio read and write values */
	void *store_ptr;
	int kind;
	int time_i[7];
	double time_d;
	double navlon;
	double navlat;
	double speed;
	double heading;
	double distance;
	double altitude;
	double sonardepth;
	int nbath;
	int namp;
	int nss;
	char *beamflag = NULL;
	double *bath = NULL;
	double *bathacrosstrack = NULL;
	double *bathalongtrack = NULL;
	double *amp = NULL;
	double *ss = NULL;
	double *ssacrosstrack = NULL;
	double *ssalongtrack = NULL;
	int idata = 0;
	int beam_ok = 0;
	int beam_null = 0;
	int beam_ok_write = 0;
	int beam_null_write = 0;
	int beam_flag = 0;
	int beam_flag_manual = 0;
	int beam_flag_filter = 0;
	int beam_flag_sonar = 0;
	char comment[MB_COMMENT_MAXLINE];
	int mode;
	int kluge = 0;

	/* time, user, host variables */
	time_t right_now;
	char date[32], user[MBP_FILENAMESIZE], *user_ptr, host[MBP_FILENAMESIZE];
	mb_path esf_header;
	int esf_mode = MB_ESF_MODE_EXPLICIT;

	/* save file control variables */
	int sofile_set = MB_NO;
	mb_path sofile = "";
	FILE *sofp = NULL;
	int i;

	/* get current default values */
	status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

	/* reset all defaults but the format and lonflip */
	pings = 1;
	bounds[0] = -360.;
	bounds[1] = 360.;
	bounds[2] = -90.;
	bounds[3] = 90.;
	btime_i[0] = 1962;
	btime_i[1] = 2;
	btime_i[2] = 21;
	btime_i[3] = 10;
	btime_i[4] = 30;
	btime_i[5] = 0;
	btime_i[6] = 0;
	etime_i[0] = 2062;
	etime_i[1] = 2;
	etime_i[2] = 21;
	etime_i[3] = 10;
	etime_i[4] = 30;
	etime_i[5] = 0;
	etime_i[6] = 0;
	speedmin = 0.0;
	timegap = 1000000000.0;
	mode = MBGETESF_FLAGONLY;

	/* set default input and output */
	strcpy(ifile, "stdin");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhB:b:E:F:f:I:i:K:k:M:m:O:o:")) != -1)
		switch (c) {
		case 'H':
		case 'h':
			help++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'B':
		case 'b':
			sscanf(optarg, "%d/%d/%d/%d/%d/%d", &btime_i[0], &btime_i[1], &btime_i[2], &btime_i[3], &btime_i[4], &btime_i[5]);
			btime_i[6] = 0;
			flag++;
			break;
		case 'E':
		case 'e':
			sscanf(optarg, "%d/%d/%d/%d/%d/%d", &etime_i[0], &etime_i[1], &etime_i[2], &etime_i[3], &etime_i[4], &etime_i[5]);
			etime_i[6] = 0;
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf(optarg, "%d", &format);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf(optarg, "%s", ifile);
			flag++;
			break;
		case 'K':
		case 'k':
			sscanf(optarg, "%d", &kluge);
			flag++;
			break;
		case 'M':
		case 'm':
			sscanf(optarg, "%d", &mode);
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf(optarg, "%s", sofile);
			sofile_set = MB_YES;
			flag++;
			break;
		case '?':
			errflg++;
		}

	/* if error flagged then print it and exit */
	if (errflg) {
		fprintf(stderr, "usage: %s\n", usage_message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
	}

	if (verbose == 1 || help) {
		fprintf(stderr, "\nProgram %s\n", program_name);
		fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Control Parameters:\n");
		fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
		fprintf(stderr, "dbg2       help:           %d\n", help);
		fprintf(stderr, "dbg2       data format:    %d\n", format);
		fprintf(stderr, "dbg2       pings:          %d\n", pings);
		fprintf(stderr, "dbg2       lonflip:        %d\n", lonflip);
		fprintf(stderr, "dbg2       bounds[0]:      %f\n", bounds[0]);
		fprintf(stderr, "dbg2       bounds[1]:      %f\n", bounds[1]);
		fprintf(stderr, "dbg2       bounds[2]:      %f\n", bounds[2]);
		fprintf(stderr, "dbg2       bounds[3]:      %f\n", bounds[3]);
		fprintf(stderr, "dbg2       btime_i[0]:     %d\n", btime_i[0]);
		fprintf(stderr, "dbg2       btime_i[1]:     %d\n", btime_i[1]);
		fprintf(stderr, "dbg2       btime_i[2]:     %d\n", btime_i[2]);
		fprintf(stderr, "dbg2       btime_i[3]:     %d\n", btime_i[3]);
		fprintf(stderr, "dbg2       btime_i[4]:     %d\n", btime_i[4]);
		fprintf(stderr, "dbg2       btime_i[5]:     %d\n", btime_i[5]);
		fprintf(stderr, "dbg2       btime_i[6]:     %d\n", btime_i[6]);
		fprintf(stderr, "dbg2       etime_i[0]:     %d\n", etime_i[0]);
		fprintf(stderr, "dbg2       etime_i[1]:     %d\n", etime_i[1]);
		fprintf(stderr, "dbg2       etime_i[2]:     %d\n", etime_i[2]);
		fprintf(stderr, "dbg2       etime_i[3]:     %d\n", etime_i[3]);
		fprintf(stderr, "dbg2       etime_i[4]:     %d\n", etime_i[4]);
		fprintf(stderr, "dbg2       etime_i[5]:     %d\n", etime_i[5]);
		fprintf(stderr, "dbg2       etime_i[6]:     %d\n", etime_i[6]);
		fprintf(stderr, "dbg2       speedmin:       %f\n", speedmin);
		fprintf(stderr, "dbg2       timegap:        %f\n", timegap);
		fprintf(stderr, "dbg2       input file:     %s\n", ifile);
		fprintf(stderr, "dbg2       mode:	   %d\n", mode);
		fprintf(stderr, "dbg2       kluge:	   %d\n", kluge);
	}

	/* if help desired then print it and exit */
	if (help) {
		fprintf(stderr, "\n%s\n", help_message);
		fprintf(stderr, "\nusage: %s\n", usage_message);
		exit(error);
	}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose, ifile, NULL, &format, &error);

	/* initialize reading the input multibeam file */
	if ((status = mb_read_init(verbose, ifile, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &imbio_ptr,
	                           &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error)) != MB_SUCCESS) {
		mb_error(verbose, error, &message);
		fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
		fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", ifile);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* allocate memory for data arrays */
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR) {
		mb_error(verbose, error, &message);
		fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* now deal with new edit save file */
	if (status == MB_SUCCESS) {
		/* get edit save file */
		if (sofile_set == MB_NO) {
			sofp = stdout;
		}
		else if ((sofp = fopen(sofile, "w")) == NULL) {
			error = MB_ERROR_OPEN_FAIL;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nEdit Save File <%s> not initialized for writing\n", sofile);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}
	}

	/* put version header at beginning */
	if (status == MB_SUCCESS) {
		memset(esf_header, 0, MB_PATH_MAXLINE);
		right_now = time((time_t *)0);
		strcpy(date, ctime(&right_now));
		date[strlen(date) - 1] = '\0';
		if ((user_ptr = getenv("USER")) == NULL)
			user_ptr = getenv("LOGNAME");
		if (user_ptr != NULL)
			strcpy(user, user_ptr);
		else
			strcpy(user, "unknown");
		gethostname(host, MBP_FILENAMESIZE);
		if (mode == MBGETESF_IMPLICITBEST) {
			if (format == MBF_3DWISSLR || format == MBF_3DWISSLP) {
				esf_mode = MB_ESF_MODE_IMPLICIT_NULL;
			}
			else {
				esf_mode = MB_ESF_MODE_IMPLICIT_GOOD;
			}
		}
		else if (mode == MBGETESF_IMPLICITNULL)
			esf_mode = MB_ESF_MODE_IMPLICIT_NULL;
		else if (mode == MBGETESF_IMPLICITGOOD)
			esf_mode = MB_ESF_MODE_IMPLICIT_GOOD;
		else
			esf_mode = MB_ESF_MODE_EXPLICIT;
		sprintf(esf_header,
				"ESFVERSION03\nESF Mode: %d\nMB-System Version %s\nProgram: %s\nUser: %s\nCPU: %s\nDate: %s\n",
				esf_mode, MB_VERSION, program_name, user, host, date);
		if (fwrite(esf_header, MB_PATH_MAXLINE, 1, sofp) != 1) {
			status = MB_FAILURE;
			error = MB_ERROR_WRITE_FAIL;
		}
	}

	/* read and write */
	while (error <= MB_ERROR_NO_ERROR) {
		/* read some data */
		error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		status = mb_get_all(verbose, imbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance,
		                    &altitude, &sonardepth, &nbath, &namp, &nss, beamflag, bath, amp, bathacrosstrack, bathalongtrack, ss,
		                    ssacrosstrack, ssalongtrack, comment, &error);
		// fprintf(stderr,"\nMBGETESF READ status:%d: error:%d kind:%d\n",status,error,kind);

		/* increment counter */
		if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA)
			idata = idata + pings;

		/* time gaps do not matter to mbgetesf */
		if (error == MB_ERROR_TIME_GAP) {
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
		}

		/* time bounds do not matter to mbgetesf */
		if (error == MB_ERROR_OUT_TIME) {
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
		}

		/* space bounds do not matter to mbgetesf */
		if (error == MB_ERROR_OUT_BOUNDS) {
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
		}

		/* output error messages */
		if (verbose >= 1 && error < MB_ERROR_NO_ERROR && error >= MB_ERROR_OTHER && error != MB_ERROR_COMMENT) {
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nNonfatal MBIO Error:\n%s\n", message);
			fprintf(stderr, "Input Record: %d\n", idata);
			fprintf(stderr, "Time: %d %d %d %d %d %d %d\n", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5],
			        time_i[6]);
		}
		else if (verbose >= 1 && error < MB_ERROR_NO_ERROR) {
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nNonfatal MBIO Error:\n%s\n", message);
			fprintf(stderr, "Number of good records so far: %d\n", idata);
		}
		else if (verbose >= 1 && error != MB_ERROR_NO_ERROR && error != MB_ERROR_EOF) {
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nFatal MBIO Error:\n%s\n", message);
			fprintf(stderr, "Last Good Time: %d %d %d %d %d %d %d\n", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4],
			        time_i[5], time_i[6]);
		}

		/* deal with data without errors */
		if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
			// fprintf(stderr,"MBGETESF PING %d: time: %f %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d beams:%d\n",
			//						idata,time_d,time_i[0],time_i[1],time_i[2],
			//						time_i[3],time_i[4],time_i[5],time_i[6],
			//						nbath);
			/* fix a problem with EM300/EM3000 data in HDCS format */
			if (format == 151 && kluge == 1) {
				for (i = 0; i < nbath - 1; i++)
					beamflag[i] = beamflag[i + 1];
				beamflag[nbath - 1] = MB_FLAG_FLAG;
			}

			/* count and write the flags */
			for (i = 0; i < nbath; i++) {
				// fprintf(stderr,"MBGETESF: time: %f %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d beam:%d flag:%d   bath:%.3f\n",
				// time_d,time_i[0],time_i[1],time_i[2],
				// time_i[3],time_i[4],time_i[5],time_i[6],
				// i,beamflag[i],bath[i]);
				if (mb_beam_ok(beamflag[i])) {
					beam_ok++;
					if (mode == MBGETESF_ALL
						|| esf_mode == MB_ESF_MODE_IMPLICIT_NULL) {
						mbgetesf_save_edit(verbose, sofp, time_d, i, MBP_EDIT_UNFLAG, &error);
						beam_ok_write++;
					}
				}
				else if (mb_beam_check_flag_unusable(beamflag[i])) {
					beam_null++;
					if (mode == MBGETESF_ALL || mode == MBGETESF_FLAGNULL
						|| esf_mode == MB_ESF_MODE_IMPLICIT_GOOD) {
						mbgetesf_save_edit(verbose, sofp, time_d, i, MBP_EDIT_ZERO, &error);
						beam_null_write++;
					}
				}
				else {
					beam_flag++;
					if (mb_beam_check_flag_manual(beamflag[i])) {
						beam_flag_manual++;
						mbgetesf_save_edit(verbose, sofp, time_d, i, MBP_EDIT_FLAG, &error);
					}
					if (mb_beam_check_flag_filter(beamflag[i])) {
						beam_flag_filter++;
						mbgetesf_save_edit(verbose, sofp, time_d, i, MBP_EDIT_FILTER, &error);
					}
					if (mb_beam_check_flag_sonar(beamflag[i])) {
						beam_flag_sonar++;
						mbgetesf_save_edit(verbose, sofp, time_d, i, MBP_EDIT_SONAR, &error);
					}
				}
			}
		}
	}

	/* close the file */
	status = mb_close(verbose, &imbio_ptr, &error);

	/* close edit save file */
	fclose(sofp);

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose, &error);

	/* give the statistics */
	if (verbose >= 1) {
		if (mode == MBGETESF_FLAGONLY)
			fprintf(stderr, "\nMBgetesf mode: Output beam flags of flagged beams\n");
		else if (mode == MBGETESF_FLAGNULL)
			fprintf(stderr, "\nMBgetesf mode: Output beam flags of flagged and null beams\n");
		else if (mode == MBGETESF_ALL)
			fprintf(stderr, "\nMBgetesf mode: Output beam flags of all beams\n");
		else if (mode == MBGETESF_IMPLICITBEST)
			fprintf(stderr, "\nMBgetesf mode: Output beam flags of flagged and good or null beams with null or good beams implicit (according to format)\n");
		else if (mode == MBGETESF_IMPLICITNULL)
			fprintf(stderr, "\nMBgetesf mode: Output beam flags of flagged and good beams with null beams implicit\n");
		else if (mode == MBGETESF_IMPLICITGOOD)
			fprintf(stderr, "\nMBgetesf mode: Output beam flags of flagged and null beams with good beams implicitbeams\n");
		fprintf(stderr, "\nData records:\n");
		fprintf(stderr, "\t%d input data records\n", idata);
		fprintf(stderr, "\nBeam flag read totals:\n");
		fprintf(stderr, "\t%d beams ok\n", beam_ok);
		fprintf(stderr, "\t%d beams null\n", beam_null);
		fprintf(stderr, "\t%d beams flagged\n", beam_flag);
		fprintf(stderr, "\t\t%d beams flagged manually\n", beam_flag_manual);
		fprintf(stderr, "\t\t%d beams flagged by filter\n", beam_flag_filter);
		fprintf(stderr, "\t\t%d beams flagged by sonar\n", beam_flag_sonar);
		if (esf_mode == MB_ESF_MODE_IMPLICIT_NULL)
			fprintf(stderr, "\nESF mode: implicit NULL beams\n");
		else if (esf_mode == MB_ESF_MODE_IMPLICIT_GOOD)
			fprintf(stderr, "\nESF mode: implicit GOOD beams\n");
		else
			fprintf(stderr, "\nESF mode: no implicit beams\n");
		fprintf(stderr, "Beam flag write totals:\n");
		fprintf(stderr, "\t%d beams ok\n", beam_ok_write);
		fprintf(stderr, "\t%d beams null\n", beam_null_write);
		fprintf(stderr, "\t%d beams flagged\n", beam_flag);
		fprintf(stderr, "\t\t%d beams flagged manually\n", beam_flag_manual);
		fprintf(stderr, "\t\t%d beams flagged by filter\n", beam_flag_filter);
		fprintf(stderr, "\t\t%d beams flagged by sonar\n", beam_flag_sonar);
	}

	exit(error);
}
/*--------------------------------------------------------------------*/

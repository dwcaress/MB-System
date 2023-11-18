/*--------------------------------------------------------------------
 *    The MB-system:	mbgetesf.c	6/15/93
 *
 *    Copyright (c) 2001-2023 by
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

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mb_swap.h"

typedef enum {
    MBGETESF_FLAGONLY = 1,
    MBGETESF_FLAGNULL = 2,
    MBGETESF_ALL = 3,
    MBGETESF_IMPLICITBEST = 4,
    MBGETESF_IMPLICITNULL = 5,
    MBGETESF_IMPLICITGOOD = 6,
} getesf_mode_t;

constexpr char program_name[] = "mbgetesf";
constexpr char help_message[] =
    "mbgetesf reads a multibeam data file and writes out\n"
    "an edit save file which can be applied to other data files\n"
    "containing the same data (but presumably in a different\n"
    "state of processing).  This allows editing of one data file to\n"
    "be transferred to another with ease.  The programs mbedit and\n"
    "mbprocess can be used to apply the edit events to another file.";
constexpr char usage_message[] =
    "mbgetesf [-Fformat -Iinfile -Mmode -Oesffile -V -H]";

/*--------------------------------------------------------------------*/
int mbgetesf_save_edit(int verbose, FILE *sofp, double time_d, int beam, int action, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");

		fprintf(stderr, "dbg2       sofp:            %p\n", (void *)sofp);
		fprintf(stderr, "dbg2       time_d:          %f\n", time_d);
		fprintf(stderr, "dbg2       beam:            %d\n", beam);
		fprintf(stderr, "dbg2       action:          %d\n", action);
	}

	int status = MB_SUCCESS;

	/* write out the edit */
	if (sofp != nullptr) {
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

	getesf_mode_t mode = MBGETESF_FLAGONLY;
	char ifile[MB_PATH_MAXLINE] = "stdin";
	int kluge = 0;
	bool sofile_set = false;
	mb_path sofile = "";

	{
		bool errflg = false;
		int c;
		bool help = false;
		while ((c = getopt(argc, argv, "VvHhB:b:E:F:f:I:i:K:k:M:m:O:o:")) != -1)
		{
			switch (c) {
			case 'H':
			case 'h':
				help = true;
				break;
			case 'V':
			case 'v':
				verbose++;
				break;
			case 'B':
			case 'b':
				sscanf(optarg, "%d/%d/%d/%d/%d/%d", &btime_i[0], &btime_i[1], &btime_i[2], &btime_i[3], &btime_i[4], &btime_i[5]);
				btime_i[6] = 0;
				break;
			case 'E':
			case 'e':
				sscanf(optarg, "%d/%d/%d/%d/%d/%d", &etime_i[0], &etime_i[1], &etime_i[2], &etime_i[3], &etime_i[4], &etime_i[5]);
				etime_i[6] = 0;
				break;
			case 'F':
			case 'f':
				sscanf(optarg, "%d", &format);
				break;
			case 'I':
			case 'i':
				sscanf(optarg, "%1023s", ifile);
				break;
			case 'K':
			case 'k':
				sscanf(optarg, "%d", &kluge);
				break;
			case 'M':
			case 'm':
			{
				int tmp;
				sscanf(optarg, "%d", &tmp);
				mode = (getesf_mode_t)tmp;  // TODO(schwehr): Range check
				break;
			}
			case 'O':
			case 'o':
				sscanf(optarg, "%1023s", sofile);
				sofile_set = true;
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

		if (help) {
			fprintf(stderr, "\n%s\n", help_message);
			fprintf(stderr, "\nusage: %s\n", usage_message);
			exit(MB_ERROR_NO_ERROR);
		}
	}

	int error = MB_ERROR_NO_ERROR;

	if (format == 0)
		mb_get_format(verbose, ifile, nullptr, &format, &error);

	void *imbio_ptr = nullptr;
	double btime_d;
	double etime_d;
	int beams_bath;
	int beams_amp;
	int pixels_ss;

	if (mb_read_init(verbose, ifile, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &imbio_ptr,
	                           &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
		char *message = nullptr;
		mb_error(verbose, error, &message);
		fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
		fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", ifile);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* allocate memory for data arrays */
	char *beamflag = nullptr;
	if (error == MB_ERROR_NO_ERROR)
		status &= mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
	double *bath = nullptr;
	if (error == MB_ERROR_NO_ERROR)
		status &= mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
	double *amp = nullptr;
	if (error == MB_ERROR_NO_ERROR)
		status &= mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
	double *bathacrosstrack = nullptr;
	if (error == MB_ERROR_NO_ERROR)
		status &= mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
	double *bathalongtrack = nullptr;
	if (error == MB_ERROR_NO_ERROR)
		status &= mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
	double *ss = nullptr;
	if (error == MB_ERROR_NO_ERROR)
		status &= mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
	double *ssacrosstrack = nullptr;
	if (error == MB_ERROR_NO_ERROR)
		status &= mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
	double *ssalongtrack = nullptr;
	if (error == MB_ERROR_NO_ERROR)
		status &= mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR) {
		char *message = nullptr;
		mb_error(verbose, error, &message);
		fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* save file control variables */
	FILE *sofp = nullptr;

	/* now deal with new edit save file */
	if (status == MB_SUCCESS) {
		/* get edit save file */
		if (!sofile_set) {
			sofp = stdout;
		}
		else if ((sofp = fopen(sofile, "w")) == nullptr) {
			error = MB_ERROR_OPEN_FAIL;
			char *message = nullptr;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nEdit Save File <%s> not initialized for writing\n", sofile);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}
	}

	mb_path esf_header;
	int esf_mode = MB_ESF_MODE_EXPLICIT;

	/* put version header at beginning */
	if (status == MB_SUCCESS) {
		memset(esf_header, 0, MB_PATH_MAXLINE);
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
    char user[256], host[256], date[32];
    status = mb_user_host_date(verbose, user, host, date, &error);
		snprintf(esf_header, sizeof(esf_header),
				"ESFVERSION03\nESF Mode: %d\nMB-System Version %s\nProgram: %s\nUser: %s\nCPU: %s\nDate: %s\n",
				esf_mode, MB_VERSION, program_name, user, host, date);
		if (fwrite(esf_header, MB_PATH_MAXLINE, 1, sofp) != 1) {
			status = MB_FAILURE;
			error = MB_ERROR_WRITE_FAIL;
		}
	}

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
	double sensordepth;
	int nbath;
	int namp;
	int nss;
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

	/* read and write */
	while (error <= MB_ERROR_NO_ERROR) {
		/* read some data */
		error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		status = mb_get_all(verbose, imbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance,
		                    &altitude, &sensordepth, &nbath, &namp, &nss, beamflag, bath, amp, bathacrosstrack, bathalongtrack, ss,
		                    ssacrosstrack, ssalongtrack, comment, &error);

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
			char *message = nullptr;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nNonfatal MBIO Error:\n%s\n", message);
			fprintf(stderr, "Input Record: %d\n", idata);
			fprintf(stderr, "Time: %d %d %d %d %d %d %d\n", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5],
			        time_i[6]);
		}
		else if (verbose >= 1 && error < MB_ERROR_NO_ERROR) {
			char *message = nullptr;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nNonfatal MBIO Error:\n%s\n", message);
			fprintf(stderr, "Number of good records so far: %d\n", idata);
		}
		else if (verbose >= 1 && error != MB_ERROR_NO_ERROR && error != MB_ERROR_EOF) {
			char *message = nullptr;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nFatal MBIO Error:\n%s\n", message);
			fprintf(stderr, "Last Good Time: %d %d %d %d %d %d %d\n", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4],
			        time_i[5], time_i[6]);
		}

		/* deal with data without errors */
		if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
			/* fix a problem with EM300/EM3000 data in HDCS format */
			if (format == 151 && kluge == 1) {
				for (int i = 0; i < nbath - 1; i++)
					beamflag[i] = beamflag[i + 1];
				beamflag[nbath - 1] = MB_FLAG_FLAG;
			}

			/* count and write the flags */
			for (int i = 0; i < nbath; i++) {
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

/*--------------------------------------------------------------------
 *    The MB-system:	mb_process.c	9/11/00
 *
 *    Copyright (c) 2000-2023 by
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
 * mb_process.c contains functions for reading and writing
 * mbprocess parameter files. The mb_process structure is defined
 * in mb_process.h. A description of mbprocess parameters and
 * parameter file keywords is found in mb_process.h
 *
 * Author:	D. W. Caress
 * Date:	September 11, 2000
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_status.h"

/*--------------------------------------------------------------------*/
int mb_pr_checkstatus(int verbose, char *file, int *prstatus, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:   %d\n", verbose);
		fprintf(stderr, "dbg2       file:      %s\n", file);
	}

	/* get started */
	*prstatus = MB_PR_FILE_NEEDS_PROCESSING;
	*error = MB_ERROR_NO_ERROR;

	/* get existence and get mod time for the input file */
	int ifilemodtime = 0;
	struct stat file_status;
	int fstat = stat(file, &file_status);
	if ((fstat = stat(file, &file_status)) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
		ifilemodtime = file_status.st_mtime;
	}
	else {
		*prstatus = MB_PR_FILE_NOT_EXIST;
	}

	/* check for existing parameter file */
	int pfilemodtime = 0;
	if (*prstatus == MB_PR_FILE_NEEDS_PROCESSING) {
		mb_path mbp_pfile;
		sprintf(mbp_pfile, "%s.par", file);
		if ((fstat = stat(mbp_pfile, &file_status)) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
			pfilemodtime = file_status.st_mtime;
		}
		else {
			*prstatus = MB_PR_NO_PARAMETER_FILE;
		}
	}

	/* if input and parameter files found check output and dependencies */
	if (*prstatus == MB_PR_FILE_NEEDS_PROCESSING) {
		/* read the parameter file */
		struct mb_process_struct process;
		mb_pr_readpar(verbose, file, false, &process, error);

		/* get mod time for the output file */
		int ofilemodtime = 0;
		if ((fstat = stat(process.mbp_ofile, &file_status)) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR)
			ofilemodtime = file_status.st_mtime;

		/* get mod time for the navigation file if needed */
		int navfilemodtime = 0;
		if (process.mbp_nav_mode != MBP_NAV_OFF && (fstat = stat(process.mbp_navfile, &file_status)) == 0 &&
		    (file_status.st_mode & S_IFMT) != S_IFDIR)
			navfilemodtime = file_status.st_mtime;

		/* get mod time for the navigation adjustment file if needed */
		int navadjfilemodtime = 0;
		if (process.mbp_navadj_mode != MBP_NAVADJ_OFF && (fstat = stat(process.mbp_navadjfile, &file_status)) == 0 &&
		    (file_status.st_mode & S_IFMT) != S_IFDIR)
			navadjfilemodtime = file_status.st_mtime;

		/* get mod time for the attitude file if needed */
		int attitudefilemodtime = 0;
		if (process.mbp_attitude_mode != MBP_ATTITUDE_OFF && (fstat = stat(process.mbp_attitudefile, &file_status)) == 0 &&
		    (file_status.st_mode & S_IFMT) != S_IFDIR)
			attitudefilemodtime = file_status.st_mtime;

		/* get mod time for the sensordepth file if needed */
		int sensordepthfilemodtime = 0;
		if (process.mbp_sensordepth_mode != MBP_SENSORDEPTH_OFF && (fstat = stat(process.mbp_sensordepthfile, &file_status)) == 0 &&
		    (file_status.st_mode & S_IFMT) != S_IFDIR)
			sensordepthfilemodtime = file_status.st_mtime;

		/* get mod time for the edit save file if needed */
		int esfmodtime = 0;
		if (process.mbp_edit_mode != MBP_EDIT_OFF && (fstat = stat(process.mbp_editfile, &file_status)) == 0 &&
		    (file_status.st_mode & S_IFMT) != S_IFDIR)
			esfmodtime = file_status.st_mtime;

		/* get mod time for the svp file if needed */
		int svpmodtime = 0;
		if (process.mbp_svp_mode != MBP_SVP_OFF && (fstat = stat(process.mbp_svpfile, &file_status)) == 0 &&
		    (file_status.st_mode & S_IFMT) != S_IFDIR)
			svpmodtime = file_status.st_mtime;

		/* now check if processed file is out of date */
		if (ofilemodtime > 0 && ofilemodtime >= ifilemodtime && ofilemodtime >= pfilemodtime && ofilemodtime >= navfilemodtime &&
		    ofilemodtime >= navadjfilemodtime && ofilemodtime >= attitudefilemodtime && ofilemodtime >= sensordepthfilemodtime &&
		    ofilemodtime >= esfmodtime && ofilemodtime >= svpmodtime) {
			*prstatus = MB_PR_FILE_UP_TO_DATE;
		}
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       prstatus:   %d\n", *prstatus);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_readpar(int verbose, char *file, int lookforfiles, struct mb_process_struct *process, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       file:         %s\n", file);
		fprintf(stderr, "dbg2       lookforfiles: %d\n", lookforfiles);
		fprintf(stderr, "dbg2       process:      %p\n", (void *)process);
	}

	/* get expected process parameter file name */
	char parfile[MBP_FILENAMESIZE];
	strcpy(parfile, file);
	strcat(parfile, ".par");

	/* initialize process parameter structure */

	/* general parameters */
	process->mbp_ifile_specified = false;
	process->mbp_ifile[0] = '\0';
	process->mbp_ofile_specified = false;
	process->mbp_ofile[0] = '\0';
	process->mbp_format_specified = false;
	process->mbp_format = 0;

	/* navigation merging */
	process->mbp_nav_mode = MBP_NAV_OFF;
	process->mbp_navfile[0] = '\0';
	process->mbp_nav_format = 0;
	process->mbp_nav_heading = MBP_NAV_OFF;
	process->mbp_nav_speed = MBP_NAV_OFF;
	process->mbp_nav_draft = MBP_NAV_OFF;
	process->mbp_nav_attitude = MBP_NAV_OFF;
	process->mbp_nav_algorithm = MBP_NAV_LINEAR;
	process->mbp_nav_timeshift = 0.0;
	process->mbp_nav_shift = MBP_NAV_OFF;
	process->mbp_nav_offsetx = 0.0;
	process->mbp_nav_offsety = 0.0;
	process->mbp_nav_offsetz = 0.0;
	process->mbp_nav_shiftlon = 0.0;
	process->mbp_nav_shiftlat = 0.0;
	process->mbp_nav_shiftx = 0.0;
	process->mbp_nav_shifty = 0.0;

	/* adjusted navigation merging */
	process->mbp_navadj_mode = MBP_NAVADJ_OFF;
	process->mbp_navadjfile[0] = '\0';
	process->mbp_navadj_algorithm = MBP_NAV_LINEAR;

	/* attitude merging */
	process->mbp_attitude_mode = 0;
	process->mbp_attitudefile[0] = '\0';
	process->mbp_attitude_format = 1;

	/* sensordepth merging */
	process->mbp_sensordepth_mode = 0;
	process->mbp_sensordepthfile[0] = '\0';
	process->mbp_sensordepth_format = 1;

	/* data cutting */
	process->mbp_cut_num = 0;
	for (int i = 0; i < MBP_CUT_NUM_MAX; i++) {
		process->mbp_cut_kind[i] = MBP_CUT_DATA_BATH;
		process->mbp_cut_mode[i] = MBP_CUT_MODE_NONE;
		process->mbp_cut_min[i] = 0.0;
		process->mbp_cut_max[i] = 0.0;
	}

	/* bathymetry editing */
	process->mbp_edit_mode = MBP_EDIT_OFF;
	process->mbp_editfile[0] = '\0';

	/* bathymetry recalculation */
	process->mbp_bathrecalc_mode = MBP_BATHRECALC_OFF;
	process->mbp_svp_mode = MBP_SVP_OFF;
	process->mbp_svpfile[0] = '\0';
	process->mbp_ssv_mode = MBP_SSV_OFF;
	process->mbp_ssv = 0.0;
	process->mbp_tt_mode = MBP_TT_OFF;
	process->mbp_tt_mult = 1.0;
	process->mbp_angle_mode = MBP_ANGLES_SNELL;
	process->mbp_corrected = true;
	process->mbp_static_mode = MBP_STATIC_OFF;
	process->mbp_staticfile[0] = '\0';

	/* draft correction */
	process->mbp_draft_mode = MBP_DRAFT_OFF;
	process->mbp_draft = 0.0;
	process->mbp_draft_offset = 0.0;
	process->mbp_draft_mult = 1.0;

	/* heave correction */
	process->mbp_heave_mode = MBP_HEAVE_OFF;
	process->mbp_heave = 0.0;
	process->mbp_heave_mult = 1.0;

	/* lever correction */
	process->mbp_lever_mode = MBP_LEVER_OFF;
	process->mbp_vru_offsetx = 0.0;
	process->mbp_vru_offsety = 0.0;
	process->mbp_vru_offsetz = 0.0;
	process->mbp_sonar_offsetx = 0.0;
	process->mbp_sonar_offsety = 0.0;
	process->mbp_sonar_offsetz = 0.0;

	/* roll correction */
	process->mbp_rollbias_mode = MBP_ROLLBIAS_OFF;
	process->mbp_rollbias = 0.0;
	process->mbp_rollbias_port = 0.0;
	process->mbp_rollbias_stbd = 0.0;

	/* pitch correction */
	process->mbp_pitchbias_mode = MBP_PITCHBIAS_OFF;
	process->mbp_pitchbias = 0.0;

	/* heading correction */
	process->mbp_heading_mode = MBP_HEADING_OFF;
	process->mbp_headingbias = 0.0;

	/* tide correction */
	process->mbp_tide_mode = MBP_TIDE_OFF;
	process->mbp_tidefile[0] = '\0';
	process->mbp_tide_format = 1;

	/* amplitude correction */
	process->mbp_ampcorr_mode = MBP_AMPCORR_OFF;
	process->mbp_ampcorrfile[0] = '\0';
	process->mbp_ampcorr_type = MBP_AMPCORR_SUBTRACTION;
	process->mbp_ampcorr_symmetry = MBP_AMPCORR_SYMMETRIC, process->mbp_ampcorr_angle = 30.0;
	process->mbp_ampcorr_slope = MBP_AMPCORR_IGNORESLOPE;

	/* sidescan correction */
	process->mbp_sscorr_mode = MBP_SSCORR_OFF;
	process->mbp_sscorrfile[0] = '\0';
	process->mbp_sscorr_type = MBP_SSCORR_SUBTRACTION;
	process->mbp_sscorr_symmetry = MBP_SSCORR_SYMMETRIC, process->mbp_sscorr_angle = 30.0;
	process->mbp_sscorr_slope = MBP_SSCORR_IGNORESLOPE;

	/* amplitude and sidescan correction */
	process->mbp_ampsscorr_topofile[0] = '\0';

	/* sidescan recalculation */
	process->mbp_ssrecalc_mode = MBP_SSRECALC_OFF;
	process->mbp_ssrecalc_pixelsize = 0.0;
	process->mbp_ssrecalc_swathwidth = 0.0;
	process->mbp_ssrecalc_interpolate = 0;

	/* metadata insertion */
	process->mbp_meta_vessel[0] = '\0';
	process->mbp_meta_institution[0] = '\0';
	process->mbp_meta_platform[0] = '\0';
	process->mbp_meta_sonar[0] = '\0';
	process->mbp_meta_sonarversion[0] = '\0';
	process->mbp_meta_cruiseid[0] = '\0';
	process->mbp_meta_cruisename[0] = '\0';
	process->mbp_meta_pi[0] = '\0';
	process->mbp_meta_piinstitution[0] = '\0';
	process->mbp_meta_client[0] = '\0';
	process->mbp_meta_svcorrected = MBP_CORRECTION_UNKNOWN;
	process->mbp_meta_tidecorrected = MBP_CORRECTION_UNKNOWN;
	process->mbp_meta_batheditmanual = MBP_CORRECTION_UNKNOWN;
	process->mbp_meta_batheditauto = MBP_CORRECTION_UNKNOWN;
	process->mbp_meta_rollbias = MBP_METANOVALUE + 1.;
	process->mbp_meta_pitchbias = MBP_METANOVALUE + 1.;
	process->mbp_meta_headingbias = MBP_METANOVALUE + 1.;
	process->mbp_meta_draft = MBP_METANOVALUE + 1.;

	/* processing kluges */
	process->mbp_kluge001 = false;
	process->mbp_kluge002 = false;
	process->mbp_kluge003 = false;
	process->mbp_kluge004 = false;
	process->mbp_kluge005 = false;
	process->mbp_kluge006 = false;
	process->mbp_kluge007 = false;
	process->mbp_kluge008 = false;
	process->mbp_kluge009 = false;
	process->mbp_kluge010 = false;

	char dummy[MBP_FILENAMESIZE];

	/* open and read parameter file */
	FILE *fp = fopen(parfile, "r");
	if (fp != NULL) {
		bool explicit = false;
		char buffer[MBP_FILENAMESIZE];
		// char *result;
		while (/* result = */ fgets(buffer, MBP_FILENAMESIZE, fp) == buffer) {
			if (buffer[0] != '#') {
				const int len = strlen(buffer);
				if (len > 0) {
					if (buffer[len - 1] == '\n')
						buffer[len - 1] = '\0';
					if (buffer[len - 2] == '\r')
						buffer[len - 2] = '\0';
				}

				/* general parameters */
				if (strncmp(buffer, "EXPLICIT", 8) == 0) {
					explicit = true;
				}
				else if (strncmp(buffer, "INFILE", 6) == 0 && !process->mbp_ifile_specified) {
					sscanf(buffer, "%s %s", dummy, process->mbp_ifile);
					process->mbp_ifile_specified = true;
				}
				else if (strncmp(buffer, "OUTFILE", 7) == 0 && !process->mbp_ofile_specified) {
					sscanf(buffer, "%s %s", dummy, process->mbp_ofile);
					process->mbp_ofile_specified = true;
				}
				else if (strncmp(buffer, "FORMAT", 6) == 0 && !process->mbp_format_specified) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_format);
					process->mbp_format_specified = true;
				}

				/* navigation merging */
				else if (strncmp(buffer, "NAVMODE", 7) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_nav_mode);
				}
				else if (strncmp(buffer, "NAVFILE", 7) == 0) {
					sscanf(buffer, "%s %s", dummy, process->mbp_navfile);
					if (!explicit) {
						process->mbp_nav_mode = MBP_NAV_ON;
						process->mbp_nav_heading = MBP_NAV_ON;
						process->mbp_nav_speed = MBP_NAV_ON;
						process->mbp_nav_draft = MBP_NAV_ON;
						process->mbp_nav_attitude = MBP_NAV_ON;
					}
				}
				else if (strncmp(buffer, "NAVFORMAT", 9) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_nav_format);
				}
				else if (strncmp(buffer, "NAVHEADING", 10) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_nav_heading);
				}
				else if (strncmp(buffer, "NAVSPEED", 8) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_nav_speed);
				}
				else if (strncmp(buffer, "NAVDRAFT", 8) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_nav_draft);
				}
				else if (strncmp(buffer, "NAVATTITUDE", 8) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_nav_attitude);
				}
				else if (strncmp(buffer, "NAVINTERP", 9) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_nav_algorithm);
				}
				else if (strncmp(buffer, "NAVTIMESHIFT", 12) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_nav_timeshift);
				}

				/* navigation offsets and shifts */
				else if (strncmp(buffer, "NAVOFFSETX", 10) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_nav_offsetx);
				}
				else if (strncmp(buffer, "NAVOFFSETY", 10) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_nav_offsety);
				}
				else if (strncmp(buffer, "NAVOFFSETZ", 10) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_nav_offsetz);
				}
				else if (strncmp(buffer, "NAVSHIFTLON", 11) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_nav_shiftlon);
				}
				else if (strncmp(buffer, "NAVSHIFTLAT", 11) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_nav_shiftlat);
				}
				else if (strncmp(buffer, "NAVSHIFTX", 9) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_nav_shiftx);
				}
				else if (strncmp(buffer, "NAVSHIFTY", 9) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_nav_shifty);
				}
				else if (strncmp(buffer, "NAVSHIFT", 8) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_nav_shift);
				}

				/* adjusted navigation merging */
				else if (strncmp(buffer, "NAVADJMODE", 10) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_navadj_mode);
				}
				else if (strncmp(buffer, "NAVADJFILE", 10) == 0) {
					sscanf(buffer, "%s %s", dummy, process->mbp_navadjfile);
					if (!explicit) {
						process->mbp_navadj_mode = MBP_NAVADJ_LLZ;
					}
				}
				else if (strncmp(buffer, "NAVADJINTERP", 12) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_navadj_algorithm);
				}

				/* attitude merging */
				else if (strncmp(buffer, "ATTITUDEMODE", 12) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_attitude_mode);
				}
				else if (strncmp(buffer, "ATTITUDEFILE", 12) == 0) {
					sscanf(buffer, "%s %s", dummy, process->mbp_attitudefile);
					if (!explicit) {
						process->mbp_attitude_mode = MBP_ATTITUDE_ON;
					}
				}
				else if (strncmp(buffer, "ATTITUDEFORMAT", 14) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_attitude_format);
				}

				/* sensordepth merging */
				else if (strncmp(buffer, "sensordepthMODE", 12) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_sensordepth_mode);
				}
				else if (strncmp(buffer, "sensordepthFILE", 12) == 0) {
					sscanf(buffer, "%s %s", dummy, process->mbp_sensordepthfile);
					if (!explicit) {
						process->mbp_sensordepth_mode = MBP_SENSORDEPTH_ON;
					}
				}
				else if (strncmp(buffer, "sensordepthFORMAT", 14) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_sensordepth_format);
				}

				/* data cutting */
				else if (strncmp(buffer, "DATACUTCLEAR", 12) == 0) {
					process->mbp_cut_num = 0;
				}
				else if (strncmp(buffer, "DATACUT", 7) == 0) {
					if (process->mbp_cut_num < MBP_CUT_NUM_MAX) {
						sscanf(buffer, "%s %d %d %lf %lf", dummy, &process->mbp_cut_kind[process->mbp_cut_num],
							&process->mbp_cut_mode[process->mbp_cut_num],
							&process->mbp_cut_min[process->mbp_cut_num],
							&process->mbp_cut_max[process->mbp_cut_num]);
						process->mbp_cut_num++;
					}
				}
				else if (strncmp(buffer, "BATHCUTNUMBER", 13) == 0) {
					if (process->mbp_cut_num < MBP_CUT_NUM_MAX) {
						sscanf(buffer, "%s %lf %lf", dummy, &process->mbp_cut_min[process->mbp_cut_num],
						       &process->mbp_cut_max[process->mbp_cut_num]);
						process->mbp_cut_kind[process->mbp_cut_num] = MBP_CUT_DATA_BATH;
						process->mbp_cut_mode[process->mbp_cut_num] = MBP_CUT_MODE_NUMBER;
						process->mbp_cut_num++;
					}
				}
				else if (strncmp(buffer, "BATHCUTDISTANCE", 15) == 0) {
					if (process->mbp_cut_num < MBP_CUT_NUM_MAX) {
						sscanf(buffer, "%s %lf %lf", dummy, &process->mbp_cut_min[process->mbp_cut_num],
						       &process->mbp_cut_max[process->mbp_cut_num]);
						process->mbp_cut_kind[process->mbp_cut_num] = MBP_CUT_DATA_BATH;
						process->mbp_cut_mode[process->mbp_cut_num] = MBP_CUT_MODE_DISTANCE;
						process->mbp_cut_num++;
					}
				}
				else if (strncmp(buffer, "BATHCUTSPEED", 12) == 0) {
					if (process->mbp_cut_num < MBP_CUT_NUM_MAX) {
						sscanf(buffer, "%s %lf %lf", dummy, &process->mbp_cut_min[process->mbp_cut_num],
						       &process->mbp_cut_max[process->mbp_cut_num]);
						process->mbp_cut_kind[process->mbp_cut_num] = MBP_CUT_DATA_BATH;
						process->mbp_cut_mode[process->mbp_cut_num] = MBP_CUT_MODE_SPEED;
						process->mbp_cut_num++;
					}
				}
				else if (strncmp(buffer, "AMPCUTNUMBER", 12) == 0) {
					if (process->mbp_cut_num < MBP_CUT_NUM_MAX) {
						sscanf(buffer, "%s %lf %lf", dummy, &process->mbp_cut_min[process->mbp_cut_num],
						       &process->mbp_cut_max[process->mbp_cut_num]);
						process->mbp_cut_kind[process->mbp_cut_num] = MBP_CUT_DATA_AMP;
						process->mbp_cut_mode[process->mbp_cut_num] = MBP_CUT_MODE_NUMBER;
						process->mbp_cut_num++;
					}
				}
				else if (strncmp(buffer, "AMPCUTDISTANCE", 14) == 0) {
					if (process->mbp_cut_num < MBP_CUT_NUM_MAX) {
						sscanf(buffer, "%s %lf %lf", dummy, &process->mbp_cut_min[process->mbp_cut_num],
						       &process->mbp_cut_max[process->mbp_cut_num]);
						process->mbp_cut_kind[process->mbp_cut_num] = MBP_CUT_DATA_AMP;
						process->mbp_cut_mode[process->mbp_cut_num] = MBP_CUT_MODE_DISTANCE;
						process->mbp_cut_num++;
					}
				}
				else if (strncmp(buffer, "AMPCUTSPEED", 11) == 0) {
					if (process->mbp_cut_num < MBP_CUT_NUM_MAX) {
						sscanf(buffer, "%s %lf %lf", dummy, &process->mbp_cut_min[process->mbp_cut_num],
						       &process->mbp_cut_max[process->mbp_cut_num]);
						process->mbp_cut_kind[process->mbp_cut_num] = MBP_CUT_DATA_AMP;
						process->mbp_cut_mode[process->mbp_cut_num] = MBP_CUT_MODE_SPEED;
						process->mbp_cut_num++;
					}
				}
				else if (strncmp(buffer, "SSCUTNUMBER", 12) == 0) {
					if (process->mbp_cut_num < MBP_CUT_NUM_MAX) {
						sscanf(buffer, "%s %lf %lf", dummy, &process->mbp_cut_min[process->mbp_cut_num],
						       &process->mbp_cut_max[process->mbp_cut_num]);
						process->mbp_cut_kind[process->mbp_cut_num] = MBP_CUT_DATA_SS;
						process->mbp_cut_mode[process->mbp_cut_num] = MBP_CUT_MODE_NUMBER;
						process->mbp_cut_num++;
					}
				}
				else if (strncmp(buffer, "SSCUTDISTANCE", 14) == 0) {
					if (process->mbp_cut_num < MBP_CUT_NUM_MAX) {
						sscanf(buffer, "%s %lf %lf", dummy, &process->mbp_cut_min[process->mbp_cut_num],
						       &process->mbp_cut_max[process->mbp_cut_num]);
						process->mbp_cut_kind[process->mbp_cut_num] = MBP_CUT_DATA_SS;
						process->mbp_cut_mode[process->mbp_cut_num] = MBP_CUT_MODE_DISTANCE;
						process->mbp_cut_num++;
					}
				}
				else if (strncmp(buffer, "SSCUTSPEED", 10) == 0) {
					if (process->mbp_cut_num < MBP_CUT_NUM_MAX) {
						sscanf(buffer, "%s %lf %lf", dummy, &process->mbp_cut_min[process->mbp_cut_num],
						       &process->mbp_cut_max[process->mbp_cut_num]);
						process->mbp_cut_kind[process->mbp_cut_num] = MBP_CUT_DATA_SS;
						process->mbp_cut_mode[process->mbp_cut_num] = MBP_CUT_MODE_SPEED;
						process->mbp_cut_num++;
					}
				}

				/* bathymetry editing */
				else if (strncmp(buffer, "EDITSAVEMODE", 12) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_edit_mode);
				}
				else if (strncmp(buffer, "EDITSAVEFILE", 12) == 0) {
					sscanf(buffer, "%s %s", dummy, process->mbp_editfile);
					if (!explicit) {
						process->mbp_edit_mode = MBP_EDIT_ON;
					}
				}

				/* bathymetry recalculation */
				else if (strncmp(buffer, "RAYTRACE", 8) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_svp_mode);
				}
				else if (strncmp(buffer, "SVPMODE", 7) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_svp_mode);
				}
				else if (strncmp(buffer, "SVPFILE", 7) == 0) {
					sscanf(buffer, "%s %s", dummy, process->mbp_svpfile);
					if (!explicit) {
						process->mbp_svp_mode = MBP_SVP_ON;
					}
				}
				else if (strncmp(buffer, "SVP", 3) == 0) {
					sscanf(buffer, "%s %s", dummy, process->mbp_svpfile);
					if (!explicit) {
						process->mbp_svp_mode = MBP_SVP_ON;
					}
				}
				else if (strncmp(buffer, "SSVMODE", 7) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_ssv_mode);
				}
				else if (strncmp(buffer, "SSV", 3) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_ssv);
				}
				else if (strncmp(buffer, "TTMODE", 6) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_tt_mode);
				}
				else if (strncmp(buffer, "TTMULTIPLY", 10) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_tt_mult);
				}
				else if (strncmp(buffer, "ANGLEMODE", 9) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_angle_mode);
				}
				else if (strncmp(buffer, "CORRECTED", 9) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_corrected);
				}
				else if (strncmp(buffer, "SOUNDSPEEDREF", 13) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_corrected);
				}

				/* static beam bathymetry correction */
				else if (strncmp(buffer, "STATICMODE", 10) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_static_mode);
				}
				else if (strncmp(buffer, "STATICFILE", 10) == 0) {
					sscanf(buffer, "%s %s", dummy, process->mbp_staticfile);
					if (!explicit) {
						process->mbp_static_mode = MBP_SVP_ON;
					}
				}

				/* draft correction */
				else if (strncmp(buffer, "DRAFTMODE", 9) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_draft_mode);
				}
				else if (strncmp(buffer, "DRAFTOFFSET", 11) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_draft_offset);
				}
				else if (strncmp(buffer, "DRAFTMULTIPLY", 13) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_draft_mult);
				}
				else if (strncmp(buffer, "DRAFT", 5) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_draft);
				}

				/* heave correction */
				else if (strncmp(buffer, "HEAVEMODE", 9) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_heave_mode);
				}
				else if (strncmp(buffer, "HEAVEOFFSET", 11) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_heave);
				}
				else if (strncmp(buffer, "HEAVEMULTIPLY", 13) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_heave_mult);
				}

				/* lever correction */
				else if (strncmp(buffer, "LEVERMODE", 9) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_lever_mode);
				}
				else if (strncmp(buffer, "VRUOFFSETX", 10) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_vru_offsetx);
				}
				else if (strncmp(buffer, "VRUOFFSETY", 10) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_vru_offsety);
				}
				else if (strncmp(buffer, "VRUOFFSETZ", 10) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_vru_offsetz);
				}
				else if (strncmp(buffer, "SONAROFFSETX", 12) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_sonar_offsetx);
				}
				else if (strncmp(buffer, "SONAROFFSETY", 12) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_sonar_offsety);
				}
				else if (strncmp(buffer, "SONAROFFSETZ", 12) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_sonar_offsetz);
				}

				/* roll correction */
				else if (strncmp(buffer, "ROLLBIASMODE", 12) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_rollbias_mode);
				}
				else if (strncmp(buffer, "ROLLBIASPORT", 12) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_rollbias_port);
				}
				else if (strncmp(buffer, "ROLLBIASSTBD", 12) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_rollbias_stbd);
				}
				else if (strncmp(buffer, "ROLLBIAS", 8) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_rollbias);
				}

				/* pitch correction */
				else if (strncmp(buffer, "PITCHBIASMODE", 13) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_pitchbias_mode);
				}
				else if (strncmp(buffer, "PITCHBIAS", 9) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_pitchbias);
				}

				/* heading correction */
				else if (strncmp(buffer, "HEADINGMODE", 11) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_heading_mode);
				}
				else if (strncmp(buffer, "HEADINGOFFSET", 13) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_headingbias);
				}

				/* tide correction */
				else if (strncmp(buffer, "TIDEMODE", 8) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_tide_mode);
				}
				else if (strncmp(buffer, "TIDEFILE", 8) == 0) {
					sscanf(buffer, "%s %s", dummy, process->mbp_tidefile);
					if (!explicit) {
						process->mbp_tide_mode = MBP_TIDE_ON;
					}
				}
				else if (strncmp(buffer, "TIDEFORMAT", 10) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_tide_format);
				}

				/* amplitude correction */
				else if (strncmp(buffer, "AMPCORRMODE", 11) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_ampcorr_mode);
				}
				else if (strncmp(buffer, "AMPCORRFILE", 11) == 0) {
					sscanf(buffer, "%s %s", dummy, process->mbp_ampcorrfile);
					if (!explicit) {
						process->mbp_ampcorr_mode = MBP_AMPCORR_ON;
					}
				}
				else if (strncmp(buffer, "AMPCORRTYPE", 11) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_ampcorr_type);
				}
				else if (strncmp(buffer, "AMPCORRSYMMETRY", 15) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_ampcorr_symmetry);
				}
				else if (strncmp(buffer, "AMPCORRANGLE", 12) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_ampcorr_angle);
				}
				else if (strncmp(buffer, "AMPCORRSLOPE", 12) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_ampcorr_slope);
				}

				/* sidescan correction */
				else if (strncmp(buffer, "SSCORRMODE", 10) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_sscorr_mode);
				}
				else if (strncmp(buffer, "SSCORRFILE", 10) == 0) {
					sscanf(buffer, "%s %s", dummy, process->mbp_sscorrfile);
					if (!explicit) {
						process->mbp_sscorr_mode = MBP_SSCORR_ON;
					}
				}
				else if (strncmp(buffer, "SSCORRTYPE", 10) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_sscorr_type);
				}
				else if (strncmp(buffer, "SSCORRSYMMETRY", 14) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_sscorr_symmetry);
				}
				else if (strncmp(buffer, "SSCORRANGLE", 11) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_sscorr_angle);
				}
				else if (strncmp(buffer, "SSCORRSLOPE", 11) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_sscorr_slope);
				}

				/* amplitude/sidescan topography correction */
				else if (strncmp(buffer, "AMPSSCORRTOPOFILE", 17) == 0) {
					sscanf(buffer, "%s %s", dummy, process->mbp_ampsscorr_topofile);
				}

				/* sidescan recalculation */
				else if (strncmp(buffer, "SSRECALCMODE", 12) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_ssrecalc_mode);
				}
				else if (strncmp(buffer, "SSPIXELSIZE", 11) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_ssrecalc_pixelsize);
				}
				else if (strncmp(buffer, "SSSWATHWIDTH", 11) == 0) {
					sscanf(buffer, "%s %lf", dummy, &process->mbp_ssrecalc_swathwidth);
				}
				else if (strncmp(buffer, "SSINTERPOLATE", 11) == 0) {
					sscanf(buffer, "%s %d", dummy, &process->mbp_ssrecalc_interpolate);
				}

				/* metadata strings */
				else if (strncmp(buffer, "METAVESSEL", 10) == 0) {
					strncpy(process->mbp_meta_vessel, &buffer[11], MBP_FILENAMESIZE);
				}
				else if (strncmp(buffer, "METAINSTITUTION", 15) == 0) {
					strncpy(process->mbp_meta_institution, &buffer[16], MBP_FILENAMESIZE);
				}
				else if (strncmp(buffer, "METAPLATFORM", 12) == 0) {
					strncpy(process->mbp_meta_platform, &buffer[13], MBP_FILENAMESIZE);
				}
				else if (strncmp(buffer, "METASONARVERSION", 16) == 0) {
					strncpy(process->mbp_meta_sonarversion, &buffer[17], MBP_FILENAMESIZE);
				}
				else if (strncmp(buffer, "METASONAR", 9) == 0) {
					strncpy(process->mbp_meta_sonar, &buffer[10], MBP_FILENAMESIZE);
				}
				else if (strncmp(buffer, "METACRUISEID", 12) == 0) {
					strncpy(process->mbp_meta_cruiseid, &buffer[13], MBP_FILENAMESIZE);
				}
				else if (strncmp(buffer, "METACRUISENAME", 14) == 0) {
					strncpy(process->mbp_meta_cruisename, &buffer[15], MBP_FILENAMESIZE);
				}
				else if (strncmp(buffer, "METAPIINSTITUTION", 17) == 0) {
					strncpy(process->mbp_meta_piinstitution, &buffer[18], MBP_FILENAMESIZE);
				}
				else if (strncmp(buffer, "METACLIENT", 10) == 0) {
					strncpy(process->mbp_meta_client, &buffer[11], MBP_FILENAMESIZE);
				}
				else if (strncmp(buffer, "METASVCORRECTED", 15) == 0) {
					sscanf(buffer, "METASVCORRECTED %d", &process->mbp_meta_svcorrected);
				}
				else if (strncmp(buffer, "METATIDECORRECTED", 17) == 0) {
					sscanf(buffer, "METATIDECORRECTED %d", &process->mbp_meta_tidecorrected);
				}
				else if (strncmp(buffer, "METABATHEDITMANUAL", 18) == 0) {
					sscanf(buffer, "METABATHEDITMANUAL %d", &process->mbp_meta_batheditmanual);
				}
				else if (strncmp(buffer, "METABATHEDITAUTO", 16) == 0) {
					sscanf(buffer, "METABATHEDITAUTO %d", &process->mbp_meta_batheditauto);
				}
				else if (strncmp(buffer, "METAROLLBIAS", 12) == 0) {
					sscanf(buffer, "METAROLLBIAS %lf", &process->mbp_meta_rollbias);
				}
				else if (strncmp(buffer, "METAPITCHBIAS", 13) == 0) {
					sscanf(buffer, "METAPITCHBIAS %lf", &process->mbp_meta_pitchbias);
				}
				else if (strncmp(buffer, "METAPI", 6) == 0) {
					strncpy(process->mbp_meta_pi, &buffer[7], MBP_FILENAMESIZE);
				}
				else if (strncmp(buffer, "METAHEADINGBIAS", 15) == 0) {
					sscanf(buffer, "METAHEADINGBIAS %lf", &process->mbp_meta_headingbias);
				}
				else if (strncmp(buffer, "METADRAFT", 9) == 0) {
					sscanf(buffer, "METADRAFT %lf", &process->mbp_meta_draft);
				}

				/* processing kluges */
				else if (strncmp(buffer, "KLUGE001", 8) == 0) {
					process->mbp_kluge001 = true;
				}
				else if (strncmp(buffer, "KLUGE002", 8) == 0) {
					process->mbp_kluge002 = true;
				}
				else if (strncmp(buffer, "KLUGE003", 8) == 0) {
					process->mbp_kluge003 = true;
				}
				else if (strncmp(buffer, "KLUGE004", 8) == 0) {
					process->mbp_kluge004 = true;
				}
				else if (strncmp(buffer, "KLUGE005", 8) == 0) {
					process->mbp_kluge005 = true;
				}
				else if (strncmp(buffer, "KLUGE006", 8) == 0) {
					process->mbp_kluge006 = true;
				}
				else if (strncmp(buffer, "KLUGE007", 8) == 0) {
					process->mbp_kluge007 = true;
				}
				else if (strncmp(buffer, "KLUGE008", 8) == 0) {
					process->mbp_kluge008 = true;
				}
				else if (strncmp(buffer, "KLUGE009", 8) == 0) {
					process->mbp_kluge009 = true;
				}
				else if (strncmp(buffer, "KLUGE010", 8) == 0) {
					process->mbp_kluge010 = true;
				}
			}
		}

		fclose(fp);
	}

	/* Now make input file global if local */
	process->mbp_ifile_specified = true;
	if (file[0] != '/' && file[1] != ':') {
    char *getcwd_result = getcwd(process->mbp_ifile, MB_PATH_MAXLINE);
    assert(strlen(process->mbp_ifile) > 0);
    assert(getcwd_result != NULL);
		strcat(process->mbp_ifile, "/");
		strcat(process->mbp_ifile, file);
	} else {
		strcpy(process->mbp_ifile, file);
	}
	mb_get_shortest_path(verbose, process->mbp_ifile, error);
  	assert(strlen(process->mbp_ifile) < MB_PATH_MAXLINE - 8);

	/* figure out data format or output filename if required */
	if (!process->mbp_format_specified || !process->mbp_ofile_specified) {
		mb_pr_default_output(verbose, process, error);
	}

	/* Make output file global if local */
	if (process->mbp_ofile[0] != '/' && process->mbp_ofile[1] != ':') {
		char *lastslash = strrchr(process->mbp_ifile, '/');
		if (lastslash != NULL) {
			strcpy(dummy, process->mbp_ofile);
			strcpy(process->mbp_ofile, process->mbp_ifile);
			process->mbp_ofile[strlen(process->mbp_ifile) - strlen(lastslash)] = '\0';
			strcat(process->mbp_ofile, "/");
			strcat(process->mbp_ofile, dummy);
		}
	}

	/* look for nav and other bath edit files if not specified */
	if (lookforfiles == 1 || lookforfiles == 2) {
		/* look for navadj file */
		if (process->mbp_navadj_mode == MBP_NAVADJ_OFF) {
			for (int i = 9; i >= 0 && process->mbp_navadj_mode == MBP_NAVADJ_OFF; i--) {
				sprintf(process->mbp_navadjfile, "%s.na%d", process->mbp_ifile, i);
				struct stat statbuf;
				if (stat(process->mbp_navadjfile, &statbuf) == 0) {
					process->mbp_navadj_mode = MBP_NAVADJ_LLZ;
				}
			}
			if (process->mbp_navadj_mode == MBP_NAVADJ_OFF) {
				process->mbp_navadjfile[0] = '\0';
			}
		}

		/* look for nav file */
		if (process->mbp_nav_mode == MBP_NAV_OFF) {
			strcpy(process->mbp_navfile, process->mbp_ifile);
			strcat(process->mbp_navfile, ".nve");
			struct stat statbuf;
			if (stat(process->mbp_navfile, &statbuf) == 0) {
				process->mbp_nav_mode = MBP_NAV_ON;
				process->mbp_nav_format = 9;
			}
			else {
				process->mbp_navfile[0] = '\0';
			}
		}

		/* look for edit file */
		if (process->mbp_edit_mode == MBP_EDIT_OFF) {
			strcpy(process->mbp_editfile, process->mbp_ifile);
			strcat(process->mbp_editfile, ".esf");
			struct stat statbuf;
			if (stat(process->mbp_editfile, &statbuf) == 0) {
				process->mbp_edit_mode = MBP_EDIT_ON;
			} else {
				strcpy(process->mbp_editfile, process->mbp_ifile);
				strcat(process->mbp_editfile, ".mbesf");
				if (stat(process->mbp_editfile, &statbuf) == 0) {
					process->mbp_edit_mode = MBP_EDIT_ON;
				} else {
					process->mbp_editfile[0] = '\0';
				}
			}
		}
	}

	/* look for svp files if not specified */
	if (lookforfiles == 2) {
		/* look for svp file */
		if (process->mbp_svp_mode == MBP_SVP_OFF) {
			strcpy(process->mbp_svpfile, process->mbp_ifile);
			strcat(process->mbp_svpfile, ".svp");
			struct stat statbuf;
			if (stat(process->mbp_svpfile, &statbuf) == 0) {
				process->mbp_svp_mode = MBP_SVP_ON;
			} else {
				strcpy(process->mbp_svpfile, process->mbp_ifile);
				strcat(process->mbp_svpfile, "_001.svp");
				if (stat(process->mbp_svpfile, &statbuf) == 0) {
					process->mbp_svp_mode = MBP_SVP_ON;
				} else {
					process->mbp_svpfile[0] = '\0';
				}
			}
		}
	}

	/* reset all output files to local path if possible */
	if (lookforfiles > 2) {
		/* reset output file */
		process->mbp_ofile_specified = false;
		mb_pr_default_output(verbose, process, error);

		/* reset navadj file */
		char *lastslash;
		if ((lastslash = strrchr(process->mbp_navadjfile, '/')) != NULL && strlen(lastslash) > 1) {
			strcpy(dummy, &(lastslash[1]));
			strcpy(process->mbp_navadjfile, dummy);
		}

		/* reset nav file */
		if ((lastslash = strrchr(process->mbp_navfile, '/')) != NULL && strlen(lastslash) > 1) {
			strcpy(dummy, &(lastslash[1]));
			strcpy(process->mbp_navfile, dummy);
		}

		/* reset edit file */
		if ((lastslash = strrchr(process->mbp_editfile, '/')) != NULL && strlen(lastslash) > 1) {
			strcpy(dummy, &(lastslash[1]));
			strcpy(process->mbp_editfile, dummy);
		}

		/* reset static file */
		if ((lastslash = strrchr(process->mbp_staticfile, '/')) != NULL && strlen(lastslash) > 1) {
			strcpy(dummy, &(lastslash[1]));
			strcpy(process->mbp_staticfile, dummy);
		}

		/* reset attitude file */
		if ((lastslash = strrchr(process->mbp_attitudefile, '/')) != NULL && strlen(lastslash) > 1) {
			strcpy(dummy, &(lastslash[1]));
			strcpy(process->mbp_attitudefile, dummy);
		}

		/* reset sensordepth file */
		if ((lastslash = strrchr(process->mbp_sensordepthfile, '/')) != NULL && strlen(lastslash) > 1) {
			strcpy(dummy, &(lastslash[1]));
			strcpy(process->mbp_sensordepthfile, dummy);
		}

		/* reset tide file */
		if ((lastslash = strrchr(process->mbp_tidefile, '/')) != NULL && strlen(lastslash) > 1) {
			strcpy(dummy, &(lastslash[1]));
			strcpy(process->mbp_tidefile, dummy);
		}

		/* reset ampcorr file */
		if ((lastslash = strrchr(process->mbp_ampcorrfile, '/')) != NULL && strlen(lastslash) > 1) {
			strcpy(dummy, &(lastslash[1]));
			strcpy(process->mbp_ampcorrfile, dummy);
		}

		/* reset sscorr file */
		if ((lastslash = strrchr(process->mbp_sscorrfile, '/')) != NULL && strlen(lastslash) > 1) {
			strcpy(dummy, &(lastslash[1]));
			strcpy(process->mbp_sscorrfile, dummy);
		}

		/* reset mbp_ampsscorr_topo file */
		if ((lastslash = strrchr(process->mbp_ampsscorr_topofile, '/')) != NULL && strlen(lastslash) > 1) {
			strcpy(dummy, &(lastslash[1]));
			strcpy(process->mbp_ampsscorr_topofile, dummy);
		}
	}

	/* Now make filenames global if local */
	char *lastslash = strrchr(process->mbp_ifile, '/');
	const int len = lastslash - process->mbp_ifile + 1;

	/* reset navadj file */
	if (len > 1 && strlen(process->mbp_navadjfile) > 1 && process->mbp_navadjfile[0] != '/' &&
	    process->mbp_navadjfile[1] != ':') {
		strcpy(dummy, process->mbp_navadjfile);
		strncpy(process->mbp_navadjfile, process->mbp_ifile, len);
		process->mbp_navadjfile[len] = '\0';
		strcat(process->mbp_navadjfile, dummy);
	}

	/* reset nav file */
	if (len > 1 && strlen(process->mbp_navfile) > 1 && process->mbp_navfile[0] != '/' && process->mbp_navfile[1] != ':') {
		strcpy(dummy, process->mbp_navfile);
		strncpy(process->mbp_navfile, process->mbp_ifile, len);
		process->mbp_navfile[len] = '\0';
		strcat(process->mbp_navfile, dummy);
	}

	/* reset attitude file */
	if (len > 1 && strlen(process->mbp_attitudefile) > 1 && process->mbp_attitudefile[0] != '/' &&
	    process->mbp_attitudefile[1] != ':') {
		strcpy(dummy, process->mbp_attitudefile);
		strncpy(process->mbp_attitudefile, process->mbp_ifile, len);
		process->mbp_attitudefile[len] = '\0';
		strcat(process->mbp_attitudefile, dummy);
	}

	/* reset sensordepth file */
	if (len > 1 && strlen(process->mbp_sensordepthfile) > 1 && process->mbp_sensordepthfile[0] != '/' &&
	    process->mbp_sensordepthfile[1] != ':') {
		strcpy(dummy, process->mbp_sensordepthfile);
		strncpy(process->mbp_sensordepthfile, process->mbp_ifile, len);
		process->mbp_sensordepthfile[len] = '\0';
		strcat(process->mbp_sensordepthfile, dummy);
	}

	/* reset svp file */
	if (len > 1 && strlen(process->mbp_svpfile) > 1 && process->mbp_svpfile[0] != '/' && process->mbp_svpfile[1] != ':') {
		strcpy(dummy, process->mbp_svpfile);
		strncpy(process->mbp_svpfile, process->mbp_ifile, len);
		process->mbp_svpfile[len] = '\0';
		strcat(process->mbp_svpfile, dummy);
	}

	/* reset edit file */
	if (len > 1 && strlen(process->mbp_editfile) > 1 && process->mbp_editfile[0] != '/' && process->mbp_editfile[1] != ':') {
		strcpy(dummy, process->mbp_editfile);
		strncpy(process->mbp_editfile, process->mbp_ifile, len);
		process->mbp_editfile[len] = '\0';
		strcat(process->mbp_editfile, dummy);
	}

	/* reset static file */
	if (len > 1 && strlen(process->mbp_staticfile) > 1 && process->mbp_staticfile[0] != '/' &&
	    process->mbp_staticfile[1] != ':') {
		strcpy(dummy, process->mbp_staticfile);
		strncpy(process->mbp_staticfile, process->mbp_ifile, len);
		process->mbp_staticfile[len] = '\0';
		strcat(process->mbp_staticfile, dummy);
	}

	/* reset tide file */
	if (len > 1 && strlen(process->mbp_tidefile) > 1 && process->mbp_tidefile[0] != '/' && process->mbp_tidefile[1] != ':') {
		strcpy(dummy, process->mbp_tidefile);
		strncpy(process->mbp_tidefile, process->mbp_ifile, len);
		process->mbp_tidefile[len] = '\0';
		strcat(process->mbp_tidefile, dummy);
	}

	/* reset amplitude correction file */
	if (len > 1 && strlen(process->mbp_ampcorrfile) > 1 && process->mbp_ampcorrfile[0] != '/' &&
	    process->mbp_ampcorrfile[1] != ':') {
		strcpy(dummy, process->mbp_ampcorrfile);
		strncpy(process->mbp_ampcorrfile, process->mbp_ifile, len);
		process->mbp_ampcorrfile[len] = '\0';
		strcat(process->mbp_ampcorrfile, dummy);
	}

	/* reset sidescan correction file */
	if (len > 1 && strlen(process->mbp_sscorrfile) > 1 && process->mbp_sscorrfile[0] != '/' &&
	    process->mbp_sscorrfile[1] != ':') {
		strcpy(dummy, process->mbp_sscorrfile);
		strncpy(process->mbp_sscorrfile, process->mbp_ifile, len);
		process->mbp_sscorrfile[len] = '\0';
		strcat(process->mbp_sscorrfile, dummy);
	}

	/* reset mbp_ampsscorr_topo file */
	if (len > 1 && strlen(process->mbp_ampsscorr_topofile) > 1 && process->mbp_ampsscorr_topofile[0] != '/' &&
	    process->mbp_ampsscorr_topofile[1] != ':') {
		strcpy(dummy, process->mbp_ampsscorr_topofile);
		strncpy(process->mbp_ampsscorr_topofile, process->mbp_ifile, len);
		process->mbp_ampsscorr_topofile[len] = '\0';
		strcat(process->mbp_ampsscorr_topofile, dummy);
	}

	/* make sure all global paths are as short as possible */
	mb_get_shortest_path(verbose, process->mbp_navadjfile, error);
	mb_get_shortest_path(verbose, process->mbp_navfile, error);
	mb_get_shortest_path(verbose, process->mbp_attitudefile, error);
	mb_get_shortest_path(verbose, process->mbp_sensordepthfile, error);
	mb_get_shortest_path(verbose, process->mbp_svpfile, error);
	mb_get_shortest_path(verbose, process->mbp_editfile, error);
	mb_get_shortest_path(verbose, process->mbp_staticfile, error);
	mb_get_shortest_path(verbose, process->mbp_tidefile, error);
	mb_get_shortest_path(verbose, process->mbp_ampcorrfile, error);
	mb_get_shortest_path(verbose, process->mbp_sscorrfile, error);
	mb_get_shortest_path(verbose, process->mbp_ampsscorr_topofile, error);

	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, process, error);

	int status = MB_SUCCESS;

	/* check for error */
	if (!process->mbp_ifile_specified || !process->mbp_ofile_specified ||
	    !process->mbp_format_specified) {
		status = MB_FAILURE;
		*error = MB_ERROR_OPEN_FAIL;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_ifile_specified:    %d\n", process->mbp_ifile_specified);
		fprintf(stderr, "dbg2       mbp_ifile:              %s\n", process->mbp_ifile);
		fprintf(stderr, "dbg2       mbp_ofile_specified:    %d\n", process->mbp_ofile_specified);
		fprintf(stderr, "dbg2       mbp_ofile:              %s\n", process->mbp_ofile);
		fprintf(stderr, "dbg2       mbp_format_specified:   %d\n", process->mbp_format_specified);
		fprintf(stderr, "dbg2       mbp_format:             %d\n", process->mbp_format);
		fprintf(stderr, "dbg2       mbp_nav_mode:           %d\n", process->mbp_nav_mode);
		fprintf(stderr, "dbg2       mbp_navfile:            %s\n", process->mbp_navfile);
		fprintf(stderr, "dbg2       mbp_nav_format:         %d\n", process->mbp_nav_format);
		fprintf(stderr, "dbg2       mbp_nav_heading:        %d\n", process->mbp_nav_heading);
		fprintf(stderr, "dbg2       mbp_nav_speed:          %d\n", process->mbp_nav_speed);
		fprintf(stderr, "dbg2       mbp_nav_draft:          %d\n", process->mbp_nav_draft);
		fprintf(stderr, "dbg2       mbp_nav_attitude:       %d\n", process->mbp_nav_attitude);
		fprintf(stderr, "dbg2       mbp_nav_algorithm:      %d\n", process->mbp_nav_algorithm);
		fprintf(stderr, "dbg2       mbp_nav_timeshift:      %f\n", process->mbp_nav_timeshift);
		fprintf(stderr, "dbg2       mbp_nav_shift:          %d\n", process->mbp_nav_shift);
		fprintf(stderr, "dbg2       mbp_nav_offsetx:        %f\n", process->mbp_nav_offsetx);
		fprintf(stderr, "dbg2       mbp_nav_offsety:        %f\n", process->mbp_nav_offsety);
		fprintf(stderr, "dbg2       mbp_nav_offsetz:        %f\n", process->mbp_nav_offsetz);
		fprintf(stderr, "dbg2       mbp_nav_shiftlon:       %f\n", process->mbp_nav_shiftlon);
		fprintf(stderr, "dbg2       mbp_nav_shiftlat:       %f\n", process->mbp_nav_shiftlat);
		fprintf(stderr, "dbg2       mbp_nav_shiftx:         %f\n", process->mbp_nav_shiftx);
		fprintf(stderr, "dbg2       mbp_nav_shifty:         %f\n", process->mbp_nav_shifty);
		fprintf(stderr, "dbg2       mbp_navadj_mode:        %d\n", process->mbp_navadj_mode);
		fprintf(stderr, "dbg2       mbp_navadjfile:         %s\n", process->mbp_navadjfile);
		fprintf(stderr, "dbg2       mbp_navadj_algorithm:   %d\n", process->mbp_navadj_algorithm);
		fprintf(stderr, "dbg2       mbp_attitude_mode:      %d\n", process->mbp_attitude_mode);
		fprintf(stderr, "dbg2       mbp_attitudefile:       %s\n", process->mbp_attitudefile);
		fprintf(stderr, "dbg2       mbp_attitude_format:    %d\n", process->mbp_attitude_format);
		fprintf(stderr, "dbg2       mbp_sensordepth_mode:    %d\n", process->mbp_sensordepth_mode);
		fprintf(stderr, "dbg2       mbp_sensordepthfile:     %s\n", process->mbp_sensordepthfile);
		fprintf(stderr, "dbg2       mbp_sensordepth_format:  %d\n", process->mbp_sensordepth_format);
		fprintf(stderr, "dbg2       mbp_cut_num:            %d\n", process->mbp_cut_num);
		for (int i = 0; i < process->mbp_cut_num; i++) {
			fprintf(stderr, "dbg2           cut %d:\n", i);
			fprintf(stderr, "dbg2           mbp_cut_kind[%d]:     %d\n", i, process->mbp_cut_kind[i]);
			fprintf(stderr, "dbg2           mbp_cut_mode[%d]:     %d\n", i, process->mbp_cut_mode[i]);
			fprintf(stderr, "dbg2           mbp_cut_min[%d]:      %f\n", i, process->mbp_cut_min[i]);
			fprintf(stderr, "dbg2           mbp_cut_max[%d]:      %f\n", i, process->mbp_cut_max[i]);
		}
		fprintf(stderr, "dbg2       mbp_bathrecalc_mode:    %d\n", process->mbp_bathrecalc_mode);
		fprintf(stderr, "dbg2       mbp_rollbias_mode:      %d\n", process->mbp_rollbias_mode);
		fprintf(stderr, "dbg2       mbp_rollbias:           %f\n", process->mbp_rollbias);
		fprintf(stderr, "dbg2       mbp_rollbias_port:      %f\n", process->mbp_rollbias_port);
		fprintf(stderr, "dbg2       mbp_rollbias_stbd:      %f\n", process->mbp_rollbias_stbd);
		fprintf(stderr, "dbg2       mbp_pitchbias_mode:     %d\n", process->mbp_pitchbias_mode);
		fprintf(stderr, "dbg2       mbp_pitchbias:          %f\n", process->mbp_pitchbias);
		fprintf(stderr, "dbg2       mbp_draft_mode:         %d\n", process->mbp_draft_mode);
		fprintf(stderr, "dbg2       mbp_draft:              %f\n", process->mbp_draft);
		fprintf(stderr, "dbg2       mbp_draft_offset:       %f\n", process->mbp_draft_offset);
		fprintf(stderr, "dbg2       mbp_draft_mult:         %f\n", process->mbp_draft_mult);
		fprintf(stderr, "dbg2       mbp_heave_mode:         %d\n", process->mbp_heave_mode);
		fprintf(stderr, "dbg2       mbp_heave:              %f\n", process->mbp_heave);
		fprintf(stderr, "dbg2       mbp_heave_mult:         %f\n", process->mbp_heave_mult);
		fprintf(stderr, "dbg2       mbp_lever_mode:         %d\n", process->mbp_heave_mode);
		fprintf(stderr, "dbg2       mbp_vru_offsetx:        %f\n", process->mbp_vru_offsetx);
		fprintf(stderr, "dbg2       mbp_vru_offsety:        %f\n", process->mbp_vru_offsety);
		fprintf(stderr, "dbg2       mbp_vru_offsetz:        %f\n", process->mbp_vru_offsetz);
		fprintf(stderr, "dbg2       mbp_sonar_offsetx:      %f\n", process->mbp_sonar_offsetx);
		fprintf(stderr, "dbg2       mbp_sonar_offsety:      %f\n", process->mbp_sonar_offsety);
		fprintf(stderr, "dbg2       mbp_sonar_offsetz:      %f\n", process->mbp_sonar_offsetz);
		fprintf(stderr, "dbg2       mbp_ssv_mode:           %d\n", process->mbp_ssv_mode);
		fprintf(stderr, "dbg2       mbp_ssv:                %f\n", process->mbp_ssv);
		fprintf(stderr, "dbg2       mbp_svp_mode:           %d\n", process->mbp_svp_mode);
		fprintf(stderr, "dbg2       mbp_svpfile:            %s\n", process->mbp_svpfile);
		fprintf(stderr, "dbg2       mbp_corrected:          %d\n", process->mbp_corrected);
		fprintf(stderr, "dbg2       mbp_tt_mode:            %d\n", process->mbp_tt_mode);
		fprintf(stderr, "dbg2       mbp_tt_mult:            %f\n", process->mbp_tt_mult);
		fprintf(stderr, "dbg2       mbp_angle_mode:         %d\n", process->mbp_angle_mode);
		fprintf(stderr, "dbg2       mbp_static_mode:        %d\n", process->mbp_static_mode);
		fprintf(stderr, "dbg2       mbp_staticfile:         %s\n", process->mbp_staticfile);
		fprintf(stderr, "dbg2       mbp_heading_mode:       %d\n", process->mbp_heading_mode);
		fprintf(stderr, "dbg2       mbp_headingbias:        %f\n", process->mbp_headingbias);
		fprintf(stderr, "dbg2       mbp_edit_mode:          %d\n", process->mbp_edit_mode);
		fprintf(stderr, "dbg2       mbp_editfile:           %s\n", process->mbp_editfile);
		fprintf(stderr, "dbg2       mbp_tide_mode:          %d\n", process->mbp_tide_mode);
		fprintf(stderr, "dbg2       mbp_tidefile:           %s\n", process->mbp_tidefile);
		fprintf(stderr, "dbg2       mbp_tide_format:        %d\n", process->mbp_tide_format);
		fprintf(stderr, "dbg2       mbp_ampcorr_mode:       %d\n", process->mbp_ampcorr_mode);
		fprintf(stderr, "dbg2       mbp_ampcorrfile:        %s\n", process->mbp_ampcorrfile);
		fprintf(stderr, "dbg2       mbp_ampcorr_type:       %d\n", process->mbp_ampcorr_type);
		fprintf(stderr, "dbg2       mbp_ampcorr_symmetry:   %d\n", process->mbp_ampcorr_symmetry);
		fprintf(stderr, "dbg2       mbp_ampcorr_angle:      %f\n", process->mbp_ampcorr_angle);
		fprintf(stderr, "dbg2       mbp_ampcorr_slope:      %d\n", process->mbp_ampcorr_slope);
		fprintf(stderr, "dbg2       mbp_sscorr_mode:        %d\n", process->mbp_sscorr_mode);
		fprintf(stderr, "dbg2       mbp_sscorrfile:         %s\n", process->mbp_sscorrfile);
		fprintf(stderr, "dbg2       mbp_sscorr_type:        %d\n", process->mbp_sscorr_type);
		fprintf(stderr, "dbg2       mbp_sscorr_symmetry:    %d\n", process->mbp_sscorr_symmetry);
		fprintf(stderr, "dbg2       mbp_sscorr_angle:       %f\n", process->mbp_sscorr_angle);
		fprintf(stderr, "dbg2       mbp_sscorr_slope:       %d\n", process->mbp_sscorr_slope);
		fprintf(stderr, "dbg2       mbp_ampsscorr_topofile: %s\n", process->mbp_ampsscorr_topofile);
		fprintf(stderr, "dbg2       mbp_ssrecalc_mode:      %d\n", process->mbp_ssrecalc_mode);
		fprintf(stderr, "dbg2       mbp_ssrecalc_pixelsize: %f\n", process->mbp_ssrecalc_pixelsize);
		fprintf(stderr, "dbg2       mbp_ssrecalc_swathwidth:%f\n", process->mbp_ssrecalc_swathwidth);
		fprintf(stderr, "dbg2       mbp_ssrecalc_interp    :%d\n", process->mbp_ssrecalc_interpolate);
		fprintf(stderr, "dbg2       mbp_meta_vessel        :%s\n", process->mbp_meta_vessel);
		fprintf(stderr, "dbg2       mbp_meta_institution   :%s\n", process->mbp_meta_institution);
		fprintf(stderr, "dbg2       mbp_meta_platform      :%s\n", process->mbp_meta_platform);
		fprintf(stderr, "dbg2       mbp_meta_sonar         :%s\n", process->mbp_meta_sonar);
		fprintf(stderr, "dbg2       mbp_meta_sonarversion  :%s\n", process->mbp_meta_sonarversion);
		fprintf(stderr, "dbg2       mbp_meta_cruiseid      :%s\n", process->mbp_meta_cruiseid);
		fprintf(stderr, "dbg2       mbp_meta_cruisename    :%s\n", process->mbp_meta_cruisename);
		fprintf(stderr, "dbg2       mbp_meta_pi            :%s\n", process->mbp_meta_pi);
		fprintf(stderr, "dbg2       mbp_meta_piinstitution :%s\n", process->mbp_meta_piinstitution);
		fprintf(stderr, "dbg2       mbp_meta_client        :%s\n", process->mbp_meta_client);
		fprintf(stderr, "dbg2       mbp_meta_svcorrected   :%d\n", process->mbp_meta_svcorrected);
		fprintf(stderr, "dbg2       mbp_meta_tidecorrected :%d\n", process->mbp_meta_tidecorrected);
		fprintf(stderr, "dbg2       mbp_meta_batheditmanual:%d\n", process->mbp_meta_batheditmanual);
		fprintf(stderr, "dbg2       mbp_meta_batheditauto:  %d\n", process->mbp_meta_batheditauto);
		fprintf(stderr, "dbg2       mbp_meta_rollbias:      %f\n", process->mbp_meta_rollbias);
		fprintf(stderr, "dbg2       mbp_meta_pitchbias:     %f\n", process->mbp_meta_pitchbias);
		fprintf(stderr, "dbg2       mbp_meta_headingbias:   %f\n", process->mbp_meta_headingbias);
		fprintf(stderr, "dbg2       mbp_meta_draft:         %f\n", process->mbp_meta_draft);
		fprintf(stderr, "dbg2       mbp_kluge001:           %d\n", process->mbp_kluge001);
		fprintf(stderr, "dbg2       mbp_kluge002:           %d\n", process->mbp_kluge002);
		fprintf(stderr, "dbg2       mbp_kluge003:           %d\n", process->mbp_kluge003);
		fprintf(stderr, "dbg2       mbp_kluge004:           %d\n", process->mbp_kluge004);
		fprintf(stderr, "dbg2       mbp_kluge005:           %d\n", process->mbp_kluge005);
		fprintf(stderr, "dbg2       mbp_kluge006:           %d\n", process->mbp_kluge006);
		fprintf(stderr, "dbg2       mbp_kluge007:           %d\n", process->mbp_kluge007);
		fprintf(stderr, "dbg2       mbp_kluge008:           %d\n", process->mbp_kluge008);
		fprintf(stderr, "dbg2       mbp_kluge009:           %d\n", process->mbp_kluge009);
		fprintf(stderr, "dbg2       mbp_kluge010:           %d\n", process->mbp_kluge010);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_writepar(int verbose, char *file, struct mb_process_struct *process, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                %d\n", verbose);
		fprintf(stderr, "dbg2       file:                   %s\n", file);
		fprintf(stderr, "dbg2       process:                %p\n", (void *)process);
		fprintf(stderr, "dbg2       mbp_ifile_specified:    %d\n", process->mbp_ifile_specified);
		fprintf(stderr, "dbg2       mbp_ifile:              %s\n", process->mbp_ifile);
		fprintf(stderr, "dbg2       mbp_ofile_specified:    %d\n", process->mbp_ofile_specified);
		fprintf(stderr, "dbg2       mbp_ofile:              %s\n", process->mbp_ofile);
		fprintf(stderr, "dbg2       mbp_format_specified:   %d\n", process->mbp_format_specified);
		fprintf(stderr, "dbg2       mbp_format:             %d\n", process->mbp_format);
		fprintf(stderr, "dbg2       mbp_nav_mode:           %d\n", process->mbp_nav_mode);
		fprintf(stderr, "dbg2       mbp_navfile:            %s\n", process->mbp_navfile);
		fprintf(stderr, "dbg2       mbp_nav_format:         %d\n", process->mbp_nav_format);
		fprintf(stderr, "dbg2       mbp_nav_heading:        %d\n", process->mbp_nav_heading);
		fprintf(stderr, "dbg2       mbp_nav_speed:          %d\n", process->mbp_nav_speed);
		fprintf(stderr, "dbg2       mbp_nav_draft:          %d\n", process->mbp_nav_draft);
		fprintf(stderr, "dbg2       mbp_nav_attitude:       %d\n", process->mbp_nav_attitude);
		fprintf(stderr, "dbg2       mbp_nav_algorithm:      %d\n", process->mbp_nav_algorithm);
		fprintf(stderr, "dbg2       mbp_nav_timeshift:      %f\n", process->mbp_nav_timeshift);
		fprintf(stderr, "dbg2       mbp_nav_shift:          %d\n", process->mbp_nav_shift);
		fprintf(stderr, "dbg2       mbp_nav_offsetx:        %f\n", process->mbp_nav_offsetx);
		fprintf(stderr, "dbg2       mbp_nav_offsety:        %f\n", process->mbp_nav_offsety);
		fprintf(stderr, "dbg2       mbp_nav_offsetz:        %f\n", process->mbp_nav_offsetz);
		fprintf(stderr, "dbg2       mbp_nav_shiftlon:       %f\n", process->mbp_nav_shiftlon);
		fprintf(stderr, "dbg2       mbp_nav_shiftlat:       %f\n", process->mbp_nav_shiftlat);
		fprintf(stderr, "dbg2       mbp_nav_shiftx:         %f\n", process->mbp_nav_shiftx);
		fprintf(stderr, "dbg2       mbp_nav_shifty:         %f\n", process->mbp_nav_shifty);
		fprintf(stderr, "dbg2       mbp_navadj_mode:        %d\n", process->mbp_navadj_mode);
		fprintf(stderr, "dbg2       mbp_navadjfile:         %s\n", process->mbp_navadjfile);
		fprintf(stderr, "dbg2       mbp_navadj_algorithm:   %d\n", process->mbp_navadj_algorithm);
		fprintf(stderr, "dbg2       mbp_attitude_mode:      %d\n", process->mbp_attitude_mode);
		fprintf(stderr, "dbg2       mbp_attitudefile:       %s\n", process->mbp_attitudefile);
		fprintf(stderr, "dbg2       mbp_attitude_format:    %d\n", process->mbp_attitude_format);
		fprintf(stderr, "dbg2       mbp_cut_num:            %d\n", process->mbp_cut_num);
		fprintf(stderr, "dbg2       mbp_sensordepth_mode:    %d\n", process->mbp_sensordepth_mode);
		fprintf(stderr, "dbg2       mbp_sensordepthfile:     %s\n", process->mbp_sensordepthfile);
		fprintf(stderr, "dbg2       mbp_sensordepth_format:  %d\n", process->mbp_sensordepth_format);
		fprintf(stderr, "dbg2       mbp_cut_num:            %d\n", process->mbp_cut_num);
		for (int i = 0; i < process->mbp_cut_num; i++) {
			fprintf(stderr, "dbg2           cut %d:\n", i);
			fprintf(stderr, "dbg2           mbp_cut_kind[%d]:     %d\n", i, process->mbp_cut_kind[i]);
			fprintf(stderr, "dbg2           mbp_cut_mode[%d]:     %d\n", i, process->mbp_cut_mode[i]);
			fprintf(stderr, "dbg2           mbp_cut_min[%d]:      %f\n", i, process->mbp_cut_min[i]);
			fprintf(stderr, "dbg2           mbp_cut_max[%d]:      %f\n", i, process->mbp_cut_max[i]);
		}
		fprintf(stderr, "dbg2       mbp_bathrecalc_mode:    %d\n", process->mbp_bathrecalc_mode);
		fprintf(stderr, "dbg2       mbp_rollbias_mode:      %d\n", process->mbp_rollbias_mode);
		fprintf(stderr, "dbg2       mbp_rollbias:           %f\n", process->mbp_rollbias);
		fprintf(stderr, "dbg2       mbp_rollbias_port:      %f\n", process->mbp_rollbias_port);
		fprintf(stderr, "dbg2       mbp_rollbias_stbd:      %f\n", process->mbp_rollbias_stbd);
		fprintf(stderr, "dbg2       mbp_pitchbias_mode:     %d\n", process->mbp_pitchbias_mode);
		fprintf(stderr, "dbg2       mbp_pitchbias:          %f\n", process->mbp_pitchbias);
		fprintf(stderr, "dbg2       mbp_draft_mode:         %d\n", process->mbp_draft_mode);
		fprintf(stderr, "dbg2       mbp_draft:              %f\n", process->mbp_draft);
		fprintf(stderr, "dbg2       mbp_draft_offset:       %f\n", process->mbp_draft_offset);
		fprintf(stderr, "dbg2       mbp_draft_mult:         %f\n", process->mbp_draft_mult);
		fprintf(stderr, "dbg2       mbp_heave_mode:         %d\n", process->mbp_heave_mode);
		fprintf(stderr, "dbg2       mbp_heave:              %f\n", process->mbp_heave);
		fprintf(stderr, "dbg2       mbp_heave_mult:         %f\n", process->mbp_heave_mult);
		fprintf(stderr, "dbg2       mbp_lever_mode:         %d\n", process->mbp_heave_mode);
		fprintf(stderr, "dbg2       mbp_vru_offsetx:        %f\n", process->mbp_vru_offsetx);
		fprintf(stderr, "dbg2       mbp_vru_offsety:        %f\n", process->mbp_vru_offsety);
		fprintf(stderr, "dbg2       mbp_vru_offsetz:        %f\n", process->mbp_vru_offsetz);
		fprintf(stderr, "dbg2       mbp_sonar_offsetx:      %f\n", process->mbp_sonar_offsetx);
		fprintf(stderr, "dbg2       mbp_sonar_offsety:      %f\n", process->mbp_sonar_offsety);
		fprintf(stderr, "dbg2       mbp_sonar_offsetz:      %f\n", process->mbp_sonar_offsetz);
		fprintf(stderr, "dbg2       mbp_ssv_mode:           %d\n", process->mbp_ssv_mode);
		fprintf(stderr, "dbg2       mbp_ssv:                %f\n", process->mbp_ssv);
		fprintf(stderr, "dbg2       mbp_svp_mode:           %d\n", process->mbp_svp_mode);
		fprintf(stderr, "dbg2       mbp_svpfile:            %s\n", process->mbp_svpfile);
		fprintf(stderr, "dbg2       mbp_corrected:          %d\n", process->mbp_corrected);
		fprintf(stderr, "dbg2       mbp_tt_mode:            %d\n", process->mbp_tt_mode);
		fprintf(stderr, "dbg2       mbp_tt_mult:            %f\n", process->mbp_tt_mult);
		fprintf(stderr, "dbg2       mbp_angle_mode:         %d\n", process->mbp_angle_mode);
		fprintf(stderr, "dbg2       mbp_static_mode:        %d\n", process->mbp_static_mode);
		fprintf(stderr, "dbg2       mbp_staticfile:         %s\n", process->mbp_staticfile);
		fprintf(stderr, "dbg2       mbp_heading_mode:       %d\n", process->mbp_heading_mode);
		fprintf(stderr, "dbg2       mbp_headingbias:        %f\n", process->mbp_headingbias);
		fprintf(stderr, "dbg2       mbp_edit_mode:          %d\n", process->mbp_edit_mode);
		fprintf(stderr, "dbg2       mbp_editfile:           %s\n", process->mbp_editfile);
		fprintf(stderr, "dbg2       mbp_tide_mode:          %d\n", process->mbp_tide_mode);
		fprintf(stderr, "dbg2       mbp_tidefile:           %s\n", process->mbp_tidefile);
		fprintf(stderr, "dbg2       mbp_tide_format:        %d\n", process->mbp_tide_format);
		fprintf(stderr, "dbg2       mbp_ampcorr_mode:       %d\n", process->mbp_ampcorr_mode);
		fprintf(stderr, "dbg2       mbp_ampcorrfile:        %s\n", process->mbp_ampcorrfile);
		fprintf(stderr, "dbg2       mbp_ampcorr_type:       %d\n", process->mbp_ampcorr_type);
		fprintf(stderr, "dbg2       mbp_ampcorr_symmetry:   %d\n", process->mbp_ampcorr_symmetry);
		fprintf(stderr, "dbg2       mbp_ampcorr_angle:      %f\n", process->mbp_ampcorr_angle);
		fprintf(stderr, "dbg2       mbp_ampcorr_slope:      %d\n", process->mbp_ampcorr_slope);
		fprintf(stderr, "dbg2       mbp_sscorr_mode:        %d\n", process->mbp_sscorr_mode);
		fprintf(stderr, "dbg2       mbp_sscorrfile:         %s\n", process->mbp_sscorrfile);
		fprintf(stderr, "dbg2       mbp_sscorr_type:        %d\n", process->mbp_sscorr_type);
		fprintf(stderr, "dbg2       mbp_sscorr_symmetry:    %d\n", process->mbp_sscorr_symmetry);
		fprintf(stderr, "dbg2       mbp_sscorr_angle:       %f\n", process->mbp_sscorr_angle);
		fprintf(stderr, "dbg2       mbp_sscorr_slope:       %d\n", process->mbp_sscorr_slope);
		fprintf(stderr, "dbg2       mbp_ampsscorr_topofile: %s\n", process->mbp_ampsscorr_topofile);
		fprintf(stderr, "dbg2       mbp_ssrecalc_mode:      %d\n", process->mbp_ssrecalc_mode);
		fprintf(stderr, "dbg2       mbp_ssrecalc_pixelsize: %f\n", process->mbp_ssrecalc_pixelsize);
		fprintf(stderr, "dbg2       mbp_ssrecalc_swathwidth:%f\n", process->mbp_ssrecalc_swathwidth);
		fprintf(stderr, "dbg2       mbp_ssrecalc_interp    :%d\n", process->mbp_ssrecalc_interpolate);
		fprintf(stderr, "dbg2       mbp_meta_vessel        :%s\n", process->mbp_meta_vessel);
		fprintf(stderr, "dbg2       mbp_meta_institution   :%s\n", process->mbp_meta_institution);
		fprintf(stderr, "dbg2       mbp_meta_platform      :%s\n", process->mbp_meta_platform);
		fprintf(stderr, "dbg2       mbp_meta_sonar         :%s\n", process->mbp_meta_sonar);
		fprintf(stderr, "dbg2       mbp_meta_sonarversion  :%s\n", process->mbp_meta_sonarversion);
		fprintf(stderr, "dbg2       mbp_meta_cruiseid      :%s\n", process->mbp_meta_cruiseid);
		fprintf(stderr, "dbg2       mbp_meta_cruisename    :%s\n", process->mbp_meta_cruisename);
		fprintf(stderr, "dbg2       mbp_meta_pi            :%s\n", process->mbp_meta_pi);
		fprintf(stderr, "dbg2       mbp_meta_piinstitution :%s\n", process->mbp_meta_piinstitution);
		fprintf(stderr, "dbg2       mbp_meta_client        :%s\n", process->mbp_meta_client);
		fprintf(stderr, "dbg2       mbp_meta_svcorrected   :%d\n", process->mbp_meta_svcorrected);
		fprintf(stderr, "dbg2       mbp_meta_tidecorrected :%d\n", process->mbp_meta_tidecorrected);
		fprintf(stderr, "dbg2       mbp_meta_batheditmanual:%d\n", process->mbp_meta_batheditmanual);
		fprintf(stderr, "dbg2       mbp_meta_batheditauto:  %d\n", process->mbp_meta_batheditauto);
		fprintf(stderr, "dbg2       mbp_meta_rollbias:      %f\n", process->mbp_meta_rollbias);
		fprintf(stderr, "dbg2       mbp_meta_pitchbias:     %f\n", process->mbp_meta_pitchbias);
		fprintf(stderr, "dbg2       mbp_meta_headingbias:   %f\n", process->mbp_meta_headingbias);
		fprintf(stderr, "dbg2       mbp_meta_draft:         %f\n", process->mbp_meta_draft);
		fprintf(stderr, "dbg2       mbp_kluge001:           %d\n", process->mbp_kluge001);
		fprintf(stderr, "dbg2       mbp_kluge002:           %d\n", process->mbp_kluge002);
		fprintf(stderr, "dbg2       mbp_kluge003:           %d\n", process->mbp_kluge003);
		fprintf(stderr, "dbg2       mbp_kluge004:           %d\n", process->mbp_kluge004);
		fprintf(stderr, "dbg2       mbp_kluge005:           %d\n", process->mbp_kluge005);
		fprintf(stderr, "dbg2       mbp_kluge006:           %d\n", process->mbp_kluge006);
		fprintf(stderr, "dbg2       mbp_kluge007:           %d\n", process->mbp_kluge007);
		fprintf(stderr, "dbg2       mbp_kluge008:           %d\n", process->mbp_kluge008);
		fprintf(stderr, "dbg2       mbp_kluge009:           %d\n", process->mbp_kluge009);
		fprintf(stderr, "dbg2       mbp_kluge010:           %d\n", process->mbp_kluge010);
	}

	mb_path pwd;
  assert(strlen(file) < MB_PATH_MAXLINE);
  assert(process != NULL);

	/* try to avoid absolute pathnames - get pwd */
	char *lastslash = strrchr(file, '/');
	if (file[0] == '/' && lastslash != NULL) {
		strcpy(pwd, file);
		pwd[strlen(file) - strlen(lastslash)] = '\0';
	}
	else {
    char *getcwd_result = getcwd(pwd, MB_PATH_MAXLINE);
    assert(strlen(pwd) > 0);
    assert(getcwd_result != NULL);
		if (lastslash != NULL) {
			strcat(pwd, "/");
			strcat(pwd, file);
			lastslash = strrchr(pwd, '/');
			pwd[strlen(pwd) - strlen(lastslash)] = '\0';
    }
	}
	mb_get_shortest_path(verbose, pwd, error);

	/* get expected process parameter file name */
	char parfile[MBP_FILENAMESIZE];
	strcpy(parfile, file);
	strcat(parfile, ".par");

	int status = MB_SUCCESS;

	/* open parameter file */
	FILE *fp = fopen(parfile, "w");
	if (fp != NULL) {
		fprintf(fp, "## MB-System processing parameter file\n");
		fprintf(fp, "## Written by %s\n", __func__);
		fprintf(fp, "## MB-system Version %s\n", MB_VERSION);
    char user[256], host[256], date[32];
    status = mb_user_host_date(verbose, user, host, date, error);
		fprintf(fp, "## Generated by user <%s> on cpu <%s> at <%s>\n##\n", user, host, date);

		/* general parameters */
		fprintf(fp, "##\n## Forces explicit reading of parameter modes.\n");
		fprintf(fp, "EXPLICIT\n");
		fprintf(fp, "##\n## General Parameters:\n");
		if (process->mbp_format_specified) {
			fprintf(fp, "FORMAT %d\n", process->mbp_format);
		}
		else {
			fprintf(fp, "## FORMAT format\n");
		}
		char relative_path[MBP_FILENAMESIZE];
		if (process->mbp_ifile_specified) {
			strcpy(relative_path, process->mbp_ifile);
			status = mb_get_relative_path(verbose, relative_path, pwd, error);
			fprintf(fp, "INFILE %s\n", relative_path);
		}
		else {
			fprintf(fp, "## INFILE infile\n");
		}
		if (process->mbp_ofile_specified) {
			strcpy(relative_path, process->mbp_ofile);
			status = mb_get_relative_path(verbose, relative_path, pwd, error);
			fprintf(fp, "OUTFILE %s\n", relative_path);
		}
		else {
			fprintf(fp, "## OUTFILE outfile\n");
		}

		/* navigation merging */
		fprintf(fp, "##\n## Navigation Merging:\n");
		fprintf(fp, "NAVMODE %d\n", process->mbp_nav_mode);
		strcpy(relative_path, process->mbp_navfile);
		status = mb_get_relative_path(verbose, relative_path, pwd, error);
		fprintf(fp, "NAVFILE %s\n", relative_path);
		fprintf(fp, "NAVFORMAT %d\n", process->mbp_nav_format);
		fprintf(fp, "NAVHEADING %d\n", process->mbp_nav_heading);
		fprintf(fp, "NAVSPEED %d\n", process->mbp_nav_speed);
		fprintf(fp, "NAVDRAFT %d\n", process->mbp_nav_draft);
		fprintf(fp, "NAVATTITUDE %d\n", process->mbp_nav_attitude);
		fprintf(fp, "NAVINTERP %d\n", process->mbp_nav_algorithm);
		fprintf(fp, "NAVTIMESHIFT %f\n", process->mbp_nav_timeshift);

		/* navigation offsets and shifts */
		fprintf(fp, "##\n## Navigation Offsets and Shifts:\n");
		fprintf(fp, "NAVSHIFT %d\n", process->mbp_nav_shift);
		fprintf(fp, "NAVOFFSETX %f\n", process->mbp_nav_offsetx);
		fprintf(fp, "NAVOFFSETY %f\n", process->mbp_nav_offsety);
		fprintf(fp, "NAVOFFSETZ %f\n", process->mbp_nav_offsetz);
		fprintf(fp, "NAVSHIFTLON %f\n", process->mbp_nav_shiftlon);
		fprintf(fp, "NAVSHIFTLAT %f\n", process->mbp_nav_shiftlat);
		fprintf(fp, "NAVSHIFTX %f\n", process->mbp_nav_shiftx);
		fprintf(fp, "NAVSHIFTY %f\n", process->mbp_nav_shifty);

		/* adjusted navigation merging */
		fprintf(fp, "##\n## Adjusted Navigation Merging:\n");
		fprintf(fp, "NAVADJMODE %d\n", process->mbp_navadj_mode);
		strcpy(relative_path, process->mbp_navadjfile);
		status = mb_get_relative_path(verbose, relative_path, pwd, error);
		fprintf(fp, "NAVADJFILE %s\n", relative_path);
		fprintf(fp, "NAVADJINTERP %d\n", process->mbp_navadj_algorithm);

		/* attitude merging */
		fprintf(fp, "##\n## Attitude Merging:\n");
		fprintf(fp, "ATTITUDEMODE %d\n", process->mbp_attitude_mode);
		strcpy(relative_path, process->mbp_attitudefile);
		status = mb_get_relative_path(verbose, relative_path, pwd, error);
		fprintf(fp, "ATTITUDEFILE %s\n", relative_path);
		fprintf(fp, "ATTITUDEFORMAT %d\n", process->mbp_attitude_format);

		/* sensordepth merging */
		fprintf(fp, "##\n## sensordepth Merging:\n");
		fprintf(fp, "sensordepthMODE %d\n", process->mbp_sensordepth_mode);
		strcpy(relative_path, process->mbp_sensordepthfile);
		status = mb_get_relative_path(verbose, relative_path, pwd, error);
		fprintf(fp, "sensordepthFILE %s\n", relative_path);
		fprintf(fp, "sensordepthFORMAT %d\n", process->mbp_sensordepth_format);

		/* data cutting */
		fprintf(fp, "##\n## Data cutting:\n");
		if (process->mbp_cut_num == 0)
			fprintf(fp, "DATACUTCLEAR\n");
		else {
			for (int i = 0; i < process->mbp_cut_num; i++)
				fprintf(fp, "DATACUT %d %d %f %f\n", process->mbp_cut_kind[i], process->mbp_cut_mode[i], process->mbp_cut_min[i],
				        process->mbp_cut_max[i]);
		}

		/* bathymetry editing */
		fprintf(fp, "##\n## Bathymetry Flagging:\n");
		fprintf(fp, "EDITSAVEMODE %d\n", process->mbp_edit_mode);
		strcpy(relative_path, process->mbp_editfile);
		status = mb_get_relative_path(verbose, relative_path, pwd, error);
		fprintf(fp, "EDITSAVEFILE %s\n", relative_path);

		/* bathymetry recalculation */
		fprintf(fp, "##\n## Bathymetry Recalculation:\n");
		fprintf(fp, "SVPMODE %d\n", process->mbp_svp_mode);
		strcpy(relative_path, process->mbp_svpfile);
		status = mb_get_relative_path(verbose, relative_path, pwd, error);
		fprintf(fp, "SVPFILE %s\n", relative_path);
		fprintf(fp, "SSVMODE %d\n", process->mbp_ssv_mode);
		fprintf(fp, "SSV %f\n", process->mbp_ssv);
		fprintf(fp, "TTMODE %d\n", process->mbp_tt_mode);
		fprintf(fp, "TTMULTIPLY %f\n", process->mbp_tt_mult);
		fprintf(fp, "ANGLEMODE %d\n", process->mbp_angle_mode);
		fprintf(fp, "SOUNDSPEEDREF %d\n", process->mbp_corrected);
		fprintf(fp, "STATICMODE %d\n", process->mbp_static_mode);
		strcpy(relative_path, process->mbp_staticfile);
		status = mb_get_relative_path(verbose, relative_path, pwd, error);
		fprintf(fp, "STATICFILE %s\n", relative_path);

		/* draft correction */
		fprintf(fp, "##\n## Draft Correction:\n");
		fprintf(fp, "DRAFTMODE %d\n", process->mbp_draft_mode);
		fprintf(fp, "DRAFT %f\n", process->mbp_draft);
		fprintf(fp, "DRAFTOFFSET %f\n", process->mbp_draft_offset);
		fprintf(fp, "DRAFTMULTIPLY %f\n", process->mbp_draft_mult);

		/* heave correction */
		fprintf(fp, "##\n## Heave Correction:\n");
		fprintf(fp, "HEAVEMODE %d\n", process->mbp_heave_mode);
		fprintf(fp, "HEAVEOFFSET %f\n", process->mbp_heave);
		fprintf(fp, "HEAVEMULTIPLY %f\n", process->mbp_heave_mult);

		/* lever correction */
		fprintf(fp, "##\n## Lever Correction:\n");
		fprintf(fp, "LEVERMODE %d\n", process->mbp_lever_mode);
		fprintf(fp, "VRUOFFSETX %f\n", process->mbp_vru_offsetx);
		fprintf(fp, "VRUOFFSETY %f\n", process->mbp_vru_offsety);
		fprintf(fp, "VRUOFFSETZ %f\n", process->mbp_vru_offsetz);
		fprintf(fp, "SONAROFFSETX %f\n", process->mbp_sonar_offsetx);
		fprintf(fp, "SONAROFFSETY %f\n", process->mbp_sonar_offsety);
		fprintf(fp, "SONAROFFSETZ %f\n", process->mbp_sonar_offsetz);

		/* roll correction */
		fprintf(fp, "##\n## Roll Correction:\n");
		fprintf(fp, "ROLLBIASMODE %d\n", process->mbp_rollbias_mode);
		fprintf(fp, "ROLLBIAS %f\n", process->mbp_rollbias);
		fprintf(fp, "ROLLBIASPORT %f\n", process->mbp_rollbias_port);
		fprintf(fp, "ROLLBIASSTBD %f\n", process->mbp_rollbias_stbd);

		/* pitch correction */
		fprintf(fp, "##\n## Pitch Correction:\n");
		fprintf(fp, "PITCHBIASMODE %d\n", process->mbp_pitchbias_mode);
		fprintf(fp, "PITCHBIAS %f\n", process->mbp_pitchbias);

		/* heading correction */
		fprintf(fp, "##\n## Heading Correction:\n");
		fprintf(fp, "HEADINGMODE %d\n", process->mbp_heading_mode);
		fprintf(fp, "HEADINGOFFSET %f\n", process->mbp_headingbias);

		/* tide correction */
		fprintf(fp, "##\n## Tide Correction:\n");
		fprintf(fp, "TIDEMODE %d\n", process->mbp_tide_mode);
		strcpy(relative_path, process->mbp_tidefile);
		status = mb_get_relative_path(verbose, relative_path, pwd, error);
		fprintf(fp, "TIDEFILE %s\n", relative_path);
		fprintf(fp, "TIDEFORMAT %d\n", process->mbp_tide_format);

		/* amplitude correction */
		fprintf(fp, "##\n## Amplitude Correction:\n");
		fprintf(fp, "AMPCORRMODE %d\n", process->mbp_ampcorr_mode);
		strcpy(relative_path, process->mbp_ampcorrfile);
		status = mb_get_relative_path(verbose, relative_path, pwd, error);
		fprintf(fp, "AMPCORRFILE %s\n", relative_path);
		fprintf(fp, "AMPCORRTYPE %d\n", process->mbp_ampcorr_type);
		fprintf(fp, "AMPCORRSYMMETRY %d\n", process->mbp_ampcorr_symmetry);
		fprintf(fp, "AMPCORRANGLE %f\n", process->mbp_ampcorr_angle);
		fprintf(fp, "AMPCORRSLOPE %d\n", process->mbp_ampcorr_slope);

		/* sidescan correction */
		fprintf(fp, "##\n## Sidescan Correction:\n");
		fprintf(fp, "SSCORRMODE %d\n", process->mbp_sscorr_mode);
		strcpy(relative_path, process->mbp_sscorrfile);
		status = mb_get_relative_path(verbose, relative_path, pwd, error);
		fprintf(fp, "SSCORRFILE %s\n", relative_path);
		fprintf(fp, "SSCORRTYPE %d\n", process->mbp_sscorr_type);
		fprintf(fp, "SSCORRSYMMETRY %d\n", process->mbp_sscorr_symmetry);
		fprintf(fp, "SSCORRANGLE %f\n", process->mbp_sscorr_angle);
		fprintf(fp, "SSCORRSLOPE %d\n", process->mbp_sscorr_slope);

		/* amplitude/sidescan topography correction */
		strcpy(relative_path, process->mbp_ampsscorr_topofile);
		status = mb_get_relative_path(verbose, relative_path, pwd, error);
		fprintf(fp, "AMPSSCORRTOPOFILE %s\n", relative_path);

		/* sidescan recalculation */
		fprintf(fp, "##\n## Sidescan Recalculation:\n");
		fprintf(fp, "SSRECALCMODE %d\n", process->mbp_ssrecalc_mode);
		fprintf(fp, "SSPIXELSIZE %f\n", process->mbp_ssrecalc_pixelsize);
		fprintf(fp, "SSSWATHWIDTH %f\n", process->mbp_ssrecalc_swathwidth);
		fprintf(fp, "SSINTERPOLATE %d\n", process->mbp_ssrecalc_interpolate);

		/* metadata insertion */
		fprintf(fp, "##\n## Metadata Insertion:\n");
		fprintf(fp, "METAVESSEL %s\n", process->mbp_meta_vessel);
		fprintf(fp, "METAINSTITUTION %s\n", process->mbp_meta_institution);
		fprintf(fp, "METAPLATFORM %s\n", process->mbp_meta_platform);
		fprintf(fp, "METASONAR %s\n", process->mbp_meta_sonar);
		fprintf(fp, "METASONARVERSION %s\n", process->mbp_meta_sonarversion);
		fprintf(fp, "METACRUISEID %s\n", process->mbp_meta_cruiseid);
		fprintf(fp, "METACRUISENAME %s\n", process->mbp_meta_cruisename);
		fprintf(fp, "METAPI %s\n", process->mbp_meta_pi);
		fprintf(fp, "METAPIINSTITUTION %s\n", process->mbp_meta_piinstitution);
		fprintf(fp, "METACLIENT %s\n", process->mbp_meta_client);
		fprintf(fp, "METASVCORRECTED %d\n", process->mbp_meta_svcorrected);
		fprintf(fp, "METATIDECORRECTED %d\n", process->mbp_meta_tidecorrected);
		fprintf(fp, "METABATHEDITMANUAL %d\n", process->mbp_meta_batheditmanual);
		fprintf(fp, "METABATHEDITAUTO %d\n", process->mbp_meta_batheditauto);
		fprintf(fp, "METAROLLBIAS %f\n", process->mbp_meta_rollbias);
		fprintf(fp, "METAPITCHBIAS %f\n", process->mbp_meta_pitchbias);
		fprintf(fp, "METAHEADINGBIAS %f\n", process->mbp_meta_headingbias);
		fprintf(fp, "METADRAFT %f\n", process->mbp_meta_draft);

		/* processing kluges */
		fprintf(fp, "##\n## Processing Kluges:\n");
		if (process->mbp_kluge001)
			fprintf(fp, "KLUGE001\n");
		if (process->mbp_kluge002)
			fprintf(fp, "KLUGE002\n");
		if (process->mbp_kluge003)
			fprintf(fp, "KLUGE003\n");
		if (process->mbp_kluge004)
			fprintf(fp, "KLUGE004\n");
		if (process->mbp_kluge005)
			fprintf(fp, "KLUGE005\n");
		if (process->mbp_kluge006)
			fprintf(fp, "KLUGE006\n");
		if (process->mbp_kluge007)
			fprintf(fp, "KLUGE007\n");
		if (process->mbp_kluge008)
			fprintf(fp, "KLUGE008\n");
		if (process->mbp_kluge009)
			fprintf(fp, "KLUGE009\n");
		if (process->mbp_kluge010)
			fprintf(fp, "KLUGE010\n");

		/* close file */
		fclose(fp);
	}

	/* set error */
	else {
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		if (verbose > 0)
			fprintf(stderr, "\nUnable to Open Parameter File <%s> for writing\n", parfile);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_compare(int verbose, struct mb_process_struct *process1,
                  struct mb_process_struct *process2, int *num_difference, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
		fprintf(stderr, "dbg2       process1:       %p\n", (void *)process1);
		fprintf(stderr, "dbg2       process2:       %p\n", (void *)process2);
	}

  *num_difference = 0;

	if (process1->mbp_ifile_specified != process2->mbp_ifile_specified) (*num_difference)++;
	if (strncmp(process1->mbp_ifile, process2->mbp_ifile, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (process1->mbp_ofile_specified != process2->mbp_ofile_specified) (*num_difference)++;
	if (strncmp(process1->mbp_ofile, process2->mbp_ofile, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (process1->mbp_format_specified != process2->mbp_format_specified) (*num_difference)++;
	if (process1->mbp_format != process2->mbp_format) (*num_difference)++;
	if (process1->mbp_nav_mode != process2->mbp_nav_mode) (*num_difference)++;
	if (strncmp(process1->mbp_navfile, process2->mbp_navfile, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (process1->mbp_nav_format != process2->mbp_nav_format) (*num_difference)++;
	if (process1->mbp_nav_heading != process2->mbp_nav_heading) (*num_difference)++;
	if (process1->mbp_nav_speed != process2->mbp_nav_speed) (*num_difference)++;
	if (process1->mbp_nav_draft != process2->mbp_nav_draft) (*num_difference)++;
	if (process1->mbp_nav_attitude != process2->mbp_nav_attitude) (*num_difference)++;
	if (process1->mbp_nav_algorithm != process2->mbp_nav_algorithm) (*num_difference)++;
	if (process1->mbp_nav_timeshift != process2->mbp_nav_timeshift) (*num_difference)++;
	if (process1->mbp_nav_shift != process2->mbp_nav_shift) (*num_difference)++;
	if (process1->mbp_nav_offsetx != process2->mbp_nav_offsetx) (*num_difference)++;
	if (process1->mbp_nav_offsety != process2->mbp_nav_offsety) (*num_difference)++;
	if (process1->mbp_nav_offsetz != process2->mbp_nav_offsetz) (*num_difference)++;
	if (process1->mbp_nav_shiftlon != process2->mbp_nav_shiftlon) (*num_difference)++;
	if (process1->mbp_nav_shiftlat != process2->mbp_nav_shiftlat) (*num_difference)++;
	if (process1->mbp_nav_shiftx != process2->mbp_nav_shiftx) (*num_difference)++;
	if (process1->mbp_nav_shifty != process2->mbp_nav_shifty) (*num_difference)++;
	if (process1->mbp_navadj_mode != process2->mbp_navadj_mode) (*num_difference)++;
	if (strncmp(process1->mbp_navadjfile, process2->mbp_navadjfile, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (process1->mbp_navadj_algorithm != process2->mbp_navadj_algorithm) (*num_difference)++;
	if (process1->mbp_attitude_mode != process2->mbp_attitude_mode) (*num_difference)++;
	if (strncmp(process1->mbp_attitudefile, process2->mbp_attitudefile, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (process1->mbp_attitude_format != process2->mbp_attitude_format) (*num_difference)++;
	if (process1->mbp_cut_num != process2->mbp_cut_num) (*num_difference)++;
	if (process1->mbp_sensordepth_mode != process2->mbp_sensordepth_mode) (*num_difference)++;
	if (strncmp(process1->mbp_sensordepthfile, process2->mbp_sensordepthfile, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (process1->mbp_sensordepth_format != process2->mbp_sensordepth_format) (*num_difference)++;
	if (process1->mbp_cut_num != process2->mbp_cut_num) (*num_difference)++;
	for (int i = 0; i < process1->mbp_cut_num; i++) {
		if (process1->mbp_cut_kind[i] != process2->mbp_cut_kind[i]) (*num_difference)++;
		if (process1->mbp_cut_mode[i] != process2->mbp_cut_mode[i]) (*num_difference)++;
		if (process1->mbp_cut_min[i] != process2->mbp_cut_min[i]) (*num_difference)++;
		if (process1->mbp_cut_max[i] != process2->mbp_cut_max[i]) (*num_difference)++;
		}
	if (process1->mbp_bathrecalc_mode != process2->mbp_bathrecalc_mode) (*num_difference)++;
	if (process1->mbp_rollbias_mode != process2->mbp_rollbias_mode) (*num_difference)++;
	if (process1->mbp_rollbias != process2->mbp_rollbias) (*num_difference)++;
	if (process1->mbp_rollbias_port != process2->mbp_rollbias_port) (*num_difference)++;
	if (process1->mbp_rollbias_stbd != process2->mbp_rollbias_stbd) (*num_difference)++;
	if (process1->mbp_pitchbias_mode != process2->mbp_pitchbias_mode) (*num_difference)++;
	if (process1->mbp_pitchbias != process2->mbp_pitchbias) (*num_difference)++;
	if (process1->mbp_draft_mode != process2->mbp_draft_mode) (*num_difference)++;
	if (process1->mbp_draft != process2->mbp_draft) (*num_difference)++;
	if (process1->mbp_draft_offset != process2->mbp_draft_offset) (*num_difference)++;
	if (process1->mbp_draft_mult != process2->mbp_draft_mult) (*num_difference)++;
	if (process1->mbp_heave_mode != process2->mbp_heave_mode) (*num_difference)++;
	if (process1->mbp_heave != process2->mbp_heave) (*num_difference)++;
	if (process1->mbp_heave_mult != process2->mbp_heave_mult) (*num_difference)++;
	if (process1->mbp_lever_mode != process2->mbp_heave_mode) (*num_difference)++;
	if (process1->mbp_vru_offsetx != process2->mbp_vru_offsetx) (*num_difference)++;
	if (process1->mbp_vru_offsety != process2->mbp_vru_offsety) (*num_difference)++;
	if (process1->mbp_vru_offsetz != process2->mbp_vru_offsetz) (*num_difference)++;
	if (process1->mbp_sonar_offsetx != process2->mbp_sonar_offsetx) (*num_difference)++;
	if (process1->mbp_sonar_offsety != process2->mbp_sonar_offsety) (*num_difference)++;
	if (process1->mbp_sonar_offsetz != process2->mbp_sonar_offsetz) (*num_difference)++;
	if (process1->mbp_ssv_mode != process2->mbp_ssv_mode) (*num_difference)++;
	if (process1->mbp_ssv != process2->mbp_ssv) (*num_difference)++;
	if (process1->mbp_svp_mode != process2->mbp_svp_mode) (*num_difference)++;
	if (strncmp(process1->mbp_svpfile, process2->mbp_svpfile, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (process1->mbp_corrected != process2->mbp_corrected) (*num_difference)++;
	if (process1->mbp_tt_mode != process2->mbp_tt_mode) (*num_difference)++;
	if (process1->mbp_tt_mult != process2->mbp_tt_mult) (*num_difference)++;
	if (process1->mbp_angle_mode != process2->mbp_angle_mode) (*num_difference)++;
	if (process1->mbp_static_mode != process2->mbp_static_mode) (*num_difference)++;
	if (strncmp(process1->mbp_staticfile, process2->mbp_staticfile, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (process1->mbp_heading_mode != process2->mbp_heading_mode) (*num_difference)++;
	if (process1->mbp_headingbias != process2->mbp_headingbias) (*num_difference)++;
	if (process1->mbp_edit_mode != process2->mbp_edit_mode) (*num_difference)++;
	if (strncmp(process1->mbp_editfile, process2->mbp_editfile, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (process1->mbp_tide_mode != process2->mbp_tide_mode) (*num_difference)++;
	if (strncmp(process1->mbp_tidefile, process2->mbp_tidefile, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (process1->mbp_tide_format != process2->mbp_tide_format) (*num_difference)++;
	if (process1->mbp_ampcorr_mode != process2->mbp_ampcorr_mode) (*num_difference)++;
	if (strncmp(process1->mbp_ampcorrfile, process2->mbp_ampcorrfile, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (process1->mbp_ampcorr_type != process2->mbp_ampcorr_type) (*num_difference)++;
	if (process1->mbp_ampcorr_symmetry != process2->mbp_ampcorr_symmetry) (*num_difference)++;
	if (process1->mbp_ampcorr_angle != process2->mbp_ampcorr_angle) (*num_difference)++;
	if (process1->mbp_ampcorr_slope != process2->mbp_ampcorr_slope) (*num_difference)++;
	if (process1->mbp_sscorr_mode != process2->mbp_sscorr_mode) (*num_difference)++;
	if (strncmp(process1->mbp_sscorrfile, process2->mbp_sscorrfile, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (process1->mbp_sscorr_type != process2->mbp_sscorr_type) (*num_difference)++;
	if (process1->mbp_sscorr_symmetry != process2->mbp_sscorr_symmetry) (*num_difference)++;
	if (process1->mbp_sscorr_angle != process2->mbp_sscorr_angle) (*num_difference)++;
	if (process1->mbp_sscorr_slope != process2->mbp_sscorr_slope) (*num_difference)++;
	if (strncmp(process1->mbp_ampsscorr_topofile, process2->mbp_ampsscorr_topofile, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (process1->mbp_ssrecalc_mode != process2->mbp_ssrecalc_mode) (*num_difference)++;
	if (process1->mbp_ssrecalc_pixelsize != process2->mbp_ssrecalc_pixelsize) (*num_difference)++;
	if (process1->mbp_ssrecalc_swathwidth != process2->mbp_ssrecalc_swathwidth) (*num_difference)++;
	if (process1->mbp_ssrecalc_interpolate != process2->mbp_ssrecalc_interpolate) (*num_difference)++;
	if (strncmp(process1->mbp_meta_vessel, process2->mbp_meta_vessel, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (strncmp(process1->mbp_meta_institution, process2->mbp_meta_institution, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (strncmp(process1->mbp_meta_platform, process2->mbp_meta_platform, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (strncmp(process1->mbp_meta_sonar, process2->mbp_meta_sonar, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (strncmp(process1->mbp_meta_sonarversion, process2->mbp_meta_sonarversion, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (strncmp(process1->mbp_meta_cruiseid, process2->mbp_meta_cruiseid, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (strncmp(process1->mbp_meta_cruisename, process2->mbp_meta_cruisename, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (strncmp(process1->mbp_meta_pi, process2->mbp_meta_pi, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (strncmp(process1->mbp_meta_piinstitution, process2->mbp_meta_piinstitution, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (strncmp(process1->mbp_meta_client, process2->mbp_meta_client, MBP_FILENAMESIZE) != 0) (*num_difference)++;
	if (process1->mbp_meta_svcorrected != process2->mbp_meta_svcorrected) (*num_difference)++;
	if (process1->mbp_meta_tidecorrected != process2->mbp_meta_tidecorrected) (*num_difference)++;
	if (process1->mbp_meta_batheditmanual != process2->mbp_meta_batheditmanual) (*num_difference)++;
	if (process1->mbp_meta_batheditauto != process2->mbp_meta_batheditauto) (*num_difference)++;
	if (process1->mbp_meta_rollbias != process2->mbp_meta_rollbias) (*num_difference)++;
	if (process1->mbp_meta_pitchbias != process2->mbp_meta_pitchbias) (*num_difference)++;
	if (process1->mbp_meta_headingbias != process2->mbp_meta_headingbias) (*num_difference)++;
	if (process1->mbp_meta_draft != process2->mbp_meta_draft) (*num_difference)++;
	if (process1->mbp_kluge001 != process2->mbp_kluge001) (*num_difference)++;
	if (process1->mbp_kluge002 != process2->mbp_kluge002) (*num_difference)++;
	if (process1->mbp_kluge003 != process2->mbp_kluge003) (*num_difference)++;
	if (process1->mbp_kluge004 != process2->mbp_kluge004) (*num_difference)++;
	if (process1->mbp_kluge005 != process2->mbp_kluge005) (*num_difference)++;
	if (process1->mbp_kluge006 != process2->mbp_kluge006) (*num_difference)++;
	if (process1->mbp_kluge007 != process2->mbp_kluge007) (*num_difference)++;
	if (process1->mbp_kluge008 != process2->mbp_kluge008) (*num_difference)++;
	if (process1->mbp_kluge009 != process2->mbp_kluge009) (*num_difference)++;
	if (process1->mbp_kluge010 != process2->mbp_kluge010) (*num_difference)++;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       num_difference: %d\n", *num_difference);
		fprintf(stderr, "dbg2       error:          %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:         %d\n", MB_SUCCESS);
	}

	return (MB_SUCCESS);
}

/*--------------------------------------------------------------------*/
int mb_pr_bathmode(int verbose, struct mb_process_struct *process, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:   %d\n", verbose);
		fprintf(stderr, "dbg2       process:   %p\n", (void *)process);
	}

	/* figure out bathymetry recalculation mode */
	if (process->mbp_svp_mode == MBP_SVP_ON)
		process->mbp_bathrecalc_mode = MBP_BATHRECALC_RAYTRACE;
	else if (process->mbp_svp_mode != MBP_SVP_ON &&
	         (process->mbp_rollbias_mode != MBP_ROLLBIAS_OFF || process->mbp_pitchbias_mode != MBP_PITCHBIAS_OFF ||
	          process->mbp_nav_attitude != MBP_NAV_OFF || process->mbp_attitude_mode != MBP_ATTITUDE_OFF))
		process->mbp_bathrecalc_mode = MBP_BATHRECALC_ROTATE;
	else if (process->mbp_svp_mode != MBP_SVP_ON && process->mbp_rollbias_mode == MBP_ROLLBIAS_OFF &&
	         (process->mbp_draft_mode != MBP_DRAFT_OFF || process->mbp_nav_draft != MBP_NAV_OFF ||
	          process->mbp_sensordepth_mode != MBP_SENSORDEPTH_OFF || process->mbp_lever_mode != MBP_LEVER_OFF ||
	          process->mbp_svp_mode == MBP_SVP_SOUNDSPEEDREF))
		process->mbp_bathrecalc_mode = MBP_BATHRECALC_OFFSET;
	else
		process->mbp_bathrecalc_mode = MBP_BATHRECALC_OFF;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_default_output(int verbose, struct mb_process_struct *process, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:             %d\n", verbose);
		fprintf(stderr, "dbg2       process:             %p\n", (void *)process);
		fprintf(stderr, "dbg2       mbp_ifile_specified: %d\n", process->mbp_ifile_specified);
		fprintf(stderr, "dbg2       mbp_ifile:           %s\n", process->mbp_ifile);
		fprintf(stderr, "dbg2       mbp_format_specified:%d\n", process->mbp_format_specified);
		fprintf(stderr, "dbg2       mbp_format:          %d\n", process->mbp_format);
	}

	/* figure out data format and fileroot if possible */
	mb_path fileroot;
	int format;
	int status = mb_get_format(verbose, process->mbp_ifile, fileroot, &format, error);
  	assert(strlen(fileroot) < MB_PATH_MAXLINE - 12);

	/* deal with format */
	if (status == MB_SUCCESS && format > 0) {
		/* set format if found */
		if (!process->mbp_format_specified) {
			process->mbp_format = format;
			process->mbp_format_specified = true;
		}

		/* set output file if needed */
		if (!process->mbp_ofile_specified && process->mbp_format_specified) {
			/* use p.mbXXX suffix if already edited MBARI ROV navigation */
			if (process->mbp_format == MBF_MBARIROV
        && strlen(fileroot) > 6
        && strncmp(&fileroot[strlen(fileroot)-6], "edited", 6) == 0) {
				sprintf(process->mbp_ofile, "%sp.mb%d", fileroot, process->mbp_format);
      }
			/* else use .txt suffix if unedited MBARI ROV navigation */
			else if (process->mbp_format == MBF_MBARIROV) {
        sprintf(process->mbp_ofile, "%sedited.txt", fileroot);
      }
			/* else use standard .mbXXX suffix */
			else {
				sprintf(process->mbp_ofile, "%sp.mb%d", fileroot, process->mbp_format);
      }
			process->mbp_ofile_specified = true;
		}
	}
	else if (!process->mbp_ofile_specified && process->mbp_format_specified) {
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		strcpy(fileroot, process->mbp_ifile);
		if (strncmp(&process->mbp_ifile[strlen(process->mbp_ifile) - 4], ".txt", 4) == 0)
			fileroot[strlen(process->mbp_ifile) - 4] = '\0';
		sprintf(process->mbp_ofile, "%sp.mb%d", fileroot, process->mbp_format);
		process->mbp_ofile_specified = true;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_ofile_specified: %d\n", process->mbp_ofile_specified);
		fprintf(stderr, "dbg2       mbp_ofile:           %s\n", process->mbp_ofile);
		fprintf(stderr, "dbg2       mbp_format_specified:%d\n", process->mbp_format_specified);
		fprintf(stderr, "dbg2       mbp_format:          %d\n", process->mbp_format);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_output(int verbose, int *format, char *ifile, char *ofile, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:	%d\n", verbose);
		fprintf(stderr, "dbg2       format:	%d\n", *format);
		fprintf(stderr, "dbg2       ifile:	%s\n", ifile);
	}

	/* figure out data format and fileroot if possible */
	mb_path fileroot;
	int tformat;
	int status = mb_get_format(verbose, ifile, fileroot, &tformat, error);

	/* use fileroot if possible */
	if (status == MB_SUCCESS) {
		/* set format if needed */
		if (*format <= 0)
			*format = tformat;

		/* use .txt suffix if unedited MBARI ROV navigation, p.mbXXX suffix
       	if already edited and processed */
		if (*format == MBF_MBARIROV) {
      		if (strlen(fileroot) > 6
          		&& strncmp(&fileroot[strlen(fileroot)-6], "edited", 6) == 0) {
				sprintf(ofile, "%sp.mb%d", fileroot, *format);
      		}
      		else {
			  	sprintf(ofile, "%sedited.txt", fileroot);
      		}
    	}

		/* else use standard .mbXXX suffix */
		else {
			sprintf(ofile, "%sp.mb%d", fileroot, *format);
    	}
	}

	/* else just add suffix */
	else if (*format > 0) {
		sprintf(ofile, "%sp.mb%d", ifile, *format);
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
	}

	/* else failure */
	else {
		sprintf(ofile, "%s.proc", ifile);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       format:	%d\n", *format);
		fprintf(stderr, "dbg2       ofile:	%s\n", ofile);
		fprintf(stderr, "dbg2       error:	%d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:	%d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_check(int verbose, char *ifile, int *nparproblem, int *ndataproblem, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:   %d\n", verbose);
		fprintf(stderr, "dbg2       ifile:     %s\n", ifile);
	}

	/* output stream for basic stuff (stdout if verbose <= 1,
	    output if verbose > 1) */
	FILE *output = verbose <= 1 ? stdout : stderr;

	/* set no problem */
	*nparproblem = 0;
	*ndataproblem = 0;

	/* check if input exists */
	bool missing_ifile = false;
	struct stat statbuf;
	if (stat(ifile, &statbuf) != 0) {
		missing_ifile = true;
		(*nparproblem)++;
	}

	int status = MB_SUCCESS;
	struct mb_process_struct process;

	int format;
	bool unexpected_format = false;
	bool unexpected_output = false;
	bool missing_ofile = false;
	bool missing_navfile = false;
	bool missing_navadjfile = false;
	bool missing_attitudefile = false;
	bool missing_sensordepthfile = false;
	bool missing_svpfile = false;
	bool missing_editfile = false;
	bool missing_tidefile = false;

	/* only check parameter file if parameter file exists */
	char ofile[MBP_FILENAMESIZE];
	sprintf(ofile, "%s.par", ifile);
	if (stat(ofile, &statbuf) == 0) {

		/* get known process parameters */
		status = mb_pr_readpar(verbose, ifile, false, &process, error);

		/* get default data format and output file */
		format = 0;
		status = mb_pr_get_output(verbose, &format, process.mbp_ifile, ofile, error);

		/* check data format */
		if (status == MB_SUCCESS && process.mbp_format_specified && format != 0 && process.mbp_format != format) {
			unexpected_format = true;
			(*nparproblem)++;

			/* get output file with specified format */
			status = mb_pr_get_output(verbose, &process.mbp_format, process.mbp_ifile, ofile, error);
		}

		/* check output file */
		if (status == MB_SUCCESS && process.mbp_ofile_specified && format != 0) {
			if (strcmp(process.mbp_ofile, ofile) != 0) {
				unexpected_output = true;
				(*nparproblem)++;
			}
		}

		/* check if output file specified but does not exist */
		if (process.mbp_ofile_specified && stat(process.mbp_ofile, &statbuf) != 0) {
			missing_ofile = true;
			(*nparproblem)++;
		}

		/* check if nav file specified but does not exist */
		if (process.mbp_nav_mode == MBP_NAV_ON && stat(process.mbp_navfile, &statbuf) != 0) {
			missing_navfile = true;
			(*nparproblem)++;
		}

		/* check if navadj file specified but does not exist */
		if ((process.mbp_navadj_mode == MBP_NAVADJ_LLZ
          || process.mbp_navadj_mode == MBP_NAVADJ_LLZ)
        && stat(process.mbp_navadjfile, &statbuf) != 0) {
			missing_navadjfile = true;
			(*nparproblem)++;
		}

		/* check if attitude file specified but does not exist */
		if (process.mbp_attitude_mode == MBP_ATTITUDE_ON && stat(process.mbp_attitudefile, &statbuf) != 0) {
			missing_attitudefile = true;
			(*nparproblem)++;
		}

		/* check if sensordepth file specified but does not exist */
		if (process.mbp_sensordepth_mode == MBP_SENSORDEPTH_ON && stat(process.mbp_sensordepthfile, &statbuf) != 0) {
			missing_sensordepthfile = true;
			(*nparproblem)++;
		}

		/* check if svp file specified but does not exist */
		if (process.mbp_svp_mode == MBP_SVP_ON && stat(process.mbp_svpfile, &statbuf) != 0) {
			missing_svpfile = true;
			(*nparproblem)++;
		}

		/* check if edit file specified but does not exist */
		if (process.mbp_edit_mode == MBP_EDIT_ON && stat(process.mbp_editfile, &statbuf) != 0) {
			missing_editfile = true;
			(*nparproblem)++;
		}

		/* check if tide file specified but does not exist */
		if (process.mbp_tide_mode == MBP_TIDE_ON && stat(process.mbp_tidefile, &statbuf) != 0) {
			missing_tidefile = true;
			(*nparproblem)++;
		}
	}

	/* only check inf file if inf file exists */
	sprintf(ofile, "%s.inf", ifile);
	if (stat(ofile, &statbuf) == 0) {
		/* open if possible */
		FILE *fp = fopen(ofile, "r");
		if (fp != NULL) {
			/* read the inf file */
			char line[MB_PATH_MAXLINE];
			while (fgets(line, MB_PATH_MAXLINE, fp) != NULL) {
				if (strncmp(line, "PN: ", 4) == 0) {
					if (*ndataproblem == 0 && verbose > 0)
						fprintf(output, "\nData File Problems: %s\n", ifile);
					fprintf(output, "%s: %s", ifile, &line[4]);
					(*ndataproblem)++;
				}
			}
			fclose(fp);
		}
	}

	/* output results */
	if (*nparproblem > 0 && verbose > 0) {
		fprintf(output, "\nParameter File Problems: %s\n", ifile);
		if (unexpected_format)
			fprintf(output, "\tUnexpected format: %d instead of %d\n", process.mbp_format, format);
		if (unexpected_output)
			fprintf(output, "\tUnexpected output: %s instead of %s\n", process.mbp_ofile, ofile);
		if (missing_ifile)
			fprintf(output, "\tMissing input file: %s does not exist\n", process.mbp_ifile);
		if (missing_ofile)
			fprintf(output, "\tMissing output file: %s does not exist\n", process.mbp_ofile);
		if (missing_navfile)
			fprintf(output, "\tMissing nav file: %s does not exist\n", process.mbp_navfile);
		if (missing_navadjfile)
			fprintf(output, "\tMissing navadj file: %s does not exist\n", process.mbp_navadjfile);
		if (missing_attitudefile)
			fprintf(output, "\tMissing attitude file: %s does not exist\n", process.mbp_attitudefile);
		if (missing_sensordepthfile)
			fprintf(output, "\tMissing sensordepth file: %s does not exist\n", process.mbp_sensordepthfile);
		if (missing_svpfile)
			fprintf(output, "\tMissing svp file: %s does not exist\n", process.mbp_svpfile);
		if (missing_editfile)
			fprintf(output, "\tMissing edit file: %s does not exist\n", process.mbp_editfile);
		if (missing_tidefile)
			fprintf(output, "\tMissing tide file: %s does not exist\n", process.mbp_tidefile);
	}
	else if (*nparproblem > 0) {
		if (unexpected_format)
			fprintf(output, "%s : Unexpected format : %d\n", process.mbp_ifile, process.mbp_format);
		if (unexpected_output)
			fprintf(output, "%s : Unexpected output : %s\n", process.mbp_ifile, process.mbp_ofile);
		if (missing_ifile)
			fprintf(output, "%s : Missing input file : %s\n", process.mbp_ifile, process.mbp_ifile);
		if (missing_ofile)
			fprintf(output, "%s : Missing output file : %s\n", process.mbp_ifile, process.mbp_ofile);
		if (missing_navfile)
			fprintf(output, "%s : Missing nav file : %s\n", process.mbp_ifile, process.mbp_navfile);
		if (missing_navadjfile)
			fprintf(output, "%s : Missing navadj file : %s\n", process.mbp_ifile, process.mbp_navadjfile);
		if (missing_attitudefile)
			fprintf(output, "%s : Missing attitude file : %s\n", process.mbp_ifile, process.mbp_attitudefile);
		if (missing_sensordepthfile)
			fprintf(output, "%s : Missing sensordepth file : %s\n", process.mbp_ifile, process.mbp_sensordepthfile);
		if (missing_svpfile)
			fprintf(output, "%s : Missing svp file : %s\n", process.mbp_ifile, process.mbp_svpfile);
		if (missing_editfile)
			fprintf(output, "%s : Missing edit file : %s\n", process.mbp_ifile, process.mbp_editfile);
		if (missing_tidefile)
			fprintf(output, "%s : Missing tide file : %s\n", process.mbp_ifile, process.mbp_tidefile);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       nparproblem:  %d\n", *nparproblem);
		fprintf(stderr, "dbg2       ndataproblem: %d\n", *ndataproblem);
		fprintf(stderr, "dbg2       error:        %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:       %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_ofile(int verbose, char *file, int mbp_ofile_specified, char *mbp_ofile, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:             %d\n", verbose);
		fprintf(stderr, "dbg2       file:                %s\n", file);
		fprintf(stderr, "dbg2       mbp_ofile_specified: %d\n", mbp_ofile_specified);
		fprintf(stderr, "dbg2       ofile:               %s\n", mbp_ofile);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set ofile value */
	if (mbp_ofile != NULL) {
		strcpy(process.mbp_ofile, mbp_ofile);
		process.mbp_ofile_specified = mbp_ofile_specified;
	}
	else {
		process.mbp_ofile[0] = '\0';
		process.mbp_ofile_specified = false;
	}

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_format(int verbose, char *file, int mbp_format_specified, int mbp_format, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:              %d\n", verbose);
		fprintf(stderr, "dbg2       file:                 %s\n", file);
		fprintf(stderr, "dbg2       mbp_format_specified: %d\n", mbp_format_specified);
		fprintf(stderr, "dbg2       mbp_format:           %d\n", mbp_format);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set format value */
	process.mbp_format_specified = mbp_format_specified;
	process.mbp_format = mbp_format;

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_rollbias(int verbose, char *file, int mbp_rollbias_mode, double mbp_rollbias, double mbp_rollbias_port,
                          double mbp_rollbias_stbd, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
		fprintf(stderr, "dbg2       mbp_rollbias_mode: %d\n", mbp_rollbias_mode);
		fprintf(stderr, "dbg2       mbp_rollbias:      %f\n", mbp_rollbias);
		fprintf(stderr, "dbg2       mbp_rollbias_port: %f\n", mbp_rollbias_port);
		fprintf(stderr, "dbg2       mbp_rollbias_stbd: %f\n", mbp_rollbias_stbd);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set rollbias values */
	process.mbp_rollbias_mode = mbp_rollbias_mode;
	process.mbp_rollbias = mbp_rollbias;
	process.mbp_rollbias_port = mbp_rollbias_port;
	process.mbp_rollbias_stbd = mbp_rollbias_stbd;

	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, &process, error);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_pitchbias(int verbose, char *file, int mbp_pitchbias_mode, double mbp_pitchbias, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
		fprintf(stderr, "dbg2       mbp_pitchbias_mode: %d\n", mbp_pitchbias_mode);
		fprintf(stderr, "dbg2       mbp_pitchbias:      %f\n", mbp_pitchbias);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set pitchbias values */
	process.mbp_pitchbias_mode = mbp_pitchbias_mode;
	process.mbp_pitchbias = mbp_pitchbias;

	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, &process, error);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_draft(int verbose, char *file, int mbp_draft_mode, double mbp_draft, double mbp_draft_offset,
                       double mbp_draft_mult, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
		fprintf(stderr, "dbg2       mbp_draft_mode:    %d\n", mbp_draft_mode);
		fprintf(stderr, "dbg2       mbp_draft:         %f\n", mbp_draft);
		fprintf(stderr, "dbg2       mbp_draft_offset:  %f\n", mbp_draft_offset);
		fprintf(stderr, "dbg2       mbp_draft_mult:    %f\n", mbp_draft_mult);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set draft values */
	process.mbp_draft_mode = mbp_draft_mode;
	process.mbp_draft = mbp_draft;
	process.mbp_draft_offset = mbp_draft_offset;
	process.mbp_draft_mult = mbp_draft_mult;

	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, &process, error);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_heave(int verbose, char *file, int mbp_heave_mode, double mbp_heave, double mbp_heave_mult, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
		fprintf(stderr, "dbg2       mbp_heave_mode:    %d\n", mbp_heave_mode);
		fprintf(stderr, "dbg2       mbp_heave:         %f\n", mbp_heave);
		fprintf(stderr, "dbg2       mbp_heave_mult:    %f\n", mbp_heave_mult);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set heave values */
	process.mbp_heave_mode = mbp_heave_mode;
	process.mbp_heave = mbp_heave;
	process.mbp_heave_mult = mbp_heave_mult;

	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, &process, error);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_lever(int verbose, char *file, int mbp_lever_mode, double mbp_vru_offsetx, double mbp_vru_offsety,
                       double mbp_vru_offsetz, double mbp_sonar_offsetx, double mbp_sonar_offsety, double mbp_sonar_offsetz,
                       int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
		fprintf(stderr, "dbg2       mbp_lever_mode:    %d\n", mbp_lever_mode);
		fprintf(stderr, "dbg2       mbp_vru_offsetx:   %f\n", mbp_vru_offsetx);
		fprintf(stderr, "dbg2       mbp_vru_offsety:   %f\n", mbp_vru_offsety);
		fprintf(stderr, "dbg2       mbp_vru_offsetz:   %f\n", mbp_vru_offsetz);
		fprintf(stderr, "dbg2       mbp_sonar_offsetx: %f\n", mbp_sonar_offsetx);
		fprintf(stderr, "dbg2       mbp_sonar_offsety: %f\n", mbp_sonar_offsety);
		fprintf(stderr, "dbg2       mbp_sonar_offsetz: %f\n", mbp_sonar_offsetz);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set lever values */
	process.mbp_lever_mode = mbp_lever_mode;
	process.mbp_vru_offsetx = mbp_vru_offsetx;
	process.mbp_vru_offsety = mbp_vru_offsety;
	process.mbp_vru_offsetz = mbp_vru_offsetz;
	process.mbp_sonar_offsetx = mbp_sonar_offsetx;
	process.mbp_sonar_offsety = mbp_sonar_offsety;
	process.mbp_sonar_offsetz = mbp_sonar_offsetz;

	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, &process, error);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_tide(int verbose, char *file, int mbp_tide_mode, char *mbp_tidefile, int mbp_tide_format, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
		fprintf(stderr, "dbg2       mbp_tide_mode:     %d\n", mbp_tide_mode);
		fprintf(stderr, "dbg2       mbp_tidefile:      %s\n", mbp_tidefile);
		fprintf(stderr, "dbg2       mbp_tide_format:   %d\n", mbp_tide_format);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set lever values */
	process.mbp_tide_mode = mbp_tide_mode;
	if (mbp_tidefile != NULL)
		strcpy(process.mbp_tidefile, mbp_tidefile);
	process.mbp_tide_format = mbp_tide_format;

	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, &process, error);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_tt(int verbose, char *file, int mbp_tt_mode, double mbp_tt_mult, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
		fprintf(stderr, "dbg2       mbp_tt_mode:       %d\n", mbp_tt_mode);
		fprintf(stderr, "dbg2       mbp_tt_mult:       %f\n", mbp_tt_mult);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set tt values */
	process.mbp_tt_mode = mbp_tt_mode;
	process.mbp_tt_mult = mbp_tt_mult;

	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, &process, error);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_ssv(int verbose, char *file, int mbp_ssv_mode, double mbp_ssv, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
		fprintf(stderr, "dbg2       mbp_ssv_mode:      %d\n", mbp_ssv_mode);
		fprintf(stderr, "dbg2       mbp_ssv:           %f\n", mbp_ssv);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set ssv values */
	process.mbp_ssv_mode = mbp_ssv_mode;
	process.mbp_ssv = mbp_ssv;

	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, &process, error);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_svp(int verbose, char *file, int mbp_svp_mode, char *mbp_svpfile, int mbp_angle_mode, int mbp_corrected,
                     int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
		fprintf(stderr, "dbg2       mbp_svp_mode:      %d\n", mbp_svp_mode);
		fprintf(stderr, "dbg2       mbp_svpfile:       %s\n", mbp_svpfile);
		fprintf(stderr, "dbg2       mbp_angle_mode:    %d\n", mbp_angle_mode);
		fprintf(stderr, "dbg2       mbp_corrected:     %d\n", mbp_corrected);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set svp values */
	process.mbp_svp_mode = mbp_svp_mode;
	if (mbp_svpfile != NULL)
		strcpy(process.mbp_svpfile, mbp_svpfile);
	process.mbp_angle_mode = mbp_angle_mode;
	process.mbp_corrected = mbp_corrected;

	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, &process, error);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_static(int verbose, char *file, int mbp_static_mode, char *mbp_staticfile, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
		fprintf(stderr, "dbg2       mbp_static_mode:   %d\n", mbp_static_mode);
		fprintf(stderr, "dbg2       mbp_staticfile:    %s\n", mbp_staticfile);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set svp values */
	process.mbp_static_mode = mbp_static_mode;
	if (mbp_staticfile != NULL)
		strcpy(process.mbp_staticfile, mbp_staticfile);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_navadj(int verbose, char *file, int mbp_navadj_mode, char *mbp_navadjfile, int mbp_navadj_algorithm,
                        int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:              %d\n", verbose);
		fprintf(stderr, "dbg2       file:                 %s\n", file);
		fprintf(stderr, "dbg2       mbp_navadj_mode:      %d\n", mbp_navadj_mode);
		fprintf(stderr, "dbg2       mbp_navadjfile:       %s\n", mbp_navadjfile);
		fprintf(stderr, "dbg2       mbp_navadj_algorithm: %d\n", mbp_navadj_algorithm);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set navadj values */
	process.mbp_navadj_mode = mbp_navadj_mode;
	if (mbp_navadj_mode == MBP_NAVADJ_OFF)
		process.mbp_navadjfile[0] = '\0';
	else if (mbp_navadjfile != NULL)
		strcpy(process.mbp_navadjfile, mbp_navadjfile);
	process.mbp_navadj_algorithm = mbp_navadj_algorithm;

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_attitude(int verbose, char *file, int mbp_attitude_mode, char *mbp_attitudefile, int mbp_attitude_format,
                          int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
		fprintf(stderr, "dbg2       mbp_attitude_mode: %d\n", mbp_attitude_mode);
		fprintf(stderr, "dbg2       mbp_attitudefile:  %s\n", mbp_attitudefile);
		fprintf(stderr, "dbg2       mbp_attitude_format:%d\n", mbp_attitude_format);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set lever values */
	process.mbp_attitude_mode = mbp_attitude_mode;
	if (mbp_attitudefile != NULL)
		strcpy(process.mbp_attitudefile, mbp_attitudefile);
	process.mbp_attitude_format = mbp_attitude_format;

	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, &process, error);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_sensordepth(int verbose, char *file, int mbp_sensordepth_mode, char *mbp_sensordepthfile, int mbp_sensordepth_format,
                            int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:               %d\n", verbose);
		fprintf(stderr, "dbg2       file:                  %s\n", file);
		fprintf(stderr, "dbg2       mbp_sensordepth_mode:   %d\n", mbp_sensordepth_mode);
		fprintf(stderr, "dbg2       mbp_sensordepthfile:    %s\n", mbp_sensordepthfile);
		fprintf(stderr, "dbg2       mbp_sensordepth_format: %d\n", mbp_sensordepth_format);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set lever values */
	process.mbp_sensordepth_mode = mbp_sensordepth_mode;
	if (mbp_sensordepthfile != NULL)
		strcpy(process.mbp_sensordepthfile, mbp_sensordepthfile);
	process.mbp_sensordepth_format = mbp_sensordepth_format;

	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, &process, error);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_nav(int verbose, char *file, int mbp_nav_mode, char *mbp_navfile, int mbp_nav_format, int mbp_nav_heading,
                     int mbp_nav_speed, int mbp_nav_draft, int mbp_nav_attitude, int mbp_nav_algorithm, double mbp_nav_timeshift,
                     int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
		fprintf(stderr, "dbg2       mbp_nav_mode:      %d\n", mbp_nav_mode);
		fprintf(stderr, "dbg2       mbp_navfile:       %s\n", mbp_navfile);
		fprintf(stderr, "dbg2       mbp_nav_format:    %d\n", mbp_nav_format);
		fprintf(stderr, "dbg2       mbp_nav_heading:   %d\n", mbp_nav_heading);
		fprintf(stderr, "dbg2       mbp_nav_speed:     %d\n", mbp_nav_speed);
		fprintf(stderr, "dbg2       mbp_nav_draft:     %d\n", mbp_nav_draft);
		fprintf(stderr, "dbg2       mbp_nav_attitude:  %d\n", mbp_nav_attitude);
		fprintf(stderr, "dbg2       mbp_nav_algorithm: %d\n", mbp_nav_algorithm);
		fprintf(stderr, "dbg2       mbp_nav_timeshift: %f\n", mbp_nav_timeshift);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set nav values */
	process.mbp_nav_mode = mbp_nav_mode;
	if (mbp_navfile != NULL)
		strcpy(process.mbp_navfile, mbp_navfile);
	process.mbp_nav_format = mbp_nav_format;
	process.mbp_nav_heading = mbp_nav_heading;
	process.mbp_nav_speed = mbp_nav_speed;
	process.mbp_nav_draft = mbp_nav_draft;
	process.mbp_nav_attitude = mbp_nav_attitude;
	process.mbp_nav_algorithm = mbp_nav_algorithm;
	process.mbp_nav_timeshift = mbp_nav_timeshift;

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_navshift(int verbose, char *file, int mbp_nav_shift, double mbp_nav_offsetx, double mbp_nav_offsety,
                          double mbp_nav_offsetz, double mbp_nav_shiftlon, double mbp_nav_shiftlat, double mbp_nav_shiftx,
                          double mbp_nav_shifty, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
		fprintf(stderr, "dbg2       mbp_nav_shift:     %d\n", mbp_nav_shift);
		fprintf(stderr, "dbg2       mbp_nav_offsetx:   %f\n", mbp_nav_offsetx);
		fprintf(stderr, "dbg2       mbp_nav_offsety:   %f\n", mbp_nav_offsety);
		fprintf(stderr, "dbg2       mbp_nav_offsetz:   %f\n", mbp_nav_offsetz);
		fprintf(stderr, "dbg2       mbp_nav_shiftlon:  %f\n", mbp_nav_shiftlon);
		fprintf(stderr, "dbg2       mbp_nav_shiftlat:  %f\n", mbp_nav_shiftlat);
		fprintf(stderr, "dbg2       mbp_nav_shiftx:    %f\n", mbp_nav_shiftx);
		fprintf(stderr, "dbg2       mbp_nav_shifty:    %f\n", mbp_nav_shifty);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set nav values */
	process.mbp_nav_shift = mbp_nav_shift;
	process.mbp_nav_offsetx = mbp_nav_offsetx;
	process.mbp_nav_offsety = mbp_nav_offsety;
	process.mbp_nav_offsetz = mbp_nav_offsetz;
	process.mbp_nav_shiftlon = mbp_nav_shiftlon;
	process.mbp_nav_shiftlat = mbp_nav_shiftlat;
	process.mbp_nav_shiftx = mbp_nav_shiftx;
	process.mbp_nav_shifty = mbp_nav_shifty;

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_heading(int verbose, char *file, int mbp_heading_mode, double mbp_headingbias, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
		fprintf(stderr, "dbg2       mbp_heading_mode:  %d\n", mbp_heading_mode);
		fprintf(stderr, "dbg2       mbp_headingbias:   %f\n", mbp_headingbias);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set heading values */
	process.mbp_heading_mode = mbp_heading_mode;
	process.mbp_headingbias = mbp_headingbias;

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_datacut(int verbose, char *file, int mbp_cut_num, int *mbp_cut_kind, int *mbp_cut_mode, double *mbp_cut_min,
                         double *mbp_cut_max, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
		fprintf(stderr, "dbg2       mbp_cut_num:       %d\n", mbp_cut_num);
		for (int i = 0; i < mbp_cut_num; i++) {
			fprintf(stderr, "dbg2       mbp_cut_kind[%d]:   %d\n", i, mbp_cut_kind[i]);
			fprintf(stderr, "dbg2       mbp_cut_mode[%d]:   %d\n", i, mbp_cut_mode[i]);
			fprintf(stderr, "dbg2       mbp_cut_min[%d]:    %f\n", i, mbp_cut_min[i]);
			fprintf(stderr, "dbg2       mbp_cut_max[%d]:    %f\n", i, mbp_cut_max[i]);
		}
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set datacut values */
	process.mbp_cut_num = mbp_cut_num;
	for (int i = 0; i < mbp_cut_num; i++) {
		process.mbp_cut_kind[i] = mbp_cut_kind[i];
		process.mbp_cut_mode[i] = mbp_cut_mode[i];
		process.mbp_cut_min[i] = mbp_cut_min[i];
		process.mbp_cut_max[i] = mbp_cut_max[i];
	}

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_edit(int verbose, char *file, int mbp_edit_mode, char *mbp_editfile, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
		fprintf(stderr, "dbg2       mbp_edit_mode:     %d\n", mbp_edit_mode);
		fprintf(stderr, "dbg2       mbp_editfile:      %s\n", mbp_editfile);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set edit values */
	process.mbp_edit_mode = mbp_edit_mode;
	if (mbp_editfile != NULL)
		strcpy(process.mbp_editfile, mbp_editfile);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_ampcorr(int verbose, char *file, int mbp_ampcorr_mode, char *mbp_ampcorrfile, int mbp_ampcorr_type,
                         int mbp_ampcorr_symmetry, double mbp_ampcorr_angle, int mbp_ampcorr_slope, char *mbp_ampsscorr_topofile,
                         int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                  %d\n", verbose);
		fprintf(stderr, "dbg2       file:                     %s\n", file);
		fprintf(stderr, "dbg2       mbp_ampcorr_mode:          %d\n", mbp_ampcorr_mode);
		fprintf(stderr, "dbg2       mbp_ampcorrfile:           %s\n", mbp_ampcorrfile);
		fprintf(stderr, "dbg2       mbp_ampcorr_type:          %d\n", mbp_ampcorr_type);
		fprintf(stderr, "dbg2       mbp_ampcorr_symmetry:      %d\n", mbp_ampcorr_symmetry);
		fprintf(stderr, "dbg2       mbp_ampcorr_angle:         %f\n", mbp_ampcorr_angle);
		fprintf(stderr, "dbg2       mbp_ampcorr_slope:         %d\n", mbp_ampcorr_slope);
		fprintf(stderr, "dbg2       mbp_ampsscorr_topofile:    %s\n", mbp_ampsscorr_topofile);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set ampcorr values */
	process.mbp_ampcorr_mode = mbp_ampcorr_mode;
	if (mbp_ampcorrfile != NULL)
		strcpy(process.mbp_ampcorrfile, mbp_ampcorrfile);
	process.mbp_ampcorr_type = mbp_ampcorr_type;
	process.mbp_ampcorr_symmetry = mbp_ampcorr_symmetry;
	process.mbp_ampcorr_angle = mbp_ampcorr_angle;
	process.mbp_ampcorr_slope = mbp_ampcorr_slope;
	if (mbp_ampsscorr_topofile != NULL)
		strcpy(process.mbp_ampsscorr_topofile, mbp_ampsscorr_topofile);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:                    %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                   %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_sscorr(int verbose, char *file, int mbp_sscorr_mode, char *mbp_sscorrfile, int mbp_sscorr_type,
                        int mbp_sscorr_symmetry, double mbp_sscorr_angle, int mbp_sscorr_slope, char *mbp_ampsscorr_topofile,
                        int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                  %d\n", verbose);
		fprintf(stderr, "dbg2       file:                     %s\n", file);
		fprintf(stderr, "dbg2       mbp_sscorr_mode:          %d\n", mbp_sscorr_mode);
		fprintf(stderr, "dbg2       mbp_sscorrfile:           %s\n", mbp_sscorrfile);
		fprintf(stderr, "dbg2       mbp_sscorr_type:          %d\n", mbp_sscorr_type);
		fprintf(stderr, "dbg2       mbp_sscorr_symmetry:      %d\n", mbp_sscorr_symmetry);
		fprintf(stderr, "dbg2       mbp_sscorr_angle:         %f\n", mbp_sscorr_angle);
		fprintf(stderr, "dbg2       mbp_sscorr_slope:         %d\n", mbp_sscorr_slope);
		fprintf(stderr, "dbg2       mbp_ampsscorr_topofile:   %s\n", mbp_ampsscorr_topofile);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set sscorr values */
	process.mbp_sscorr_mode = mbp_sscorr_mode;
	if (mbp_sscorrfile != NULL)
		strcpy(process.mbp_sscorrfile, mbp_sscorrfile);
	process.mbp_sscorr_type = mbp_sscorr_type;
	process.mbp_sscorr_symmetry = mbp_sscorr_symmetry;
	process.mbp_sscorr_angle = mbp_sscorr_angle;
	process.mbp_sscorr_slope = mbp_sscorr_slope;
	if (mbp_ampsscorr_topofile != NULL)
		strcpy(process.mbp_ampsscorr_topofile, mbp_ampsscorr_topofile);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:                    %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                   %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_ssrecalc(int verbose, char *file, int mbp_ssrecalc_mode, double mbp_ssrecalc_pixelsize,
                          double mbp_ssrecalc_swathwidth, int mbp_ssrecalc_interpolate, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                  %d\n", verbose);
		fprintf(stderr, "dbg2       file:                     %s\n", file);
		fprintf(stderr, "dbg2       mbp_ssrecalc_mode:        %d\n", mbp_ssrecalc_mode);
		fprintf(stderr, "dbg2       mbp_ssrecalc_pixelsize:   %f\n", mbp_ssrecalc_pixelsize);
		fprintf(stderr, "dbg2       mbp_ssrecalc_swathwidth:  %f\n", mbp_ssrecalc_swathwidth);
		fprintf(stderr, "dbg2       mbp_ssrecalc_interpolate: %d\n", mbp_ssrecalc_interpolate);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set ssrecalc values */
	process.mbp_ssrecalc_mode = mbp_ssrecalc_mode;
	process.mbp_ssrecalc_pixelsize = mbp_ssrecalc_pixelsize;
	process.mbp_ssrecalc_swathwidth = mbp_ssrecalc_swathwidth;
	process.mbp_ssrecalc_interpolate = mbp_ssrecalc_interpolate;

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:                    %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                   %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_metadata(int verbose, char *file, char *mbp_meta_vessel, char *mbp_meta_institution, char *mbp_meta_platform,
                          char *mbp_meta_sonar, char *mbp_meta_sonarversion, char *mbp_meta_cruiseid, char *mbp_meta_cruisename,
                          char *mbp_meta_pi, char *mbp_meta_piinstitution, char *mbp_meta_client, int mbp_meta_svcorrected,
                          int mbp_meta_tidecorrected, int mbp_meta_batheditmanual, int mbp_meta_batheditauto,
                          double mbp_meta_rollbias, double mbp_meta_pitchbias, double mbp_meta_headingbias, double mbp_meta_draft,
                          int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                  %d\n", verbose);
		fprintf(stderr, "dbg2       file:                     %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "dbg2       mbp_meta_vessel:          %s\n", process.mbp_meta_vessel);
		fprintf(stderr, "dbg2       mbp_meta_institution:     %s\n", process.mbp_meta_institution);
		fprintf(stderr, "dbg2       mbp_meta_platform:        %s\n", process.mbp_meta_platform);
		fprintf(stderr, "dbg2       mbp_meta_sonar:           %s\n", process.mbp_meta_sonar);
		fprintf(stderr, "dbg2       mbp_meta_sonarversion:    %s\n", process.mbp_meta_sonarversion);
		fprintf(stderr, "dbg2       mbp_meta_cruiseid:        %s\n", process.mbp_meta_cruiseid);
		fprintf(stderr, "dbg2       mbp_meta_cruisename:      %s\n", process.mbp_meta_cruisename);
		fprintf(stderr, "dbg2       mbp_meta_p:i              %s\n", process.mbp_meta_pi);
		fprintf(stderr, "dbg2       mbp_meta_piinstitution:   %s\n", process.mbp_meta_piinstitution);
		fprintf(stderr, "dbg2       mbp_meta_client:          %s\n", process.mbp_meta_client);
		fprintf(stderr, "dbg2       mbp_meta_svcorrected:     %d\n", process.mbp_meta_svcorrected);
		fprintf(stderr, "dbg2       mbp_meta_tidecorrected    %d\n", process.mbp_meta_tidecorrected);
		fprintf(stderr, "dbg2       mbp_meta_batheditmanual   %d\n", process.mbp_meta_batheditmanual);
		fprintf(stderr, "dbg2       mbp_meta_batheditauto:    %d\n", process.mbp_meta_batheditauto);
		fprintf(stderr, "dbg2       mbp_meta_rollbias:        %f\n", process.mbp_meta_rollbias);
		fprintf(stderr, "dbg2       mbp_meta_pitchbias:       %f\n", process.mbp_meta_pitchbias);
		fprintf(stderr, "dbg2       mbp_meta_headingbias:     %f\n", process.mbp_meta_headingbias);
		fprintf(stderr, "dbg2       mbp_meta_draft:           %f\n", process.mbp_meta_draft);
	}

	/* set metadata values */
	strcpy(process.mbp_meta_vessel, mbp_meta_vessel);
	strcpy(process.mbp_meta_institution, mbp_meta_institution);
	strcpy(process.mbp_meta_platform, mbp_meta_platform);
	strcpy(process.mbp_meta_sonar, mbp_meta_sonar);
	strcpy(process.mbp_meta_sonarversion, mbp_meta_sonarversion);
	strcpy(process.mbp_meta_cruiseid, mbp_meta_cruiseid);
	strcpy(process.mbp_meta_cruisename, mbp_meta_cruisename);
	strcpy(process.mbp_meta_pi, mbp_meta_pi);
	strcpy(process.mbp_meta_piinstitution, mbp_meta_piinstitution);
	strcpy(process.mbp_meta_client, mbp_meta_client);
	process.mbp_meta_svcorrected = mbp_meta_svcorrected;
	process.mbp_meta_tidecorrected = mbp_meta_tidecorrected;
	process.mbp_meta_batheditmanual = mbp_meta_batheditmanual;
	process.mbp_meta_batheditauto = mbp_meta_batheditauto;
	process.mbp_meta_rollbias = mbp_meta_rollbias;
	process.mbp_meta_pitchbias = mbp_meta_pitchbias;
	process.mbp_meta_headingbias = mbp_meta_headingbias;
	process.mbp_meta_draft = mbp_meta_draft;

	/* write new process parameter file */
	status &= mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:                    %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                   %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_kluges(int verbose, char *file, int mbp_kluge001, int mbp_kluge002, int mbp_kluge003, int mbp_kluge004,
                        int mbp_kluge005, int mbp_kluge006, int mbp_kluge007, int mbp_kluge008, int mbp_kluge009,
                        int mbp_kluge010, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                  %d\n", verbose);
		fprintf(stderr, "dbg2       file:                     %s\n", file);
		fprintf(stderr, "dbg2       mbp_kluge001:             %d\n", mbp_kluge001);
		fprintf(stderr, "dbg2       mbp_kluge002:             %d\n", mbp_kluge002);
		fprintf(stderr, "dbg2       mbp_kluge003:             %d\n", mbp_kluge003);
		fprintf(stderr, "dbg2       mbp_kluge004:             %d\n", mbp_kluge004);
		fprintf(stderr, "dbg2       mbp_kluge005:             %d\n", mbp_kluge005);
		fprintf(stderr, "dbg2       mbp_kluge006:             %d\n", mbp_kluge006);
		fprintf(stderr, "dbg2       mbp_kluge007:             %d\n", mbp_kluge007);
		fprintf(stderr, "dbg2       mbp_kluge008:             %d\n", mbp_kluge008);
		fprintf(stderr, "dbg2       mbp_kluge009:             %d\n", mbp_kluge009);
		fprintf(stderr, "dbg2       mbp_kluge010:             %d\n", mbp_kluge010);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set metadata values */
	process.mbp_kluge001 = mbp_kluge001;
	process.mbp_kluge002 = mbp_kluge002;
	process.mbp_kluge003 = mbp_kluge003;
	process.mbp_kluge004 = mbp_kluge004;
	process.mbp_kluge005 = mbp_kluge005;
	process.mbp_kluge006 = mbp_kluge006;
	process.mbp_kluge007 = mbp_kluge007;
	process.mbp_kluge008 = mbp_kluge008;
	process.mbp_kluge009 = mbp_kluge009;
	process.mbp_kluge010 = mbp_kluge010;

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:                    %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                   %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_ofile(int verbose, char *file, int *mbp_ofile_specified, char *mbp_ofile, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:             %d\n", verbose);
		fprintf(stderr, "dbg2       file:                %s\n", file);
	}

	/* this function looks for the output filename directly
	 * rather than by calling mb_pr_readpar() in order to
	 * speed up mbgrid and other programs that parse large
	 * datalists looking for processed files
	 */

	/* get expected process parameter file name */
	char parfile[MBP_FILENAMESIZE];
	strcpy(parfile, file);
	strcat(parfile, ".par");

	/* open and read parameter file */
	*mbp_ofile_specified = false;
	if (mbp_ofile != NULL)
		mbp_ofile[0] = '\0';

	FILE *fp = fopen(parfile, "r");
	if (fp != NULL) {
		char buffer[MBP_FILENAMESIZE];
		char dummy[MBP_FILENAMESIZE];
		char *result;
		while ((result = fgets(buffer, MBP_FILENAMESIZE, fp)) == buffer && !*mbp_ofile_specified) {
			if (strncmp(buffer, "OUTFILE", 7) == 0) {
				sscanf(buffer, "%s %s", dummy, mbp_ofile);
				*mbp_ofile_specified = true;
			}
		}

		fclose(fp);
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_ofile_specified: %d\n", *mbp_ofile_specified);
		fprintf(stderr, "dbg2       ofile:               %s\n", mbp_ofile);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_format(int verbose, char *file, int *mbp_format_specified, int *mbp_format, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:              %d\n", verbose);
		fprintf(stderr, "dbg2       file:                 %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set format value */
	*mbp_format_specified = process.mbp_format_specified;
	*mbp_format = process.mbp_format;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_format_specified: %d\n", *mbp_format_specified);
		fprintf(stderr, "dbg2       mbp_format:           %d\n", *mbp_format);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_rollbias(int verbose, char *file, int *mbp_rollbias_mode, double *mbp_rollbias, double *mbp_rollbias_port,
                       double *mbp_rollbias_stbd, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set rollbias values */
	*mbp_rollbias_mode = process.mbp_rollbias_mode;
	*mbp_rollbias = process.mbp_rollbias;
	*mbp_rollbias_port = process.mbp_rollbias_port;
	*mbp_rollbias_stbd = process.mbp_rollbias_stbd;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_rollbias_mode: %d\n", *mbp_rollbias_mode);
		fprintf(stderr, "dbg2       mbp_rollbias:      %f\n", *mbp_rollbias);
		fprintf(stderr, "dbg2       mbp_rollbias_port: %f\n", *mbp_rollbias_port);
		fprintf(stderr, "dbg2       mbp_rollbias_stbd: %f\n", *mbp_rollbias_stbd);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_pitchbias(int verbose, char *file, int *mbp_pitchbias_mode, double *mbp_pitchbias, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set pitchbias values */
	*mbp_pitchbias_mode = process.mbp_pitchbias_mode;
	*mbp_pitchbias = process.mbp_pitchbias;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_pitchbias_mode: %d\n", *mbp_pitchbias_mode);
		fprintf(stderr, "dbg2       mbp_pitchbias:      %f\n", *mbp_pitchbias);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_draft(int verbose, char *file, int *mbp_draft_mode, double *mbp_draft, double *mbp_draft_offset,
                    double *mbp_draft_mult, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set draft values */
	*mbp_draft_mode = process.mbp_draft_mode;
	*mbp_draft = process.mbp_draft;
	*mbp_draft_offset = process.mbp_draft_offset;
	*mbp_draft_mult = process.mbp_draft_mult;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_draft_mode:    %d\n", *mbp_draft_mode);
		fprintf(stderr, "dbg2       mbp_draft:         %f\n", *mbp_draft);
		fprintf(stderr, "dbg2       mbp_draft_offset:  %f\n", *mbp_draft_offset);
		fprintf(stderr, "dbg2       mbp_draft_mult:    %f\n", *mbp_draft_mult);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_heave(int verbose, char *file, int *mbp_heave_mode, double *mbp_heave, double *mbp_heave_mult, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set heave values */
	*mbp_heave_mode = process.mbp_heave_mode;
	*mbp_heave = process.mbp_heave;
	*mbp_heave_mult = process.mbp_heave_mult;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_heave_mode:    %d\n", *mbp_heave_mode);
		fprintf(stderr, "dbg2       mbp_heave:         %f\n", *mbp_heave);
		fprintf(stderr, "dbg2       mbp_heave_mult:    %f\n", *mbp_heave_mult);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_lever(int verbose, char *file, int *mbp_lever_mode, double *mbp_vru_offsetx, double *mbp_vru_offsety,
                    double *mbp_vru_offsetz, double *mbp_sonar_offsetx, double *mbp_sonar_offsety, double *mbp_sonar_offsetz,
                    int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set lever values */
	*mbp_lever_mode = process.mbp_lever_mode;
	*mbp_vru_offsetx = process.mbp_vru_offsetx;
	*mbp_vru_offsety = process.mbp_vru_offsety;
	*mbp_vru_offsetz = process.mbp_vru_offsetz;
	*mbp_sonar_offsetx = process.mbp_sonar_offsetx;
	*mbp_sonar_offsety = process.mbp_sonar_offsety;
	*mbp_sonar_offsetz = process.mbp_sonar_offsetz;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_lever_mode:    %d\n", *mbp_lever_mode);
		fprintf(stderr, "dbg2       mbp_vru_offsetx:   %f\n", *mbp_vru_offsetx);
		fprintf(stderr, "dbg2       mbp_vru_offsety:   %f\n", *mbp_vru_offsety);
		fprintf(stderr, "dbg2       mbp_vru_offsetz:   %f\n", *mbp_vru_offsetz);
		fprintf(stderr, "dbg2       mbp_sonar_offsetx:   %f\n", *mbp_sonar_offsetx);
		fprintf(stderr, "dbg2       mbp_sonar_offsety:   %f\n", *mbp_sonar_offsety);
		fprintf(stderr, "dbg2       mbp_sonar_offsetz:   %f\n", *mbp_sonar_offsetz);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_tide(int verbose, char *file, int *mbp_tide_mode, char *mbp_tidefile, int *mbp_tide_format, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set lever values */
	*mbp_tide_mode = process.mbp_tide_mode;
	if (mbp_tidefile != NULL)
		strcpy(mbp_tidefile, process.mbp_tidefile);
	*mbp_tide_format = process.mbp_tide_format;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_tide_mode:     %d\n", *mbp_tide_mode);
		fprintf(stderr, "dbg2       mbp_tidefile:      %s\n", mbp_tidefile);
		fprintf(stderr, "dbg2       mbp_tide_format:   %d\n", *mbp_tide_format);
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:            %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_tt(int verbose, char *file, int *mbp_tt_mode, double *mbp_tt_mult, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set tt values */
	*mbp_tt_mode = process.mbp_tt_mode;
	*mbp_tt_mult = process.mbp_tt_mult;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_tt_mode:       %d\n", *mbp_tt_mode);
		fprintf(stderr, "dbg2       mbp_tt_mult:       %f\n", *mbp_tt_mult);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_ssv(int verbose, char *file, int *mbp_ssv_mode, double *mbp_ssv, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set ssv values */
	*mbp_ssv_mode = process.mbp_ssv_mode;
	*mbp_ssv = process.mbp_ssv;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_ssv_mode:      %d\n", *mbp_ssv_mode);
		fprintf(stderr, "dbg2       mbp_ssv:           %f\n", *mbp_ssv);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_svp(int verbose, char *file, int *mbp_svp_mode, char *mbp_svpfile, int *mbp_angle_mode, int *mbp_corrected,
                  int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set svp values */
	*mbp_svp_mode = process.mbp_svp_mode;
	if (mbp_svpfile != NULL)
		strcpy(mbp_svpfile, process.mbp_svpfile);
	*mbp_angle_mode = process.mbp_angle_mode;
	*mbp_corrected = process.mbp_corrected;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_svp_mode:      %d\n", *mbp_svp_mode);
		fprintf(stderr, "dbg2       mbp_svpfile:       %s\n", mbp_svpfile);
		fprintf(stderr, "dbg2       mbp_angle_mode:    %d\n", *mbp_angle_mode);
		fprintf(stderr, "dbg2       mbp_corrected:     %d\n", *mbp_corrected);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_static(int verbose, char *file, int *mbp_static_mode, char *mbp_staticfile, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set svp values */
	*mbp_static_mode = process.mbp_static_mode;
	if (mbp_staticfile != NULL)
		strcpy(mbp_staticfile, process.mbp_staticfile);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_static_mode:   %d\n", *mbp_static_mode);
		fprintf(stderr, "dbg2       mbp_staticfile:    %s\n", mbp_staticfile);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_navadj(int verbose, char *file, int *mbp_navadj_mode, char *mbp_navadjfile, int *mbp_navadj_algorithm, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:              %d\n", verbose);
		fprintf(stderr, "dbg2       file:                 %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set navadj values */
	*mbp_navadj_mode = process.mbp_navadj_mode;
	if (mbp_navadjfile != NULL)
		strcpy(mbp_navadjfile, process.mbp_navadjfile);
	*mbp_navadj_algorithm = process.mbp_navadj_algorithm;

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_navadj_mode:      %d\n", *mbp_navadj_mode);
		fprintf(stderr, "dbg2       mbp_navadjfile:       %s\n", mbp_navadjfile);
		fprintf(stderr, "dbg2       mbp_navadj_algorithm: %d\n", *mbp_navadj_algorithm);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_attitude(int verbose, char *file, int *mbp_attitude_mode, char *mbp_attitudefile, int *mbp_attitude_format,
                       int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set lever values */
	*mbp_attitude_mode = process.mbp_attitude_mode;
	if (mbp_attitudefile != NULL)
		strcpy(mbp_attitudefile, process.mbp_attitudefile);
	*mbp_attitude_format = process.mbp_attitude_format;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_attitude_mode: %d\n", *mbp_attitude_mode);
		fprintf(stderr, "dbg2       mbp_attitudefile:  %s\n", mbp_attitudefile);
		fprintf(stderr, "dbg2       mbp_attitude_format:%d\n", *mbp_attitude_format);
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:            %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_sensordepth(int verbose, char *file, int *mbp_sensordepth_mode, char *mbp_sensordepthfile, int *mbp_sensordepth_format,
                         int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set lever values */
	*mbp_sensordepth_mode = process.mbp_sensordepth_mode;
	if (mbp_sensordepthfile != NULL)
		strcpy(mbp_sensordepthfile, process.mbp_sensordepthfile);
	*mbp_sensordepth_format = process.mbp_sensordepth_format;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_sensordepth_mode:   %d\n", *mbp_sensordepth_mode);
		fprintf(stderr, "dbg2       mbp_sensordepthfile:    %s\n", mbp_sensordepthfile);
		fprintf(stderr, "dbg2       mbp_sensordepth_format: %d\n", *mbp_sensordepth_format);
		fprintf(stderr, "dbg2       error:                 %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_nav(int verbose, char *file, int *mbp_nav_mode, char *mbp_navfile, int *mbp_nav_format, int *mbp_nav_heading,
                  int *mbp_nav_speed, int *mbp_nav_draft, int *mbp_nav_attitude, int *mbp_nav_algorithm,
                  double *mbp_nav_timeshift, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set nav values */
	*mbp_nav_mode = process.mbp_nav_mode;
	if (mbp_navfile != NULL)
		strcpy(mbp_navfile, process.mbp_navfile);
	*mbp_nav_format = process.mbp_nav_format;
	*mbp_nav_heading = process.mbp_nav_heading;
	*mbp_nav_speed = process.mbp_nav_speed;
	*mbp_nav_draft = process.mbp_nav_draft;
	*mbp_nav_attitude = process.mbp_nav_attitude;
	*mbp_nav_algorithm = process.mbp_nav_algorithm;
	*mbp_nav_timeshift = process.mbp_nav_timeshift;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_nav_mode:      %d\n", *mbp_nav_mode);
		fprintf(stderr, "dbg2       mbp_navfile:       %s\n", mbp_navfile);
		fprintf(stderr, "dbg2       mbp_nav_format:    %d\n", *mbp_nav_format);
		fprintf(stderr, "dbg2       mbp_nav_heading:   %d\n", *mbp_nav_heading);
		fprintf(stderr, "dbg2       mbp_nav_speed:     %d\n", *mbp_nav_speed);
		fprintf(stderr, "dbg2       mbp_nav_draft:     %d\n", *mbp_nav_draft);
		fprintf(stderr, "dbg2       mbp_nav_attitude:  %d\n", *mbp_nav_attitude);
		fprintf(stderr, "dbg2       mbp_nav_algorithm: %d\n", *mbp_nav_algorithm);
		fprintf(stderr, "dbg2       mbp_nav_timeshift: %f\n", *mbp_nav_timeshift);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_navshift(int verbose, char *file, int *mbp_nav_shift, double *mbp_nav_offsetx, double *mbp_nav_offsety,
                       double *mbp_nav_offsetz, double *mbp_nav_shiftlon, double *mbp_nav_shiftlat, double *mbp_nav_shiftx,
                       double *mbp_nav_shifty, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set nav values */
	*mbp_nav_shift = process.mbp_nav_shift;
	*mbp_nav_offsetx = process.mbp_nav_offsetx;
	*mbp_nav_offsety = process.mbp_nav_offsety;
	*mbp_nav_offsetz = process.mbp_nav_offsetz;
	*mbp_nav_shiftlon = process.mbp_nav_shiftlon;
	*mbp_nav_shiftlat = process.mbp_nav_shiftlat;
	*mbp_nav_shiftx = process.mbp_nav_shiftx;
	*mbp_nav_shifty = process.mbp_nav_shifty;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_nav_shift:     %d\n", *mbp_nav_shift);
		fprintf(stderr, "dbg2       mbp_nav_offsetx:   %f\n", *mbp_nav_offsetx);
		fprintf(stderr, "dbg2       mbp_nav_offsety:   %f\n", *mbp_nav_offsety);
		fprintf(stderr, "dbg2       mbp_nav_offsetz:   %f\n", *mbp_nav_offsetz);
		fprintf(stderr, "dbg2       mbp_nav_shiftlon:  %f\n", *mbp_nav_shiftlon);
		fprintf(stderr, "dbg2       mbp_nav_shiftlat:  %f\n", *mbp_nav_shiftlat);
		fprintf(stderr, "dbg2       mbp_nav_shiftx:    %f\n", *mbp_nav_shiftx);
		fprintf(stderr, "dbg2       mbp_nav_shifty:    %f\n", *mbp_nav_shifty);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
} /*--------------------------------------------------------------------*/
int mb_pr_get_heading(int verbose, char *file, int *mbp_heading_mode, double *mbp_headingbias, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set heading values */
	*mbp_heading_mode = process.mbp_heading_mode;
	*mbp_headingbias = process.mbp_headingbias;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);

		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_heading_mode:  %d\n", *mbp_heading_mode);
		fprintf(stderr, "dbg2       mbp_headingbias:   %f\n", *mbp_headingbias);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_datacut(int verbose, char *file, int *mbp_cut_num, int *mbp_cut_kind, int *mbp_cut_mode, double *mbp_cut_min,
                      double *mbp_cut_max, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set datacut values */
	*mbp_cut_num = process.mbp_cut_num;
	for (int i = 0; i < *mbp_cut_num; i++) {
		mbp_cut_kind[i] = process.mbp_cut_kind[i];
		mbp_cut_mode[i] = process.mbp_cut_mode[i];
		mbp_cut_min[i] = process.mbp_cut_min[i];
		mbp_cut_max[i] = process.mbp_cut_max[i];
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_cut_num:        %d\n", *mbp_cut_num);
		for (int i = 0; i < *mbp_cut_num; i++) {
			fprintf(stderr, "dbg2       mbp_cut_kind[%d]:   %d\n", i, mbp_cut_kind[i]);
			fprintf(stderr, "dbg2       mbp_cut_mode[%d]:   %d\n", i, mbp_cut_mode[i]);
			fprintf(stderr, "dbg2       mbp_cut_min[%d]:    %f\n", i, mbp_cut_min[i]);
			fprintf(stderr, "dbg2       mbp_cut_max[%d]:    %f\n", i, mbp_cut_max[i]);
		}
		fprintf(stderr, "dbg2       error:              %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:             %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_edit(int verbose, char *file, int *mbp_edit_mode, char *mbp_editfile, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set edit values */
	*mbp_edit_mode = process.mbp_edit_mode;
	if (mbp_editfile != NULL)
		strcpy(mbp_editfile, process.mbp_editfile);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_edit_mode:     %d\n", *mbp_edit_mode);
		fprintf(stderr, "dbg2       mbp_editfile:      %s\n", mbp_editfile);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_ampcorr(int verbose, char *file, int *mbp_ampcorr_mode, char *mbp_ampcorrfile, int *mbp_ampcorr_type,
                      int *mbp_ampcorr_symmetry, double *mbp_ampcorr_angle, int *mbp_ampcorr_slope, char *mbp_ampsscorr_topofile,
                      int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set ssrecalc values */
	*mbp_ampcorr_mode = process.mbp_ampcorr_mode;
	if (mbp_ampcorrfile != NULL)
		strcpy(mbp_ampcorrfile, process.mbp_ampcorrfile);
	*mbp_ampcorr_type = process.mbp_ampcorr_type;
	*mbp_ampcorr_symmetry = process.mbp_ampcorr_symmetry;
	*mbp_ampcorr_angle = process.mbp_ampcorr_angle;
	*mbp_ampcorr_slope = process.mbp_ampcorr_slope;
	if (mbp_ampsscorr_topofile != NULL)
		strcpy(mbp_ampsscorr_topofile, process.mbp_ampsscorr_topofile);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_ampcorr_mode:         %d\n", *mbp_ampcorr_mode);
		fprintf(stderr, "dbg2       mbp_ampcorrfile:          %s\n", mbp_ampcorrfile);
		fprintf(stderr, "dbg2       mbp_ampcorr_type:         %d\n", *mbp_ampcorr_type);
		fprintf(stderr, "dbg2       mbp_ampcorr_symmetry:     %d\n", *mbp_ampcorr_symmetry);
		fprintf(stderr, "dbg2       mbp_ampcorr_angle:        %f\n", *mbp_ampcorr_angle);
		fprintf(stderr, "dbg2       mbp_ampcorr_slope:        %d\n", *mbp_ampcorr_slope);
		fprintf(stderr, "dbg2       mbp_ampsscorr_topofile:   %s\n", mbp_ampsscorr_topofile);
		fprintf(stderr, "dbg2       error:                    %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                   %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_sscorr(int verbose, char *file, int *mbp_sscorr_mode, char *mbp_sscorrfile, int *mbp_sscorr_type,
                     int *mbp_sscorr_symmetry, double *mbp_sscorr_angle, int *mbp_sscorr_slope, char *mbp_ampsscorr_topofile,
                     int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set ssrecalc values */
	*mbp_sscorr_mode = process.mbp_sscorr_mode;
	if (mbp_sscorrfile != NULL)
		strcpy(mbp_sscorrfile, process.mbp_sscorrfile);
	*mbp_sscorr_type = process.mbp_sscorr_type;
	*mbp_sscorr_symmetry = process.mbp_sscorr_symmetry;
	*mbp_sscorr_angle = process.mbp_sscorr_angle;
	*mbp_sscorr_slope = process.mbp_sscorr_slope;
	if (mbp_ampsscorr_topofile != NULL)
		strcpy(mbp_ampsscorr_topofile, process.mbp_ampsscorr_topofile);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_sscorr_mode:          %d\n", *mbp_sscorr_mode);
		fprintf(stderr, "dbg2       mbp_sscorrfile:           %s\n", mbp_sscorrfile);
		fprintf(stderr, "dbg2       mbp_sscorr_type:          %d\n", *mbp_sscorr_type);
		fprintf(stderr, "dbg2       mbp_sscorr_symmetry:      %d\n", *mbp_sscorr_symmetry);
		fprintf(stderr, "dbg2       mbp_sscorr_angle:         %f\n", *mbp_sscorr_angle);
		fprintf(stderr, "dbg2       mbp_sscorr_slope:         %d\n", *mbp_sscorr_slope);
		fprintf(stderr, "dbg2       mbp_ampsscorr_topofile:   %s\n", mbp_ampsscorr_topofile);
		fprintf(stderr, "dbg2       error:                    %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                   %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_ssrecalc(int verbose, char *file, int *mbp_ssrecalc_mode, double *mbp_ssrecalc_pixelsize,
                       double *mbp_ssrecalc_swathwidth, int *mbp_ssrecalc_interpolate, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set ssrecalc values */
	*mbp_ssrecalc_mode = process.mbp_ssrecalc_mode;
	*mbp_ssrecalc_pixelsize = process.mbp_ssrecalc_pixelsize;
	*mbp_ssrecalc_swathwidth = process.mbp_ssrecalc_swathwidth;
	*mbp_ssrecalc_interpolate = process.mbp_ssrecalc_interpolate;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_ssrecalc_mode:        %d\n", *mbp_ssrecalc_mode);
		fprintf(stderr, "dbg2       mbp_ssrecalc_pixelsize:   %f\n", *mbp_ssrecalc_pixelsize);
		fprintf(stderr, "dbg2       mbp_ssrecalc_swathwidth:  %f\n", *mbp_ssrecalc_swathwidth);
		fprintf(stderr, "dbg2       mbp_ssrecalc_interpolate: %d\n", *mbp_ssrecalc_interpolate);
		fprintf(stderr, "dbg2       error:                    %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                   %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_metadata(int verbose, char *file, char *mbp_meta_vessel, char *mbp_meta_institution, char *mbp_meta_platform,
                       char *mbp_meta_sonar, char *mbp_meta_sonarversion, char *mbp_meta_cruiseid, char *mbp_meta_cruisename,
                       char *mbp_meta_pi, char *mbp_meta_piinstitution, char *mbp_meta_client, int *mbp_meta_svcorrected,
                       int *mbp_meta_tidecorrected, int *mbp_meta_batheditmanual, int *mbp_meta_batheditauto,
                       double *mbp_meta_rollbias, double *mbp_meta_pitchbias, double *mbp_meta_headingbias,
                       double *mbp_meta_draft, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set metadata values */
	strcpy(mbp_meta_vessel, process.mbp_meta_vessel);
	strcpy(mbp_meta_institution, process.mbp_meta_institution);
	strcpy(mbp_meta_platform, process.mbp_meta_platform);
	strcpy(mbp_meta_sonar, process.mbp_meta_sonar);
	strcpy(mbp_meta_sonarversion, process.mbp_meta_sonarversion);
	strcpy(mbp_meta_cruiseid, process.mbp_meta_cruiseid);
	strcpy(mbp_meta_cruisename, process.mbp_meta_cruisename);
	strcpy(mbp_meta_pi, process.mbp_meta_pi);
	strcpy(mbp_meta_piinstitution, process.mbp_meta_piinstitution);
	strcpy(mbp_meta_client, process.mbp_meta_client);
	*mbp_meta_svcorrected = process.mbp_meta_svcorrected;
	*mbp_meta_tidecorrected = process.mbp_meta_tidecorrected;
	*mbp_meta_batheditmanual = process.mbp_meta_batheditmanual;
	*mbp_meta_batheditauto = process.mbp_meta_batheditauto;
	*mbp_meta_rollbias = process.mbp_meta_rollbias;
	*mbp_meta_pitchbias = process.mbp_meta_pitchbias;
	*mbp_meta_headingbias = process.mbp_meta_headingbias;
	*mbp_meta_draft = process.mbp_meta_draft;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_meta_vessel:          %s\n", mbp_meta_vessel);
		fprintf(stderr, "dbg2       mbp_meta_institution:     %s\n", mbp_meta_institution);
		fprintf(stderr, "dbg2       mbp_meta_platform:        %s\n", mbp_meta_platform);
		fprintf(stderr, "dbg2       mbp_meta_sonar:           %s\n", mbp_meta_sonar);
		fprintf(stderr, "dbg2       mbp_meta_sonarversion:    %s\n", mbp_meta_sonarversion);
		fprintf(stderr, "dbg2       mbp_meta_cruiseid:        %s\n", mbp_meta_cruiseid);
		fprintf(stderr, "dbg2       mbp_meta_cruisename:      %s\n", mbp_meta_cruisename);
		fprintf(stderr, "dbg2       mbp_meta_p:i              %s\n", mbp_meta_pi);
		fprintf(stderr, "dbg2       mbp_meta_piinstitution:   %s\n", mbp_meta_piinstitution);
		fprintf(stderr, "dbg2       mbp_meta_client:          %s\n", mbp_meta_client);
		fprintf(stderr, "dbg2       mbp_meta_svcorrected:     %d\n", *mbp_meta_svcorrected);
		fprintf(stderr, "dbg2       mbp_meta_tidecorrected    %d\n", *mbp_meta_tidecorrected);
		fprintf(stderr, "dbg2       mbp_meta_batheditmanual   %d\n", *mbp_meta_batheditmanual);
		fprintf(stderr, "dbg2       mbp_meta_batheditauto:    %d\n", *mbp_meta_batheditauto);
		fprintf(stderr, "dbg2       mbp_meta_rollbias:        %f\n", *mbp_meta_rollbias);
		fprintf(stderr, "dbg2       mbp_meta_pitchbias:       %f\n", *mbp_meta_pitchbias);
		fprintf(stderr, "dbg2       mbp_meta_headingbias:     %f\n", *mbp_meta_headingbias);
		fprintf(stderr, "dbg2       mbp_meta_draft:           %f\n", *mbp_meta_draft);
		fprintf(stderr, "dbg2       error:                    %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                   %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_kluges(int verbose, char *file, int *mbp_kluge001, int *mbp_kluge002, int *mbp_kluge003, int *mbp_kluge004,
                     int *mbp_kluge005, int *mbp_kluge006, int *mbp_kluge007, int *mbp_kluge008, int *mbp_kluge009,
                     int *mbp_kluge010, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       file:              %s\n", file);
	}

	/* get known process parameters */
	struct mb_process_struct process;
	int status = mb_pr_readpar(verbose, file, true, &process, error);

	/* set metadata values */
	*mbp_kluge001 = process.mbp_kluge001;
	*mbp_kluge002 = process.mbp_kluge002;
	*mbp_kluge003 = process.mbp_kluge003;
	*mbp_kluge004 = process.mbp_kluge004;
	*mbp_kluge005 = process.mbp_kluge005;
	*mbp_kluge006 = process.mbp_kluge006;
	*mbp_kluge007 = process.mbp_kluge007;
	*mbp_kluge008 = process.mbp_kluge008;
	*mbp_kluge009 = process.mbp_kluge009;
	*mbp_kluge010 = process.mbp_kluge010;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       mbp_kluge001:             %d\n", *mbp_kluge001);
		fprintf(stderr, "dbg2       mbp_kluge002:             %d\n", *mbp_kluge002);
		fprintf(stderr, "dbg2       mbp_kluge003:             %d\n", *mbp_kluge003);
		fprintf(stderr, "dbg2       mbp_kluge004:             %d\n", *mbp_kluge004);
		fprintf(stderr, "dbg2       mbp_kluge005:             %d\n", *mbp_kluge005);
		fprintf(stderr, "dbg2       mbp_kluge006:             %d\n", *mbp_kluge006);
		fprintf(stderr, "dbg2       mbp_kluge007:             %d\n", *mbp_kluge007);
		fprintf(stderr, "dbg2       mbp_kluge008:             %d\n", *mbp_kluge008);
		fprintf(stderr, "dbg2       mbp_kluge009:             %d\n", *mbp_kluge009);
		fprintf(stderr, "dbg2       mbp_kluge010:             %d\n", *mbp_kluge010);
		fprintf(stderr, "dbg2       error:                    %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                   %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_set_bathyslopenew(int verbose, int nsmooth, int nbath, char *beamflag, double *bath, double *bathacrosstrack,
                            int *ndepths, double *depths, double *depthacrosstrack, int *nslopes, double *slopes,
                            double *slopeacrosstrack, double *depthsmooth, int *error) {
	(void)depthsmooth;  // unused

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       nsmooth:         %d\n", nsmooth);
		fprintf(stderr, "dbg2       nbath:           %d\n", nbath);
		fprintf(stderr, "dbg2       beamflag:        %p\n", (void *)beamflag);
		fprintf(stderr, "dbg2       bath:            %p\n", (void *)bath);
		fprintf(stderr, "dbg2       bathacrosstrack: %p\n", (void *)bathacrosstrack);
		fprintf(stderr, "dbg2       bath:\n");
		for (int i = 0; i < nbath; i++)
			fprintf(stderr, "dbg2         %d  %d  %f %f\n", i, beamflag[i], bath[i], bathacrosstrack[i]);
		fprintf(stderr, "dbg2       depths:           %p\n", (void *)depths);
		fprintf(stderr, "dbg2       depthacrosstrack: %p\n", (void *)depthacrosstrack);
		fprintf(stderr, "dbg2       slopes:           %p\n", (void *)slopes);
		fprintf(stderr, "dbg2       slopeacrosstrack: %p\n", (void *)slopeacrosstrack);
	}

	/* initialize depths */
	*ndepths = 0;
	for (int i = 0; i < nbath; i++) {
		depths[i] = 0.0;
		depthacrosstrack[i] = 0.0;
	}

	/* decimate by nsmooth, averaging the values used */
	for (int i = 0; i <= nbath / nsmooth; i++) {
		const int j1 = i * nsmooth;
		const int j2 = MIN((i + 1) * nsmooth, nbath);
		depths[*ndepths] = 0.0;
		depthacrosstrack[*ndepths] = 0.0;
		double weight = 0.0;
		for (int j = j1; j < j2; j++) {
			if (mb_beam_ok(beamflag[j])) {
				depths[*ndepths] += bath[j];
				depthacrosstrack[*ndepths] += bathacrosstrack[j];
				weight += 1.0;
			}
		}
		if (weight > 0.0) {
			depths[*ndepths] /= weight;
			depthacrosstrack[*ndepths] /= weight;
			(*ndepths) += 1;
		}
	}

	/* now calculate slopes */
	if (*ndepths > 0) {
		*nslopes = *ndepths + 1;
		slopeacrosstrack[0] = depthacrosstrack[0];
		slopes[0] = 0.0;
		for (int i = 1; i < *ndepths; i++) {
			const double dxtrack = depthacrosstrack[i] - depthacrosstrack[i - 1];
			slopeacrosstrack[i] = depthacrosstrack[i - 1] + 0.5 * dxtrack;
			if (dxtrack > 0.0)
				slopes[i] = (depths[i] - depths[i - 1]) / dxtrack;
			else
				slopes[i] = 0.0;
		}
		slopeacrosstrack[*ndepths] = depthacrosstrack[*ndepths - 1];
		slopes[*ndepths] = 0.0;
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ndepths:         %d\n", *ndepths);
		fprintf(stderr, "dbg2       depths:\n");
		for (int i = 0; i < *ndepths; i++)
			fprintf(stderr, "dbg2         %d %f %f\n", i, depths[i], depthacrosstrack[i]);
		fprintf(stderr, "dbg2       nslopes:         %d\n", *nslopes);
		fprintf(stderr, "dbg2       slopes:\n");
		for (int i = 0; i < *nslopes; i++)
			fprintf(stderr, "dbg2         %d %f %f\n", i, slopes[i], slopeacrosstrack[i]);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_set_bathyslope(int verbose, int nsmooth, int nbath, char *beamflag, double *bath, double *bathacrosstrack, int *ndepths,
                         double *depths, double *depthacrosstrack, int *nslopes, double *slopes, double *slopeacrosstrack,
                         double *depthsmooth, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       nbath:           %d\n", nbath);
		fprintf(stderr, "dbg2       beamflag:        %p\n", (void *)beamflag);
		fprintf(stderr, "dbg2       bath:            %p\n", (void *)bath);
		fprintf(stderr, "dbg2       bathacrosstrack: %p\n", (void *)bathacrosstrack);
		fprintf(stderr, "dbg2       bath:\n");
		for (int i = 0; i < nbath; i++)
			fprintf(stderr, "dbg2         %d  %d  %f %f\n", i, beamflag[i], bath[i], bathacrosstrack[i]);
		fprintf(stderr, "dbg2       depths:           %p\n", (void *)depths);
		fprintf(stderr, "dbg2       depthacrosstrack: %p\n", (void *)depthacrosstrack);
		fprintf(stderr, "dbg2       slopes:           %p\n", (void *)slopes);
		fprintf(stderr, "dbg2       slopeacrosstrack: %p\n", (void *)slopeacrosstrack);
	}

	/* initialize depths */
	*ndepths = 0;
	for (int i = 0; i < nbath; i++) {
		depths[i] = 0.0;
		depthacrosstrack[i] = 0.0;
	}

	/* Fill in the existing depths */
	int first = -1;
	int last = -1;
	int nbathgood = 0;
	for (int i = 0; i < nbath; i++) {
		if (mb_beam_ok(beamflag[i])) {
			if (first == -1) {
				first = i;
			}
			last = i;
			depths[i] = bath[i];
			depthacrosstrack[i] = bathacrosstrack[i];
			nbathgood++;
		}
	}

	/* now interpolate the depths */
	if (nbathgood > 0)
		for (int i = first; i < last; i++) {
			if (mb_beam_ok(beamflag[i])) {
				int next = i;
				int j = i + 1;
				while (next == i && j < nbath) {
					if (mb_beam_ok(beamflag[j]))
						next = j;
					else
						j++;
				}
				if (next > i) {
					// TODO(schwehr): This j is not related to the j above.
					for (j = i + 1; j < next; j++) {
						const double factor = ((double)(j - i)) / ((double)(next - i));
						depths[j] = bath[i] + factor * (bath[next] - bath[i]);
						depthacrosstrack[j] = bathacrosstrack[i] + factor * (bathacrosstrack[next] - bathacrosstrack[i]);
					}
				}
			}
		}

	/* now smooth the depths */
	if (nbathgood > 0 && nsmooth > 0) {
		for (int i = first; i <= last; i++) {
			int j1 = i - nsmooth;
			int j2 = i + nsmooth;
			if (j1 < first)
				j1 = first;
			if (j2 > last)
				j2 = last;
			double depthsum = 0.0;
			for (int j = j1; j <= j2; j++) {
				depthsum += depths[j];
			}
			if (depthsum > 0.0)
				depthsmooth[i] = depthsum / ((double)(j2 - j1 + 1));
			else
				depthsmooth[i] = depths[i];
		}
		for (int i = first; i <= last; i++)
			depths[i] = depthsmooth[i];
	}

	/* now extrapolate the depths at the ends of the swath */
	if (nbathgood > 0) {
		*ndepths = nbath;
		double dacrosstrack;
		if (last - first > 0)
			dacrosstrack = (depthacrosstrack[last] - depthacrosstrack[first]) / (last - first);
		else
			dacrosstrack = 1.0;
		for (int i = 0; i < first; i++) {
			depths[i] = depths[first];
			depthacrosstrack[i] = depthacrosstrack[first] + dacrosstrack * (i - first);
		}
		for (int i = last + 1; i < nbath; i++) {
			depths[i] = depths[last];
			depthacrosstrack[i] = depthacrosstrack[last] + dacrosstrack * (i - last);
		}
	}

	/* now calculate slopes */
	if (nbathgood > 0) {
		*nslopes = nbath + 1;
		for (int i = 0; i < nbath - 1; i++) {
			slopes[i + 1] = (depths[i + 1] - depths[i]) / (depthacrosstrack[i + 1] - depthacrosstrack[i]);
			slopeacrosstrack[i + 1] = 0.5 * (depthacrosstrack[i + 1] + depthacrosstrack[i]);
		}
		slopes[0] = 0.0;
		slopeacrosstrack[0] = depthacrosstrack[0];
		slopes[nbath] = 0.0;
		slopeacrosstrack[nbath] = depthacrosstrack[nbath - 1];
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ndepths:         %d\n", *ndepths);
		fprintf(stderr, "dbg2       depths:\n");
		for (int i = 0; i < nbath; i++)
			fprintf(stderr, "dbg2         %d %f %f\n", i, depths[i], depthacrosstrack[i]);
		fprintf(stderr, "dbg2       nslopes:         %d\n", *nslopes);
		fprintf(stderr, "dbg2       slopes:\n");
		for (int i = 0; i < *nslopes; i++)
			fprintf(stderr, "dbg2         %d %f %f\n", i, slopes[i], slopeacrosstrack[i]);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_bathyslope(int verbose, int ndepths, double *depths, double *depthacrosstrack, int nslopes, double *slopes,
                         double *slopeacrosstrack, double acrosstrack, double *depth, double *slope, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       ndepths:         %d\n", ndepths);
		fprintf(stderr, "dbg2       depths:\n");
		for (int i = 0; i < ndepths; i++)
			fprintf(stderr, "dbg2         %d %f %f\n", i, depths[i], depthacrosstrack[i]);
		fprintf(stderr, "dbg2       nslopes:         %d\n", nslopes);
		fprintf(stderr, "dbg2       slopes:\n");
		for (int i = 0; i < nslopes; i++)
			fprintf(stderr, "dbg2         %d %f %f\n", i, slopes[i], slopeacrosstrack[i]);
		fprintf(stderr, "dbg2       acrosstrack:     %f\n", acrosstrack);
	}

	/* check if acrosstrack is in defined interval */
	bool found_depth = false;
	bool found_slope = false;
	if (ndepths > 1) {
		if (acrosstrack < depthacrosstrack[0]) {
			*depth = depths[0];
			*slope = 0.0;
			found_depth = true;
			found_slope = true;
		}

		else if (acrosstrack > depthacrosstrack[ndepths - 1]) {
			*depth = depths[ndepths - 1];
			*slope = 0.0;
			found_depth = true;
			found_slope = true;
		}

		else if (acrosstrack >= depthacrosstrack[0] && acrosstrack <= depthacrosstrack[ndepths - 1]) {

			/* look for depth */
			int idepth = -1;
			while (!found_depth && idepth < ndepths - 2) {
				idepth++;
				if (acrosstrack >= depthacrosstrack[idepth] && acrosstrack <= depthacrosstrack[idepth + 1]) {
					*depth = depths[idepth] + (acrosstrack - depthacrosstrack[idepth]) /
					                              (depthacrosstrack[idepth + 1] - depthacrosstrack[idepth]) *
					                              (depths[idepth + 1] - depths[idepth]);
					found_depth = true;
					*error = MB_ERROR_NO_ERROR;
				}
			}

			/* look for slope */
			int islope = -1;
			while (!found_slope && islope < nslopes - 2) {
				islope++;
				if (acrosstrack >= slopeacrosstrack[islope] && acrosstrack <= slopeacrosstrack[islope + 1]) {
					*slope = slopes[islope] + (acrosstrack - slopeacrosstrack[islope]) /
					                              (slopeacrosstrack[islope + 1] - slopeacrosstrack[islope]) *
					                              (slopes[islope + 1] - slopes[islope]);
					found_slope = true;
					*error = MB_ERROR_NO_ERROR;
				}
			}
		}
	}

	int status = MB_SUCCESS;

	/* check for failure */
	if (found_depth != true || found_slope != true) {
		status = MB_FAILURE;
		*error = MB_ERROR_OTHER;
		*depth = 0.0;
		*slope = 0.0;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       depth:           %f\n", *depth);
		fprintf(stderr, "dbg2       slope:           %f\n", *slope);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_point_in_quad(int verbose, double px, double py, double *x, double *y, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       px:              %f\n", px);
		fprintf(stderr, "dbg2       py:              %f\n", px);
		fprintf(stderr, "dbg2       x[0]: %f   y[0]: %f\n", x[0], y[0]);
		fprintf(stderr, "dbg2       x[1]: %f   y[1]: %f\n", x[1], y[1]);
		fprintf(stderr, "dbg2       x[2]: %f   y[2]: %f\n", x[2], y[2]);
		fprintf(stderr, "dbg2       x[3]: %f   y[3]: %f\n", x[3], y[3]);
	}

	/* check if point is inside defined quadrilateral
	    - the quad should be defined by four points
	    in counterclockwise order
	    - this is accomplished by calculating the
	    z component of the cross product of the vector from
	    each quad point to the next with the vector from the
	    quad point to the candidate point - if all four cross
	    product z components are positive, the point is inside
	    the quad */

	double ax = x[1] - x[0];
	double ay = y[1] - y[0];
	double bx = px - x[0];
	double by = py - y[0];
	const double z1 = ax * by - ay * bx;

	ax = x[2] - x[1];
	ay = y[2] - y[1];
	bx = px - x[1];
	by = py - y[1];
	const double z2 = ax * by - ay * bx;

	ax = x[3] - x[2];
	ay = y[3] - y[2];
	bx = px - x[2];
	by = py - y[2];
	const double z3 = ax * by - ay * bx;

	ax = x[0] - x[3];
	ay = y[0] - y[3];
	bx = px - x[3];
	by = py - y[3];
	const double z4 = ax * by - ay * bx;

	const double z = z1 * z2 * z3 * z4;
	bool inside = true;
	if (z <= 0.0)
		inside = false;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       inside:          %d\n", inside);
	}

	return (inside);
}
/*--------------------------------------------------------------------*/

int mb_pr_lockswathfile(int verbose, const char *file, int purpose, const char *program_name, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       file:       %s\n", file);
		fprintf(stderr, "dbg2       program_name: %s\n", program_name);
		fprintf(stderr, "dbg2       purpose:    %d\n", purpose);
	}

	int status = MB_SUCCESS;

	/* proceed only if lock file does not exist */
	mb_path lockfile;
	sprintf(lockfile, "%s.lck", file);

	struct stat file_status;
	const int fstat = stat(lockfile, &file_status);
	if (fstat == -1) {
		/* proceed only if we create the lockfile */
		FILE *fp = fopen(lockfile, "wx");
		if (fp != NULL) {
      char user[256], host[256], date[32];
      status = mb_user_host_date(verbose, user, host, date, error);
			fprintf(fp, "# File %s \n# Locked by user <%s> on cpu <%s> at <%s>\n", file, user, host, date);
			fprintf(fp, "Locking Program: %s\n", program_name);
			fprintf(fp, "Locking User: %s\n", user);
			fprintf(fp, "Locking CPU: %s\n", host);
			fprintf(fp, "Locking Time: %s\n", date);
			fprintf(fp, "Locking Purpose ID: %d\n", purpose);
			if (purpose == MBP_LOCK_NONE)
				fprintf(fp, "Locking Purpose Description: None (unknown)\n");
			else if (purpose == MBP_LOCK_PROCESS)
				fprintf(fp, "Locking Purpose Description: Process\n");
			else if (purpose == MBP_LOCK_EDITBATHY)
				fprintf(fp, "Locking Purpose Description: Edit Bathymetry\n");
			else if (purpose == MBP_LOCK_EDITNAV)
				fprintf(fp, "Locking Purpose Description: Edit Navigation\n");
			fclose(fp);
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
		else {
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
		}
	}
	else {
		*error = MB_ERROR_FILE_LOCKED;
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/

int mb_pr_lockinfo(int verbose, const char *file, bool *locked, int *purpose,
                   char *program, char *user, char *cpu, char *date,
                   int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       file:       %s\n", file);
	}

	/* initialize return parameters */
	*locked = false;
	*purpose = MBP_LOCK_NONE;
	program[0] = '\0';
	user[0] = '\0';
	cpu[0] = '\0';
	date[0] = '\0';

	int status = MB_SUCCESS;

	/* check if lock file exists */
	mb_path lockfile;
	sprintf(lockfile, "%s.lck", file);
	struct stat file_status;
	const int fstat = stat(lockfile, &file_status);
	if (fstat == 0) {
		/* read the lockfile */
		*locked = true;
		FILE *fp = fopen(lockfile, "r");
		if (fp != NULL) {
			mb_path line;
			while (fgets(line, MBP_FILENAMESIZE, fp) != NULL) {
				line[strlen(line) - 1] = '\0';
				if (strncmp(line, "Locking Program: ", 17) == 0) {
					strcpy(program, &(line[17]));
				}
				else if (strncmp(line, "Locking User: ", 14) == 0) {
					strcpy(user, &(line[14]));
				}
				else if (strncmp(line, "Locking CPU: ", 13) == 0) {
					strcpy(cpu, &(line[13]));
				}
				else if (strncmp(line, "Locking Time: ", 14) == 0) {
					strcpy(date, &(line[14]));
				}
				else if (strncmp(line, "Locking Purpose ID: ", 20) == 0) {
					sscanf(&(line[20]), "%d", purpose);
				}
			}
			fclose(fp);
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
		else {
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
		}
	}
	else {
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       locked:     %d\n", *locked);
		fprintf(stderr, "dbg2       purpose:    %d\n", *purpose);
		fprintf(stderr, "dbg2       program:    %s\n", program);
		fprintf(stderr, "dbg2       user:       %s\n", user);
		fprintf(stderr, "dbg2       cpu:        %s\n", cpu);
		fprintf(stderr, "dbg2       date:       %s\n", date);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_pr_unlockswathfile(int verbose, const char *file, int purpose, const char *program_name, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       file:       %s\n", file);
		fprintf(stderr, "dbg2       purpose:    %d\n", purpose);
		fprintf(stderr, "dbg2       program_name: %s\n", program_name);
	}

	int status = MB_SUCCESS;

	/* check if lock file exists */
	mb_path lockfile;
	sprintf(lockfile, "%s.lck", file);
	struct stat file_status;
	int fstat = stat(lockfile, &file_status);
	if (fstat == 0) {
		bool locked;
		int lock_purpose;
		mb_path lock_program;
		mb_path lock_user;
		mb_path lock_cpu;
		mb_path lock_date;
		/* get lock info */
		status = mb_pr_lockinfo(verbose, file, &locked, &lock_purpose, lock_program, lock_user, lock_cpu, lock_date, error);

    char user[256], host[256], date[32];
    status = mb_user_host_date(verbose, user, host, date, error);

		/* if locked and everything matches remove lock file */
		if (locked && strncmp(program_name, lock_program, MAX(strlen(program_name), strlen(lock_program))) == 0
		    && strncmp(user, lock_user, MAX(strlen(user), strlen(lock_user))) == 0
		    && purpose == lock_purpose) {
      if (remove(lockfile) == 0) {
			  status = MB_SUCCESS;
			  *error = MB_ERROR_NO_ERROR;
      } else {
			  status = MB_FAILURE;
			  *error = MB_ERROR_FILE_LOCKED;
      }
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_FILE_LOCKED;
		}
	}
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_FILE_NOT_FOUND;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/

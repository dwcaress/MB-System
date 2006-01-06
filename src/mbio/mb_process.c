/*--------------------------------------------------------------------
 *    The MB-system:	mb_process.c	9/11/00
 *    $Id: mb_process.c,v 5.34 2006-01-06 18:27:19 caress Exp $
 *
 *    Copyright (c) 2000, 2002, 2003, 2004 by
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
 * mb_process.c contains functions for reading and writing
 * mbprocess parameter files. The mb_process structure is defined
 * in mb_process.h. A description of mbprocess parameters and 
 * parameter file keywords is found in mb_process.h
 *
 * Author:	D. W. Caress
 * Date:	September 11, 2000
 * 
 * $Log: not supported by cvs2svn $
 * Revision 5.33  2005/11/05 00:48:04  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.32  2005/03/25 04:16:41  caress
 * Added sonar depth merging to mbprocess.
 *
 * Revision 5.31  2005/02/08 22:37:38  caress
 * Heading towards 5.0.6 release.
 *
 * Revision 5.30  2004/12/18 01:34:43  caress
 * Working towards release 5.0.6.
 *
 * Revision 5.29  2004/12/02 06:33:30  caress
 * Fixes while supporting Reson 7k data.
 *
 * Revision 5.28  2004/10/06 19:04:24  caress
 * Release 5.0.5 update.
 *
 * Revision 5.27  2004/09/16 01:11:48  caress
 * Fixed how esf file path is determined.
 *
 * Revision 5.26  2003/04/18 00:35:42  caress
 * Added capability to look for svp files with lookforfiles=2.
 *
 * Revision 5.25  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.24  2003/04/16 16:47:41  caress
 * Release 5.0.beta30
 *
 * Revision 5.23  2002/09/07 04:48:34  caress
 * Added slope mode option to mb_process.
 *
 * Revision 5.22  2002/07/25 19:09:04  caress
 * Release 5.0.beta21
 *
 * Revision 5.21  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.20  2002/05/29 23:36:53  caress
 * Release 5.0.beta18
 *
 * Revision 5.19  2002/05/02 03:55:34  caress
 * Release 5.0.beta17
 *
 * Revision 5.18  2002/04/06 02:43:39  caress
 * Release 5.0.beta16
 *
 * Revision 5.17  2001/12/20 21:03:18  caress
 * Release 5.0.beta11
 *
 * Revision 5.16  2001/12/18  04:27:45  caress
 * Release 5.0.beta11.
 *
 * Revision 5.15  2001/11/16  01:30:02  caress
 * Fixed handling of paths.
 *
 * Revision 5.14  2001/11/04  00:14:41  caress
 * Fixed handling of angle_mode
 *
 * Revision 5.13  2001/10/19 19:41:09  caress
 * Now uses relative paths.
 *
 * Revision 5.12  2001/10/19  00:54:37  caress
 * Now tries to use relative paths.
 *
 * Revision 5.11  2001/09/17  23:22:51  caress
 * Fixed metadata support.
 *
 * Revision 5.10  2001/08/10  22:41:19  dcaress
 * Release 5.0.beta07
 *
\ * Revision 5.9  2001-08-03 18:00:02-07  caress
 * Added cut by speed.
 *
 * Revision 5.8  2001/07/31  00:40:52  caress
 * Added data cutting capability.
 *
 * Revision 5.7  2001/07/27  19:07:16  caress
 * Added data cutting.
 *
 * Revision 5.6  2001/07/20 00:31:11  caress
 * Release 5.0.beta03
 *
 * Revision 5.5  2001/06/08  21:44:01  caress
 * Version 5.0.beta01
 *
 * Revision 5.4  2001/06/03  06:54:56  caress
 * Improved handling of lever calculation.
 *
 * Revision 5.3  2001/06/01  00:14:06  caress
 * Added support for metadata insertion.
 *
 * Revision 5.2  2001/03/22  20:45:56  caress
 * Trying to make 5.0.beta0...
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.2  2000/10/11  01:02:30  caress
 * Convert to ANSI C
 *
 * Revision 4.1  2000/10/03  21:46:59  caress
 * Snapshot for Dale.
 *
 * Revision 4.0  2000/09/30  06:28:42  caress
 * Snapshot for Dale.
 *
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

/* mbio include files */
#include "../../include/mb_io.h"
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"
#include "../../include/mb_format.h"
#include "../../include/mb_process.h"

static char rcs_id[]="$Id: mb_process.c,v 5.34 2006-01-06 18:27:19 caress Exp $";

/*--------------------------------------------------------------------*/
int mb_pr_readpar(int verbose, char *file, int lookforfiles, 
			struct mb_process_struct *process, 
			int *error)
{
	char	*function_name = "mb_pr_readpar";
	char	parfile[MBP_FILENAMESIZE];
	char	buffer[MBP_FILENAMESIZE], dummy[MBP_FILENAMESIZE], *result;
	char	*lastslash;
	FILE	*fp;
	struct	stat statbuf;
	int	status = MB_SUCCESS;
	int	len;
	int	explicit;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:      %d\n",verbose);
		fprintf(stderr,"dbg2       file:         %s\n",file);
		fprintf(stderr,"dbg2       lookforfiles: %d\n",lookforfiles);
		fprintf(stderr,"dbg2       process:      %d\n",process);
		}

	/* get expected process parameter file name */
	strcpy(parfile, file);
	strcat(parfile, ".par");
	
	/* initialize process parameter structure */

	/* general parameters */
	explicit = MB_NO;
	process->mbp_ifile_specified = MB_NO;
	process->mbp_ifile[0] = '\0';
	process->mbp_ofile_specified = MB_NO;
	process->mbp_ofile[0] = '\0';
	process->mbp_format_specified = MB_NO;
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
	
	/* adjusted navigation merging */
	process->mbp_navadj_mode = MBP_NAV_OFF;
	process->mbp_navadjfile[0] = '\0';
	process->mbp_navadj_algorithm = MBP_NAV_LINEAR;
	
	/* attitude merging */
	process->mbp_attitude_mode = 0;
	process->mbp_attitudefile[0] = '\0';
	process->mbp_attitude_format = 1;
	
	/* sonardepth merging */
	process->mbp_sonardepth_mode = 0;
	process->mbp_sonardepthfile[0] = '\0';
	process->mbp_sonardepth_format = 1;

	/* data cutting */
	process->mbp_cut_num = 0;
	for (i=0;i<MBP_CUT_NUM_MAX;i++)
		{
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
	process->mbp_corrected = MB_YES;
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
	process->mbp_ampcorr_symmetry = MBP_AMPCORR_SYMMETRIC,
	process->mbp_ampcorr_angle = 30.0;
	process->mbp_ampcorr_slope = MBP_AMPCORR_IGNORESLOPE;
	
	/* sidescan correction */
	process->mbp_sscorr_mode = MBP_SSCORR_OFF;
	process->mbp_sscorrfile[0] = '\0';
	process->mbp_sscorr_type = MBP_SSCORR_SUBTRACTION;
	process->mbp_sscorr_symmetry = MBP_SSCORR_SYMMETRIC,
	process->mbp_sscorr_angle = 30.0;
	process->mbp_sscorr_slope = MBP_SSCORR_IGNORESLOPE;
	
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
	process->mbp_kluge001 = MB_NO;
	process->mbp_kluge002 = MB_NO;
	process->mbp_kluge003 = MB_NO;
	process->mbp_kluge004 = MB_NO;
	process->mbp_kluge005 = MB_NO;

	/* open and read parameter file */
	if ((fp = fopen(parfile, "r")) != NULL) 
	    {
	    while ((result = fgets(buffer,MBP_FILENAMESIZE,fp)) == buffer)
		{
		if (buffer[0] != '#')
		    {
			if (strlen(buffer) > 0)
				{
				if (buffer[strlen(buffer)-1] == '\n')
					buffer[strlen(buffer)-1] = '\0';
				}

		    /* general parameters */
		    if (strncmp(buffer, "EXPLICIT", 8) == 0)
			{
			explicit = MB_YES;
			}
		    else if (strncmp(buffer, "INFILE", 6) == 0
			&& process->mbp_ifile_specified == MB_NO)
			{
			sscanf(buffer, "%s %s", dummy, process->mbp_ifile);
			process->mbp_ifile_specified = MB_YES;
			}
		    else if (strncmp(buffer, "OUTFILE", 7) == 0
			&& process->mbp_ofile_specified == MB_NO)
			{
			sscanf(buffer, "%s %s", dummy, process->mbp_ofile);
			process->mbp_ofile_specified = MB_YES;
			}
		    else if (strncmp(buffer, "FORMAT", 6) == 0
			&& process->mbp_format_specified == MB_NO)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_format);
			process->mbp_format_specified = MB_YES;
			}
			
		    /* navigation merging */
		    else if (strncmp(buffer, "NAVMODE", 7) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_nav_mode);
			}
		    else if (strncmp(buffer, "NAVFILE", 7) == 0)
			{
			sscanf(buffer, "%s %s", dummy, process->mbp_navfile);
			if (explicit == MB_NO)
			    {
			    process->mbp_nav_mode = MBP_NAV_ON;
			    process->mbp_nav_heading = MBP_NAV_ON;
			    process->mbp_nav_speed = MBP_NAV_ON;
			    process->mbp_nav_draft = MBP_NAV_ON;
			    process->mbp_nav_attitude = MBP_NAV_ON;
			    }
			}
		    else if (strncmp(buffer, "NAVFORMAT", 9) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_nav_format);
			}
		    else if (strncmp(buffer, "NAVHEADING", 10) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_nav_heading);
			}
		    else if (strncmp(buffer, "NAVSPEED", 8) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_nav_speed);
			}
		    else if (strncmp(buffer, "NAVDRAFT", 8) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_nav_draft);
			}
		    else if (strncmp(buffer, "NAVATTITUDE", 8) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_nav_attitude);
			}
		    else if (strncmp(buffer, "NAVINTERP", 9) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_nav_algorithm);
			}
		    else if (strncmp(buffer, "NAVSHIFT", 12) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_nav_shift);
			}
		    else if (strncmp(buffer, "NAVTIMESHIFT", 12) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_nav_timeshift);
			}
		    else if (strncmp(buffer, "NAVOFFSETX", 10) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_nav_offsetx);
			}
		    else if (strncmp(buffer, "NAVOFFSETY", 10) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_nav_offsety);
			}
		    else if (strncmp(buffer, "NAVOFFSETZ", 10) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_nav_offsetz);
			}

		    /* adjusted navigation merging */
		    else if (strncmp(buffer, "NAVADJMODE", 10) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_navadj_mode);
			}
		    else if (strncmp(buffer, "NAVADJFILE", 10) == 0)
			{
			sscanf(buffer, "%s %s", dummy, process->mbp_navadjfile);
			if (explicit == MB_NO)
			    {
			    process->mbp_navadj_mode = MBP_NAV_ON;
			    }
			}
		    else if (strncmp(buffer, "NAVADJINTERP", 12) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_navadj_algorithm);
			}
	
		    /* attitude merging */
		    else if (strncmp(buffer, "ATTITUDEMODE", 12) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_attitude_mode);
			}
		    else if (strncmp(buffer, "ATTITUDEFILE", 12) == 0)
			{
			sscanf(buffer, "%s %s", dummy, process->mbp_attitudefile);
			if (explicit == MB_NO)
			    {
			    process->mbp_attitude_mode = MBP_ATTITUDE_ON;
			    }
			}
		    else if (strncmp(buffer, "ATTITUDEFORMAT", 14) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_attitude_format);
			}
	
		    /* sonardepth merging */
		    else if (strncmp(buffer, "SONARDEPTHMODE", 12) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_sonardepth_mode);
			}
		    else if (strncmp(buffer, "SONARDEPTHFILE", 12) == 0)
			{
			sscanf(buffer, "%s %s", dummy, process->mbp_sonardepthfile);
			if (explicit == MB_NO)
			    {
			    process->mbp_sonardepth_mode = MBP_SONARDEPTH_ON;
			    }
			}
		    else if (strncmp(buffer, "SONARDEPTHFORMAT", 14) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_sonardepth_format);
			}

		    /* data cutting */
		    else if (strncmp(buffer, "DATACUTCLEAR", 12) == 0)
			{
			process->mbp_cut_num = 0;
			}
		    else if (strncmp(buffer, "DATACUT", 7) == 0)
			{
			if (process->mbp_cut_num < MBP_CUT_NUM_MAX)
				{
				sscanf(buffer, "%s %d %d %lf %lf", dummy, 
					&process->mbp_cut_kind[process->mbp_cut_num],
					&process->mbp_cut_mode[process->mbp_cut_num],
					&process->mbp_cut_min[process->mbp_cut_num],
					&process->mbp_cut_max[process->mbp_cut_num]);
				process->mbp_cut_num++;
				}
			}
		    else if (strncmp(buffer, "BATHCUTNUMBER", 13) == 0)
			{
			if (process->mbp_cut_num < MBP_CUT_NUM_MAX)
				{
				sscanf(buffer, "%s %lf %lf", dummy, 
					&process->mbp_cut_min[process->mbp_cut_num],
					&process->mbp_cut_max[process->mbp_cut_num]);
				process->mbp_cut_kind[process->mbp_cut_num] = MBP_CUT_DATA_BATH; 
				process->mbp_cut_mode[process->mbp_cut_num] = MBP_CUT_MODE_NUMBER; 
				process->mbp_cut_num++;
				}
			}
		    else if (strncmp(buffer, "BATHCUTDISTANCE", 15) == 0)
			{
			if (process->mbp_cut_num < MBP_CUT_NUM_MAX)
				{
				sscanf(buffer, "%s %lf %lf", dummy, 
					&process->mbp_cut_min[process->mbp_cut_num],
					&process->mbp_cut_max[process->mbp_cut_num]);
				process->mbp_cut_kind[process->mbp_cut_num] = MBP_CUT_DATA_BATH; 
				process->mbp_cut_mode[process->mbp_cut_num] = MBP_CUT_MODE_DISTANCE; 
				process->mbp_cut_num++;
				}
			}
		    else if (strncmp(buffer, "BATHCUTSPEED", 12) == 0)
			{
			if (process->mbp_cut_num < MBP_CUT_NUM_MAX)
				{
				sscanf(buffer, "%s %lf %lf", dummy, 
					&process->mbp_cut_min[process->mbp_cut_num],
					&process->mbp_cut_max[process->mbp_cut_num]);
				process->mbp_cut_kind[process->mbp_cut_num] = MBP_CUT_DATA_BATH; 
				process->mbp_cut_mode[process->mbp_cut_num] = MBP_CUT_MODE_SPEED; 
				process->mbp_cut_num++;
				}
			}
		    else if (strncmp(buffer, "AMPCUTNUMBER", 12) == 0)
			{
			if (process->mbp_cut_num < MBP_CUT_NUM_MAX)
				{
				sscanf(buffer, "%s %lf %lf", dummy, 
					&process->mbp_cut_min[process->mbp_cut_num],
					&process->mbp_cut_max[process->mbp_cut_num]);
				process->mbp_cut_kind[process->mbp_cut_num] = MBP_CUT_DATA_AMP; 
				process->mbp_cut_mode[process->mbp_cut_num] = MBP_CUT_MODE_NUMBER; 
				process->mbp_cut_num++;
				}
			}
		    else if (strncmp(buffer, "AMPCUTDISTANCE", 14) == 0)
			{
			if (process->mbp_cut_num < MBP_CUT_NUM_MAX)
				{
				sscanf(buffer, "%s %lf %lf", dummy, 
					&process->mbp_cut_min[process->mbp_cut_num],
					&process->mbp_cut_max[process->mbp_cut_num]);
				process->mbp_cut_kind[process->mbp_cut_num] = MBP_CUT_DATA_AMP; 
				process->mbp_cut_mode[process->mbp_cut_num] = MBP_CUT_MODE_DISTANCE; 
				process->mbp_cut_num++;
				}
			}
		    else if (strncmp(buffer, "AMPCUTSPEED", 11) == 0)
			{
			if (process->mbp_cut_num < MBP_CUT_NUM_MAX)
				{
				sscanf(buffer, "%s %lf %lf", dummy, 
					&process->mbp_cut_min[process->mbp_cut_num],
					&process->mbp_cut_max[process->mbp_cut_num]);
				process->mbp_cut_kind[process->mbp_cut_num] = MBP_CUT_DATA_AMP; 
				process->mbp_cut_mode[process->mbp_cut_num] = MBP_CUT_MODE_SPEED; 
				process->mbp_cut_num++;
				}
			}
		    else if (strncmp(buffer, "SSCUTNUMBER", 12) == 0)
			{
			if (process->mbp_cut_num < MBP_CUT_NUM_MAX)
				{
				sscanf(buffer, "%s %lf %lf", dummy, 
					&process->mbp_cut_min[process->mbp_cut_num],
					&process->mbp_cut_max[process->mbp_cut_num]);
				process->mbp_cut_kind[process->mbp_cut_num] = MBP_CUT_DATA_SS; 
				process->mbp_cut_mode[process->mbp_cut_num] = MBP_CUT_MODE_NUMBER; 
				process->mbp_cut_num++;
				}
			}
		    else if (strncmp(buffer, "SSCUTDISTANCE", 14) == 0)
			{
			if (process->mbp_cut_num < MBP_CUT_NUM_MAX)
				{
				sscanf(buffer, "%s %lf %lf", dummy, 
					&process->mbp_cut_min[process->mbp_cut_num],
					&process->mbp_cut_max[process->mbp_cut_num]);
				process->mbp_cut_kind[process->mbp_cut_num] = MBP_CUT_DATA_SS; 
				process->mbp_cut_mode[process->mbp_cut_num] = MBP_CUT_MODE_DISTANCE; 
				process->mbp_cut_num++;
				}
			}
		    else if (strncmp(buffer, "SSCUTSPEED", 10) == 0)
			{
			if (process->mbp_cut_num < MBP_CUT_NUM_MAX)
				{
				sscanf(buffer, "%s %lf %lf", dummy, 
					&process->mbp_cut_min[process->mbp_cut_num],
					&process->mbp_cut_max[process->mbp_cut_num]);
				process->mbp_cut_kind[process->mbp_cut_num] = MBP_CUT_DATA_SS; 
				process->mbp_cut_mode[process->mbp_cut_num] = MBP_CUT_MODE_SPEED; 
				process->mbp_cut_num++;
				}
			}
	
		    /* bathymetry editing */
		    else if (strncmp(buffer, "EDITSAVEMODE", 12) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_edit_mode);
			}
		    else if (strncmp(buffer, "EDITSAVEFILE", 12) == 0)
			{
			sscanf(buffer, "%s %s", dummy, process->mbp_editfile);
			if (explicit == MB_NO)
			    {
			    process->mbp_edit_mode = MBP_EDIT_ON;
			    }
			}
	
		    /* bathymetry recalculation */
		    else if (strncmp(buffer, "RAYTRACE", 8) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_svp_mode);
			}
		    else if (strncmp(buffer, "SVPMODE", 7) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_svp_mode);
			}
		    else if (strncmp(buffer, "SVPFILE", 7) == 0)
			{
			sscanf(buffer, "%s %s", dummy, process->mbp_svpfile);
			if (explicit == MB_NO)
			    {
			    process->mbp_svp_mode = MBP_SVP_ON;
			    }
			}
		    else if (strncmp(buffer, "SVP", 3) == 0)
			{
			sscanf(buffer, "%s %s", dummy, process->mbp_svpfile);
			if (explicit == MB_NO)
			    {
			    process->mbp_svp_mode = MBP_SVP_ON;
			    }
			}
		    else if (strncmp(buffer, "SSVMODE", 7) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_ssv_mode);
			}
		    else if (strncmp(buffer, "SSV", 3) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_ssv);
			}
		    else if (strncmp(buffer, "TTMODE", 6) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_tt_mode);
			}
		    else if (strncmp(buffer, "TTMULTIPLY", 10) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_tt_mult);
			}
		    else if (strncmp(buffer, "ANGLEMODE", 9) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_angle_mode);
			}
		    else if (strncmp(buffer, "CORRECTED", 9) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_corrected);
			}
		    else if (strncmp(buffer, "SOUNDSPEEDREF", 13) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_corrected);
			}
		    
		    /* static beam bathymetry correction */
		    else if (strncmp(buffer, "STATICMODE", 10) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_static_mode);
			}
		    else if (strncmp(buffer, "STATICFILE", 10) == 0)
			{
			sscanf(buffer, "%s %s", dummy, process->mbp_staticfile);
			if (explicit == MB_NO)
			    {
			    process->mbp_static_mode = MBP_SVP_ON;
			    }
			}
	
		    /* draft correction */
		    else if (strncmp(buffer, "DRAFTMODE", 9) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_draft_mode);
			}
		    else if (strncmp(buffer, "DRAFTOFFSET", 11) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_draft_offset);
			}
		    else if (strncmp(buffer, "DRAFTMULTIPLY", 13) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_draft_mult);
			}
		    else if (strncmp(buffer, "DRAFT", 5) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_draft);
			}
	
		    /* heave correction */
		    else if (strncmp(buffer, "HEAVEMODE", 9) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_heave_mode);
			}
		    else if (strncmp(buffer, "HEAVEOFFSET", 11) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_heave);
			}
		    else if (strncmp(buffer, "HEAVEMULTIPLY", 13) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_heave_mult);
			}
	
		    /* lever correction */
		    else if (strncmp(buffer, "LEVERMODE", 9) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_lever_mode);
			}
		    else if (strncmp(buffer, "VRUOFFSETX", 10) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_vru_offsetx);
			}
		    else if (strncmp(buffer, "VRUOFFSETY", 10) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_vru_offsety);
			}
		    else if (strncmp(buffer, "VRUOFFSETZ", 10) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_vru_offsetz);
			}
		    else if (strncmp(buffer, "SONAROFFSETX", 12) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_sonar_offsetx);
			}
		    else if (strncmp(buffer, "SONAROFFSETY", 12) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_sonar_offsety);
			}
		    else if (strncmp(buffer, "SONAROFFSETZ", 12) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_sonar_offsetz);
			}
	
		    /* roll correction */
		    else if (strncmp(buffer, "ROLLBIASMODE", 12) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_rollbias_mode);
			}
		    else if (strncmp(buffer, "ROLLBIASPORT", 12) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_rollbias_port);
			}
		    else if (strncmp(buffer, "ROLLBIASSTBD", 12) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_rollbias_stbd);
			}
		    else if (strncmp(buffer, "ROLLBIAS", 8) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_rollbias);
			}
	
		    /* pitch correction */
		    else if (strncmp(buffer, "PITCHBIASMODE", 13) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_pitchbias_mode);
			}
		    else if (strncmp(buffer, "PITCHBIAS", 9) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_pitchbias);
			}
	
		    /* heading correction */
		    else if (strncmp(buffer, "HEADINGMODE", 11) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_heading_mode);
			}
		    else if (strncmp(buffer, "HEADINGOFFSET", 13) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_headingbias);
			}
	
		    /* tide correction */
		    else if (strncmp(buffer, "TIDEMODE", 8) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_tide_mode);
			}
		    else if (strncmp(buffer, "TIDEFILE", 8) == 0)
			{
			sscanf(buffer, "%s %s", dummy, process->mbp_tidefile);
			if (explicit == MB_NO)
			    {
			    process->mbp_tide_mode = MBP_TIDE_ON;
			    }
			}
		    else if (strncmp(buffer, "TIDEFORMAT", 10) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_tide_format);
			}
	
		    /* amplitude correction */
		    else if (strncmp(buffer, "AMPCORRMODE", 11) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_ampcorr_mode);
			}
		    else if (strncmp(buffer, "AMPCORRFILE", 11) == 0)
			{
			sscanf(buffer, "%s %s", dummy, process->mbp_ampcorrfile);
			if (explicit == MB_NO)
			    {
			    process->mbp_ampcorr_mode = MBP_AMPCORR_ON;
			    }
			}
		    else if (strncmp(buffer, "AMPCORRTYPE", 11) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_ampcorr_type);
			}
		    else if (strncmp(buffer, "AMPCORRSYMMETRY", 15) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_ampcorr_symmetry);
			}
		    else if (strncmp(buffer, "AMPCORRANGLE", 12) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_ampcorr_angle);
			}
		    else if (strncmp(buffer, "AMPCORRSLOPE", 12) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_ampcorr_slope);
			}
	
		    /* sidescan correction */
		    else if (strncmp(buffer, "SSCORRMODE", 10) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_sscorr_mode);
			}
		    else if (strncmp(buffer, "SSCORRFILE", 10) == 0)
			{
			sscanf(buffer, "%s %s", dummy, process->mbp_sscorrfile);
			if (explicit == MB_NO)
			    {
			    process->mbp_sscorr_mode = MBP_SSCORR_ON;
			    }
			}
		    else if (strncmp(buffer, "SSCORRTYPE", 10) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_sscorr_type);
			}
		    else if (strncmp(buffer, "SSCORRSYMMETRY", 14) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_sscorr_symmetry);
			}
		    else if (strncmp(buffer, "SSCORRANGLE", 11) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_sscorr_angle);
			}
		    else if (strncmp(buffer, "SSCORRSLOPE", 11) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_sscorr_slope);
			}
	
		    /* sidescan recalculation */
		    else if (strncmp(buffer, "SSRECALCMODE", 12) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_ssrecalc_mode);
			}
		    else if (strncmp(buffer, "SSPIXELSIZE", 11) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_ssrecalc_pixelsize);
			}
		    else if (strncmp(buffer, "SSSWATHWIDTH", 11) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_ssrecalc_swathwidth);
			}
		    else if (strncmp(buffer, "SSINTERPOLATE", 11) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_ssrecalc_interpolate);
			}
	
		    /* metadata strings */
		    else if (strncmp(buffer, "METAVESSEL", 10) == 0)
			{
			strncpy(process->mbp_meta_vessel, &buffer[11], MBP_FILENAMESIZE);
			}
		    else if (strncmp(buffer, "METAINSTITUTION", 15) == 0)
			{
			strncpy(process->mbp_meta_institution, &buffer[16], MBP_FILENAMESIZE);
			}
		    else if (strncmp(buffer, "METAPLATFORM", 12) == 0)
			{
			strncpy(process->mbp_meta_platform, &buffer[13], MBP_FILENAMESIZE);
			}
		    else if (strncmp(buffer, "METASONARVERSION", 16) == 0)
			{
			strncpy(process->mbp_meta_sonarversion, &buffer[17], MBP_FILENAMESIZE);
			}
		    else if (strncmp(buffer, "METASONAR", 9) == 0)
			{
			strncpy(process->mbp_meta_sonar, &buffer[10], MBP_FILENAMESIZE);
			}
		    else if (strncmp(buffer, "METACRUISEID", 12) == 0)
			{
			strncpy(process->mbp_meta_cruiseid, &buffer[13], MBP_FILENAMESIZE);
			}
		    else if (strncmp(buffer, "METACRUISENAME", 14) == 0)
			{
			strncpy(process->mbp_meta_cruisename, &buffer[15], MBP_FILENAMESIZE);
			}
		    else if (strncmp(buffer, "METAPIINSTITUTION", 17) == 0)
			{
			strncpy(process->mbp_meta_piinstitution, &buffer[18], MBP_FILENAMESIZE);
			}
		    else if (strncmp(buffer, "METACLIENT", 10) == 0)
			{
			strncpy(process->mbp_meta_client, &buffer[11], MBP_FILENAMESIZE);
			}
		    else if (strncmp(buffer, "METASVCORRECTED", 15) == 0)
			{
			sscanf(buffer, "METASVCORRECTED %d", &process->mbp_meta_svcorrected);
			}
		    else if (strncmp(buffer, "METATIDECORRECTED", 17) == 0)
			{
			sscanf(buffer, "METATIDECORRECTED %d", &process->mbp_meta_tidecorrected);
			}
		    else if (strncmp(buffer, "METABATHEDITMANUAL", 18) == 0)
			{
			sscanf(buffer, "METABATHEDITMANUAL %d", &process->mbp_meta_batheditmanual);
			}
		    else if (strncmp(buffer, "METABATHEDITAUTO", 16) == 0)
			{
			sscanf(buffer, "METABATHEDITAUTO %d", &process->mbp_meta_batheditauto);
			}
		    else if (strncmp(buffer, "METAROLLBIAS", 12) == 0)
			{
			sscanf(buffer, "METAROLLBIAS %lf", &process->mbp_meta_rollbias);
			}
		    else if (strncmp(buffer, "METAPITCHBIAS", 13) == 0)
			{
			sscanf(buffer, "METAPITCHBIAS %lf", &process->mbp_meta_pitchbias);
			}
		    else if (strncmp(buffer, "METAPI", 6) == 0)
			{
			strncpy(process->mbp_meta_pi, &buffer[7], MBP_FILENAMESIZE);
			}
		    else if (strncmp(buffer, "METAHEADINGBIAS", 15) == 0)
			{
			sscanf(buffer, "METAHEADINGBIAS %lf", &process->mbp_meta_headingbias);
			}
		    else if (strncmp(buffer, "METADRAFT", 9) == 0)
			{
			sscanf(buffer, "METADRAFT %lf", &process->mbp_meta_draft);
			}
	
		    /* processing kluges */
		    else if (strncmp(buffer, "KLUGE001", 8) == 0)
			{
			process->mbp_kluge001 = MB_YES;
			}
		    else if (strncmp(buffer, "KLUGE002", 8) == 0)
			{
			process->mbp_kluge002 = MB_YES;
			}
		    else if (strncmp(buffer, "KLUGE003", 8) == 0)
			{
			process->mbp_kluge003 = MB_YES;
			}
		    else if (strncmp(buffer, "KLUGE004", 8) == 0)
			{
			process->mbp_kluge004 = MB_YES;
			}
		    else if (strncmp(buffer, "KLUGE005", 8) == 0)
			{
			process->mbp_kluge005 = MB_YES;
			}			
		    }
		}
		
	    /* close file */
	    fclose(fp);
	    
	    }
	    
	/* Now make input file global if local */
	process->mbp_ifile_specified = MB_YES;
	if (file[0] != '/')
	    {
	    getcwd(process->mbp_ifile, MB_PATH_MAXLINE);
	    strcat(process->mbp_ifile, "/");
	    strcat(process->mbp_ifile, file);
	    }
	else
	    strcpy(process->mbp_ifile, file);
	mb_get_shortest_path(verbose, process->mbp_ifile, error);
	    
	/* figure out data format or output filename if required */
	if (process->mbp_format_specified == MB_NO
	    || process->mbp_ofile_specified == MB_NO)
	    {
	    mb_pr_default_output(verbose, process, error);
	    }
	    
	/* Make output file global if local */
	if (process->mbp_ofile[0] != '/')
	    {
	    lastslash = strrchr(process->mbp_ifile, '/');
	    if (lastslash != NULL)
		{
		strcpy(dummy, process->mbp_ofile);
		strcpy(process->mbp_ofile, process->mbp_ifile);
		process->mbp_ofile[strlen(process->mbp_ifile) - strlen(lastslash)] = '\0';
		strcat(process->mbp_ofile, "/");
		strcat(process->mbp_ofile, dummy);
		}
	    }
	    
	/* look for nav and other bath edit files if not specified */
	if (lookforfiles == 1 || lookforfiles == 2)
	    {
	    /* look for navadj file */
	    if (process->mbp_navadj_mode == MBP_NAV_OFF)
		{
		for (i = 9; i >= 0 && process->mbp_navadj_mode == MBP_NAV_OFF; i--)
		    {
		    sprintf(process->mbp_navadjfile, "%s.na%d", process->mbp_ifile, i);
		    if (stat(process->mbp_navadjfile, &statbuf) == 0)
			    {
			    process->mbp_navadj_mode = MBP_NAV_ON;
			    }
		    }
		if (process->mbp_navadj_mode == MBP_NAV_OFF)
		    {
		    process->mbp_navadjfile[0] = '\0';
		    }
		}

	    /* look for nav file */
	    if (process->mbp_nav_mode == MBP_NAV_OFF)
 		{
		strcpy(process->mbp_navfile, process->mbp_ifile);
		strcat(process->mbp_navfile, ".nve");
		if (stat(process->mbp_navfile, &statbuf) == 0)
		    {
		    process->mbp_nav_mode = MBP_NAV_ON;
		    process->mbp_nav_format = 9;
		    }
		else
		    {
		    process->mbp_navfile[0] = '\0';
		    }
		}

	    /* look for edit file */
	    if (process->mbp_edit_mode == MBP_EDIT_OFF)
 		{
		strcpy(process->mbp_editfile, process->mbp_ifile);
		strcat(process->mbp_editfile, ".esf");
		if (stat(process->mbp_editfile, &statbuf) == 0)
		    {
		    process->mbp_edit_mode = MBP_EDIT_ON;
		    }
		else
		    {
		    strcpy(process->mbp_editfile, process->mbp_ifile);
		    strcat(process->mbp_editfile, ".mbesf");
		    if (stat(process->mbp_editfile, &statbuf) == 0)
			{
			process->mbp_edit_mode = MBP_EDIT_ON;
			}
		    else
			{
			process->mbp_editfile[0] = '\0';
			}
		    }
		}
	     }
	    
	/* look for svp files if not specified */
	if (lookforfiles == 2)
	    {
	    /* look for svp file */
	    if (process->mbp_svp_mode == MBP_SVP_OFF)
 		{
		strcpy(process->mbp_svpfile, process->mbp_ifile);
		strcat(process->mbp_svpfile, ".svp");
		if (stat(process->mbp_svpfile, &statbuf) == 0)
		    {
		    process->mbp_svp_mode = MBP_SVP_ON;
		    }
		else
		    {
		    strcpy(process->mbp_svpfile, process->mbp_ifile);
		    strcat(process->mbp_svpfile, "_001.svp");
		    if (stat(process->mbp_svpfile, &statbuf) == 0)
			{
			process->mbp_svp_mode = MBP_SVP_ON;
			}
		    else
			{
			process->mbp_svpfile[0] = '\0';
			}
		    }
		}
	    }
	    
	/* reset all output files to local path if possible */
	if (lookforfiles > 2)
	    {
	    /* reset output file */
	    process->mbp_ofile_specified = MB_NO;
	    mb_pr_default_output(verbose, process, error);

	    /* reset navadj file */
	    if ((lastslash = strrchr(process->mbp_navadjfile, '/')) != NULL
		&& strlen(lastslash) > 1)
		{
		strcpy(dummy, &(lastslash[1]));
		strcpy(process->mbp_navadjfile, dummy);
		}

	    /* reset nav file */
	    if ((lastslash = strrchr(process->mbp_navfile, '/')) != NULL
		&& strlen(lastslash) > 1)
		{
		strcpy(dummy, &(lastslash[1]));
		strcpy(process->mbp_navfile, dummy);
		}

	    /* reset edit file */
	    if ((lastslash = strrchr(process->mbp_editfile, '/')) != NULL
		&& strlen(lastslash) > 1)
		{
		strcpy(dummy, &(lastslash[1]));
		strcpy(process->mbp_editfile, dummy);
		}

	    /* reset static file */
	    if ((lastslash = strrchr(process->mbp_staticfile, '/')) != NULL
		&& strlen(lastslash) > 1)
		{
		strcpy(dummy, &(lastslash[1]));
		strcpy(process->mbp_staticfile, dummy);
		}

	    /* reset attitude file */
	    if ((lastslash = strrchr(process->mbp_attitudefile, '/')) != NULL
		&& strlen(lastslash) > 1)
		{
		strcpy(dummy, &(lastslash[1]));
		strcpy(process->mbp_attitudefile, dummy);
		}

	    /* reset sonardepth file */
	    if ((lastslash = strrchr(process->mbp_sonardepthfile, '/')) != NULL
		&& strlen(lastslash) > 1)
		{
		strcpy(dummy, &(lastslash[1]));
		strcpy(process->mbp_sonardepthfile, dummy);
		}

	    /* reset tide file */
	    if ((lastslash = strrchr(process->mbp_tidefile, '/')) != NULL
		&& strlen(lastslash) > 1)
		{
		strcpy(dummy, &(lastslash[1]));
		strcpy(process->mbp_tidefile, dummy);
		}

	    /* reset ampcorr file */
	    if ((lastslash = strrchr(process->mbp_ampcorrfile, '/')) != NULL
		&& strlen(lastslash) > 1)
		{
		strcpy(dummy, &(lastslash[1]));
		strcpy(process->mbp_ampcorrfile, dummy);
		}

	    /* reset sscorr file */
	    if ((lastslash = strrchr(process->mbp_sscorrfile, '/')) != NULL
		&& strlen(lastslash) > 1)
		{
		strcpy(dummy, &(lastslash[1]));
		strcpy(process->mbp_sscorrfile, dummy);
		}
	    }
	    
	/* Now make filenames global if local */
	lastslash = strrchr(process->mbp_ifile, '/');
	len = lastslash - process->mbp_ifile + 1;

	/* reset navadj file */
	if (len > 1
	    && strlen(process->mbp_navadjfile) > 1
	    && process->mbp_navadjfile[0] != '/')
	    {
	    strcpy(dummy, process->mbp_navadjfile);
	    strncpy(process->mbp_navadjfile, process->mbp_ifile, len);
	    process->mbp_navadjfile[len] = '\0';
	    strcat(process->mbp_navadjfile, dummy);
	    }

	/* reset nav file */
	if (len > 1
	    && strlen(process->mbp_navfile) > 1
	    && process->mbp_navfile[0] != '/')
	    {
	    strcpy(dummy, process->mbp_navfile);
	    strncpy(process->mbp_navfile, process->mbp_ifile, len);
	    process->mbp_navfile[len] = '\0';
	    strcat(process->mbp_navfile, dummy);
	    }

	/* reset attitude file */
	if (len > 1
	    && strlen(process->mbp_attitudefile) > 1
	    && process->mbp_attitudefile[0] != '/')
	    {
	    strcpy(dummy, process->mbp_attitudefile);
	    strncpy(process->mbp_attitudefile, process->mbp_ifile, len);
	    process->mbp_attitudefile[len] = '\0';
	    strcat(process->mbp_attitudefile, dummy);
	    }

	/* reset sonardepth file */
	if (len > 1
	    && strlen(process->mbp_sonardepthfile) > 1
	    && process->mbp_sonardepthfile[0] != '/')
	    {
	    strcpy(dummy, process->mbp_sonardepthfile);
	    strncpy(process->mbp_sonardepthfile, process->mbp_ifile, len);
	    process->mbp_sonardepthfile[len] = '\0';
	    strcat(process->mbp_sonardepthfile, dummy);
	    }

	/* reset svp file */
	if (len > 1
	    && strlen(process->mbp_svpfile) > 1
	    && process->mbp_svpfile[0] != '/')
	    {
	    strcpy(dummy, process->mbp_svpfile);
	    strncpy(process->mbp_svpfile, process->mbp_ifile, len);
	    process->mbp_svpfile[len] = '\0';
	    strcat(process->mbp_svpfile, dummy);
	    }

	/* reset edit file */
	if (len > 1
	    && strlen(process->mbp_editfile) > 1
	    && process->mbp_editfile[0] != '/')
	    {
	    strcpy(dummy, process->mbp_editfile);
	    strncpy(process->mbp_editfile, process->mbp_ifile, len);
	    process->mbp_editfile[len] = '\0';
	    strcat(process->mbp_editfile, dummy);
	    }

	/* reset static file */
	if (len > 1
	    && strlen(process->mbp_staticfile) > 1
	    && process->mbp_staticfile[0] != '/')
	    {
	    strcpy(dummy, process->mbp_staticfile);
	    strncpy(process->mbp_staticfile, process->mbp_ifile, len);
	    process->mbp_staticfile[len] = '\0';
	    strcat(process->mbp_staticfile, dummy);
	    }

	/* reset tide file */
	if (len > 1
	    && strlen(process->mbp_tidefile) > 1
	    && process->mbp_tidefile[0] != '/')
	    {
	    strcpy(dummy, process->mbp_tidefile);
	    strncpy(process->mbp_tidefile, process->mbp_ifile, len);
	    process->mbp_tidefile[len] = '\0';
	    strcat(process->mbp_tidefile, dummy);
	    }

	/* reset amplitude correction file */
	if (len > 1
	    && strlen(process->mbp_ampcorrfile) > 1
	    && process->mbp_ampcorrfile[0] != '/')
	    {
	    strcpy(dummy, process->mbp_ampcorrfile);
	    strncpy(process->mbp_ampcorrfile, process->mbp_ifile, len);
	    process->mbp_ampcorrfile[len] = '\0';
	    strcat(process->mbp_ampcorrfile, dummy);
	    }

	/* reset sidescan correction file */
	if (len > 1
	    && strlen(process->mbp_sscorrfile) > 1
	    && process->mbp_sscorrfile[0] != '/')
	    {
	    strcpy(dummy, process->mbp_sscorrfile);
	    strncpy(process->mbp_sscorrfile, process->mbp_ifile, len);
	    process->mbp_sscorrfile[len] = '\0';
	    strcat(process->mbp_sscorrfile, dummy);
	    }
	    
	/* make sure all global paths are as short as possible */
	mb_get_shortest_path(verbose, process->mbp_navadjfile, error);
	mb_get_shortest_path(verbose, process->mbp_navfile, error);
	mb_get_shortest_path(verbose, process->mbp_attitudefile, error);
	mb_get_shortest_path(verbose, process->mbp_sonardepthfile, error);
	mb_get_shortest_path(verbose, process->mbp_svpfile, error);
	mb_get_shortest_path(verbose, process->mbp_editfile, error);
	mb_get_shortest_path(verbose, process->mbp_staticfile, error);
	mb_get_shortest_path(verbose, process->mbp_tidefile, error);
	mb_get_shortest_path(verbose, process->mbp_ampcorrfile, error);
	mb_get_shortest_path(verbose, process->mbp_sscorrfile, error);
	    
	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, process, error);
	    
	/* check for error */
	if (process->mbp_ifile_specified == MB_NO
	    || process->mbp_ofile_specified == MB_NO
	    || process->mbp_format_specified == MB_NO)
	    {
	    status = MB_FAILURE;
	    *error = MB_ERROR_OPEN_FAIL;
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_ifile_specified:    %d\n",process->mbp_ifile_specified);
		fprintf(stderr,"dbg2       mbp_ifile:              %s\n",process->mbp_ifile);
		fprintf(stderr,"dbg2       mbp_ofile_specified:    %d\n",process->mbp_ofile_specified);
		fprintf(stderr,"dbg2       mbp_ofile:              %s\n",process->mbp_ofile);
		fprintf(stderr,"dbg2       mbp_format_specified:   %d\n",process->mbp_format_specified);
		fprintf(stderr,"dbg2       mbp_format:             %d\n",process->mbp_format);
		fprintf(stderr,"dbg2       mbp_nav_mode:           %d\n",process->mbp_nav_mode);
		fprintf(stderr,"dbg2       mbp_navfile:            %s\n",process->mbp_navfile);
		fprintf(stderr,"dbg2       mbp_nav_format:         %d\n",process->mbp_nav_format);
		fprintf(stderr,"dbg2       mbp_nav_heading:        %d\n",process->mbp_nav_heading);
		fprintf(stderr,"dbg2       mbp_nav_speed:          %d\n",process->mbp_nav_speed);
		fprintf(stderr,"dbg2       mbp_nav_draft:          %d\n",process->mbp_nav_draft);
		fprintf(stderr,"dbg2       mbp_nav_attitude:       %d\n",process->mbp_nav_attitude);
		fprintf(stderr,"dbg2       mbp_nav_algorithm:      %d\n",process->mbp_nav_algorithm);
		fprintf(stderr,"dbg2       mbp_nav_timeshift:      %f\n",process->mbp_nav_timeshift);
		fprintf(stderr,"dbg2       mbp_nav_shift:          %d\n",process->mbp_nav_shift);
		fprintf(stderr,"dbg2       mbp_nav_offsetx:        %f\n",process->mbp_nav_offsetx);
		fprintf(stderr,"dbg2       mbp_nav_offsety:        %f\n",process->mbp_nav_offsety);
		fprintf(stderr,"dbg2       mbp_nav_offsetz:        %f\n",process->mbp_nav_offsetz);
		fprintf(stderr,"dbg2       mbp_navadj_mode:        %d\n",process->mbp_navadj_mode);
		fprintf(stderr,"dbg2       mbp_navadjfile:         %s\n",process->mbp_navadjfile);
		fprintf(stderr,"dbg2       mbp_navadj_algorithm:   %d\n",process->mbp_navadj_algorithm);
		fprintf(stderr,"dbg2       mbp_attitude_mode:      %d\n",process->mbp_attitude_mode);
		fprintf(stderr,"dbg2       mbp_attitudefile:       %s\n",process->mbp_attitudefile);
		fprintf(stderr,"dbg2       mbp_attitude_format:    %d\n",process->mbp_attitude_format);
		fprintf(stderr,"dbg2       mbp_sonardepth_mode:    %d\n",process->mbp_sonardepth_mode);
		fprintf(stderr,"dbg2       mbp_sonardepthfile:     %s\n",process->mbp_sonardepthfile);
		fprintf(stderr,"dbg2       mbp_sonardepth_format:  %d\n",process->mbp_sonardepth_format);
		fprintf(stderr,"dbg2       mbp_cut_num:            %d\n",process->mbp_cut_num);
		for (i=0;i<process->mbp_cut_num;i++)
			{
			fprintf(stderr,"dbg2           cut %d:\n",i);
			fprintf(stderr,"dbg2           mbp_cut_kind[%d]:     %d\n", i, process->mbp_cut_kind[i]);
			fprintf(stderr,"dbg2           mbp_cut_mode[%d]:     %d\n", i, process->mbp_cut_mode[i]);
			fprintf(stderr,"dbg2           mbp_cut_min[%d]:      %f\n", i, process->mbp_cut_min[i]);
			fprintf(stderr,"dbg2           mbp_cut_max[%d]:      %f\n", i, process->mbp_cut_max[i]);
			}
		fprintf(stderr,"dbg2       mbp_bathrecalc_mode:    %d\n",process->mbp_bathrecalc_mode);
		fprintf(stderr,"dbg2       mbp_rollbias_mode:      %d\n",process->mbp_rollbias_mode);
		fprintf(stderr,"dbg2       mbp_rollbias:           %f\n",process->mbp_rollbias);
		fprintf(stderr,"dbg2       mbp_rollbias_port:      %f\n",process->mbp_rollbias_port);
		fprintf(stderr,"dbg2       mbp_rollbias_stbd:      %f\n",process->mbp_rollbias_stbd);
		fprintf(stderr,"dbg2       mbp_pitchbias_mode:     %d\n",process->mbp_pitchbias_mode);
		fprintf(stderr,"dbg2       mbp_pitchbias:          %f\n",process->mbp_pitchbias);
		fprintf(stderr,"dbg2       mbp_draft_mode:         %d\n",process->mbp_draft_mode);
		fprintf(stderr,"dbg2       mbp_draft:              %f\n",process->mbp_draft);
		fprintf(stderr,"dbg2       mbp_draft_offset:       %f\n",process->mbp_draft_offset);
		fprintf(stderr,"dbg2       mbp_draft_mult:         %f\n",process->mbp_draft_mult);
		fprintf(stderr,"dbg2       mbp_heave_mode:         %d\n",process->mbp_heave_mode);
		fprintf(stderr,"dbg2       mbp_heave:              %f\n",process->mbp_heave);
		fprintf(stderr,"dbg2       mbp_heave_mult:         %f\n",process->mbp_heave_mult);
		fprintf(stderr,"dbg2       mbp_lever_mode:         %d\n",process->mbp_heave_mode);
		fprintf(stderr,"dbg2       mbp_vru_offsetx:        %f\n",process->mbp_vru_offsetx);
		fprintf(stderr,"dbg2       mbp_vru_offsety:        %f\n",process->mbp_vru_offsety);
		fprintf(stderr,"dbg2       mbp_vru_offsetz:        %f\n",process->mbp_vru_offsetz);
		fprintf(stderr,"dbg2       mbp_sonar_offsetx:      %f\n",process->mbp_sonar_offsetx);
		fprintf(stderr,"dbg2       mbp_sonar_offsety:      %f\n",process->mbp_sonar_offsety);
		fprintf(stderr,"dbg2       mbp_sonar_offsetz:      %f\n",process->mbp_sonar_offsetz);
		fprintf(stderr,"dbg2       mbp_ssv_mode:           %d\n",process->mbp_ssv_mode);
		fprintf(stderr,"dbg2       mbp_ssv:                %f\n",process->mbp_ssv);
		fprintf(stderr,"dbg2       mbp_svp_mode:           %d\n",process->mbp_svp_mode);
		fprintf(stderr,"dbg2       mbp_svpfile:            %s\n",process->mbp_svpfile);
		fprintf(stderr,"dbg2       mbp_corrected:          %d\n",process->mbp_corrected);
		fprintf(stderr,"dbg2       mbp_tt_mode:            %d\n",process->mbp_tt_mode);
		fprintf(stderr,"dbg2       mbp_tt_mult:            %f\n",process->mbp_tt_mult);
		fprintf(stderr,"dbg2       mbp_angle_mode:         %d\n",process->mbp_angle_mode);
		fprintf(stderr,"dbg2       mbp_static_mode:        %d\n",process->mbp_static_mode);
		fprintf(stderr,"dbg2       mbp_staticfile:         %s\n",process->mbp_staticfile);
		fprintf(stderr,"dbg2       mbp_heading_mode:       %d\n",process->mbp_heading_mode);
		fprintf(stderr,"dbg2       mbp_headingbias:        %f\n",process->mbp_headingbias);
		fprintf(stderr,"dbg2       mbp_edit_mode:          %d\n",process->mbp_edit_mode);
		fprintf(stderr,"dbg2       mbp_editfile:           %s\n",process->mbp_editfile);
		fprintf(stderr,"dbg2       mbp_tide_mode:          %d\n",process->mbp_tide_mode);
		fprintf(stderr,"dbg2       mbp_tidefile:           %s\n",process->mbp_tidefile);
		fprintf(stderr,"dbg2       mbp_tide_format:        %d\n",process->mbp_tide_format);
		fprintf(stderr,"dbg2       mbp_ampcorr_mode:       %d\n",process->mbp_ampcorr_mode);
		fprintf(stderr,"dbg2       mbp_ampcorrfile:        %s\n",process->mbp_ampcorrfile);
		fprintf(stderr,"dbg2       mbp_ampcorr_type:       %d\n",process->mbp_ampcorr_type);
		fprintf(stderr,"dbg2       mbp_ampcorr_symmetry:   %d\n",process->mbp_ampcorr_symmetry);
		fprintf(stderr,"dbg2       mbp_ampcorr_angle:      %f\n",process->mbp_ampcorr_angle);
		fprintf(stderr,"dbg2       mbp_ampcorr_slope:      %d\n",process->mbp_ampcorr_slope);
		fprintf(stderr,"dbg2       mbp_sscorr_mode:        %d\n",process->mbp_sscorr_mode);
		fprintf(stderr,"dbg2       mbp_sscorrfile:         %s\n",process->mbp_sscorrfile);
		fprintf(stderr,"dbg2       mbp_sscorr_type:        %d\n",process->mbp_sscorr_type);
		fprintf(stderr,"dbg2       mbp_sscorr_symmetry:    %d\n",process->mbp_sscorr_symmetry);
		fprintf(stderr,"dbg2       mbp_sscorr_angle:       %f\n",process->mbp_sscorr_angle);
		fprintf(stderr,"dbg2       mbp_sscorr_slope:       %d\n",process->mbp_sscorr_slope);
		fprintf(stderr,"dbg2       mbp_ssrecalc_mode:      %d\n",process->mbp_ssrecalc_mode);
		fprintf(stderr,"dbg2       mbp_ssrecalc_pixelsize: %f\n",process->mbp_ssrecalc_pixelsize);
		fprintf(stderr,"dbg2       mbp_ssrecalc_swathwidth:%f\n",process->mbp_ssrecalc_swathwidth);
		fprintf(stderr,"dbg2       mbp_ssrecalc_interp    :%d\n",process->mbp_ssrecalc_interpolate);
		fprintf(stderr,"dbg2       mbp_meta_vessel        :%s\n",process->mbp_meta_vessel);
		fprintf(stderr,"dbg2       mbp_meta_institution   :%s\n",process->mbp_meta_institution);
		fprintf(stderr,"dbg2       mbp_meta_platform      :%s\n",process->mbp_meta_platform);
		fprintf(stderr,"dbg2       mbp_meta_sonar         :%s\n",process->mbp_meta_sonar);
		fprintf(stderr,"dbg2       mbp_meta_sonarversion  :%s\n",process->mbp_meta_sonarversion);
		fprintf(stderr,"dbg2       mbp_meta_cruiseid      :%s\n",process->mbp_meta_cruiseid);
		fprintf(stderr,"dbg2       mbp_meta_cruisename    :%s\n",process->mbp_meta_cruisename);
		fprintf(stderr,"dbg2       mbp_meta_pi            :%s\n",process->mbp_meta_pi);
		fprintf(stderr,"dbg2       mbp_meta_piinstitution :%s\n",process->mbp_meta_piinstitution);
		fprintf(stderr,"dbg2       mbp_meta_client        :%s\n",process->mbp_meta_client);
		fprintf(stderr,"dbg2       mbp_meta_svcorrected   :%d\n",process->mbp_meta_svcorrected);
		fprintf(stderr,"dbg2       mbp_meta_tidecorrected :%d\n",process->mbp_meta_tidecorrected);
		fprintf(stderr,"dbg2       mbp_meta_batheditmanual:%d\n",process->mbp_meta_batheditmanual);
		fprintf(stderr,"dbg2       mbp_meta_batheditauto:  %d\n",process->mbp_meta_batheditauto);
		fprintf(stderr,"dbg2       mbp_meta_rollbias:      %f\n",process->mbp_meta_rollbias);
		fprintf(stderr,"dbg2       mbp_meta_pitchbias:     %f\n",process->mbp_meta_pitchbias);
		fprintf(stderr,"dbg2       mbp_meta_headingbias:   %f\n",process->mbp_meta_headingbias);
		fprintf(stderr,"dbg2       mbp_meta_draft:         %f\n",process->mbp_meta_draft);
		fprintf(stderr,"dbg2       mbp_kluge001:           %d\n",process->mbp_kluge001);
		fprintf(stderr,"dbg2       mbp_kluge002:           %d\n",process->mbp_kluge002);
		fprintf(stderr,"dbg2       mbp_kluge003:           %d\n",process->mbp_kluge003);
		fprintf(stderr,"dbg2       mbp_kluge004:           %d\n",process->mbp_kluge004);
		fprintf(stderr,"dbg2       mbp_kluge005:           %d\n",process->mbp_kluge005);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_writepar(int verbose, char *file, 
			struct mb_process_struct *process, 
			int *error)
{
	char	*function_name = "mb_pr_writepar";
	char	parfile[MBP_FILENAMESIZE];
	char	pwd[MBP_FILENAMESIZE];
	char	relative_path[MBP_FILENAMESIZE];
	char	*lastslash;
	FILE	*fp;
	int	status = MB_SUCCESS;
	time_t	right_now;
	char	date[25], user[MBP_FILENAMESIZE], *user_ptr, host[MBP_FILENAMESIZE];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:   %d\n",verbose);
		fprintf(stderr,"dbg2       file:      %s\n",file);
		fprintf(stderr,"dbg2       process:   %d\n",process);
		fprintf(stderr,"dbg2       mbp_ifile_specified:    %d\n",process->mbp_ifile_specified);
		fprintf(stderr,"dbg2       mbp_ifile:              %s\n",process->mbp_ifile);
		fprintf(stderr,"dbg2       mbp_ofile_specified:    %d\n",process->mbp_ofile_specified);
		fprintf(stderr,"dbg2       mbp_ofile:              %s\n",process->mbp_ofile);
		fprintf(stderr,"dbg2       mbp_format_specified:   %d\n",process->mbp_format_specified);
		fprintf(stderr,"dbg2       mbp_format:             %d\n",process->mbp_format);
		fprintf(stderr,"dbg2       mbp_nav_mode:           %d\n",process->mbp_nav_mode);
		fprintf(stderr,"dbg2       mbp_navfile:            %s\n",process->mbp_navfile);
		fprintf(stderr,"dbg2       mbp_nav_format:         %d\n",process->mbp_nav_format);
		fprintf(stderr,"dbg2       mbp_nav_heading:        %d\n",process->mbp_nav_heading);
		fprintf(stderr,"dbg2       mbp_nav_speed:          %d\n",process->mbp_nav_speed);
		fprintf(stderr,"dbg2       mbp_nav_draft:          %d\n",process->mbp_nav_draft);
		fprintf(stderr,"dbg2       mbp_nav_attitude:       %d\n",process->mbp_nav_attitude);
		fprintf(stderr,"dbg2       mbp_nav_algorithm:      %d\n",process->mbp_nav_algorithm);
		fprintf(stderr,"dbg2       mbp_nav_timeshift:      %f\n",process->mbp_nav_timeshift);
		fprintf(stderr,"dbg2       mbp_nav_shift:          %d\n",process->mbp_nav_shift);
		fprintf(stderr,"dbg2       mbp_nav_offsetx:        %f\n",process->mbp_nav_offsetx);
		fprintf(stderr,"dbg2       mbp_nav_offsety:        %f\n",process->mbp_nav_offsety);
		fprintf(stderr,"dbg2       mbp_nav_offsetz:        %f\n",process->mbp_nav_offsetz);
		fprintf(stderr,"dbg2       mbp_navadj_mode:        %d\n",process->mbp_navadj_mode);
		fprintf(stderr,"dbg2       mbp_navadjfile:         %s\n",process->mbp_navadjfile);
		fprintf(stderr,"dbg2       mbp_navadj_algorithm:   %d\n",process->mbp_navadj_algorithm);
		fprintf(stderr,"dbg2       mbp_attitude_mode:      %d\n",process->mbp_attitude_mode);
		fprintf(stderr,"dbg2       mbp_attitudefile:       %s\n",process->mbp_attitudefile);
		fprintf(stderr,"dbg2       mbp_attitude_format:    %d\n",process->mbp_attitude_format);
		fprintf(stderr,"dbg2       mbp_cut_num:            %d\n",process->mbp_cut_num);
		fprintf(stderr,"dbg2       mbp_sonardepth_mode:    %d\n",process->mbp_sonardepth_mode);
		fprintf(stderr,"dbg2       mbp_sonardepthfile:     %s\n",process->mbp_sonardepthfile);
		fprintf(stderr,"dbg2       mbp_sonardepth_format:  %d\n",process->mbp_sonardepth_format);
		fprintf(stderr,"dbg2       mbp_cut_num:            %d\n",process->mbp_cut_num);
		for (i=0;i<process->mbp_cut_num;i++)
			{
			fprintf(stderr,"dbg2           cut %d:\n",i);
			fprintf(stderr,"dbg2           mbp_cut_kind[%d]:     %d\n", i, process->mbp_cut_kind[i]);
			fprintf(stderr,"dbg2           mbp_cut_mode[%d]:     %d\n", i, process->mbp_cut_mode[i]);
			fprintf(stderr,"dbg2           mbp_cut_min[%d]:      %f\n", i, process->mbp_cut_min[i]);
			fprintf(stderr,"dbg2           mbp_cut_max[%d]:      %f\n", i, process->mbp_cut_max[i]);
			}
		fprintf(stderr,"dbg2       mbp_bathrecalc_mode:    %d\n",process->mbp_bathrecalc_mode);
		fprintf(stderr,"dbg2       mbp_rollbias_mode:      %d\n",process->mbp_rollbias_mode);
		fprintf(stderr,"dbg2       mbp_rollbias:           %f\n",process->mbp_rollbias);
		fprintf(stderr,"dbg2       mbp_rollbias_port:      %f\n",process->mbp_rollbias_port);
		fprintf(stderr,"dbg2       mbp_rollbias_stbd:      %f\n",process->mbp_rollbias_stbd);
		fprintf(stderr,"dbg2       mbp_pitchbias_mode:     %d\n",process->mbp_pitchbias_mode);
		fprintf(stderr,"dbg2       mbp_pitchbias:          %f\n",process->mbp_pitchbias);
		fprintf(stderr,"dbg2       mbp_draft_mode:         %d\n",process->mbp_draft_mode);
		fprintf(stderr,"dbg2       mbp_draft:              %f\n",process->mbp_draft);
		fprintf(stderr,"dbg2       mbp_draft_offset:       %f\n",process->mbp_draft_offset);
		fprintf(stderr,"dbg2       mbp_draft_mult:         %f\n",process->mbp_draft_mult);
		fprintf(stderr,"dbg2       mbp_heave_mode:         %d\n",process->mbp_heave_mode);
		fprintf(stderr,"dbg2       mbp_heave:              %f\n",process->mbp_heave);
		fprintf(stderr,"dbg2       mbp_heave_mult:         %f\n",process->mbp_heave_mult);
		fprintf(stderr,"dbg2       mbp_lever_mode:         %d\n",process->mbp_heave_mode);
		fprintf(stderr,"dbg2       mbp_vru_offsetx:        %f\n",process->mbp_vru_offsetx);
		fprintf(stderr,"dbg2       mbp_vru_offsety:        %f\n",process->mbp_vru_offsety);
		fprintf(stderr,"dbg2       mbp_vru_offsetz:        %f\n",process->mbp_vru_offsetz);
		fprintf(stderr,"dbg2       mbp_sonar_offsetx:      %f\n",process->mbp_sonar_offsetx);
		fprintf(stderr,"dbg2       mbp_sonar_offsety:      %f\n",process->mbp_sonar_offsety);
		fprintf(stderr,"dbg2       mbp_sonar_offsetz:      %f\n",process->mbp_sonar_offsetz);
		fprintf(stderr,"dbg2       mbp_ssv_mode:           %d\n",process->mbp_ssv_mode);
		fprintf(stderr,"dbg2       mbp_ssv:                %f\n",process->mbp_ssv);
		fprintf(stderr,"dbg2       mbp_svp_mode:           %d\n",process->mbp_svp_mode);
		fprintf(stderr,"dbg2       mbp_svpfile:            %s\n",process->mbp_svpfile);
		fprintf(stderr,"dbg2       mbp_corrected:          %d\n",process->mbp_corrected);
		fprintf(stderr,"dbg2       mbp_tt_mode:            %d\n",process->mbp_tt_mode);
		fprintf(stderr,"dbg2       mbp_tt_mult:            %f\n",process->mbp_tt_mult);
		fprintf(stderr,"dbg2       mbp_angle_mode:         %d\n",process->mbp_angle_mode);
		fprintf(stderr,"dbg2       mbp_static_mode:        %d\n",process->mbp_static_mode);
		fprintf(stderr,"dbg2       mbp_staticfile:         %s\n",process->mbp_staticfile);
		fprintf(stderr,"dbg2       mbp_heading_mode:       %d\n",process->mbp_heading_mode);
		fprintf(stderr,"dbg2       mbp_headingbias:        %f\n",process->mbp_headingbias);
		fprintf(stderr,"dbg2       mbp_edit_mode:          %d\n",process->mbp_edit_mode);
		fprintf(stderr,"dbg2       mbp_editfile:           %s\n",process->mbp_editfile);
		fprintf(stderr,"dbg2       mbp_tide_mode:          %d\n",process->mbp_tide_mode);
		fprintf(stderr,"dbg2       mbp_tidefile:           %s\n",process->mbp_tidefile);
		fprintf(stderr,"dbg2       mbp_tide_format:        %d\n",process->mbp_tide_format);
		fprintf(stderr,"dbg2       mbp_ampcorr_mode:       %d\n",process->mbp_ampcorr_mode);
		fprintf(stderr,"dbg2       mbp_ampcorrfile:        %s\n",process->mbp_ampcorrfile);
		fprintf(stderr,"dbg2       mbp_ampcorr_type:       %d\n",process->mbp_ampcorr_type);
		fprintf(stderr,"dbg2       mbp_ampcorr_symmetry:   %d\n",process->mbp_ampcorr_symmetry);
		fprintf(stderr,"dbg2       mbp_ampcorr_angle:      %f\n",process->mbp_ampcorr_angle);
		fprintf(stderr,"dbg2       mbp_ampcorr_slope:      %d\n",process->mbp_ampcorr_slope);
		fprintf(stderr,"dbg2       mbp_sscorr_mode:        %d\n",process->mbp_sscorr_mode);
		fprintf(stderr,"dbg2       mbp_sscorrfile:         %s\n",process->mbp_sscorrfile);
		fprintf(stderr,"dbg2       mbp_sscorr_type:        %d\n",process->mbp_sscorr_type);
		fprintf(stderr,"dbg2       mbp_sscorr_symmetry:    %d\n",process->mbp_sscorr_symmetry);
		fprintf(stderr,"dbg2       mbp_sscorr_angle:       %f\n",process->mbp_sscorr_angle);
		fprintf(stderr,"dbg2       mbp_sscorr_slope:       %d\n",process->mbp_sscorr_slope);
		fprintf(stderr,"dbg2       mbp_ssrecalc_mode:      %d\n",process->mbp_ssrecalc_mode);
		fprintf(stderr,"dbg2       mbp_ssrecalc_pixelsize: %f\n",process->mbp_ssrecalc_pixelsize);
		fprintf(stderr,"dbg2       mbp_ssrecalc_swathwidth:%f\n",process->mbp_ssrecalc_swathwidth);
		fprintf(stderr,"dbg2       mbp_ssrecalc_interp    :%d\n",process->mbp_ssrecalc_interpolate);
		fprintf(stderr,"dbg2       mbp_meta_vessel        :%s\n",process->mbp_meta_vessel);
		fprintf(stderr,"dbg2       mbp_meta_institution   :%s\n",process->mbp_meta_institution);
		fprintf(stderr,"dbg2       mbp_meta_platform      :%s\n",process->mbp_meta_platform);
		fprintf(stderr,"dbg2       mbp_meta_sonar         :%s\n",process->mbp_meta_sonar);
		fprintf(stderr,"dbg2       mbp_meta_sonarversion  :%s\n",process->mbp_meta_sonarversion);
		fprintf(stderr,"dbg2       mbp_meta_cruiseid      :%s\n",process->mbp_meta_cruiseid);
		fprintf(stderr,"dbg2       mbp_meta_cruisename    :%s\n",process->mbp_meta_cruisename);
		fprintf(stderr,"dbg2       mbp_meta_pi            :%s\n",process->mbp_meta_pi);
		fprintf(stderr,"dbg2       mbp_meta_piinstitution :%s\n",process->mbp_meta_piinstitution);
		fprintf(stderr,"dbg2       mbp_meta_client        :%s\n",process->mbp_meta_client);
		fprintf(stderr,"dbg2       mbp_meta_svcorrected   :%d\n",process->mbp_meta_svcorrected);
		fprintf(stderr,"dbg2       mbp_meta_tidecorrected :%d\n",process->mbp_meta_tidecorrected);
		fprintf(stderr,"dbg2       mbp_meta_batheditmanual:%d\n",process->mbp_meta_batheditmanual);
		fprintf(stderr,"dbg2       mbp_meta_batheditauto:  %d\n",process->mbp_meta_batheditauto);
		fprintf(stderr,"dbg2       mbp_meta_rollbias:      %f\n",process->mbp_meta_rollbias);
		fprintf(stderr,"dbg2       mbp_meta_pitchbias:     %f\n",process->mbp_meta_pitchbias);
		fprintf(stderr,"dbg2       mbp_meta_headingbias:   %f\n",process->mbp_meta_headingbias);
		fprintf(stderr,"dbg2       mbp_meta_draft:         %f\n",process->mbp_meta_draft);
		fprintf(stderr,"dbg2       mbp_kluge001:           %d\n",process->mbp_kluge001);
		fprintf(stderr,"dbg2       mbp_kluge002:           %d\n",process->mbp_kluge002);
		fprintf(stderr,"dbg2       mbp_kluge003:           %d\n",process->mbp_kluge003);
		fprintf(stderr,"dbg2       mbp_kluge004:           %d\n",process->mbp_kluge004);
		fprintf(stderr,"dbg2       mbp_kluge005:           %d\n",process->mbp_kluge005);
		}
		
	/* try to avoid absolute pathnames - get pwd */
	lastslash = strrchr(file, '/');
	if (file[0] == '/')
	    {
	    strcpy(pwd, file);
	    pwd[strlen(file) - strlen(lastslash)] = '\0';
	    }
	else
	    {
	    getcwd(pwd, MB_PATH_MAXLINE);
	    if (lastslash != NULL)
		{
		strcat(pwd, "/");
		strcat(pwd, file);
		lastslash = strrchr(pwd, '/');
		pwd[strlen(pwd) - strlen(lastslash)] = '\0';
		}
	    }
	mb_get_shortest_path(verbose, pwd, error);
	
	/* get expected process parameter file name */
	strcpy(parfile, file);
	strcat(parfile, ".par");

	/* open parameter file */
	if ((fp = fopen(parfile, "w")) != NULL) 
	    {
	    fprintf(fp,"## MB-System processing parameter file\n");
	    fprintf(fp,"## Written by %s version %s\n", function_name, rcs_id);
	    fprintf(fp,"## MB-system Version %s\n",MB_VERSION);
	    right_now = time((time_t *)0);
	    strncpy(date,ctime(&right_now),24);
	    date[24] = 0;
	    if ((user_ptr = getenv("USER")) == NULL)
		    user_ptr = getenv("LOGNAME");
	    if (user_ptr != NULL)
		    strcpy(user,user_ptr);
	    else
		    strcpy(user, "unknown");
	    gethostname(host,MBP_FILENAMESIZE);
	    fprintf(fp,"## Generated by user <%s> on cpu <%s> at <%s>\n##\n",
		    user,host,date);

	    /* general parameters */
	    fprintf(fp, "##\n## Forces explicit reading of parameter modes.\n");
	    fprintf(fp, "EXPLICIT\n");
	    fprintf(fp, "##\n## General Parameters:\n");
	    if (process->mbp_format_specified == MB_YES)
		{
		fprintf(fp, "FORMAT %d\n", process->mbp_format);
		}
	    else
		{
		fprintf(fp, "## FORMAT format\n");
		}
	    if (process->mbp_ifile_specified == MB_YES)
		{
		strcpy(relative_path, process->mbp_ifile);
		status = mb_get_relative_path(verbose, relative_path, pwd, error);
		fprintf(fp, "INFILE %s\n", relative_path);
		}
	    else
		{
		fprintf(fp, "## INFILE infile\n");
		}
	    if (process->mbp_ofile_specified == MB_YES)
		{
		strcpy(relative_path, process->mbp_ofile);
		status = mb_get_relative_path(verbose, relative_path, pwd, error);
		fprintf(fp, "OUTFILE %s\n", relative_path);
		}
	    else
		{
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
	    fprintf(fp, "NAVSHIFT %d\n", process->mbp_nav_shift);
	    fprintf(fp, "NAVOFFSETX %f\n", process->mbp_nav_offsetx);
	    fprintf(fp, "NAVOFFSETY %f\n", process->mbp_nav_offsety);
	    fprintf(fp, "NAVOFFSETZ %f\n", process->mbp_nav_offsetz);
	    
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
	    
	    /* sonardepth merging */
	    fprintf(fp, "##\n## Sonardepth Merging:\n");
	    fprintf(fp, "SONARDEPTHMODE %d\n", process->mbp_sonardepth_mode);
	    strcpy(relative_path, process->mbp_sonardepthfile);
	    status = mb_get_relative_path(verbose, relative_path, pwd, error);
	    fprintf(fp, "SONARDEPTHFILE %s\n", relative_path);
	    fprintf(fp, "SONARDEPTHFORMAT %d\n", process->mbp_sonardepth_format);

	    /* data cutting */
	    fprintf(fp, "##\n## Data cutting:\n");
	    if (process->mbp_cut_num == 0)
	    	fprintf(fp, "DATACUTCLEAR\n");
	    else
		{
		for (i=0;i<process->mbp_cut_num;i++)
	    		fprintf(fp, "DATACUT %d %d %f %f\n", 
				process->mbp_cut_kind[i],
				process->mbp_cut_mode[i],
				process->mbp_cut_min[i],
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
	    if (process->mbp_kluge001 == MB_YES)
	    	fprintf(fp, "KLUGE001\n");
	    if (process->mbp_kluge002 == MB_YES)
	    	fprintf(fp, "KLUGE002\n");
	    if (process->mbp_kluge003 == MB_YES)
	    	fprintf(fp, "KLUGE003\n");
	    if (process->mbp_kluge004 == MB_YES)
	    	fprintf(fp, "KLUGE004\n");
	    if (process->mbp_kluge005 == MB_YES)
	    	fprintf(fp, "KLUGE005\n");
  	
	    /* close file */
	    fclose(fp);
	    }
		
	/* set error */
	else
	    {
	    *error = MB_ERROR_OPEN_FAIL;
	    status = MB_FAILURE;
	    if (verbose > 0)
		fprintf(stderr,"\nUnable to Open Parameter File <%s> for writing\n",parfile);
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_bathmode(int verbose, struct mb_process_struct *process, 
			int *error)
{
	char	*function_name = "mb_pr_bathmode";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:   %d\n",verbose);
		fprintf(stderr,"dbg2       process:   %d\n",process);
		}

	/* figure out bathymetry recalculation mode */
	if (process->mbp_svp_mode == MBP_SVP_ON)
	    process->mbp_bathrecalc_mode = MBP_BATHRECALC_RAYTRACE;
	else if (process->mbp_svp_mode != MBP_SVP_ON
	    && (process->mbp_rollbias_mode != MBP_ROLLBIAS_OFF
		|| process->mbp_pitchbias_mode != MBP_PITCHBIAS_OFF
		|| process->mbp_nav_attitude != MBP_NAV_OFF
		|| process->mbp_attitude_mode != MBP_ATTITUDE_OFF))
	    process->mbp_bathrecalc_mode = MBP_BATHRECALC_ROTATE;
	else if (process->mbp_svp_mode != MBP_SVP_ON
	    && process->mbp_rollbias_mode == MBP_ROLLBIAS_OFF
		&& (process->mbp_draft_mode != MBP_DRAFT_OFF
		    || process->mbp_nav_draft != MBP_NAV_OFF
		    || process->mbp_sonardepth_mode != MBP_SONARDEPTH_OFF
		    || process->mbp_lever_mode != MBP_LEVER_OFF
		    || process->mbp_svp_mode == MBP_SVP_SOUNDSPEEDREF))
	    process->mbp_bathrecalc_mode = MBP_BATHRECALC_OFFSET;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_default_output(int verbose, struct mb_process_struct *process, 
			int *error)
{
	char	*function_name = "mb_pr_default_output";
	int	status = MB_SUCCESS;
	char	fileroot[MBP_FILENAMESIZE];
	int	format;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:   %d\n",verbose);
		fprintf(stderr,"dbg2       process:   %d\n",process);
		fprintf(stderr,"dbg2       mbp_ifile_specified: %d\n",process->mbp_ifile_specified);
		fprintf(stderr,"dbg2       mbp_ifile:           %s\n",process->mbp_ifile);
		fprintf(stderr,"dbg2       mbp_format_specified:%d\n",process->mbp_format_specified);
		fprintf(stderr,"dbg2       mbp_format:          %d\n",process->mbp_format);
		}
   
	/* figure out data format and fileroot if possible */
	status = mb_get_format(verbose, process->mbp_ifile, 
					fileroot, &format, error);
				
	/* deal with format */
	if (status == MB_SUCCESS && format > 0)
	    {
	    /* set format if found */
	    if (process->mbp_format_specified == MB_NO)
		{
		process->mbp_format = format;
		process->mbp_format_specified = MB_YES;
		}
		
	    /* set output file if needed */
	    if (process->mbp_ofile_specified == MB_NO
		&& process->mbp_format_specified == MB_YES)
		{
		/* use .txt suffix if MBARI ROV navigation */
		if (process->mbp_format == MBF_MBARIROV)
		    sprintf(process->mbp_ofile, "%sedited.txt", 
			    fileroot);
		/* else use standard .mbXXX suffix */
		else
		    sprintf(process->mbp_ofile, "%sp.mb%d", 
			    fileroot, process->mbp_format);
		process->mbp_ofile_specified = MB_YES;
		}
	    }
	else if (process->mbp_ofile_specified == MB_NO
		&& process->mbp_format_specified == MB_YES)
	    {
	    sprintf(process->mbp_ofile, "%sp.mb%d", 
			process->mbp_ifile, process->mbp_format);
	    process->mbp_ofile_specified = MB_YES;
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_ofile_specified: %d\n",process->mbp_ofile_specified);
		fprintf(stderr,"dbg2       mbp_ofile:           %s\n",process->mbp_ofile);
		fprintf(stderr,"dbg2       mbp_format_specified:%d\n",process->mbp_format_specified);
		fprintf(stderr,"dbg2       mbp_format:          %d\n",process->mbp_format);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_output(int verbose, int *format, 
			char *ifile, char *ofile, 
			int *error)
{
	char	*function_name = "mb_pr_get_output";
	int	status = MB_SUCCESS;
	char	fileroot[MBP_FILENAMESIZE];
	int	tformat;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:	%d\n",verbose);
		fprintf(stderr,"dbg2       format:	%d\n",*format);
		fprintf(stderr,"dbg2       ifile:	%s\n",ifile);
		}
   
	/* figure out data format and fileroot if possible */
	status = mb_get_format(verbose, ifile, 
					fileroot, &tformat, error);
				
	/* use fileroot if possible */
	if (status == MB_SUCCESS)
	    {
	    /* set format if needed */
	    if (*format <= 0)
		*format = tformat;
		
	    /* use .txt suffix if MBARI ROV navigation */
	    if (*format == MBF_MBARIROV)
		sprintf(ofile, "%sedited.txt", 
			fileroot);
			
	    /* else use standard .mbXXX suffix */
	    else
		sprintf(ofile, "%sp.mb%d", 
			fileroot, *format);
	    }
	    
	/* else just add suffix */
	else if (*format > 0)
	    {
	    sprintf(ofile, "%sp.mb%d", 
			ifile, *format);
	    status = MB_SUCCESS;
	    *error = MB_ERROR_NO_ERROR;
	    }
	    
	/* else failure */
	else
	    {
	    sprintf(ofile, "%s.proc", 
		    ifile);	    
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       format:	%d\n",*format);
		fprintf(stderr,"dbg2       ofile:	%s\n",ofile);
		fprintf(stderr,"dbg2       error:	%d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:	%d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_check(int verbose, char *ifile, 
			int *nparproblem, 	
			int *ndataproblem, 	
			int *error)
{
	char	*function_name = "mb_pr_check";
	int	status = MB_SUCCESS;
	struct mb_process_struct process;
	char	ofile[MBP_FILENAMESIZE];
	int	format;
	char	line[MB_PATH_MAXLINE];
	FILE	*fp;
	int	dataproblemid;
	int	unexpected_format = MB_NO;
	int	unexpected_output = MB_NO;
	int	missing_ifile = MB_NO;
	int	missing_ofile = MB_NO;
	int	missing_navfile = MB_NO;
	int	missing_navadjfile = MB_NO;
	int	missing_attitudefile = MB_NO;
	int	missing_sonardepthfile = MB_NO;
	int	missing_svpfile = MB_NO;
	int	missing_editfile = MB_NO;
	int	missing_tidefile = MB_NO;
	struct	stat statbuf;
	
	/* output stream for basic stuff (stdout if verbose <= 1,
		output if verbose > 1) */
	FILE	*output;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:   %d\n",verbose);
		fprintf(stderr,"dbg2       ifile:     %s\n",ifile);
		}

	/* set output stream */
	if (verbose <= 1)
		output = stdout;
	else
		output = stderr;
		
	/* set no problem */
	*nparproblem = 0;
	*ndataproblem = 0;
	unexpected_format = MB_NO;
	unexpected_output = MB_NO;
	missing_ifile = MB_NO;
	missing_ofile = MB_NO;
	missing_navfile = MB_NO;
	missing_navadjfile = MB_NO;
	missing_attitudefile = MB_NO;
	missing_sonardepthfile = MB_NO;
	missing_svpfile = MB_NO;
	missing_editfile = MB_NO;
	missing_tidefile = MB_NO;
	    
	/* check if input exists */
	if (stat(ifile, &statbuf) != 0)
	    {
	    missing_ifile = MB_YES;
	    *nparproblem++;
	    }
	
	/* only check parameter file if parameter file exists */
	sprintf(ofile, "%s.par", ifile);
	if (stat(ofile, &statbuf) == 0)
	    {

	    /* get known process parameters */
	    status = mb_pr_readpar(verbose, ifile, MB_NO, &process, error);
       
	    /* get default data format and output file */
	    format = 0;
	    status = mb_pr_get_output(verbose, &format, process.mbp_ifile, 
					    ofile, error);
    
	    /* check data format */
	    if (status == MB_SUCCESS 
		&& process.mbp_format_specified == MB_YES
		&& format != 0
		&& process.mbp_format != format)
		{
		unexpected_format = MB_YES;
		*nparproblem++;
		
		/* get output file with specified format */
		status = mb_pr_get_output(verbose, &process.mbp_format, process.mbp_ifile, 
					    ofile, error);	    
		}
		
	    /* check output file */
	    if (status == MB_SUCCESS
		&& process.mbp_ofile_specified == MB_YES
		&& format != 0)
		{
		if (strcmp(process.mbp_ofile, ofile) != 0)
		    {
		    unexpected_output = MB_YES;
		    *nparproblem++;
		    }
		}
    
	    /* check if output file specified but does not exist */
	    if (process.mbp_ofile_specified == MB_YES
		&& stat(process.mbp_ofile, &statbuf) != 0)
		{
		missing_ofile = MB_YES;
		*nparproblem++;
		}
    
	    /* check if nav file specified but does not exist */
	    if (process.mbp_nav_mode == MBP_NAV_ON
		&& stat(process.mbp_navfile, &statbuf) != 0)
		{
		missing_navfile = MB_YES;
		*nparproblem++;
		}
    
	    /* check if navadj file specified but does not exist */
	    if (process.mbp_navadj_mode == MBP_NAV_ON
		&& stat(process.mbp_navadjfile, &statbuf) != 0)
		{
		missing_navadjfile = MB_YES;
		*nparproblem++;
		}
   
	    /* check if attitude file specified but does not exist */
	    if (process.mbp_attitude_mode == MBP_ATTITUDE_ON
		&& stat(process.mbp_attitudefile, &statbuf) != 0)
		{
		missing_attitudefile = MB_YES;
		*nparproblem++;
		}
   
	    /* check if sonardepth file specified but does not exist */
	    if (process.mbp_sonardepth_mode == MBP_SONARDEPTH_ON
		&& stat(process.mbp_sonardepthfile, &statbuf) != 0)
		{
		missing_sonardepthfile = MB_YES;
		*nparproblem++;
		}
    
	    /* check if svp file specified but does not exist */
	    if (process.mbp_svp_mode == MBP_SVP_ON
		&& stat(process.mbp_svpfile, &statbuf) != 0)
		{
		missing_svpfile = MB_YES;
		*nparproblem++;
		}
    
	    /* check if edit file specified but does not exist */
	    if (process.mbp_edit_mode == MBP_EDIT_ON
		&& stat(process.mbp_editfile, &statbuf) != 0)
		{
		missing_editfile = MB_YES;
		*nparproblem++;
		}
    
	    /* check if tide file specified but does not exist */
	    if (process.mbp_tide_mode == MBP_TIDE_ON
		&& stat(process.mbp_tidefile, &statbuf) != 0)
		{
		missing_tidefile = MB_YES;
		*nparproblem++;
		}
	    }
	
	/* only check inf file if inf file exists */
	sprintf(ofile, "%s.inf", ifile);
	if (stat(ofile, &statbuf) == 0)
	    {
	    /* open if possible */
	    if ((fp = fopen(ofile,"r")) != NULL)
		    {
		    /* read the inf file */
		    while (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
			{
			if (strncmp(line, "PN: ", 4) == 0)
			    {
			    if (*ndataproblem == 0 && verbose > 0)
			    	fprintf(output, "\nData File Problems: %s\n", ifile);
			    fprintf(output, "%s: %s", ifile, &line[4]);
			    (*ndataproblem)++;
			    }
			}
		    }
	    }

	    
	/* output results */
	if (*nparproblem > 0 && verbose > 0)
	    {
	    fprintf(output, "\nParameter File Problems: %s\n", ifile);
	    if (unexpected_format == MB_YES)
		fprintf(output, "\tUnexpected format: %d instead of %d\n", 
			process.mbp_format, format);
	    if (unexpected_output == MB_YES)
		fprintf(output, "\tUnexpected output: %s instead of %s\n", 
			process.mbp_ofile, ofile);
	    if (missing_ifile == MB_YES)
		fprintf(output, "\tMissing input file: %s does not exist\n", 
			process.mbp_ifile);
	    if (missing_ofile == MB_YES)
		fprintf(output, "\tMissing output file: %s does not exist\n", 
			process.mbp_ofile);
	    if (missing_navfile == MB_YES)
		fprintf(output, "\tMissing nav file: %s does not exist\n", 
			process.mbp_navfile);
	    if (missing_navadjfile == MB_YES)
		fprintf(output, "\tMissing navadj file: %s does not exist\n", 
			process.mbp_navadjfile);
	    if (missing_attitudefile == MB_YES)
		fprintf(output, "\tMissing attitude file: %s does not exist\n", 
			process.mbp_attitudefile);
	    if (missing_sonardepthfile == MB_YES)
		fprintf(output, "\tMissing sonardepth file: %s does not exist\n", 
			process.mbp_sonardepthfile);
	    if (missing_svpfile == MB_YES)
		fprintf(output, "\tMissing svp file: %s does not exist\n", 
			process.mbp_svpfile);
	    if (missing_editfile == MB_YES)
		fprintf(output, "\tMissing edit file: %s does not exist\n", 
			process.mbp_editfile);
	    if (missing_tidefile == MB_YES)
		fprintf(output, "\tMissing tide file: %s does not exist\n", 
			process.mbp_tidefile);
	    }
	else if (*nparproblem > 0)
	    {
	    if (unexpected_format == MB_YES)
		fprintf(output, "%s : Unexpected format : %d\n", 
			process.mbp_ifile, process.mbp_format);
	    if (unexpected_output == MB_YES)
		fprintf(output, "%s : Unexpected output : %s\n", 
			process.mbp_ifile, process.mbp_ofile);
	    if (missing_ifile == MB_YES)
		fprintf(output, "%s : Missing input file : %s\n", 
			process.mbp_ifile, process.mbp_ifile);
	    if (missing_ofile == MB_YES)
		fprintf(output, "%s : Missing output file : %s\n", 
			process.mbp_ifile, process.mbp_ofile);
	    if (missing_navfile == MB_YES)
		fprintf(output, "%s : Missing nav file : %s\n", 
			process.mbp_ifile, process.mbp_navfile);
	    if (missing_navadjfile == MB_YES)
		fprintf(output, "%s : Missing navadj file : %s\n", 
			process.mbp_ifile, process.mbp_navadjfile);
	    if (missing_attitudefile == MB_YES)
		fprintf(output, "%s : Missing attitude file : %s\n", 
			process.mbp_ifile, process.mbp_attitudefile);
	    if (missing_sonardepthfile == MB_YES)
		fprintf(output, "%s : Missing sonardepth file : %s\n", 
			process.mbp_ifile, process.mbp_sonardepthfile);
	    if (missing_svpfile == MB_YES)
		fprintf(output, "%s : Missing svp file : %s\n", 
			process.mbp_ifile, process.mbp_svpfile);
	    if (missing_editfile == MB_YES)
		fprintf(output, "%s : Missing edit file : %s\n", 
			process.mbp_ifile, process.mbp_editfile);
	    if (missing_tidefile == MB_YES)
		fprintf(output, "%s : Missing tide file : %s\n", 
			process.mbp_ifile, process.mbp_tidefile);
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       nparproblem:  %d\n",*nparproblem);
		fprintf(stderr,"dbg2       ndataproblem: %d\n",*ndataproblem);
		fprintf(stderr,"dbg2       error:        %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:       %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_ofile(int verbose, char *file, 
			int	mbp_ofile_specified, 
			char	*mbp_ofile, 
			int	*error)
{
	char	*function_name = "mb_pr_update_ofile";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:             %d\n",verbose);
		fprintf(stderr,"dbg2       file:                %s\n",file);
		fprintf(stderr,"dbg2       mbp_ofile_specified: %d\n",mbp_ofile_specified);
		fprintf(stderr,"dbg2       ofile:               %s\n",mbp_ofile);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set ofile value */
	if (mbp_ofile != NULL)
	    {
	    strcpy(process.mbp_ofile, mbp_ofile);
	    process.mbp_ofile_specified = mbp_ofile_specified;
	    }
	else
	    {
	    process.mbp_ofile[0] = '\0';
	    process.mbp_ofile_specified = MB_NO;
	    }

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_format(int verbose, char *file, 
			int mbp_format_specified, 
			int mbp_format, 
			int *error)
{
	char	*function_name = "mb_pr_update_format";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:              %d\n",verbose);
		fprintf(stderr,"dbg2       file:                 %s\n",file);
		fprintf(stderr,"dbg2       mbp_format_specified: %d\n",mbp_format_specified);
		fprintf(stderr,"dbg2       mbp_format:           %d\n",mbp_format);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set format value */
	process.mbp_format_specified = mbp_format_specified;
	process.mbp_format = mbp_format;

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_rollbias(int verbose, char *file, 
			int	mbp_rollbias_mode, 
			double	mbp_rollbias, 
			double	mbp_rollbias_port, 
			double	mbp_rollbias_stbd, 
			int *error)
{
	char	*function_name = "mb_pr_update_rollbias";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		fprintf(stderr,"dbg2       mbp_rollbias_mode: %d\n",mbp_rollbias_mode);
		fprintf(stderr,"dbg2       mbp_rollbias:      %f\n",mbp_rollbias);
		fprintf(stderr,"dbg2       mbp_rollbias_port: %f\n",mbp_rollbias_port);
		fprintf(stderr,"dbg2       mbp_rollbias_stbd: %f\n",mbp_rollbias_stbd);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set rollbias values */
	process.mbp_rollbias_mode = mbp_rollbias_mode;
	process.mbp_rollbias = mbp_rollbias;
	process.mbp_rollbias_port = mbp_rollbias_port;
	process.mbp_rollbias_stbd = mbp_rollbias_stbd;
	    
	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, &process, error);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_pitchbias(int verbose, char *file, 
			int	mbp_pitchbias_mode, 
			double	mbp_pitchbias, 
			int *error)
{
	char	*function_name = "mb_pr_update_pitchbias";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		fprintf(stderr,"dbg2       mbp_pitchbias_mode: %d\n",mbp_pitchbias_mode);
		fprintf(stderr,"dbg2       mbp_pitchbias:      %f\n",mbp_pitchbias);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set pitchbias values */
	process.mbp_pitchbias_mode = mbp_pitchbias_mode;
	process.mbp_pitchbias = mbp_pitchbias;
	    
	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, &process, error);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_draft(int verbose, char *file, 
			int	mbp_draft_mode, 
			double	mbp_draft, 
			double	mbp_draft_offset, 
			double	mbp_draft_mult, 
			int *error)
{
	char	*function_name = "mb_pr_update_draft";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		fprintf(stderr,"dbg2       mbp_draft_mode:    %d\n",mbp_draft_mode);
		fprintf(stderr,"dbg2       mbp_draft:         %f\n",mbp_draft);
		fprintf(stderr,"dbg2       mbp_draft_offset:  %f\n",mbp_draft_offset);
		fprintf(stderr,"dbg2       mbp_draft_mult:    %f\n",mbp_draft_mult);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set draft values */
	process.mbp_draft_mode = mbp_draft_mode;
	process.mbp_draft = mbp_draft;
	process.mbp_draft_offset = mbp_draft_offset;
	process.mbp_draft_mult = mbp_draft_mult;
	    
	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, &process, error);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_heave(int verbose, char *file, 
			int	mbp_heave_mode, 
			double	mbp_heave, 
			double	mbp_heave_mult, 
			int *error)
{
	char	*function_name = "mb_pr_update_heave";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		fprintf(stderr,"dbg2       mbp_heave_mode:    %d\n",mbp_heave_mode);
		fprintf(stderr,"dbg2       mbp_heave:         %f\n",mbp_heave);
		fprintf(stderr,"dbg2       mbp_heave_mult:    %f\n",mbp_heave_mult);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set heave values */
	process.mbp_heave_mode = mbp_heave_mode;
	process.mbp_heave = mbp_heave;
	process.mbp_heave_mult = mbp_heave_mult;
	    
	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, &process, error);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_lever(int verbose, char *file, 
			int	mbp_lever_mode, 
			double	mbp_vru_offsetx, 
			double	mbp_vru_offsety, 
			double	mbp_vru_offsetz, 
			double	mbp_sonar_offsetx, 
			double	mbp_sonar_offsety, 
			double	mbp_sonar_offsetz, 
			int *error)
{
	char	*function_name = "mb_pr_update_lever";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		fprintf(stderr,"dbg2       mbp_lever_mode:    %d\n",mbp_lever_mode);
		fprintf(stderr,"dbg2       mbp_vru_offsetx:   %f\n",mbp_vru_offsetx);
		fprintf(stderr,"dbg2       mbp_vru_offsety:   %f\n",mbp_vru_offsety);
		fprintf(stderr,"dbg2       mbp_vru_offsetz:   %f\n",mbp_vru_offsetz);
		fprintf(stderr,"dbg2       mbp_sonar_offsetx: %f\n",mbp_sonar_offsetx);
		fprintf(stderr,"dbg2       mbp_sonar_offsety: %f\n",mbp_sonar_offsety);
		fprintf(stderr,"dbg2       mbp_sonar_offsetz: %f\n",mbp_sonar_offsetz);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

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

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_tide(int verbose, char *file, 
			int	mbp_tide_mode, 
			char *mbp_tidefile, 
			int	mbp_tide_format, 
			int *error)
{
	char	*function_name = "mb_pr_update_tide";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		fprintf(stderr,"dbg2       mbp_tide_mode:     %d\n",mbp_tide_mode);
		fprintf(stderr,"dbg2       mbp_tidefile:      %s\n",mbp_tidefile);
		fprintf(stderr,"dbg2       mbp_tide_format:   %d\n",mbp_tide_format);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set lever values */
	process.mbp_tide_mode = mbp_tide_mode;
	if (mbp_tidefile != NULL)
		strcpy(process.mbp_tidefile,mbp_tidefile);
	process.mbp_tide_format = mbp_tide_format;
	    
	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, &process, error);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_tt(int verbose, char *file, 
			int	mbp_tt_mode, 
			double	mbp_tt_mult, 
			int *error)
{
	char	*function_name = "mb_pr_update_tt";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		fprintf(stderr,"dbg2       mbp_tt_mode:       %d\n",mbp_tt_mode);
		fprintf(stderr,"dbg2       mbp_tt_mult:       %f\n",mbp_tt_mult);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set tt values */
	process.mbp_tt_mode = mbp_tt_mode;
	process.mbp_tt_mult = mbp_tt_mult;
	    
	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, &process, error);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_ssv(int verbose, char *file, 
			int	mbp_ssv_mode, 
			double	mbp_ssv, 
			int *error)
{
	char	*function_name = "mb_pr_update_ssv";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		fprintf(stderr,"dbg2       mbp_ssv_mode:      %d\n",mbp_ssv_mode);
		fprintf(stderr,"dbg2       mbp_ssv:           %f\n",mbp_ssv);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set ssv values */
	process.mbp_ssv_mode = mbp_ssv_mode;
	process.mbp_ssv = mbp_ssv;
	    
	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, &process, error);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_svp(int verbose, char *file, 
			int	mbp_svp_mode, 
			char	*mbp_svpfile, 
			int	mbp_angle_mode, 
			int	mbp_corrected, 
			int *error)
{
	char	*function_name = "mb_pr_update_svp";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		fprintf(stderr,"dbg2       mbp_svp_mode:      %d\n",mbp_svp_mode);
		fprintf(stderr,"dbg2       mbp_svpfile:       %s\n",mbp_svpfile);
		fprintf(stderr,"dbg2       mbp_angle_mode:    %d\n",mbp_angle_mode);
		fprintf(stderr,"dbg2       mbp_corrected:     %d\n",mbp_corrected);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

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

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_static(int verbose, char *file, 
			int	mbp_static_mode, 
			char	*mbp_staticfile, 
			int *error)
{
	char	*function_name = "mb_pr_update_static";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		fprintf(stderr,"dbg2       mbp_static_mode:   %d\n",mbp_static_mode);
		fprintf(stderr,"dbg2       mbp_staticfile:    %s\n",mbp_staticfile);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set svp values */
	process.mbp_static_mode = mbp_static_mode;
	if (mbp_staticfile != NULL)
	    strcpy(process.mbp_staticfile, mbp_staticfile);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_navadj(int verbose, char *file, 
			int	mbp_navadj_mode, 
			char	*mbp_navadjfile, 
			int	mbp_navadj_algorithm, 
			int *error)
{
	char	*function_name = "mb_pr_update_navadj";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:              %d\n",verbose);
		fprintf(stderr,"dbg2       file:                 %s\n",file);
		fprintf(stderr,"dbg2       mbp_navadj_mode:      %d\n",mbp_navadj_mode);
		fprintf(stderr,"dbg2       mbp_navadjfile:       %s\n",mbp_navadjfile);
		fprintf(stderr,"dbg2       mbp_navadj_algorithm: %d\n",mbp_navadj_algorithm);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set navadj values */
	process.mbp_navadj_mode = mbp_navadj_mode;
	if (mbp_navadjfile != NULL)
	    strcpy(process.mbp_navadjfile, mbp_navadjfile);
	process.mbp_navadj_algorithm = mbp_navadj_algorithm;

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_attitude(int verbose, char *file, 
			int	mbp_attitude_mode, 
			char *mbp_attitudefile, 
			int	mbp_attitude_format, 
			int *error)
{
	char	*function_name = "mb_pr_update_attitude";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		fprintf(stderr,"dbg2       mbp_attitude_mode: %d\n",mbp_attitude_mode);
		fprintf(stderr,"dbg2       mbp_attitudefile:  %s\n",mbp_attitudefile);
		fprintf(stderr,"dbg2       mbp_attitude_format:%d\n",mbp_attitude_format);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set lever values */
	process.mbp_attitude_mode = mbp_attitude_mode;
	if (mbp_attitudefile != NULL)
		strcpy(process.mbp_attitudefile,mbp_attitudefile);
	process.mbp_attitude_format = mbp_attitude_format;
	    
	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, &process, error);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_sonardepth(int verbose, char *file, 
			int	mbp_sonardepth_mode, 
			char *mbp_sonardepthfile, 
			int	mbp_sonardepth_format, 
			int *error)
{
	char	*function_name = "mb_pr_update_sonardepth";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:               %d\n",verbose);
		fprintf(stderr,"dbg2       file:                  %s\n",file);
		fprintf(stderr,"dbg2       mbp_sonardepth_mode:   %d\n",mbp_sonardepth_mode);
		fprintf(stderr,"dbg2       mbp_sonardepthfile:    %s\n",mbp_sonardepthfile);
		fprintf(stderr,"dbg2       mbp_sonardepth_format: %d\n",mbp_sonardepth_format);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set lever values */
	process.mbp_sonardepth_mode = mbp_sonardepth_mode;
	if (mbp_sonardepthfile != NULL)
		strcpy(process.mbp_sonardepthfile,mbp_sonardepthfile);
	process.mbp_sonardepth_format = mbp_sonardepth_format;
	    
	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, &process, error);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_nav(int verbose, char *file, 
			int	mbp_nav_mode, 
			char	*mbp_navfile, 
			int	mbp_nav_format, 
			int	mbp_nav_heading, 
			int	mbp_nav_speed, 
			int	mbp_nav_draft, 
			int	mbp_nav_attitude, 
			int	mbp_nav_algorithm, 
			double mbp_nav_timeshift,
			int *error)
{
	char	*function_name = "mb_pr_update_nav";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		fprintf(stderr,"dbg2       mbp_nav_mode:      %d\n",mbp_nav_mode);
		fprintf(stderr,"dbg2       mbp_navfile:       %s\n",mbp_navfile);
		fprintf(stderr,"dbg2       mbp_nav_format:    %d\n",mbp_nav_format);
		fprintf(stderr,"dbg2       mbp_nav_heading:   %d\n",mbp_nav_heading);
		fprintf(stderr,"dbg2       mbp_nav_speed:     %d\n",mbp_nav_speed);
		fprintf(stderr,"dbg2       mbp_nav_draft:     %d\n",mbp_nav_draft);
		fprintf(stderr,"dbg2       mbp_nav_attitude:  %d\n",mbp_nav_attitude);
		fprintf(stderr,"dbg2       mbp_nav_algorithm: %d\n",mbp_nav_algorithm);
		fprintf(stderr,"dbg2       mbp_nav_timeshift: %f\n",mbp_nav_timeshift);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

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

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_navshift(int verbose, char *file, 
			int	mbp_nav_shift, 
			double	mbp_nav_offsetx, 
			double	mbp_nav_offsety, 
			double	mbp_nav_offsetz, 
			int *error)
{
	char	*function_name = "mb_pr_update_navshift";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		fprintf(stderr,"dbg2       mbp_nav_shift:     %d\n",mbp_nav_shift);
		fprintf(stderr,"dbg2       mbp_nav_offsetx:   %f\n",mbp_nav_offsetx);
		fprintf(stderr,"dbg2       mbp_nav_offsety:   %f\n",mbp_nav_offsety);
		fprintf(stderr,"dbg2       mbp_nav_offsetz:   %f\n",mbp_nav_offsetz);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set nav values */
	process.mbp_nav_shift = mbp_nav_shift;
	process.mbp_nav_offsetx = mbp_nav_offsetx;
	process.mbp_nav_offsety = mbp_nav_offsety;
	process.mbp_nav_offsetz = mbp_nav_offsetz;

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_heading(int verbose, char *file, 
			int	mbp_heading_mode, 
			double	mbp_headingbias, 
			int *error)
{
	char	*function_name = "mb_pr_update_heading";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		fprintf(stderr,"dbg2       mbp_heading_mode:  %d\n",mbp_heading_mode);
		fprintf(stderr,"dbg2       mbp_headingbias:   %f\n",mbp_headingbias);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set heading values */
	process.mbp_heading_mode = mbp_heading_mode;
	process.mbp_headingbias = mbp_headingbias;

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_datacut(int verbose, char *file, 
			int	mbp_cut_num,
			int	*mbp_cut_kind,
			int	*mbp_cut_mode,
			double	*mbp_cut_min,
			double	*mbp_cut_max,
			int *error)
{
	char	*function_name = "mb_pr_update_datacut";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		fprintf(stderr,"dbg2       mbp_cut_num:       %d\n",mbp_cut_num);
		for (i=0;i<mbp_cut_num;i++)
			{
			fprintf(stderr,"dbg2       mbp_cut_kind[%d]:   %d\n",i,mbp_cut_kind[i]);
			fprintf(stderr,"dbg2       mbp_cut_mode[%d]:   %d\n",i,mbp_cut_mode[i]);
			fprintf(stderr,"dbg2       mbp_cut_min[%d]:    %f\n",i,mbp_cut_min[i]);
			fprintf(stderr,"dbg2       mbp_cut_max[%d]:    %f\n",i,mbp_cut_max[i]);
			}
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set datacut values */
	process.mbp_cut_num = mbp_cut_num;
	for (i=0;i<mbp_cut_num;i++)
		{
		process.mbp_cut_kind[i] = mbp_cut_kind[i];
		process.mbp_cut_mode[i] = mbp_cut_mode[i];
		process.mbp_cut_min[i] = mbp_cut_min[i];
		process.mbp_cut_max[i] = mbp_cut_max[i];
		}

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_edit(int verbose, char *file, 
			int	mbp_edit_mode, 
			char	*mbp_editfile, 
			int *error)
{
	char	*function_name = "mb_pr_update_edit";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;
	char	*lastslash;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		fprintf(stderr,"dbg2       mbp_edit_mode:     %d\n",mbp_edit_mode);
		fprintf(stderr,"dbg2       mbp_editfile:      %s\n",mbp_editfile);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set edit values */
	process.mbp_edit_mode = mbp_edit_mode;
	if (mbp_editfile != NULL)
		strcpy(process.mbp_editfile, mbp_editfile);

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_ampcorr(int verbose, char *file, 
			int	mbp_ampcorr_mode,
			char	*mbp_ampcorrfile,
			int	mbp_ampcorr_type,
			int	mbp_ampcorr_symmetry,
			double	mbp_ampcorr_angle,
			int	mbp_ampcorr_slope,
			int *error)
{
	char	*function_name = "mb_pr_update_ampcorr";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                  %d\n",verbose);
		fprintf(stderr,"dbg2       file:                     %s\n",file);
		fprintf(stderr,"dbg2       mbp_ampcorr_mode:          %d\n",mbp_ampcorr_mode);
		fprintf(stderr,"dbg2       mbp_ampcorrfile:           %s\n",mbp_ampcorrfile);
		fprintf(stderr,"dbg2       mbp_ampcorr_type:          %d\n",mbp_ampcorr_type);
		fprintf(stderr,"dbg2       mbp_ampcorr_symmetry:      %d\n",mbp_ampcorr_symmetry);
		fprintf(stderr,"dbg2       mbp_ampcorr_angle:         %f\n",mbp_ampcorr_angle);
		fprintf(stderr,"dbg2       mbp_ampcorr_slope:         %d\n",mbp_ampcorr_slope);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set ampcorr values */
	process.mbp_ampcorr_mode = mbp_ampcorr_mode;
	if (mbp_ampcorrfile != NULL)
	    strcpy(process.mbp_ampcorrfile, mbp_ampcorrfile);
	process.mbp_ampcorr_type = mbp_ampcorr_type;
	process.mbp_ampcorr_symmetry = mbp_ampcorr_symmetry;
	process.mbp_ampcorr_angle = mbp_ampcorr_angle;
	process.mbp_ampcorr_slope = mbp_ampcorr_slope;

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:                    %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                   %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_sscorr(int verbose, char *file, 
			int	mbp_sscorr_mode,
			char	*mbp_sscorrfile,
			int	mbp_sscorr_type,
			int	mbp_sscorr_symmetry,
			double	mbp_sscorr_angle,
			int	mbp_sscorr_slope,
			int *error)
{
	char	*function_name = "mb_pr_update_sscorr";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                  %d\n",verbose);
		fprintf(stderr,"dbg2       file:                     %s\n",file);
		fprintf(stderr,"dbg2       mbp_sscorr_mode:          %d\n",mbp_sscorr_mode);
		fprintf(stderr,"dbg2       mbp_sscorrfile:           %s\n",mbp_sscorrfile);
		fprintf(stderr,"dbg2       mbp_sscorr_type:          %d\n",mbp_sscorr_type);
		fprintf(stderr,"dbg2       mbp_sscorr_symmetry:      %d\n",mbp_sscorr_symmetry);
		fprintf(stderr,"dbg2       mbp_sscorr_angle:         %f\n",mbp_sscorr_angle);
		fprintf(stderr,"dbg2       mbp_sscorr_slope:         %d\n",mbp_sscorr_slope);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set sscorr values */
	process.mbp_sscorr_mode = mbp_sscorr_mode;
	if (mbp_sscorrfile != NULL)
	    strcpy(process.mbp_sscorrfile, mbp_sscorrfile);
	process.mbp_sscorr_type = mbp_sscorr_type;
	process.mbp_sscorr_symmetry = mbp_sscorr_symmetry;
	process.mbp_sscorr_angle = mbp_sscorr_angle;
	process.mbp_sscorr_slope = mbp_sscorr_slope;

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:                    %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                   %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_ssrecalc(int verbose, char *file, 
			int		mbp_ssrecalc_mode,
			double	mbp_ssrecalc_pixelsize,
			double	mbp_ssrecalc_swathwidth,
			int		mbp_ssrecalc_interpolate,
			int *error)
{
	char	*function_name = "mb_pr_update_ssrecalc";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                  %d\n",verbose);
		fprintf(stderr,"dbg2       file:                     %s\n",file);
		fprintf(stderr,"dbg2       mbp_ssrecalc_mode:        %d\n",mbp_ssrecalc_mode);
		fprintf(stderr,"dbg2       mbp_ssrecalc_pixelsize:   %f\n",mbp_ssrecalc_pixelsize);
		fprintf(stderr,"dbg2       mbp_ssrecalc_swathwidth:  %f\n",mbp_ssrecalc_swathwidth);
		fprintf(stderr,"dbg2       mbp_ssrecalc_interpolate: %d\n",mbp_ssrecalc_interpolate);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set ssrecalc values */
	process.mbp_ssrecalc_mode = mbp_ssrecalc_mode;
	process.mbp_ssrecalc_pixelsize = mbp_ssrecalc_pixelsize;
	process.mbp_ssrecalc_swathwidth = mbp_ssrecalc_swathwidth;
	process.mbp_ssrecalc_interpolate = mbp_ssrecalc_interpolate;

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:                    %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                   %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_metadata(int verbose, char *file, 
			char	*mbp_meta_vessel,
			char	*mbp_meta_institution,
			char	*mbp_meta_platform,
			char	*mbp_meta_sonar,
			char	*mbp_meta_sonarversion,
			char	*mbp_meta_cruiseid,
			char	*mbp_meta_cruisename,
			char	*mbp_meta_pi,
			char	*mbp_meta_piinstitution,
			char	*mbp_meta_client,
			int	mbp_meta_svcorrected,
			int	mbp_meta_tidecorrected,
			int	mbp_meta_batheditmanual,
			int	mbp_meta_batheditauto,
			double	mbp_meta_rollbias,
			double	mbp_meta_pitchbias,
			double	mbp_meta_headingbias,
			double	mbp_meta_draft,
			int *error)
{
	char	*function_name = "mb_pr_update_metadata";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                  %d\n",verbose);
		fprintf(stderr,"dbg2       file:                     %s\n",file);
		fprintf(stderr,"dbg2       mbp_meta_vessel:          %s\n",process.mbp_meta_vessel);
		fprintf(stderr,"dbg2       mbp_meta_institution:     %s\n",process.mbp_meta_institution);
		fprintf(stderr,"dbg2       mbp_meta_platform:        %s\n",process.mbp_meta_platform);
		fprintf(stderr,"dbg2       mbp_meta_sonar:           %s\n",process.mbp_meta_sonar);
		fprintf(stderr,"dbg2       mbp_meta_sonarversion:    %s\n",process.mbp_meta_sonarversion);
		fprintf(stderr,"dbg2       mbp_meta_cruiseid:        %s\n",process.mbp_meta_cruiseid);
		fprintf(stderr,"dbg2       mbp_meta_cruisename:      %s\n",process.mbp_meta_cruisename);
		fprintf(stderr,"dbg2       mbp_meta_p:i              %s\n",process.mbp_meta_pi);
		fprintf(stderr,"dbg2       mbp_meta_piinstitution:   %s\n",process.mbp_meta_piinstitution);
		fprintf(stderr,"dbg2       mbp_meta_client:          %s\n",process.mbp_meta_client);
		fprintf(stderr,"dbg2       mbp_meta_svcorrected:     %d\n",process.mbp_meta_svcorrected);
		fprintf(stderr,"dbg2       mbp_meta_tidecorrected    %d\n",process.mbp_meta_tidecorrected);
		fprintf(stderr,"dbg2       mbp_meta_batheditmanual   %d\n",process.mbp_meta_batheditmanual);
		fprintf(stderr,"dbg2       mbp_meta_batheditauto:    %d\n",process.mbp_meta_batheditauto);
		fprintf(stderr,"dbg2       mbp_meta_rollbias:        %f\n",process.mbp_meta_rollbias);
		fprintf(stderr,"dbg2       mbp_meta_pitchbias:       %f\n",process.mbp_meta_pitchbias);
		fprintf(stderr,"dbg2       mbp_meta_headingbias:     %f\n",process.mbp_meta_headingbias);
		fprintf(stderr,"dbg2       mbp_meta_draft:           %f\n",process.mbp_meta_draft);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set metadata values */
	strcpy(process.mbp_meta_vessel,mbp_meta_vessel);
	strcpy(process.mbp_meta_institution,mbp_meta_institution);
	strcpy(process.mbp_meta_platform,mbp_meta_platform);
	strcpy(process.mbp_meta_sonar,mbp_meta_sonar);
	strcpy(process.mbp_meta_sonarversion,mbp_meta_sonarversion);
	strcpy(process.mbp_meta_cruiseid,mbp_meta_cruiseid);
	strcpy(process.mbp_meta_cruisename,mbp_meta_cruisename);
	strcpy(process.mbp_meta_pi,mbp_meta_pi);
	strcpy(process.mbp_meta_piinstitution,mbp_meta_piinstitution);
	strcpy(process.mbp_meta_client,mbp_meta_client);
        process.mbp_meta_svcorrected = mbp_meta_svcorrected;
        process.mbp_meta_tidecorrected = mbp_meta_tidecorrected;
        process.mbp_meta_batheditmanual = mbp_meta_batheditmanual;
        process.mbp_meta_batheditauto = mbp_meta_batheditauto;
        process.mbp_meta_rollbias = mbp_meta_rollbias;
        process.mbp_meta_pitchbias = mbp_meta_pitchbias;
        process.mbp_meta_headingbias = mbp_meta_headingbias;
        process.mbp_meta_draft = mbp_meta_draft;
 
	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:                    %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                   %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_update_kluges(int verbose, char *file, 
			int	mbp_kluge001,
			int	mbp_kluge002,
			int	mbp_kluge003,
			int	mbp_kluge004,
			int	mbp_kluge005,
			int *error)
{
	char	*function_name = "mb_pr_update_kluges";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                  %d\n",verbose);
		fprintf(stderr,"dbg2       file:                     %s\n",file);
		fprintf(stderr,"dbg2       mbp_kluge001:             %d\n",mbp_kluge001);
		fprintf(stderr,"dbg2       mbp_kluge002:             %d\n",mbp_kluge002);
		fprintf(stderr,"dbg2       mbp_kluge003:             %d\n",mbp_kluge003);
		fprintf(stderr,"dbg2       mbp_kluge004:             %d\n",mbp_kluge004);
		fprintf(stderr,"dbg2       mbp_kluge005:             %d\n",mbp_kluge005);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set metadata values */
        process.mbp_kluge001 = mbp_kluge001;
        process.mbp_kluge002 = mbp_kluge002;
        process.mbp_kluge003 = mbp_kluge003;
        process.mbp_kluge004 = mbp_kluge004;
        process.mbp_kluge005 = mbp_kluge005;
 
	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:                    %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                   %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_ofile(int verbose, char *file, 
			int	*mbp_ofile_specified, 
			char	*mbp_ofile, 
			int	*error)
{
	char	*function_name = "mb_pr_get_ofile";
	int	status = MB_SUCCESS;
	char	parfile[MBP_FILENAMESIZE];
	char	buffer[MBP_FILENAMESIZE], dummy[MBP_FILENAMESIZE], *result;
	FILE	*fp;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:             %d\n",verbose);
		fprintf(stderr,"dbg2       file:                %s\n",file);
		}
		
	/* this function looks for the output filename directly
	 * rather than by calling mb_pr_readpar() in order to
	 * speed up mbgrid and other programs that parse large
	 * datalists looking for processed files
	 */

	/* get expected process parameter file name */
	strcpy(parfile, file);
	strcat(parfile, ".par");

	/* open and read parameter file */
	*mbp_ofile_specified = MB_NO;
	if (mbp_ofile != NULL)
	    mbp_ofile[0] = '\0';
	if ((fp = fopen(parfile, "r")) != NULL) 
	    {
	    while ((result = fgets(buffer,MBP_FILENAMESIZE,fp)) == buffer
		&& *mbp_ofile_specified == MB_NO)
		{
		if (strncmp(buffer, "OUTFILE", 7) == 0)
		    {
		    sscanf(buffer, "%s %s", dummy, mbp_ofile);
		    *mbp_ofile_specified = MB_YES;
		    }
		}
		
	    /* close file */
	    fclose(fp);
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_ofile_specified: %d\n",*mbp_ofile_specified);
		fprintf(stderr,"dbg2       ofile:               %s\n",mbp_ofile);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_format(int verbose, char *file, 
			int *mbp_format_specified, 
			int *mbp_format, 
			int *error)
{
	char	*function_name = "mb_pr_get_format";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:              %d\n",verbose);
		fprintf(stderr,"dbg2       file:                 %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set format value */
	*mbp_format_specified = process.mbp_format_specified;
	*mbp_format = process.mbp_format;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_format_specified: %d\n",*mbp_format_specified);
		fprintf(stderr,"dbg2       mbp_format:           %d\n",*mbp_format);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_rollbias(int verbose, char *file, 
			int	*mbp_rollbias_mode, 
			double	*mbp_rollbias, 
			double	*mbp_rollbias_port, 
			double	*mbp_rollbias_stbd, 
			int *error)
{
	char	*function_name = "mb_pr_get_rollbias";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set rollbias values */
	*mbp_rollbias_mode = process.mbp_rollbias_mode;
	*mbp_rollbias = process.mbp_rollbias;
	*mbp_rollbias_port = process.mbp_rollbias_port;
	*mbp_rollbias_stbd = process.mbp_rollbias_stbd;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_rollbias_mode: %d\n",*mbp_rollbias_mode);
		fprintf(stderr,"dbg2       mbp_rollbias:      %f\n",*mbp_rollbias);
		fprintf(stderr,"dbg2       mbp_rollbias_port: %f\n",*mbp_rollbias_port);
		fprintf(stderr,"dbg2       mbp_rollbias_stbd: %f\n",*mbp_rollbias_stbd);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_pitchbias(int verbose, char *file, 
			int	*mbp_pitchbias_mode, 
			double	*mbp_pitchbias, 
			int *error)
{
	char	*function_name = "mb_pr_get_pitchbias";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set pitchbias values */
	*mbp_pitchbias_mode = process.mbp_pitchbias_mode;
	*mbp_pitchbias = process.mbp_pitchbias;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_pitchbias_mode: %d\n",*mbp_pitchbias_mode);
		fprintf(stderr,"dbg2       mbp_pitchbias:      %f\n",*mbp_pitchbias);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_draft(int verbose, char *file, 
			int	*mbp_draft_mode, 
			double	*mbp_draft, 
			double	*mbp_draft_offset, 
			double	*mbp_draft_mult, 
			int *error)
{
	char	*function_name = "mb_pr_get_draft";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set draft values */
	*mbp_draft_mode = process.mbp_draft_mode;
	*mbp_draft = process.mbp_draft;
	*mbp_draft_offset = process.mbp_draft_offset;
	*mbp_draft_mult = process.mbp_draft_mult;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_draft_mode:    %d\n",*mbp_draft_mode);
		fprintf(stderr,"dbg2       mbp_draft:         %f\n",*mbp_draft);
		fprintf(stderr,"dbg2       mbp_draft_offset:  %f\n",*mbp_draft_offset);
		fprintf(stderr,"dbg2       mbp_draft_mult:    %f\n",*mbp_draft_mult);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_heave(int verbose, char *file, 
			int	*mbp_heave_mode, 
			double	*mbp_heave, 
			double	*mbp_heave_mult, 
			int *error)
{
	char	*function_name = "mb_pr_get_heave";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set heave values */
	*mbp_heave_mode = process.mbp_heave_mode;
	*mbp_heave = process.mbp_heave;
	*mbp_heave_mult = process.mbp_heave_mult;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_heave_mode:    %d\n",*mbp_heave_mode);
		fprintf(stderr,"dbg2       mbp_heave:         %f\n",*mbp_heave);
		fprintf(stderr,"dbg2       mbp_heave_mult:    %f\n",*mbp_heave_mult);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_lever(int verbose, char *file, 
			int	*mbp_lever_mode, 
			double	*mbp_vru_offsetx, 
			double	*mbp_vru_offsety, 
			double	*mbp_vru_offsetz, 
			double	*mbp_sonar_offsetx, 
			double	*mbp_sonar_offsety, 
			double	*mbp_sonar_offsetz, 
			int *error)
{
	char	*function_name = "mb_pr_get_lever";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set lever values */
	*mbp_lever_mode = process.mbp_lever_mode;
	*mbp_vru_offsetx = process.mbp_vru_offsetx;
	*mbp_vru_offsety = process.mbp_vru_offsety;
	*mbp_vru_offsetz = process.mbp_vru_offsetz;
	*mbp_sonar_offsetx = process.mbp_sonar_offsetx;
	*mbp_sonar_offsety = process.mbp_sonar_offsety;
	*mbp_sonar_offsetz = process.mbp_sonar_offsetz;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_lever_mode:    %d\n",*mbp_lever_mode);
		fprintf(stderr,"dbg2       mbp_vru_offsetx:   %f\n",*mbp_vru_offsetx);
		fprintf(stderr,"dbg2       mbp_vru_offsety:   %f\n",*mbp_vru_offsety);
		fprintf(stderr,"dbg2       mbp_vru_offsetz:   %f\n",*mbp_vru_offsetz);
		fprintf(stderr,"dbg2       mbp_sonar_offsetx:   %f\n",*mbp_sonar_offsetx);
		fprintf(stderr,"dbg2       mbp_sonar_offsety:   %f\n",*mbp_sonar_offsety);
		fprintf(stderr,"dbg2       mbp_sonar_offsetz:   %f\n",*mbp_sonar_offsetz);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_tide(int verbose, char *file, 
			int	*mbp_tide_mode, 
			char *mbp_tidefile, 
			int	*mbp_tide_format, 
			int *error)
{
	char	*function_name = "mb_pr_get_tide";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set lever values */
	*mbp_tide_mode = process.mbp_tide_mode;
	if (mbp_tidefile != NULL)
		strcpy(mbp_tidefile,process.mbp_tidefile);
	*mbp_tide_format = process.mbp_tide_format;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_tide_mode:     %d\n",*mbp_tide_mode);
		fprintf(stderr,"dbg2       mbp_tidefile:      %s\n",mbp_tidefile);
		fprintf(stderr,"dbg2       mbp_tide_format:   %d\n",*mbp_tide_format);
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_tt(int verbose, char *file, 
			int	*mbp_tt_mode, 
			double	*mbp_tt_mult, 
			int *error)
{
	char	*function_name = "mb_pr_get_tt";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set tt values */
	*mbp_tt_mode = process.mbp_tt_mode;
	*mbp_tt_mult = process.mbp_tt_mult;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_tt_mode:       %d\n",*mbp_tt_mode);
		fprintf(stderr,"dbg2       mbp_tt_mult:       %f\n",*mbp_tt_mult);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_ssv(int verbose, char *file, 
			int	*mbp_ssv_mode, 
			double	*mbp_ssv, 
			int *error)
{
	char	*function_name = "mb_pr_get_ssv";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set ssv values */
	*mbp_ssv_mode = process.mbp_ssv_mode;
	*mbp_ssv = process.mbp_ssv;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_ssv_mode:      %d\n",*mbp_ssv_mode);
		fprintf(stderr,"dbg2       mbp_ssv:           %f\n",*mbp_ssv);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_svp(int verbose, char *file, 
			int	*mbp_svp_mode, 
			char	*mbp_svpfile, 
			int	*mbp_angle_mode, 
			int	*mbp_corrected, 
			int *error)
{
	char	*function_name = "mb_pr_get_svp";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set svp values */
	*mbp_svp_mode = process.mbp_svp_mode;
	if (mbp_svpfile != NULL)
	    strcpy(mbp_svpfile, process.mbp_svpfile);
	*mbp_angle_mode = process.mbp_angle_mode;
	*mbp_corrected = process.mbp_corrected;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_svp_mode:      %d\n",*mbp_svp_mode);
		fprintf(stderr,"dbg2       mbp_svpfile:       %s\n",mbp_svpfile);
		fprintf(stderr,"dbg2       mbp_angle_mode:    %d\n",*mbp_angle_mode);
		fprintf(stderr,"dbg2       mbp_corrected:     %d\n",*mbp_corrected);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_static(int verbose, char *file, 
			int	*mbp_static_mode, 
			char	*mbp_staticfile, 
			int *error)
{
	char	*function_name = "mb_pr_get_static";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set svp values */
	*mbp_static_mode = process.mbp_static_mode;
	if (mbp_staticfile != NULL)
	    strcpy(mbp_staticfile, process.mbp_staticfile);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_static_mode:   %d\n",*mbp_static_mode);
		fprintf(stderr,"dbg2       mbp_staticfile:    %s\n",mbp_staticfile);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_navadj(int verbose, char *file, 
			int	*mbp_navadj_mode, 
			char	*mbp_navadjfile, 
			int	*mbp_navadj_algorithm, 
			int *error)
{
	char	*function_name = "mb_pr_get_navadj";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:              %d\n",verbose);
		fprintf(stderr,"dbg2       file:                 %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set navadj values */
	*mbp_navadj_mode = process.mbp_navadj_mode;
	if (mbp_navadjfile != NULL)
	    strcpy(mbp_navadjfile, process.mbp_navadjfile);
	*mbp_navadj_algorithm = process.mbp_navadj_algorithm;

	/* write new process parameter file */
	status = mb_pr_writepar(verbose, file, &process, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_navadj_mode:      %d\n",*mbp_navadj_mode);
		fprintf(stderr,"dbg2       mbp_navadjfile:       %s\n",mbp_navadjfile);
		fprintf(stderr,"dbg2       mbp_navadj_algorithm: %d\n",*mbp_navadj_algorithm);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_attitude(int verbose, char *file, 
			int	*mbp_attitude_mode, 
			char *mbp_attitudefile, 
			int	*mbp_attitude_format, 
			int *error)
{
	char	*function_name = "mb_pr_get_attitude";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set lever values */
	*mbp_attitude_mode = process.mbp_attitude_mode;
	if (mbp_attitudefile != NULL)
		strcpy(mbp_attitudefile,process.mbp_attitudefile);
	*mbp_attitude_format = process.mbp_attitude_format;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_attitude_mode: %d\n",*mbp_attitude_mode);
		fprintf(stderr,"dbg2       mbp_attitudefile:  %s\n",mbp_attitudefile);
		fprintf(stderr,"dbg2       mbp_attitude_format:%d\n",*mbp_attitude_format);
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_sonardepth(int verbose, char *file, 
			int	*mbp_sonardepth_mode, 
			char *mbp_sonardepthfile, 
			int	*mbp_sonardepth_format, 
			int *error)
{
	char	*function_name = "mb_pr_get_sonardepth";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set lever values */
	*mbp_sonardepth_mode = process.mbp_sonardepth_mode;
	if (mbp_sonardepthfile != NULL)
		strcpy(mbp_sonardepthfile,process.mbp_sonardepthfile);
	*mbp_sonardepth_format = process.mbp_sonardepth_format;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_sonardepth_mode:   %d\n",*mbp_sonardepth_mode);
		fprintf(stderr,"dbg2       mbp_sonardepthfile:    %s\n",mbp_sonardepthfile);
		fprintf(stderr,"dbg2       mbp_sonardepth_format: %d\n",*mbp_sonardepth_format);
		fprintf(stderr,"dbg2       error:                 %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_nav(int verbose, char *file, 
			int	*mbp_nav_mode, 
			char	*mbp_navfile, 
			int	*mbp_nav_format, 
			int	*mbp_nav_heading, 
			int	*mbp_nav_speed, 
			int	*mbp_nav_draft, 
			int	*mbp_nav_attitude, 
			int	*mbp_nav_algorithm, 
			double *mbp_nav_timeshift,
			int *error)
{
	char	*function_name = "mb_pr_get_nav";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

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

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_nav_mode:      %d\n",*mbp_nav_mode);
		fprintf(stderr,"dbg2       mbp_navfile:       %s\n",mbp_navfile);
		fprintf(stderr,"dbg2       mbp_nav_format:    %d\n",*mbp_nav_format);
		fprintf(stderr,"dbg2       mbp_nav_heading:   %d\n",*mbp_nav_heading);
		fprintf(stderr,"dbg2       mbp_nav_speed:     %d\n",*mbp_nav_speed);
		fprintf(stderr,"dbg2       mbp_nav_draft:     %d\n",*mbp_nav_draft);
		fprintf(stderr,"dbg2       mbp_nav_attitude:  %d\n",*mbp_nav_attitude);
		fprintf(stderr,"dbg2       mbp_nav_algorithm: %d\n",*mbp_nav_algorithm);
		fprintf(stderr,"dbg2       mbp_nav_timeshift: %f\n",*mbp_nav_timeshift);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_navshift(int verbose, char *file, 
			int	*mbp_nav_shift, 
			double	*mbp_nav_offsetx, 
			double	*mbp_nav_offsety, 
			double	*mbp_nav_offsetz, 
			int *error)
{
	char	*function_name = "mb_pr_get_navshift";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set nav values */
	*mbp_nav_shift = process.mbp_nav_shift;
	*mbp_nav_offsetx = process.mbp_nav_offsetx;
	*mbp_nav_offsety = process.mbp_nav_offsety;
	*mbp_nav_offsetz = process.mbp_nav_offsetz;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_nav_shift:     %d\n",*mbp_nav_shift);
		fprintf(stderr,"dbg2       mbp_nav_offsetx:   %f\n",*mbp_nav_offsetx);
		fprintf(stderr,"dbg2       mbp_nav_offsety:   %f\n",*mbp_nav_offsety);
		fprintf(stderr,"dbg2       mbp_nav_offsetz:   %f\n",*mbp_nav_offsetz);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}/*--------------------------------------------------------------------*/
int mb_pr_get_heading(int verbose, char *file, 
			int	*mbp_heading_mode, 
			double	*mbp_headingbias, 
			int *error)
{
	char	*function_name = "mb_pr_get_heading";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set heading values */
	*mbp_heading_mode = process.mbp_heading_mode;
	*mbp_headingbias = process.mbp_headingbias;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_heading_mode:  %d\n",*mbp_heading_mode);
		fprintf(stderr,"dbg2       mbp_headingbias:   %f\n",*mbp_headingbias);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_datacut(int verbose, char *file, 
			int	*mbp_cut_num,
			int	*mbp_cut_kind,
			int	*mbp_cut_mode,
			double	*mbp_cut_min,
			double	*mbp_cut_max,
			int *error)
{
	char	*function_name = "mb_pr_update_datacut";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set datacut values */
	*mbp_cut_num = process.mbp_cut_num;
	for (i=0;i<*mbp_cut_num;i++)
		{
		mbp_cut_kind[i] = process.mbp_cut_kind[i];
		mbp_cut_mode[i] = process.mbp_cut_mode[i];
		mbp_cut_min[i] = process.mbp_cut_min[i];
		mbp_cut_max[i] = process.mbp_cut_max[i];
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_cut_num:        %d\n",*mbp_cut_num);
		for (i=0;i<*mbp_cut_num;i++)
			{
			fprintf(stderr,"dbg2       mbp_cut_kind[%d]:   %d\n",i,mbp_cut_kind[i]);
			fprintf(stderr,"dbg2       mbp_cut_mode[%d]:   %d\n",i,mbp_cut_mode[i]);
			fprintf(stderr,"dbg2       mbp_cut_min[%d]:    %f\n",i,mbp_cut_min[i]);
			fprintf(stderr,"dbg2       mbp_cut_max[%d]:    %f\n",i,mbp_cut_max[i]);
			}
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:             %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_edit(int verbose, char *file, 
			int	*mbp_edit_mode, 
			char	*mbp_editfile, 
			int *error)
{
	char	*function_name = "mb_pr_get_edit";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set edit values */
	*mbp_edit_mode = process.mbp_edit_mode;
	if (mbp_editfile != NULL)
	    strcpy(mbp_editfile, process.mbp_editfile);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_edit_mode:     %d\n",*mbp_edit_mode);
		fprintf(stderr,"dbg2       mbp_editfile:      %s\n",mbp_editfile);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_ampcorr(int verbose, char *file, 
			int	*mbp_ampcorr_mode,
			char	*mbp_ampcorrfile,
			int	*mbp_ampcorr_type,
			int	*mbp_ampcorr_symmetry,
			double	*mbp_ampcorr_angle,
			int	*mbp_ampcorr_slope,
			int *error)
{
	char	*function_name = "mb_pr_get_ampcorr";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set ssrecalc values */
	*mbp_ampcorr_mode = process.mbp_ampcorr_mode;
	if (mbp_ampcorrfile != NULL)
	    strcpy(mbp_ampcorrfile, process.mbp_ampcorrfile);
	*mbp_ampcorr_type = process.mbp_ampcorr_type;
	*mbp_ampcorr_symmetry = process.mbp_ampcorr_symmetry;
	*mbp_ampcorr_angle = process.mbp_ampcorr_angle;
	*mbp_ampcorr_slope = process.mbp_ampcorr_slope;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_ampcorr_mode:         %d\n",*mbp_ampcorr_mode);
		fprintf(stderr,"dbg2       mbp_ampcorrfile:          %s\n",mbp_ampcorrfile);
		fprintf(stderr,"dbg2       mbp_ampcorr_type:         %d\n",*mbp_ampcorr_type);
		fprintf(stderr,"dbg2       mbp_ampcorr_symmetry:     %d\n",*mbp_ampcorr_symmetry);
		fprintf(stderr,"dbg2       mbp_ampcorr_angle:        %f\n",*mbp_ampcorr_angle);
		fprintf(stderr,"dbg2       mbp_ampcorr_slope:        %d\n",*mbp_ampcorr_slope);
		fprintf(stderr,"dbg2       error:                    %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                   %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_sscorr(int verbose, char *file, 
			int	*mbp_sscorr_mode,
			char	*mbp_sscorrfile,
			int	*mbp_sscorr_type,
			int	*mbp_sscorr_symmetry,
			double	*mbp_sscorr_angle,
			int	*mbp_sscorr_slope,
			int *error)
{
	char	*function_name = "mb_pr_get_sscorr";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set ssrecalc values */
	*mbp_sscorr_mode = process.mbp_sscorr_mode;
	if (mbp_sscorrfile != NULL)
	    strcpy(mbp_sscorrfile, process.mbp_sscorrfile);
	*mbp_sscorr_type = process.mbp_sscorr_type;
	*mbp_sscorr_symmetry = process.mbp_sscorr_symmetry;
	*mbp_sscorr_angle = process.mbp_sscorr_angle;
	*mbp_sscorr_slope = process.mbp_sscorr_slope;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_sscorr_mode:          %d\n",*mbp_sscorr_mode);
		fprintf(stderr,"dbg2       mbp_sscorrfile:           %s\n",mbp_sscorrfile);
		fprintf(stderr,"dbg2       mbp_sscorr_type:          %d\n",*mbp_sscorr_type);
		fprintf(stderr,"dbg2       mbp_sscorr_symmetry:      %d\n",*mbp_sscorr_symmetry);
		fprintf(stderr,"dbg2       mbp_sscorr_angle:         %f\n",*mbp_sscorr_angle);
		fprintf(stderr,"dbg2       mbp_sscorr_slope:         %d\n",*mbp_sscorr_slope);
		fprintf(stderr,"dbg2       error:                    %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                   %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_ssrecalc(int verbose, char *file, 
			int	*mbp_ssrecalc_mode,
			double	*mbp_ssrecalc_pixelsize,
			double	*mbp_ssrecalc_swathwidth,
			int	*mbp_ssrecalc_interpolate,
			int *error)
{
	char	*function_name = "mb_pr_get_ssrecalc";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set ssrecalc values */
	*mbp_ssrecalc_mode = process.mbp_ssrecalc_mode;
	*mbp_ssrecalc_pixelsize = process.mbp_ssrecalc_pixelsize;
	*mbp_ssrecalc_swathwidth = process.mbp_ssrecalc_swathwidth;
	*mbp_ssrecalc_interpolate = process.mbp_ssrecalc_interpolate;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_ssrecalc_mode:        %d\n",*mbp_ssrecalc_mode);
		fprintf(stderr,"dbg2       mbp_ssrecalc_pixelsize:   %f\n",*mbp_ssrecalc_pixelsize);
		fprintf(stderr,"dbg2       mbp_ssrecalc_swathwidth:  %f\n",*mbp_ssrecalc_swathwidth);
		fprintf(stderr,"dbg2       mbp_ssrecalc_interpolate: %d\n",*mbp_ssrecalc_interpolate);
		fprintf(stderr,"dbg2       error:                    %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                   %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_metadata(int verbose, char *file, 
			char	*mbp_meta_vessel,
			char	*mbp_meta_institution,
			char	*mbp_meta_platform,
			char	*mbp_meta_sonar,
			char	*mbp_meta_sonarversion,
			char	*mbp_meta_cruiseid,
			char	*mbp_meta_cruisename,
			char	*mbp_meta_pi,
			char	*mbp_meta_piinstitution,
			char	*mbp_meta_client,
			int	*mbp_meta_svcorrected,
			int	*mbp_meta_tidecorrected,
			int	*mbp_meta_batheditmanual,
			int	*mbp_meta_batheditauto,
			double	*mbp_meta_rollbias,
			double	*mbp_meta_pitchbias,
			double	*mbp_meta_headingbias,
			double	*mbp_meta_draft,
			int *error)
{
	char	*function_name = "mb_pr_get_metadata";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set metadata values */
	strcpy(mbp_meta_vessel,process.mbp_meta_vessel);
	strcpy(mbp_meta_institution,process.mbp_meta_institution);
	strcpy(mbp_meta_platform,process.mbp_meta_platform);
	strcpy(mbp_meta_sonar,process.mbp_meta_sonar);
	strcpy(mbp_meta_sonarversion,process.mbp_meta_sonarversion);
	strcpy(mbp_meta_cruiseid,process.mbp_meta_cruiseid);
	strcpy(mbp_meta_cruisename,process.mbp_meta_cruisename);
	strcpy(mbp_meta_pi,process.mbp_meta_pi);
	strcpy(mbp_meta_piinstitution,process.mbp_meta_piinstitution);
	strcpy(mbp_meta_client,process.mbp_meta_client);
        *mbp_meta_svcorrected = process.mbp_meta_svcorrected;
        *mbp_meta_tidecorrected = process.mbp_meta_tidecorrected;
        *mbp_meta_batheditmanual = process.mbp_meta_batheditmanual;
        *mbp_meta_batheditauto = process.mbp_meta_batheditauto;
        *mbp_meta_rollbias = process.mbp_meta_rollbias;
        *mbp_meta_pitchbias = process.mbp_meta_pitchbias;
        *mbp_meta_headingbias = process.mbp_meta_headingbias;
        *mbp_meta_draft = process.mbp_meta_draft;
 

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_meta_vessel:          %s\n",mbp_meta_vessel);
		fprintf(stderr,"dbg2       mbp_meta_institution:     %s\n",mbp_meta_institution);
		fprintf(stderr,"dbg2       mbp_meta_platform:        %s\n",mbp_meta_platform);
		fprintf(stderr,"dbg2       mbp_meta_sonar:           %s\n",mbp_meta_sonar);
		fprintf(stderr,"dbg2       mbp_meta_sonarversion:    %s\n",mbp_meta_sonarversion);
		fprintf(stderr,"dbg2       mbp_meta_cruiseid:        %s\n",mbp_meta_cruiseid);
		fprintf(stderr,"dbg2       mbp_meta_cruisename:      %s\n",mbp_meta_cruisename);
		fprintf(stderr,"dbg2       mbp_meta_p:i              %s\n",mbp_meta_pi);
		fprintf(stderr,"dbg2       mbp_meta_piinstitution:   %s\n",mbp_meta_piinstitution);
		fprintf(stderr,"dbg2       mbp_meta_client:          %s\n",mbp_meta_client);
		fprintf(stderr,"dbg2       mbp_meta_svcorrected:     %d\n",*mbp_meta_svcorrected);
		fprintf(stderr,"dbg2       mbp_meta_tidecorrected    %d\n",*mbp_meta_tidecorrected);
		fprintf(stderr,"dbg2       mbp_meta_batheditmanual   %d\n",*mbp_meta_batheditmanual);
		fprintf(stderr,"dbg2       mbp_meta_batheditauto:    %d\n",*mbp_meta_batheditauto);
		fprintf(stderr,"dbg2       mbp_meta_rollbias:        %f\n",*mbp_meta_rollbias);
		fprintf(stderr,"dbg2       mbp_meta_pitchbias:       %f\n",*mbp_meta_pitchbias);
		fprintf(stderr,"dbg2       mbp_meta_headingbias:     %f\n",*mbp_meta_headingbias);
		fprintf(stderr,"dbg2       mbp_meta_draft:           %f\n",*mbp_meta_draft);
		fprintf(stderr,"dbg2       error:                    %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                   %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_kluges(int verbose, char *file, 
			int	*mbp_kluge001,
			int	*mbp_kluge002,
			int	*mbp_kluge003,
			int	*mbp_kluge004,
			int	*mbp_kluge005,
			int *error)
{
	char	*function_name = "mb_pr_get_kluges";
	struct mb_process_struct process;
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set metadata values */
        *mbp_kluge001 = process.mbp_kluge001;
        *mbp_kluge002 = process.mbp_kluge002;
        *mbp_kluge003 = process.mbp_kluge003;
        *mbp_kluge004 = process.mbp_kluge004;
        *mbp_kluge005 = process.mbp_kluge005;
 

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_kluge001:             %d\n",mbp_kluge001);
		fprintf(stderr,"dbg2       mbp_kluge002:             %d\n",mbp_kluge002);
		fprintf(stderr,"dbg2       mbp_kluge003:             %d\n",mbp_kluge003);
		fprintf(stderr,"dbg2       mbp_kluge004:             %d\n",mbp_kluge004);
		fprintf(stderr,"dbg2       mbp_kluge005:             %d\n",mbp_kluge005);
		fprintf(stderr,"dbg2       error:                    %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                   %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_set_bathyslopenew(int verbose,
	int nsmooth, 
	int nbath, char *beamflag, double *bath, double *bathacrosstrack,
	int *ndepths, double *depths, double *depthacrosstrack, 
	int *nslopes, double *slopes, double *slopeacrosstrack, 
	double *depthsmooth, 
	int *error)
{
	char	*function_name = "mb_pr_set_bathyslopenew";
	int	status = MB_SUCCESS;
	int	first, next, last;
	double	dxtrack;
	double	weight;
	int	j1, j2;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       nsmooth:         %d\n",nsmooth);
		fprintf(stderr,"dbg2       nbath:           %d\n",nbath);
		fprintf(stderr,"dbg2       bath:            %d\n",bath);
		fprintf(stderr,"dbg2       bathacrosstrack: %d\n",
			bathacrosstrack);
		fprintf(stderr,"dbg2       bath:\n");
		for (i=0;i<nbath;i++)
			fprintf(stderr,"dbg2         %d  %d  %f %f\n", 
				i, beamflag[i], bath[i], bathacrosstrack[i]);
		fprintf(stderr,"dbg2       depths:           %d\n",depths);
		fprintf(stderr,"dbg2       depthacrosstrack: %d\n",depthacrosstrack);
		fprintf(stderr,"dbg2       slopes:           %d\n",slopes);
		fprintf(stderr,"dbg2       slopeacrosstrack: %d\n",slopeacrosstrack);
		}

	/* initialize depths */
	*ndepths = 0;
	for (i=0;i<nbath;i++)
		{
		depths[i] = 0.0;
		depthacrosstrack[i] = 0.0;
		}
		
	/* decimate by nsmooth, averaging the values used */
	for (i=0;i<=nbath/nsmooth;i++)
		{
		j1 = i * nsmooth;
		j2 = MIN((i + 1) * nsmooth, nbath);
		depths[*ndepths] = 0.0;
		depthacrosstrack[*ndepths] = 0.0;
		weight = 0.0;
		for (j=j1;j<j2;j++)
			{
			if (mb_beam_ok(beamflag[j]))
				{
				depths[*ndepths] += bath[j];
				depthacrosstrack[*ndepths] += bathacrosstrack[j];
				weight += 1.0;
				}
			}
		if (weight > 0.0)
			{
			depths[*ndepths] /= weight;
			depthacrosstrack[*ndepths] /= weight;
			(*ndepths) += 1;
			}
		}

	/* now calculate slopes */
	if (*ndepths > 0)
		{
		*nslopes = *ndepths + 1;
		slopeacrosstrack[0] = depthacrosstrack[0];
		slopes[0] = 0.0;
		for (i=1;i<*ndepths;i++)
			{
			dxtrack = depthacrosstrack[i] - depthacrosstrack[i-1];
			slopeacrosstrack[i] = depthacrosstrack[i-1] + 0.5 * dxtrack;
			if (dxtrack > 0.0)
				slopes[i] = (depths[i] - depths[i-1])
					/ dxtrack;
			else 
				slopes[i] = 0.0;
/*fprintf(stderr,"SLOPECALC: i:%d depths: %f %f  xtrack: %f %f  slope:%f\n",
i,depths[i-1],depths[i],depthacrosstrack[i-1],depthacrosstrack[i],slopes[i]);*/
			}
		slopeacrosstrack[*ndepths] = depthacrosstrack[*ndepths-1];
		slopes[*ndepths] = 0.0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ndepths:         %d\n",
			*ndepths);
		fprintf(stderr,"dbg2       depths:\n");
		for (i=0;i<*ndepths;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, depths[i], depthacrosstrack[i]);
		fprintf(stderr,"dbg2       nslopes:         %d\n",
			*nslopes);
		fprintf(stderr,"dbg2       slopes:\n");
		for (i=0;i<*nslopes;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, slopes[i], slopeacrosstrack[i]);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_set_bathyslope(int verbose,
	int nsmooth, 
	int nbath, char *beamflag, double *bath, double *bathacrosstrack,
	int *ndepths, double *depths, double *depthacrosstrack, 
	int *nslopes, double *slopes, double *slopeacrosstrack, 
	double *depthsmooth, 
	int *error)
{
	char	*function_name = "mb_pr_set_bathyslope";
	int	status = MB_SUCCESS;
	int	first, next, last;
	int	nbathgood;
	double	depthsum;
	double	dacrosstrack;
	double	factor;
	int	j1, j2;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       nbath:           %d\n",nbath);
		fprintf(stderr,"dbg2       bath:            %d\n",bath);
		fprintf(stderr,"dbg2       bathacrosstrack: %d\n",
			bathacrosstrack);
		fprintf(stderr,"dbg2       bath:\n");
		for (i=0;i<nbath;i++)
			fprintf(stderr,"dbg2         %d  %d  %f %f\n", 
				i, beamflag[i], bath[i], bathacrosstrack[i]);
		fprintf(stderr,"dbg2       depths:           %d\n",depths);
		fprintf(stderr,"dbg2       depthacrosstrack: %d\n",depthacrosstrack);
		fprintf(stderr,"dbg2       slopes:           %d\n",slopes);
		fprintf(stderr,"dbg2       slopeacrosstrack: %d\n",slopeacrosstrack);
		}

	/* initialize depths */
	*ndepths = 0;
	for (i=0;i<nbath;i++)
		{
		depths[i] = 0.0;
		depthacrosstrack[i] = 0.0;
		}

	/* first fill in the existing depths */
	first = -1;
	last = -1;
	nbathgood = 0;
	for (i=0;i<nbath;i++)
		{
		if (mb_beam_ok(beamflag[i]))
			{
			if (first == -1)
				{
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
	for (i=first;i<last;i++)
		{
		if (mb_beam_ok(beamflag[i]))
			{
			next = i;
			j = i + 1;
			while (next == i && j < nbath)
				{
				if (mb_beam_ok(beamflag[j]))
					next = j;
				else
					j++;
				}
			if (next > i)
				{
				for (j=i+1;j<next;j++)
					{
					factor = ((double)(j - i))
							/ ((double)(next - i));
					depths[j] = bath[i] + 
						factor * (bath[next] - bath[i]);
					depthacrosstrack[j] = bathacrosstrack[i] + 
						factor * (bathacrosstrack[next] - bathacrosstrack[i]);
					}
				}
			}
		}

	/* now smooth the depths */
	if (nbathgood > 0 && nsmooth > 0)
		{
		for (i=first;i<=last;i++)
			{
			j1 = i - nsmooth;
			j2 = i + nsmooth;
			if (j1 < first)
				j1 = first;
			if (j2 > last)
				j2 = last;
			depthsum = 0.0;
			for (j=j1;j<=j2;j++)
				{
				depthsum += depths[j];
				}
			if (depthsum > 0.0)
				depthsmooth[i] = depthsum/((double)(j2-j1+1));
			else
				depthsmooth[i] = depths[i];
			}
		for (i=first;i<=last;i++)
			depths[i] = depthsmooth[i];
		}

	/* now extrapolate the depths at the ends of the swath */
	if (nbathgood > 0)
		{
		*ndepths = nbath;
		if (last - first > 0)
			dacrosstrack = 
				(depthacrosstrack[last] 
				- depthacrosstrack[first]) 
				/ (last - first);
		else 
			dacrosstrack = 1.0;
		for (i=0;i<first;i++)
			{
			depths[i] = depths[first];
			depthacrosstrack[i] = depthacrosstrack[first] 
				+ dacrosstrack * (i - first);
			}
		for (i=last+1;i<nbath;i++)
			{
			depths[i] = depths[last];
			depthacrosstrack[i] = depthacrosstrack[last] 
				+ dacrosstrack * (i - last);
			}
		}

	/* now calculate slopes */
	if (nbathgood > 0)
		{
		*nslopes = nbath + 1;
		for (i=0;i<nbath-1;i++)
			{
			slopes[i+1] = (depths[i+1] - depths[i])
				/(depthacrosstrack[i+1] - depthacrosstrack[i]);
			slopeacrosstrack[i+1] = 0.5*(depthacrosstrack[i+1] 
				+ depthacrosstrack[i]);
/*fprintf(stderr,"SLOPECALC: i:%d depths: %f %f  xtrack: %f %f  slope:%f\n",
i,depths[i],depths[i+1],depthacrosstrack[i],depthacrosstrack[i+1],slopes[i+1]);*/
			}
		slopes[0] = 0.0;
		slopeacrosstrack[0] = depthacrosstrack[0];
		slopes[nbath] = 0.0;
		slopeacrosstrack[nbath] = depthacrosstrack[nbath-1];
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ndepths:         %d\n",
			*ndepths);
		fprintf(stderr,"dbg2       depths:\n");
		for (i=0;i<nbath;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, depths[i], depthacrosstrack[i]);
		fprintf(stderr,"dbg2       nslopes:         %d\n",
			*nslopes);
		fprintf(stderr,"dbg2       slopes:\n");
		for (i=0;i<*nslopes;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, slopes[i], slopeacrosstrack[i]);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pr_get_bathyslope(int verbose,
	int ndepths, double *depths, double *depthacrosstrack,
	int nslopes, double *slopes, double *slopeacrosstrack, 
	double acrosstrack, double *depth, double *slope,
	int *error)
{
	char	*function_name = "get_bathyslope";
	int	status = MB_SUCCESS;
	int	found_depth, found_slope;
	int	idepth, islope;
	int	i;
	
	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       ndepths:         %d\n",
			ndepths);
		fprintf(stderr,"dbg2       depths:\n");
		for (i=0;i<ndepths;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, depths[i], depthacrosstrack[i]);
		fprintf(stderr,"dbg2       nslopes:         %d\n",
			nslopes);
		fprintf(stderr,"dbg2       slopes:\n");
		for (i=0;i<nslopes;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, slopes[i], slopeacrosstrack[i]);
		fprintf(stderr,"dbg2       acrosstrack:     %f\n",acrosstrack);
		}

	/* check if acrosstrack is in defined interval */
	found_depth = MB_NO;
	found_slope = MB_NO;
	if (ndepths > 1)
	    {
	    
	    if (acrosstrack < depthacrosstrack[0])
		{
		*depth = depths[0];
		*slope = 0.0;
		found_depth = MB_YES;
		found_slope = MB_YES;
		}

	    else if (acrosstrack > depthacrosstrack[ndepths-1])
		{
		*depth = depths[ndepths-1];
		*slope = 0.0;
		found_depth = MB_YES;
		found_slope = MB_YES;
		}
    
	    else if (acrosstrack >= depthacrosstrack[0]
		    && acrosstrack <= depthacrosstrack[ndepths-1])
		{
    
		/* look for depth */
		idepth = -1;
		while (found_depth == MB_NO && idepth < ndepths - 2)
		    {
		    idepth++;
		    if (acrosstrack >= depthacrosstrack[idepth]
			&& acrosstrack <= depthacrosstrack[idepth+1])
			{
			*depth = depths[idepth] 
				+ (acrosstrack - depthacrosstrack[idepth])
				/(depthacrosstrack[idepth+1] 
				- depthacrosstrack[idepth])
				*(depths[idepth+1] - depths[idepth]);
			found_depth = MB_YES;
			*error = MB_ERROR_NO_ERROR;
			}
		    }
    
		/* look for slope */
		islope = -1;
		while (found_slope == MB_NO && islope < nslopes - 2)
		    {
		    islope++;
		    if (acrosstrack >= slopeacrosstrack[islope]
			&& acrosstrack <= slopeacrosstrack[islope+1])
			{
			*slope = slopes[islope] 
				+ (acrosstrack - slopeacrosstrack[islope])
				/(slopeacrosstrack[islope+1] 
				- slopeacrosstrack[islope])
				*(slopes[islope+1] - slopes[islope]);
			found_slope = MB_YES;
			*error = MB_ERROR_NO_ERROR;
			}
		    }
		}
	    }

	/* check for failure */
	if (found_depth != MB_YES || found_slope != MB_YES)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_OTHER;
		*depth = 0.0;
		*slope = 0.0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       depth:           %f\n",*depth);
		fprintf(stderr,"dbg2       slope:           %f\n",*slope);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/


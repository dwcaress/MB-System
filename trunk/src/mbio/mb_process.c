/*--------------------------------------------------------------------
 *    The MB-system:	mb_process.c	9/11/00
 *    $Id: mb_process.c,v 5.3 2001-06-01 00:14:06 caress Exp $
 *
 *    Copyright (c) 2000 by
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

static char rcs_id[]="$Id: mb_process.c,v 5.3 2001-06-01 00:14:06 caress Exp $";

/*--------------------------------------------------------------------*/
int mb_pr_readpar(int verbose, char *file, int lookforfiles, 
			struct mb_process_struct *process, 
			int *error)
{
	char	*function_name = "mb_pr_readpar";
	char	parfile[MBP_FILENAMESIZE], fileroot[MBP_FILENAMESIZE];
	char	buffer[MBP_FILENAMESIZE], dummy[MBP_FILENAMESIZE], *result;
	int	format;
	FILE	*fp;
	int	stat_status;
	struct stat statbuf;
	int	status = MB_SUCCESS;
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
	process->mbp_nav_algorithm = MBP_NAV_LINEAR;
	process->mbp_nav_timeshift = 0.0;
	process->mbp_nav_shift = MBP_NAV_OFF;
	process->mbp_nav_offsetx = 0.0;
	process->mbp_nav_offsety = 0.0;
	
	/* adjusted navigation merging */
	process->mbp_navadj_mode = MBP_NAV_OFF;
	process->mbp_navadjfile[0] = '\0';
	process->mbp_navadj_algorithm = MBP_NAV_LINEAR;
	
	/* bathymetry editing */
	process->mbp_edit_mode = MBP_EDIT_OFF;
	process->mbp_editfile[0] = '\0';
	
	/* bathymetry recalculation */
	process->mbp_bathrecalc_mode = MBP_BATHRECALC_OFF;
	process->mbp_svp_mode = MBP_SVP_OFF;
	process->mbp_svpfile[0] = '\0';
	process->mbp_ssv_mode = MBP_SSV_OFF;
	process->mbp_ssv = 0.0;
	process->mbp_corrected = MB_YES;
	process->mbp_tt_mode = MBP_TT_OFF;
	process->mbp_tt_mult = 1.0;
	process->mbp_angle_mode = MBP_ANGLES_OK;
	
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
	
	/* sidescan recalculation */
	process->mbp_ssrecalc_mode = MBP_SSRECALC_OFF;
	process->mbp_ssrecalc_pixelsize = 0.0;
	process->mbp_ssrecalc_swathwidth = 0.0;
	process->mbp_ssrecalc_interpolate = 0;

	/* metadata insertion */
	process->mbp_meta_operator[0] = '\0';
	process->mbp_meta_platform[0] = '\0';
	process->mbp_meta_sonar[0] = '\0';
	process->mbp_meta_survey[0] = '\0';
	process->mbp_meta_pi[0] = '\0';
	process->mbp_meta_client[0] = '\0';

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
		    else if (strncmp(buffer, "TIDEFILE", 10) == 0)
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
		    else if (strncmp(buffer, "METAOPERATOR", 12) == 0)
			{
			strncpy(process->mbp_meta_operator, &buffer[13], MBP_FILENAMESIZE);
			}
		    else if (strncmp(buffer, "METAPLATFORM", 12) == 0)
			{
			strncpy(process->mbp_meta_platform, &buffer[13], MBP_FILENAMESIZE);
			}
		    else if (strncmp(buffer, "METASONAR", 9) == 0)
			{
			strncpy(process->mbp_meta_sonar, &buffer[10], MBP_FILENAMESIZE);
			}
		    else if (strncmp(buffer, "METASURVEY", 10) == 0)
			{
			strncpy(process->mbp_meta_survey, &buffer[11], MBP_FILENAMESIZE);
			}
		    else if (strncmp(buffer, "METAPI", 6) == 0)
			{
			strncpy(process->mbp_meta_pi, &buffer[7], MBP_FILENAMESIZE);
			}
		    else if (strncmp(buffer, "METACLIENT", 10) == 0)
			{
			strncpy(process->mbp_meta_client, &buffer[11], MBP_FILENAMESIZE);
			}
	
		    /* processing kluges */
		    else if (strncmp(buffer, "KLUGE001", 8) == 0)
			{
			}
			
		    }
		}
		
	    /* close file */
	    fclose(fp);
	    
	    }
	    
	/* reset input file */
	strcpy(process->mbp_ifile, file);
	process->mbp_ifile_specified = MB_YES;
	    
	/* figure out data format or output filename if required */
	if (process->mbp_format_specified == MB_NO
	    || process->mbp_ofile_specified == MB_NO)
	    {
	    /* get format if possible */
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
				fileroot, process->mbp_format);
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
	    }
	    
	/* update bathymetry recalculation mode */
	mb_pr_bathmode(verbose, process, error);
	    
	/* look for nav and other bath edit files if not specified */
	if (lookforfiles == MB_YES)
	    {
	    /* look for nav file */
	    if (process->mbp_nav_mode == MBP_NAV_OFF)
		{
		for (i = 9; i >= 0 && process->mbp_nav_mode == MBP_NAV_OFF; i--)
		    {
		    sprintf(process->mbp_navfile, "%s.na%d", process->mbp_ifile, i);
		    if (stat(process->mbp_navfile, &statbuf) == 0)
			    {
			    process->mbp_nav_mode = MBP_NAV_ON;
			    process->mbp_nav_format = 9;
			    }
		    }
		}
	    if (process->mbp_nav_mode == MBP_NAV_OFF)
 		{
		strcpy(process->mbp_navfile, process->mbp_ifile);
		strcat(process->mbp_navfile, ".nve");
		if (stat(process->mbp_navfile, &statbuf) == 0)
		    {
		    process->mbp_nav_mode = MBP_NAV_ON;
		    process->mbp_nav_format = 9;
		    }
		}

	    /* look for edit file */
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
		}
	    }
	    
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
		fprintf(stderr,"dbg2       mbp_nav_algorithm:      %d\n",process->mbp_nav_algorithm);
		fprintf(stderr,"dbg2       mbp_nav_timeshift:      %f\n",process->mbp_nav_timeshift);
		fprintf(stderr,"dbg2       mbp_nav_shift:          %d\n",process->mbp_nav_shift);
		fprintf(stderr,"dbg2       mbp_nav_offsetx:        %f\n",process->mbp_nav_offsetx);
		fprintf(stderr,"dbg2       mbp_nav_offsety:        %f\n",process->mbp_nav_offsety);
		fprintf(stderr,"dbg2       mbp_nav_offsetz:        %f\n",process->mbp_nav_offsetz);
		fprintf(stderr,"dbg2       mbp_navadj_mode:        %d\n",process->mbp_navadj_mode);
		fprintf(stderr,"dbg2       mbp_navadjfile:         %s\n",process->mbp_navadjfile);
		fprintf(stderr,"dbg2       mbp_navadj_algorithm:   %d\n",process->mbp_navadj_algorithm);
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
		fprintf(stderr,"dbg2       mbp_heading_mode:       %d\n",process->mbp_heading_mode);
		fprintf(stderr,"dbg2       mbp_headingbias:        %f\n",process->mbp_headingbias);
		fprintf(stderr,"dbg2       mbp_edit_mode:          %d\n",process->mbp_edit_mode);
		fprintf(stderr,"dbg2       mbp_editfile:           %s\n",process->mbp_editfile);
		fprintf(stderr,"dbg2       mbp_tide_mode:          %d\n",process->mbp_tide_mode);
		fprintf(stderr,"dbg2       mbp_tidefile:           %s\n",process->mbp_tidefile);
		fprintf(stderr,"dbg2       mbp_tide_format:        %d\n",process->mbp_tide_format);
		fprintf(stderr,"dbg2       mbp_ssrecalc_mode:      %d\n",process->mbp_ssrecalc_mode);
		fprintf(stderr,"dbg2       mbp_ssrecalc_pixelsize: %f\n",process->mbp_ssrecalc_pixelsize);
		fprintf(stderr,"dbg2       mbp_ssrecalc_swathwidth:%f\n",process->mbp_ssrecalc_swathwidth);
		fprintf(stderr,"dbg2       mbp_ssrecalc_interp    :%d\n",process->mbp_ssrecalc_interpolate);
		fprintf(stderr,"dbg2       mbp_meta_operator      :%s\n",process->mbp_meta_operator);
		fprintf(stderr,"dbg2       mbp_meta_platform      :%s\n",process->mbp_meta_platform);
		fprintf(stderr,"dbg2       mbp_meta_sonar         :%s\n",process->mbp_meta_sonar);
		fprintf(stderr,"dbg2       mbp_meta_survey        :%s\n",process->mbp_meta_survey);
		fprintf(stderr,"dbg2       mbp_meta_pi            :%s\n",process->mbp_meta_pi);
		fprintf(stderr,"dbg2       mbp_meta_client        :%s\n",process->mbp_meta_client);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
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
		fprintf(stderr,"dbg2       mbp_nav_algorithm:      %d\n",process->mbp_nav_algorithm);
		fprintf(stderr,"dbg2       mbp_nav_timeshift:      %f\n",process->mbp_nav_timeshift);
		fprintf(stderr,"dbg2       mbp_nav_shift:          %d\n",process->mbp_nav_shift);
		fprintf(stderr,"dbg2       mbp_nav_offsetx:        %f\n",process->mbp_nav_offsetx);
		fprintf(stderr,"dbg2       mbp_nav_offsety:        %f\n",process->mbp_nav_offsety);
		fprintf(stderr,"dbg2       mbp_nav_offsetz:        %f\n",process->mbp_nav_offsetz);
		fprintf(stderr,"dbg2       mbp_navadj_mode:        %d\n",process->mbp_navadj_mode);
		fprintf(stderr,"dbg2       mbp_navadjfile:         %s\n",process->mbp_navadjfile);
		fprintf(stderr,"dbg2       mbp_navadj_algorithm:   %d\n",process->mbp_navadj_algorithm);
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
		fprintf(stderr,"dbg2       mbp_heading_mode:       %d\n",process->mbp_heading_mode);
		fprintf(stderr,"dbg2       mbp_headingbias:        %f\n",process->mbp_headingbias);
		fprintf(stderr,"dbg2       mbp_edit_mode:          %d\n",process->mbp_edit_mode);
		fprintf(stderr,"dbg2       mbp_editfile:           %s\n",process->mbp_editfile);
		fprintf(stderr,"dbg2       mbp_tide_mode:          %d\n",process->mbp_tide_mode);
		fprintf(stderr,"dbg2       mbp_tidefile:           %s\n",process->mbp_tidefile);
		fprintf(stderr,"dbg2       mbp_tide_format:        %d\n",process->mbp_tide_format);
		fprintf(stderr,"dbg2       mbp_ssrecalc_mode:      %d\n",process->mbp_ssrecalc_mode);
		fprintf(stderr,"dbg2       mbp_ssrecalc_pixelsize: %f\n",process->mbp_ssrecalc_pixelsize);
		fprintf(stderr,"dbg2       mbp_ssrecalc_swathwidth:%f\n",process->mbp_ssrecalc_swathwidth);
		fprintf(stderr,"dbg2       mbp_ssrecalc_interp    :%d\n",process->mbp_ssrecalc_interpolate);
		fprintf(stderr,"dbg2       mbp_meta_operator      :%s\n",process->mbp_meta_operator);
		fprintf(stderr,"dbg2       mbp_meta_platform      :%s\n",process->mbp_meta_platform);
		fprintf(stderr,"dbg2       mbp_meta_sonar         :%s\n",process->mbp_meta_sonar);
		fprintf(stderr,"dbg2       mbp_meta_survey        :%s\n",process->mbp_meta_survey);
		fprintf(stderr,"dbg2       mbp_meta_pi            :%s\n",process->mbp_meta_pi);
		fprintf(stderr,"dbg2       mbp_meta_client        :%s\n",process->mbp_meta_client);
		}

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
		fprintf(fp, "INFILE %s\n", process->mbp_ifile);
		}
	    else
		{
		fprintf(fp, "## INFILE infile\n");
		}
	    if (process->mbp_ofile_specified == MB_YES)
		{
		fprintf(fp, "OUTFILE %s\n", process->mbp_ofile);
		}
	    else
		{
		fprintf(fp, "## OUTFILE outfile\n");
		}
	    
	    /* navigation merging */
	    fprintf(fp, "##\n## Navigation Merging:\n");
	    fprintf(fp, "NAVMODE %d\n", process->mbp_nav_mode);
	    fprintf(fp, "NAVFILE %s\n", process->mbp_navfile);
	    fprintf(fp, "NAVFORMAT %d\n", process->mbp_nav_format);
	    fprintf(fp, "NAVHEADING %d\n", process->mbp_nav_heading);
	    fprintf(fp, "NAVSPEED %d\n", process->mbp_nav_speed);
	    fprintf(fp, "NAVDRAFT %d\n", process->mbp_nav_draft);
	    fprintf(fp, "NAVINTERP %d\n", process->mbp_nav_algorithm);
	    fprintf(fp, "NAVTIMESHIFT %f\n", process->mbp_nav_timeshift);
	    fprintf(fp, "NAVSHIFT %d\n", process->mbp_nav_shift);
	    fprintf(fp, "NAVOFFSETX %f\n", process->mbp_nav_offsetx);
	    fprintf(fp, "NAVOFFSETY %f\n", process->mbp_nav_offsety);
	    fprintf(fp, "NAVOFFSETZ %f\n", process->mbp_nav_offsetz);
	    
	    /* adjusted navigation merging */
	    fprintf(fp, "##\n## Adjusted Navigation Merging:\n");
	    fprintf(fp, "NAVADJMODE %d\n", process->mbp_navadj_mode);
	    fprintf(fp, "NAVADJFILE %s\n", process->mbp_navadjfile);
	    fprintf(fp, "NAVADJINTERP %d\n", process->mbp_navadj_algorithm);
	    
	    /* bathymetry editing */
	    fprintf(fp, "##\n## Bathymetry Flagging:\n");
	    fprintf(fp, "EDITSAVEMODE %d\n", process->mbp_edit_mode);
	    fprintf(fp, "EDITSAVEFILE %s\n", process->mbp_editfile);
	    
	    /* bathymetry recalculation */
	    fprintf(fp, "##\n## Bathymetry Recalculation:\n");
	    fprintf(fp, "RAYTRACE %d\n", process->mbp_svp_mode);
	    fprintf(fp, "SVPFILE %s\n", process->mbp_svpfile);
	    fprintf(fp, "SSVMODE %d\n", process->mbp_ssv_mode);
	    fprintf(fp, "SSV %f\n", process->mbp_ssv);
	    fprintf(fp, "TTMODE %d\n", process->mbp_tt_mode);
	    fprintf(fp, "TTMULTIPLY %f\n", process->mbp_tt_mult);
	    fprintf(fp, "ANGLEMODE %d\n", process->mbp_angle_mode);
	    fprintf(fp, "CORRECTED %d\n", process->mbp_corrected);
	    
	    /* draft correction */
	    fprintf(fp, "##\n## Draft Correction:\n");
	    fprintf(fp, "DRAFTMODE %d\n", process->mbp_draft_mode);
	    fprintf(fp, "DRAFT %f\n", process->mbp_draft);
	    fprintf(fp, "DRAFTOFFSET %f\n", process->mbp_draft_offset);
	    fprintf(fp, "DRAFTMULTIPLY %f\n", process->mbp_draft_mult);
	    
	    /* heave correction */
	    fprintf(fp, "##\n## Heave Correction:\n");
	    fprintf(fp, "HEAVEMODE %f\n", process->mbp_heave_mode);
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
	    fprintf(fp, "TIDEFILE %s\n", process->mbp_tidefile);
	    fprintf(fp, "TIDEFORMAT %d\n", process->mbp_tide_format);
	    
	    /* sidescan recalculation */
	    fprintf(fp, "##\n## Sidescan Recalculation:\n");
	    fprintf(fp, "SSRECALCMODE %d\n", process->mbp_ssrecalc_mode);
	    fprintf(fp, "SSPIXELSIZE %f\n", process->mbp_ssrecalc_pixelsize);
	    fprintf(fp, "SSSWATHWIDTH %f\n", process->mbp_ssrecalc_swathwidth);
	    fprintf(fp, "SSINTERPOLATE %f\n", process->mbp_ssrecalc_interpolate);
	    
	    /* metadata insertion */
	    fprintf(fp, "##\n## Metadata Insertion:\n");
	    fprintf(fp, "METAOPERATOR %s\n", process->mbp_meta_operator);
	    fprintf(fp, "METAPLATFORM %s\n", process->mbp_meta_platform);
	    fprintf(fp, "METASONAR %s\n", process->mbp_meta_sonar);
	    fprintf(fp, "METASURVEY %s\n", process->mbp_meta_survey);
	    fprintf(fp, "METAPI %s\n", process->mbp_meta_pi);
	    fprintf(fp, "METACLIENT %s\n", process->mbp_meta_client);
  	
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
	else if (process->mbp_svp_mode == MBP_SVP_OFF
	    && (process->mbp_rollbias_mode != MBP_ROLLBIAS_OFF
		|| process->mbp_pitchbias_mode != MBP_PITCHBIAS_OFF))
	    process->mbp_bathrecalc_mode = MBP_BATHRECALC_ROTATE;
	else if (process->mbp_svp_mode == MBP_SVP_OFF
	    && process->mbp_rollbias_mode == MBP_ROLLBIAS_OFF
		&& (process->mbp_draft_mode != MBP_DRAFT_OFF
		    || process->mbp_lever_mode != MBP_DRAFT_OFF))
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
		fprintf(stderr,"dbg2       mbp_ofile_specified: %s\n",mbp_ofile_specified);
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
int mb_pr_update_nav(int verbose, char *file, 
			int	mbp_nav_mode, 
			char	*mbp_navfile, 
			int	mbp_nav_format, 
			int	mbp_nav_heading, 
			int	mbp_nav_speed, 
			int	mbp_nav_draft, 
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
		fprintf(stderr,"dbg2       mbp_nav_algorithm: %d\n",mbp_nav_algorithm);
		fprintf(stderr,"dbg2       mbp_nav_timeshift: %d\n",mbp_nav_timeshift);
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
int mb_pr_update_edit(int verbose, char *file, 
			int	mbp_edit_mode, 
			char	*mbp_editfile, 
			int *error)
{
	char	*function_name = "mb_pr_update_edit";
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
			char	*mbp_meta_operator,
			char	*mbp_meta_platform,
			char	*mbp_meta_sonar,
			char	*mbp_meta_survey,
			char	*mbp_meta_pi,
			char	*mbp_meta_client,
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
		fprintf(stderr,"dbg2       mbp_meta_operator:        %s\n",mbp_meta_operator);
		fprintf(stderr,"dbg2       mbp_meta_platform:        %s\n",mbp_meta_platform);
		fprintf(stderr,"dbg2       mbp_meta_sonar:           %s\n",mbp_meta_sonar);
		fprintf(stderr,"dbg2       mbp_meta_survey:          %s\n",mbp_meta_survey);
		fprintf(stderr,"dbg2       mbp_meta_pi:              %s\n",mbp_meta_pi);
		fprintf(stderr,"dbg2       mbp_meta_client:          %s\n",mbp_meta_client);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set metadata values */
	strcpy(process.mbp_meta_operator,mbp_meta_operator);
	strcpy(process.mbp_meta_platform,mbp_meta_platform);
	strcpy(process.mbp_meta_sonar,mbp_meta_sonar);
	strcpy(process.mbp_meta_survey,mbp_meta_survey);
	strcpy(process.mbp_meta_pi,mbp_meta_pi);
	strcpy(process.mbp_meta_client,mbp_meta_client);
 
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
	struct mb_process_struct process;
	int	status = MB_SUCCESS;
	char	parfile[MBP_FILENAMESIZE], fileroot[MBP_FILENAMESIZE];
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
		fprintf(stderr,"dbg2       mbp_ofile_specified: %s\n",*mbp_ofile_specified);
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
		fprintf(stderr,"dbg2       mbp_tidefile:      %s\n",*mbp_tidefile);
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
int mb_pr_get_nav(int verbose, char *file, 
			int	*mbp_nav_mode, 
			char	*mbp_navfile, 
			int	*mbp_nav_format, 
			int	*mbp_nav_heading, 
			int	*mbp_nav_speed, 
			int	*mbp_nav_draft, 
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
		fprintf(stderr,"dbg2       mbp_nav_algorithm: %d\n",*mbp_nav_algorithm);
		fprintf(stderr,"dbg2       mbp_nav_timeshift: %d\n",*mbp_nav_timeshift);
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
			char	*mbp_meta_operator,
			char	*mbp_meta_platform,
			char	*mbp_meta_sonar,
			char	*mbp_meta_survey,
			char	*mbp_meta_pi,
			char	*mbp_meta_client,
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
	strcpy(mbp_meta_operator,process.mbp_meta_operator);
	strcpy(mbp_meta_platform,process.mbp_meta_platform);
	strcpy(mbp_meta_sonar,process.mbp_meta_sonar);
	strcpy(mbp_meta_survey,process.mbp_meta_survey);
	strcpy(mbp_meta_pi,process.mbp_meta_pi);
	strcpy(mbp_meta_client,process.mbp_meta_client);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       mbp_meta_operator:        %s\n",process.mbp_meta_operator);
		fprintf(stderr,"dbg2       mbp_meta_platform:        %s\n",process.mbp_meta_platform);
		fprintf(stderr,"dbg2       mbp_meta_sonar:           %s\n",process.mbp_meta_sonar);
		fprintf(stderr,"dbg2       mbp_meta_survey:          %s\n",process.mbp_meta_survey);
		fprintf(stderr,"dbg2       mbp_meta_pi:              %s\n",process.mbp_meta_pi);
		fprintf(stderr,"dbg2       mbp_meta_client:          %s\n",process.mbp_meta_client);
		fprintf(stderr,"dbg2       error:                    %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                   %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/


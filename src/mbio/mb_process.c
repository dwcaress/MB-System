/*--------------------------------------------------------------------
 *    The MB-system:	mb_process.c	9/11/00
 *    $Id: mb_process.c,v 4.2 2000-10-11 01:02:30 caress Exp $
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
 * in mb_process.h
 *
 * Author:	D. W. Caress
 * Date:	September 11, 2000
 * 
 * $Log: not supported by cvs2svn $
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
#include "../../include/mb_process.h"

static char rcs_id[]="$Id: mb_process.c,v 4.2 2000-10-11 01:02:30 caress Exp $";

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
	process->mbp_ifile_specified = MB_NO;
	process->mbp_ifile[0] = '\0';
	process->mbp_ofile_specified = MB_NO;
	process->mbp_ofile[0] = '\0';
	process->mbp_format_specified = MB_NO;
	process->mbp_format = 0;
	process->mbp_bathrecalc_mode = MBP_BATHRECALC_OFF;
	process->mbp_rollbias_mode = MBP_ROLLBIAS_OFF;
	process->mbp_rollbias = 0.0;
	process->mbp_rollbias_port = 0.0;
	process->mbp_rollbias_stbd = 0.0;
	process->mbp_pitchbias_mode = MBP_PITCHBIAS_OFF;
	process->mbp_pitchbias = 0.0;
	process->mbp_draft_mode = MBP_DRAFT_OFF;
	process->mbp_draft = 0.0;
	process->mbp_draft_mult = 0.0;
	process->mbp_dfile[0] = '\0';
	process->mbp_ssv_mode = MBP_SSV_OFF;
	process->mbp_ssv = 0.0;
	process->mbp_svp_mode = MBP_SVP_OFF;
	process->mbp_svpfile[0] = '\0';
	process->mbp_uncorrected = MB_NO;
	process->mbp_navadj_mode = MBP_NAV_OFF;
	process->mbp_navadjfile[0] = '\0';
	process->mbp_navadj_algorithm = MBP_NAV_LINEAR;
	process->mbp_nav_mode = MBP_NAV_OFF;
	process->mbp_navfile[0] = '\0';
	process->mbp_nav_format = 0;
	process->mbp_nav_heading = MBP_NAV_OFF;
	process->mbp_nav_speed = MBP_NAV_OFF;
	process->mbp_nav_draft = MBP_NAV_OFF;
	process->mbp_nav_algorithm = MBP_NAV_LINEAR;
	process->mbp_heading_mode = MBP_HEADING_OFF;
	process->mbp_headingbias = 0.0;
	process->mbp_edit_mode = MBP_EDIT_OFF;
	process->mbp_editfile[0] = '\0';
	process->mbp_mask_mode = MBP_MASK_OFF;
	process->mbp_maskfile[0] = '\0';

	/* open and read parameter file */
	if ((fp = fopen(parfile, "r")) != NULL) 
	    {
	    while ((result = fgets(buffer,MBP_FILENAMESIZE,fp)) == buffer)
		{
		if (buffer[0] != '#')
		    {
		    if (strncmp(buffer, "INFILE", 6) == 0
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
		    else if (strncmp(buffer, "DRAFTOFFSET", 11) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_draft);
			if (process->mbp_draft_mode == MBP_DRAFT_OFF)
				process->mbp_draft_mode = MBP_DRAFT_OFFSET;
			else if (process->mbp_draft_mode == MBP_DRAFT_MULTIPLY)
				process->mbp_draft_mode = MBP_DRAFT_MULTIPLYOFFSET;
			}
		    else if (strncmp(buffer, "DRAFTMULTIPLY", 13) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_draft_mult);
			if (process->mbp_draft_mode == MBP_DRAFT_OFF)
				process->mbp_draft_mode = MBP_DRAFT_MULTIPLY;
			else if (process->mbp_draft_mode == MBP_DRAFT_OFFSET)
				process->mbp_draft_mode = MBP_DRAFT_MULTIPLYOFFSET;
			}
		    else if (strncmp(buffer, "DRAFT", 5) == 0
			&& process->mbp_draft_mode == MBP_DRAFT_OFF)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_draft);
			process->mbp_draft_mode = MBP_DRAFT_SET;
			}
		    else if (strncmp(buffer, "ROLLBIASPORT", 12) == 0
			&& (process->mbp_rollbias_mode == MBP_ROLLBIAS_OFF
			    || process->mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE))
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_rollbias_port);
			process->mbp_rollbias_mode = MBP_ROLLBIAS_DOUBLE;
			}
		    else if (strncmp(buffer, "ROLLBIASSTBD", 12) == 0
			&& (process->mbp_rollbias_mode == MBP_ROLLBIAS_OFF
			    || process->mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE))
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_rollbias_stbd);
			process->mbp_rollbias_mode = MBP_ROLLBIAS_DOUBLE;
			}
		    else if (strncmp(buffer, "ROLLBIAS", 8) == 0
			&& process->mbp_rollbias_mode == MBP_ROLLBIAS_OFF)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_rollbias);
			process->mbp_rollbias_mode = MBP_ROLLBIAS_SINGLE;
			}
		    else if (strncmp(buffer, "PITCHBIAS", 9) == 0
			&& process->mbp_pitchbias_mode == MBP_PITCHBIAS_OFF)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_pitchbias);
			process->mbp_pitchbias_mode = MBP_PITCHBIAS_ON;
			}
		    else if (strncmp(buffer, "NAVADJFILE", 10) == 0
			&& process->mbp_navadj_mode == MBP_NAV_OFF)
			{
			sscanf(buffer, "%s %s", dummy, process->mbp_navadjfile);
			process->mbp_navadj_mode = MBP_NAV_ON;
			}
		    else if (strncmp(buffer, "NAVADJSPLINE", 12) == 0)
			{
			process->mbp_navadj_algorithm = MBP_NAV_SPLINE;
			}
		    else if (strncmp(buffer, "NAVFILE", 7) == 0
			&& process->mbp_nav_mode == MBP_NAV_OFF)
			{
			sscanf(buffer, "%s %s", dummy, process->mbp_navfile);
			process->mbp_nav_mode = MBP_NAV_ON;
			}
		    else if (strncmp(buffer, "NAVFORMAT", 9) == 0)
			{
			sscanf(buffer, "%s %d", dummy, &process->mbp_nav_format);
			}
		    else if (strncmp(buffer, "NAVHEADING", 10) == 0)
			{
			process->mbp_nav_heading = MBP_NAV_ON;
			}
		    else if (strncmp(buffer, "NAVSPEED", 8) == 0)
			{
			process->mbp_nav_speed = MBP_NAV_ON;
			}
		    else if (strncmp(buffer, "NAVDRAFT", 8) == 0)
			{
			process->mbp_nav_draft = MBP_NAV_ON;
			}
		    else if (strncmp(buffer, "NAVSPLINE", 9) == 0)
			{
			process->mbp_nav_algorithm = MBP_NAV_SPLINE;
			}
		    else if (strncmp(buffer, "HEADINGOFFSET", 13) == 0)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_headingbias);
			process->mbp_heading_mode = MBP_HEADING_OFFSET;
			}
		    else if (strncmp(buffer, "HEADING", 7) == 0)
			{
			process->mbp_heading_mode = MBP_HEADING_CALC;
			}
		    else if (strncmp(buffer, "SSVOFFSET", 11) == 0
			&& process->mbp_ssv_mode == MBP_SSV_OFF)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_ssv);
			process->mbp_ssv_mode = MBP_SSV_OFFSET;
			}
		    else if (strncmp(buffer, "SSV", 5) == 0
			&& process->mbp_ssv_mode == MBP_SSV_OFF)
			{
			sscanf(buffer, "%s %lf", dummy, &process->mbp_ssv);
			process->mbp_ssv_mode = MBP_SSV_SET;
			}
		    else if (strncmp(buffer, "SVP", 3) == 0
			&& process->mbp_svp_mode == MBP_SVP_OFF)
			{
			sscanf(buffer, "%s %s", dummy, process->mbp_svpfile);
			process->mbp_svp_mode = MBP_SVP_ON;
			}
		    else if (strncmp(buffer, "UNCORRECTED", 3) == 0
			&& process->mbp_uncorrected == MB_NO)
			{
			process->mbp_uncorrected = MB_YES;
			}
		    else if (strncmp(buffer, "EDITSAVEFILE", 12) == 0
			&& process->mbp_edit_mode == MBP_EDIT_OFF)
			{
			sscanf(buffer, "%s %s", dummy, process->mbp_editfile);
			process->mbp_edit_mode = MBP_EDIT_ON;
			}
		    else if (strncmp(buffer, "EDITMASKFILE", 12) == 0
			&& process->mbp_mask_mode == MBP_MASK_OFF)
			{
			sscanf(buffer, "%s %s", dummy, process->mbp_maskfile);
			process->mbp_mask_mode = MBP_MASK_ON;
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
	    status = mb_get_format(verbose, process->mbp_ifile, 
				    fileroot, &format, error);
	    if (status == MB_SUCCESS && format > 0)
		{
		if (process->mbp_format_specified == MB_NO)
		    {
		    process->mbp_format = format;
		    process->mbp_format_specified = MB_YES;
		    }
		if (process->mbp_ofile_specified == MB_NO
		    && process->mbp_format_specified == MB_YES)
		    {
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
		fprintf(stderr,"dbg2       mbp_bathrecalc_mode:    %d\n",process->mbp_bathrecalc_mode);
		fprintf(stderr,"dbg2       mbp_rollbias_mode:      %d\n",process->mbp_rollbias_mode);
		fprintf(stderr,"dbg2       mbp_rollbias:           %f\n",process->mbp_rollbias);
		fprintf(stderr,"dbg2       mbp_rollbias_port:      %f\n",process->mbp_rollbias_port);
		fprintf(stderr,"dbg2       mbp_rollbias_stbd:      %f\n",process->mbp_rollbias_stbd);
		fprintf(stderr,"dbg2       mbp_pitchbias_mode:     %d\n",process->mbp_pitchbias_mode);
		fprintf(stderr,"dbg2       mbp_pitchbias:          %f\n",process->mbp_pitchbias);
		fprintf(stderr,"dbg2       mbp_draft_mode:         %d\n",process->mbp_draft_mode);
		fprintf(stderr,"dbg2       mbp_draft:              %f\n",process->mbp_draft);
		fprintf(stderr,"dbg2       mbp_draft_mult:         %f\n",process->mbp_draft_mult);
		fprintf(stderr,"dbg2       mbp_dfile:              %s\n",process->mbp_dfile);
		fprintf(stderr,"dbg2       mbp_ssv_mode:           %d\n",process->mbp_ssv_mode);
		fprintf(stderr,"dbg2       mbp_ssv:                %f\n",process->mbp_ssv);
		fprintf(stderr,"dbg2       mbp_svp_mode:           %d\n",process->mbp_svp_mode);
		fprintf(stderr,"dbg2       mbp_svpfile:            %s\n",process->mbp_svpfile);
		fprintf(stderr,"dbg2       mbp_uncorrected:        %d\n",process->mbp_uncorrected);
		fprintf(stderr,"dbg2       mbp_navadj_mode:        %d\n",process->mbp_nav_mode);
		fprintf(stderr,"dbg2       mbp_navadjfile:         %s\n",process->mbp_navadjfile);
		fprintf(stderr,"dbg2       mbp_navadj_algorithm:   %d\n",process->mbp_navadj_algorithm);
		fprintf(stderr,"dbg2       mbp_nav_mode:           %d\n",process->mbp_navadj_mode);
		fprintf(stderr,"dbg2       mbp_navfile:            %s\n",process->mbp_navfile);
		fprintf(stderr,"dbg2       mbp_nav_format:         %d\n",process->mbp_nav_format);
		fprintf(stderr,"dbg2       mbp_nav_heading:        %d\n",process->mbp_nav_heading);
		fprintf(stderr,"dbg2       mbp_nav_speed:          %d\n",process->mbp_nav_speed);
		fprintf(stderr,"dbg2       mbp_nav_draft:          %d\n",process->mbp_nav_draft);
		fprintf(stderr,"dbg2       mbp_nav_algorithm:      %d\n",process->mbp_nav_algorithm);
		fprintf(stderr,"dbg2       mbp_heading_mode:       %d\n",process->mbp_heading_mode);
		fprintf(stderr,"dbg2       mbp_headingbias:        %f\n",process->mbp_headingbias);
		fprintf(stderr,"dbg2       mbp_edit_mode:          %d\n",process->mbp_edit_mode);
		fprintf(stderr,"dbg2       mbp_editfile:           %s\n",process->mbp_editfile);
		fprintf(stderr,"dbg2       mbp_mask_mode:          %d\n",process->mbp_mask_mode);
		fprintf(stderr,"dbg2       mbp_maskfile:           %s\n",process->mbp_maskfile);
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
		fprintf(stderr,"dbg2       mbp_bathrecalc_mode:    %d\n",process->mbp_bathrecalc_mode);
		fprintf(stderr,"dbg2       mbp_rollbias_mode:      %d\n",process->mbp_rollbias_mode);
		fprintf(stderr,"dbg2       mbp_rollbias:           %f\n",process->mbp_rollbias);
		fprintf(stderr,"dbg2       mbp_rollbias_port:      %f\n",process->mbp_rollbias_port);
		fprintf(stderr,"dbg2       mbp_rollbias_stbd:      %f\n",process->mbp_rollbias_stbd);
		fprintf(stderr,"dbg2       mbp_pitchbias_mode:     %d\n",process->mbp_pitchbias_mode);
		fprintf(stderr,"dbg2       mbp_pitchbias:          %f\n",process->mbp_pitchbias);
		fprintf(stderr,"dbg2       mbp_draft_mode:         %d\n",process->mbp_draft_mode);
		fprintf(stderr,"dbg2       mbp_draft:              %f\n",process->mbp_draft);
		fprintf(stderr,"dbg2       mbp_draft_mult:         %f\n",process->mbp_draft_mult);
		fprintf(stderr,"dbg2       mbp_dfile:              %s\n",process->mbp_dfile);
		fprintf(stderr,"dbg2       mbp_ssv_mode:           %d\n",process->mbp_ssv_mode);
		fprintf(stderr,"dbg2       mbp_ssv:                %f\n",process->mbp_ssv);
		fprintf(stderr,"dbg2       mbp_svp_mode:           %d\n",process->mbp_svp_mode);
		fprintf(stderr,"dbg2       mbp_svpfile:            %s\n",process->mbp_svpfile);
		fprintf(stderr,"dbg2       mbp_uncorrected:        %d\n",process->mbp_uncorrected);
		fprintf(stderr,"dbg2       mbp_navadj_mode:        %d\n",process->mbp_nav_mode);
		fprintf(stderr,"dbg2       mbp_navadjfile:         %s\n",process->mbp_navadjfile);
		fprintf(stderr,"dbg2       mbp_navadj_algorithm:   %d\n",process->mbp_navadj_algorithm);
		fprintf(stderr,"dbg2       mbp_nav_mode:           %d\n",process->mbp_nav_mode);
		fprintf(stderr,"dbg2       mbp_navfile:            %s\n",process->mbp_navfile);
		fprintf(stderr,"dbg2       mbp_nav_format:         %d\n",process->mbp_nav_format);
		fprintf(stderr,"dbg2       mbp_nav_heading:        %d\n",process->mbp_nav_heading);
		fprintf(stderr,"dbg2       mbp_nav_speed:          %d\n",process->mbp_nav_speed);
		fprintf(stderr,"dbg2       mbp_nav_draft:          %d\n",process->mbp_nav_draft);
		fprintf(stderr,"dbg2       mbp_nav_algorithm:      %d\n",process->mbp_nav_algorithm);
		fprintf(stderr,"dbg2       mbp_heading_mode:       %d\n",process->mbp_heading_mode);
		fprintf(stderr,"dbg2       mbp_headingbias:        %f\n",process->mbp_headingbias);
		fprintf(stderr,"dbg2       mbp_edit_mode:          %d\n",process->mbp_edit_mode);
		fprintf(stderr,"dbg2       mbp_editfile:           %s\n",process->mbp_editfile);
		fprintf(stderr,"dbg2       mbp_mask_mode:          %d\n",process->mbp_mask_mode);
		fprintf(stderr,"dbg2       mbp_maskfile:           %s\n",process->mbp_maskfile);
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
	    if ((user_ptr = getenv("USER")) == NULL)
		    user_ptr = getenv("LOGNAME");
	    if (user_ptr != NULL)
		    strcpy(user,user_ptr);
	    else
		    strcpy(user, "unknown");
	    gethostname(host,MBP_FILENAMESIZE);
	    fprintf(fp,"## Generated by user <%s> on cpu <%s> at <%s>\n##\n",
		    user,host,date);



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

	    if (process->mbp_format_specified == MB_YES)
		{
		fprintf(fp, "FORMAT %d\n", process->mbp_format);
		}
	    else
		{
		fprintf(fp, "## FORMAT format\n");
		}

	    if (process->mbp_draft_mode == MBP_DRAFT_SET)
		{
		fprintf(fp, "DRAFT %f\n", process->mbp_draft);
		}
	    else
		{
		fprintf(fp, "## DRAFT draft\n");
		}
	    if (process->mbp_draft_mode == MBP_DRAFT_OFFSET
		|| process->mbp_draft_mode == MBP_DRAFT_MULTIPLYOFFSET)
		{
		fprintf(fp, "DRAFTOFFSET %f\n", process->mbp_draft);
		}
	    else
		{
		fprintf(fp, "## DRAFTOFFSET offset\n");
		}

	    if (process->mbp_draft_mode == MBP_DRAFT_MULTIPLY
		|| process->mbp_draft_mode == MBP_DRAFT_MULTIPLYOFFSET)
		{
		fprintf(fp, "DRAFTMULTIPLY %f\n", process->mbp_draft_mult);
		}
	    else
		{
		fprintf(fp, "## DRAFTMULTIPLY multiplier\n");
		}

	    if (process->mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE)
		{
		fprintf(fp, "ROLLBIASPORT %f\n", process->mbp_rollbias_port);
		fprintf(fp, "ROLLBIASSTBD %f\n", process->mbp_rollbias_stbd);
		}
	    else
		{
		fprintf(fp, "## ROLLBIASPORT bias\n");
		fprintf(fp, "## ROLLBIASSTBD bias\n");
		}

	    if (process->mbp_rollbias_mode == MBP_ROLLBIAS_SINGLE)
		{
		fprintf(fp, "ROLLBIAS %f\n", process->mbp_rollbias);
		}
	    else
		{
		fprintf(fp, "## ROLLBIAS bias\n");
		}

	    if (process->mbp_pitchbias_mode == MBP_PITCHBIAS_ON)
		{
		fprintf(fp, "PITCHBIAS %f\n", process->mbp_pitchbias);
		}
	    else
		{
		fprintf(fp, "## PITCHBIAS bias\n");
		}

	    if (process->mbp_navadj_mode == MBP_NAV_ON)
		{
		fprintf(fp, "NAVADJFILE %s\n", process->mbp_navadjfile);
		}
	    else
		{
		fprintf(fp, "## NAVADJFILE navadjfile\n");
		}

	    if (process->mbp_navadj_algorithm == MBP_NAV_SPLINE)
		{
		fprintf(fp, "NAVADJSPLINE\n");
		}
	    else
		{
		fprintf(fp, "## NAVADJSPLINE\n");
		}

	    if (process->mbp_nav_mode == MBP_NAV_ON)
		{
		fprintf(fp, "NAVFILE %s\n", process->mbp_navfile);
		fprintf(fp, "NAVFORMAT %d\n", process->mbp_nav_format);
		}
	    else
		{
		fprintf(fp, "## NAVFILE navfile\n");
		fprintf(fp, "## NAVFORMAT format\n");
		}

	    if (process->mbp_nav_heading == MBP_NAV_ON)
		{
		fprintf(fp, "NAVHEADING\n");
		}
	    else
		{
		fprintf(fp, "## NAVHEADING\n");
		}

	    if (process->mbp_nav_speed == MBP_NAV_ON)
		{
		fprintf(fp, "NAVSPEED\n");
		}
	    else
		{
		fprintf(fp, "## NAVSPEED\n");
		}

	    if (process->mbp_nav_draft == MBP_NAV_ON)
		{
		fprintf(fp, "NAVDRAFT\n");
		}
	    else
		{
		fprintf(fp, "## NAVDRAFT\n");
		}

	    if (process->mbp_nav_algorithm == MBP_NAV_SPLINE)
		{
		fprintf(fp, "NAVSPLINE\n");
		}
	    else
		{
		fprintf(fp, "## NAVSPLINE\n");
		}

	    if (process->mbp_heading_mode == MBP_HEADING_CALC)
		{
		fprintf(fp, "HEADING\n");
		}
	    else
		{
		fprintf(fp, "## HEADING\n");
		}

	    if (process->mbp_heading_mode == MBP_HEADING_OFFSET)
		{
		fprintf(fp, "HEADINGOFFSET %f\n", process->mbp_headingbias);
		}
	    else
		{
		fprintf(fp, "## HEADINGOFFSET offset\n");
		}

	    if (process->mbp_ssv_mode == MBP_SSV_OFFSET)
		{
		fprintf(fp, "SSVOFFSET %f\n", process->mbp_ssv);
		}
	    else
		{
		fprintf(fp, "## SSVOFFSET offset\n");
		}

	    if (process->mbp_ssv_mode == MBP_SSV_SET)
		{
		fprintf(fp, "SSV %f\n", process->mbp_ssv);
		}
	    else
		{
		fprintf(fp, "## SSV ssv\n");
		}

	    if (process->mbp_svp_mode == MBP_SVP_ON)
		{
		fprintf(fp, "SVP %s\n", process->mbp_svpfile);
		}
	    else
		{
		fprintf(fp, "## SVP svpfile\n");
		}

	    if (process->mbp_uncorrected == MB_YES)
		{
		fprintf(fp, "UNCORRECTED\n");
		}
	    else
		{
		fprintf(fp, "## UNCORRECTED\n");
		}

	    if (process->mbp_edit_mode == MBP_EDIT_ON)
		{
		fprintf(fp, "EDITSAVEFILE %s\n", process->mbp_editfile);
		}
	    else
		{
		fprintf(fp, "## EDITSAVEFILE editsavefile\n");
		}

	    if (process->mbp_mask_mode == MBP_MASK_ON)
		{
		fprintf(fp, "EDITMASKFILE %s\n", process->mbp_maskfile);
		}
	
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
		&& process->mbp_draft_mode != MBP_DRAFT_OFF)
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
			double	mbp_draft_mult, 
			char	*mbp_dfile, 
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
		fprintf(stderr,"dbg2       mbp_draft_mult:    %f\n",mbp_draft_mult);
		fprintf(stderr,"dbg2       mbp_dfile:         %s\n",mbp_dfile);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set draft values */
	process.mbp_draft_mode = mbp_draft_mode;
	process.mbp_draft = mbp_draft;
	process.mbp_draft_mult = mbp_draft_mult;
	if (mbp_dfile != NULL)
	    strcpy(process.mbp_dfile, mbp_dfile);
	    
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

	/* set draft values */
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
			int	mbp_uncorrected, 
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
		fprintf(stderr,"dbg2       mbp_uncorrected:   %d\n",mbp_uncorrected);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set draft values */
	process.mbp_svp_mode = mbp_svp_mode;
	if (mbp_svpfile != NULL)
	    strcpy(process.mbp_svpfile, mbp_svpfile);
	process.mbp_uncorrected = mbp_uncorrected;
	    
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

	/* set draft values */
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
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set draft values */
	process.mbp_nav_mode = mbp_nav_mode;
	if (mbp_navfile != NULL)
	    strcpy(process.mbp_navfile, mbp_navfile);
	process.mbp_nav_format = mbp_nav_format;
	process.mbp_nav_heading = mbp_nav_heading;
	process.mbp_nav_speed = mbp_nav_speed;
	process.mbp_nav_draft = mbp_nav_draft;
	process.mbp_nav_algorithm = mbp_nav_algorithm;

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

	/* set draft values */
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
		fprintf(stderr,"dbg2       mbp_edit_mode:     %d\n",mbp_edit_mode);
		fprintf(stderr,"dbg2       mbp_editfile:      %s\n",mbp_editfile);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set draft values */
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
int mb_pr_update_mask(int verbose, char *file, 
			int	mbp_mask_mode, 
			char	*mbp_maskfile, 
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
		fprintf(stderr,"dbg2       mbp_mask_mode:     %d\n",mbp_mask_mode);
		fprintf(stderr,"dbg2       mbp_maskfile:      %s\n",mbp_maskfile);
		}

	/* get known process parameters */
	status = mb_pr_readpar(verbose, file, MB_YES, &process, error);

	/* set draft values */
	process.mbp_mask_mode = mbp_mask_mode;
	if (mbp_maskfile != NULL)
	    strcpy(process.mbp_maskfile, mbp_maskfile);

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

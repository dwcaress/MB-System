/*--------------------------------------------------------------------
 *    The MB-system:	mbset.c	3/31/93
 *    $Id: mbset.c,v 5.2 2001-06-03 07:07:34 caress Exp $
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
 * MBset is a tool for setting values in an mbprocess parameter file.
 * MBprocess is a tool for processing swath sonar bathymetry data  
 * which performs a number of functions, including:
 *   - merging navigation
 *   - recalculating bathymetry from travel time and angle data
 *     by raytracing through a layered water sound velocity model.
 *   - applying changes to ship draft, roll bias and pitch bias
 *   - applying bathymetry edits from edit save files.
 * The parameters controlling mbprocess are included in an ascii
 * parameter file. The parameter file syntax is documented by
 * comments in the source file mbsystem/src/mbio/mb_process.h
 * and the manual pages for mbprocess and mbset. 
 *
 * Author:	D. W. Caress
 * Date:	January 4, 2000
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.1  2001/03/22 21:15:49  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.0  2001/01/22  07:55:22  caress
 * Version 5.0.beta01
 *
 * Revision 1.1  2001/01/22  07:54:22  caress
 * Initial revision
 *
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

/* mbio include files */
#include "../../include/mb_format.h"
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"
#include "../../include/mb_process.h"
#include "../../include/mb_swap.h"

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	/* id variables */
	static char rcs_id[] = "$Id: mbset.c,v 5.2 2001-06-03 07:07:34 caress Exp $";
	static char program_name[] = "mbset";
	static char help_message[] = "MBset is a tool for setting values in an mbprocess parameter file.\n\
MBprocess is a tool for processing swath sonar bathymetry data  \n\
which performs a number of functions, including:\n\
  - merging navigation\n\
  - recalculating bathymetry from travel time and angle data\n\
    by raytracing through a layered water sound velocity model.\n\
  - applying changes to ship draft, roll bias and pitch bias\n\
  - applying bathymetry edits from  edit save files.\n\
The parameters controlling mbprocess are included in an ascii\n\
parameter file. The parameter file syntax is documented by\n\
the manual pages for mbprocess and mbset. \n\n";
	static char usage_message[] = "mbset -Iinfile -PPARAMETER:value [-E -L -V -H]";

	/* parsing variables */
	extern char *optarg;
	extern int optind;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;
	int	pargc = 0;
	char	**pargv = NULL;
	char	*parg = NULL;
	
	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	*message = NULL;
	
	/* parameter controls */
	struct mb_process_struct process;
	
	/* processing variables */
	int	explicit = MB_NO;
	int	lookforfiles = MB_NO;
	int	mbp_ifile_specified;
	char	mbp_ifile[MBP_FILENAMESIZE];
	int	nscan, toggle;
	int	i, j;
	
	char	*ctime();
	char	*getenv();

	/* set default input and output */
	mbp_ifile_specified = MB_NO;
	strcpy (mbp_ifile, "\0");
	
	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhEeI:i:LlP:p:")) != -1)
	  switch (c) 
		{
		case 'H':
		case 'h':
			help++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'E':
		case 'e':
			explicit = MB_YES;
			flag++;
			break;
		case 'I':
		case 'i':
			mbp_ifile_specified = MB_YES;
			sscanf (optarg,"%s", mbp_ifile);
			flag++;
			break;
		case 'L':
		case 'l':
			lookforfiles = MB_YES;
			flag++;
			break;
		case 'P':
		case 'p':
			if (strlen(optarg) > 1)
			    {
			    pargv = (char **) realloc(pargv, (pargc + 1) * sizeof(char *));
			    pargv[pargc] = (char *) malloc(strlen(optarg)+1);
			    strcpy(pargv[pargc], optarg);
			    pargc++;
			    }
			flag++;
			break;
		case '?':
			errflg++;
		}

	/* if error flagged then print it and exit */
	if (errflg)
		{
		fprintf(stderr,"usage: %s\n", usage_message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}

	/* print starting message */
	if (verbose == 1 || help)
		{
		fprintf(stderr,"\nProgram %s\n",program_name);
		fprintf(stderr,"Version %s\n",rcs_id);
		fprintf(stderr,"MB-System Version %s\n",MB_VERSION);
		}

	/* if help desired then print it and exit */
	if (help)
	    {
	    fprintf(stderr,"MB-System Version %s\n",MB_VERSION);
	    fprintf(stderr,"\n%s\n",help_message);
	    fprintf(stderr,"\nusage: %s\n", usage_message);
	    exit(error);
	    }

	/* quit if no input file specified */
	if (mbp_ifile_specified == MB_NO)
	    {
	    fprintf(stderr,"\nProgram <%s> requires an input data file.\n",program_name);
	    fprintf(stderr,"The input file must be specified with the -I option.\n");
	    fprintf(stderr,"\nProgram <%s> Terminated\n",
		    program_name);
	    error = MB_ERROR_OPEN_FAIL;
	    exit(error);
	    }
		
	/* load parameters */
	status = mb_pr_readpar(verbose, mbp_ifile, lookforfiles, 
			&process, &error);
	strcpy(process.mbp_ifile, mbp_ifile);
	process.mbp_ifile_specified = MB_YES;
	
	/* process parameter list */
	for (i=0;i<pargc;i++)
		{
		/* general parameters */
		if (strncmp(pargv[i], "OUTFILE", 7) == 0)
		    {
		    nscan = sscanf(pargv[i], "OUTFILE:%s", process.mbp_ofile);
		    if (nscan == 1)
			process.mbp_ofile_specified = MB_YES;
		    else
			process.mbp_ofile_specified = MB_NO;
		    }
		else if (strncmp(pargv[i], "FORMAT", 6) == 0)
		    {
		    sscanf(pargv[i], "FORMAT:%d", &process.mbp_format);
		    process.mbp_format_specified = MB_YES;
		    }
		    
		/* navigation merging */
		else if (strncmp(pargv[i], "NAVMODE", 7) == 0)
		    {
		    sscanf(pargv[i], "NAVMODE:%d", &process.mbp_nav_mode);
		    }
		else if (strncmp(pargv[i], "NAVFILE", 7) == 0)
		    {
		    sscanf(pargv[i], "NAVFILE:%s", process.mbp_navfile);
		    if (explicit == MB_NO)
			{
			process.mbp_nav_mode = MBP_NAV_ON;
			process.mbp_nav_heading = MBP_NAV_ON;
			process.mbp_nav_speed = MBP_NAV_ON;
			process.mbp_nav_draft = MBP_NAV_ON;
			}
		    }
		else if (strncmp(pargv[i], "NAVFORMAT", 9) == 0)
		    {
		    sscanf(pargv[i], "NAVFORMAT:%d", &process.mbp_nav_format);
		    }
		else if (strncmp(pargv[i], "NAVHEADING", 10) == 0)
		    {
		    sscanf(pargv[i], "NAVHEADING:%d", &process.mbp_nav_heading);
		    }
		else if (strncmp(pargv[i], "NAVSPEED", 8) == 0)
		    {
		    sscanf(pargv[i], "%s:%d", &process.mbp_nav_speed);
		    }
		else if (strncmp(pargv[i], "NAVDRAFT", 8) == 0)
		    {
		    sscanf(pargv[i], "NAVSPEED:%d", &process.mbp_nav_draft);
		    }
		else if (strncmp(pargv[i], "NAVINTERP", 9) == 0)
		    {
		    sscanf(pargv[i], "NAVINTERP:%d", &process.mbp_nav_algorithm);
		    }

		/* adjusted navigation merging */
		else if (strncmp(pargv[i], "NAVADJMODE", 10) == 0)
		    {
		    sscanf(pargv[i], "NAVADJMODE:%d", &process.mbp_navadj_mode);
		    }
		else if (strncmp(pargv[i], "NAVADJFILE", 10) == 0)
		    {
		    sscanf(pargv[i], "NAVADJFILE:%s", process.mbp_navadjfile);
		    if (explicit == MB_NO)
			{
			process.mbp_navadj_mode = MBP_NAV_ON;
			}
		    }
		else if (strncmp(pargv[i], "NAVADJINTERP", 12) == 0)
		    {
		    sscanf(pargv[i], "NAVADJINTERP:%d", &process.mbp_navadj_algorithm);
		    }
    
		/* bathymetry editing */
		else if (strncmp(pargv[i], "EDITSAVEMODE", 12) == 0)
		    {
		    sscanf(pargv[i], "EDITSAVEMODE:%d", process.mbp_edit_mode);
		    }
		else if (strncmp(pargv[i], "EDITSAVEFILE", 12) == 0)
		    {
		    sscanf(pargv[i], "EDITSAVEFILE:%s", process.mbp_editfile);
		    if (explicit == MB_NO)
			{
			process.mbp_edit_mode = MBP_EDIT_ON;
			}
		    }
    
		/* bathymetry recalculation */
		else if (strncmp(pargv[i], "RAYTRACE", 8) == 0)
		    {
		    sscanf(pargv[i], "RAYTRACE:%d", &process.mbp_svp_mode);
		    }
		else if (strncmp(pargv[i], "SVPFILE", 7) == 0)
		    {
		    sscanf(pargv[i], "SVPFILE:%s", process.mbp_svpfile);
		    if (explicit == MB_NO)
			{
			process.mbp_svp_mode = MBP_SVP_ON;
			}
		    }
		else if (strncmp(pargv[i], "SSVMODE", 7) == 0)
		    {
		    sscanf(pargv[i], "SSVMODE:%d", &process.mbp_ssv_mode);
		    }
		else if (strncmp(pargv[i], "SSV", 3) == 0)
		    {
		    sscanf(pargv[i], "SSV:%lf", &process.mbp_ssv);
		    }
		else if (strncmp(pargv[i], "TTMULTIPLY", 10) == 0)
		    {
		    sscanf(pargv[i], "TTMULTIPLY:%lf", &process.mbp_tt_mult);
		    }
		else if (strncmp(pargv[i], "CORRECTED", 9) == 0)
		    {
		    sscanf(pargv[i], "CORRECTED:%d", &process.mbp_corrected);
		    }
    
		/* draft correction */
		else if (strncmp(pargv[i], "DRAFTMODE", 9) == 0)
		    {
		    sscanf(pargv[i], "DRAFTMODE:%d", &process.mbp_draft_mode);
		    }
		else if (strncmp(pargv[i], "DRAFTOFFSET", 11) == 0)
		    {
		    sscanf(pargv[i], "DRAFTOFFSET:%lf", &process.mbp_draft_offset);
		    if (explicit == MB_NO
			&& process.mbp_draft_mode == MBP_DRAFT_MULTIPLY)
			{
			process.mbp_draft_mode = MBP_DRAFT_MULTIPLYOFFSET;
			}
		    else if (explicit == MB_NO
			&& process.mbp_draft_mode == MBP_DRAFT_OFF)
			{
			process.mbp_draft_mode = MBP_DRAFT_OFFSET;
			}
		    }
		else if (strncmp(pargv[i], "DRAFTMULTIPLY", 13) == 0)
		    {
		    sscanf(pargv[i], "DRAFTMULTIPLY:%lf", &process.mbp_draft_mult);
		    if (explicit == MB_NO
			&& process.mbp_draft_mode == MBP_DRAFT_OFFSET)
			{
			process.mbp_draft_mode = MBP_DRAFT_MULTIPLYOFFSET;
			}
		    else if (explicit == MB_NO
			&& process.mbp_draft_mode == MBP_DRAFT_OFF)
			{
			process.mbp_draft_mode = MBP_DRAFT_MULTIPLY;
			}
		    }
		else if (strncmp(pargv[i], "DRAFT", 5) == 0)
		    {
		    sscanf(pargv[i], "DRAFT:%lf", &process.mbp_draft);
		    if (explicit == MB_NO)
			{
			process.mbp_draft_mode = MBP_DRAFT_SET;
			}
		    }
    
		/* heave correction */
		else if (strncmp(pargv[i], "HEAVEMODE", 9) == 0)
		    {
		    sscanf(pargv[i], "HEAVEMODE:%d", &process.mbp_heave_mode);
		    }
		else if (strncmp(pargv[i], "HEAVEOFFSET", 11) == 0)
		    {
		    sscanf(pargv[i], "HEAVEOFFSET:%lf", &process.mbp_heave);
		    if (explicit == MB_NO
			&& process.mbp_heave_mode == MBP_HEAVE_MULTIPLY)
			{
			process.mbp_heave_mode = MBP_HEAVE_MULTIPLYOFFSET;
			}
		    else if (explicit == MB_NO
			&& process.mbp_heave_mode == MBP_HEAVE_OFF)
			{
			process.mbp_heave_mode = MBP_HEAVE_OFFSET;
			}
		    }
		else if (strncmp(pargv[i], "HEAVEMULTIPLY", 13) == 0)
		    {
		    sscanf(pargv[i], "HEAVEMULTIPLY:%lf", &process.mbp_heave_mult);
		    if (explicit == MB_NO
			&& process.mbp_heave_mode == MBP_HEAVE_OFFSET)
			{
			process.mbp_heave_mode = MBP_HEAVE_MULTIPLYOFFSET;
			}
		    else if (explicit == MB_NO
			&& process.mbp_heave_mode == MBP_HEADING_OFF)
			{
			process.mbp_heave_mode = MBP_HEAVE_MULTIPLY;
			}
		    }
    
		/* roll correction */
		else if (strncmp(pargv[i], "ROLLBIASMODE", 12) == 0)
		    {
		    sscanf(pargv[i], "ROLLBIASMODE:%d", &process.mbp_rollbias_mode);
		    }
		else if (strncmp(pargv[i], "ROLLBIASPORT", 12) == 0)
		    {
		    sscanf(pargv[i], "ROLLBIASPORT:%lf", &process.mbp_rollbias_port);
		    if (explicit == MB_NO)
			{
			process.mbp_rollbias_mode = MBP_ROLLBIAS_DOUBLE;
			}
		    }
		else if (strncmp(pargv[i], "ROLLBIASSTBD", 12) == 0)
		    {
		    sscanf(pargv[i], "ROLLBIASSTBD:%lf", &process.mbp_rollbias_stbd);
		    if (explicit == MB_NO)
			{
			process.mbp_rollbias_mode = MBP_ROLLBIAS_DOUBLE;
			}
		    }
		else if (strncmp(pargv[i], "ROLLBIAS", 8) == 0)
		    {
		    sscanf(pargv[i], "ROLLBIAS:%lf", &process.mbp_rollbias);
		    if (explicit == MB_NO)
			{
			process.mbp_rollbias_mode = MBP_ROLLBIAS_SINGLE;
			}
		    }
    
		/* pitch correction */
		else if (strncmp(pargv[i], "PITCHBIASMODE", 13) == 0)
		    {
		    sscanf(pargv[i], "PITCHBIASMODE:%d", &process.mbp_pitchbias_mode);
		    }
		else if (strncmp(pargv[i], "PITCHBIAS", 9) == 0)
		    {
		    sscanf(pargv[i], "PITCHBIAS:%lf", &process.mbp_pitchbias);
		    if (explicit == MB_NO)
			{
			process.mbp_pitchbias = MBP_PITCHBIAS_ON;
			}
		    }
    
		/* heading correction */
		else if (strncmp(pargv[i], "HEADINGMODE", 11) == 0)
		    {
		    sscanf(pargv[i], "HEADINGMODE:%d", &process.mbp_heading_mode);
		    }
		else if (strncmp(pargv[i], "HEADINGOFFSET", 13) == 0)
		    {
		    sscanf(pargv[i], "HEADINGOFFSET:%lf", &process.mbp_headingbias);
		    if (explicit == MB_NO
			&& process.mbp_heading_mode == MBP_HEADING_CALC)
			{
			process.mbp_heading_mode = MBP_HEADING_CALCOFFSET;
			}
		    else if (explicit == MB_NO
			&& process.mbp_heading_mode == MBP_HEADING_OFF)
			{
			process.mbp_heading_mode = MBP_HEADING_OFFSET;
			}
		    }
    
		/* sidescan recalculation */
		else if (strncmp(pargv[i], "SSRECALCMODE", 12) == 0)
		    {
		    sscanf(pargv[i], "SSRECALCMODE:%d", &process.mbp_ssrecalc_mode);
		    }
		else if (strncmp(pargv[i], "SSPIXELSIZE", 11) == 0)
		    {
		    sscanf(pargv[i], "SSPIXELSIZE:%lf", &process.mbp_ssrecalc_pixelsize);
		    }
		else if (strncmp(pargv[i], "SSSWATHWIDTH", 11) == 0)
		    {
		    sscanf(pargv[i], "SSSWATHWIDTH:%lf", &process.mbp_ssrecalc_swathwidth);
		    }
		else if (strncmp(pargv[i], "SSINTERPOLATE", 11) == 0)
		    {
		    sscanf(pargv[i], "SSINTERPOLATE:%d", &process.mbp_ssrecalc_interpolate);
		    }
   
		/* metadata insertion */
		else if (strncmp(pargv[i], "METAOPERATOR", 12) == 0)
		    {
			strcpy(process.mbp_meta_operator,&(pargv[i][13]));
		    }
		else if (strncmp(pargv[i], "METAPLATFORM", 12) == 0)
		    {
			strcpy(process.mbp_meta_platform,&(pargv[i][13]));
		    }
		else if (strncmp(pargv[i], "METASONAR", 9) == 0)
		    {
			strcpy(process.mbp_meta_sonar,&(pargv[i][10]));
		    }
		else if (strncmp(pargv[i], "METASURVEY", 10) == 0)
		    {
			strcpy(process.mbp_meta_survey,&(pargv[i][11]));
		    }
		else if (strncmp(pargv[i], "METAPI", 6) == 0)
		    {
			strcpy(process.mbp_meta_pi,&(pargv[i][7]));
		    }
		else if (strncmp(pargv[i], "METACLIENT", 10) == 0)
		    {
			strcpy(process.mbp_meta_client,&(pargv[i][11]));
		    }
		}
		
	/* get bathymetry recalculation mode */
	if (process.mbp_svp_mode == MBP_SVP_ON)
	    {
	    process.mbp_bathrecalc_mode = MBP_BATHRECALC_RAYTRACE;
	    }
	else if (process.mbp_rollbias_mode != MBP_ROLLBIAS_OFF
	    || process.mbp_pitchbias_mode != MBP_PITCHBIAS_OFF)
	    {
	    process.mbp_bathrecalc_mode = MBP_BATHRECALC_ROTATE;
	    }
	else if (process.mbp_draft_mode != MBP_DRAFT_OFF)
	    {
	    process.mbp_bathrecalc_mode = MBP_BATHRECALC_OFFSET;
	    }
	else
	    {
	    process.mbp_bathrecalc_mode = MBP_BATHRECALC_OFF;
	    }

	/* print starting debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
	    fprintf(stderr,"dbg2  Version %s\n",rcs_id);
	    fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
	    fprintf(stderr,"\ndbg2  MB-System Control Parameters:\n");
	    fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
	    fprintf(stderr,"dbg2       help:            %d\n",help);
	    fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
	    }

	/* print starting info statements */
	if (verbose == 1)
	    {
	    fprintf(stderr,"\nProgram <%s>\n",program_name);
	    fprintf(stderr,"Version %s\n",rcs_id);
	    fprintf(stderr,"MB-system Version %s\n",MB_VERSION);		
	    fprintf(stderr,"\nOutput MBprocess Parameters:\n");
	    fprintf(stderr,"\nInput and Output Files:\n");
	    if (process.mbp_format_specified == MB_YES)
		fprintf(stderr,"  Format:                        %d\n",process.mbp_format);
	    if (process.mbp_ifile_specified == MB_YES)
		fprintf(stderr,"  Input file:                    %s\n",process.mbp_ifile);
	    if (process.mbp_ifile_specified == MB_YES)
		fprintf(stderr,"  Output file:                   %s\n",process.mbp_ofile);

	    fprintf(stderr,"\nNavigation Merging:\n");
	    if (process.mbp_nav_mode == MBP_NAV_ON)
		{
		fprintf(stderr,"  Navigation merged from navigation file.\n");
		if (process.mbp_nav_heading == MBP_NAV_ON)
		    fprintf(stderr,"  Heading merged from navigation file.\n");
		else
		    fprintf(stderr,"  Heading not merged from navigation file.\n");
		if (process.mbp_nav_speed == MBP_NAV_ON)
		    fprintf(stderr,"  Speed merged from navigation file.\n");
		else
		    fprintf(stderr,"  Speed not merged from navigation file.\n");
		if (process.mbp_nav_draft == MBP_NAV_ON)
		    fprintf(stderr,"  Draft merged from navigation file.\n");
		else
		    fprintf(stderr,"  Draft not merged from navigation file.\n");
		}
	    else
		fprintf(stderr,"  Navigation not merged from navigation file.\n");
	    fprintf(stderr,"  Navigation file:               %s\n", process.mbp_navfile);
	    if (process.mbp_nav_algorithm == MBP_NAV_LINEAR)
		fprintf(stderr,"  Navigation algorithm:          linear interpolation\n");
	    else if (process.mbp_nav_algorithm == MBP_NAV_SPLINE)
		fprintf(stderr,"  Navigation algorithm:          spline interpolation\n");

	    fprintf(stderr,"\nAdjusted Navigation Merging:\n");
	    if (process.mbp_navadj_mode == MBP_NAV_ON)
		fprintf(stderr,"  Navigation merged from adjusted navigation file.\n");
	    else
		fprintf(stderr,"  Navigation not merged from adjusted navigation file.\n");
	    fprintf(stderr,"  Adjusted navigation file:      %s\n", process.mbp_navadjfile);
	    if (process.mbp_navadj_algorithm == MBP_NAV_LINEAR)
		fprintf(stderr,"  Adjusted navigation algorithm: linear interpolation\n");
	    else if (process.mbp_navadj_algorithm == MBP_NAV_SPLINE)
		fprintf(stderr,"  Adjusted navigation algorithm: spline interpolation\n");

	    fprintf(stderr,"\nBathymetry Editing:\n");
	    if (process.mbp_edit_mode == MBP_EDIT_ON)
		fprintf(stderr,"  Bathymetry edits applied from file.\n");
	    else
		fprintf(stderr,"  Bathymetry edits not applied from file.\n");
	    fprintf(stderr,"  Bathymetry edit file:          %s\n", process.mbp_editfile);

	    fprintf(stderr,"\nBathymetry Recalculation:\n");
	    if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_OFF)
		fprintf(stderr,"  Bathymetry not recalculated.\n");
	    else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_RAYTRACE)
		fprintf(stderr,"  Bathymetry recalculated by raytracing.\n");
	    else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_ROTATE)
		fprintf(stderr,"  Bathymetry recalculated by rigid rotation.\n");
	    else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_OFFSET)
		fprintf(stderr,"  Bathymetry recalculated by transducer depth shift.\n");
	    fprintf(stderr,"  SVP file:                      %s\n", process.mbp_svpfile);
	    if (process.mbp_ssv_mode == MBP_SSV_OFF)
		fprintf(stderr,"  SSV not modified.\n");
	    else if (process.mbp_ssv_mode == MBP_SSV_OFFSET)
		fprintf(stderr,"  SSV offset by constant.\n");
	    else
		fprintf(stderr,"  SSV set to constant.\n");
	    fprintf(stderr,"  SSV offset/constant:           %f m/s\n", process.mbp_ssv);
	    fprintf(stderr,"  Travel time multiplier:        %f m\n", process.mbp_tt_mult);
	    if (process.mbp_corrected == MB_YES)
		fprintf(stderr,"  Bathymetry reference:          CORRECTED\n");
	    else if (process.mbp_corrected == MB_NO)
		fprintf(stderr,"  Bathymetry reference:          UNCORRECTED\n");

	    fprintf(stderr,"\nDraft Correction:\n");
	    if (process.mbp_draft_mode == MBP_DRAFT_OFF)
		fprintf(stderr,"  Draft not modified.\n");
	    else if (process.mbp_draft_mode == MBP_DRAFT_SET)
		fprintf(stderr,"  Draft set to constant.\n");
	    else if (process.mbp_draft_mode == MBP_DRAFT_OFFSET)
		fprintf(stderr,"  Draft offset by constant.\n");
	    else if (process.mbp_draft_mode == MBP_DRAFT_MULTIPLY)
		fprintf(stderr,"  Draft multiplied by constant.\n");
	    else if (process.mbp_draft_mode == MBP_DRAFT_MULTIPLYOFFSET)
		fprintf(stderr,"  Draft multiplied and offset by constants.\n");
	    fprintf(stderr,"  Draft constant:                %f m\n", process.mbp_draft);
	    fprintf(stderr,"  Draft offset:                  %f m\n", process.mbp_draft_offset);
	    fprintf(stderr,"  Draft multiplier:              %f m\n", process.mbp_draft_mult);

	    fprintf(stderr,"\nHeave Correction:\n");
	    if (process.mbp_heave_mode == MBP_HEAVE_OFF)
		fprintf(stderr,"  Heave not modified.\n");
	    else if (process.mbp_heave_mode == MBP_HEAVE_OFFSET)
		fprintf(stderr,"  Heave offset by constant.\n");
	    else if (process.mbp_heave_mode == MBP_HEAVE_MULTIPLY)
		fprintf(stderr,"  Heave multiplied by constant.\n");
	    else if (process.mbp_heave_mode == MBP_HEAVE_MULTIPLYOFFSET)
		fprintf(stderr,"  Heave multiplied and offset by constants.\n");
	    fprintf(stderr,"  Heave offset:                  %f m\n", process.mbp_heave);
	    fprintf(stderr,"  Heave multiplier:              %f m\n", process.mbp_heave_mult);

	    fprintf(stderr,"\nRoll Correction:\n");
	    if (process.mbp_rollbias_mode == MBP_ROLLBIAS_OFF)
		fprintf(stderr,"  Roll not modified.\n");
	    else if (process.mbp_rollbias_mode == MBP_ROLLBIAS_SINGLE)
		fprintf(stderr,"  Roll offset by bias.\n");
	    else if (process.mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE)
		fprintf(stderr,"  Roll offset by separate port and starboard biases.\n");
	    fprintf(stderr,"  Roll bias:                     %f deg\n", process.mbp_rollbias);
	    fprintf(stderr,"  Port roll bias:                %f deg\n", process.mbp_rollbias_port);
	    fprintf(stderr,"  Starboard roll bias:           %f deg\n", process.mbp_rollbias_stbd);

	    fprintf(stderr,"\nPitch Correction:\n");
	    if (process.mbp_pitchbias_mode == MBP_PITCHBIAS_OFF)
		fprintf(stderr,"  Pitch not modified.\n");
	    else
		fprintf(stderr,"  Pitch offset by bias.\n");
	    fprintf(stderr,"  Pitch bias:                    %f deg\n", process.mbp_pitchbias);

	    fprintf(stderr,"\nHeading Correction:\n");
	    if (process.mbp_heading_mode == MBP_HEADING_OFF)
		fprintf(stderr,"  Heading not modified.\n");
	    else if (process.mbp_heading_mode == MBP_HEADING_CALC)
		fprintf(stderr,"  Heading replaced by course-made-good.\n");
	    else if (process.mbp_heading_mode == MBP_HEADING_OFFSET)
		fprintf(stderr,"  Heading offset by bias.\n");
	    else if (process.mbp_heading_mode == MBP_HEADING_CALCOFFSET)
		fprintf(stderr,"  Heading replaced by course-made-good and then offset by bias.\n");
	    fprintf(stderr,"  Heading offset:                %f deg\n", process.mbp_headingbias);

	    fprintf(stderr,"\nSidescan Recalculation:\n");
	    if (process.mbp_ssrecalc_mode == MBP_SSRECALC_ON)
		fprintf(stderr,"  Sidescan recalculated.\n");
	    else
		fprintf(stderr,"  Sidescan not recalculated.\n");
	    fprintf(stderr,"  Sidescan pixel size:           %f\n",process.mbp_ssrecalc_pixelsize);
	    fprintf(stderr,"  Sidescan swath width:          %f\n",process.mbp_ssrecalc_swathwidth);
	    fprintf(stderr,"  Sidescan interpolation:        %d\n",process.mbp_ssrecalc_interpolate);

	    fprintf(stderr,"\nMetadata Insertion:\n");
	    fprintf(stderr,"  Metadata operator:             %s\n",process.mbp_meta_operator);
	    fprintf(stderr,"  Metadata platform:             %s\n",process.mbp_meta_platform);
	    fprintf(stderr,"  Metadata sonar:                %s\n",process.mbp_meta_sonar);
	    fprintf(stderr,"  Metadata survey:               %s\n",process.mbp_meta_survey);
	    fprintf(stderr,"  Metadata pi:                   %s\n",process.mbp_meta_pi);
	    fprintf(stderr,"  Metadata client:               %s\n",process.mbp_meta_client);
	    }

	/* write parameters */
	status = mb_pr_writepar(verbose, mbp_ifile, 
			&process, &error);

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/


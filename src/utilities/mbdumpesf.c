/*--------------------------------------------------------------------
 *    The MB-system:	mbdumpesf.c	3/20/2008
 *    $Id: mbdumpesf.c,v 5.0 2008-05-26 03:27:31 caress Exp $
 *
 *    Copyright (c) 2008 by
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
 * mbdumpesf reads an MB-System edit save file and dumps the contents
 * as an ascii table to stdout. This is primarily used for debugging
 * bathymetry editing tools such as mbedit and mbeditviz.
 *
 * Author:	D. W. Caress
 * Date:	March 20, 2008
 * 
 * $Log: not supported by cvs2svn $
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"
#include "../../include/mb_process.h"
#include "../../include/mb_swap.h"

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	/* id variables */
	static char rcs_id[] = "$Id: mbdumpesf.c,v 5.0 2008-05-26 03:27:31 caress Exp $";
	static char program_name[] = "mbdumpesf";
	static char help_message[] =  "mbdumpesf reads an MB-System edit save file and dumps the \ncontents as an ascii table to stdout.";
	static char usage_message[] = "mbdumpesf [-Iesffile -V -H]";

	/* parsing variables */
	extern char *optarg;
	extern int optkind;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	*message = NULL;

	/* MBIO read control parameters */
	char	esffile[MB_PATH_MAXLINE];
	FILE	*esffp = NULL;
	struct stat file_status;
	int	fstat;
	double	stime_d;
	int	sbeam;
	int	saction;
	char	fmode[16];
	int 	byteswapped;
	int 	nedit;
	double	time_d;
	int	time_i[7];
	int	beam;
	int	action;
	int	beam_flag = 0;
	int	beam_unflag = 0;
	int	beam_zero = 0;
	int	beam_filter = 0;

	int	i, j, k, l, m;

	/* get current default values */
	byteswapped = mb_swap_check();

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhI:i:")) != -1)
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
		case 'I':
		case 'i':
			sscanf (optarg,"%s", esffile);
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

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* print starting message */
	if (verbose == 1 || help)
		{
		fprintf(stderr,"\nProgram %s\n",program_name);
		fprintf(stderr,"Version %s\n",rcs_id);
		fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Control Parameters:\n");
		fprintf(stderr,"dbg2       verbose:        %d\n",verbose);
		fprintf(stderr,"dbg2       help:           %d\n",help);
		fprintf(stderr,"dbg2       esf file:       %s\n",esffile);
		}
		
	/* check that esf file exists */
	fstat = stat(esffile, &file_status);
	if (fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR)
	    	{
		/* get number of edits */
		nedit = file_status.st_size / (sizeof(double) + 2 * sizeof(int));
		
		/* open the esf file */
		if ((esffp = fopen(esffile, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to edit save file <%s> for reading\n",esffile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
			exit(error);
			}
			
		/* loop over reading edit events and printing them out */
		for (i=0;i<nedit && error == MB_ERROR_NO_ERROR;i++)
			{
			if (fread(&(time_d), sizeof(double), 1, esffp) != 1
				|| fread(&(beam), sizeof(int), 1, esffp) != 1
				|| fread(&(action), sizeof(int), 1, esffp) != 1)
				{
				status = MB_FAILURE;
				error = MB_ERROR_EOF;
				}
			else if (byteswapped == MB_YES)
				{
				mb_swap_double(&(time_d));
				beam = mb_swap_int(beam);
				action = mb_swap_int(action);
				}
			if (action == MBP_EDIT_FLAG)
				{
				beam_flag++;
				}
			else if (action == MBP_EDIT_UNFLAG)
				{
				beam_unflag++;
				}
			else if (action == MBP_EDIT_ZERO)
				{
				beam_zero++;
				}
			else if (action == MBP_EDIT_FILTER)
				{
				beam_filter++;
				}
				
			mb_get_date(verbose,time_d,time_i);
			fprintf(stdout,"EDITS READ: i:%d time: %f %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d beam:%d action:%d\n",
				i,time_d,time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				beam,action);
			}
		    	
		/* close the edit save file */
		fclose(esffp);
		}
	

	/* give the statistics */
	if (verbose >= 1)
		{
		fprintf(stderr,"\nBeam flag read totals:\n");
		fprintf(stderr,"\t%d beams flagged manually\n",beam_flag);
		fprintf(stderr,"\t%d beams unflagged\n",beam_unflag);
		fprintf(stderr,"\t%d beams zeroed\n",beam_zero);
		fprintf(stderr,"\t%d beams flagged by filter\n",beam_filter);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/

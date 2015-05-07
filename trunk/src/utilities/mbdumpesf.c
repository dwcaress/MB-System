/*--------------------------------------------------------------------
 *    The MB-system:	mbdumpesf.c	3/20/2008
 *    $Id$
 *
 *    Copyright (c) 2008-2015 by
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
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_process.h"
#include "mb_swap.h"

static char rcs_id[] = "$Id$";

#define OUTPUT_TEXT	0
#define OUTPUT_ESF	1

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	/* id variables */
	char program_name[] = "mbdumpesf";
	char help_message[] =  "mbdumpesf reads an MB-System edit save file and dumps the \ncontents as an ascii table to stdout.";
	//char usage_message[] = "mbdumpesf [-Iesffile -V -H]";
	char usage_message[] = "mbdumpesf --input=esffile\n"
				"\t[--output=esffile --ignore-unflag --ignore-flag \n"
				"\t--ignore-filter --ignore-zero \n"
				"\t--verbose --help]";

	/* parsing variables */
	extern char *optarg;
	int	option_index;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;

	/* command line option definitions */
	/* mbdumpesf --input=esffile
	 *		[--output=esffile --ignore-unflag --ignore-flag
	 *		--ignore-filter --ignore-zero
	 *		--verbose --help] */
	static struct option options[] =
		{
		{"verbose",			no_argument, 		NULL, 		0},
		{"help",			no_argument, 		NULL, 		0},
		{"input",			required_argument, 	NULL, 		0},
		{"output",			required_argument, 	NULL, 		0},
		{"ignore-unflag",		no_argument, 		NULL, 		0},
		{"ignore-flag",			no_argument, 		NULL, 		0},
		{"ignore-filter",		no_argument, 		NULL, 		0},
		{"ignore-zero",			no_argument, 		NULL, 		0},
		{NULL,				0, 			NULL, 		0}
		};

	/* MBIO read and write control parameters */
	char	iesffile[MB_PATH_MAXLINE];
	char	oesffile[MB_PATH_MAXLINE];
	int	omode = OUTPUT_TEXT;
	FILE	*iesffp = NULL;
	FILE	*oesffp = NULL;
	struct stat file_status;
	int	fstat;
	int 	byteswapped;
	int 	nedit;
	double	time_d;
	int	time_i[7];
	int	beam;
	int	action;
	
	int	ignore;
	int	ignore_unflag = MB_NO;
	int	ignore_flag = MB_NO;
	int	ignore_filter = MB_NO;
	int	ignore_zero = MB_NO;
	
	int	beam_flag = 0;
	int	beam_unflag = 0;
	int	beam_zero = 0;
	int	beam_filter = 0;
	int	beam_flag_ignore = 0;
	int	beam_unflag_ignore = 0;
	int	beam_zero_ignore = 0;
	int	beam_filter_ignore = 0;

	int	i;

	/* get current default values */
	byteswapped = mb_swap_check();

	/* process argument list */
	while ((c = getopt_long(argc, argv, "VvHhI:i:", options, &option_index)) != -1)
	  switch (c)
		{
		/* long options all return c=0 */
		case 0:
			/* verbose */
			if (strcmp("verbose", options[option_index].name) == 0)
				{
				verbose++;
				}
			
			/* help */
			else if (strcmp("help", options[option_index].name) == 0)
				{
				help++;
				}
				
			/*-------------------------------------------------------
			 * Define input and optional output esf file */
			
			/* input */
			else if (strcmp("input", options[option_index].name) == 0)
				{
				strcpy(iesffile, optarg);
				}
			
			/* output */
			else if (strcmp("output", options[option_index].name) == 0)
				{
				strcpy(oesffile, optarg);
				omode = OUTPUT_ESF;
				}
				
			/*-------------------------------------------------------
			 * Set special modes to ignore types of edit events,
			 * thereby removing them from the output stream */
			
			/* ignore-unflag */
			else if (strcmp("ignore-unflag", options[option_index].name) == 0)
				{
				ignore_unflag = MB_YES;
				}
			
			/* ignore-flag */
			else if (strcmp("ignore-flag", options[option_index].name) == 0)
				{
				ignore_flag = MB_YES;
				}
			
			/* ignore-unflag */
			else if (strcmp("ignore-filter", options[option_index].name) == 0)
				{
				ignore_filter = MB_YES;
				}
			
			/* ignore-unflag */
			else if (strcmp("ignore-zero", options[option_index].name) == 0)
				{
				ignore_zero = MB_YES;
				}
				
			break;
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
			sscanf (optarg,"%s", iesffile);
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
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		fprintf(stderr,"dbg2       help:             %d\n",help);
		fprintf(stderr,"dbg2       input esf file:   %s\n",iesffile);
		fprintf(stderr,"dbg2       omode:            %d\n",omode);
		if (omode == OUTPUT_ESF)
			fprintf(stderr,"dbg2       output esf file:  %s\n",oesffile);
		fprintf(stderr,"dbg2       ignore_unflag:    %d\n",ignore_unflag);
		fprintf(stderr,"dbg2       ignore_flag:      %d\n",ignore_flag);
		fprintf(stderr,"dbg2       ignore_filter:    %d\n",ignore_filter);
		fprintf(stderr,"dbg2       ignore_zero:      %d\n",ignore_zero);
		}

	/* check that esf file exists */
	fstat = stat(iesffile, &file_status);
	if (fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR)
	    	{
		/* get number of edits */
		nedit = file_status.st_size / (sizeof(double) + 2 * sizeof(int));

		/* open the input esf file */
		if ((iesffp = fopen(iesffile, "r")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to edit save file <%s> for reading\n",iesffile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
			exit(error);
			}

		/* open the output esf file */
		if (omode == OUTPUT_ESF
			&& (oesffp = fopen(oesffile, "w")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to edit save file <%s> for reading\n",iesffile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
			exit(error);
			}

		/* loop over reading edit events and printing them out */
		for (i=0;i<nedit && error == MB_ERROR_NO_ERROR;i++)
			{
			if (fread(&(time_d), sizeof(double), 1, iesffp) != 1
				|| fread(&(beam), sizeof(int), 1, iesffp) != 1
				|| fread(&(action), sizeof(int), 1, iesffp) != 1)
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
			ignore = MB_NO;
			if (action == MBP_EDIT_FLAG)
				{
				beam_flag++;
				if (ignore_flag == MB_YES)
					{
					ignore = MB_YES;
					beam_flag_ignore++;
					}
				}
			else if (action == MBP_EDIT_UNFLAG)
				{
				beam_unflag++;
				if (ignore_flag == MB_YES)
					{
					ignore = MB_YES;
					beam_unflag_ignore++;
					}
				}
			else if (action == MBP_EDIT_ZERO)
				{
				beam_zero++;
				if (ignore_flag == MB_YES)
					{
					ignore = MB_YES;
					beam_zero_ignore++;
					}
				}
			else if (action == MBP_EDIT_FILTER)
				{
				beam_filter++;
				if (ignore_flag == MB_YES)
					{
					ignore = MB_YES;
					beam_filter_ignore++;
					}
				}

			
			/* write out the edit if not ignored */
			if (ignore == MB_NO)
				{
				if (omode == OUTPUT_TEXT)
					{
					mb_get_date(verbose,time_d,time_i);
					fprintf(stdout,"EDITS READ: i:%d time: %f %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d beam:%d action:%d\n",
						i,time_d,time_i[0],time_i[1],time_i[2],
						time_i[3],time_i[4],time_i[5],time_i[6],
						beam,action);
					}
				else
					{
					if (byteswapped == MB_YES)
						{
						mb_swap_double(&time_d);
						beam = mb_swap_int(beam);
						action = mb_swap_int(action);
						}
					if (fwrite(&time_d, sizeof(double), 1, oesffp) != 1)
						{
						status = MB_FAILURE;
						error = MB_ERROR_WRITE_FAIL;
						}
					if (status == MB_SUCCESS
					    && fwrite(&beam, sizeof(int), 1, oesffp) != 1)
						{
						status = MB_FAILURE;
						error = MB_ERROR_WRITE_FAIL;
						}
					if (status == MB_SUCCESS
					    && fwrite(&action, sizeof(int), 1, oesffp) != 1)
						{
						status = MB_FAILURE;
						error = MB_ERROR_WRITE_FAIL;
						}
					}
				}
			}

		/* close the edit save file */
		fclose(iesffp);
		if (omode == OUTPUT_ESF)
			fclose(oesffp);
		}


	/* give the statistics */
	if (verbose >= 1)
		{
		fprintf(stderr,"\nBeam flag read totals:\n");
		fprintf(stderr,"\t%d beams flagged manually\n",beam_flag);
		fprintf(stderr,"\t%d beams unflagged\n",beam_unflag);
		fprintf(stderr,"\t%d beams zeroed\n",beam_zero);
		fprintf(stderr,"\t%d beams flagged by filter\n",beam_filter);
		if (ignore_flag == MB_YES || ignore_unflag == MB_YES
		    || ignore_zero == MB_YES || ignore_filter == MB_YES)
			{
			fprintf(stderr,"\nBeam flag ignore totals:\n");
			fprintf(stderr,"\t%d beams flagged manually (ignored in output)\n",beam_flag_ignore);
			fprintf(stderr,"\t%d beams unflagged (ignored in output)\n",beam_unflag_ignore);
			fprintf(stderr,"\t%d beams zeroed (ignored in output)\n",beam_zero_ignore);
			fprintf(stderr,"\t%d beams flagged by filter (ignored in output)\n",beam_filter_ignore);
			}
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
 *    The MB-system:	mbdatalist.c	10/10/2001
 *    $Id: mbdatalist.c,v 5.0 2001-10-12 21:07:31 caress Exp $
 *
 *    Copyright (c) 2001 by
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
 *
 * $Log: not supported by cvs2svn $
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	static char rcs_id[] = "$Id: mbdatalist.c,v 5.0 2001-10-12 21:07:31 caress Exp $";
	static char program_name[] = "mbdatalist";
	static char help_message[] =  "mbdatalist parses recursive datalist files and outputs the\ncomplete list of data files and formats. \nThe results are dumped to stdout.";
	static char usage_message[] = "mbdatalist [-Fformat -Ifile -P -R -V -H]";
	extern char *optarg;
	extern int optkind;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;

	/* MBIO read control parameters */
	int	read_datalist = MB_NO;
	char	read_file[128];
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	double	file_weight = 1.0;
	int	format;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	char	file[128];

	/* output stream for basic stuff (stdout if verbose <= 1,
		output if verbose > 1) */
	FILE	*output;
	int	i, j;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to stdin */
	strcpy (read_file, "stdin");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhF:f:I:i:PpRr")) != -1)
	  switch (c) 
		{
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			flag++;
			break;
		case 'H':
		case 'h':
			help++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", read_file);
			flag++;
			break;
		case 'P':
		case 'p':
			look_processed = MB_DATALIST_LOOK_YES;
			flag++;
			break;
		case 'R':
		case 'r':
			look_processed = MB_DATALIST_LOOK_NO;
			flag++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case '?':
			errflg++;
		}

	/* set output stream */
	if (verbose <= 1)
		output = stdout;
	else
		output = stderr;

	/* if error flagged then print it and exit */
	if (errflg)
		{
		fprintf(output,"usage: %s\n", usage_message);
		fprintf(output,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}

	/* print starting message */
	if (verbose == 1 || help)
		{
		fprintf(output,"\nProgram %s\n",program_name);
		fprintf(output,"Version %s\n",rcs_id);
		fprintf(output,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(output,"\ndbg2  Program <%s>\n",program_name);
		fprintf(output,"dbg2  Version %s\n",rcs_id);
		fprintf(output,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(output,"dbg2  Control Parameters:\n");
		fprintf(output,"dbg2       verbose:        %d\n",verbose);
		fprintf(output,"dbg2       help:           %d\n",help);
		fprintf(output,"dbg2       file:           %d\n",read_file);
		fprintf(output,"dbg2       format:         %d\n",format);
		fprintf(output,"dbg2       look_processed: %d\n",look_processed);
		fprintf(output,"dbg2       pings:          %d\n",pings);
		fprintf(output,"dbg2       lonflip:        %d\n",lonflip);
		fprintf(output,"dbg2       bounds[0]:      %f\n",bounds[0]);
		fprintf(output,"dbg2       bounds[1]:      %f\n",bounds[1]);
		fprintf(output,"dbg2       bounds[2]:      %f\n",bounds[2]);
		fprintf(output,"dbg2       bounds[3]:      %f\n",bounds[3]);
		fprintf(output,"dbg2       btime_i[0]:     %d\n",btime_i[0]);
		fprintf(output,"dbg2       btime_i[1]:     %d\n",btime_i[1]);
		fprintf(output,"dbg2       btime_i[2]:     %d\n",btime_i[2]);
		fprintf(output,"dbg2       btime_i[3]:     %d\n",btime_i[3]);
		fprintf(output,"dbg2       btime_i[4]:     %d\n",btime_i[4]);
		fprintf(output,"dbg2       btime_i[5]:     %d\n",btime_i[5]);
		fprintf(output,"dbg2       btime_i[6]:     %d\n",btime_i[6]);
		fprintf(output,"dbg2       etime_i[0]:     %d\n",etime_i[0]);
		fprintf(output,"dbg2       etime_i[1]:     %d\n",etime_i[1]);
		fprintf(output,"dbg2       etime_i[2]:     %d\n",etime_i[2]);
		fprintf(output,"dbg2       etime_i[3]:     %d\n",etime_i[3]);
		fprintf(output,"dbg2       etime_i[4]:     %d\n",etime_i[4]);
		fprintf(output,"dbg2       etime_i[5]:     %d\n",etime_i[5]);
		fprintf(output,"dbg2       etime_i[6]:     %d\n",etime_i[6]);
		fprintf(output,"dbg2       speedmin:       %f\n",speedmin);
		fprintf(output,"dbg2       timegap:        %f\n",timegap);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(output,"\n%s\n",help_message);
		fprintf(output,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose,read_file,NULL,&format,&error);

	/* if not a datalist just output filename format and weight */
	if (format > 0)
		{
		fprintf(output, "%s %d %f\n", read_file, format, file_weight);
		}
		
	/* else parse datalist */
	else
		{
		if ((status = mb_datalist_open(verbose,&datalist,
						read_file,look_processed,&error)) != MB_SUCCESS)
		    {
		    error = MB_ERROR_OPEN_FAIL;
		    fprintf(stderr,"\nUnable to open data list file: %s\n",
			    read_file);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }
		while (status = mb_datalist_read(verbose,datalist,
					    file,&format,&file_weight,&error)
			    == MB_SUCCESS)
			fprintf(output, "%s %d %f\n", file, format, file_weight);
		mb_datalist_close(verbose,&datalist,&error);
		}

	/* set program status */
	status = MB_SUCCESS;

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(output,"\ndbg2  Program <%s> completed\n",
			program_name);
		fprintf(output,"dbg2  Ending status:\n");
		fprintf(output,"dbg2       status:  %d\n",status);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/

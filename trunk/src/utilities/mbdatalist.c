/*--------------------------------------------------------------------
 *    The MB-system:	mbdatalist.c	10/10/2001
 *    $Id$
 *
 *    Copyright (c) 2001-2011 by
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
 * $Log: mbdatalist.c,v $
 * Revision 5.11  2007/07/05 19:16:19  caress
 * Added file copy function.
 *
 * Revision 5.10  2006/01/06 18:19:58  caress
 * Working towards 5.0.8
 *
 * Revision 5.9  2005/11/05 01:07:54  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.8  2004/10/06 19:10:52  caress
 * Release 5.0.5 update.
 *
 * Revision 5.7  2003/04/17 21:17:10  caress
 * Release 5.0.beta30
 *
 * Revision 5.6  2002/05/29 23:43:09  caress
 * Release 5.0.beta18
 *
 * Revision 5.5  2002/04/08 21:01:04  caress
 * Release 5.0.beta17
 *
 * Revision 5.4  2002/04/06 02:53:45  caress
 * Release 5.0.beta16
 *
 * Revision 5.3  2002/03/26 07:45:14  caress
 * Release 5.0.beta15
 *
 * Revision 5.2  2002/02/22 09:07:08  caress
 * Release 5.0.beta13
 *
 * Revision 5.1  2001/10/19  00:56:17  caress
 * Now tries to use relative paths.
 *
 * Revision 5.0  2001/10/12  21:07:31  caress
 * Initial revision.
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"
#include "../../include/mb_format.h"
#include "../../include/mb_status.h"
#include "../../include/mb_process.h"

static char rcs_id[] = "$Id$";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char program_name[] = "mbdatalist";
	char help_message[] =  "mbdatalist parses recursive datalist files and outputs the\ncomplete list of data files and formats. \nThe results are dumped to stdout.";
	char usage_message[] = "mbdatalist [-Fformat -Ifile -N -O -P -Q -Rw/e/s/n -S -U -Y -Z -V -H]";
	extern char *optarg;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;

	/* MBIO read control parameters */
	char	read_file[MB_PATH_MAXLINE];
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	int	look_bounds = MB_NO;
	int	copyfiles = MB_NO;
	int	file_in_bounds = MB_NO;
	double	file_weight = 1.0;
	int	format;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	speedmin;
	double	timegap;
	char	fileroot[MB_PATH_MAXLINE];
	char	file[MB_PATH_MAXLINE];
	char	pwd[MB_PATH_MAXLINE];
	char	command[MB_PATH_MAXLINE];
	char	*filename;
	int	nfile = 0;
	int	make_inf = MB_NO;
	int	force_update = MB_NO;
	int	status_report = MB_NO;
	int	problem_report = MB_NO;
	int	nparproblem;
	int	ndataproblem;
	int	nparproblemtot = 0;
	int	ndataproblemtot = 0;
	int	nproblemfiles = 0;
	int	remove_locks = MB_NO;
	int	make_datalistp = MB_NO;
	
	int	prstatus = MB_PR_FILE_UP_TO_DATE;
	int	lock_status;
	int	lock_error;
	int	locked;
	int	lock_purpose;
	mb_path	lock_program;
	mb_path lock_cpu;
	mb_path lock_user;
	char	lock_date[25];
	mb_path	lockfile;
	
	/* output stream for basic stuff (stdout if verbose <= 1,
		output if verbose > 1) */
	FILE	*output;
	FILE	*fp;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to stdin */
	strcpy (read_file, "datalist.mb-1");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhCcF:f:I:i:NnOoPpQqR:r:SsUuYyZz")) != -1)
	  switch (c) 
		{
		case 'C':
		case 'c':
			copyfiles = MB_YES;
			flag++;
			break;
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
		case 'N':
		case 'n':
			force_update = MB_YES;
			make_inf = MB_YES;
			flag++;
			break;
		case 'O':
		case 'o':
			make_inf = MB_YES;
			flag++;
			break;
		case 'P':
		case 'p':
			look_processed = MB_DATALIST_LOOK_YES;
			flag++;
			break;
		case 'Q':
		case 'q':
			problem_report = MB_YES;
			flag++;
			break;
		case 'R':
		case 'r':
			mb_get_bounds(optarg, bounds);
			look_bounds = MB_YES;
			flag++;
			break;
		case 'S':
		case 's':
			status_report = MB_YES;
			flag++;
			break;
		case 'U':
		case 'u':
			look_processed = MB_DATALIST_LOOK_NO;
			flag++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'Y':
		case 'y':
			remove_locks = MB_YES;
			break;
		case 'Z':
		case 'z':
			make_datalistp = MB_YES;
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
		fprintf(output,"dbg2       file:           %s\n",read_file);
		fprintf(output,"dbg2       format:         %d\n",format);
		fprintf(output,"dbg2       look_processed: %d\n",look_processed);
		fprintf(output,"dbg2       copyfiles:      %d\n",copyfiles);
		fprintf(output,"dbg2       make_inf:       %d\n",make_inf);
		fprintf(output,"dbg2       force_update:   %d\n",force_update);
		fprintf(output,"dbg2       status_report:  %d\n",status_report);
		fprintf(output,"dbg2       problem_report: %d\n",problem_report);
		fprintf(output,"dbg2       make_datalistp: %d\n",make_datalistp);
		fprintf(output,"dbg2       remove_locks:   %d\n",remove_locks);
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

	/* if make_datalistp desired then make it */
	if (make_datalistp)
		{
   		/* figure out data format and fileroot if possible */
		status = mb_get_format(verbose, read_file, 
					fileroot, &format, &error);
		sprintf(file, "%sp.mb-1", fileroot);

	    	if ((fp = fopen(file,"w")) == NULL)
		    {
		    error = MB_ERROR_OPEN_FAIL;
		    fprintf(stderr, "\nUnable to open output file %s\n", file);
		    fprintf(stderr, "Program %s aborted!\n", program_name);
		    exit(error);
		    }
		fprintf(fp, "$PROCESSED\n%s %d\n", read_file, format);
		fclose(fp);
		if (verbose > 0)
		    fprintf(output, "Convenience datalist file %s created...\n", file);
		
		/* exit unless building ancilliary files has also been requested */
		if (make_inf == MB_NO)
			exit(error);
		}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose,read_file,NULL,&format,&error);

	/* if not a datalist just output filename format and weight */
	if (format > 0)
		{
		nfile++;
		
		if (make_inf == MB_YES)
		    {
		    status = mb_make_info(verbose, force_update, 
						read_file, format, &error);
		    }
		else if (problem_report == MB_YES)
		    {
		    status = mb_pr_check(verbose, read_file, &nparproblem, &ndataproblem, &error);
		    if (nparproblem + ndataproblem > 0)
			nproblemfiles++;
		    nparproblemtot += nparproblem;
		    ndataproblemtot += ndataproblem;
			
		    }
		else
		    {
		    /* check for mbinfo file if bounds checking enabled */
		    if (look_bounds == MB_YES)
			{
			status = mb_check_info(verbose, read_file, lonflip, bounds, 
				    &file_in_bounds, &error);
			if (status == MB_FAILURE)
			    {
			    file_in_bounds = MB_YES;
			    status = MB_SUCCESS;
			    error = MB_ERROR_NO_ERROR;
			    }
			}
			
		    /* ouput file if no bounds checking or in bounds */
		    if (look_bounds == MB_NO || file_in_bounds == MB_YES)
			{
			fprintf(output, "%s %d %f\n", read_file, format, file_weight);
			
			/* check status if desired */
			if (status_report == MB_YES)
			    {
			    status = mb_pr_checkstatus(verbose, read_file, &prstatus, &error);
			    if (prstatus == MB_PR_FILE_UP_TO_DATE)
				fprintf(output, "\tStatus: up to date\n");
			    else if (prstatus == MB_PR_FILE_NEEDS_PROCESSING)
				fprintf(output, "\tStatus: out of date - needs processing\n");
			    else if (prstatus == MB_PR_FILE_NOT_EXIST)
				fprintf(output, "\tStatus: file does not exist\n");
			    else if (prstatus == MB_PR_NO_PARAMETER_FILE)
				fprintf(output, "\tStatus: no parameter file - processing undefined\n");
			    }
			
			/* check locks if desired */
			if (status_report == MB_YES || remove_locks == MB_YES)
			    {
			    lock_status = mb_pr_lockinfo(verbose, read_file, &locked,
							    &lock_purpose, lock_program, lock_user, 
							    lock_cpu, lock_date, &lock_error);
			    if (locked == MB_YES && status_report == MB_YES)
				fprintf(output,"\tLocked by program <%s> run by <%s> on <%s> at <%s>\n", 
						lock_program, lock_user, lock_cpu, lock_date);
			    if (locked == MB_YES && remove_locks == MB_YES)
			        {
				sprintf(lockfile, "%s.lck", file);
				sprintf(command, "/bin/rm -f %s", lockfile);
				}
			    }
			}
		    }
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
		while ((status = mb_datalist_read(verbose,datalist,
					    file,&format,&file_weight,&error))
			    == MB_SUCCESS)
			{
			nfile++;
			getcwd(pwd, MB_PATH_MAXLINE);
			mb_get_relative_path(verbose, file, pwd, &error);
			if (make_inf == MB_YES)
			    {
			    status = mb_make_info(verbose, force_update,
							file, format, &error);
			    }
			else if (problem_report == MB_YES)
			    {
			    status = mb_pr_check(verbose, file, &nparproblem, &ndataproblem, &error);
		    	    if (nparproblem + ndataproblem > 0)
				nproblemfiles++;
		    	    nparproblemtot += nparproblem;
		    	    ndataproblemtot += ndataproblem;
			    }
			else if (copyfiles == MB_YES)
			    {
			    /* check for mbinfo file if bounds checking enabled */
			    if (look_bounds == MB_YES)
				{
				status = mb_check_info(verbose, file, lonflip, bounds, 
					    &file_in_bounds, &error);
				if (status == MB_FAILURE)
				    {
				    file_in_bounds = MB_YES;
				    status = MB_SUCCESS;
				    error = MB_ERROR_NO_ERROR;
				    }
				}
				
			    /* copy file if no bounds checking or in bounds */
			    if (look_bounds == MB_NO || file_in_bounds == MB_YES)
				{
				fprintf(output, "Copying %s %d %f\n", file, format, file_weight);
				sprintf(command, "cp %s* .", file);
				system(command);
				if ((filename = strrchr(file, '/')) != NULL)
					filename++;
				else
					filename = file;
				if (nfile == 1)
					system("rm datalist.mb-1");
			    	sprintf(command, "echo %s %d %f >> datalist.mb-1", 
					filename, format, file_weight);
				system(command);
			    	}
			    }
			else
			    {
			    /* check for mbinfo file if bounds checking enabled */
			    if (look_bounds == MB_YES)
				{
				status = mb_check_info(verbose, file, lonflip, bounds, 
					    &file_in_bounds, &error);
				if (status == MB_FAILURE)
				    {
				    file_in_bounds = MB_YES;
				    status = MB_SUCCESS;
				    error = MB_ERROR_NO_ERROR;
				    }
				}
			
			    /* ouput file if no bounds checking or in bounds */
			    if (look_bounds == MB_NO || file_in_bounds == MB_YES)
				fprintf(output, "%s %d %f\n", file, format, file_weight);

			    /* check status if desired */
			    if (status_report == MB_YES)
				{
				status = mb_pr_checkstatus(verbose, file, &prstatus, &error);
				if (prstatus == MB_PR_FILE_UP_TO_DATE)
				    fprintf(output, "\tstatus: up to date\n");
				else if (prstatus == MB_PR_FILE_NEEDS_PROCESSING)
				    fprintf(output, "\tstatus: out of date - needs processing\n");
				else if (prstatus == MB_PR_FILE_NOT_EXIST)
				    fprintf(output, "\tstatus: file does not exist\n");
				else if (prstatus == MB_PR_NO_PARAMETER_FILE)
				    fprintf(output, "\tstatus: no parameter file - processing undefined\n");
				}

			    /* check locks if desired */
			    if (status_report == MB_YES || remove_locks == MB_YES)
				{
				lock_status = mb_pr_lockinfo(verbose, file, &locked,
								&lock_purpose, lock_program, lock_user, 
								lock_cpu, lock_date, &lock_error);
				if (locked == MB_YES && status_report == MB_YES)
				    fprintf(output,"\tLocked by program <%s> run by <%s> on <%s> at <%s>\n", 
						    lock_program, lock_user, lock_cpu, lock_date);
				if (locked == MB_YES && remove_locks == MB_YES)
			            {
				    sprintf(lockfile, "%s.lck", file);
				    fprintf(output, "\tRemoving lock file %s\n", lockfile);
				    sprintf(command, "/bin/rm -f %s", lockfile);
				    system(command);
				    }
				}
			    }
			    
			}
		mb_datalist_close(verbose,&datalist,&error);
		}

	/* set program status */
	status = MB_SUCCESS;
	
	/* output counts */
	if (verbose > 0)
	    {
	    fprintf(output, "\nTotal swath files:         %d\n", nfile);
	    if (problem_report == MB_YES)
	    	{
		fprintf(output, "Total files with problems: %d\n", nproblemfiles);
		fprintf(output, "Total parameter problems:  %d\n", nparproblemtot);
		fprintf(output, "Total data problems:       %d\n", ndataproblemtot);
		}
	    }

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

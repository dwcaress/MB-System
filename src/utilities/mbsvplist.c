/*--------------------------------------------------------------------
 *    The MB-system:	mbsvplist.c	1/3/2001
 *    $Id: mbsvplist.c,v 5.6 2005-03-25 04:42:59 caress Exp $
 *
 *    Copyright (c) 2001, 2003, 2004 by
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
 * This program, mbsvplist, lists all water sound velocity
 * profiles (SVPs) within swath data files. Swath bathymetry is
 * calculated from raw angles and travel times by raytracing
 * through a model of the speed of sound in water. Many swath
 * data formats allow SVPs to be embedded in the data, and
 * often the SVPs used to calculate the data will be included.
 * By default, all unique SVPs encountered are listed to
 * stdout. The SVPs may instead be written to individual files
 * with names FILE_XXX.svp, where FILE is the swath data
 * filename and XXX is the SVP count within the file.  The -D
 * option causes duplicate SVPs to be output. The -P option
 * implies -O, and also causes the parameter file to be modified
 * so that the first svp output for each file becomes the
 * svp used for recalculating bathymetry for that swath file.
 *
 * Author:	D. W. Caress
 * Date:	January 3,  2001
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.5  2004/10/06 19:10:53  caress
 * Release 5.0.5 update.
 *
 * Revision 5.4  2003/07/02 18:14:19  caress
 * Release 5.0.0
 *
 * Revision 5.3  2003/04/17 21:18:57  caress
 * Release 5.0.beta30
 *
 * Revision 5.2  2001/07/20 00:34:38  caress
 * Release 5.0.beta03
 *
 * Revision 5.1  2001/03/22 21:15:49  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.0  2001/01/04  21:43:50  caress
 * Initial revision.
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"
#include "../../include/mb_process.h"

/* system function declarations */
char	*ctime();
char	*getenv();

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	static char rcs_id[] = "$Id: mbsvplist.c,v 5.6 2005-03-25 04:42:59 caress Exp $";
	static char program_name[] = "mbsvplist";
	static char help_message[] =  "mbsvplist lists all water sound velocity\nprofiles (SVPs) within swath data files. Swath bathymetry is\ncalculated from raw angles and travel times by raytracing\nthrough a model of the speed of sound in water. Many swath\ndata formats allow SVPs to be embedded in the data, and\noften the SVPs used to calculate the data will be included.\nBy default, all unique SVPs encountered are listed to\nstdout. The SVPs may instead be written to individual files\nwith names FILE_XXX.svp, where FILE is the swath data\nfilename and XXX is the SVP count within the file.  The -D\noption causes duplicate SVPs to be output.";
	static char usage_message[] = "mbsvplist [-D -Fformat -H -Ifile -O -P -V -Z]";
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
	char	*message;

	/* MBIO read control parameters */
	int	read_datalist = MB_NO;
	char	read_file[MB_PATH_MAXLINE];
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	double	file_weight;
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
	char	file[MB_PATH_MAXLINE];
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;

	/* MBIO read values */
	void	*mbio_ptr = NULL;
	void	*store_ptr;
	int	kind;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	altitude;
	double	sonardepth;
	char	*beamflag = NULL;
	double	*bath = NULL;
	double	*bathacrosstrack = NULL;
	double	*bathalongtrack = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*ssacrosstrack = NULL;
	double	*ssalongtrack = NULL;
	char	comment[MB_COMMENT_MAXLINE];
	int	icomment = 0;
	
	/* data record source types */
	int	nav_source;
	int	heading_source;
	int	vru_source;
	int	svp_source;
	
	/* SVP values */
	int	svp_loaded = MB_NO;
	int	svp_new = MB_NO;
	int	svp_duplicate;
	int	svp_force_zero;
	int	svp_file_output;
	int	svp_file_use;
	int	svp_count = 0;
	double	svp_time_d;
	int	svp_time_i[7];
	int	nsvp = 0;
	double	svp_depth[MB_SVP_MAX];
	double	svp_velocity[MB_SVP_MAX];
	int	nsvp_old = 0;
	double	svp_depth_old[MB_SVP_MAX];
	double	svp_velocity_old[MB_SVP_MAX];
	char	svp_file[MB_PATH_MAXLINE];
	FILE	*svp_fp;
	int	svp_read, svp_read_tot;
	int	svp_written, svp_written_tot;
	int	svp_depthzero_reset;
	double	svp_depthzero;

	time_t	right_now;
	char	date[25], user[MB_PATH_MAXLINE], *user_ptr, host[MB_PATH_MAXLINE];
	int	read_data;
	char	line[MB_PATH_MAXLINE];
	int	i, j, k;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);
	pings = 1;
	bounds[0] = -360.0;
	bounds[1] = 360.0;
	bounds[2] = -90.0;
	bounds[3] = 90.0;
	svp_duplicate = MB_NO;
	svp_file_output = MB_NO;
	svp_file_use = MB_NO;
	svp_force_zero = MB_NO;
	svp_count = 0;
	svp_read = 0;
	svp_written = 0;
	svp_read_tot = 0;
	svp_written_tot = 0;

	/* set default input to datalist.mb-1 */
	strcpy (read_file, "datalist.mb-1");

	/* process argument list */
	while ((c = getopt(argc, argv, "DdF:f:I:i:OoPpZzVvHh")) != -1)
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
		case 'D':
		case 'd':
			svp_duplicate = MB_YES;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", read_file);
			flag++;
			break;
		case 'O':
		case 'o':
			svp_file_output = MB_YES;
			break;
		case 'P':
		case 'p':
			svp_file_output = MB_YES;
			svp_file_use = MB_YES;
			break;
		case 'Z':
		case 'z':
			svp_force_zero = MB_YES;
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
		fprintf(stderr,"dbg2       format:         %d\n",format);
		fprintf(stderr,"dbg2       pings:          %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:        %d\n",lonflip);
		fprintf(stderr,"dbg2       bounds[0]:      %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:      %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:      %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:      %f\n",bounds[3]);
		fprintf(stderr,"dbg2       btime_i[0]:     %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]:     %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]:     %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]:     %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]:     %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]:     %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       btime_i[6]:     %d\n",btime_i[6]);
		fprintf(stderr,"dbg2       etime_i[0]:     %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:     %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:     %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:     %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:     %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:     %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       etime_i[6]:     %d\n",etime_i[6]);
		fprintf(stderr,"dbg2       speedmin:       %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:        %f\n",timegap);
		fprintf(stderr,"dbg2       file:           %s\n",file);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose,read_file,NULL,&format,&error);

	/* determine whether to read one file or a list of files */
	if (format < 0)
		read_datalist = MB_YES;

	/* open file list */
	if (read_datalist == MB_YES)
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
	    if (status = mb_datalist_read(verbose,datalist,
			    file,&format,&file_weight,&error)
			    == MB_SUCCESS)
		read_data = MB_YES;
	    else
		read_data = MB_NO;
	    }
	/* else copy single filename to be read */
	else
	    {
	    strcpy(file, read_file);
	    read_data = MB_YES;
	    }

	/* loop over all files to be read */
	while (read_data == MB_YES)
	{		
	/* check format and get data sources */
	if ((status = mb_format_source(verbose, &format, 
			&nav_source, &heading_source, 
			&vru_source, &svp_source, 
			&error)) == MB_FAILURE)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_format_source>:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
		
	/* initialize reading the swath file */
	if ((status = mb_read_init(
		verbose,file,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&mbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* allocate memory for data arrays */
	status = mb_malloc(verbose,beams_bath*sizeof(char),&beamflag,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),&bath,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			&bathacrosstrack,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			&bathalongtrack,&error);
	status = mb_malloc(verbose,beams_amp*sizeof(double),&amp,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&ss,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&ssacrosstrack,
			&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&ssalongtrack,
			&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
			message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* output info */
	if (verbose >= 1)
		{
		fprintf(stderr, "\nSearching %s for SVP records\n", file);
		}

	/* read and print data */
	svp_loaded = MB_NO;
	nsvp = 0;
	svp_count = 0;
	svp_read = 0;
	svp_written = 0;
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* read a data record */
		status = mb_get_all(verbose,mbio_ptr,&store_ptr,&kind,
			time_i,&time_d,&navlon,&navlat,
			&speed,&heading,
			&distance,&altitude,&sonardepth,
			&beams_bath,&beams_amp,&pixels_ss,
			beamflag,bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",
				program_name);
			fprintf(stderr,"dbg2       kind:           %d\n",kind);
			fprintf(stderr,"dbg2       error:          %d\n",error);
			fprintf(stderr,"dbg2       status:         %d\n",status);
			}
			
		/* if svp then extract data */
		if (error <= MB_ERROR_NO_ERROR
			&& kind == svp_source
			&& svp_source != MB_DATA_NONE)
			{
			/* extract svp */
			status = mb_extract_svp(verbose, mbio_ptr, store_ptr,
						&kind, &nsvp, 
						svp_depth, svp_velocity, 
						&error);
						
			/* force zero depth if requested */
			svp_depthzero_reset = MB_NO;
			if (status == MB_SUCCESS
				&& nsvp > 0
				&& svp_force_zero == MB_YES
				&& svp_depth[0] != 0.0)
				{
				svp_depthzero = svp_depth[0];
				svp_depth[0] = 0.0;
				svp_depthzero_reset = MB_YES;
				}
						
			/* check if svp is a duplicate */
			if (status == MB_SUCCESS
				&& svp_duplicate == MB_YES)
				{
				svp_loaded = MB_YES;
				svp_count++;
				}
			else if (status == MB_SUCCESS
				&& nsvp != nsvp_old)
				{
				svp_loaded = MB_YES;
				svp_count++;
				}
			else if (status == MB_SUCCESS)
				{
				for (i=0;i<nsvp;i++)
				    {
				    if (svp_loaded == MB_NO
					&& (svp_depth[i] != svp_depth_old[i]
					    || svp_velocity[i] != svp_velocity_old[i]))
					{
					svp_loaded = MB_YES;
					svp_count++;
					}
				    }
				}
				
			/* save svp */
			if (status == MB_SUCCESS)
				{
				svp_read++;
				svp_read_tot++;
				nsvp_old = nsvp;
				for (i=0;i<nsvp;i++)
				    {
				    svp_depth_old[i] = svp_depth[i];
				    svp_velocity_old[i] = svp_velocity[i];
				    }
				}
			}
			
		/* else if survey data save time */
		else if (error <= MB_ERROR_NO_ERROR
			&& kind == MB_DATA_DATA)
			{
			/* save time */
			svp_time_d = time_d;
			for (i=0;i<7;i++)
			    svp_time_i[i] = time_i[i];
			}
			
		/* if svp loaded print it out */
		if (svp_loaded == MB_YES
			&& ((error <= MB_ERROR_NO_ERROR
			    && kind == MB_DATA_DATA)
				|| error > MB_ERROR_NO_ERROR))
			{
			/* set the output */
			if (svp_file_output == MB_YES)
				{
				/* set file name */
				sprintf(svp_file, "%s_%3.3d.svp", file, svp_count);
				
				/* open the file */
				svp_fp = fopen(svp_file, "w");
				}
			else
				svp_fp = stdout;
			
			/* print out the svp */
			if (svp_fp != NULL)
				{
				/* output info */
				if (verbose >= 1)
				    {
				    fprintf(stderr, "Outputting SVP to file: %s\n", svp_file);
				    }

				/* write it out */
				fprintf(svp_fp, "## Water Sound Velocity Profile (SVP)\n");
				fprintf(svp_fp, "## Output by Program %s\n",program_name); 
				fprintf(svp_fp, "## Program Version %s\n",rcs_id);
				fprintf(svp_fp, "## MB-System Version %s\n",MB_VERSION);
				strncpy(date,"\0",25);
				right_now = time((time_t *)0);
				strncpy(date,ctime(&right_now),24);
				if ((user_ptr = getenv("USER")) == NULL)
					user_ptr = getenv("LOGNAME");
				if (user_ptr != NULL)
					strcpy(user,user_ptr);
				else
					strcpy(user, "unknown");
				gethostname(host,MB_PATH_MAXLINE);
				fprintf(svp_fp, "## Run by user <%s> on cpu <%s> at <%s>\n",
					user,host,date);
				fprintf(svp_fp, "## Swath File: %s\n",file); 
				fprintf(svp_fp, "## Start Time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
						    svp_time_i[0], svp_time_i[1], 
						    svp_time_i[2], svp_time_i[3], 
						    svp_time_i[4], svp_time_i[5], 
						    svp_time_i[6]);
				fprintf(svp_fp, "## SVP Count: %d\n", svp_count); 
				if (svp_depthzero_reset == MB_YES)
					{
					fprintf(svp_fp, "## Initial depth reset from %f to 0.0 meters\n", svp_depthzero);
					}
				if (verbose >= 1 && svp_depthzero_reset == MB_YES)
				    {
				    fprintf(stderr, "Initial depth reset from %f to 0.0 meters\n", svp_depthzero);
				    }
				fprintf(svp_fp, "## Number of SVP Points: %d\n",nsvp); 
				for (i=0;i<nsvp;i++)
					fprintf(svp_fp, "%8.2f\t%7.2f\n", 
						svp_depth[i], svp_velocity[i]);
				if (svp_file_output == MB_NO)
					{
					fprintf(svp_fp, "## \n"); 
					fprintf(svp_fp, "## \n");
					}
				svp_written++;
				svp_written_tot++;
				}
				
			/* close the svp file */
			if (svp_file_output == MB_YES
				&& svp_fp != NULL)
				fclose(svp_fp);
				
			/* if desired, set first svp output to be used for recalculating
				bathymetry */
			if (svp_file_output == MB_YES
				&& svp_file_use == MB_YES
				&& svp_count == 1)
				{
	    			status = mb_pr_update_svp(verbose, file, 
						MB_YES, svp_file, MBP_ANGLES_SNELL, MB_YES, &error);
				}

			/* reset svp flag */
			svp_loaded = MB_NO;
			}
			
		}

	/* close the swath file */
	status = mb_close(verbose,&mbio_ptr,&error);

	/* output info */
	if (verbose >= 1)
		{
		fprintf(stderr, "%d SVP records read\n", svp_read);
		fprintf(stderr, "%d SVP records written\n", svp_written);
		}

	/* deallocate memory used for data arrays */
	mb_free(verbose,&beamflag,&error); 
	mb_free(verbose,&bath,&error); 
	mb_free(verbose,&bathacrosstrack,&error); 
	mb_free(verbose,&bathalongtrack,&error); 
	mb_free(verbose,&amp,&error); 
	mb_free(verbose,&ss,&error); 
	mb_free(verbose,&ssacrosstrack,&error); 
	mb_free(verbose,&ssalongtrack,&error); 

	/* figure out whether and what to read next */
        if (read_datalist == MB_YES)
                {
		if (status = mb_datalist_read(verbose,datalist,
				    file,&format,&file_weight,&error)
				    == MB_SUCCESS)
                        read_data = MB_YES;
                else
                        read_data = MB_NO;
                }
        else
                {
                read_data = MB_NO;
                }

	/* end loop over files in list */
	}
	if (read_datalist == MB_YES)
		mb_datalist_close(verbose,&datalist,&error);

	/* output info */
	if (verbose >= 1)
		{
		fprintf(stderr, "\nTotal %d SVP records read\n", svp_read_tot);
		fprintf(stderr, "Total %d SVP records written\n", svp_written_tot);
		}

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s> completed\n",
			program_name);
		fprintf(stderr,"dbg2  Ending status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
 *    The MB-system:	mbclean.c	2/27/2003
 *    $Id: mbareaclean.c,v 5.0 2003-03-10 20:47:08 caress Exp $
 *
 *    Copyright (c) 2003 by
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
 * mbareaclean identifies and flags artifacts in swath sonar bathymetry data.  
 * The edit events are output to edit save files which can be applied
 * to the data by the program mbprocess. These are the same edit save
 * files created and/or modified by mbclean and mbedit.
 * The input data are one swath file or a datalist referencing multiple
 * swath files. An area is specified in longitude and latitude bounds,
 * along with a bin size in meters. The area is divided into a grid with
 * square cells of the specified bin size. As the data are read, each of
 * the soundings that fall within one of the bins is stored. Once all of
 * data are read, one or more statistical tests are performed on the soundings
 * within each bin, providing there are a sufficient number of soundings.
 * The user may specify one or both of the following actions:
 *   1) Previously unflagged soundings that fail a test are flagged as bad.
 *   2) Previously flagged soundings that pass all tests are unflagged.
 * If a sounding's flag status is changed, that flagging action is output
 * to the edit save file of the swath file containing that sounding. This
 * program will create edit save files if necessary, or append to those that
 * already exist.
 * 
 * Author:	D. W. Caress
 * Date:	February 27, 2003
 *		Amsterdam Airport
 *
 * $Log: not supported by cvs2svn $
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"
#include "../../include/mb_io.h"
#include "../../include/mb_swap.h"
#include "../../include/mb_process.h"

/* edit output function */
int mbareaclean_save_edit(int verbose, FILE *sofp, double time_d, int beam, 
			int action, int *error);

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	static char rcs_id[] = "$Id: mbareaclean.c,v 5.0 2003-03-10 20:47:08 caress Exp $";
	static char program_name[] = "MBAREACLEAN";
	static char help_message[] =  "MBAREACLEAN identifies and flags artifacts in swath bathymetry data";
	static char usage_message[] = "mbareaclean [-Fformat -Iinfile -Rwest/east/south/north-Sbinsize -B -G -M -Sbinsize]";
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
	void	*mbio_ptr = NULL;
	void	*store_ptr = NULL;
	int	kind;
	int	read_datalist = MB_NO;
	char	read_file[MB_PATH_MAXLINE];
	char	swathfile[MB_PATH_MAXLINE];
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	int	read_data;
	double	file_weight;
	int	format;
	int	variable_beams;
	int	traveltime;
	int	beam_flagging; 
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	altitude;
	double	sonardepth;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	char	*beamflag;
	double	*bath;
	double	*amp;
	double	*bathlon;
	double	*bathlat;
	double	*ss;
	double	*sslon;
	double	*sslat;
	char	comment[MB_COMMENT_MAXLINE];

	/* mbareaclean control parameters */
	int	median_filter = MB_NO;
	int	plane_fit = MB_NO;
	int	output_good = MB_NO;
	int	output_bad = MB_NO;
	double	areabounds[4];
	double	binsize;
	int	nx;
	int	ny;
	double	mtodeglon;
	double	mtodeglat;
	
	/* sounding atorage values and arrays */
	int	nfile = 0;
	int	nfile_alloc = 0;
	char	**filelist = NULL;
	int	*file_format = NULL;
	int	*file_flagged = NULL;
	int	*file_unflagged = NULL;
	int	nping = 0;
	int	nping_alloc = 0;
	double	*ping_time_d = NULL;
	double	*ping_altitude = NULL;
	int	nsdg = 0;
	int	nsdg_alloc = 0;
	short	*sdg_file = NULL;
	int	*sdg_ping = NULL;
	short	*sdg_beam = NULL;
	double	*sdg_time_d = NULL;
	char	*sdg_beamflag_org = NULL;
	char	*sdg_beamflag = NULL;
	double	*sdg_depth = NULL;
	double	*sdg_x = NULL;
	double	*sdg_y = NULL;
	
	int	files_tot = 0;
	int	pings_tot = 0;
	int	beams_tot = 0;
	int	beams_good_org_tot = 0;
	int	beams_flag_org_tot = 0;
	int	beams_null_org_tot = 0;
	int	beams_good_new_tot = 0;
	int	beams_flag_new_tot = 0;
	int	beams_null_new_tot = 0;
	int	pings_file = 0;
	int	beams_file = 0;
	int	beams_good_org_file = 0;
	int	beams_flag_org_file = 0;
	int	beams_null_org_file = 0;

	/* save file control variables */
	int	sofile_open = MB_NO;
	char	sifile[MB_PATH_MAXLINE];
	char	sofile[MB_PATH_MAXLINE];
	FILE	*sifp;
	FILE	*sofp;
	struct stat file_status;
	int	fstat;
	char	command[MB_PATH_MAXLINE];
	double	stime_d;
	int	sbeam;
	int	saction;
	int	neditsave;
	double	*editsave_time_d;
	int	*editsave_beam;
	int	*editsave_action;
	int	*editcount;
	int	insert;
	char	notice[MB_PATH_MAXLINE];
	int	apply;
	int	firstedit, lastedit;

	int	done;
	int	i, j, k;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* reset all defaults but the format and lonflip */
	strcpy(read_file,"datalist.mb-1");
	format = 0;
	pings = 1;
	bounds[0] = -360.;
	bounds[1] = 360.;
	bounds[2] = -90.;
	bounds[3] = 90.;
	btime_i[0] = 1962;
	btime_i[1] = 2;
	btime_i[2] = 21;
	btime_i[3] = 10;
	btime_i[4] = 30;
	btime_i[5] = 0;
	btime_i[6] = 0;
	etime_i[0] = 2062;
	etime_i[1] = 2;
	etime_i[2] = 21;
	etime_i[3] = 10;
	etime_i[4] = 30;
	etime_i[5] = 0;
	etime_i[6] = 0;
	speedmin = 0.0;
	timegap = 1000000000.0;
fprintf(stderr,"WARNING: THIS PROGRAM %s IS NOT YET FUNCTIONAL!!!\n", program_name);

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhBbGgF:f:I:i:Mm")) != -1)
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
		case 'B':
		case 'b':
			output_bad = MB_YES;
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			flag++;
			break;
		case 'G':
		case 'g':
			output_good = MB_YES;
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", read_file);
			flag++;
			break;
		case 'M':
		case 'm':
			median_filter = MB_YES;
			flag++;
			break;
		case 'R':
		case 'r':
			sscanf (optarg,"%lf/%lf/%lf/%lf", 
				&areabounds[0],
				&areabounds[1],
				&areabounds[2],
				&areabounds[3]);
			flag++;
			break;
		case 'S':
		case 's':
			sscanf (optarg,"%lf", &binsize);
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

	/* turn on median filter if nothing specified */
	if (median_filter == MB_NO
		&& plane_fit == MB_NO)
		median_filter = MB_YES;

	/* turn on output bad if nothing specified */
	if (output_bad == MB_NO
		&& output_good == MB_NO)
		output_bad = MB_YES;

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
		fprintf(stderr,"dbg2       data format:    %d\n",format);
		fprintf(stderr,"dbg2       input file:     %s\n",read_file);
		fprintf(stderr,"dbg2       median_filter:  %d\n",median_filter);
		fprintf(stderr,"dbg2       plane_fit:      %d\n",plane_fit);
		fprintf(stderr,"dbg2       output_good:    %d\n",output_good);
		fprintf(stderr,"dbg2       output_bad:     %d\n",output_bad);
		fprintf(stderr,"dbg2       areabounds[0]:  %f\n",areabounds[0]);
		fprintf(stderr,"dbg2       areabounds[1]:  %f\n",areabounds[1]);
		fprintf(stderr,"dbg2       areabounds[2]:  %f\n",areabounds[2]);
		fprintf(stderr,"dbg2       areabounds[3]:  %f\n",areabounds[3]);
		fprintf(stderr,"dbg2       binsize:        %f\n",binsize);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}
		
	/* set up arrays */

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
			    swathfile,&format,&file_weight,&error)
			    == MB_SUCCESS)
		read_data = MB_YES;
	    else
		read_data = MB_NO;
	    }
	/* else copy single filename to be read */
	else
	    {
	    strcpy(swathfile, read_file);
	    read_data = MB_YES;
	    }

	/* loop over all files to be read */
	while (read_data == MB_YES)
	{

	/* check format and get format flags */
	if ((status = mb_format_flags(verbose,&format,
			&variable_beams, &traveltime, &beam_flagging, 
			&error)) 
		!= MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_format_flags> regarding input format %d:\n%s\n",format,message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* initialize reading the input swath sonar file */
	if ((status = mb_read_init(
		verbose,swathfile,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&mbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",swathfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
		
	/* initialize and increment counting variables */
	pings_file = 0;
	beams_file = 0;

	/* give the statistics */
	if (verbose >= 0)
		{
		fprintf(stderr,"\nProcessing %s\n",swathfile);
		}

	/* allocate memory for data arrays */
	status = mb_malloc(verbose,beams_bath*sizeof(char),
			&beamflag,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			&bath,&error);
	status = mb_malloc(verbose,beams_amp*sizeof(double),
			&amp,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			&bathlon,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			&bathlat,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ss,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&sslon,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&sslat,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
		
	/* now deal with old edit save file */
	if (status == MB_SUCCESS)
	    {
	    /* check if old edit save file exists */
	    sprintf(sofile, "%s.esf", swathfile);
	    fstat = stat(sofile, &file_status);
	    if (fstat != 0
		|| (file_status.st_mode & S_IFMT) == S_IFDIR)
		{
		sprintf(sofile, "%s.mbesf", swathfile);
		fstat = stat(sofile, &file_status);
		}
	    if (fstat == 0
		&& (file_status.st_mode & S_IFMT) != S_IFDIR)
		{
		/* get temporary file name */
		sprintf(sifile, "%s.esf.tmp", swathfile);

		/* copy old edit save file to tmp file */
		sprintf(command, "cp %s %s\n", 
		    sofile, sifile);
		system(command);
    
		/* get number of old edits */
		neditsave = file_status.st_size
			     / (sizeof(double) + 2 * sizeof(int));
    
		/* allocate arrays for old edits */
		if (neditsave > 0)
		    {
		    status = mb_malloc(verbose,neditsave *sizeof(double),&editsave_time_d,&error);
		    status = mb_malloc(verbose,neditsave *sizeof(int),&editsave_beam,&error);
		    status = mb_malloc(verbose,neditsave *sizeof(int),&editsave_action,&error);
    
		    /* if error initializing memory then quit */
		    if (error != MB_ERROR_NO_ERROR)
			{
			error = MB_ERROR_NO_ERROR;
			fprintf(stderr, "\nUnable to allocate memory for %d old edit saves\n",
			    neditsave);
			neditsave = 0;
			}	
		    }	
	    
		/* open and read the old edit file */
		if (neditsave > 0
		    && (sifp = fopen(sifile,"r")) == NULL)
		    {
		    neditsave = 0;
		    fprintf(stderr, "\nUnable to copy and open old edit save file %s\n",
			sifile);
		    }
		else if (neditsave > 0)
		    {
		    /* reset message */
		    fprintf(stderr, "Sorting %d old edits...\n", neditsave);

		    error = MB_ERROR_NO_ERROR;
		    insert = 0;
		    for (i=0;i<neditsave && error == MB_ERROR_NO_ERROR;i++)
			{
			/* reset message */
			if ((i+1)%10000 == 0)
			    {
			    fprintf(stderr, "%d of %d old edits sorted...\n", i+1, neditsave);
			    }

			if (fread(&stime_d, sizeof(double), 1, sifp) != 1
			    || fread(&sbeam, sizeof(int), 1, sifp) != 1
			    || fread(&saction, sizeof(int), 1, sifp) != 1)
			    {
			    status = MB_FAILURE;
			    error = MB_ERROR_EOF;
			    }
#ifdef BYTESWAPPED
			else
			    {
			    mb_swap_double(&stime_d);
			    sbeam = mb_swap_int(sbeam);
			    saction = mb_swap_int(saction);
			    }
#endif

			/* insert into sorted array */
			if (i > 0)
			    {
			    if (stime_d < editsave_time_d[insert - 1])
				{
				for (j = insert - 1; j >= 0 && stime_d < editsave_time_d[j]; j--)
				    insert--;
				}
			    else if (stime_d >= editsave_time_d[insert - 1])
				{
				for (j = insert; j < i && stime_d >= editsave_time_d[j]; j++)
				    insert++;
				}
			    if (insert < i)
				{
				memmove(&editsave_time_d[insert+1], 
					&editsave_time_d[insert], 
					sizeof(double) * (i - insert));
				memmove(&editsave_beam[insert+1], 
					&editsave_beam[insert], 
					sizeof(int) * (i - insert));
				memmove(&editsave_action[insert+1], 
					&editsave_action[insert], 
					sizeof(int) * (i - insert));
				}
			    }
			editsave_time_d[insert] = stime_d;
			editsave_beam[insert] = sbeam;
			editsave_action[insert] = saction;
			}
		    fclose(sifp);
		    }
		}
	    }

	/* read */
	done = MB_NO;
	files_tot++;
	pings_file = 0;
	pings_file = 0;
	pings_file = 0;
	beams_file = 0;
	beams_good_org_file = 0;
	beams_flag_org_file = 0;
	beams_null_org_file = 0;
	while (done == MB_NO)
	    {
	    if (verbose > 1) fprintf(stderr,"\n");

	    /* read next record */
	    error = MB_ERROR_NO_ERROR;
	    status = mb_read(verbose,mbio_ptr,&kind,
			    &pings,time_i,&time_d,
			    &navlon,&navlat,
			    &speed,&heading,
			    &distance,&altitude,&sonardepth,
			    &beams_bath,&beams_amp,&pixels_ss,
			    beamflag,bath,amp,bathlon,bathlat,
			    ss,sslon,sslat,
			    comment,&error);
	    if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  current data status:\n");
		fprintf(stderr,"dbg2    kind:       %d\n",kind);
		fprintf(stderr,"dbg2    status:     %d\n",status);
		}
	    if (status == MB_SUCCESS && kind == MB_DATA_DATA)
		{
		/* apply saved edits */
		if (neditsave > 0)
			{
		    	/* find first and last edits for this ping */
			firstedit = 0;
		    	lastedit = firstedit - 1;
		    	for (j = firstedit; j < neditsave && time_d >= editsave_time_d[j]; j++)
				{
				if (editsave_time_d[j] == time_d)
				    {
				    if (lastedit < firstedit)
					firstedit = j;
				    lastedit = j;
				    }
				}
			
		    	/* apply relevant edits, if any, to this ping */
		    	if (lastedit > -1)
				{
				for (k=0;k<beams_bath;k++)
				    editcount[k] = 0;
				for (j=firstedit;j<=lastedit;j++)
				    {
				    editcount[editsave_beam[j]] = editsave_action[j];
				    }
				for (k=0;k<beams_bath;k++)
				    {
				    /* apply edit */
				    if (editcount[k] == MBP_EDIT_FLAG
					&& mb_beam_ok(beamflag[k]))
					{
					beamflag[k] 
					    = MB_FLAG_FLAG + MB_FLAG_MANUAL;
					}
			    	    else if (editcount[k] == MBP_EDIT_FILTER
					&& mb_beam_ok(beamflag[k]))
					{
					beamflag[k]
					    = MB_FLAG_FLAG + MB_FLAG_FILTER;
					}
			    	    else if (editcount[k] == MBP_EDIT_UNFLAG
					&& !mb_beam_ok(beamflag[k]))
					{
					beamflag[k] = MB_FLAG_NONE;
					}
				    else if (editcount[k] == MBP_EDIT_ZERO
					&& beamflag[k] != MB_FLAG_NULL)
					{
					beamflag[k] = MB_FLAG_NULL;
					}
				    }
				}
			}
		
		/* update counters */
		pings_tot++;
		beams_tot += beams_bath;
		pings_file++;
		beams_file += beams_bath;
		for (i=0;i<beams_bath;i++)
			{
			if (mb_beam_ok(beamflag[i]))
				{
				beams_good_org_tot++;
				beams_good_org_file++;
				}
			else if (beamflag[i] == MB_FLAG_NULL)
				{
				beams_null_org_tot++;
				beams_null_org_file++;
				}
			else
				{
				beams_flag_org_tot++;
				beams_flag_org_file++;
				}
			}
		}
	    else if (error > MB_ERROR_NO_ERROR)
		{
		done = MB_YES;
		}
		    
	    /* process a record */

	    /* reset counters and data */
	    }

	/* close the files */
	status = mb_close(verbose,&mbio_ptr,&error);
	if (sofile_open == MB_YES)
	    {
	    /* close edit save file */
	    fclose(sofp);
	    sofile_open = MB_NO;
	    
	    /* update mbprocess parameter file */
	    status = mb_pr_update_format(verbose, swathfile, 
			MB_YES, format, 
			&error);
	    status = mb_pr_update_edit(verbose, swathfile, 
			MBP_EDIT_ON, sofile, 
			&error);
	    }

	/* free the memory */
	if (neditsave > 0)
	    {
	    mb_free(verbose,&editsave_time_d,&error);
	    mb_free(verbose,&editsave_beam,&error);
	    mb_free(verbose,&editsave_action,&error);
	    }
	mb_free(verbose,&beamflag,&error); 
	mb_free(verbose,&bath,&error); 
	mb_free(verbose,&amp,&error); 
	mb_free(verbose,&bathlon,&error); 
	mb_free(verbose,&bathlat,&error); 
	mb_free(verbose,&ss,&error); 
	mb_free(verbose,&sslon,&error); 
	mb_free(verbose,&sslat,&error); 

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* give the statistics */
	if (verbose >= 0)
		{
		fprintf(stderr,"%d pings read in %s\n",pings_file,swathfile);
		fprintf(stderr,"%d good beams\n",beams_good_org_file);
		fprintf(stderr,"%d flagged beams\n",beams_flag_org_file);
		fprintf(stderr,"%d null beams\n",beams_null_org_file);
		}

	/* figure out whether and what to read next */
        if (read_datalist == MB_YES)
                {
		if (status = mb_datalist_read(verbose,datalist,
			    swathfile,&format,&file_weight,&error)
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

	/* give the total statistics */
	if (verbose >= 0)
		{
		fprintf(stderr,"\nMBareaclean Processing Totals:\n");
		fprintf(stderr,"-------------------------\n");
		fprintf(stderr,"%d total swath data files processed\n",files_tot);
		fprintf(stderr,"%d total bathymetry data records processed\n",pings_tot);
		fprintf(stderr,"%d total bathymetry soundings processed\n",beams_tot);
		}

	/* set program status */
	status = MB_SUCCESS;

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
int mbclean_save_edit(int verbose, FILE *sofp, double time_d, int beam, int action, int *error)
{
	/* local variables */
	char	*function_name = "mbclean_save_edit";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
	
		fprintf(stderr,"dbg2       sofp:            %d\n",sofp);
		fprintf(stderr,"dbg2       time_d:          %f\n",time_d);
		fprintf(stderr,"dbg2       beam:            %d\n",beam);
		fprintf(stderr,"dbg2       action:          %d\n",action);
		}
		
	/* write out the edit */
	if (sofp != NULL)
	    {		
#ifdef BYTESWAPPED
	    mb_swap_double(&time_d);
	    beam = mb_swap_int(beam);
	    action = mb_swap_int(action);
#endif
	    if (fwrite(&time_d, sizeof(double), 1, sofp) != 1)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	    if (status == MB_SUCCESS
		&& fwrite(&beam, sizeof(int), 1, sofp) != 1)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	    if (status == MB_SUCCESS
		&& fwrite(&action, sizeof(int), 1, sofp) != 1)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/


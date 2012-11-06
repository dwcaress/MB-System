/*--------------------------------------------------------------------
 *    The MB-system:	mbeditviz_prog.c		5/1/2007
 *    $Id$
 *
 *    Copyright (c) 2007-2012 by
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
 *
 * MBeditviz is an interactive swath bathymetry editor and patch
 * test tool for  MB-System.
 * It can work with any data format supported by the MBIO library.
 * This include file contains global control parameters shared with
 * the Motif interface code.
 *
 * Author:	D. W. Caress
 * Date:	May 1, 2007
 *
 * $Log: mbeditviz_prog.c,v $
 * Revision 5.9  2008/11/16 21:51:18  caress
 * Updating all recent changes, including time lag analysis using mbeditviz and improvements to the mbgrid footprint gridding algorithm.
 *
 * Revision 5.8  2008/05/16 22:59:42  caress
 * Release 5.1.1beta18.
 *
 * Revision 5.7  2008/03/14 19:04:32  caress
 * Fixed memory problems with route editing.
 *
 * Revision 5.6  2008/01/14 18:20:13  caress
 * Improved ability to identify raw vs processed data files regardless of source datalist.
 *
 * Revision 5.5  2007/11/16 17:26:56  caress
 * Progress on MBeditviz
 *
 * Revision 5.4  2007/10/17 20:35:05  caress
 * Release 5.1.1beta11
 *
 * Revision 5.3  2007/10/08 16:32:08  caress
 * Code status as of 8 October 2007.
 *
 * Revision 5.2  2007/07/05 19:53:37  caress
 * Added sys/stat.h include.
 *
 * Revision 5.1  2007/07/03 17:35:54  caress
 * Working on MBeditviz.
 *
 * Revision 5.0  2007/06/17 23:25:57  caress
 * Added NBeditviz.
 *
 *
 */

/* Standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"
#include "../../include/mb_format.h"
#include "../../include/mb_aux.h"
#include "../../include/mbsys_singlebeam.h"

/* GMT include files */
#include "gmt.h"

/* mbview include file */
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include "mbview.h"
#include "mbeditviz.h"

/* id variables */
static char rcs_id[] = "$Id$";
static char program_name[] = "MBeditviz";
static char help_message[] = "MBeditviz is a bathymetry editor and patch test tool.";
static char usage_message[] = "mbeditviz [-H -T -V]";


/* status variables */
char	*error_message;
char	message[MB_PATH_MAXLINE];
char	error1[MB_PATH_MAXLINE];
char	error2[MB_PATH_MAXLINE];
char	error3[MB_PATH_MAXLINE];

/* data file parameters */
void	*datalist;

/* MBIO control parameters */
int	mbdef_pings;
int	mbdef_format;
int	mbdef_lonflip;
double	mbdef_bounds[4];
int	mbdef_btime_i[7];
int	mbdef_etime_i[7];
double	mbdef_btime_d;
double	mbdef_etime_d;
double	mbdef_speedmin;
double	mbdef_timegap;
int	mbdef_uselockfiles;

/*--------------------------------------------------------------------*/
int mbeditviz_init(int argc,char **argv)
{
	/* local variables */
	char	*function_name = "mbeditviz_init";
	int	fileflag = 0;
	char	ifile[MB_PATH_MAXLINE];
	int	i;

	/* parsing variables */
	extern char *optarg;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* set default global control parameters */
	mbev_status = MB_SUCCESS;
	mbev_error = MB_ERROR_NO_ERROR;
	mbev_verbose = 0;

	mbev_mode_output = MBEV_OUTPUT_MODE_EDIT;
	mbev_grid_algorithm = MBEV_GRID_ALGORITH_FOOTPRINT;
	mbev_num_files = 0;
	mbev_num_files_alloc = 0;
	mbev_num_files_loaded = 0;
	mbev_num_pings_loaded = 0;
	mbev_num_soundings_loaded = 0;
	for (i=0;i<4;i++)
		{
		mbev_bounds[i] = 0.0;
		}
	mbev_files = NULL;
	mbev_grid.status = MBEV_GRID_NONE;
	mbev_grid.projection_id[0] = 0;
	for (i=0;i<4;i++)
		{
		mbev_grid.bounds[i] = 0.0;
		mbev_grid.boundsutm[i] = 0.0;
		}
	mbev_grid.dx = 0.0;
	mbev_grid.dy = 0.0;
	mbev_grid.nx = 0;
	mbev_grid.ny = 0;
	mbev_grid.min = 0.0;
	mbev_grid.max = 0.0;
	mbev_grid.smin = 0.0;
	mbev_grid.smax = 0.0;
	mbev_grid.nodatavalue = 0.0;
	mbev_grid.sum = NULL;
	mbev_grid.wgt = NULL;
	mbev_grid.val = NULL;
	mbev_grid.sgm = NULL;
	for (i=0;i<4;i++)
		{
		mbev_grid_bounds[i] = 0.0;
		mbev_grid_boundsutm[i] = 0.0;
		}
	mbev_grid_cellsize = 0.0;
	mbev_grid_nx = 0;
	mbev_grid_ny = 0;
	mbev_selected.xorigin = 0.0;
	mbev_selected.yorigin = 0.0;
	mbev_selected.zorigin = 0.0;
	mbev_selected.bearing = 0.0;
	mbev_selected.xmin = 0.0;
	mbev_selected.ymin = 0.0;
	mbev_selected.zmin = 0.0;
	mbev_selected.xmax = 0.0;
	mbev_selected.ymax = 0.0;
	mbev_selected.zmax = 0.0;
	mbev_selected.sinbearing = 0.0;
	mbev_selected.cosbearing = 0.0;
	mbev_selected.scale = 0.0;
	mbev_selected.zscale = 0.0;
	mbev_selected.num_soundings = 0;
	mbev_selected.num_soundings_unflagged = 0;
	mbev_selected.num_soundings_flagged = 0;
	mbev_selected.num_soundings_alloc = 0;
	mbev_selected.soundings = NULL;
	mbev_rollbias = 0.0;
	mbev_pitchbias = 0.0;
	mbev_headingbias = 0.0;
	mbev_timelag = 0.0;

	/* set mbio default values */
	mb_lonflip(mbev_verbose,&mbdef_lonflip);
	mb_uselockfiles(mbev_verbose,&mbdef_uselockfiles);
	mbdef_pings = 1;
	mbdef_format = 0;
	mbdef_bounds[0] = -360.;
	mbdef_bounds[1] = 360.;
	mbdef_bounds[2] = -90.;
	mbdef_bounds[3] = 90.;
	mbdef_btime_i[0] = 1962;
	mbdef_btime_i[1] = 2;
	mbdef_btime_i[2] = 21;
	mbdef_btime_i[3] = 10;
	mbdef_btime_i[4] = 30;
	mbdef_btime_i[5] = 0;
	mbdef_btime_i[6] = 0;
	mbdef_etime_i[0] = 2062;
	mbdef_etime_i[1] = 2;
	mbdef_etime_i[2] = 21;
	mbdef_etime_i[3] = 10;
	mbdef_etime_i[4] = 30;
	mbdef_etime_i[5] = 0;
	mbdef_etime_i[6] = 0;
	mbdef_speedmin = 0.0;
	mbdef_timegap = 1000000000.0;

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhF:f:GgI:i:")) != -1)
	  switch (c)
		{
		case 'H':
		case 'h':
			help++;
			break;
		case 'V':
		case 'v':
			mbev_verbose++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &mbdef_format);
			flag++;
			break;
		case 'G':
		case 'g':
			mbev_grid_algorithm = MBEV_GRID_ALGORITH_SIMPLE;
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", ifile);
			flag++;
			fileflag++;
			mbev_status = mbeditviz_open_data(ifile, mbdef_format);
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
		mbev_error = MB_ERROR_BAD_USAGE;
		exit(mbev_error);
		}

	/* print starting message */
	if (mbev_verbose == 1 || help)
		{
		fprintf(stderr,"\nProgram %s\n",program_name);
		fprintf(stderr,"Version %s\n",rcs_id);
		fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Control Parameters:\n");
		fprintf(stderr,"dbg2       mbev_verbose:    %d\n",mbev_verbose);
		fprintf(stderr,"dbg2       help:            %d\n",help);
		fprintf(stderr,"dbg2       input file:      %s\n",ifile);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(mbev_error);
		}

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       argc:      %d\n",argc);
		for (i=0;i<argc;i++)
			fprintf(stderr,"dbg2       argv[%d]:    %s\n",
				i,argv[i]);
		}

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBeditviz function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:        %d\n",mbev_error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status:  %d\n",mbev_status);
		}

	/* return */
	return(mbev_status);
}
/*--------------------------------------------------------------------*/
int mbeditviz_get_format(char *file, int *form)
{
	/* local variables */
	char	*function_name = "mbedit_get_format";
	char	tmp[MB_PATH_MAXLINE];
	int	tform;

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       file:        %s\n",file);
		fprintf(stderr,"dbg2       format:      %d\n",*form);
		}

	/* get filenames */
	/* look for MB suffix convention */
	if ((mbev_status = mb_get_format(mbev_verbose, file, tmp,
				    &tform, &mbev_error))
				    == MB_SUCCESS)
	    {
	    *form = tform;
	    }

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       format:      %d\n",*form);
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status: %d\n",mbev_status);
		}

	/* return */
	return(mbev_status);
}
/*--------------------------------------------------------------------*/
int mbeditviz_open_data(char *path, int format)
{
	/* local variables */
	char	*function_name = "mbeditviz_open_data";
	int	done;
	double	weight;
	int	filestatus;
	char	fileraw[MB_PATH_MAXLINE];
	char	fileprocessed[MB_PATH_MAXLINE];

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       file:        %s\n",path);
		fprintf(stderr,"dbg2       format:      %d\n",format);
		}

	/* get format if required */
	if (format == 0)
		mb_get_format(mbev_verbose,path,NULL,&format,&mbev_error);

	/* loop until all inf files are read */
	done = MB_NO;
	while (done == MB_NO)
		{
		if (format > 0)
			{
			mbev_status = mbeditviz_import_file(path,format);
			done = MB_YES;
			}
		else if (format == -1)
			{
			if ((mbev_status = mb_datalist_open(mbev_verbose,&datalist,
							path,MB_DATALIST_LOOK_NO,&mbev_error)) == MB_SUCCESS)
				{
				while (done == MB_NO)
					{
					if ((mbev_status = mb_datalist_read2(mbev_verbose,datalist,
							&filestatus,fileraw,fileprocessed,&format,&weight,&mbev_error))
							== MB_SUCCESS)
						{
						mbev_status = mbeditviz_import_file(fileraw,format);
						}
					else
						{
						mbev_status = mb_datalist_close(mbev_verbose,&datalist,&mbev_error);
						done = MB_YES;
						}
					}
				}
			}
		}
	do_mbeditviz_message_off();
	do_mbeditviz_update_gui();

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status: %d\n",mbev_status);
		}

	/* return */
	return(mbev_status);
}
/*--------------------------------------------------------------------*/
int mbeditviz_import_file(char *path, int format)
{
	/* local variables */
	char	*function_name = "mbeditviz_import_file";
	char	*root;
	struct mbev_file_struct *file;
	struct stat file_status;
	int	fstatus;

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       path:        %s\n",path);
		fprintf(stderr,"dbg2       format:      %d\n",format);
		}

	/* turn on message */
	root = (char *) strrchr(path, '/');
	if (root == NULL)
		root = path;
	else
		root++;
	sprintf(message,"Importing format %d data from %s",format,root);
	do_mbeditviz_message_on(message);

	/* allocate mbpr_file_struct array if needed */
	mbev_status = MB_SUCCESS;
	if (mbev_num_files_alloc <= mbev_num_files)
		{
		mbev_files = (struct mbev_file_struct *) realloc(mbev_files,
				sizeof(struct mbev_file_struct) * (mbev_num_files_alloc + MBEV_ALLOC_NUM));
		if (mbev_files != NULL)
			mbev_num_files_alloc += MBEV_ALLOC_NUM;
		else
			{
			mbev_status = MB_FAILURE;
			mbev_error = MB_ERROR_MEMORY_FAIL;
			}
		}

	/* set new file structure */
	if (mbev_status == MB_SUCCESS)
		{
		file = &(mbev_files[mbev_num_files]);
		file->load_status = MB_NO;
		file->load_status_shown = MB_NO;
		file->locked = MB_NO;
		file->esf_exists = MB_NO;
		strcpy(file->path, path);
		strcpy(file->name, root);
		file->format = format;
		file->raw_info_loaded = MB_NO;
		file->esf_open = MB_NO;
		file->n_async_heading = 0;
		file->n_async_heading_alloc = 0;
		file->async_heading_time_d = NULL;
		file->async_heading_heading = NULL;
		file->n_async_attitude = 0;
		file->n_async_attitude_alloc = 0;
		file->async_attitude_time_d = NULL;
		file->async_attitude_roll = NULL;
		file->async_attitude_pitch = NULL;
		file->n_sync_attitude = 0;
		file->n_sync_attitude_alloc = 0;
		file->sync_attitude_time_d = NULL;
		file->sync_attitude_roll = NULL;
		file->sync_attitude_pitch = NULL;

		/* load info */
		mbev_status = mb_get_info(mbev_verbose, file->path, &(file->raw_info), mbdef_lonflip, &mbev_error);
		if (mbev_status == MB_SUCCESS)
			{
			file->raw_info_loaded = MB_YES;
			mbev_num_files++;
			}

		/* load processing parameters */
		if (mbev_status == MB_SUCCESS)
			{
			mbev_status = mb_pr_readpar(mbev_verbose, file->path, MB_NO, &(file->process), &mbev_error);
			if (file->process.mbp_format_specified == MB_NO)
				{
				file->process.mbp_format_specified = MB_YES;
				file->process.mbp_format = file->format;
				}
			}

		/* load processed file info */
		if (mbev_status == MB_SUCCESS)
			{
			if ((fstatus = stat(file->process.mbp_ofile, &file_status)) == 0
				&& (file_status.st_mode & S_IFMT) != S_IFDIR)
				{
				mbev_status = mb_get_info(mbev_verbose, file->process.mbp_ofile,
							&(file->processed_info),mbdef_lonflip, &mbev_error);
				if (mbev_status == MB_SUCCESS)
					file->processed_info_loaded = MB_YES;
				}
			}
		}

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status: %d\n",mbev_status);
		}

	/* return */
	return(mbev_status);
}
/*--------------------------------------------------------------------*/
int mbeditviz_load_file(int ifile)
{
	/* local variables */
	char	*function_name = "mbeditviz_load_file";
	struct mbev_file_struct *file;
	struct mbev_ping_struct *ping;
	mb_path	swathfile;
	struct stat file_status;
	int	fstatus;
	FILE	*afp;
	mb_path	asyncfile;
	mb_path	geffile;
	char	buffer[MBP_FILENAMESIZE], *result;
	char	command[MBP_FILENAMESIZE];
	int	nread;

	mb_path	error1;
	mb_path	error2;
	mb_path	error3;

	/* swath file locking variables */
	int	lock_status;
	int	locked;
	int	lock_purpose;
	mb_path	lock_program;
	mb_path lock_cpu;
	mb_path lock_user;
	char	lock_date[25];

	/* mbio read and write values */
	int	format;
	void	*imbio_ptr = NULL;
	struct mb_io_struct *imb_io_ptr = NULL;
	void	*istore_ptr = NULL;
	int	kind;
	double	draft;
	int	beams_bath;
	int	nbeams;
	int	beams_amp;
	int	pixels_ss;
	char	*beamflag = NULL;
	double	*bath = NULL;
	double	*bathacrosstrack = NULL;
	double	*bathalongtrack = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*ssacrosstrack = NULL;
	double	*ssalongtrack = NULL;
	char	comment[MB_COMMENT_MAXLINE];

	int	rawmodtime = 0;
	int	gefmodtime = 0;

	double	mtodeglon, mtodeglat;
	double	heading, sonardepth;
	double	headingx, headingy;
	double	rolldelta, pitchdelta;
	int	swathbounds;
	int	icenter, iport, istbd;
	double	centerdistance, portdistance, stbddistance;
	int	iping, ibeam;

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       ifile:       %d\n",ifile);
		}

	/* lock the file if it needs loading */
	mbev_status = MB_SUCCESS;
	mbev_error = MB_ERROR_NO_ERROR;
	if (ifile >= 0 && ifile < mbev_num_files
		&& mbev_files[ifile].load_status == MB_NO)
		{
		file = &(mbev_files[ifile]);

		/* try to lock file */
		if (mbdef_uselockfiles == MB_YES)
			{
			mbev_status = mb_pr_lockswathfile(mbev_verbose, file->path,
					MBP_LOCK_EDITBATHY, program_name, &mbev_error);
			}
		else
			{
			mbev_status = mb_pr_lockinfo(mbev_verbose, file->path, &locked,
					&lock_purpose, lock_program, lock_user, lock_cpu,
					lock_date, &mbev_error);

			/* if locked get lock info */
			if (mbev_error == MB_ERROR_FILE_LOCKED)
				{
				fprintf(stderr, "\nFile %s locked but lock ignored\n", file->path);
				fprintf(stderr, "File locked by <%s> running <%s>\n", lock_user, lock_program);
				fprintf(stderr, "on cpu <%s> at <%s>\n", lock_cpu, lock_date);
				mbev_error = MB_ERROR_NO_ERROR;
				mbev_status = MB_SUCCESS;
				}
			}

		/* if locked let the user know file can't be opened */
		if (mbev_status == MB_FAILURE)
			{
			/* turn off message */
			do_mbeditviz_message_off();

			/* if locked get lock info */
			if (mbev_error == MB_ERROR_FILE_LOCKED)
				{
				lock_status = mb_pr_lockinfo(mbev_verbose, file->path, &locked,
						&lock_purpose, lock_program, lock_user, lock_cpu,
						lock_date, &mbev_error);

				sprintf(error1, "Unable to open input file:");
				sprintf(error2, "File locked by <%s> running <%s>", lock_user, lock_program);
				sprintf(error3, "on cpu <%s> at <%s>", lock_cpu, lock_date);
				fprintf(stderr, "\nUnable to open input file:\n");
				fprintf(stderr, "  %s\n", file->path);
				fprintf(stderr, "File locked by <%s> running <%s>\n", lock_user, lock_program);
				fprintf(stderr, "on cpu <%s> at <%s>\n", lock_cpu, lock_date);
				}

			/* else if unable to create lock file there is a permissions problem */
			else if (mbev_error == MB_ERROR_OPEN_FAIL)
				{
				sprintf(error1, "Unable to create lock file");
				sprintf(error2, "for intended input file:");
				sprintf(error3, "-Likely permissions issue");
				fprintf(stderr, "Unable to create lock file\n");
				fprintf(stderr, "for intended input file:\n");
				fprintf(stderr, "  %s\n", file->path);
				fprintf(stderr, "-Likely permissions issue\n");
				}

			/* put up error dialog */
			do_error_dialog(error1,error2, error3);
			}
		}

	/* load the file if it needs loading and has been locked */
	if (mbev_status == MB_SUCCESS
		&& ifile >= 0 && ifile < mbev_num_files
		&& mbev_files[ifile].load_status == MB_NO)
		{
		file = &(mbev_files[ifile]);

		/* allocate memory for pings */
		if (file->raw_info.nrecords > 0)
			{
			file->pings = (struct mbev_ping_struct *)
				malloc(sizeof(struct mbev_ping_struct) * (file->raw_info.nrecords + 1));
			if (mbev_files != NULL)
				{
				file->num_pings_alloc = file->raw_info.nrecords + 1;
				memset(file->pings,0,sizeof(struct mbev_ping_struct) * (file->num_pings_alloc));
				file->num_pings = 0;
				}
			else
				{
				file->num_pings_alloc = 0;
				file->num_pings = 0;
				mbev_status = MB_FAILURE;
				mbev_error = MB_ERROR_MEMORY_FAIL;
				}
			}

		/* open the file for reading */
		if (mbev_status == MB_SUCCESS)
			{
			/* read processed file if available, raw otherwise (fbt if possible) */
			if (file->processed_info_loaded == MB_YES)
				strcpy(swathfile, file->process.mbp_ofile);
			else
				strcpy(swathfile, file->path);
			format = file->format;
			file->esf_open = MB_NO;
			mb_get_shortest_path(mbev_verbose, swathfile, &mbev_error);

			/* use fbt file if possible */
			mb_get_fbt(mbev_verbose, swathfile, &format, &mbev_error);

			/* initialize reading the swath file */
			if ((mbev_status = mb_read_init(
				mbev_verbose,swathfile,format,mbdef_pings,mbdef_lonflip,mbdef_bounds,
				mbdef_btime_i,mbdef_etime_i,mbdef_speedmin,mbdef_timegap,
				&imbio_ptr,&mbdef_btime_d,&mbdef_etime_d,
				&beams_bath,&beams_amp,&pixels_ss,&mbev_error)) != MB_SUCCESS)
				{
				mb_error(mbev_verbose,mbev_error,&error_message);
				fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",error_message);
				fprintf(stderr,"\nSwath sonar File <%s> not initialized for reading\n",file->path);
				}
			}

		/* allocate memory for data arrays */
		if (mbev_status == MB_SUCCESS)
			{
			beamflag = NULL;
			bath = NULL;
			amp = NULL;
			bathacrosstrack = NULL;
			bathalongtrack = NULL;
			ss = NULL;
			ssacrosstrack = NULL;
			ssalongtrack = NULL;
			if (mbev_error == MB_ERROR_NO_ERROR)
				mbev_status = mb_register_array(mbev_verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
								sizeof(char), (void **)&beamflag, &mbev_error);
			if (mbev_error == MB_ERROR_NO_ERROR)
				mbev_status = mb_register_array(mbev_verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
								sizeof(double), (void **)&bath, &mbev_error);
			if (mbev_error == MB_ERROR_NO_ERROR)
				mbev_status = mb_register_array(mbev_verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE,
								sizeof(double), (void **)&amp, &mbev_error);
			if (mbev_error == MB_ERROR_NO_ERROR)
				mbev_status = mb_register_array(mbev_verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
								sizeof(double), (void **)&bathacrosstrack, &mbev_error);
			if (mbev_error == MB_ERROR_NO_ERROR)
				mbev_status = mb_register_array(mbev_verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
								sizeof(double), (void **)&bathalongtrack, &mbev_error);
			if (mbev_error == MB_ERROR_NO_ERROR)
				mbev_status = mb_register_array(mbev_verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
								sizeof(double), (void **)&ss, &mbev_error);
			if (mbev_error == MB_ERROR_NO_ERROR)
				mbev_status = mb_register_array(mbev_verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
								sizeof(double), (void **)&ssacrosstrack, &mbev_error);
			if (mbev_error == MB_ERROR_NO_ERROR)
				mbev_status = mb_register_array(mbev_verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
								sizeof(double), (void **)&ssalongtrack, &mbev_error);

			/* if error initializing memory then don't read the file */
			if (mbev_error != MB_ERROR_NO_ERROR)
				{
				mb_error(mbev_verbose,mbev_error,&error_message);
				fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
					error_message);
				}
 			}

		/* set the beamwidths */
		imb_io_ptr = (struct mb_io_struct *) imbio_ptr;
		file->beamwidth_xtrack = imb_io_ptr->beamwidth_xtrack;
		file->beamwidth_ltrack = imb_io_ptr->beamwidth_ltrack;

		/* read the data */
		if (mbev_status == MB_SUCCESS)
			{
			file->num_pings = 0;
			while (mbev_error <= MB_ERROR_NO_ERROR)
				{
				/* get pointer to next ping */
				ping = &(file->pings[file->num_pings]);

				/* read a ping of data */
				mbev_status = mb_get_all(mbev_verbose,imbio_ptr,&istore_ptr,&kind,
					ping->time_i,&ping->time_d,&ping->navlon,&ping->navlat,&ping->speed,
					&ping->heading,&ping->distance,&ping->altitude,&ping->sonardepth,
					&ping->beams_bath,&beams_amp,&pixels_ss,
					beamflag,bath,amp,bathacrosstrack,bathalongtrack,
					ss,ssacrosstrack,ssalongtrack,
					comment,&mbev_error);

				/* ignore minor errors */
				if (kind == MB_DATA_DATA
					&& (mbev_error == MB_ERROR_TIME_GAP
						|| mbev_error == MB_ERROR_OUT_BOUNDS
						|| mbev_error == MB_ERROR_OUT_TIME
						|| mbev_error == MB_ERROR_SPEED_TOO_SMALL))
					{
					mbev_status = MB_SUCCESS;
					mbev_error = MB_ERROR_NO_ERROR;
					}

				/* check for multiplicity of pings with the same time stamp */
				if (mbev_error == MB_ERROR_NO_ERROR
				    && kind == MB_DATA_DATA)
					{
					if (file->num_pings > 0 && ping->time_d == file->pings[file->num_pings-1].time_d)
						{
						ping->multiplicity = file->pings[file->num_pings-1].multiplicity + 1;
						}
					else
						{
						ping->multiplicity = 0;
						}
					}

				/* allocate memory for pings */
				if (mbev_error == MB_ERROR_NO_ERROR
				    && kind == MB_DATA_DATA)
					{
					if ((ping->beamflag = (char *) malloc(ping->beams_bath)) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
					if ((ping->beamflagorg = (char *) malloc(ping->beams_bath)) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
					if ((ping->bath = (double *) malloc(sizeof(double) * (ping->beams_bath))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
					if ((ping->bathacrosstrack = (double *) malloc(sizeof(double) * (ping->beams_bath))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
					if ((ping->bathalongtrack = (double *) malloc(sizeof(double) * (ping->beams_bath))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
					if ((ping->bathcorr = (double *) malloc(sizeof(double) * (ping->beams_bath))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
					if ((ping->bathlon = (double *) malloc(sizeof(double) * (ping->beams_bath))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
					if ((ping->bathlat = (double *) malloc(sizeof(double) * (ping->beams_bath))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
					if ((ping->bathx = (double *) malloc(sizeof(double) * (ping->beams_bath))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
					if ((ping->bathy = (double *) malloc(sizeof(double) * (ping->beams_bath))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
					if ((ping->angles = (double *) malloc(sizeof(double) * (ping->beams_bath))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
					if ((ping->angles_forward = (double *) malloc(sizeof(double) * (ping->beams_bath))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
					if ((ping->angles_null = (double *) malloc(sizeof(double) * (ping->beams_bath))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
					if ((ping->ttimes = (double *) malloc(sizeof(double) * (ping->beams_bath))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
					if ((ping->bheave = (double *) malloc(sizeof(double) * (ping->beams_bath))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
					if ((ping->alongtrack_offset = (double *) malloc(sizeof(double) * (ping->beams_bath))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
					if (mbev_error == MB_ERROR_MEMORY_FAIL)
						{
fprintf(stderr,"MEMORY FAILURE in mbeditviz_load_file\n");
						mbev_status = MB_FAILURE;
						if (ping->beamflag != NULL)
							{
							free(ping->beamflag);
							ping->beamflag = NULL;
							}
						if (ping->beamflagorg != NULL)
							{
							free(ping->beamflagorg);
							ping->beamflagorg = NULL;
							}
						if (ping->bath != NULL)
							{
							free(ping->bath);
							ping->bath = NULL;
							}
						if (ping->bathacrosstrack != NULL)
							{
							free(ping->bathacrosstrack);
							ping->bathacrosstrack = NULL;
							}
						if (ping->bathalongtrack != NULL)
							{
							free(ping->bathalongtrack);
							ping->bathalongtrack = NULL;
							}
						if (ping->bathcorr != NULL)
							{
							free(ping->bathcorr);
							ping->bathcorr = NULL;
							}
						if (ping->bathlon != NULL)
							{
							free(ping->bathlon);
							ping->bathlon = NULL;
							}
						if (ping->bathlat != NULL)
							{
							free(ping->bathlat);
							ping->bathlat = NULL;
							}
						if (ping->bathx != NULL)
							{
							free(ping->bathx);
							ping->bathx = NULL;
							}
						if (ping->bathy != NULL)
							{
							free(ping->bathy);
							ping->bathy = NULL;
							}
						if (ping->angles != NULL)
							{
							free(ping->angles);
							ping->angles = NULL;
							}
						if (ping->angles_forward != NULL)
							{
							free(ping->angles_forward);
							ping->angles_forward = NULL;
							}
						if (ping->angles_null != NULL)
							{
							free(ping->angles_null);
							ping->angles_null = NULL;
							}
						if (ping->ttimes != NULL)
							{
							free(ping->ttimes);
							ping->ttimes = NULL;
							}
						if (ping->bheave != NULL)
							{
							free(ping->bheave);
							ping->bheave = NULL;
							}
						if (ping->alongtrack_offset != NULL)
							{
							free(ping->alongtrack_offset);
							ping->alongtrack_offset = NULL;
							}
						}
					}

				/* copy data into ping arrays */
				if (mbev_error == MB_ERROR_NO_ERROR
				    && kind == MB_DATA_DATA)
					{
					mbeditviz_apply_timelag(file, ping,
								mbev_rollbias, mbev_pitchbias, mbev_headingbias, mbev_timelag,
								&heading, &sonardepth,
								&rolldelta, &pitchdelta);
					mb_coor_scale(mbev_verbose,ping->navlat,&mtodeglon,&mtodeglat);
					headingx = sin(heading * DTR);
					headingy = cos(heading * DTR);

					for (ibeam=0;ibeam<ping->beams_bath;ibeam++)
						{
						ping->beamflag[ibeam] = beamflag[ibeam];
						ping->beamflagorg[ibeam] = beamflag[ibeam];
						if (ping->beamflag[ibeam] != MB_FLAG_NULL
							&& (isnan(bath[ibeam] || isnan(bathacrosstrack[ibeam] || isnan(bathalongtrack[ibeam])))))
							{
							ping->beamflag[ibeam] = MB_FLAG_NULL;
fprintf(stderr,"\nEncountered NaN value in swath data from file: %s\n",swathfile);
fprintf(stderr,"     Ping time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
	ping->time_i[0],ping->time_i[1],ping->time_i[2],ping->time_i[3],ping->time_i[4],ping->time_i[5],ping->time_i[6]);
fprintf(stderr,"     Beam bathymetry: %d %f %f %f\n",ibeam,ping->bath[ibeam],ping->bathacrosstrack[ibeam],ping->bathalongtrack[ibeam]);
							}
						if (ping->beamflag[ibeam] != MB_FLAG_NULL)
							{
							/* copy bath */
							ping->bath[ibeam] = bath[ibeam];
							ping->bathacrosstrack[ibeam] = bathacrosstrack[ibeam];
							ping->bathalongtrack[ibeam] = bathalongtrack[ibeam];

							/* apply rotations and calculate position */
							mbeditviz_beam_position(ping->navlon, ping->navlat, headingx, headingy,
										mtodeglon, mtodeglat,
										ping->bath[ibeam], ping->bathacrosstrack[ibeam], ping->bathalongtrack[ibeam],
										sonardepth,
										rolldelta, pitchdelta,
										&(ping->bathcorr[ibeam]), &(ping->bathlon[ibeam]), &(ping->bathlat[ibeam]));
							}
						}
					}


				/* extract some more values */
				if (mbev_error == MB_ERROR_NO_ERROR
				    && kind == MB_DATA_DATA)
					{
					mbev_status = mb_extract_nav(mbev_verbose,imbio_ptr,
						istore_ptr,&kind,
						ping->time_i,&ping->time_d,&ping->navlon,&ping->navlat,&ping->speed,
						&ping->heading,&draft,&ping->roll,&ping->pitch,&ping->heave,
						&mbev_error);
					}

				/* extract some more values */
				if (mbev_error == MB_ERROR_NO_ERROR
				    && kind == MB_DATA_DATA)
					{
					mbev_status = mb_ttimes(mbev_verbose,imbio_ptr,
						istore_ptr,&kind,&nbeams,
						ping->ttimes,ping->angles,
						ping->angles_forward,ping->angles_null,
						ping->bheave,ping->alongtrack_offset,
						&ping->draft,&ping->ssv,&mbev_error);
					}

				/* get swathbounds */
				if (mbev_error == MB_ERROR_NO_ERROR
				    && kind == MB_DATA_DATA)
				        {
					if (format == MBF_MBPRONAV)
						{
						mbev_status = mbsys_singlebeam_swathbounds(mbev_verbose, imbio_ptr, istore_ptr, &kind,
											&ping->portlon, &ping->portlat,
											&ping->stbdlon, &ping->stbdlat,
											&mbev_error);
						if (ping->portlon != ping->stbdlon
							|| ping->portlat != ping->stbdlat)
							swathbounds = MB_YES;
						}

					else
						{
						/* find centermost beam */
						icenter = -1;
						iport = -1;
						istbd = -1;
						centerdistance = 0.0;
						portdistance = 0.0;
						stbddistance = 0.0;
						for (ibeam=0;ibeam<beams_bath;ibeam++)
							{
							if (beamflag[ibeam] != MB_FLAG_NULL)
								{
								if (icenter == -1
									|| fabs(bathacrosstrack[ibeam]) < centerdistance)
									{
									icenter = ibeam;
									centerdistance = bathacrosstrack[ibeam];
									}
								if (iport == -1
									|| bathacrosstrack[ibeam] < portdistance)
									{
									iport = ibeam;
									portdistance = bathacrosstrack[ibeam];
									}
								if (istbd == -1
									|| bathacrosstrack[ibeam] > stbddistance)
									{
									istbd = ibeam;
									stbddistance = bathacrosstrack[ibeam];
									}
								}
							}

						mb_coor_scale(mbev_verbose,ping->navlat,&mtodeglon,&mtodeglat);
						headingx = sin(ping->heading * DTR);
						headingy = cos(ping->heading * DTR);
						if (icenter >= 0)
							{
							ping->portlon = ping->bathlon[iport];
							ping->portlat = ping->bathlat[iport];
							ping->stbdlon = ping->bathlon[istbd];
							ping->stbdlat = ping->bathlat[istbd];
							}
						else
							{
							ping->portlon = ping->navlon;
							ping->portlat = ping->navlat;
							ping->stbdlon = ping->navlon;
							ping->stbdlat = ping->navlat;
							}
						}
					}

				/* increment counters */
				if (mbev_error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA)
					file->num_pings++;

				/* print debug statements */
				if (mbev_verbose >= 2)
					{
					fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",
						program_name);
					fprintf(stderr,"dbg2       kind:           %d\n",kind);
					fprintf(stderr,"dbg2       error:          %d\n",mbev_error);
					fprintf(stderr,"dbg2       status:         %d\n",mbev_status);
					}
				if (mbev_verbose >= 2 && kind == MB_DATA_COMMENT)
					{
					fprintf(stderr,"dbg2       comment:        %s\n",comment);
					}
				if (mbev_verbose >= 2 && mbev_error <= 0 && kind == MB_DATA_DATA)
					{
					fprintf(stderr,"dbg2       time_i:         %4d/%2d/%2d %2.2d:%2.2d:%2.2d.%6.6d\n",
								ping->time_i[0],ping->time_i[1],
								ping->time_i[2],ping->time_i[3],
								ping->time_i[4],ping->time_i[5],ping->time_i[6]);
					fprintf(stderr,"dbg2       time_d:         %f\n",ping->time_d);
					fprintf(stderr,"dbg2       navlon:         %f\n",ping->navlon);
					fprintf(stderr,"dbg2       navlat:         %f\n",ping->navlat);
					fprintf(stderr,"dbg2       speed:          %f\n",ping->speed);
					fprintf(stderr,"dbg2       heading:        %f\n",ping->heading);
					fprintf(stderr,"dbg2       distance:       %f\n",ping->distance);
					fprintf(stderr,"dbg2       beams_bath:     %d\n",ping->beams_bath);
					fprintf(stderr,"dbg2       beams_amp:      %d\n",beams_amp);
					fprintf(stderr,"dbg2       pixels_ss:      %d\n",pixels_ss);
					}
				}

			/* close the file */
			mbev_status = mb_close(mbev_verbose,&imbio_ptr,&mbev_error);

			/* if processed file read, then reset the beam edits to the original raw state
				by reading in a global esf file from the raw file */
			if (file->processed_info_loaded == MB_YES)
				{
		    		/* check if global edit file (*.gef) exists and is up to date */
				if ((fstatus = stat(file->path, &file_status)) == 0
					&& (file_status.st_mode & S_IFMT) != S_IFDIR)
					rawmodtime = file_status.st_mtime;
				  else
					rawmodtime = 0;
				sprintf(geffile, "%s.gef", file->path);
				if ((fstatus = stat(geffile, &file_status)) == 0
					&& (file_status.st_mode & S_IFMT) != S_IFDIR)
					gefmodtime = file_status.st_mtime;
				  else
					gefmodtime = 0;
				if (rawmodtime >= gefmodtime)
					{
					sprintf(command, "mbgetesf -I %s -M2 -O %s.gef", file->path, file->path);
					fprintf(stderr,"Generating global edit file:\n\t%s\n",command);
					system(command);
					}

				/* now read and apply the global edits */
				mbev_status = mb_esf_open(mbev_verbose, geffile, MB_YES, MBP_ESF_NOWRITE,
							&(file->esf), &mbev_error);
				if (mbev_status == MB_SUCCESS)
					{
					file->esf_open = MB_YES;
fprintf(stderr,"%d global beam states read from %s...\n",file->esf.nedit,geffile);
					}
				else
					{
					file->esf_open = MB_NO;
					mbev_status = MB_SUCCESS;
					mbev_error = MB_ERROR_NO_ERROR;
					}
				if (file->esf_open == MB_YES)
					{
					/* loop over pings applying edits */
					do_mbeditviz_message_on("MBeditviz is applying original beam states...");
fprintf(stderr,"MBeditviz is applying %d original beam states\n",file->esf.nedit);
					for (iping=0;iping<file->num_pings;iping++)
						{
						ping = &(file->pings[iping]);

						/* apply edits for this ping */
						mb_esf_apply(mbev_verbose, &(file->esf),
		    					    ping->time_d, ping->multiplicity, ping->beams_bath,
							    ping->beamflag, &mbev_error);
						for (ibeam=0;ibeam<ping->beams_bath;ibeam++)
							ping->beamflagorg[ibeam] = ping->beamflag[ibeam];

						/* update message every 250 records */
						if (iping % 250 == 0)
							{
							sprintf(message, "MBeditviz: global edits applied to %d of %d records so far...",
								iping, file->num_pings);
							do_mbeditviz_message_on(message);
							}
						}

					/* close the esf */
					if (file->esf_open == MB_YES)
						{
						mb_esf_close(mbev_verbose, &file->esf, &mbev_error);
						file->esf_open = MB_NO;
						}
					}
				}

if (mbev_verbose > 0)
fprintf(stderr,"loaded swathfile:%s file->processed_info_loaded:%d file->process.mbp_edit_mode:%d\n",
swathfile,file->processed_info_loaded,file->process.mbp_edit_mode);

			/* attempt to load bathymetry edits */
			mbev_status = mb_esf_load(mbev_verbose, file->path, MB_YES, MBP_ESF_NOWRITE,
							file->esffile, &(file->esf), &mbev_error);
			if (mbev_status == MB_SUCCESS)
				{
				file->esf_open = MB_YES;
				}
			else
				{
				file->esf_open = MB_NO;
				mbev_status = MB_SUCCESS;
				mbev_error = MB_ERROR_NO_ERROR;
				}
			if (file->esf_open == MB_YES)
				{
				/* loop over pings applying edits */
				do_mbeditviz_message_on("MBeditviz is applying saved edits...");
				for (iping=0;iping<file->num_pings;iping++)
					{
					ping = &(file->pings[iping]);

					/* apply edits for this ping */
					mb_esf_apply(mbev_verbose, &(file->esf),
		    				    ping->time_d, ping->multiplicity, ping->beams_bath,
						    ping->beamflag, &mbev_error);
					for (ibeam=0;ibeam<ping->beams_bath;ibeam++)
						ping->beamflagorg[ibeam] = ping->beamflag[ibeam];

					/* update message every 250 records */
					if (iping % 250 == 0)
						{
						sprintf(message, "MBeditviz: saved edits applied to %d of %d records so far...",
							iping, file->num_pings);
						do_mbeditviz_message_on(message);
						}
					}

				/* close the esf */
				if (file->esf_open == MB_YES)
					{
					mb_esf_close(mbev_verbose, &file->esf, &mbev_error);
					file->esf_open = MB_NO;
					}
				}
			}

		/* load asynchronous data if available */
		if (mbev_status == MB_SUCCESS)
			{
			/* try to load heading data from file */
			strcpy(asyncfile, file->path);
			strcat(asyncfile, ".ath");
			if ((fstatus = stat(asyncfile, &file_status)) == 0
				&& (file_status.st_mode & S_IFMT) != S_IFDIR)
				{
				/* count the asynchronous heading data */
				file->n_async_heading = 0;
				file->n_async_heading_alloc = 0;
				if ((afp = fopen(asyncfile, "r")) != NULL)
					{
					while ((result = fgets(buffer,MBP_FILENAMESIZE,afp)) == buffer)
						if (buffer[0] != '#')
						    file->n_async_heading++;
					fclose(afp);
					}

				/* allocate space for asynchronous heading */
				if (file->n_async_heading > 0)
					{
					if ((file->async_heading_time_d = (double *) malloc(sizeof(double) * (file->n_async_heading))) != NULL)
						{
						if ((file->async_heading_heading = (double *) malloc(sizeof(double) * (file->n_async_heading))) != NULL)
							{
							file->n_async_heading_alloc = file->n_async_heading;
							}
						else if (file->async_heading_time_d != NULL)
							{
							free(file->async_heading_time_d);
							file->async_heading_time_d = NULL;
							}
						}
					}

				/* read the asynchronous heading data */
				file->n_async_heading = 0;
				if ((afp = fopen(asyncfile, "r")) != NULL)
					{
					while ((result = fgets(buffer,MBP_FILENAMESIZE,afp)) == buffer)
						{
						if (buffer[0] != '#')
		    					{
							nread = sscanf(buffer,"%lf %lf",
									&(file->async_heading_time_d[file->n_async_heading]),
						    			&(file->async_heading_heading[file->n_async_heading]));
							if (nread == 2)
						    	    file->n_async_heading++;
							}
						}
					fclose(afp);
					}

				}

			/* if heading data not loaded from file extract from ping data */
			if (file->n_async_heading <= 0)
				{
				if (file->num_pings > 0)
					{
					if ((file->async_heading_time_d = (double *) malloc(sizeof(double) * (file->num_pings))) != NULL)
						{
						if ((file->async_heading_heading = (double *) malloc(sizeof(double) * (file->num_pings))) != NULL)
							{
							file->n_async_heading = file->num_pings;
							file->n_async_heading_alloc = file->n_async_heading;
							}
						else if (file->async_heading_time_d != NULL)
							{
							free(file->async_heading_time_d);
							file->async_heading_time_d = NULL;
							}
						}
					for (iping=0;iping<file->num_pings;iping++)
						{
						ping = &(file->pings[iping]);
						file->async_heading_time_d[iping] = ping->time_d;
						file->async_heading_heading[iping] = ping->heading;
						}
					}
				}

			/* try to load sonardepth data */
			strcpy(asyncfile, file->path);
			strcat(asyncfile, ".ats");
			if ((fstatus = stat(asyncfile, &file_status)) == 0
				&& (file_status.st_mode & S_IFMT) != S_IFDIR)
				{
				/* count the asynchronous sonardepth data */
				file->n_async_sonardepth = 0;
				file->n_async_sonardepth_alloc = 0;
				if ((afp = fopen(asyncfile, "r")) != NULL)
					{
					while ((result = fgets(buffer,MBP_FILENAMESIZE,afp)) == buffer)
						if (buffer[0] != '#')
						    file->n_async_sonardepth++;
					fclose(afp);
					}

				/* allocate space for asynchronous sonardepth */
				if (file->n_async_sonardepth > 0)
					{
					if ((file->async_sonardepth_time_d = (double *) malloc(sizeof(double) * (file->n_async_sonardepth))) != NULL)
						{
						if ((file->async_sonardepth_sonardepth = (double *) malloc(sizeof(double) * (file->n_async_sonardepth))) != NULL)
							{
							file->n_async_sonardepth_alloc = file->n_async_sonardepth;
							}
						else if (file->async_sonardepth_time_d != NULL)
							{
							free(file->async_sonardepth_time_d);
							file->async_sonardepth_time_d = NULL;
							}
						}
					}

				/* read the asynchronous sonardepth data */
				file->n_async_sonardepth = 0;
				if ((afp = fopen(asyncfile, "r")) != NULL)
					{
					while ((result = fgets(buffer,MBP_FILENAMESIZE,afp)) == buffer)
						{
						if (buffer[0] != '#')
		    					{
							nread = sscanf(buffer,"%lf %lf",
									&(file->async_sonardepth_time_d[file->n_async_sonardepth]),
						    			&(file->async_sonardepth_sonardepth[file->n_async_sonardepth]));
							if (nread == 2)
						    	    file->n_async_sonardepth++;
							}
						}
					fclose(afp);
					}

				}

			/* if sonardepth data not loaded from file extract from ping data */
			if (file->n_async_sonardepth <= 0)
				{
				if (file->num_pings > 0)
					{
					if ((file->async_sonardepth_time_d = (double *) malloc(sizeof(double) * (file->num_pings))) != NULL)
						{
						if ((file->async_sonardepth_sonardepth = (double *) malloc(sizeof(double) * (file->num_pings))) != NULL)
							{
							file->n_async_sonardepth = file->num_pings;
							file->n_async_sonardepth_alloc = file->n_async_sonardepth;
							}
						else if (file->async_sonardepth_time_d != NULL)
							{
							free(file->async_sonardepth_time_d);
							file->async_sonardepth_time_d = NULL;
							}
						}
					for (iping=0;iping<file->num_pings;iping++)
						{
						ping = &(file->pings[iping]);
						file->async_sonardepth_time_d[iping] = ping->time_d;
						file->async_sonardepth_sonardepth[iping] = ping->sonardepth;
						}
					}
				}

			/* try to load asynchronous attitude data */
			strcpy(asyncfile, file->path);
			strcat(asyncfile, ".ata");
			if ((fstatus = stat(asyncfile, &file_status)) == 0
				&& (file_status.st_mode & S_IFMT) != S_IFDIR)
				{
				/* count the asynchronous attitude data */
				file->n_async_attitude = 0;
				file->n_async_attitude_alloc = 0;
				if ((afp = fopen(asyncfile, "r")) != NULL)
					{
					while ((result = fgets(buffer,MBP_FILENAMESIZE,afp)) == buffer)
						if (buffer[0] != '#')
						    file->n_async_attitude++;
					fclose(afp);
					}

				/* allocate space for asynchronous attitude */
				if (file->n_async_attitude > 0)
					{
					if ((file->async_attitude_time_d = (double *) malloc(sizeof(double) * (file->n_async_attitude))) != NULL)
						{
						if ((file->async_attitude_roll = (double *) malloc(sizeof(double) * (file->n_async_attitude))) != NULL)
							{
							if ((file->async_attitude_pitch = (double *) malloc(sizeof(double) * (file->n_async_attitude))) != NULL)
								{
								file->n_async_attitude_alloc = file->n_async_attitude;
								}
							else
								{
								if (file->async_attitude_time_d != NULL)
									{
									free(file->async_attitude_time_d);
									file->async_attitude_time_d = NULL;
									}
								if (file->async_attitude_roll != NULL)
									{
									free(file->async_attitude_roll);
									file->async_attitude_roll = NULL;
									}
								}
							}
						else if (file->async_attitude_time_d != NULL)
							{
							free(file->async_attitude_time_d);
							file->async_attitude_time_d = NULL;
							}
						}
					}

				/* read the asynchronous attitude data */
				file->n_async_attitude = 0;
				if ((afp = fopen(asyncfile, "r")) != NULL)
					{
					while ((result = fgets(buffer,MBP_FILENAMESIZE,afp)) == buffer)
						{
						if (buffer[0] != '#')
		    					{
							nread = sscanf(buffer,"%lf %lf %lf",
									&(file->async_attitude_time_d[file->n_async_attitude]),
						    			&(file->async_attitude_roll[file->n_async_attitude]),
						    			&(file->async_attitude_pitch[file->n_async_attitude]));
							if (nread == 3)
						    	    file->n_async_attitude++;
							}
						}
					fclose(afp);
					}
				}

			/* if attitude data not loaded from file extract from ping data */
			if (file->n_async_attitude <= 0)
				{
				if (file->num_pings > 0)
					{
					if ((file->async_attitude_time_d = (double *) malloc(sizeof(double) * (file->num_pings))) != NULL)
						{
						if ((file->async_attitude_roll = (double *) malloc(sizeof(double) * (file->num_pings))) != NULL)
							{
							if ((file->async_attitude_pitch = (double *) malloc(sizeof(double) * (file->num_pings))) != NULL)
								{
								file->n_async_attitude = file->num_pings;
								file->n_async_attitude_alloc = file->n_async_attitude;
								}
							else
								{
								if (file->async_attitude_time_d != NULL)
									{
									free(file->async_attitude_time_d);
									file->async_attitude_time_d = NULL;
									}
								if (file->async_attitude_roll != NULL)
									{
									free(file->async_attitude_roll);
									file->async_attitude_roll = NULL;
									}
								}
							}
						else if (file->async_attitude_time_d != NULL)
							{
							free(file->async_attitude_time_d);
							file->async_attitude_time_d = NULL;
							}
						}
					for (iping=0;iping<file->num_pings;iping++)
						{
						ping = &(file->pings[iping]);
						file->async_attitude_time_d[iping] = ping->time_d;
						file->async_attitude_roll[iping] = ping->roll;
						file->async_attitude_pitch[iping] = ping->pitch;
						}
					}
				}

			/* try to load synchronous attitude data */
			strcpy(asyncfile, file->path);
			strcat(asyncfile, ".sta");
			if ((fstatus = stat(asyncfile, &file_status)) == 0
				&& (file_status.st_mode & S_IFMT) != S_IFDIR)
				{
				/* count the synchronous attitude data */
				file->n_sync_attitude = 0;
				file->n_sync_attitude_alloc = 0;
				if ((afp = fopen(asyncfile, "r")) != NULL)
					{
					while ((result = fgets(buffer,MBP_FILENAMESIZE,afp)) == buffer)
						if (buffer[0] != '#')
						    file->n_sync_attitude++;
					fclose(afp);
					}

				/* allocate space for asynchronous attitude */
				if (file->n_sync_attitude > 0)
					{
					if ((file->sync_attitude_time_d = (double *) malloc(sizeof(double) * (file->n_sync_attitude))) != NULL)
						{
						if ((file->sync_attitude_roll = (double *) malloc(sizeof(double) * (file->n_sync_attitude))) != NULL)
							{
							if ((file->sync_attitude_pitch = (double *) malloc(sizeof(double) * (file->n_sync_attitude))) != NULL)
								{
								file->n_sync_attitude_alloc = file->n_sync_attitude;
								}
							else
								{
								if (file->sync_attitude_time_d != NULL)
									{
									free(file->sync_attitude_time_d);
									file->sync_attitude_time_d = NULL;
									}
								if (file->sync_attitude_roll != NULL)
									{
									free(file->sync_attitude_roll);
									file->sync_attitude_roll = NULL;
									}
								}
							}
						else if (file->sync_attitude_time_d != NULL)
							{
							free(file->sync_attitude_time_d);
							file->sync_attitude_time_d = NULL;
							}
						}
					}

				/* read the synchronous attitude data */
				file->n_sync_attitude = 0;
				if ((afp = fopen(asyncfile, "r")) != NULL)
					{
					while ((result = fgets(buffer,MBP_FILENAMESIZE,afp)) == buffer)
						{
						if (buffer[0] != '#')
		    					{
							nread = sscanf(buffer,"%lf %lf %lf",
									&(file->sync_attitude_time_d[file->n_sync_attitude]),
						    			&(file->sync_attitude_roll[file->n_sync_attitude]),
						    			&(file->sync_attitude_pitch[file->n_sync_attitude]));
							if (nread == 3)
						    	    file->n_sync_attitude++;
							}
						}
					fclose(afp);
					}
				}

			/* if attitude data not loaded from file extract from ping data */
			if (file->n_sync_attitude <= 0)
				{
				if (file->num_pings > 0)
					{
					if ((file->sync_attitude_time_d = (double *) malloc(sizeof(double) * (file->num_pings))) != NULL)
						{
						if ((file->sync_attitude_roll = (double *) malloc(sizeof(double) * (file->num_pings))) != NULL)
							{
							if ((file->sync_attitude_pitch = (double *) malloc(sizeof(double) * (file->num_pings))) != NULL)
								{
								file->n_sync_attitude = file->num_pings;
								file->n_sync_attitude_alloc = file->n_sync_attitude;
								}
							else
								{
								if (file->sync_attitude_time_d != NULL)
									{
									free(file->sync_attitude_time_d);
									file->sync_attitude_time_d = NULL;
									}
								if (file->sync_attitude_roll != NULL)
									{
									free(file->sync_attitude_roll);
									file->sync_attitude_roll = NULL;
									}
								}
							}
						else if (file->sync_attitude_time_d != NULL)
							{
							free(file->sync_attitude_time_d);
							file->sync_attitude_time_d = NULL;
							}
						}
					for (iping=0;iping<file->num_pings;iping++)
						{
						ping = &(file->pings[iping]);
						file->sync_attitude_time_d[iping] = ping->time_d;
						file->sync_attitude_roll[iping] = ping->roll;
						file->sync_attitude_pitch[iping] = ping->pitch;
						}
					}
				}
			}

		/* set the load status */
		if (mbev_status == MB_SUCCESS)
			{
			file->load_status = MB_YES;
			mbev_num_files_loaded++;
			}
		}

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status: %d\n",mbev_status);
		}

	/* return */
	return(mbev_status);
}
/*--------------------------------------------------------------------*/
int mbeditviz_apply_timelag(struct mbev_file_struct *file, struct mbev_ping_struct *ping,
				double rollbias, double pitchbias, double headingbias, double timelag,
				double *heading, double *sonardepth,
				double *rolldelta, double *pitchdelta)
{
	/* local variables */
	char	*function_name = "mbeditviz_apply_timelag";
	double	time_d;
	int	intstat;
	int	iheading = 0;
	int	isonardepth = 0;
	int	iattitude = 0;
	double	rollsync, pitchsync;
	double	rollasync, pitchasync;

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       file:        %lu\n",(size_t)file);
		fprintf(stderr,"dbg2       ping:        %lu\n",(size_t)ping);
		fprintf(stderr,"dbg2       rollbias:    %f\n",rollbias);
		fprintf(stderr,"dbg2       pitchbias:   %f\n",pitchbias);
		fprintf(stderr,"dbg2       headingbias: %f\n",headingbias);
		fprintf(stderr,"dbg2       timelag:     %f\n",timelag);
		}

	/* apply timelag to get new values of lon, lat, heading, rollbias, pitchbias */
	if (file != NULL && ping != NULL)
		{
		/* get adjusted time for interpolation in asyncronous time series */
		time_d = ping->time_d + timelag;

		/* if asyncronous heading available, interpolate new value */
		if (file->n_async_heading > 0)
			{
			intstat = mb_linear_interp_degrees(mbev_verbose,
					file->async_heading_time_d-1, file->async_heading_heading-1,
					file->n_async_heading, time_d, heading, &iheading,
					&mbev_error);
			*heading += headingbias;
			}
		else
			{
			*heading = ping->heading + headingbias;
			}

		/* if asyncronous sonardepth available, interpolate new value */
		if (file->n_async_sonardepth > 0)
			{
			intstat = mb_linear_interp(mbev_verbose,
					file->async_sonardepth_time_d-1, file->async_sonardepth_sonardepth-1,
					file->n_async_sonardepth, time_d, sonardepth, &isonardepth,
					&mbev_error);
			}
		else
			{
			*sonardepth = ping->sonardepth;
			}

		/* if both synchronous and asyncronous attitude available, interpolate new values */
		if (file->n_sync_attitude > 0 && file->n_async_attitude > 0)
			{
			intstat = mb_linear_interp(mbev_verbose,
					file->sync_attitude_time_d-1, file->sync_attitude_roll-1,
					file->n_sync_attitude, ping->time_d, &rollsync, &iattitude,
					&mbev_error);
			intstat = mb_linear_interp(mbev_verbose,
					file->sync_attitude_time_d-1, file->sync_attitude_pitch-1,
					file->n_sync_attitude, ping->time_d, &pitchsync, &iattitude,
					&mbev_error);
			intstat = mb_linear_interp(mbev_verbose,
					file->async_attitude_time_d-1, file->async_attitude_roll-1,
					file->n_async_attitude, time_d, &rollasync, &iattitude,
					&mbev_error);
			intstat = mb_linear_interp(mbev_verbose,
					file->async_attitude_time_d-1, file->async_attitude_pitch-1,
					file->n_async_attitude, time_d, &pitchasync, &iattitude,
					&mbev_error);
			*rolldelta = rollasync - rollsync + rollbias;
			*pitchdelta = pitchasync - pitchsync + pitchbias;
			}
		else
			{
			*rolldelta = rollbias;
			*pitchdelta = pitchbias;
			}
/*fprintf(stderr,"heading: %f %f   %f %d\n", *heading, ping->heading, *heading-ping->heading, iheading);
fprintf(stderr,"sonardepth: %f %f   %f %d\n", *sonardepth, ping->sonardepth,*sonardepth-ping->sonardepth, isonardepth);
fprintf(stderr,"rolldelta:  %f %f    roll:%f %f   %d\n", *rolldelta, rollbias, rollasync, rollsync, iattitude);
fprintf(stderr,"pitchdelta: %f %f    pitch:%f %f   %d\n", *pitchdelta, pitchbias, pitchasync, pitchsync, iattitude);*/
		}

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2       heading:    %f\n",*heading);
		fprintf(stderr,"dbg2       sonardepth: %f\n",*sonardepth);
		fprintf(stderr,"dbg2       rolldelta:  %f\n",*rolldelta);
		fprintf(stderr,"dbg2       pitchdelta: %f\n",*pitchdelta);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status: %d\n",mbev_status);
		}

	/* return */
	return(mbev_status);
}
/*--------------------------------------------------------------------*/
int mbeditviz_beam_position(double navlon, double navlat, double headingx, double headingy,
				double mtodeglon, double mtodeglat,
				double bath, double acrosstrack, double alongtrack,
				double sonardepth,
				double rollbias, double pitchbias,
				double *bathcorr, double *lon, double *lat)
{
	/* local variables */
	char	*function_name = "mbeditviz_beam_position";
	double	bathuse;
	double	range;
	double	alpha, beta;
	double	newbath, newacrosstrack, newalongtrack;

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       navlon:      %f\n",navlon);
		fprintf(stderr,"dbg2       navlat:      %f\n",navlat);
		fprintf(stderr,"dbg2       mtodeglon:   %f\n",mtodeglon);
		fprintf(stderr,"dbg2       mtodeglat:   %f\n",mtodeglat);
		fprintf(stderr,"dbg2       headingx:    %f\n",headingx);
		fprintf(stderr,"dbg2       headingy:    %f\n",headingy);
		fprintf(stderr,"dbg2       bath:        %f\n",bath);
		fprintf(stderr,"dbg2       acrosstrack: %f\n",acrosstrack);
		fprintf(stderr,"dbg2       alongtrack:  %f\n",alongtrack);
		fprintf(stderr,"dbg2       sonardepth:  %f\n",sonardepth);
		fprintf(stderr,"dbg2       rollbias:    %f\n",rollbias);
		fprintf(stderr,"dbg2       pitchbias:   %f\n",pitchbias);
		}

	/* strip off heave + draft */
	bathuse = bath - sonardepth;

	/* get range and angles in
	    roll-pitch frame */
	range = sqrt(bathuse * bathuse
		    + acrosstrack
			* acrosstrack
		    + alongtrack
			* alongtrack);
	if (fabs(range) < 0.001)
		{
		alpha = 0.0;
		beta = 0.5 * M_PI;
		}
	else
		{
		alpha = asin(MAX(-1.0, MIN(1.0, (alongtrack / range))));
		beta = acos(MAX(-1.0, MIN(1.0, (acrosstrack / range / cos(alpha)))));
		}
	if (bathuse < 0.0)
		beta = 2.0 * M_PI - beta;

	/* apply roll pitch corrections */
	alpha += DTR * pitchbias;
	beta += DTR * rollbias;

	/* recalculate bathymetry */
	newbath = range * cos(alpha) * sin(beta);
	newalongtrack = range * sin(alpha);
	newacrosstrack = range * cos(alpha) * cos(beta);

	/* add heave and draft back in */
	*bathcorr = newbath + sonardepth;

	/* locate lon lat position */
	*lon = navlon
		+ headingy * mtodeglon * acrosstrack
		+ headingx * mtodeglon * alongtrack;
	*lat = navlat
		- headingx * mtodeglat * acrosstrack
		+ headingy * mtodeglat * alongtrack;
if (isnan(*bathcorr))
{
fprintf(stderr,"\nFunction mbeditviz_beam_position(): Calculated NaN bathcorr\n");
fprintf(stderr,"     navlon:      %f\n",navlon);
fprintf(stderr,"     navlat:      %f\n",navlat);
fprintf(stderr,"     mtodeglon:   %f\n",mtodeglon);
fprintf(stderr,"     mtodeglat:   %f\n",mtodeglat);
fprintf(stderr,"     headingx:    %f\n",headingx);
fprintf(stderr,"     headingy:    %f\n",headingy);
fprintf(stderr,"     bath:        %f\n",bath);
fprintf(stderr,"     acrosstrack: %f\n",acrosstrack);
fprintf(stderr,"     alongtrack:  %f\n",alongtrack);
fprintf(stderr,"     sonardepth:  %f\n",sonardepth);
fprintf(stderr,"     rollbias:    %f\n",rollbias);
fprintf(stderr,"     pitchbias:   %f\n",pitchbias);
fprintf(stderr,"     bathuse:     %f\n",bathuse);
fprintf(stderr,"     range:       %f\n",range);
fprintf(stderr,"     alpha:       %f\n",alpha);
fprintf(stderr,"     beta:        %f\n",beta);
fprintf(stderr,"     newbath:     %f\n",newbath);
fprintf(stderr,"     bathcorr:    %f\n",*bathcorr);
fprintf(stderr,"     lon:         %f\n",*lon);
fprintf(stderr,"     lat:         %f\n",*lat);
}

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2       bathcorr:    %f\n",*bathcorr);
		fprintf(stderr,"dbg2       lon:         %f\n",*lon);
		fprintf(stderr,"dbg2       lat:         %f\n",*lat);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status: %d\n",mbev_status);
		}

	/* return */
	return(mbev_status);
}
/*--------------------------------------------------------------------*/
int mbeditviz_unload_file(int ifile)
{
	/* local variables */
	char	*function_name = "mbeditviz_unload_file";
	struct mbev_file_struct *file;
	struct mbev_ping_struct *ping;
	int	lock_status;
	int	lock_error = MB_ERROR_NO_ERROR;
	int	iping;

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       ifile:       %d\n",ifile);
		}

	/* unload the file */
	if (ifile >= 0 && ifile < mbev_num_files
		&& mbev_files[ifile].load_status == MB_YES)
		{

		/* release memory */
		file = &(mbev_files[ifile]);
		if (file->pings != NULL)
		    {
		    for (iping=0;iping<file->num_pings;iping++)
			{
			ping = &(file->pings[iping]);
			if (ping->beamflag != NULL)
				{
				free(ping->beamflag);
				ping->beamflag = NULL;
				}
			if (ping->beamflagorg != NULL)
				{
				free(ping->beamflagorg);
				ping->beamflagorg = NULL;
				}
			if (ping->bath != NULL)
				{
				free(ping->bath);
				ping->bath = NULL;
				}
			if (ping->bathacrosstrack != NULL)
				{
				free(ping->bathacrosstrack);
				ping->bathacrosstrack = NULL;
				}
			if (ping->bathalongtrack != NULL)
				{
				free(ping->bathalongtrack);
				ping->bathalongtrack = NULL;
				}
			if (ping->bathcorr != NULL)
				{
				free(ping->bathcorr);
				ping->bathcorr = NULL;
				}
			if (ping->bathlon != NULL)
				{
				free(ping->bathlon);
				ping->bathlon = NULL;
				}
			if (ping->bathlat != NULL)
				{
				free(ping->bathlat);
				ping->bathlat = NULL;
				}
			if (ping->bathx != NULL)
				{
				free(ping->bathx);
				ping->bathx = NULL;
				}
			if (ping->bathy != NULL)
				{
				free(ping->bathy);
				ping->bathy = NULL;
				}
			if (ping->angles != NULL)
				{
				free(ping->angles);
				ping->angles = NULL;
				}
			if (ping->angles_forward != NULL)
				{
				free(ping->angles_forward);
				ping->angles_forward = NULL;
				}
			if (ping->angles_null != NULL)
				{
				free(ping->angles_null);
				ping->angles_null = NULL;
				}
			if (ping->ttimes != NULL)
				{
				free(ping->ttimes);
				ping->ttimes = NULL;
				}
			if (ping->bheave != NULL)
				{
				free(ping->bheave);
				ping->bheave = NULL;
				}
			if (ping->alongtrack_offset != NULL)
				{
				free(ping->alongtrack_offset);
				ping->alongtrack_offset = NULL;
				}
			}
		    free(file->pings);
		    file->pings = NULL;

		    file->n_async_heading = 0;
		    file->n_async_heading_alloc = 0;
		    if (file->async_heading_time_d != NULL)
		    	{
			free(file->async_heading_time_d);
			file->async_heading_time_d = NULL;
			}
		    if (file->async_heading_heading != NULL)
		    	{
			free(file->async_heading_heading);
			file->async_heading_heading = NULL;
			}
		    file->n_async_sonardepth = 0;
		    file->n_async_sonardepth_alloc = 0;
		    if (file->async_sonardepth_time_d != NULL)
		    	{
			free(file->async_sonardepth_time_d);
			file->async_sonardepth_time_d = NULL;
			}
		    if (file->async_sonardepth_sonardepth != NULL)
		    	{
			free(file->async_sonardepth_sonardepth);
			file->async_sonardepth_sonardepth = NULL;
			}
		    file->n_async_attitude = 0;
		    file->n_async_attitude_alloc = 0;
		    if (file->async_attitude_time_d != NULL)
		    	{
			free(file->async_attitude_time_d);
			file->async_attitude_time_d = NULL;
			}
		    if (file->async_attitude_roll != NULL)
		    	{
			free(file->async_attitude_roll);
			file->async_attitude_roll = NULL;
			}
		    if (file->async_attitude_pitch != NULL)
		    	{
			free(file->async_attitude_pitch);
			file->async_attitude_pitch = NULL;
			}
		    file->n_sync_attitude = 0;
		    file->n_sync_attitude_alloc = 0;
		    if (file->sync_attitude_time_d != NULL)
		    	{
			free(file->sync_attitude_time_d);
			file->sync_attitude_time_d = NULL;
			}
		    if (file->sync_attitude_roll != NULL)
		    	{
			free(file->sync_attitude_roll);
			file->sync_attitude_roll = NULL;
			}
		    if (file->sync_attitude_pitch != NULL)
		    	{
			free(file->sync_attitude_pitch);
			file->sync_attitude_pitch = NULL;
			}
		    }

		/* reset load status */
		file->load_status = MB_NO;
		mbev_num_files_loaded--;

		/* unlock the file */
		if (mbdef_uselockfiles == MB_YES)
			lock_status = mb_pr_unlockswathfile(mbev_verbose, file->path,
						MBP_LOCK_EDITBATHY, program_name, &lock_error);

		}

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status: %d\n",mbev_status);
		}

	/* return */
	return(mbev_status);
}
/*--------------------------------------------------------------------*/
int mbeditviz_delete_file(int ifile)
{
	/* local variables */
	char	*function_name = "mbeditviz_delete_file";
	int	i;

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       ifile:       %d\n",ifile);
		}

	/* unload the file if needed */
	if (ifile >= 0 && ifile < mbev_num_files
		&& mbev_files[ifile].load_status == MB_YES)
		{
		mbeditviz_unload_file(ifile);
		}

	/* delete the file */
	for (i=ifile;i<mbev_num_files-1;i++)
		{
		mbev_files[i] = mbev_files[i+1];
		}
	mbev_num_files--;

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status: %d\n",mbev_status);
		}

	/* return */
	return(mbev_status);
}
/*--------------------------------------------------------------------*/
/* approximate error function altered from numerical recipies */
double mbeditviz_erf(double x)
{
	double t, z, erfc_d, erf_d;

	z=fabs(x);
	t=1.0/(1.0+0.5*z);
	erfc_d=t*exp(-z*z-1.26551223+t*(1.00002368+t*(0.37409196+t*(0.09678418+
		t*(-0.18628806+t*(0.27886807+t*(-1.13520398+t*(1.48851587+
		t*(-0.82215223+t*0.17087277)))))))));
	erfc_d =  x >= 0.0 ? erfc_d : 2.0-erfc_d;
	erf_d = 1.0 - erfc_d;
	return  erf_d;
}
/*--------------------------------------------------------------------*/
/*
 * function mbeditviz_bin_weight calculates the integrated weight over a bin
 * given the footprint of a sounding
 */
int mbeditviz_bin_weight(double foot_a, double foot_b, double scale,
		    double pcx, double pcy, double dx, double dy,
		    double *px, double *py,
		    double *weight, int *use)
{
	char	*function_name = "mbeditviz_bin_weight";
	double	fa, fb;
	double	xe, ye, ang, ratio;
	int	i;

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       foot_a:     %f\n",foot_a);
		fprintf(stderr,"dbg2       foot_b:     %f\n",foot_b);
		fprintf(stderr,"dbg2       scale:      %f\n",scale);
		fprintf(stderr,"dbg2       pcx:        %f\n",pcx);
		fprintf(stderr,"dbg2       pcy:        %f\n",pcy);
		fprintf(stderr,"dbg2       dx:         %f\n",dx);
		fprintf(stderr,"dbg2       dy:         %f\n",dy);
		fprintf(stderr,"dbg2       p1 x:       %f\n",px[0]);
		fprintf(stderr,"dbg2       p1 y:       %f\n",py[0]);
		fprintf(stderr,"dbg2       p2 x:       %f\n",px[1]);
		fprintf(stderr,"dbg2       p2 y:       %f\n",py[1]);
		fprintf(stderr,"dbg2       p3 x:       %f\n",px[2]);
		fprintf(stderr,"dbg2       p3 y:       %f\n",py[2]);
		fprintf(stderr,"dbg2       p4 x:       %f\n",px[3]);
		fprintf(stderr,"dbg2       p4 y:       %f\n",py[3]);
		}

	/* The weighting function is
		w(x, y) = (1 / (PI * a * b)) * exp(-(x**2/a**2 + y**2/b**2))
	    in the footprint coordinate system, where the x axis
	    is along the horizontal projection of the beam and the
	    y axix is perpendicular to that. The integral of the
	    weighting function over an simple rectangle defined
	    by corners (x1, y1), (x2, y1), (x1, y2), (x2, y2) is
		    x2 y2
		W = I  I { w(x, y) } dx dy
		    x1 y1

		  = 1 / 4 * ( erfc(x1/a) - erfc(x2/a)) * ( erfc(y1/a) - erfc(y2/a))
	    where erfc(u) is the complementary error function.
	    Each bin is represented as a simple integral in geographic
	    coordinates, but is rotated in the footprint coordinate system.
	    I can't figure out how to evaluate this integral over a
	    rotated rectangle,  and so I am crudely and incorrectly
	    approximating the integrated weight value by evaluating it over
	    the same sized rectangle centered at the same location.
	    Maybe someday I'll figure out how to do it correctly.
	    DWC 11/18/99 */

	/* get integrated weight */
	fa = scale * foot_a;
	fb = scale * foot_b;
	*weight = 0.25 * ( mbeditviz_erf((pcx + dx) / fa) - mbeditviz_erf((pcx - dx) / fa))
			* ( mbeditviz_erf((pcy + dy) / fb) - mbeditviz_erf((pcy - dy) / fb));

	/* use if weight large or any ratio <= 1 */
	if (*weight > 0.05)
	    {
	    *use = MBEV_USE_YES;
	    }
	/* check ratio of each corner footprint 1/e distance */
	else
	    {
	    *use = MBEV_USE_NO;
	    for (i=0;i<4;i++)
		{
		ang = RTD * atan2(py[i], px[i]);
		xe = foot_a * cos(DTR * ang);
		ye = foot_b * sin(DTR * ang);
		ratio = sqrt((px[i] * px[i] + py[i] * py[i])
				/ (xe * xe + ye * ye));
		if (ratio <= 1.0)
		    *use = MBEV_USE_YES;
		else if (ratio <= 2.0)
		    *use = MBEV_USE_CONDITIONAL;
		}
	    }

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2       weight:     %f\n",*weight);
		fprintf(stderr,"dbg2       use:        %d\n",*use);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status:%d\n",mbev_status);
		}

	/* return status */
	return(mbev_status);
}
/*--------------------------------------------------------------------*/
int mbeditviz_get_grid_bounds()
{
	/* local variables */
	char	*function_name = "mbeditviz_get_grid_bounds";
	struct mbev_file_struct *file;
	struct mb_info_struct *info;
	double	depth_min, depth_max;
	double	altitude_min, altitude_max;
	int	first;
	double	xx, yy;
	double	reference_lon, reference_lat;
	int	utm_zone;
	int	proj_status;
	char	projection_id[MB_PATH_MAXLINE];
	void	*pjptr;
	int	ifile;

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		}

	/* find lon lat bounds of loaded files */
	if (mbev_num_files_loaded > 0)
		{
		first = MB_YES;
		for (ifile=0;ifile<mbev_num_files;ifile++)
			{
			file = &mbev_files[ifile];
			if (file->load_status == MB_YES)
				{
				if (file->processed_info_loaded == MB_YES)
					info = &(file->processed_info);
				else
					info = &(file->raw_info);
				if (first == MB_YES)
					{
					mbev_grid_bounds[0] = info->lon_min;
					mbev_grid_bounds[1] = info->lon_max;
					mbev_grid_bounds[2] = info->lat_min;
					mbev_grid_bounds[3] = info->lat_max;
					depth_min = info->depth_min;
					depth_max = info->depth_max;
					altitude_min = info->altitude_min;
					altitude_max = info->altitude_max;
					first = MB_NO;
/*fprintf(stderr,"Processed:%d Name:%s Bounds: %f %f %f %F   File Bounds: %f %f %f %f\n",
file->processed_info_loaded,file->name,
mbev_grid_bounds[0],mbev_grid_bounds[1],mbev_grid_bounds[2],mbev_grid_bounds[3],
info->lon_min,info->lon_max,info->lat_min,info->lat_max);*/
					}
				else
					{
					mbev_grid_bounds[0] = MIN(mbev_grid_bounds[0],info->lon_min);
					mbev_grid_bounds[1] = MAX(mbev_grid_bounds[1],info->lon_max);
					mbev_grid_bounds[2] = MIN(mbev_grid_bounds[2],info->lat_min);
					mbev_grid_bounds[3] = MAX(mbev_grid_bounds[3],info->lat_max);
					depth_min = MIN(depth_min,info->depth_min);
					depth_max = MIN(depth_max,info->depth_max);
					altitude_min = MIN(altitude_min,info->altitude_min);
					altitude_max = MIN(altitude_max,info->altitude_max);
/*fprintf(stderr,"Processed:%d Name:%s Bounds: %f %f %f %F   File Bounds: %f %f %f %f\n",
file->processed_info_loaded,file->name,
mbev_grid_bounds[0],mbev_grid_bounds[1],mbev_grid_bounds[2],mbev_grid_bounds[3],
info->lon_min,info->lon_max,info->lat_min,info->lat_max);*/
					}
				}
			}
		}
	if (mbev_num_files_loaded <= 0 || mbev_grid_bounds[1] <= mbev_grid_bounds[0]
		|| mbev_grid_bounds[3] <= mbev_grid_bounds[2])
		{
		mbev_status = MB_FAILURE;
		mbev_error = MB_ERROR_BAD_PARAMETER;
		}
	else
		{
		mbev_status = MB_SUCCESS;
		mbev_error = MB_ERROR_NO_ERROR;
		}

	/* get projection */
	if (mbev_status == MB_SUCCESS)
		{
		/* get projection */
		reference_lon = 0.5 * (mbev_grid_bounds[0] + mbev_grid_bounds[1]);
		reference_lat = 0.5 * (mbev_grid_bounds[2] + mbev_grid_bounds[3]);
		if (reference_lon < 180.0)
			reference_lon += 360.0;
		if (reference_lon >= 180.0)
			reference_lon -= 360.0;
		utm_zone = (int)(((reference_lon + 183.0)
			/ 6.0) + 0.5);
		if (reference_lat >= 0.0)
			sprintf(projection_id, "UTM%2.2dN", utm_zone);
		else
			sprintf(projection_id, "UTM%2.2dS", utm_zone);
		proj_status = mb_proj_init(mbev_verbose,projection_id,
			&(pjptr), &mbev_error);
		if (proj_status != MB_SUCCESS)
			{
			mbev_status = MB_FAILURE;
			mbev_error = MB_ERROR_BAD_PARAMETER;
			}
		}

	/* get grid cell size and dimensions */
	if (mbev_status == MB_SUCCESS)
		{
		/* get projected bounds */

		/* first point */
		mb_proj_forward(mbev_verbose, pjptr, mbev_grid_bounds[0], mbev_grid_bounds[2],
				&xx, &yy, &mbev_error);
		mbev_grid_boundsutm[0] = xx;
		mbev_grid_boundsutm[1] = xx;
		mbev_grid_boundsutm[2] = yy;
		mbev_grid_boundsutm[3] = yy;

		/* second point */
		mb_proj_forward(mbev_verbose, pjptr, mbev_grid_bounds[1], mbev_grid_bounds[2],
				&xx, &yy, &mbev_error);
		mbev_grid_boundsutm[0] = MIN(mbev_grid_boundsutm[0], xx);
		mbev_grid_boundsutm[1] = MAX(mbev_grid_boundsutm[1], xx);
		mbev_grid_boundsutm[2] = MIN(mbev_grid_boundsutm[2], yy);
		mbev_grid_boundsutm[3] = MAX(mbev_grid.boundsutm[3], yy);

		/* third point */
		mb_proj_forward(mbev_verbose, pjptr, mbev_grid_bounds[0], mbev_grid_bounds[3],
				&xx, &yy, &mbev_error);
		mbev_grid_boundsutm[0] = MIN(mbev_grid_boundsutm[0], xx);
		mbev_grid_boundsutm[1] = MAX(mbev_grid_boundsutm[1], xx);
		mbev_grid_boundsutm[2] = MIN(mbev_grid_boundsutm[2], yy);
		mbev_grid_boundsutm[3] = MAX(mbev_grid_boundsutm[3], yy);

		/* fourth point */
		mb_proj_forward(mbev_verbose, pjptr, mbev_grid_bounds[1], mbev_grid_bounds[3],
				&xx, &yy, &mbev_error);
		mbev_grid_boundsutm[0] = MIN(mbev_grid_boundsutm[0], xx);
		mbev_grid_boundsutm[1] = MAX(mbev_grid_boundsutm[1], xx);
		mbev_grid_boundsutm[2] = MIN(mbev_grid_boundsutm[2], yy);
		mbev_grid_boundsutm[3] = MAX(mbev_grid_boundsutm[3], yy);

		/* get grid spacing */
/*fprintf(stderr,"altitude: %f %f\n", altitude_min, altitude_max);*/
		if (altitude_max > 0.0)
			mbev_grid_cellsize = 0.02 * altitude_max;
		else if (depth_max > 0.0)
			mbev_grid_cellsize = 0.02 * depth_max;
		else
			mbev_grid_cellsize = (mbev_grid_boundsutm[1] - mbev_grid_boundsutm[0]) / 250;

		/* get grid dimensions */
		mbev_grid_nx = (mbev_grid_boundsutm[1] - mbev_grid_boundsutm[0]) / mbev_grid_cellsize + 1;
		mbev_grid_ny = (mbev_grid_boundsutm[3] - mbev_grid_boundsutm[2]) / mbev_grid_cellsize + 1;
		mbev_grid_boundsutm[1] = mbev_grid_boundsutm[0] + (mbev_grid_nx - 1) * mbev_grid_cellsize;
		mbev_grid_boundsutm[3] = mbev_grid_boundsutm[2] + (mbev_grid_ny - 1) * mbev_grid_cellsize;
/*fprintf(stderr,"Grid bounds: %f %f %f %f    %f %f %f %f\n",
mbev_grid_bounds[0],mbev_grid_bounds[1],mbev_grid_bounds[2],mbev_grid_bounds[3],
mbev_grid_boundsutm[0],mbev_grid_boundsutm[1],mbev_grid_boundsutm[2],mbev_grid_boundsutm[3]);
fprintf(stderr,"cell size:%f dimensions: %d %d\n",
mbev_grid_cellsize,mbev_grid_nx,mbev_grid_ny);*/

		/* release projection */
		mb_proj_free(mbev_verbose, &(pjptr), &mbev_error);
		}

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status: %d\n",mbev_status);
		}

	/* return */
	return(mbev_status);
}

/*--------------------------------------------------------------------*/
int mbeditviz_setup_grid()
{
	/* local variables */
	char	*function_name = "mbeditviz_setup_grid";
	double	xx, yy;
	double	reference_lon, reference_lat;
	int	utm_zone;
	int	proj_status;

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		}

	/* find lon lat bounds of loaded files */
	if (mbev_num_files_loaded > 0)
		{
		/* get grid bounds */
		mbev_grid.bounds[0] = mbev_grid_bounds[0];
		mbev_grid.bounds[1] = mbev_grid_bounds[1];
		mbev_grid.bounds[2] = mbev_grid_bounds[2];
		mbev_grid.bounds[3] = mbev_grid_bounds[3];

		/* get grid spacing */
		mbev_grid.dx = mbev_grid_cellsize;
		mbev_grid.dy = mbev_grid_cellsize;
		}
	if (mbev_num_files_loaded <= 0 || mbev_grid.bounds[1] <= mbev_grid.bounds[0]
		|| mbev_grid.bounds[3] <= mbev_grid.bounds[2])
		{
		mbev_status = MB_FAILURE;
		mbev_error = MB_ERROR_BAD_PARAMETER;
		}
	else
		{
		mbev_status = MB_SUCCESS;
		mbev_error = MB_ERROR_NO_ERROR;
		}

	/* get projection */
	if (mbev_status == MB_SUCCESS)
		{
		/* get projection */
		reference_lon = 0.5 * (mbev_grid.bounds[0] + mbev_grid.bounds[1]);
		reference_lat = 0.5 * (mbev_grid.bounds[2] + mbev_grid.bounds[3]);
		if (reference_lon < 180.0)
			reference_lon += 360.0;
		if (reference_lon >= 180.0)
			reference_lon -= 360.0;
		utm_zone = (int)(((reference_lon + 183.0)
			/ 6.0) + 0.5);
		if (reference_lat >= 0.0)
			sprintf(mbev_grid.projection_id, "UTM%2.2dN", utm_zone);
		else
			sprintf(mbev_grid.projection_id, "UTM%2.2dS", utm_zone);
		proj_status = mb_proj_init(mbev_verbose,mbev_grid.projection_id,
			&(mbev_grid.pjptr), &mbev_error);
		if (proj_status != MB_SUCCESS)
			{
			mbev_status = MB_FAILURE;
			mbev_error = MB_ERROR_BAD_PARAMETER;
			}
		}

	/* get grid cell size and dimensions */
	if (mbev_status == MB_SUCCESS)
		{
		/* get projected bounds */

		/* first point */
		mb_proj_forward(mbev_verbose, mbev_grid.pjptr, mbev_grid.bounds[0], mbev_grid.bounds[2],
				&xx, &yy, &mbev_error);
		mbev_grid.boundsutm[0] = xx;
		mbev_grid.boundsutm[1] = xx;
		mbev_grid.boundsutm[2] = yy;
		mbev_grid.boundsutm[3] = yy;

		/* second point */
		mb_proj_forward(mbev_verbose, mbev_grid.pjptr, mbev_grid.bounds[1], mbev_grid.bounds[2],
				&xx, &yy, &mbev_error);
		mbev_grid.boundsutm[0] = MIN(mbev_grid.boundsutm[0], xx);
		mbev_grid.boundsutm[1] = MAX(mbev_grid.boundsutm[1], xx);
		mbev_grid.boundsutm[2] = MIN(mbev_grid.boundsutm[2], yy);
		mbev_grid.boundsutm[3] = MAX(mbev_grid.boundsutm[3], yy);

		/* third point */
		mb_proj_forward(mbev_verbose, mbev_grid.pjptr, mbev_grid.bounds[0], mbev_grid.bounds[3],
				&xx, &yy, &mbev_error);
		mbev_grid.boundsutm[0] = MIN(mbev_grid.boundsutm[0], xx);
		mbev_grid.boundsutm[1] = MAX(mbev_grid.boundsutm[1], xx);
		mbev_grid.boundsutm[2] = MIN(mbev_grid.boundsutm[2], yy);
		mbev_grid.boundsutm[3] = MAX(mbev_grid.boundsutm[3], yy);

		/* fourth point */
		mb_proj_forward(mbev_verbose, mbev_grid.pjptr, mbev_grid.bounds[1], mbev_grid.bounds[3],
				&xx, &yy, &mbev_error);
		mbev_grid.boundsutm[0] = MIN(mbev_grid.boundsutm[0], xx);
		mbev_grid.boundsutm[1] = MAX(mbev_grid.boundsutm[1], xx);
		mbev_grid.boundsutm[2] = MIN(mbev_grid.boundsutm[2], yy);
		mbev_grid.boundsutm[3] = MAX(mbev_grid.boundsutm[3], yy);

		/* get grid dimensions */
		mbev_grid.nx = (mbev_grid.boundsutm[1] - mbev_grid.boundsutm[0]) / mbev_grid.dx + 1;
		mbev_grid.ny = (mbev_grid.boundsutm[3] - mbev_grid.boundsutm[2]) / mbev_grid.dy + 1;
		mbev_grid.boundsutm[1] = mbev_grid.boundsutm[0] + (mbev_grid.nx - 1) * mbev_grid.dx;
		mbev_grid.boundsutm[3] = mbev_grid.boundsutm[2] + (mbev_grid.ny - 1) * mbev_grid.dy;
/*fprintf(stderr,"Grid bounds: %f %f %f %f    %f %f %f %f\n",
mbev_grid.bounds[0],mbev_grid.bounds[1],mbev_grid.bounds[2],mbev_grid.bounds[3],
mbev_grid.boundsutm[0],mbev_grid.boundsutm[1],mbev_grid.boundsutm[2],mbev_grid.boundsutm[3]);
fprintf(stderr,"cell size:%f %f dimensions: %d %d\n",
mbev_grid.dx,mbev_grid.dy,mbev_grid.nx,mbev_grid.ny);*/
		}

	/* allocate memory for grid */
	if (mbev_status == MB_SUCCESS)
		{
		if ((mbev_grid.sum = (float *) malloc(mbev_grid.nx * mbev_grid.ny * sizeof(float))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
		if ((mbev_grid.wgt = (float *) malloc(mbev_grid.nx * mbev_grid.ny * sizeof(float))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
		if ((mbev_grid.val = (float *) malloc(mbev_grid.nx * mbev_grid.ny * sizeof(float))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
		if ((mbev_grid.sgm = (float *) malloc(mbev_grid.nx * mbev_grid.ny * sizeof(float))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
		if (mbev_error == MB_ERROR_NO_ERROR)
			{
			memset(mbev_grid.sum, 0, mbev_grid.nx * mbev_grid.ny * sizeof(float));
			memset(mbev_grid.wgt, 0, mbev_grid.nx * mbev_grid.ny * sizeof(float));
			memset(mbev_grid.val, 0, mbev_grid.nx * mbev_grid.ny * sizeof(float));
			memset(mbev_grid.sgm, 0, mbev_grid.nx * mbev_grid.ny * sizeof(float));
			}
		else
			mbev_status = MB_FAILURE;
		}

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status: %d\n",mbev_status);
		}

	/* return */
	return(mbev_status);
}

/*--------------------------------------------------------------------*/
int mbeditviz_project_soundings()
{
	/* local variables */
	char	*function_name = "mbeditviz_project_soundings";
	struct mbev_file_struct *file;
	struct mbev_ping_struct *ping;
	int	ifile, iping, ibeam;
	int	filecount;

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		}

	/* project all soundings into the grid coordinates */
	if (mbev_status == MB_SUCCESS)
		{
		/* loop over loaded files */
		filecount = 0;
		for (ifile=0;ifile<mbev_num_files;ifile++)
			{
			file = &mbev_files[ifile];
			if (file->load_status == MB_YES)
				{
				filecount++;
				sprintf(message, "Projecting file %d of %d...", filecount, mbev_num_files_loaded);
				do_mbeditviz_message_on(message);
				for (iping=0;iping<file->num_pings;iping++)
					{
					ping = &(file->pings[iping]);
					mb_proj_forward(mbev_verbose, mbev_grid.pjptr,
							ping->navlon, ping->navlat,
							&ping->navlonx, &ping->navlaty,
							&mbev_error);
					for (ibeam=0;ibeam<ping->beams_bath;ibeam++)
						{
						if (ping->beamflag[ibeam] != MB_FLAG_NULL)
							{
							mb_proj_forward(mbev_verbose, mbev_grid.pjptr,
									ping->bathlon[ibeam], ping->bathlat[ibeam],
									&ping->bathx[ibeam], &ping->bathy[ibeam],
									&mbev_error);
							}
						}
					}
				}
			}
		}

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status: %d\n",mbev_status);
		}

	/* return */
	return(mbev_status);
}


/*--------------------------------------------------------------------*/
int mbeditviz_make_grid()
{
	/* local variables */
	char	*function_name = "mbeditviz_make_grid";
	struct mbev_file_struct *file;
	struct mbev_ping_struct *ping;
	int	first;
	int	ifile, iping, ibeam;
	int	filecount;
	int	i, j, k;

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		}

	/* zero the grid arrays */
	memset(mbev_grid.sum, 0, mbev_grid.nx * mbev_grid.ny * sizeof(float));
	memset(mbev_grid.wgt, 0, mbev_grid.nx * mbev_grid.ny * sizeof(float));
	/* memset(mbev_grid.val, 0, mbev_grid.nx * mbev_grid.ny * sizeof(float));*/
	memset(mbev_grid.sgm, 0, mbev_grid.nx * mbev_grid.ny * sizeof(float));

	/* loop over loaded files */
	filecount = 0;
	for (ifile=0;ifile<mbev_num_files;ifile++)
		{
		file = &mbev_files[ifile];
		if (file->load_status == MB_YES)
			{
			filecount++;
			sprintf(message, "Gridding file %d of %d...", filecount, mbev_num_files_loaded);
			do_mbeditviz_message_on(message);
			for (iping=0;iping<file->num_pings;iping++)
				{
				ping = &(file->pings[iping]);
				for (ibeam=0;ibeam<ping->beams_bath;ibeam++)
					{
					if (mb_beam_ok(ping->beamflag[ibeam]))
						{
						mbeditviz_grid_beam(file, ping, ibeam, MB_YES, MB_NO);
						}
					}
				}
			}
		}
	mbev_grid.nodatavalue = MBEV_NODATA;
	first = MB_YES;
	for (i=0;i<mbev_grid.nx;i++)
		for (j=0;j<mbev_grid.ny;j++)
			{
			k = i * mbev_grid.ny + j;
			if (mbev_grid.wgt[k] > 0.0)
				{
				mbev_grid.val[k] = mbev_grid.sum[k] / mbev_grid.wgt[k];
				mbev_grid.sgm[k] = sqrt(fabs(mbev_grid.sgm[k] / mbev_grid.wgt[k]
							- mbev_grid.val[k] * mbev_grid.val[k]));
				if (first == MB_YES)
					{
					mbev_grid.min = mbev_grid.val[k];
					mbev_grid.max = mbev_grid.val[k];
					mbev_grid.smin = mbev_grid.sgm[k];
					mbev_grid.smax = mbev_grid.sgm[k];
					first = MB_NO;
					}
				else
					{
					mbev_grid.min = MIN(mbev_grid.min, mbev_grid.val[k]);
					mbev_grid.max = MAX(mbev_grid.max, mbev_grid.val[k]);
					mbev_grid.smin = MIN(mbev_grid.smin, mbev_grid.sgm[k]);
					mbev_grid.smax = MAX(mbev_grid.smax, mbev_grid.sgm[k]);
					}
				}
			else
				{
				mbev_grid.val[k] = mbev_grid.nodatavalue;
				mbev_grid.sgm[k] = mbev_grid.nodatavalue;
				}
			}
	if (mbev_grid.status == MBEV_GRID_NONE)
		mbev_grid.status = MBEV_GRID_NOTVIEWED;

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status: %d\n",mbev_status);
		}

	/* return */
	return(mbev_status);
}

/*--------------------------------------------------------------------*/
int mbeditviz_grid_beam(struct mbev_file_struct *file, struct mbev_ping_struct *ping, int ibeam, int beam_ok, int apply_now)
{
	/* local variables */
	char	*function_name = "mbeditviz_grid_beam";
	double	xx, yy;
	double	foot_dx, foot_dy, foot_dxn, foot_dyn;
	double	foot_lateral, foot_range, foot_theta;
	double	foot_dtheta, foot_dphi;
	double	foot_hwidth, foot_hlength;
	int	foot_wix, foot_wiy, foot_lix, foot_liy, foot_dix, foot_diy;
	double	xx0, yy0, bdx, bdy, xx1, xx2, yy1, yy2;
	double	prx[5], pry[5];
	double	weight;
	int	use_weight;
	int	ix1, ix2, iy1, iy2;
	int	ii, jj, kk;
	int	i, j;

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       file:       %lu\n",(size_t)file);
		fprintf(stderr,"dbg2       ping:       %lu\n",(size_t)ping);
		fprintf(stderr,"dbg2       ibeam:      %d\n",ibeam);
		fprintf(stderr,"dbg2       beam_ok:    %d\n",beam_ok);
		fprintf(stderr,"dbg2       apply_now:  %d\n",apply_now);
		}

	/* find location of beam center */
	i = (ping->bathx[ibeam] - mbev_grid.boundsutm[0] + 0.5 * mbev_grid.dx)
		/ mbev_grid.dx;
	j = (ping->bathy[ibeam] - mbev_grid.boundsutm[2] + 0.5 * mbev_grid.dy)
		/ mbev_grid.dy;

	/* proceed if beam in grid */
	if (i >= 0 && i < mbev_grid.nx && j >= 0 && j < mbev_grid.ny)
		{
		/* simple gridding mode */
		if (mbev_grid_algorithm == MBEV_GRID_ALGORITH_SIMPLE)
			{
			/* get location in grid arrays */
			kk = i * mbev_grid.ny + j;

if (isnan(ping->bathcorr[ibeam]))
{
fprintf(stderr,"\nFunction mbeditviz_grid_beam(): Encountered NaN value in swath data from file: %s\n",file->path);
fprintf(stderr,"     Ping time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
	ping->time_i[0],ping->time_i[1],ping->time_i[2],ping->time_i[3],ping->time_i[4],ping->time_i[5],ping->time_i[6]);
fprintf(stderr,"     Beam bathymetry: beam:%d flag:%d bath:<%f %f> acrosstrack:%f alongtrack:%f\n",ibeam,ping->beamflag[ibeam],ping->bath[ibeam],ping->bathcorr[ibeam],ping->bathacrosstrack[ibeam],ping->bathalongtrack[ibeam]);
}

			/* add to weights and sums */
			if (beam_ok == MB_YES)
				{
				mbev_grid.wgt[kk] += 1.0;
				mbev_grid.sum[kk] += (-ping->bathcorr[ibeam]);
				mbev_grid.sgm[kk] += ping->bathcorr[ibeam] * ping->bathcorr[ibeam];
				}
			else
				{
				mbev_grid.wgt[kk] -= 1.0;
				mbev_grid.sum[kk] -= (-ping->bathcorr[ibeam]);
				mbev_grid.sgm[kk] -= ping->bathcorr[ibeam] * ping->bathcorr[ibeam];
				if (mbev_grid.wgt[kk] < MBEV_GRID_WEIGHT_TINY)
					mbev_grid.wgt[kk] = 0.0;
				}

			/* recalculate grid cell if desired */
			if (apply_now == MB_YES)
				{
				/* recalculate grid cell */
				if (mbev_grid.wgt[kk] > 0.0)
					{
					mbev_grid.val[kk] = mbev_grid.sum[kk] / mbev_grid.wgt[kk];
					mbev_grid.sgm[kk] = sqrt(fabs(mbev_grid.sgm[kk] / mbev_grid.wgt[kk]
								- mbev_grid.val[kk] * mbev_grid.val[kk]));
					mbev_grid.min = MIN(mbev_grid.min, mbev_grid.val[kk]);
					mbev_grid.max = MAX(mbev_grid.max, mbev_grid.val[kk]);
					mbev_grid.smin = MIN(mbev_grid.smin, mbev_grid.sgm[kk]);
					mbev_grid.smax = MAX(mbev_grid.smax, mbev_grid.sgm[kk]);
					}
				else
					{
					mbev_grid.val[kk] = mbev_grid.nodatavalue;
					mbev_grid.sgm[kk] = mbev_grid.nodatavalue;
					}

				/* update grid in mbview display */
				mbview_updateprimarygridcell(mbev_verbose, 0, i, j, mbev_grid.val[kk], &mbev_error);
				}
			}

		/* else footprint gridding algorithm */
		else
			{
			/* calculate footprint */
			foot_dx = (ping->bathx[ibeam] - ping->navlonx);
			foot_dy = (ping->bathy[ibeam] - ping->navlaty);
			foot_lateral = sqrt(foot_dx * foot_dx + foot_dy * foot_dy);
			if (foot_lateral > 0.0)
				{
				foot_dxn = foot_dx / foot_lateral;
				foot_dyn = foot_dy / foot_lateral;
				}
			else
				{
				foot_dxn = 1.0;
				foot_dyn = 0.0;
				}
			foot_range = sqrt(foot_lateral * foot_lateral + ping->altitude * ping->altitude);
			foot_theta = RTD * atan2(foot_lateral, (ping->bathcorr[ibeam] - ping->sonardepth));
			foot_dtheta = 0.5 * file->beamwidth_xtrack;
			foot_dphi = 0.5 * file->beamwidth_ltrack;
			if (foot_dtheta <= 0.0)
				foot_dtheta = 1.0;
			if (foot_dphi <= 0.0)
				foot_dphi = 1.0;
			foot_hwidth =(ping->bathcorr[ibeam] - ping->sonardepth)
					* tan(DTR * (foot_theta + foot_dtheta))
					    - foot_lateral;
			foot_hlength = foot_range * tan(DTR * foot_dphi);

			/* get range of bins around footprint to examine */
			foot_wix = fabs(foot_hwidth * cos(DTR * foot_theta) / mbev_grid.dx);
			foot_wiy = fabs(foot_hwidth * sin(DTR * foot_theta) / mbev_grid.dx);
			foot_lix = fabs(foot_hlength * sin(DTR * foot_theta) / mbev_grid.dy);
			foot_liy = fabs(foot_hlength * cos(DTR * foot_theta) / mbev_grid.dy);
			foot_dix = 2 * MAX(foot_wix, foot_lix);
			foot_diy = 2 * MAX(foot_wiy, foot_liy);
			ix1 = MAX(i - foot_dix, 0);
			ix2 = MIN(i + foot_dix, mbev_grid.nx - 1);
			iy1 = MAX(j - foot_diy, 0);
			iy2 = MIN(j + foot_diy, mbev_grid.ny - 1);

			/* loop over neighborhood of bins */
			for (ii=ix1;ii<=ix2;ii++)
				for (jj=iy1;jj<=iy2;jj++)
					{
					/* find distance of bin center from sounding center */
					xx = (mbev_grid.boundsutm[0] + ii * mbev_grid.dx
						+ 0.5 * mbev_grid.dx - ping->bathx[ibeam]);
					yy = (mbev_grid.boundsutm[2] + jj * mbev_grid.dy
						+ 0.5 * mbev_grid.dy - ping->bathy[ibeam]);

					/* get center and corners of bin in meters from sounding center */
					xx0 = xx;
					yy0 = yy;
					bdx = 0.5 * mbev_grid.dx;
					bdy = 0.5 * mbev_grid.dy;
					xx1 = xx0 - bdx;
					xx2 = xx0 + bdx;
					yy1 = yy0 - bdy;
					yy2 = yy0 + bdy;

					/* rotate center and corners of bin to footprint coordinates */
					prx[0] = xx0 * foot_dxn + yy0 * foot_dyn;
					pry[0] = -xx0 * foot_dyn + yy0 * foot_dxn;
					prx[1] = xx1 * foot_dxn + yy1 * foot_dyn;
					pry[1] = -xx1 * foot_dyn + yy1 * foot_dxn;
					prx[2] = xx2 * foot_dxn + yy1 * foot_dyn;
					pry[2] = -xx2 * foot_dyn + yy1 * foot_dxn;
					prx[3] = xx1 * foot_dxn + yy2 * foot_dyn;
					pry[3] = -xx1 * foot_dyn + yy2 * foot_dxn;
					prx[4] = xx2 * foot_dxn + yy2 * foot_dyn;
					pry[4] = -xx2 * foot_dyn + yy2 * foot_dxn;

					/* get weight integrated over bin */
					mbeditviz_bin_weight(foot_hwidth, foot_hlength, 1.0,
						prx[0], pry[0], bdx, bdy,
						&prx[1], &pry[1],
						&weight, &use_weight);

					/* if beam affects cell apply using weight */
					if (use_weight == MBEV_USE_YES)
						{
						/* get location in grid arrays */
						kk = ii * mbev_grid.ny + jj;

						/* add to weights and sums */
						if (beam_ok == MB_YES)
							{
							mbev_grid.wgt[kk] += weight;
							mbev_grid.sum[kk] += weight * (-ping->bathcorr[ibeam]);
							mbev_grid.sgm[kk] += weight * ping->bathcorr[ibeam] * ping->bathcorr[ibeam];
							}
						else
							{
							mbev_grid.wgt[kk] -= weight;
							mbev_grid.sum[kk] -= weight * (-ping->bathcorr[ibeam]);
							mbev_grid.sgm[kk] -= weight * ping->bathcorr[ibeam] * ping->bathcorr[ibeam];
							if (mbev_grid.wgt[kk] < MBEV_GRID_WEIGHT_TINY)
								mbev_grid.wgt[kk] = 0.0;
							}

						/* recalculate grid cell if desired */
						if (apply_now == MB_YES)
							{
							/* recalculate grid cell */
							if (mbev_grid.wgt[kk] > 0.0)
								{
								mbev_grid.val[kk] = mbev_grid.sum[kk] / mbev_grid.wgt[kk];
								mbev_grid.sgm[kk] = sqrt(fabs(mbev_grid.sgm[kk] / mbev_grid.wgt[kk]
											- mbev_grid.val[kk] * mbev_grid.val[kk]));
								mbev_grid.min = MIN(mbev_grid.min, mbev_grid.val[kk]);
								mbev_grid.max = MAX(mbev_grid.max, mbev_grid.val[kk]);
								mbev_grid.smin = MIN(mbev_grid.smin, mbev_grid.sgm[kk]);
								mbev_grid.smax = MAX(mbev_grid.smax, mbev_grid.sgm[kk]);
								}
							else
								{
								mbev_grid.val[kk] = mbev_grid.nodatavalue;
								mbev_grid.sgm[kk] = mbev_grid.nodatavalue;
								}

							/* update grid in mbview display */
							mbview_updateprimarygridcell(mbev_verbose, 0, ii, jj, mbev_grid.val[kk], &mbev_error);
							}
						}
					}
			}
		}

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status: %d\n",mbev_status);
		}

	/* return */
	return(mbev_status);
}

/*--------------------------------------------------------------------*/
int mbeditviz_make_grid_simple()
{
	/* local variables */
	char	*function_name = "mbeditviz_make_grid_simple";
	struct mbev_file_struct *file;
	struct mbev_ping_struct *ping;
	struct mb_info_struct *info;
	double	depth_min, depth_max;
	double	altitude_min, altitude_max;
	int	first;
	double	xx, yy;
	double	reference_lon, reference_lat;
	int	utm_zone;
	int	proj_status;
	int	ifile, iping, ibeam;
	int	filecount;
	int	i, j, k;

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		}

	/* find lon lat bounds of loaded files */
	if (mbev_num_files_loaded > 0)
		{
		first = MB_YES;
		for (ifile=0;ifile<mbev_num_files;ifile++)
			{
			file = &mbev_files[ifile];
			if (file->load_status == MB_YES)
				{
				if (file->processed_info_loaded == MB_YES)
					info = &(file->processed_info);
				else
					info = &(file->raw_info);
				if (first == MB_YES)
					{
					mbev_grid.bounds[0] = info->lon_min;
					mbev_grid.bounds[1] = info->lon_max;
					mbev_grid.bounds[2] = info->lat_min;
					mbev_grid.bounds[3] = info->lat_max;
					depth_min = info->depth_min;
					depth_max = info->depth_max;
					altitude_min = info->altitude_min;
					altitude_max = info->altitude_max;
					first = MB_NO;
if (mbev_verbose > 0)
fprintf(stderr,"Processed:%d Name:%s Bounds: %f %f %f %F   File Bounds: %f %f %f %f\n",
file->processed_info_loaded,file->name,
mbev_grid.bounds[0],mbev_grid.bounds[1],mbev_grid.bounds[2],mbev_grid.bounds[3],
info->lon_min,info->lon_max,info->lat_min,info->lat_max);
					}
				else
					{
					mbev_grid.bounds[0] = MIN(mbev_grid.bounds[0],info->lon_min);
					mbev_grid.bounds[1] = MAX(mbev_grid.bounds[1],info->lon_max);
					mbev_grid.bounds[2] = MIN(mbev_grid.bounds[2],info->lat_min);
					mbev_grid.bounds[3] = MAX(mbev_grid.bounds[3],info->lat_max);
					depth_min = MIN(depth_min,info->depth_min);
					depth_max = MIN(depth_max,info->depth_max);
					altitude_min = MIN(altitude_min,info->altitude_min);
					altitude_max = MIN(altitude_max,info->altitude_max);
if (mbev_verbose > 0)
fprintf(stderr,"Processed:%d Name:%s Bounds: %f %f %f %F   File Bounds: %f %f %f %f\n",
file->processed_info_loaded,file->name,
mbev_grid.bounds[0],mbev_grid.bounds[1],mbev_grid.bounds[2],mbev_grid.bounds[3],
info->lon_min,info->lon_max,info->lat_min,info->lat_max);
					}
				}
			}
		}
	if (mbev_num_files_loaded <= 0 || mbev_grid.bounds[1] <= mbev_grid.bounds[0]
		|| mbev_grid.bounds[3] <= mbev_grid.bounds[2])
		{
		mbev_status = MB_FAILURE;
		mbev_error = MB_ERROR_BAD_PARAMETER;
		}
	else
		{
		mbev_status = MB_SUCCESS;
		mbev_error = MB_ERROR_NO_ERROR;
		}

	/* get projection */
	if (mbev_status == MB_SUCCESS)
		{
		/* get projection */
		reference_lon = 0.5 * (mbev_grid.bounds[0] + mbev_grid.bounds[1]);
		reference_lat = 0.5 * (mbev_grid.bounds[2] + mbev_grid.bounds[3]);
		if (reference_lon < 180.0)
			reference_lon += 360.0;
		if (reference_lon >= 180.0)
			reference_lon -= 360.0;
		utm_zone = (int)(((reference_lon + 183.0)
			/ 6.0) + 0.5);
		if (reference_lat >= 0.0)
			sprintf(mbev_grid.projection_id, "UTM%2.2dN", utm_zone);
		else
			sprintf(mbev_grid.projection_id, "UTM%2.2dS", utm_zone);
		proj_status = mb_proj_init(mbev_verbose,mbev_grid.projection_id,
			&(mbev_grid.pjptr), &mbev_error);
		if (proj_status != MB_SUCCESS)
			{
			mbev_status = MB_FAILURE;
			mbev_error = MB_ERROR_BAD_PARAMETER;
			}
		}

	/* get grid cell size and dimensions */
	if (mbev_status == MB_SUCCESS)
		{
		/* get projected bounds */

		/* first point */
		mb_proj_forward(mbev_verbose, mbev_grid.pjptr, mbev_grid.bounds[0], mbev_grid.bounds[2],
				&xx, &yy, &mbev_error);
		mbev_grid.boundsutm[0] = xx;
		mbev_grid.boundsutm[1] = xx;
		mbev_grid.boundsutm[2] = yy;
		mbev_grid.boundsutm[3] = yy;

		/* second point */
		mb_proj_forward(mbev_verbose, mbev_grid.pjptr, mbev_grid.bounds[1], mbev_grid.bounds[2],
				&xx, &yy, &mbev_error);
		mbev_grid.boundsutm[0] = MIN(mbev_grid.boundsutm[0], xx);
		mbev_grid.boundsutm[1] = MAX(mbev_grid.boundsutm[1], xx);
		mbev_grid.boundsutm[2] = MIN(mbev_grid.boundsutm[2], yy);
		mbev_grid.boundsutm[3] = MAX(mbev_grid.boundsutm[3], yy);

		/* third point */
		mb_proj_forward(mbev_verbose, mbev_grid.pjptr, mbev_grid.bounds[0], mbev_grid.bounds[3],
				&xx, &yy, &mbev_error);
		mbev_grid.boundsutm[0] = MIN(mbev_grid.boundsutm[0], xx);
		mbev_grid.boundsutm[1] = MAX(mbev_grid.boundsutm[1], xx);
		mbev_grid.boundsutm[2] = MIN(mbev_grid.boundsutm[2], yy);
		mbev_grid.boundsutm[3] = MAX(mbev_grid.boundsutm[3], yy);

		/* fourth point */
		mb_proj_forward(mbev_verbose, mbev_grid.pjptr, mbev_grid.bounds[1], mbev_grid.bounds[3],
				&xx, &yy, &mbev_error);
		mbev_grid.boundsutm[0] = MIN(mbev_grid.boundsutm[0], xx);
		mbev_grid.boundsutm[1] = MAX(mbev_grid.boundsutm[1], xx);
		mbev_grid.boundsutm[2] = MIN(mbev_grid.boundsutm[2], yy);
		mbev_grid.boundsutm[3] = MAX(mbev_grid.boundsutm[3], yy);

		/* get grid spacing */
		mbev_grid.dx = 0.14 * altitude_max;
		mbev_grid.dy = 0.14 * altitude_max;
		if (altitude_max > 0.0)
			{
			mbev_grid.dx = 0.02 * altitude_max;
			mbev_grid.dy = 0.02 * altitude_max;
			}
		else if (depth_max > 0.0)
			{
			mbev_grid.dx = 0.02 * depth_max;
			mbev_grid.dy = 0.02 * depth_max;
			}
		else
			{
			mbev_grid.dx = (mbev_grid.boundsutm[1] - mbev_grid.boundsutm[0]) / 250;
			mbev_grid.dy = (mbev_grid.boundsutm[1] - mbev_grid.boundsutm[0]) / 250;
			}

		/* get grid dimensions */
		mbev_grid.nx = (mbev_grid.boundsutm[1] - mbev_grid.boundsutm[0]) / mbev_grid.dx + 1;
		mbev_grid.ny = (mbev_grid.boundsutm[3] - mbev_grid.boundsutm[2]) / mbev_grid.dy + 1;
		mbev_grid.boundsutm[1] = mbev_grid.boundsutm[0] + (mbev_grid.nx - 1) * mbev_grid.dx;
		mbev_grid.boundsutm[3] = mbev_grid.boundsutm[2] + (mbev_grid.ny - 1) * mbev_grid.dy;
if (mbev_verbose > 0)
fprintf(stderr,"Grid bounds: %f %f %f %f    %f %f %f %f\n",
mbev_grid.bounds[0],mbev_grid.bounds[1],mbev_grid.bounds[2],mbev_grid.bounds[3],
mbev_grid.boundsutm[0],mbev_grid.boundsutm[1],mbev_grid.boundsutm[2],mbev_grid.boundsutm[3]);
if (mbev_verbose > 0)
fprintf(stderr,"cell size:%f %f dimensions: %d %d\n",
mbev_grid.dx,mbev_grid.dy,mbev_grid.nx,mbev_grid.ny);
		}

	/* allocate memory for grid */
	if (mbev_status == MB_SUCCESS)
		{
		if ((mbev_grid.sum = (float *) malloc(mbev_grid.nx * mbev_grid.ny * sizeof(float))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
		if ((mbev_grid.wgt = (float *) malloc(mbev_grid.nx * mbev_grid.ny * sizeof(float))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
		if ((mbev_grid.val = (float *) malloc(mbev_grid.nx * mbev_grid.ny * sizeof(float))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
		if ((mbev_grid.sgm = (float *) malloc(mbev_grid.nx * mbev_grid.ny * sizeof(float))) == NULL)
						mbev_error = MB_ERROR_MEMORY_FAIL;
		if (mbev_error == MB_ERROR_NO_ERROR)
			{
			memset(mbev_grid.sum, 0, mbev_grid.nx * mbev_grid.ny * sizeof(float));
			memset(mbev_grid.wgt, 0, mbev_grid.nx * mbev_grid.ny * sizeof(float));
			memset(mbev_grid.val, 0, mbev_grid.nx * mbev_grid.ny * sizeof(float));
			memset(mbev_grid.sgm, 0, mbev_grid.nx * mbev_grid.ny * sizeof(float));
			}
		else
			mbev_status = MB_FAILURE;
		}

	/* make grid */
	if (mbev_status == MB_SUCCESS)
		{
		/* loop over loaded files */
		filecount = 0;
		for (ifile=0;ifile<mbev_num_files;ifile++)
			{
			file = &mbev_files[ifile];
			if (file->load_status == MB_YES)
				{
				filecount++;
				sprintf(message, "Gridding file %d of %d...", filecount, mbev_num_files_loaded);
				do_mbeditviz_message_on(message);
				for (iping=0;iping<file->num_pings;iping++)
					{
					ping = &(file->pings[iping]);
					for (ibeam=0;ibeam<ping->beams_bath;ibeam++)
						{
						if (ping->beamflag[ibeam] != MB_FLAG_NULL)
							{
							mb_proj_forward(mbev_verbose, mbev_grid.pjptr,
									ping->bathlon[ibeam], ping->bathlat[ibeam],
									&ping->bathx[ibeam], &ping->bathy[ibeam],
									&mbev_error);
							}
						if (mb_beam_ok(ping->beamflag[ibeam]))
							{
							i = (ping->bathx[ibeam] - mbev_grid.boundsutm[0] + 0.5 * mbev_grid.dx)
								/ mbev_grid.dx;
							j = (ping->bathy[ibeam] - mbev_grid.boundsutm[2] + 0.5 * mbev_grid.dy)
								/ mbev_grid.dy;
							k = i * mbev_grid.ny + j;
							mbev_grid.sum[k] += (-ping->bathcorr[ibeam]);
							mbev_grid.wgt[k] += 1.0;
							mbev_grid.sgm[k] += ping->bathcorr[ibeam] * ping->bathcorr[ibeam];
							}
						}
					}
				}
			}
		mbev_grid.nodatavalue = MBEV_NODATA;
		first = MB_YES;
		for (i=0;i<mbev_grid.nx;i++)
			for (j=0;j<mbev_grid.ny;j++)
				{
				k = i * mbev_grid.ny + j;
				if (mbev_grid.wgt[k] > 0.0)
					{
					mbev_grid.val[k] = mbev_grid.sum[k] / mbev_grid.wgt[k];
					mbev_grid.sgm[k] = sqrt(fabs(mbev_grid.sgm[k] / mbev_grid.wgt[k]
								- mbev_grid.val[k] * mbev_grid.val[k]));
					if (first == MB_YES)
						{
						mbev_grid.min = mbev_grid.val[k];
						mbev_grid.max = mbev_grid.val[k];
						mbev_grid.smin = mbev_grid.sgm[k];
						mbev_grid.smax = mbev_grid.sgm[k];
						first = MB_NO;
						}
					else
						{
						mbev_grid.min = MIN(mbev_grid.min, mbev_grid.val[k]);
						mbev_grid.max = MAX(mbev_grid.max, mbev_grid.val[k]);
						mbev_grid.smin = MIN(mbev_grid.smin, mbev_grid.sgm[k]);
						mbev_grid.smax = MAX(mbev_grid.smax, mbev_grid.sgm[k]);
						}
					}
				else
					{
					mbev_grid.val[k] = mbev_grid.nodatavalue;
					mbev_grid.sgm[k] = mbev_grid.nodatavalue;
					}
				}
		mbev_grid.status = MBEV_GRID_NOTVIEWED;
		}

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status: %d\n",mbev_status);
		}

	/* return */
	return(mbev_status);
}
/*--------------------------------------------------------------------*/
int mbeditviz_destroy_grid()
{
	/* local variables */
	char	*function_name = "mbeditviz_destroy_grid";
	struct mbev_file_struct *file;
	struct mbev_ping_struct *ping;
	int	action;
	int	ifile, iping, ibeam;
	int	i;

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		}

if (mbev_verbose > 0)
fprintf(stderr,"mbeditviz_destroy_grid status:%d\n", mbev_status);

	/* loop over all files and output edits as necessary */
	for (ifile=0;ifile<mbev_num_files;ifile++)
		{
		file = &mbev_files[ifile];
if (mbev_verbose > 0)
fprintf(stderr,"ifile:%d load_status:%d esf_open:%d\n",
ifile,file->load_status,file->esf_open);
		if (file->load_status == MB_YES && file->esf_open == MB_YES)
			{
			for (iping=0;iping<file->num_pings;iping++)
				{
				ping = &(file->pings[iping]);
				for (ibeam=0;ibeam<ping->beams_bath;ibeam++)
					{
					if (ping->beamflag[ibeam] != ping->beamflagorg[ibeam])
						{
						if (mb_beam_ok(ping->beamflag[ibeam]))
							action = MBP_EDIT_UNFLAG;
						else if (mb_beam_check_flag_filter2(ping->beamflag[ibeam]))
							action = MBP_EDIT_FILTER;
						else if (mb_beam_check_flag_filter(ping->beamflag[ibeam]))
							action = MBP_EDIT_FILTER;
						else if (ping->beamflag[ibeam] != MB_FLAG_NULL)
							action = MBP_EDIT_FLAG;
						else
							action = MBP_EDIT_ZERO;
if (mbev_verbose > 0)
fprintf(stderr,"mb_esf_save: ifile:%d iping:%d ibeam:%d %d action:%d\n",
ifile,iping,ibeam, ibeam + ping->multiplicity * MB_ESF_MULTIPLICITY_FACTOR, action);
						mb_esf_save(mbev_verbose, &(file->esf),
								ping->time_d, ibeam + ping->multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
								action, &mbev_error);
						}
					}
				}

			/* update the process structure */
			file->process.mbp_edit_mode = MBP_EDIT_ON;
			strcpy(file->process.mbp_editfile, file->esf.esffile);

			/* close the esf file */
			mb_esf_close(mbev_verbose, &(file->esf), &mbev_error);
			file->esf_open = MB_NO;

			/* update mbprocess parameter file */
			mb_pr_writepar(mbev_verbose, file->path,
						&(file->process), &mbev_error);
			}
		}

	/* deallocate memory and reset status */
	if (mbev_grid.status != MBEV_GRID_NONE)
		{
		/* deallocate arrays */
		if (mbev_grid.sum != NULL)
			free(mbev_grid.sum);
		if (mbev_grid.wgt != NULL)
			free(mbev_grid.wgt);
		if (mbev_grid.val != NULL)
			free(mbev_grid.val);
		if (mbev_grid.sgm != NULL)
			free(mbev_grid.sgm);
		mbev_grid.sum = NULL;
		mbev_grid.wgt = NULL;
		mbev_grid.val = NULL;
		mbev_grid.sgm = NULL;

		/* release projection */
		mb_proj_free(mbev_verbose, &(mbev_grid.pjptr), &mbev_error);

		/* reset parameters */
		memset(mbev_grid.projection_id, 0, MB_PATH_MAXLINE);
		for (i=0;i<4;i++)
			{
			mbev_grid.bounds[i] = 0.0;
			mbev_grid.boundsutm[i] = 0.0;
			}
		mbev_grid.dx = 0.0;
		mbev_grid.dy = 0.0;
		mbev_grid.nx = 0;
		mbev_grid.ny = 0;

		/* reset status */
		mbev_grid.status = MBEV_GRID_NONE;
		}

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status: %d\n",mbev_status);
		}

	/* return */
	return(mbev_status);
}
/*--------------------------------------------------------------------*/
int mbeditviz_selectregion(size_t instance)
{
	char	*function_name = "mbeditviz_selectregion";
	struct mbev_file_struct *file;
	struct mbev_ping_struct *ping;
	struct mbview_struct *mbviewdata;
	struct mbview_region_struct *region;
	double	xmin, xmax, ymin, ymax, zmin, zmax;
	double	dx, dy, dz;
	double	x, y;
	double	xx, yy;
	double	heading, sonardepth;
	double	rolldelta, pitchdelta;
	double	headingx, headingy;
	double	mtodeglon, mtodeglat;
	int	i, ifile, iping, ibeam;

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:     %ld\n",instance);
		}

    	/* check data source for selected area */
	mbev_status = mbview_getdataptr(mbev_verbose, instance, &mbviewdata, &mbev_error);

	/* check if area is currently defined */
	if (mbev_status == MB_SUCCESS && mbviewdata->region_type == MBV_REGION_QUAD)
		{
		/* get area */
		region = (struct mbview_region_struct *) &mbviewdata->region;

		/* get region bounds */
if (mbev_verbose > 0)
fprintf(stderr,"mbeditviz_selectregion: rollbias:%f pitchbias:%f headingbias:%f timelag:%f\n",
mbev_rollbias_3dsdg, mbev_pitchbias_3dsdg, mbev_headingbias_3dsdg, mbev_timelag_3dsdg);
if (mbev_verbose > 0)
fprintf(stderr,"REGION: %f %f   %f %f   %f %f   %f %f\n",
region->cornerpoints[0].xgrid,region->cornerpoints[0].ygrid,
region->cornerpoints[1].xgrid,region->cornerpoints[2].ygrid,
region->cornerpoints[2].xgrid,region->cornerpoints[2].ygrid,
region->cornerpoints[3].xgrid,region->cornerpoints[3].ygrid);
		xmin = region->cornerpoints[0].xgrid;
		xmax = region->cornerpoints[0].xgrid;
		ymin = region->cornerpoints[0].ygrid;
		ymax = region->cornerpoints[0].ygrid;
		zmin = region->cornerpoints[0].zdata;
		zmax = region->cornerpoints[0].zdata;
		for (i=1;i<4;i++)
			{
			xmin = MIN(xmin, region->cornerpoints[i].xgrid);
			xmax = MAX(xmax, region->cornerpoints[i].xgrid);
			ymin = MIN(ymin, region->cornerpoints[i].ygrid);
			ymax = MAX(ymax, region->cornerpoints[i].ygrid);
			zmin = MIN(zmin, region->cornerpoints[i].zdata);
			zmax = MAX(zmax, region->cornerpoints[i].zdata);
			}

		/* get sounding bounds */
		mbev_selected.xorigin = 0.5 * (xmin + xmax);
		mbev_selected.yorigin = 0.5 * (ymin + ymax);
		mbev_selected.zorigin = 0.5 * (zmin + zmax);
		dx = xmax - xmin;
		dy = ymax - ymin;
		mbev_selected.xmin = -0.5 * dx;
		mbev_selected.ymin = -0.5 * dy;
		mbev_selected.xmax = 0.5 * dx;
		mbev_selected.ymax = 0.5 * dy;
		mbev_selected.bearing = 90.0;
		mbev_selected.sinbearing = sin(DTR * mbev_selected.bearing);
		mbev_selected.cosbearing = cos(DTR * mbev_selected.bearing);
		mbev_selected.scale = 2.0 / sqrt((xmax - xmin) * (xmax - xmin) + (ymax - ymin) * (ymax - ymin));
		mbev_selected.num_soundings = 0;
		mbev_selected.num_soundings_unflagged = 0;
		mbev_selected.num_soundings_flagged = 0;

		/* loop over all files */
		for (ifile=0;ifile<mbev_num_files;ifile++)
			{
			file = &mbev_files[ifile];
			if (file->load_status == MB_YES)
				{
				for (iping=0;iping<file->num_pings;iping++)
					{
					ping = &(file->pings[iping]);
					mbeditviz_apply_timelag(file, ping,
								mbev_rollbias_3dsdg, mbev_pitchbias_3dsdg, mbev_headingbias_3dsdg, mbev_timelag_3dsdg,
								&heading, &sonardepth,
								&rolldelta, &pitchdelta);
					mb_coor_scale(mbev_verbose,ping->navlat,&mtodeglon,&mtodeglat);
					headingx = sin(heading * DTR);
					headingy = cos(heading * DTR);
					for (ibeam=0;ibeam<ping->beams_bath;ibeam++)
						{
						if (ping->beamflag[ibeam] != MB_FLAG_NULL)
							{
							if (ping->bathx[ibeam] >= xmin
								&& ping->bathx[ibeam] <= xmax
								&& ping->bathy[ibeam] >= ymin
								&& ping->bathy[ibeam] <= ymax)
								{
								/* allocate memory if needed */
								if (mbev_selected.num_soundings
									>= mbev_selected.num_soundings_alloc)
									{
									mbev_selected.num_soundings_alloc += MBEV_ALLOCK_NUM;
									mbev_selected.soundings = realloc(mbev_selected.soundings, mbev_selected.num_soundings_alloc
													* sizeof(struct mb3dsoundings_sounding_struct));
									}

								/* same beam ids */
								mbev_selected.soundings[mbev_selected.num_soundings].ifile = ifile;
								mbev_selected.soundings[mbev_selected.num_soundings].iping = iping;
								mbev_selected.soundings[mbev_selected.num_soundings].ibeam = ibeam;
								mbev_selected.soundings[mbev_selected.num_soundings].beamflag = ping->beamflag[ibeam];

								/* apply rotations and recalculate position */
								mbeditviz_beam_position(ping->navlon, ping->navlat, headingx, headingy,
										mtodeglon, mtodeglat,
										ping->bath[ibeam], ping->bathacrosstrack[ibeam], ping->bathalongtrack[ibeam],
										sonardepth,
										rolldelta, pitchdelta,
										&(ping->bathcorr[ibeam]), &(ping->bathlon[ibeam]), &(ping->bathlat[ibeam]));
								mb_proj_forward(mbev_verbose, mbev_grid.pjptr,
									ping->bathlon[ibeam], ping->bathlat[ibeam],
									&ping->bathx[ibeam], &ping->bathy[ibeam],
									&mbev_error);

								/* get local position in selected region */
								x = ping->bathx[ibeam] - mbev_selected.xorigin;
								y = ping->bathy[ibeam] - mbev_selected.yorigin;
								xx = x * mbev_selected.sinbearing + y * mbev_selected.cosbearing;
								yy = -x * mbev_selected.cosbearing + y * mbev_selected.sinbearing;
								mbev_selected.soundings[mbev_selected.num_soundings].x = xx;
								mbev_selected.soundings[mbev_selected.num_soundings].y = yy;
									/*mbev_selected.soundings[mbev_selected.num_soundings].x
										= xx * mbev_selected.cosbearing - yy * mbev_selected.sinbearing;
									mbev_selected.soundings[mbev_selected.num_soundings].y
										= xx * mbev_selected.sinbearing + yy * mbev_selected.cosbearing;*/
								mbev_selected.soundings[mbev_selected.num_soundings].z
									= -ping->bathcorr[ibeam];
								if (mbev_selected.num_soundings == 0)
									{
									zmin = -ping->bathcorr[ibeam];
									zmax = -ping->bathcorr[ibeam];
									}
								else
									{
									zmin = MIN(zmin, -ping->bathcorr[ibeam]);
									zmax = MAX(zmax, -ping->bathcorr[ibeam]);
									}
/*fprintf(stderr,"SELECTED SOUNDING: %d %d %d  %f %f  |  %d %f %f %f\n",
ifile,iping,ibeam,ping->bathx[ibeam],ping->bathy[ibeam],
mbev_selected.num_soundings,
mbev_selected.soundings[mbev_selected.num_soundings].x,
mbev_selected.soundings[mbev_selected.num_soundings].y,
mbev_selected.soundings[mbev_selected.num_soundings].z);*/
								/* keep the counts right */
								mbev_selected.num_soundings++;
								if (mb_beam_ok(ping->beamflag[ibeam]))
									mbev_selected.num_soundings_unflagged++;
								else
									mbev_selected.num_soundings_flagged++;
								}
							}
						}
					}
				}
			}

		/* get zscaling */
		mbev_selected.zscale = mbev_selected.scale;
		dz = zmax - zmin;
		mbev_selected.zorigin = 0.5 * (zmin + zmax);
		mbev_selected.zmin = -0.5 * dz;
		mbev_selected.zmax = 0.5 * dz;
		for (i=0;i<mbev_selected.num_soundings;i++)
			mbev_selected.soundings[i].z = mbev_selected.soundings[i].z - mbev_selected.zorigin;
if (mbev_verbose > 0)
fprintf(stderr,"mbeditviz_selectregion: num_soundings:%d\n",
mbev_selected.num_soundings);
		}

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status:%d\n",mbev_status);
		}

	/* return status */
	return(mbev_status);
}
/*--------------------------------------------------------------------*/
int mbeditviz_selectarea(size_t instance)
{
	char	*function_name = "mbeditviz_selectarea";
	struct mbev_file_struct *file;
	struct mbev_ping_struct *ping;
	struct mbview_struct *mbviewdata;
	struct mbview_area_struct *area;
	int	ifile, iping, ibeam;
	double	x, y, xx, yy;
	double	zmin, zmax, dz;
	double	heading, sonardepth;
	double	rolldelta, pitchdelta;
	double	headingx, headingy;
	double	mtodeglon, mtodeglat;
	int	i;

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:     %ld\n",instance);
		}

    	/* check data source for selected area */
	mbev_status = mbview_getdataptr(mbev_verbose, instance, &mbviewdata, &mbev_error);

	/* check if area is currently defined */
	if (mbev_status == MB_SUCCESS && mbviewdata->area_type == MBV_AREA_QUAD)
		{
		/* get area */
		area = (struct mbview_area_struct *) &mbviewdata->area;
if (mbev_verbose > 0)
fprintf(stderr,"mbeditviz_selectarea: rollbias:%f pitchbias:%f headingbias:%f timelag:%f\n",
mbev_rollbias_3dsdg, mbev_pitchbias_3dsdg, mbev_headingbias_3dsdg, mbev_timelag_3dsdg);
if (mbev_verbose > 0)
fprintf(stderr,"AREA: %f %f   %f %f   %f %f   %f %f\n",
area->cornerpoints[0].xgrid,area->cornerpoints[0].ygrid,
area->cornerpoints[1].xgrid,area->cornerpoints[2].ygrid,
area->cornerpoints[2].xgrid,area->cornerpoints[2].ygrid,
area->cornerpoints[3].xgrid,area->cornerpoints[3].ygrid);

		/* get sounding bounds */
		mbev_selected.xorigin = 0.5 * (area->endpoints[0].xgrid + area->endpoints[1].xgrid);
		mbev_selected.yorigin = 0.5 * (area->endpoints[0].ygrid + area->endpoints[1].ygrid);;
		mbev_selected.zorigin = 0.5 * (area->endpoints[0].zdata + area->endpoints[1].zdata);;
		mbev_selected.xmin = -0.5 * area->length;
		mbev_selected.ymin = -0.5 * area->width;
		mbev_selected.xmax = 0.5 * area->length;
		mbev_selected.ymax = 0.5 * area->width;
		mbev_selected.bearing = area->bearing;
		mbev_selected.sinbearing = sin(DTR * mbev_selected.bearing);
		mbev_selected.cosbearing = cos(DTR * mbev_selected.bearing);
		mbev_selected.scale = 2.0 / sqrt(area->length * area->length + area->width * area->width);
		mbev_selected.num_soundings = 0;
		mbev_selected.num_soundings_unflagged = 0;
		mbev_selected.num_soundings_flagged = 0;

		/* loop over all files */
		for (ifile=0;ifile<mbev_num_files;ifile++)
			{
			file = &mbev_files[ifile];
			if (file->load_status == MB_YES)
				{
				for (iping=0;iping<file->num_pings;iping++)
					{
					ping = &(file->pings[iping]);
					mbeditviz_apply_timelag(file, ping,
								mbev_rollbias_3dsdg, mbev_pitchbias_3dsdg, mbev_headingbias_3dsdg, mbev_timelag_3dsdg,
								&heading, &sonardepth,
								&rolldelta, &pitchdelta);
					mb_coor_scale(mbev_verbose,ping->navlat,&mtodeglon,&mtodeglat);
					headingx = sin(heading * DTR);
					headingy = cos(heading * DTR);
					for (ibeam=0;ibeam<ping->beams_bath;ibeam++)
						{
						if (ping->beamflag[ibeam] != MB_FLAG_NULL)
							{
							x = ping->bathx[ibeam] - mbev_selected.xorigin;
							y = ping->bathy[ibeam] - mbev_selected.yorigin;
							yy = -x * mbev_selected.cosbearing + y * mbev_selected.sinbearing;
							xx = x * mbev_selected.sinbearing + y * mbev_selected.cosbearing;
							if (xx >= mbev_selected.xmin
								&& xx <= mbev_selected.xmax
								&& yy >= mbev_selected.ymin
								&& yy <= mbev_selected.ymax)
								{
								/* allocate memory if needed */
								if (mbev_selected.num_soundings
									>= mbev_selected.num_soundings_alloc)
									{
									mbev_selected.num_soundings_alloc += MBEV_ALLOCK_NUM;
									mbev_selected.soundings = realloc(mbev_selected.soundings, mbev_selected.num_soundings_alloc
													* sizeof(struct mb3dsoundings_sounding_struct));
									}

								/* same beam ids */
								mbev_selected.soundings[mbev_selected.num_soundings].ifile = ifile;
								mbev_selected.soundings[mbev_selected.num_soundings].iping = iping;
								mbev_selected.soundings[mbev_selected.num_soundings].ibeam = ibeam;
								mbev_selected.soundings[mbev_selected.num_soundings].beamflag = ping->beamflag[ibeam];

								/* apply rotations and recalculate position */
								mbeditviz_beam_position(ping->navlon, ping->navlat, headingx, headingy,
										mtodeglon, mtodeglat,
										ping->bath[ibeam], ping->bathacrosstrack[ibeam], ping->bathalongtrack[ibeam],
										sonardepth,
										rolldelta, pitchdelta,
										&(ping->bathcorr[ibeam]), &(ping->bathlon[ibeam]), &(ping->bathlat[ibeam]));
								mb_proj_forward(mbev_verbose, mbev_grid.pjptr,
									ping->bathlon[ibeam], ping->bathlat[ibeam],
									&ping->bathx[ibeam], &ping->bathy[ibeam],
									&mbev_error);
								x = ping->bathx[ibeam] - mbev_selected.xorigin;
								y = ping->bathy[ibeam] - mbev_selected.yorigin;
								yy = -x * mbev_selected.cosbearing + y * mbev_selected.sinbearing;
								xx = x * mbev_selected.sinbearing + y * mbev_selected.cosbearing;

								/* get local position in selected region */
								mbev_selected.soundings[mbev_selected.num_soundings].x = xx;
								mbev_selected.soundings[mbev_selected.num_soundings].y = yy;
								mbev_selected.soundings[mbev_selected.num_soundings].z
									= -ping->bathcorr[ibeam];
								if (mbev_selected.num_soundings == 0)
									{
									zmin = -ping->bathcorr[ibeam];
									zmax = -ping->bathcorr[ibeam];
									}
								else
									{
									zmin = MIN(zmin, -ping->bathcorr[ibeam]);
									zmax = MAX(zmax, -ping->bathcorr[ibeam]);
									}
/*fprintf(stderr,"SELECTED SOUNDING: %d %d %d  %f %f  |  %d %f %f %f\n",
ifile,iping,ibeam,ping->bathx[ibeam],ping->bathy[ibeam],
mbev_selected.num_soundings,
mbev_selected.soundings[mbev_selected.num_soundings].x,
mbev_selected.soundings[mbev_selected.num_soundings].y,
mbev_selected.soundings[mbev_selected.num_soundings].z);*/
								mbev_selected.num_soundings++;
								if (mb_beam_ok(ping->beamflag[ibeam]))
									mbev_selected.num_soundings_unflagged++;
								else
									mbev_selected.num_soundings_flagged++;
								}
							}
						}
					}
				}
			}

		/* get zscaling */
		mbev_selected.zscale = mbev_selected.scale;
		dz = zmax - zmin;
		mbev_selected.zorigin = 0.5 * (zmin + zmax);
		mbev_selected.zmin = -0.5 * dz;
		mbev_selected.zmax = 0.5 * dz;
		for (i=0;i<mbev_selected.num_soundings;i++)
			mbev_selected.soundings[i].z = mbev_selected.soundings[i].z - mbev_selected.zorigin;
if (mbev_verbose > 0)
fprintf(stderr,"mbeditviz_selectarea: num_soundings:%d\n",
mbev_selected.num_soundings);
		}

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status:%d\n",mbev_status);
		}

	/* return status */
	return(mbev_status);
}
/*--------------------------------------------------------------------*/
int mbeditviz_selectnav(size_t instance)
{
	char	*function_name = "mbeditviz_selectnav";
	struct mbev_file_struct *file;
	struct mbev_ping_struct *ping;
	struct mbview_shareddata_struct *mbviewshared;
	struct mbview_navpointw_struct *navpts;
	int	inavcount;
	int	ifile, iping, ibeam;
	double	dx, dy, dz;
	double	xmin, xmax, ymin, ymax, zmin, zmax;
	double	heading, sonardepth;
	double	rolldelta, pitchdelta;
	double	headingx, headingy;
	double	mtodeglon, mtodeglat;
	int	i;

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:     %ld\n",instance);
		}
if (mbev_verbose > 0)
fprintf(stderr,"mbeditviz_selectnav: \n");

    	/* check shared data source for selected nav */
	mbev_status = mbview_getsharedptr(mbev_verbose, &mbviewshared, &mbev_error);

	/* check if any nav is currently selected */
	if (mbev_status == MB_SUCCESS)
		{
		/* reset sounding count */
		mbev_selected.num_soundings = 0;
		mbev_selected.num_soundings_unflagged = 0;
		mbev_selected.num_soundings_flagged = 0;

		/* get sounding bearing */
		mbev_selected.bearing = 90.0;
		mbev_selected.sinbearing = sin(DTR * mbev_selected.bearing);
		mbev_selected.cosbearing = cos(DTR * mbev_selected.bearing);

		/* loop over all files to get bounds */
		inavcount = 0;
		for (ifile=0;ifile<mbev_num_files;ifile++)
			{
			file = &mbev_files[ifile];
			if (file->load_status == MB_YES)
				{
				navpts = (struct mbview_navpointw_struct *) mbviewshared->navs[inavcount].navpts;
				for (iping=0;iping<file->num_pings;iping++)
					{
					if (navpts[iping].selected == MB_YES)
						{
						ping = &(file->pings[iping]);
						mbeditviz_apply_timelag(file, ping,
								  	mbev_rollbias_3dsdg, mbev_pitchbias_3dsdg, mbev_headingbias_3dsdg, mbev_timelag_3dsdg,
								  	&heading, &sonardepth,
								  	&rolldelta, &pitchdelta);
						mb_coor_scale(mbev_verbose,ping->navlat,&mtodeglon,&mtodeglat);
						headingx = sin(heading * DTR);
						headingy = cos(heading * DTR);
						for (ibeam=0;ibeam<ping->beams_bath;ibeam++)
							{
							if (ping->beamflag[ibeam] != MB_FLAG_NULL)
								{
								/* allocate memory if needed */
								if (mbev_selected.num_soundings
									>= mbev_selected.num_soundings_alloc)
									{
									mbev_selected.num_soundings_alloc += MBEV_ALLOCK_NUM;
									mbev_selected.soundings = realloc(mbev_selected.soundings, mbev_selected.num_soundings_alloc
													* sizeof(struct mb3dsoundings_sounding_struct));
									}

								/* same beam ids */
								mbev_selected.soundings[mbev_selected.num_soundings].ifile = ifile;
								mbev_selected.soundings[mbev_selected.num_soundings].iping = iping;
								mbev_selected.soundings[mbev_selected.num_soundings].ibeam = ibeam;
								mbev_selected.soundings[mbev_selected.num_soundings].beamflag = ping->beamflag[ibeam];

								/* apply rotations and recalculate position */
								mbeditviz_beam_position(ping->navlon, ping->navlat, headingx, headingy,
										mtodeglon, mtodeglat,
										ping->bath[ibeam], ping->bathacrosstrack[ibeam], ping->bathalongtrack[ibeam],
										sonardepth,
										rolldelta, pitchdelta,
										&(ping->bathcorr[ibeam]), &(ping->bathlon[ibeam]), &(ping->bathlat[ibeam]));
								mb_proj_forward(mbev_verbose, mbev_grid.pjptr,
									ping->bathlon[ibeam], ping->bathlat[ibeam],
									&ping->bathx[ibeam], &ping->bathy[ibeam],
									&mbev_error);

								/* get local position in selected region */
								mbev_selected.soundings[mbev_selected.num_soundings].x = ping->bathx[ibeam];
								mbev_selected.soundings[mbev_selected.num_soundings].y = ping->bathy[ibeam];
								mbev_selected.soundings[mbev_selected.num_soundings].z = -ping->bathcorr[ibeam];
								if (mbev_selected.num_soundings == 0)
									{
									xmin = ping->bathx[ibeam];
									xmax = ping->bathx[ibeam];
									ymin = ping->bathy[ibeam];
									ymax = ping->bathy[ibeam];
									zmin = -ping->bathcorr[ibeam];
									zmax = -ping->bathcorr[ibeam];
									}
								else
									{
									xmin = MIN(xmin, ping->bathx[ibeam]);
									xmax = MAX(xmax, ping->bathx[ibeam]);
									ymin = MIN(ymin, ping->bathy[ibeam]);
									ymax = MAX(ymax, ping->bathy[ibeam]);
									zmin = MIN(zmin, -ping->bathcorr[ibeam]);
									zmax = MAX(zmax, -ping->bathcorr[ibeam]);
									}
/*fprintf(stderr,"SELECTED SOUNDING: %d %d %d  %f %f  |  %d %f %f %f\n",
ifile,iping,ibeam,ping->bathx[ibeam],ping->bathy[ibeam],
mbev_selected.num_soundings,
mbev_selected.soundings[mbev_selected.num_soundings].x,
mbev_selected.soundings[mbev_selected.num_soundings].y,
mbev_selected.soundings[mbev_selected.num_soundings].z);*/
								mbev_selected.num_soundings++;
								if (mb_beam_ok(ping->beamflag[ibeam]))
									mbev_selected.num_soundings_unflagged++;
								else
									mbev_selected.num_soundings_flagged++;
								}
							}
						}
					}

				inavcount++;
				}
			}

		/* get origin and scaling */
		dz = zmax - zmin;
		dx = xmax - xmin;
		dy = ymax - ymin;
		mbev_selected.xorigin = 0.5 * (xmin + xmax);
		mbev_selected.yorigin = 0.5 * (ymin + ymax);;
		mbev_selected.zorigin = 0.5 * (zmin + zmax);;
		mbev_selected.scale = 2.0 / sqrt(dy * dy + dx * dx);
		mbev_selected.zscale = mbev_selected.scale;
		mbev_selected.xmin = -0.5 * dx;
		mbev_selected.xmax = 0.5 * dx;
		mbev_selected.ymin = -0.5 * dy;
		mbev_selected.ymax = 0.5 * dy;
		mbev_selected.zmin = -0.5 * dz;
		mbev_selected.zmax = 0.5 * dz;
		for (i=0;i<mbev_selected.num_soundings;i++)
			{
			mbev_selected.soundings[i].x = mbev_selected.soundings[i].x - mbev_selected.xorigin;
			mbev_selected.soundings[i].y = mbev_selected.soundings[i].y - mbev_selected.yorigin;
			mbev_selected.soundings[i].z = mbev_selected.soundings[i].z - mbev_selected.zorigin;
			}
if (mbev_verbose > 0)
fprintf(stderr,"mbeditviz_selectarea: num_soundings:%d\n",
mbev_selected.num_soundings);
		}

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status:%d\n",mbev_status);
		}

	/* return status */
	return(mbev_status);
}
/*--------------------------------------------------------------------*/
void mbeditviz_mb3dsoundings_dismiss()
{
	char	*function_name = "mbeditviz_mb3dsoundings_dismiss";
if (mbev_verbose > 0)
fprintf(stderr,"mbeditviz_mb3dsoundings_dismiss\n");

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		}

	/* release the memory of the soundings */
	if (mbev_selected.num_soundings_alloc > 0)
		{
		if (mbev_selected.soundings != NULL)
			{
			free(mbev_selected.soundings);
			mbev_selected.soundings = NULL;
			}
		mbev_selected.xorigin = 0.0;
		mbev_selected.yorigin = 0.0;
		mbev_selected.zorigin = 0.0;
		mbev_selected.bearing = 0.0;
		mbev_selected.xmin = 0.0;
		mbev_selected.ymin = 0.0;
		mbev_selected.zmin = 0.0;
		mbev_selected.xmax = 0.0;
		mbev_selected.ymax = 0.0;
		mbev_selected.zmax = 0.0;
		mbev_selected.sinbearing = 0.0;
		mbev_selected.cosbearing = 0.0;
		mbev_selected.scale = 0.0;
		mbev_selected.zscale = 0.0;
		mbev_selected.num_soundings = 0;
		mbev_selected.num_soundings_unflagged = 0;
		mbev_selected.num_soundings_flagged = 0;
		mbev_selected.num_soundings_alloc = 0;
		}

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status:%d\n",mbev_status);
		}
}
/*--------------------------------------------------------------------*/
void mbeditviz_mb3dsoundings_edit(int ifile, int iping, int ibeam, char beamflag, int flush)
{
	char	*function_name = "mbeditviz_mb3dsoundings_edit";
	struct mbev_file_struct *file;
	struct mbev_ping_struct *ping;
	int	action;
/* fprintf(stderr,"mbeditviz_mb3dsoundings_edit:%d %d %d beamflag:%d flush:%d\n",
ifile, iping, ibeam, beamflag, flush); */

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       ifile:       %d\n",ifile);
		fprintf(stderr,"dbg2       iping:       %d\n",iping);
		fprintf(stderr,"dbg2       ibeam:       %d\n",ibeam);
		fprintf(stderr,"dbg2       beamflag:    %d\n",beamflag);
		fprintf(stderr,"dbg2       flush:       %d\n",flush);
		}

	/* apply current edit event */
	if (flush != MB3DSDG_EDIT_FLUSHPREVIOUS)
		{
		file = &mbev_files[ifile];
		ping = &(file->pings[iping]);

		/* check for real flag state change */
		if (mb_beam_ok(ping->beamflag[ibeam]) != mb_beam_ok(beamflag))
			{
			/* apply change to grid */
			mbeditviz_grid_beam(file, ping, ibeam, mb_beam_ok(beamflag), MB_YES);
			}

		/* output edits if desired */
		if (mbev_mode_output == MBEV_OUTPUT_MODE_EDIT)
			{
			/* open esf and ess files if not already open */
			if (file->esf_open == MB_NO)
				{
				mbev_status = mb_esf_load(mbev_verbose, file->path, MB_NO, MBP_ESF_APPEND,
								file->esffile, &(file->esf), &mbev_error);
				if (mbev_status == MB_SUCCESS)
					{
					file->esf_open = MB_YES;
					}
				else
					{
					file->esf_open = MB_NO;
					mbev_status = MB_SUCCESS;
					mbev_error = MB_ERROR_NO_ERROR;
					}
				}

			/* save the edits to the esf stream */
			if (file->esf_open == MB_YES)
				{
				if (mb_beam_ok(beamflag))
					action = MBP_EDIT_UNFLAG;
				else if (mb_beam_check_flag_filter2(beamflag))
					action = MBP_EDIT_FILTER;
				else if (mb_beam_check_flag_filter(beamflag))
					action = MBP_EDIT_FILTER;
				else if (beamflag != MB_FLAG_NULL)
					action = MBP_EDIT_FLAG;
				else
					action = MBP_EDIT_ZERO;
				mb_ess_save(mbev_verbose, &(file->esf),
						ping->time_d, ibeam + ping->multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
						action, &mbev_error);
				}
			}

		/* save new beamflag */
		ping->beamflag[ibeam] = beamflag;
		}

	/* redisplay grid if flush specified */
	if (flush !=  MB3DSDG_EDIT_NOFLUSH)
		{
		mbview_plothigh(0);
		}

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status:%d\n",mbev_status);
		}
}
/*--------------------------------------------------------------------*/
void mbeditviz_mb3dsoundings_info(int ifile, int iping, int ibeam, char *infostring)
{
	char	*function_name = "mbeditviz_mb3dsoundings_info";
	struct mbev_file_struct *file;
	struct mbev_ping_struct *ping;
if (mbev_verbose > 0)
fprintf(stderr,"mbeditviz_mb3dsoundings_info:%d %d %d\n",
ifile, iping, ibeam);

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       ifile:       %d\n",ifile);
		fprintf(stderr,"dbg2       iping:       %d\n",iping);
		fprintf(stderr,"dbg2       ibeam:       %d\n",ibeam);
		}

	/* generate info string */
	file = &mbev_files[ifile];
	ping = &(file->pings[iping]);
	sprintf(infostring,"Beam %d of %d   Ping %d of %d   File:%s\nPing Time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %f\nLon:%.6f Lat:%.6f Depth:%.3f X:%.3f L:%.3f",
		ibeam,ping->beams_bath,iping,file->num_pings,file->name,ping->time_i[0],ping->time_i[1],ping->time_i[2],
		ping->time_i[3],ping->time_i[4],ping->time_i[5],ping->time_i[6],ping->time_d,
		ping->bathlon[ibeam],ping->bathlat[ibeam],ping->bath[ibeam],ping->bathacrosstrack[ibeam],ping->bathalongtrack[ibeam]);

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2       infostring: %s\n",infostring);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status:%d\n",mbev_status);
		}
}
/*--------------------------------------------------------------------*/
void mbeditviz_mb3dsoundings_bias(double rollbias, double pitchbias, double headingbias, double timelag)
{
	char	*function_name = "mbeditviz_mb3dsoundings_bias";
	struct mbev_file_struct *file;
	struct mbev_ping_struct *ping;
	int	ifile, iping, ibeam;
	double	x, y, xx, yy;
	double	zmin, zmax, dz;
	double	heading, sonardepth;
	double	rolldelta, pitchdelta;
	double	headingx, headingy;
	double	mtodeglon, mtodeglat;
	int	ifilelast, ipinglast;
	int	i;

if (mbev_verbose > 0)
fprintf(stderr,"mbeditviz_mb3dsoundings_bias:%f %f %f %f\n",
rollbias, pitchbias, headingbias, timelag);

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rollbias:    %f\n",rollbias);
		fprintf(stderr,"dbg2       pitchbias:   %f\n",pitchbias);
		fprintf(stderr,"dbg2       headingbias: %f\n",headingbias);
		fprintf(stderr,"dbg2       timelag:     %f\n",timelag);
		}

	/* copy bias parameters */
	mbev_rollbias_3dsdg = rollbias;
	mbev_pitchbias_3dsdg = pitchbias;
	mbev_headingbias_3dsdg = headingbias;
	mbev_timelag_3dsdg = timelag;
	ifilelast = -1;
	ipinglast = -1;

	/* apply bias parameters */
	for (i=0;i<mbev_selected.num_soundings;i++)
		{
		ifile = mbev_selected.soundings[i].ifile;
		iping = mbev_selected.soundings[i].iping;
		ibeam = mbev_selected.soundings[i].ibeam;
		file = &mbev_files[ifile];
		ping = &(file->pings[iping]);

		if (ifile != ifilelast || iping != ipinglast)
			{
			mbeditviz_apply_timelag(file, ping,
					mbev_rollbias_3dsdg, mbev_pitchbias_3dsdg, mbev_headingbias_3dsdg, mbev_timelag_3dsdg,
					&heading, &sonardepth,
					&rolldelta, &pitchdelta);
			mb_coor_scale(mbev_verbose,ping->navlat,&mtodeglon,&mtodeglat);
			headingx = sin(heading * DTR);
			headingy = cos(heading * DTR);
			ifilelast = ifile;
			ipinglast = iping;
			}

		/* apply rotations and recalculate position */
		mbeditviz_beam_position(ping->navlon, ping->navlat, headingx, headingy,
				mtodeglon, mtodeglat,
				ping->bath[ibeam], ping->bathacrosstrack[ibeam], ping->bathalongtrack[ibeam],
				sonardepth,
				rolldelta, pitchdelta,
				&(ping->bathcorr[ibeam]), &(ping->bathlon[ibeam]), &(ping->bathlat[ibeam]));
		mb_proj_forward(mbev_verbose, mbev_grid.pjptr,
			ping->bathlon[ibeam], ping->bathlat[ibeam],
			&ping->bathx[ibeam], &ping->bathy[ibeam],
			&mbev_error);
		x = ping->bathx[ibeam] - mbev_selected.xorigin;
		y = ping->bathy[ibeam] - mbev_selected.yorigin;
		xx = x * mbev_selected.sinbearing + y * mbev_selected.cosbearing;
		yy = -x * mbev_selected.cosbearing + y * mbev_selected.sinbearing;

		/* get local position in selected region */
		mbev_selected.soundings[i].x = xx;
		mbev_selected.soundings[i].y = yy;
		mbev_selected.soundings[i].z = -ping->bathcorr[ibeam];
		if (i == 0)
			{
			zmin = -ping->bathcorr[ibeam];
			zmax = -ping->bathcorr[ibeam];
			}
		else
			{
			zmin = MIN(zmin, -ping->bathcorr[ibeam]);
			zmax = MAX(zmax, -ping->bathcorr[ibeam]);
			}
		}

	/* get zscaling */
	mbev_selected.zscale = mbev_selected.scale;
	dz = zmax - zmin;
	mbev_selected.zorigin = 0.5 * (zmin + zmax);
	mbev_selected.zmin = -0.5 * dz;
	mbev_selected.zmax = 0.5 * dz;
	for (i=0;i<mbev_selected.num_soundings;i++)
		mbev_selected.soundings[i].z = mbev_selected.soundings[i].z - mbev_selected.zorigin;

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status:%d\n",mbev_status);
		}
}
/*--------------------------------------------------------------------*/
void mbeditviz_mb3dsoundings_biasapply(double rollbias, double pitchbias, double headingbias, double timelag)
{
	char	*function_name = "mbeditviz_mb3dsoundings_biasapply";
	struct mbev_file_struct *file;
	struct mbev_ping_struct *ping;
	int	ifile, iping, ibeam;
	double	heading, sonardepth;
	double	rolldelta, pitchdelta;
	double	headingx, headingy;
	double	mtodeglon, mtodeglat;

if (mbev_verbose > 0)
fprintf(stderr,"mbeditviz_mb3dsoundings_biasapply:%f %f %f %f\n",
rollbias, pitchbias, headingbias, timelag);

	/* print input debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rollbias:    %f\n",rollbias);
		fprintf(stderr,"dbg2       pitchbias:   %f\n",pitchbias);
		fprintf(stderr,"dbg2       headingbias: %f\n",headingbias);
		fprintf(stderr,"dbg2       timelag:     %f\n",timelag);
		}

	/* copy bias parameters */
	mbev_rollbias = rollbias;
	mbev_pitchbias = pitchbias;
	mbev_headingbias = headingbias;
	mbev_timelag = timelag;

	/* turn message on */
	sprintf(message, "Regridding using new bias parameters %f %f %f %f\n",
				mbev_rollbias, mbev_pitchbias, mbev_headingbias, mbev_timelag);
	do_mbeditviz_message_on(message);

	/* apply bias parameters to swath data */
	for (ifile=0;ifile<mbev_num_files;ifile++)
		{
		file = &mbev_files[ifile];
		if (file->load_status == MB_YES)
			{
			for (iping=0;iping<file->num_pings;iping++)
				{
				ping = &(file->pings[iping]);
				mbeditviz_apply_timelag(file, ping,
							mbev_rollbias_3dsdg, mbev_pitchbias_3dsdg, mbev_headingbias_3dsdg, mbev_timelag_3dsdg,
							&heading, &sonardepth,
							&rolldelta, &pitchdelta);
				mb_coor_scale(mbev_verbose,ping->navlat,&mtodeglon,&mtodeglat);
				headingx = sin(heading * DTR);
				headingy = cos(heading * DTR);
				for (ibeam=0;ibeam<ping->beams_bath;ibeam++)
					{
					/* apply rotations and recalculate position */
					mbeditviz_beam_position(ping->navlon, ping->navlat, headingx, headingy,
							mtodeglon, mtodeglat,
							ping->bath[ibeam], ping->bathacrosstrack[ibeam], ping->bathalongtrack[ibeam],
							sonardepth,
							rolldelta, pitchdelta,
							&(ping->bathcorr[ibeam]), &(ping->bathlon[ibeam]), &(ping->bathlat[ibeam]));
					mb_proj_forward(mbev_verbose, mbev_grid.pjptr,
						ping->bathlon[ibeam], ping->bathlat[ibeam],
						&ping->bathx[ibeam], &ping->bathy[ibeam],
						&mbev_error);
					}
				}
			}
		}

	/* recalculate grid */
	mbeditviz_make_grid();

	/* update the grid to mbview */
	mbview_updateprimarygrid(mbev_verbose, 0, mbev_grid.nx, mbev_grid.ny, mbev_grid.val, &mbev_error);
	mbview_updatesecondarygrid(mbev_verbose, 0, mbev_grid.nx, mbev_grid.ny, mbev_grid.sgm, &mbev_error);

	/* turn message of */
	do_mbeditviz_message_off();

	/* redisplay grid */
	mbview_plothigh(0);

	/* print output debug statements */
	if (mbev_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",mbev_error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       mbev_status:%d\n",mbev_status);
		}
}
/*--------------------------------------------------------------------*/

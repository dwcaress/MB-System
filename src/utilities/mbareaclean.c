/*--------------------------------------------------------------------
 *    The MB-system:	mbareaclean.c	2/27/2003
 *    $Id: mbareaclean.c,v 5.1 2003-04-17 21:17:10 caress Exp $
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
 * Revision 5.0  2003/03/10 20:47:08  caress
 * Initial version.
 *
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

/* allocation */
#define FILEALLOCNUM	16
#define PINGALLOCNUM	128
#define SNDGALLOCNUM	128

struct mbareaclean_file_struct {
	char	filelist[MB_PATH_MAXLINE];
	int	file_format;
	int	nping;
	int	nping_alloc;
	int	nnull;
	int	nflag;
	int	ngood;
	int	nunflagged;
	int	nflagged;
	double	*ping_time_d;
	double	*ping_altitude;
	int	nsndg;
	int	nsndg_alloc;
	int	sndg_countstart;
	struct mbareaclean_sndg_struct *sndg;
	};
struct mbareaclean_sndg_struct {
	int	sndg_file;
	int	sndg_ping;
	int	sndg_beam;
	double	sndg_depth;
	double	sndg_x;
	double	sndg_y;
	char	sndg_beamflag_org;
	char	sndg_beamflag_esf;
	char	sndg_beamflag;
	};
	
/* sounding atorage values and arrays */
int	nfile = 0;
int	nfile_alloc = 0;
struct mbareaclean_file_struct 	*files = NULL;
int	nsndg = 0;
int	nsndg_alloc = 0;
int	sndg_countstart = 0;
int	**gsndg = NULL;
int	*gsndgnum = NULL;
int	*gsndgnum_alloc = NULL;
struct mbareaclean_sndg_struct *sndg = NULL;

/* sounding pointer resolving function */
int getsoundingptr(int verbose, int soundingid, 
		struct mbareaclean_sndg_struct **sndgptr, 
		int *error);

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	static char rcs_id[] = "$Id: mbareaclean.c,v 5.1 2003-04-17 21:17:10 caress Exp $";
	static char program_name[] = "MBAREACLEAN";
	static char help_message[] =  "MBAREACLEAN identifies and flags artifacts in swath bathymetry data";
	static char usage_message[] = "mbareaclean [-Fformat -Iinfile -Rwest/east/south/north -B -G -Mthreshold/nmin -Sbinsize]";
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
	char	swathfileread[MB_PATH_MAXLINE];
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	int	read_data;
	double	file_weight;
	int	format;
	int	formatread;
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
	int	pingsread;
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
	char	*beamflagesf;
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
	double	median_filter_threshold = 0.25;
	int	median_filter_nmin = 10;
	int	plane_fit = MB_NO;
	double	plane_fit_threshold = 0.05;
	int	plane_fit_nmin = 10;
	int	output_good = MB_NO;
	int	output_bad = MB_NO;
	double	areabounds[4];
	double	binsize;
	double	dx, dy;
	int	nx, ny;
	double	mtodeglon;
	double	mtodeglat;
	
	/* median filter parameters */
	int	binnum;
	int	binnummax;
	double	*bindepths;
	double	threshold;
	double	median_depth;
	
	/* counting parameters */
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
	int	esffile_open = MB_NO;
	char	esffile[MB_PATH_MAXLINE];
	struct mb_esf_struct esf;
	char	notice[MB_PATH_MAXLINE];
	int	action;

	double	xx, yy;
	int	done;
	int	ix, iy, ib, kgrid;
	int	d1, d2;
	int	i1, n;
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

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhBbGgF:f:I:i:M:m:P:p:S:s:R:r:")) != -1)
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
			n = sscanf (optarg,"%lf/%d/%lf", 
					&d1,&i1,&d2);
			if (n > 0) median_filter_threshold = d1;
			if (n > 1) median_filter_nmin = i1;
			flag++;
			break;
		case 'P':
		case 'p':
			plane_fit = MB_YES;
			sscanf (optarg,"%lf", &plane_fit_threshold);
			n = sscanf (optarg,"%lf/%d/%lf", 
					&d1,&i1,&d2);
			if (n > 0) plane_fit_threshold = d1;
			if (n > 1) plane_fit_nmin = i1;
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
		fprintf(stderr,"dbg2       median_filter:             %d\n",median_filter);
		fprintf(stderr,"dbg2       median_filter_threshold:   %f\n",median_filter_threshold);
		fprintf(stderr,"dbg2       median_filter_nmin:        %d\n",median_filter_nmin);
		fprintf(stderr,"dbg2       plane_fit:                 %d\n",plane_fit);
		fprintf(stderr,"dbg2       plane_fit_threshold:       %f\n",plane_fit_threshold);
		fprintf(stderr,"dbg2       plane_fit_nmin:            %d\n",plane_fit_nmin);
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

	/* calculate grid properties */
	mb_coor_scale(verbose,0.5*(areabounds[2]+areabounds[3]),&mtodeglon,&mtodeglat);
	dx = binsize * mtodeglon;
	dy = binsize * mtodeglat;
	nx = 1 + (int)((areabounds[1] - areabounds[0]) / dx);
	ny = 1 + (int)((areabounds[3] - areabounds[2]) / dy);
	if (nx > 1 && ny > 1)
		{
		dx = (areabounds[1] - areabounds[0]) / (nx - 1);
		dy = (areabounds[3] - areabounds[2]) / (ny - 1);
		}
	
	/* allocate grid arrays */
	nsndg = 0;
	nsndg_alloc = 0;
	status = mb_malloc(verbose, nx * ny * sizeof(int *),
			&gsndg, &error);
	if (status == MB_SUCCESS)
	status = mb_malloc(verbose, nx * ny * sizeof(int),
			&gsndgnum, &error);
	if (status == MB_SUCCESS)
	status = mb_malloc(verbose, nx * ny * sizeof(int),
			&gsndgnum_alloc, &error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* if error initializing memory then quit */
	for (i=0;i<nx*ny;i++)
		{
		gsndg[i] = NULL;
		gsndgnum[i] = 0;
		gsndgnum_alloc[i] = 0;
		}

	/* give the statistics */
	if (verbose >= 0)
		{
		fprintf(stderr,"Area of interest:\n");
		fprintf(stderr,"     Minimum Longitude: %.6f Maximum Longitude: %.6f\n",
			areabounds[0],areabounds[1]);
		fprintf(stderr,"     Minimum Latitude:  %.6f Maximum Latitude:  %.6f\n",
			areabounds[2],areabounds[3]);
		fprintf(stderr,"     Bin Size:   %f\n", binsize);
		fprintf(stderr,"     Dimensions: %d %d\n", nx, ny);
		fprintf(stderr,"Cleaning algorithms:\n");
		if (median_filter == MB_YES)
			{
			fprintf(stderr,"     Median filter: ON\n");
			fprintf(stderr,"     Median filter threshold:    %f\n");
			fprintf(stderr,"     Median filter minimum N:    %d\n");
			fprintf(stderr,"     Median filter max fraction: %f\n");
			}
		else
			fprintf(stderr,"     Median filter: OFF\n");
		if (plane_fit == MB_YES)
			{
			fprintf(stderr,"     Plane fit:     ON\n");
			fprintf(stderr,"     Plane fit threshold:        %f\n");
			fprintf(stderr,"     Plane fit minimum N:        %d\n");
			fprintf(stderr,"     Plane fit max fraction:     %f\n");
			}
		else
			fprintf(stderr,"     Plane fit:     OFF\n");
		fprintf(stderr,"Output:\n");
		if (output_bad == MB_YES)
			fprintf(stderr,"     Flag unflagged soundings identified as bad:  ON\n");
		else
			fprintf(stderr,"     Flag unflagged soundings identified as bad:  OFF\n");
		if (output_good == MB_YES)
			fprintf(stderr,"     Unflag flagged soundings identified as good: ON\n");
		else
			fprintf(stderr,"     Unflag flagged soundings identified as good: OFF\n");
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

	/* check for "fast bathymetry" or "fbt" file */
	strcpy(swathfileread, swathfile);
	formatread = format;
	mb_get_fbt(verbose, swathfileread, &formatread, &error);

	/* initialize reading the input swath sonar file */
	if ((status = mb_read_init(
		verbose,swathfileread,formatread,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&mbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",swathfileread);
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
		fprintf(stderr,"\nProcessing %s\n",swathfileread);
		}

	/* allocate memory for data arrays */
	status = mb_malloc(verbose, beams_bath * sizeof(char),
			&beamflag, &error);
	status = mb_malloc(verbose, beams_bath * sizeof(char),
			&beamflagesf, &error);
	status = mb_malloc(verbose, beams_bath * sizeof(double),
			&bath, &error);
	status = mb_malloc(verbose, beams_amp * sizeof(double),
			&amp, &error);
	status = mb_malloc(verbose, beams_bath * sizeof(double),
			&bathlon, &error);
	status = mb_malloc(verbose, beams_bath * sizeof(double),
			&bathlat, &error);
	status = mb_malloc(verbose, pixels_ss * sizeof(double),
			&ss, &error);
	status = mb_malloc(verbose, pixels_ss * sizeof(double),
			&sslon, &error);
	status = mb_malloc(verbose, pixels_ss * sizeof(double),
			&sslat, &error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	
	/* update memory for files */
	if (nfile >= nfile_alloc)
		{
		nfile_alloc += FILEALLOCNUM;
		status = mb_realloc(verbose,
				nfile_alloc * sizeof(struct mbareaclean_file_struct),
				&files, &error);

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}	
		}
		
	/* initialize current file */
	strcpy(files[nfile].filelist, swathfile);
	files[nfile].file_format = format;
	files[nfile].nping = 0;
	files[nfile].nping_alloc = PINGALLOCNUM;
	files[nfile].nnull = 0;
	files[nfile].nflag = 0;
	files[nfile].ngood = 0;
	files[nfile].nflagged = 0;
	files[nfile].nunflagged = 0;
	files[nfile].ping_time_d = NULL;
	files[nfile].ping_altitude = NULL;
	files[nfile].nsndg = 0;
	files[nfile].nsndg_alloc = SNDGALLOCNUM;
	files[nfile].sndg_countstart = nsndg;
	files[nfile].sndg = NULL;
	status = mb_malloc(verbose,
			files[nfile].nping_alloc * sizeof(double),
			&(files[nfile].ping_time_d), &error);
	if (status == MB_SUCCESS)
	status = mb_malloc(verbose,
			files[nfile].nping_alloc * sizeof(double),
			&(files[nfile].ping_altitude), &error);
	if (status == MB_SUCCESS)
	status = mb_malloc(verbose,
			files[nfile].nsndg_alloc * sizeof(struct mbareaclean_sndg_struct),
			&(files[nfile].sndg), &error);
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	nfile++;

	/* now deal with old edit save file */
	if (status == MB_SUCCESS)
	    {
	    /* handle esf edits */
	    status = mb_esf_load(verbose, swathfile, 
			    MB_YES, MB_NO, esffile, &esf, &error);
	    }

	/* read */
	done = MB_NO;
	files_tot++;
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
			    &pingsread,time_i,&time_d,
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
		for (i=0;i<beams_bath;i++)
			beamflagesf[i] = beamflag[i];
		status = mb_esf_apply(verbose, &esf, 
		    		time_d, beams_bath, 
				beamflagesf, &error);
		
		/* update counters */
		pings_tot++;
		pings_file++;
		for (i=0;i<beams_bath;i++)
			{
			if (mb_beam_ok(beamflagesf[i]))
				{
				beams_tot++;
				beams_file++;
				beams_good_org_tot++;
				beams_good_org_file++;
				files[nfile-1].ngood++;
				}
			else if (beamflagesf[i] == MB_FLAG_NULL)
				{
				beams_null_org_tot++;
				beams_null_org_file++;
				files[nfile-1].nnull++;
				}
			else
				{
				beams_tot++;
				beams_file++;
				beams_flag_org_tot++;
				beams_flag_org_file++;
				files[nfile-1].nflag++;
				}
			}
			
		/* allocate memory if necessary */
		if (files[nfile-1].nping >= files[nfile-1].nping_alloc)
			{
			files[nfile-1].nping_alloc += PINGALLOCNUM;
			status = mb_realloc(verbose,
					files[nfile-1].nping_alloc * sizeof(double),
					&(files[nfile-1].ping_time_d), &error);
			if (status == MB_SUCCESS)
			status = mb_realloc(verbose,
					files[nfile-1].nping_alloc * sizeof(double),
					&(files[nfile-1].ping_altitude), &error);
			if (error != MB_ERROR_NO_ERROR)
				{
				mb_error(verbose,error,&message);
				fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
				fprintf(stderr,"\nProgram <%s> Terminated\n",
					program_name);
				exit(error);
				}
			}
			
		/* store the ping data */
		files[nfile-1].ping_time_d[files[nfile-1].nping] = time_d;
		files[nfile-1].ping_altitude[files[nfile-1].nping] = altitude;
		files[nfile-1].nping++;
		
		/* now loop over the beams and store the soundings in the grid bins */
		for (ib=0;ib<beams_bath;ib++)
			{
			if (beamflagesf[ib] != MB_FLAG_NULL)
				{
				/* get bin for current beam */
				ix = (bathlon[ib] - areabounds[0] - 0.5 * dx) / dx;
				iy = (bathlat[ib] - areabounds[2] - 0.5 * dy) / dy;
				kgrid = ix*ny + iy;
				
				/* add sounding */
				if (ix >= 0 && ix < nx 
					&& iy >= 0 && iy < ny)
			        	{
					if (files[nfile-1].nsndg >= files[nfile-1].nsndg_alloc)
						{
						files[nfile-1].nsndg_alloc += SNDGALLOCNUM;
						status = mb_realloc(verbose,
							files[nfile-1].nsndg_alloc * sizeof(struct mbareaclean_sndg_struct),
							&files[nfile-1].sndg, &error);
						if (error != MB_ERROR_NO_ERROR)
							{
							mb_error(verbose,error,&message);
							fprintf(stderr,"\nMBIO Error allocating sounding arrays:\n%s\n",message);
							fprintf(stderr,"\nProgram <%s> Terminated\n",
								program_name);
							exit(error);
							}
						}
					
					
					/* allocate space for sounding if needed */
					if (gsndgnum[kgrid] >= gsndgnum_alloc[kgrid])
						{
						gsndgnum_alloc[kgrid] += SNDGALLOCNUM;
						status = mb_realloc(verbose,
							gsndgnum_alloc[kgrid] * sizeof(int),
							&gsndg[kgrid], &error);
						if (error != MB_ERROR_NO_ERROR)
							{
							mb_error(verbose,error,&message);
							fprintf(stderr,"\nMBIO Error allocating sounding arrays:\n%s\n",message);
							fprintf(stderr,"\nProgram <%s> Terminated\n",
								program_name);
							exit(error);
							}
						}
					
					/* store sounding data */
					sndg = &(files[nfile-1].sndg[files[nfile-1].nsndg]);
					sndg->sndg_file = nfile - 1;
					sndg->sndg_ping = files[nfile - 1].nping - 1;
					sndg->sndg_beam = ib;
					sndg->sndg_depth = bath[ib];
					sndg->sndg_x = bathlon[ib];
					sndg->sndg_y = bathlat[ib];
					sndg->sndg_beamflag_org = beamflag[ib];
					sndg->sndg_beamflag_esf = beamflagesf[ib];
					sndg->sndg_beamflag = beamflagesf[ib];
					files[nfile-1].nsndg++;
					nsndg++;
					gsndg[kgrid][gsndgnum[kgrid]] 
						= files[nfile-1].sndg_countstart 
							+ files[nfile-1].nsndg - 1;
					gsndgnum[kgrid]++;
/*fprintf(stderr,"NEW sounding:%d:%d file:%d time_d:%f depth:%f\n",
nsndg-1,gsndg[kgrid][gsndgnum[kgrid]-1],sndg->sndg_file,
files[sndg->sndg_file].ping_time_d[sndg->sndg_ping], sndg->sndg_depth);*/
					}
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
	mb_esf_close(verbose, &esf, &error);
		

	/* free the memory */
	mb_free(verbose,&beamflag,&error); 
	mb_free(verbose,&beamflagesf,&error); 
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
		fprintf(stderr,"pings:%4d  beams: %7d good %7d flagged %7d null \n",
				pings_file,beams_good_org_file
				,beams_flag_org_file,beams_null_org_file);
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
		
	/* loop over grid cells to find maximum number of soundings */
	binnummax = 0;
	for (ix=0;ix<nx;ix++)
	for (iy=0;iy<ny;iy++)
		{
		/* get cell id */
		kgrid = ix*ny + iy;
		xx = areabounds[0] + 0.5 * dx + ix * dx;
		yy = areabounds[3] + 0.5 * dy + iy * dy;
		binnummax = MAX(binnummax, gsndgnum[kgrid]);
		}
	status = mb_malloc(verbose,
			binnummax * sizeof(double),
			&(bindepths), &error);
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating sounding sorting array:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* deal with median filter */
	if (median_filter == MB_YES)
	{
	/* loop over grid cells applying median filter test */
	for (ix=0;ix<nx;ix++)
	for (iy=0;iy<ny;iy++)
		{
		/* get cell id */
		kgrid = ix*ny + iy;
		xx = areabounds[0] + 0.5 * dx + ix * dx;
		yy = areabounds[3] + 0.5 * dy + iy * dy;
		
		/* load up array */
		binnum = 0;
		for (i=0;i<gsndgnum[kgrid];i++)
			{
			getsoundingptr(verbose, gsndg[kgrid][i], &sndg, &error);
			if (mb_beam_ok(sndg->sndg_beamflag))
				{
				bindepths[binnum] = sndg->sndg_depth;
				binnum++;
				}
/*fprintf(stderr,"ix:%d iy:%d kgrid:%d soundingid:%d beamflag:%d   binnum:%d\n",
ix,iy,kgrid,gsndg[kgrid][i],sndg->sndg_beamflag,binnum);*/
			}
			
		/* apply median filter only if there are enough soundings */
		if (binnum >= median_filter_nmin)
			{
			/* run qsort */
			qsort((char *)bindepths,binnum,sizeof(double),
				mb_double_compare);
			median_depth = bindepths[binnum / 2];
/*if (binnum>0)
fprintf(stderr,"bin: %d %d %d  pos: %f %f  nsoundings:%d median:%f\n",
ix,iy,kgrid,xx,yy,binnum,median_depth);*/

			/* process the soundings */
			for (i=0;i<gsndgnum[kgrid];i++)
				{
				getsoundingptr(verbose, gsndg[kgrid][i], &sndg, &error);
				threshold = median_filter_threshold 
						* files[sndg->sndg_file].ping_altitude[sndg->sndg_ping];
/*fprintf(stderr,"sounding:%d file:%d time_d:%f depth:%f\n",
gsndg[kgrid][i],sndg->sndg_file,
files[sndg->sndg_file].ping_time_d[sndg->sndg_ping], sndg->sndg_depth);*/
				if (output_bad == MB_YES
					&& mb_beam_ok(sndg->sndg_beamflag)
					&& fabs(sndg->sndg_depth - median_depth) 
						> threshold)
					{
					sndg->sndg_beamflag = MB_FLAG_FLAG + MB_FLAG_FILTER;
					files[sndg->sndg_file].nflagged++;
					} 
				else if (output_good == MB_YES
					&& !mb_beam_ok(sndg->sndg_beamflag)
					&& sndg->sndg_beamflag != MB_FLAG_NULL
					&& fabs(sndg->sndg_depth - median_depth) 
						<= threshold)
					{
					sndg->sndg_beamflag = MB_FLAG_NONE;
					files[sndg->sndg_file].nunflagged++;
					}
				}
			}
		}
	}

	/* loop over files checking for changed soundings */
	for (i=0; i < nfile; i++)
		{
		/* open esf file */
	    	status = mb_esf_load(verbose, files[i].filelist, 
			    MB_NO, MB_YES, esffile, &esf, &error);
	    	if (status == MB_SUCCESS
		    	&& esf.esffp != NULL)
		    	esffile_open = MB_YES;
	    	if (status == MB_FAILURE 
		    	&& error == MB_ERROR_OPEN_FAIL)
		    	{
		    	esffile_open = MB_NO;
		    	fprintf(stderr, "\nUnable to open new edit save file %s\n", 
					esf.esffile);
		    }
		
		/* loop over all of the soundings */
		for (j=0;j<files[i].nsndg;j++)
			{
			sndg = &(files[i].sndg[j]);
			if (sndg->sndg_beamflag
				!= sndg->sndg_beamflag_org)
				{
				if (mb_beam_ok(sndg->sndg_beamflag))
					{
					action = MBP_EDIT_UNFLAG;
					}
				else if (mb_beam_check_flag_manual(sndg->sndg_beamflag))
					{
					action = MBP_EDIT_FLAG;
					}
				else if (mb_beam_check_flag_filter(sndg->sndg_beamflag))
					{
					action = MBP_EDIT_FILTER;
					}
				mb_esf_save(verbose, &esf, 
						files[i].ping_time_d[sndg->sndg_ping], 
						sndg->sndg_beam, 
						action, &error);
				}
			}
			
		/* close esf file */
		mb_esf_close(verbose, &esf, &error);
		}

	/* give the total statistics */
	if (verbose >= 0)
		{
		fprintf(stderr,"\nMBareaclean Processing Totals:\n");
		fprintf(stderr,"-------------------------\n");
		fprintf(stderr,"%d total swath data files processed\n",files_tot);
		fprintf(stderr,"%d total pings processed\n",pings_tot);
		fprintf(stderr,"%d total soundings processed\n",beams_tot);
		fprintf(stderr,"-------------------------\n");
		for (i=0;i<nfile;i++)
			{
			fprintf(stderr,"%3d soundings:%7d flagged:%7d unflagged:%7d  file:%s\n",
				i, files[i].ngood + files[i].nflag,
				files[i].nflagged, files[i].nunflagged,
				files[i].filelist);
			}
		}

	/* free arrays */
	mb_free(verbose,&bindepths,&error);
	for (i=0;i<nx*ny;i++)
		if (gsndg[i] != NULL)
			mb_free(verbose,&gsndg[i],&error);
	mb_free(verbose,&gsndg,&error);
	mb_free(verbose,&gsndgnum,&error);
	mb_free(verbose,&gsndgnum_alloc,&error);

	for (i=0;i<nfile;i++)
		{
		mb_free(verbose,&(files[nfile-1].ping_time_d),&error);
		mb_free(verbose,&(files[nfile-1].ping_altitude),&error);
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
int getsoundingptr(int verbose, int soundingid, 
		struct mbareaclean_sndg_struct **sndgptr, 
		int *error)
{
	/* local variables */
	char	*function_name = "getsoundingptr";
	int	status = MB_SUCCESS;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       soundingid:      %d\n",soundingid);
		fprintf(stderr,"dbg2       sndgptr:         %d\n",sndgptr);
		}

	/* loop over the files until the sounding is found */
	*sndgptr = NULL;
	for (i=0; i < nfile && *sndgptr == NULL; i++)
		{
		if (soundingid >= files[i].sndg_countstart 
			&& soundingid < files[i].sndg_countstart + files[i].nsndg)
			{
			j = soundingid - files[i].sndg_countstart;
			*sndgptr = &(files[i].sndg[j]);
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       *sndgptr:        %d\n",*sndgptr);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return */
	return(status);
}

/*--------------------------------------------------------------------*/


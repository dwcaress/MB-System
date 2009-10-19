/*--------------------------------------------------------------------
 *    The MB-system:	mbctdlist.c	9/14/2008
 *    $Id: mbctdlist.c,v 5.0 2008/09/20 00:51:31 caress Exp $
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
 * This program, mbctdlist, lists all ctd data records within swath data files. 
 * The -O option specifies how the values are output in an mblist-like
 * fashion. The basic available values are
 *     conductivity
 *     temperature
 *     depth
 *     salinity
 *     sound speed
 *     longitude
 *     latittude
 *
 * Author:	D. W. Caress
 * Date:	September 14,  2008
 *
 * $Log: mbctdlist.c,v $
 * Revision 5.0  2008/09/20 00:51:31  caress
 * Initial version of mbctdlist - list ctd data.
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"
#include "../../include/mb_process.h"

/* local options */
#define	MAX_OPTIONS	25
#define	MBCTDLIST_ALLOC_CHUNK 1024
/* function prototypes */
int printsimplevalue(int verbose, 
	double value, int width, int precision, 
	int ascii, int *invert, int *flipsign, int *error);
int printNaN(int verbose, int ascii, int *invert, int *flipsign, int *error);

/* NaN value */
double	NaN;

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	static char rcs_id[] = "$Id: mbctdlist.c,v 5.0 2008/09/20 00:51:31 caress Exp $";
	static char program_name[] = "mbctdlist";
	static char help_message[] =  "mbctdlist lists all CTD records within swath data files\nThe -O option specifies how the values are output\nin an mblist-likefashion.\n";
	static char usage_message[] = "mbctdlist [-A -Ddecimate -Fformat -Gdelimeter -H -Ifile -Llonflip -Ooutput_format -V -Zsegment]";
	extern char *optarg;
	extern int optkind;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	interp_status = MB_SUCCESS;
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
	int	decimate;
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

	/* output format list controls */
	char	list[MAX_OPTIONS];
	int	n_list;
	double	distance_total;
	int	nread;
	int	time_j[5];
	int	mblist_next_value = MB_NO;
	int	invert_next_value = MB_NO;
	int	signflip_next_value = MB_NO;
	int	first = MB_YES;
	int	ascii = MB_YES;
	int	segment = MB_NO;
	char	segment_tag[MB_PATH_MAXLINE];
	char	delimiter[MB_PATH_MAXLINE];

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
	
	/* navigation, heading, attitude data */
	int	survey_count = 0;
	int	survey_count_tot = 0;
	int	nnav = 0;
	int	nnav_alloc = 0;
	double	*nav_time_d = NULL;
	double	*nav_lon = NULL;
	double	*nav_lat = NULL;
	double	*nav_sonardepth = NULL;
	double	*nav_heading = NULL;
	double	*nav_speed = NULL;
	double	*nav_altitude = NULL;
	
	/* CTD values */
	int	ctd_count = 0;
	int	ctd_count_tot = 0;
	int	ctd_time_i[7];
	int	nctd;
	double	ctd_time_d[MB_CTD_MAX];
	double	ctd_conductivity[MB_CTD_MAX];
	double	ctd_temperature[MB_CTD_MAX];
	double	ctd_depth[MB_CTD_MAX];
	double	ctd_salinity[MB_CTD_MAX];
	double	ctd_soundspeed[MB_CTD_MAX];
	int	nsensor;
	double	sensor_time_d[MB_CTD_MAX];
	double	sensor1[MB_CTD_MAX];
	double	sensor2[MB_CTD_MAX];
	double	sensor3[MB_CTD_MAX];
	double	sensor4[MB_CTD_MAX];
	double	sensor5[MB_CTD_MAX];
	double	sensor6[MB_CTD_MAX];
	double	sensor7[MB_CTD_MAX];
	double	sensor8[MB_CTD_MAX];
	double	conductivity;
	double	temperature;
	double	depth;
	double	salinity;
	double	soundspeed;

	/* additional time variables */
	int	first_m = MB_YES;
	double	time_d_ref;
	struct tm	time_tm;
	int	first_u = MB_YES;
	time_t	time_u;
	time_t	time_u_ref;

	/* course calculation variables */
	double	dlon, dlat, minutes;
	int	degrees;
	char	hemi;
	double	headingx, headingy, mtodeglon, mtodeglat;
	double	course, course_old;
	double	time_d_old;
	double	time_interval;
	double	speed_made_good, speed_made_good_old;
	double	navlon_old, navlat_old;
	double	dx, dy;
	double	b;

	int	read_data;
	char	line[MB_PATH_MAXLINE];
	int	ictd;
	int	i, j, k;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);
	pings = 1;
	bounds[0] = -360.0;
	bounds[1] = 360.0;
	bounds[2] = -90.0;
	bounds[3] = 90.0;
	ctd_count = 0;
	ctd_count_tot = 0;

	/* set default input to datalist.mb-1 */
	strcpy (read_file, "datalist.mb-1");

	/* set up the default list controls 
		(Time, lon, lat, conductivity, temperature, depth, salinity, sound speed) */
	list[0]='T';
	list[1]='X';
	list[2]='Y';
	list[3]='H';
	list[4]='C';
	list[5]='c';
	list[6]='^';
	list[7]='c';
	list[8]='S';
	list[9]='s';
	n_list = 10;
	sprintf(delimiter, "\t");
	decimate = 1;

	/* process argument list */
	while ((c = getopt(argc, argv, "AaDdF:f:G:g:I:i:L:l:O:o:Z:z:VvHh")) != -1)
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
		case 'A':
		case 'a':
			ascii = MB_NO;
			flag++;
			break;
		case 'D':
		case 'd':
			sscanf (optarg,"%d", &decimate);
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			flag++;
			break;
		case 'G':
		case 'g':
			sscanf (optarg,"%s", delimiter);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", read_file);
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
			flag++;
			break;
		case 'O':
		case 'o':
			for(j=0,n_list=0;j<(int)strlen(optarg);j++,n_list++)
				if (n_list<MAX_OPTIONS)
					list[n_list] = optarg[j];
			break;
		case 'Z':
		case 'z':
			segment = MB_YES;
			sscanf (optarg,"%s", segment_tag);
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
		fprintf(stderr,"dbg2       decimate:       %d\n",decimate);
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
		fprintf(stderr,"dbg2       ascii:          %d\n",ascii);
		fprintf(stderr,"dbg2       segment:        %d\n",segment);
		fprintf(stderr,"dbg2       segment_tag:    %s\n",segment_tag);
		fprintf(stderr,"dbg2       delimiter:      %s\n",delimiter);
		fprintf(stderr,"dbg2       file:           %s\n",file);
		fprintf(stderr,"dbg2       n_list:         %d\n",n_list);
		for (i=0;i<n_list;i++)
			fprintf(stderr,"dbg2         list[%d]:      %c\n",
						i,list[i]);
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
		
	/**************************************************************************************/
	
	/* section 1 - read all data and save nav etc for interpolation onto ctd data */
	

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
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(char), (void **)&beamflag, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bath, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
						sizeof(double), (void **)&amp, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bathacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bathalongtrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&ss, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&ssacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&ssalongtrack, &error);

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
		
	/* output separator for GMT style segment file output */
	if (segment == MB_YES && ascii == MB_YES)
		{
		printf("%s\n", segment_tag);
		}

	/* output info */
	if (verbose >= 1)
		{
		fprintf(stderr, "\nSearching %s for survey records\n", file);
		}

	/* read and print data */
	survey_count = 0;
	first = MB_YES;
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
			
		/* if survey data save the nav etc */
		if (error <= MB_ERROR_NO_ERROR
			&& kind == MB_DATA_DATA)
			{
			/* allocate memory for navigation/attitude arrays if needed */
			if (nnav + 1 >= nnav_alloc)
				{
				nnav_alloc +=  MBCTDLIST_ALLOC_CHUNK;
				status = mb_reallocd(verbose,__FILE__,__LINE__,nnav_alloc*sizeof(double),(void **)&nav_time_d,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,nnav_alloc*sizeof(double),(void **)&nav_lon,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,nnav_alloc*sizeof(double),(void **)&nav_lat,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,nnav_alloc*sizeof(double),(void **)&nav_speed,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,nnav_alloc*sizeof(double),(void **)&nav_sonardepth,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,nnav_alloc*sizeof(double),(void **)&nav_heading,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,nnav_alloc*sizeof(double),(void **)&nav_altitude,&error);
				if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
					    program_name);
					exit(error);
					}		    
				}
				
			/* save the nav etc */
			if (nnav == 0 || time_d > nav_time_d[nnav-1])
				{
				nav_time_d[nnav] = time_d;
				nav_lon[nnav] = navlon;
				nav_lat[nnav] = navlat;
				nav_speed[nnav] = speed;
				nav_sonardepth[nnav] = sonardepth;
				nav_heading[nnav] = heading;
				nav_altitude[nnav] = altitude;
				nnav++;			
				}
			survey_count++;
			survey_count_tot++;
			}
			
		}

	/* close the swath file */
	status = mb_close(verbose,&mbio_ptr,&error);

	/* output info */
	if (verbose >= 1)
		{
		fprintf(stderr, "nav extracted from %d survey records\n", survey_count);
		}

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
		fprintf(stderr, "\nTotal %d survey records\n", survey_count_tot);
		}
		
	/**************************************************************************************/
	
	/* section 2 - read data and output ctd data with time interpolation of nav etc */

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
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(char), (void **)&beamflag, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bath, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
						sizeof(double), (void **)&amp, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bathacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bathalongtrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&ss, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&ssacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&ssalongtrack, &error);

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
		fprintf(stderr, "\nSearching %s for CTD records\n", file);
		}

	/* read and print data */
	ctd_count = 0;
	first = MB_YES;
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
			
		/* if ctd then extract data */
		if (error <= MB_ERROR_NO_ERROR
			&& (kind == MB_DATA_CTD || kind == MB_DATA_SSV))
			{
			/* extract ctd */
			status = mb_ctd(verbose, mbio_ptr, store_ptr,
						&kind, &nctd, ctd_time_d,
						ctd_conductivity, ctd_temperature,
						ctd_depth, ctd_salinity, ctd_soundspeed, &error);

			/* extract ancilliary sensor data */
			status = mb_ancilliarysensor(verbose, mbio_ptr, store_ptr,
						&kind, &nsensor, sensor_time_d,
						sensor1, sensor2, sensor3,
						sensor4, sensor5, sensor6,
						sensor7, sensor8,
						&error);

			/* loop over the nctd ctd points, outputting each one */
			if (error == MB_ERROR_NO_ERROR && nctd > 0)
				{
				for (ictd=0;ictd<nctd;ictd++)
					{
					/* get data */
					time_d = ctd_time_d[ictd];
					mb_get_date(verbose, time_d, time_i);
					conductivity = ctd_conductivity[ictd];
					temperature = ctd_temperature[ictd];
					depth = ctd_depth[ictd];
					salinity = ctd_salinity[ictd];
					soundspeed = ctd_soundspeed[ictd];
					
					/* get navigation */
					j = 0;
					speed = 0.0;
					interp_status = mb_linear_interp_degrees(verbose, 
								nav_time_d-1, nav_lon-1,
								nnav, time_d, &navlon, &j, 
								&error);
					if (interp_status == MB_SUCCESS)
					interp_status = mb_linear_interp_degrees(verbose, 
								nav_time_d-1, nav_lat-1,
								nnav, time_d, &navlat, &j, 
								&error);
					if (interp_status == MB_SUCCESS)
					interp_status = mb_linear_interp_degrees(verbose, 
								nav_time_d-1, nav_heading-1,
								nnav, time_d, &heading, &j, 
								&error);
					if (interp_status == MB_SUCCESS)
					interp_status = mb_linear_interp(verbose, 
								nav_time_d-1, nav_sonardepth-1,
								nnav, time_d, &sonardepth, &j, 
								&error);
					if (interp_status == MB_SUCCESS)
					interp_status = mb_linear_interp(verbose, 
								nav_time_d-1, nav_altitude-1,
								nnav, time_d, &altitude, &j, 
								&error);
					if (interp_status == MB_SUCCESS)
					interp_status = mb_linear_interp(verbose, 
								nav_time_d-1, nav_speed-1,
								nnav, time_d, &speed, &j, 
								&error);
							    
					/* only output if interpolation of nav etc has worked */
					if (interp_status == MB_YES)
						{

						/* calculate course made good and distance */
						mb_coor_scale(verbose,navlat, &mtodeglon, &mtodeglat);
						headingx = sin(DTR * heading);
						headingy = cos(DTR * heading);
						if (first == MB_YES)
							{
							time_interval = 0.0;
							course = heading;
							speed_made_good = 0.0;
							course_old = heading;
							speed_made_good_old = speed;
							distance = 0.0;
							}
						else
							{
							time_interval = time_d - time_d_old;
							dx = (navlon - navlon_old)/mtodeglon;
							dy = (navlat - navlat_old)/mtodeglat;
							distance = sqrt(dx*dx + dy*dy);
							if (distance > 0.0)
								course = RTD*atan2(dx/distance,dy/distance);
							else
								course = course_old;
							if (course < 0.0)
								course = course + 360.0;
							if (time_interval > 0.0)
								speed_made_good = 3.6*distance/time_interval;
							else
								speed_made_good 
									= speed_made_good_old;
							}
						distance_total += 0.001 * distance;

						/* reset old values */
						navlon_old = navlon;
						navlat_old = navlat;
						course_old = course;
						speed_made_good_old = speed_made_good;
						time_d_old = time_d;

						/* now loop over list of output parameters */
						ctd_count += nctd;
						ctd_count_tot += nctd;
						if (nctd % decimate == 0)
						for (i=0; i<n_list; i++) 
							{
							switch (list[i]) 
								{
								case '/': /* Inverts next simple value */
									invert_next_value = MB_YES;
									break;
								case '-': /* Flip sign on next simple value */
									signflip_next_value = MB_YES;
									break;
								case '^': /* use mblist definitions of CcSsTt */
									mblist_next_value = MB_YES;
									break;
								case '1': /* Sensor 1 - volts */
									printsimplevalue(verbose, sensor1[ictd], 0, 3, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
									break;
								case '2': /* Sensor 2 - volts */
									printsimplevalue(verbose, sensor2[ictd], 0, 3, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
									break;
								case '3': /* Sensor 3 - volts */
									printsimplevalue(verbose, sensor3[ictd], 0, 3, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
									break;
								case '4': /* Sensor 4 - volts */
									printsimplevalue(verbose, sensor4[ictd], 0, 3, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
									break;
								case '5': /* Sensor 5 - volts */
									printsimplevalue(verbose, sensor5[ictd], 0, 3, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
									break;
								case '6': /* Sensor 6 - volts */
									printsimplevalue(verbose, sensor6[ictd], 0, 3, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
									break;
								case '7': /* Sensor 7 - volts */
									printsimplevalue(verbose, sensor7[ictd], 0, 3, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
									break;
								case '8': /* Sensor 8 - volts */
									printsimplevalue(verbose, sensor8[ictd], 0, 3, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
									break;
								case 'C': /* Conductivity or Sonar altitude (m) */
									if (mblist_next_value == MB_NO)
										printsimplevalue(verbose, conductivity, 0, 3, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
									else
										{
										printsimplevalue(verbose, altitude, 0, 3, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
										mblist_next_value = MB_NO;
										}
									break;
								case 'c': /* Temperature or sonar transducer depth (m) */
									if (mblist_next_value == MB_NO)
										printsimplevalue(verbose, temperature, 0, 3, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
									else
										{
										printsimplevalue(verbose, sonardepth, 0, 3, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
										mblist_next_value = MB_NO;
										}
									break;
								case 'H': /* heading */
									printsimplevalue(verbose, heading, 6, 2, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
									break;
								case 'h': /* course */
									printsimplevalue(verbose, course, 6, 2, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
									break;
								case 'J': /* time string */
									mb_get_jtime(verbose,time_i,time_j);
									if (ascii == MB_YES)
									    {
									    printf("%.4d %.3d %.2d %.2d %.2d.%6.6d",
										time_j[0],time_j[1],
										time_i[3],time_i[4],
										time_i[5],time_i[6]);
									    }
									else
									    {
									    b = time_j[0];
									    fwrite(&b, sizeof(double), 1, stdout);
									    b = time_j[1];
									    fwrite(&b, sizeof(double), 1, stdout);
									    b = time_i[3];
									    fwrite(&b, sizeof(double), 1, stdout);
									    b = time_i[4];
									    fwrite(&b, sizeof(double), 1, stdout);
									    b = time_i[5];
									    fwrite(&b, sizeof(double), 1, stdout);
									    b = time_i[6];
									    fwrite(&b, sizeof(double), 1, stdout);
									    }
									break;
								case 'j': /* time string */
									mb_get_jtime(verbose,time_i,time_j);
									if (ascii == MB_YES)
									    {
									    printf("%.4d %.3d %.4d %.2d.%6.6d",
										time_j[0],time_j[1],
										time_j[2],time_j[3],time_j[4]);
									    }
									else
									    {
									    b = time_j[0];
									    fwrite(&b, sizeof(double), 1, stdout);
									    b = time_j[1];
									    fwrite(&b, sizeof(double), 1, stdout);
									    b = time_j[2];
									    fwrite(&b, sizeof(double), 1, stdout);
									    b = time_j[3];
									    fwrite(&b, sizeof(double), 1, stdout);
									    b = time_j[4];
									    fwrite(&b, sizeof(double), 1, stdout);
									    }
									break;
								case 'L': /* along-track distance (km) */
									printsimplevalue(verbose, distance_total, 7, 3, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
									break;
								case 'l': /* along-track distance (m) */
									printsimplevalue(verbose, 1000.0 * distance_total, 7, 3, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
									break;
								case 'M': /* Decimal unix seconds since 
										1/1/70 00:00:00 */
									printsimplevalue(verbose, time_d, 0, 6, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
									break;
								case 'm': /* time in decimal seconds since 
										first record */
									if (first_m == MB_YES)
										{
										time_d_ref = time_d;
										first_m = MB_NO;
										}
									b = time_d - time_d_ref;
									printsimplevalue(verbose, b, 0, 6, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
									break;
								case 'S': /* salinity or speed */
									if (mblist_next_value == MB_NO)
										printsimplevalue(verbose, salinity, 0, 3, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
									else
										{
										printsimplevalue(verbose, speed, 5, 2, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
										mblist_next_value = MB_NO;
										}
									break;
								case 's': /* speed made good */
									if (mblist_next_value == MB_NO)
										printsimplevalue(verbose, soundspeed, 0, 3, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
									else
										{
										printsimplevalue(verbose, speed_made_good, 5, 2, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
										mblist_next_value = MB_NO;
										}
									break;
								case 'T': /* yyyy/mm/dd/hh/mm/ss time string */
									if (ascii == MB_YES)
									    printf("%.4d/%.2d/%.2d/%.2d/%.2d/%.2d.%.6d",
										time_i[0],time_i[1],time_i[2],
										time_i[3],time_i[4],time_i[5],
										time_i[6]);
									else
									    {
									    b = time_i[0];
									    fwrite(&b, sizeof(double), 1, stdout);
									    b = time_i[1];
									    fwrite(&b, sizeof(double), 1, stdout);
									    b = time_i[2];
									    fwrite(&b, sizeof(double), 1, stdout);
									    b = time_i[3];
									    fwrite(&b, sizeof(double), 1, stdout);
									    b = time_i[4];
									    fwrite(&b, sizeof(double), 1, stdout);
									    b = time_i[5] + 1e-6 * time_i[6];
									    fwrite(&b, sizeof(double), 1, stdout);
									    }
									break;
								case 't': /* yyyy mm dd hh mm ss time string */
									if (ascii == MB_YES)
									    printf("%.4d %.2d %.2d %.2d %.2d %.2d.%.6d",
										time_i[0],time_i[1],time_i[2],
										time_i[3],time_i[4],time_i[5],
										time_i[6]);
									else
									    {
									    b = time_i[0];
									    fwrite(&b, sizeof(double), 1, stdout);
									    b = time_i[1];
									    fwrite(&b, sizeof(double), 1, stdout);
									    b = time_i[2];
									    fwrite(&b, sizeof(double), 1, stdout);
									    b = time_i[3];
									    fwrite(&b, sizeof(double), 1, stdout);
									    b = time_i[4];
									    fwrite(&b, sizeof(double), 1, stdout);
									    b = time_i[5] + 1e-6 * time_i[6];
									    fwrite(&b, sizeof(double), 1, stdout);
									    }
									break;
								case 'U': /* unix time in seconds since 1/1/70 00:00:00 */
									time_u = (int) time_d;
									if (ascii == MB_YES)
									    printf("%d",time_u);
									else
									    {
									    b = time_u;
									    fwrite(&b, sizeof(double), 1, stdout);
									    }
									break;
								case 'u': /* time in seconds since first record */
									time_u = (int) time_d;
									if (first_u == MB_YES)
										{
										time_u_ref = time_u;
										first_u = MB_NO;
										}
									if (ascii == MB_YES)
									    printf("%d",time_u - time_u_ref);
									else
									    {
									    b = time_u - time_u_ref;
									    fwrite(&b, sizeof(double), 1, stdout);
									    }
									break;
								case 'V': /* time in seconds since last value */
								case 'v': 
									if (ascii == MB_YES)
									    {
									    if ( fabs(time_interval) > 100. )
										printf("%g",time_interval); 
									    else
										printf("%7.3f",time_interval);
									    }
									else
									    {
									    fwrite(&time_interval, sizeof(double), 1, stdout);
									    }
									break;
								case 'X': /* longitude decimal degrees */
									dlon = navlon;
									printsimplevalue(verbose, dlon, 11, 6, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
									break;
								case 'x': /* longitude degress + decimal minutes */
									dlon = navlon;
									if (dlon < 0.0)
										{
										hemi = 'W';
										dlon = -dlon;
										}
									else
										hemi = 'E';
									degrees = (int) dlon;
									minutes = 60.0*(dlon - degrees);
									if (ascii == MB_YES)
									    {
									    printf("%3d %8.5f%c",
										degrees, minutes, hemi);
									    }
									else
									    {
									    b = degrees;
									    if (hemi == 'W') b = -b;
									    fwrite(&b, sizeof(double), 1, stdout);
									    b = minutes;
									    fwrite(&b, sizeof(double), 1, stdout);
									    }
									break;
								case 'Y': /* latitude decimal degrees */
									dlat = navlat;
									printsimplevalue(verbose, dlat, 11, 6, ascii, 
											    &invert_next_value, 
											    &signflip_next_value, &error);
									break;
								case 'y': /* latitude degrees + decimal minutes */
									dlat = navlat;
									if (dlat < 0.0)
										{
										hemi = 'S';
										dlat = -dlat;
										}
									else
										hemi = 'N';
									degrees = (int) dlat;
									minutes = 60.0*(dlat - degrees);
									if (ascii == MB_YES)
									    {
									    printf("%3d %8.5f%c",
										degrees, minutes, hemi);
									    }
									else
									    {
									    b = degrees;
									    if (hemi == 'S') b = -b;
									    fwrite(&b, sizeof(double), 1, stdout);
									    b = minutes;
									    fwrite(&b, sizeof(double), 1, stdout);
									    }
									break;
								default:
									if (ascii == MB_YES)
									    printf("<Invalid Option: %c>",
										list[i]);
									break;
								}
							if (ascii == MB_YES)
								{
								if (i<(n_list-1)) printf ("%s", delimiter);
								else printf ("\n");
								}
							}
						first = MB_NO;
						}
					}
				}
			}
			
		/* else if survey data ignore */
		else if (error <= MB_ERROR_NO_ERROR
			&& kind == MB_DATA_DATA)
			{
			}
			
		}

	/* close the swath file */
	status = mb_close(verbose,&mbio_ptr,&error);

	/* output info */
	if (verbose >= 1)
		{
		fprintf(stderr, "%d CTD records\n", ctd_count);
		}

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
		fprintf(stderr, "\nTotal %d CTD records\n", ctd_count_tot);
		}
	
	/* deallocate navigation arrays */
	if (nnav > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nav_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nav_lon,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nav_lat,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nav_speed,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nav_sonardepth,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nav_heading,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nav_altitude,&error);
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
int printsimplevalue(int verbose, 
	double value, int width, int precision, 
	int ascii, int *invert, int *flipsign, int *error)
{
	char	*function_name = "printsimplevalue";
	int	status = MB_SUCCESS;
	char	format[24];
	int	i;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBlist function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       value:           %f\n",value);
		fprintf(stderr,"dbg2       width:           %d\n",width);
		fprintf(stderr,"dbg2       precision:       %d\n",precision);
		fprintf(stderr,"dbg2       ascii:           %d\n",ascii);
		fprintf(stderr,"dbg2       invert:          %d\n",*invert);
		fprintf(stderr,"dbg2       flipsign:        %d\n",*flipsign);
		}
		
	/* make print format */
	format[0] = '%';
	if (*invert == MB_YES)
	    strcpy(format, "%g");
	else if (width > 0)
	    sprintf(&format[1], "%d.%df", width, precision);
	else
	    sprintf(&format[1], ".%df", precision);
	
	/* invert value if desired */
	if (*invert == MB_YES)
	    {
	    *invert = MB_NO;
	    if (value != 0.0)
		value = 1.0 / value;
	    }
	
	/* flip sign value if desired */
	if (*flipsign == MB_YES)
	    {
	    *flipsign = MB_NO;
	    value = -value;
	    }
	    
	/* print value */
	if (ascii == MB_YES)
	    printf(format, value);
	else
	    fwrite(&value, sizeof(double), 1, stdout);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBlist function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       invert:          %d\n",*invert);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int printNaN(int verbose, int ascii, int *invert, int *flipsign, int *error)
{
	char	*function_name = "printNaN";
	int	status = MB_SUCCESS;
	int	i;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBlist function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       ascii:           %d\n",ascii);
		fprintf(stderr,"dbg2       invert:          %d\n",*invert);
		fprintf(stderr,"dbg2       flipsign:        %d\n",*flipsign);
		}
		
	/* reset invert flag */
	if (*invert == MB_YES)
	    *invert = MB_NO;
		
	/* reset flipsign flag */
	if (*flipsign == MB_YES)
	    *flipsign = MB_NO;
	    
	/* print value */
	if (ascii == MB_YES)
	    printf("NaN");
	else
	    fwrite(&NaN, sizeof(double), 1, stdout);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBlist function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       invert:          %d\n",*invert);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/

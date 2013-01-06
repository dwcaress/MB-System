/*--------------------------------------------------------------------
 *    The MB-system:	mbkongsbergpreprocess.c	1/1/2012
 *    $Id: mbkongsbergpreprocess.c 1938 2012-02-22 20:58:08Z caress $
 *
 *    Copyright (c) 2012-2013 by
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
 * mbkongsbergpreprocess reads a HYSWEEP HSX format file, interpolates the
 * asynchronous navigation and attitude onto the multibeam data,
 * and writes a new HSX file with that information correctly embedded
 * in the multibeam data. This program can also fix various problems
 * with the data, including sensor offsets.
 *
 * Author:	D. W. Caress
 * Date:	June 1, 2012
 *
 * $Log: mbkongsbergpreprocess.c,v $
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"
#include "../../include/mb_io.h"
#include "../../include/mb_aux.h"
#include "../../include/mbsys_simrad3.h"

#define MBKONSBERGPREPROCESS_ALLOC_CHUNK 1000
#define MBKONSBERGPREPROCESS_PROCESS		1
#define MBKONSBERGPREPROCESS_TIMESTAMPLIST	2
#define	MBKONSBERGPREPROCESS_TIMELAG_OFF	0
#define	MBKONSBERGPREPROCESS_TIMELAG_CONSTANT	1
#define	MBKONSBERGPREPROCESS_TIMELAG_MODEL	2

#define MBKONSBERGPREPROCESS_SONAR_OFFSET_NONE		0
#define MBKONSBERGPREPROCESS_SONAR_OFFSET_SONAR		1
#define MBKONSBERGPREPROCESS_SONAR_OFFSET_MRU		2
#define MBKONSBERGPREPROCESS_SONAR_OFFSET_NAVIGATION	3

#define MBKONSBERGPREPROCESS_OFFSET_MAX	12

#define MBKONSBERGPREPROCESS_NAVFORMAT_NONE	0
#define MBKONSBERGPREPROCESS_NAVFORMAT_OFG	1

/* set precision of iterative raytracing depth & distance matching */
#define MBKONSBERGPREPROCESS_BATH_RECALC_PRECISION 0.0001
#define MBKONSBERGPREPROCESS_BATH_RECALC_NCALCMAX 50
#define MBKONSBERGPREPROCESS_BATH_RECALC_ANGLEMODE 0

static char rcs_id[] = "$Id: mbkongsbergpreprocess.c 1938 2012-02-22 20:58:08Z caress $";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char program_name[] = "mbkongsbergpreprocess";
	char help_message[] =  "mbkongsbergpreprocess reads a Kongsberg multibeam vendor format file (or datalist of files),\ninterpolates the asynchronous navigation and attitude onto the multibeam data, \nand writes the data as one or more format 59 files.";
	char usage_message[] = "mbkongsbergpreprocess [-C -Doutputdirectory -Iinputfile -H -V]";
	extern char *optarg;
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
	int	format = 0;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	char	ifile[MB_PATH_MAXLINE];
	char	ofile[MB_PATH_MAXLINE];
	int	ofile_set = MB_NO;
	char	odir[MB_PATH_MAXLINE];
	int	odir_set = MB_NO;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	int	obeams_bath;
	int	obeams_amp;
	int	opixels_ss;

	/* MBIO read values */
	struct mb_io_struct *imb_io_ptr = NULL;
	struct mbsys_simrad3_struct *istore = NULL;
	struct mbsys_simrad3_ping_struct *ping = NULL;
	struct mbsys_simrad3_attitude_struct *attitude = NULL;
	struct mbsys_simrad3_netattitude_struct *netattitude = NULL;
	struct mbsys_simrad3_heading_struct *headingr = NULL;
	void	*imbio_ptr = NULL;
	void	*istore_ptr = NULL;
	void	*ombio_ptr = NULL;
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
	double	roll;
	double	pitch;
	double	heave;
	char	*beamflag = NULL;
	double	*bath = NULL;
	double	*bathacrosstrack = NULL;
	double	*bathalongtrack = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*ssacrosstrack = NULL;
	double	*ssalongtrack = NULL;
	char	comment[MB_COMMENT_MAXLINE];

	/* program mode */
	int	mode = MBKONSBERGPREPROCESS_PROCESS;
	int	nav_source = MB_DATA_NAV;
	int	attitude_source = MB_DATA_ATTITUDE;
	int	heading_source = MB_DATA_HEADING;

	/* counting variables file */
	int	output_counts = MB_NO;
	int	nfile_read = 0;
	int	nfile_write = 0;
	int	nrec_0x30_parameter_stop = 0;
	int	nrec_0x31_parameter_off = 0;
	int	nrec_0x32_parameter_on = 0;
	int	nrec_0x33_parameter_extra = 0;
	int	nrec_0x41_attitude = 0;
	int	nrec_0x43_clock = 0;
	int	nrec_0x44_bathymetry = 0;
	int	nrec_0x45_singlebeam = 0;
	int	nrec_0x46_rawbeamF = 0;
	int	nrec_0x47_surfacesoundspeed2 = 0;
	int	nrec_0x48_heading = 0;
	int	nrec_0x49_parameter_start = 0;
	int	nrec_0x4A_tilt = 0;
	int	nrec_0x4B_echogram = 0;
	int	nrec_0x4E_rawbeamN = 0;
	int	nrec_0x50_pos = 0;
	int	nrec_0x52_runtime = 0;
	int	nrec_0x53_sidescan = 0;
	int	nrec_0x54_tide = 0;
	int	nrec_0x55_svp2 = 0;
	int	nrec_0x56_svp = 0;
	int	nrec_0x57_surfacesoundspeed = 0;
	int	nrec_0x58_bathymetry2 = 0;
	int	nrec_0x59_sidescan2 = 0;
	int	nrec_0x66_rawbeamf = 0;
	int	nrec_0x68_height = 0;
	int	nrec_0x69_parameter_stop = 0;
	int	nrec_0x6B_water_column = 0;
	int	nrec_0x6E_network_attitude = 0;
	int	nrec_0x70_parameter = 0;
	int	nrec_0x73_surface_sound_speed = 0;
	int	nrec_0xE1_bathymetry_mbari57 = 0;
	int	nrec_0xE2_sidescan_mbari57 = 0;
	int	nrec_0xE3_bathymetry_mbari59 = 0;
	int	nrec_0xE4_sidescan_mbari59 = 0;
	int	nrec_0xE5_bathymetry_mbari59 = 0;

	/* counting variables total */
	int	nrec_0x30_parameter_stop_tot = 0;
	int	nrec_0x31_parameter_off_tot = 0;
	int	nrec_0x32_parameter_on_tot = 0;
	int	nrec_0x33_parameter_extra_tot = 0;
	int	nrec_0x41_attitude_tot = 0;
	int	nrec_0x43_clock_tot = 0;
	int	nrec_0x44_bathymetry_tot = 0;
	int	nrec_0x45_singlebeam_tot = 0;
	int	nrec_0x46_rawbeamF_tot = 0;
	int	nrec_0x47_surfacesoundspeed2_tot = 0;
	int	nrec_0x48_heading_tot = 0;
	int	nrec_0x49_parameter_start_tot = 0;
	int	nrec_0x4A_tilt_tot = 0;
	int	nrec_0x4B_echogram_tot = 0;
	int	nrec_0x4E_rawbeamN_tot = 0;
	int	nrec_0x50_pos_tot = 0;
	int	nrec_0x52_runtime_tot = 0;
	int	nrec_0x53_sidescan_tot = 0;
	int	nrec_0x54_tide_tot = 0;
	int	nrec_0x55_svp2_tot = 0;
	int	nrec_0x56_svp_tot = 0;
	int	nrec_0x57_surfacesoundspeed_tot = 0;
	int	nrec_0x58_bathymetry2_tot = 0;
	int	nrec_0x59_sidescan2_tot = 0;
	int	nrec_0x66_rawbeamf_tot = 0;
	int	nrec_0x68_height_tot = 0;
	int	nrec_0x69_parameter_stop_tot = 0;
	int	nrec_0x6B_water_column_tot = 0;
	int	nrec_0x6E_network_attitude_tot = 0;
	int	nrec_0x70_parameter_tot = 0;
	int	nrec_0x73_surface_sound_speed_tot = 0;
	int	nrec_0xE1_bathymetry_mbari57_tot = 0;
	int	nrec_0xE2_sidescan_mbari57_tot = 0;
	int	nrec_0xE3_bathymetry_mbari59_tot = 0;
	int	nrec_0xE4_sidescan_mbari59_tot = 0;
	int	nrec_0xE5_bathymetry_mbari59_tot = 0;

	/* asynchronous navigation, heading, attitude data */
	int	ndat_nav = 0;
	int	ndat_nav_alloc = 0;
	double	*dat_nav_time_d = NULL;
	double	*dat_nav_lon = NULL;
	double	*dat_nav_lat = NULL;

	int	ndat_heading = 0;
	int	ndat_heading_alloc = 0;
	double	*dat_heading_time_d = NULL;
	double	*dat_heading_heading = NULL;

	int	ndat_rph = 0;
	int	ndat_rph_alloc = 0;
	double	*dat_rph_time_d = NULL;
	double	*dat_rph_roll = NULL;
	double	*dat_rph_pitch = NULL;
	double	*dat_rph_heave = NULL;

	/* timelag parameters */
	int	timelagmode = MBKONSBERGPREPROCESS_TIMELAG_OFF;
	double	timelag = 0.0;
	double	timelagconstant = 0.0;
	char	timelagfile[MB_PATH_MAXLINE];
	int	ntimelag = 0;
	double	*timelag_time_d = NULL;
	double	*timelag_model = NULL;

	/* output asynchronous and synchronous time series ancilliary files */
	char	athfile[MB_PATH_MAXLINE];
	char	atsfile[MB_PATH_MAXLINE];
	char	atafile[MB_PATH_MAXLINE];
	char	stafile[MB_PATH_MAXLINE];
	FILE	*athfp;
	FILE	*atsfp;
	FILE	*atafp;
	FILE	*stafp;

	/* processing kluge modes */
	int	recalculate_beam_angles = MB_NO;
	int	klugemode;

	int	interp_status;
	FILE	*tfp = NULL;
	struct stat file_status;
	int	fstat;
	char	buffer[MB_PATH_MAXLINE];
	char	*result;
	int	read_data;
	char	fileroot[MB_PATH_MAXLINE];
	char	*filenameptr;
	int	testformat;
	int	type, source;
	double	start_time_d, end_time_d;

	double	ptime_d;
	double	pheading;
	double	heave_offset = 0.0;
	double	heave_ping, heave_beam;
	double	soundspeed;
	double	transmit_alongtrack;
	double	alpha, beta, theta, phi, theta_bath, phi_bath;
	double	theta_new, theta_nadir, theta_x, theta_z, dtheta, theta_old, thetamin, thetamax;
	int	inadir;
	double	transmit_time_d, transmit_heading, transmit_heave, transmit_roll, transmit_pitch;
	double	receive_time_d, receive_heading, receive_heave, receive_roll, receive_pitch;
	mb_u_char detection_mask;
	double	xx, zz, dx, dz, dt;
	double	xxx, yyy;
	double	xxcalc, zzcalc, tt, ttt, xxcalc_old, zzcalc_old;
	double	depth_offset_use, static_shift, svpdepthstart;
	double	weight;
	double	lever_x, lever_y, lever_z;
	double	offset_x, offset_y, offset_z;
	int	ray_stat, iterx, iterz;
	int	done;

	int	jtimelag = 0;
	int	jnav = 0;
	int	jheading = 0;
	int	jattitude = 0;

	int	i;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to datalist.mb-1 */
	strcpy (read_file, "datalist.mb-1");

	/* set default nav and attitude sources */
	nav_source = MB_DATA_NAV;
	attitude_source = MB_DATA_ATTITUDE;
	heading_source = MB_DATA_NAV;

	/* process argument list */
	while ((c = getopt(argc, argv, "AaCcD:d:F:f:I:i:K:k:O:o:S:s:T:t:VvHh")) != -1)
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
			recalculate_beam_angles = MB_YES;
			flag++;
			break;
		case 'C':
		case 'c':
			output_counts = MB_YES;
			flag++;
			break;
		case 'D':
		case 'd':
			sscanf (optarg,"%s", odir);
			odir_set  = MB_YES;
			flag++;
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
		case 'K':
		case 'k':
			sscanf (optarg,"%d", &klugemode);
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", ofile);
			ofile_set  = MB_YES;
			flag++;
			break;
		case 'S':
		case 's':
			sscanf (optarg,"%d/%d", &type, &source);
			if (type == 1)
				nav_source = source;
			else if (type == 2)
				heading_source = source;
			else if (type == 3)
				attitude_source = source;
			flag++;
			break;
		case 'T':
		case 't':
			sscanf (optarg,"%s", timelagfile);
			if ((fstat = stat(timelagfile, &file_status)) == 0
				&& (file_status.st_mode & S_IFMT) != S_IFDIR)
				{
				timelagmode = MBKONSBERGPREPROCESS_TIMELAG_MODEL;
				}
			else
				{
				sscanf (optarg,"%lf", &timelagconstant);
				timelagmode = MBKONSBERGPREPROCESS_TIMELAG_CONSTANT;
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
		fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Control Parameters:\n");
		fprintf(stderr,"dbg2       verbose:             %d\n",verbose);
		fprintf(stderr,"dbg2       help:                %d\n",help);
		fprintf(stderr,"dbg2       format:              %d\n",format);
		fprintf(stderr,"dbg2       pings:               %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:             %d\n",lonflip);
		fprintf(stderr,"dbg2       bounds[0]:           %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:           %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:           %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:           %f\n",bounds[3]);
		fprintf(stderr,"dbg2       btime_i[0]:          %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]:          %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]:          %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]:          %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]:          %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]:          %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       btime_i[6]:          %d\n",btime_i[6]);
		fprintf(stderr,"dbg2       etime_i[0]:          %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:          %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:          %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:          %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:          %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:          %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       etime_i[6]:          %d\n",etime_i[6]);
		fprintf(stderr,"dbg2       speedmin:            %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:             %f\n",timegap);
		fprintf(stderr,"dbg2       read_file:           %s\n",read_file);
		fprintf(stderr,"dbg2       ofile:               %s\n",ofile);
		fprintf(stderr,"dbg2       ofile_set:           %d\n",ofile_set);
		fprintf(stderr,"dbg2       odir:               %s\n",odir);
		fprintf(stderr,"dbg2       odir_set:           %d\n",odir_set);
		if (timelagmode == MBKONSBERGPREPROCESS_TIMELAG_MODEL)
			{
			fprintf(stderr,"dbg2       timelagfile:         %s\n",timelagfile);
			fprintf(stderr,"dbg2       ntimelag:            %d\n",ntimelag);
			}
		else
			{
			fprintf(stderr,"dbg2       timelag:             %f\n",timelag);
			}
		fprintf(stderr,"dbg2       timelag:                %f\n",timelag);
		fprintf(stderr,"dbg2       recalculate_beam_angles:%d\n",recalculate_beam_angles);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* get time lag model if specified */
	if (timelagmode == MBKONSBERGPREPROCESS_TIMELAG_MODEL)
		{
		/* count the data points in the timelag file */
		ntimelag = 0;
		if ((tfp = fopen(timelagfile, "r")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open time lag model File <%s> for reading\n",timelagfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
			if (buffer[0] != '#')
			    ntimelag++;
		rewind(tfp);

		/* allocate arrays for time lag */
		if (ntimelag > 0)
		    {
		    status = mb_mallocd(verbose, __FILE__, __LINE__,ntimelag * sizeof(double), (void **)&timelag_time_d,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__,ntimelag * sizeof(double), (void **)&timelag_model,&error);
		    if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		    }

		/* if no time lag data then quit */
		else
		    {
		    error = MB_ERROR_BAD_DATA;
		    fprintf(stderr,"\nUnable to read data from time lag model file <%s>\n",timelagfile);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }

		/* read the data points in the timelag file */
		ntimelag = 0;
		while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
		    {
		    if (buffer[0] != '#')
			{
			/* read the time and time lag pair */
			if (sscanf(buffer,"%lf %lf",&timelag_time_d[ntimelag],&timelag_model[ntimelag]) == 2)
			    ntimelag++;
			}
		    }
		fclose(tfp);

		/* output info */
		if (ntimelag > 0)
			{
			mb_get_date(verbose, timelag_time_d[0], btime_i);
			mb_get_date(verbose, timelag_time_d[ntimelag-1], etime_i);
			fprintf(stderr, "%d timelag records read from %s  Start:%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d  End:%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
					ntimelag, timelagfile,
					btime_i[0], btime_i[1], btime_i[2], btime_i[3], btime_i[4], btime_i[5], btime_i[6],
					etime_i[0], etime_i[1], etime_i[2], etime_i[3], etime_i[4], etime_i[5], etime_i[6]);
			}
		else
			fprintf(stderr, "No timelag data read from %s....\n",timelagfile);
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
	    if ((status = mb_datalist_read(verbose,datalist,
			    ifile,&format,&file_weight,&error))
			    == MB_SUCCESS)
		read_data = MB_YES;
	    else
		read_data = MB_NO;
	    }
	/* else copy single filename to be read */
	else
	    {
	    strcpy(ifile, read_file);
	    read_data = MB_YES;
	    }

	/* loop over all files to be read */
	while (read_data == MB_YES &&
	       (format == MBF_EM300RAW
		|| format == MBF_EM300MBA
		|| format == MBF_EM710RAW
		|| format == MBF_EM710MBA))
	{
	/* initialize reading the swath file */
	if ((status = mb_read_init(
		verbose,ifile,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&imbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",ifile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* get pointers to data storage */
	imb_io_ptr = (struct mb_io_struct *) imbio_ptr;
	istore_ptr = imb_io_ptr->store_data;
	istore = (struct mbsys_simrad3_struct *) istore_ptr;

	if (error == MB_ERROR_NO_ERROR)
		{
		beamflag = NULL;
		bath = NULL;
		amp = NULL;
		bathacrosstrack = NULL;
		bathalongtrack = NULL;
		ss = NULL;
		ssacrosstrack = NULL;
		ssalongtrack = NULL;
		}
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(char), (void **)&beamflag, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bath, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE,
						sizeof(double), (void **)&amp, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bathacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bathalongtrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
						sizeof(double), (void **)&ss, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
						sizeof(double), (void **)&ssacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
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

	/* reset file record counters */
	nrec_0x30_parameter_stop = 0;
	nrec_0x31_parameter_off = 0;
	nrec_0x32_parameter_on = 0;
	nrec_0x33_parameter_extra = 0;
	nrec_0x41_attitude = 0;
	nrec_0x43_clock = 0;
	nrec_0x44_bathymetry = 0;
	nrec_0x45_singlebeam = 0;
	nrec_0x46_rawbeamF = 0;
	nrec_0x47_surfacesoundspeed2 = 0;
	nrec_0x48_heading = 0;
	nrec_0x49_parameter_start = 0;
	nrec_0x4A_tilt = 0;
	nrec_0x4B_echogram = 0;
	nrec_0x4E_rawbeamN = 0;
	nrec_0x50_pos = 0;
	nrec_0x52_runtime = 0;
	nrec_0x53_sidescan = 0;
	nrec_0x54_tide = 0;
	nrec_0x55_svp2 = 0;
	nrec_0x56_svp = 0;
	nrec_0x57_surfacesoundspeed = 0;
	nrec_0x58_bathymetry2 = 0;
	nrec_0x59_sidescan2 = 0;
	nrec_0x66_rawbeamf = 0;
	nrec_0x68_height = 0;
	nrec_0x69_parameter_stop = 0;
	nrec_0x6B_water_column = 0;
	nrec_0x6E_network_attitude = 0;
	nrec_0x70_parameter = 0;
	nrec_0x73_surface_sound_speed = 0;
	nrec_0xE1_bathymetry_mbari57 = 0;
	nrec_0xE2_sidescan_mbari57 = 0;
	nrec_0xE3_bathymetry_mbari59 = 0;
	nrec_0xE4_sidescan_mbari59 = 0;
	nrec_0xE5_bathymetry_mbari59 = 0;

	/* read and print data */
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* reset error */
		error = MB_ERROR_NO_ERROR;

		/* read next data record */
		status = mb_get_all(verbose,imbio_ptr,&istore_ptr,&kind,
				    time_i,&time_d,&navlon,&navlat,
				    &speed,&heading,
				    &distance,&altitude,&sonardepth,
				    &beams_bath,&beams_amp,&pixels_ss,
				    beamflag,bath,amp,bathacrosstrack,bathalongtrack,
				    ss,ssacrosstrack,ssalongtrack,
				    comment,&error);

		/* some nonfatal errors do not matter */
		if (error < MB_ERROR_NO_ERROR && error > MB_ERROR_UNINTELLIGIBLE)
			{
			error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}

		/* count the record that was just read */
		if (status == MB_SUCCESS && kind == MB_DATA_DATA)
			{
			/* get survey data structure */
			ping = (struct mbsys_simrad3_ping_struct *) istore->ping;

			if (format == MBF_EM300RAW)
				{
				nrec_0x58_bathymetry2++;
				if (ping->png_raw4_read == MB_YES)
					nrec_0x4E_rawbeamN++;
				if (ping->png_ss2_read == MB_YES)
					nrec_0x59_sidescan2++;
				}
			else if (format == MBF_EM300MBA)
				{
				nrec_0xE5_bathymetry_mbari59++;
				if (ping->png_raw4_read == MB_YES)
					nrec_0x4E_rawbeamN++;
				if (ping->png_ss2_read == MB_YES)
					nrec_0x59_sidescan2++;
				}
			else if (format == MBF_EM710RAW)
				{
				nrec_0x58_bathymetry2++;
				if (ping->png_raw4_read == MB_YES)
					nrec_0x4E_rawbeamN++;
				if (ping->png_ss2_read == MB_YES)
					nrec_0x59_sidescan2++;
				}
			else if (format == MBF_EM710MBA)
				{
				nrec_0xE5_bathymetry_mbari59++;
				if (ping->png_raw4_read == MB_YES)
					nrec_0x4E_rawbeamN++;
				if (ping->png_ss2_read == MB_YES)
					nrec_0x59_sidescan2++;
				}
			}
		else if (status == MB_SUCCESS)
			{
			if (istore->type == EM3_STOP2)
				nrec_0x30_parameter_stop++;
			if (istore->type == EM3_OFF)
				nrec_0x31_parameter_off++;
			if (istore->type == EM3_ON)
				nrec_0x32_parameter_on++;
			if (istore->type == EM3_ATTITUDE)
				nrec_0x41_attitude++;
			if (istore->type == EM3_CLOCK)
				nrec_0x43_clock++;
			if (istore->type == EM3_BATH)
				nrec_0x44_bathymetry++;
			if (istore->type == EM3_SBDEPTH)
				nrec_0x45_singlebeam++;
			if (istore->type == EM3_RAWBEAM)
				nrec_0x46_rawbeamF++;
			if (istore->type == EM3_SSV)
				nrec_0x47_surfacesoundspeed2++;
			if (istore->type == EM3_HEADING)
				nrec_0x48_heading++;
			if (istore->type == EM3_START)
				nrec_0x49_parameter_start++;
			if (istore->type == EM3_TILT)
				nrec_0x4A_tilt++;
			if (istore->type == EM3_CBECHO)
				nrec_0x4B_echogram++;
			if (istore->type == EM3_RAWBEAM4)
				nrec_0x4E_rawbeamN++;
			if (istore->type == EM3_POS)
				nrec_0x50_pos++;
			if (istore->type == EM3_RUN_PARAMETER)
				nrec_0x52_runtime++;
			if (istore->type == EM3_SS)
				nrec_0x53_sidescan++;
			if (istore->type == EM3_TIDE)
				nrec_0x54_tide++;
			if (istore->type == EM3_SVP2)
				nrec_0x55_svp2++;
			if (istore->type == EM3_SVP)
				nrec_0x56_svp++;
			if (istore->type == EM3_SSPINPUT)
				nrec_0x57_surfacesoundspeed++;
			if (istore->type == EM3_BATH2)
				nrec_0x58_bathymetry2++;
			if (istore->type == EM3_SS2)
				nrec_0x59_sidescan2++;
			if (istore->type == EM3_RAWBEAM3)
				nrec_0x66_rawbeamf++;
			if (istore->type == EM3_HEIGHT)
				nrec_0x68_height++;
			if (istore->type == EM3_STOP)
				nrec_0x69_parameter_stop++;
			if (istore->type == EM3_WATERCOLUMN)
				nrec_0x6B_water_column++;
			if (istore->type == EM3_NETATTITUDE)
				nrec_0x6E_network_attitude++;
			if (istore->type == EM3_REMOTE)
				nrec_0x70_parameter++;
			if (istore->type == EM3_SSP)
				nrec_0x73_surface_sound_speed++;
			if (istore->type == EM3_BATH_MBA)
				nrec_0xE1_bathymetry_mbari57++;
			if (istore->type == EM3_SS_MBA)
				nrec_0xE2_sidescan_mbari57++;
			if (istore->type == EM3_BATH2_MBA)
				nrec_0xE3_bathymetry_mbari59++;
			if (istore->type == EM3_SS2_MBA)
				nrec_0xE4_sidescan_mbari59++;
			if (istore->type == EM3_BATH3_MBA)
				nrec_0xE5_bathymetry_mbari59++;
			}

	   	/* save navigation and heading data from EM3_POS records */
		if (status == MB_SUCCESS
			&& istore->type == EM3_POS
			&& (istore->kind == nav_source
				|| istore->kind == heading_source))
			{
			/* get nav time */
			time_i[0] = istore->pos_date / 10000;
			time_i[1] = (istore->pos_date % 10000) / 100;
			time_i[2] = istore->pos_date % 100;
			time_i[3] = istore->pos_msec / 3600000;
			time_i[4] = (istore->pos_msec % 3600000) / 60000;
			time_i[5] = (istore->pos_msec % 60000) / 1000;
			time_i[6] = (istore->pos_msec % 1000) * 1000;
			mb_get_time(verbose, time_i, &time_d);

			if (MBKONSBERGPREPROCESS_TIMESTAMPLIST == MB_YES)
				fprintf(stderr,"Record time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d nrec_0x50_pos:%d\n",
						time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],nrec_0x50_pos);

			/* deal with desired navigation source and valid positions */
			if (istore->kind == nav_source
				&& istore->pos_longitude != EM3_INVALID_INT
				&& istore->pos_latitude != EM3_INVALID_INT)
				{
				/* allocate memory for position arrays if needed */
				if (ndat_nav + 1 >= ndat_nav_alloc)
					{
					ndat_nav_alloc +=  MBKONSBERGPREPROCESS_ALLOC_CHUNK;
					status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_nav_alloc*sizeof(double),(void **)&dat_nav_time_d,&error);
					status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_nav_alloc*sizeof(double),(void **)&dat_nav_lon,&error);
					status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_nav_alloc*sizeof(double),(void **)&dat_nav_lat,&error);
					if (error != MB_ERROR_NO_ERROR)
						{
						mb_error(verbose,error,&message);
						fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
						fprintf(stderr,"\nProgram <%s> Terminated\n",
						    program_name);
						exit(error);
						}
					}

				/* store the position data */
				if (ndat_nav == 0 || dat_nav_time_d[ndat_nav-1] < time_d)
					{
					dat_nav_time_d[ndat_nav] = time_d;
					dat_nav_lon[ndat_nav] = (double)(0.0000001 * istore->pos_longitude);
					dat_nav_lat[ndat_nav] = (double)(0.00000005 * istore->pos_latitude);

					/* apply time lag correction if specified */
					if (timelagmode == MBKONSBERGPREPROCESS_TIMELAG_CONSTANT)
						{
						dat_nav_time_d[ndat_nav] -= timelagconstant;
						}
					else if (timelagmode == MBKONSBERGPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
						{
						interp_status = mb_linear_interp(verbose,
									timelag_time_d-1, timelag_model-1,
									ntimelag, dat_nav_time_d[ndat_nav], &timelag, &jtimelag,
									&error);
						dat_nav_time_d[ndat_nav] -= timelag;
						}

					/* increment counter */
					ndat_nav++;
					}
				}

			/* deal with desired heading source and valid heading */
			if (istore->kind == heading_source
				&& istore->pos_heading != EM3_INVALID_INT)
				{

				/* allocate memory for position arrays if needed */
				if (ndat_heading + 1 >= ndat_heading_alloc)
					{
					ndat_heading_alloc +=  MBKONSBERGPREPROCESS_ALLOC_CHUNK;
					status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_heading_alloc*sizeof(double),(void **)&dat_heading_time_d,&error);
					status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_heading_alloc*sizeof(double),(void **)&dat_heading_heading,&error);
					if (error != MB_ERROR_NO_ERROR)
						{
						mb_error(verbose,error,&message);
						fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
						fprintf(stderr,"\nProgram <%s> Terminated\n",
						    program_name);
						exit(error);
						}
					}

				/* store the heading data */
				if (ndat_heading == 0 || dat_heading_time_d[ndat_heading-1] < time_d)
					{
					dat_heading_time_d[ndat_heading] = time_d;
					dat_heading_heading[ndat_heading] = (double)(0.01 * istore->pos_heading);

					/* apply time lag correction if specified */
					if (timelagmode == MBKONSBERGPREPROCESS_TIMELAG_CONSTANT)
						{
						dat_heading_time_d[ndat_heading] -= timelagconstant;
						}
					else if (timelagmode == MBKONSBERGPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
						{
						interp_status = mb_linear_interp(verbose,
									timelag_time_d-1, timelag_model-1,
									ntimelag, dat_heading_time_d[ndat_heading], &timelag, &jtimelag,
									&error);
						dat_nav_time_d[ndat_heading] -= timelag;
						}

					/* increment counter */
					ndat_heading++;
					}
				}

			}

	   	/* save primary attitude data from attitude records */
		if (status == MB_SUCCESS
			&& istore->type == EM3_ATTITUDE
		    	&& istore->kind == attitude_source)
			{
			/* get attitude structure */
			attitude = (struct mbsys_simrad3_attitude_struct *) istore->attitude;

			/* get attitude time */
			time_i[0] = attitude->att_date / 10000;
			time_i[1] = (attitude->att_date % 10000) / 100;
			time_i[2] = attitude->att_date % 100;
			time_i[3] = attitude->att_msec / 3600000;
			time_i[4] = (attitude->att_msec % 3600000) / 60000;
			time_i[5] = (attitude->att_msec % 60000) / 1000;
			time_i[6] = (attitude->att_msec % 1000) * 1000;
			mb_get_time(verbose, time_i, &time_d);

			if (MBKONSBERGPREPROCESS_TIMESTAMPLIST == MB_YES)
				fprintf(stderr,"Record time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d nrec_0x41_attitude:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],nrec_0x41_attitude);

			/* allocate memory for position arrays if needed */
			if (ndat_rph + attitude->att_ndata >= ndat_rph_alloc)
				{
				ndat_rph_alloc +=  MBKONSBERGPREPROCESS_ALLOC_CHUNK;
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_time_d,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_roll,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_pitch,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_heave,&error);
				if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
					    program_name);
					exit(error);
					}
				}

			/* store the attitude data */
			if (ndat_rph == 0 || dat_rph_time_d[ndat_rph-1] < time_d)
				{
				for (i=0;i<attitude->att_ndata;i++)
					{
					dat_rph_time_d[ndat_rph] = (double)(time_d + 0.001 * attitude->att_time[i]);
					dat_rph_heave[ndat_rph] = (double)(0.01 * attitude->att_heave[i]);
					dat_rph_roll[ndat_rph] = (double)(0.01 * attitude->att_roll[i]);
					dat_rph_pitch[ndat_rph] = (double)(0.01 * attitude->att_pitch[i]);

					/* apply time lag correction if specified */
					if (timelagmode == MBKONSBERGPREPROCESS_TIMELAG_CONSTANT)
						{
						dat_rph_time_d[ndat_rph] -= timelagconstant;
						}
					else if (timelagmode == MBKONSBERGPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
						{
						interp_status = mb_linear_interp(verbose,
									timelag_time_d-1, timelag_model-1,
									ntimelag, dat_rph_time_d[ndat_rph], &timelag, &jtimelag,
									&error);
						dat_rph_time_d[ndat_rph] -= timelag;
						}

					/* increment counter */
					ndat_rph++;
					}
				}
			}

	   	/* save primary attitude data from netattitude records */
		if (status == MB_SUCCESS
			&& istore->type == EM3_NETATTITUDE
		    	&& istore->kind == attitude_source)
			{
			/* get netattitude structure */
			netattitude = (struct mbsys_simrad3_netattitude_struct *) istore->netattitude;

			/* get attitude time */
			time_i[0] = netattitude->nat_date / 10000;
			time_i[1] = (netattitude->nat_date % 10000) / 100;
			time_i[2] = netattitude->nat_date % 100;
			time_i[3] = netattitude->nat_msec / 3600000;
			time_i[4] = (netattitude->nat_msec % 3600000) / 60000;
			time_i[5] = (netattitude->nat_msec % 60000) / 1000;
			time_i[6] = (netattitude->nat_msec % 1000) * 1000;
			mb_get_time(verbose, time_i, &time_d);

			if (MBKONSBERGPREPROCESS_TIMESTAMPLIST == MB_YES)
				fprintf(stderr,"Record time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d nrec_0x6E_network_attitude:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],nrec_0x6E_network_attitude);

			/* allocate memory for position arrays if needed */
			if (ndat_rph + netattitude->nat_ndata >= ndat_rph_alloc)
				{
				ndat_rph_alloc +=  MBKONSBERGPREPROCESS_ALLOC_CHUNK;
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_time_d,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_roll,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_pitch,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_heave,&error);
				if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
					    program_name);
					exit(error);
					}
				}

			/* store the attitude data */
			if (ndat_rph == 0 || dat_rph_time_d[ndat_rph-1] < time_d)
				{
				for (i=0;i<netattitude->nat_ndata;i++)
					{
					dat_rph_time_d[ndat_rph] = (double)(time_d + 0.001 * netattitude->nat_time[i]);
					dat_rph_heave[ndat_rph] = (double)(0.01 * netattitude->nat_heave[i]);
					dat_rph_roll[ndat_rph] = (double)(0.01 * netattitude->nat_roll[i]);
					dat_rph_pitch[ndat_rph] = (double)(0.01 * netattitude->nat_pitch[i]);

					/* apply time lag correction if specified */
					if (timelagmode == MBKONSBERGPREPROCESS_TIMELAG_CONSTANT)
						{
						dat_rph_time_d[ndat_rph] -= timelagconstant;
						}
					else if (timelagmode == MBKONSBERGPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
						{
						interp_status = mb_linear_interp(verbose,
									timelag_time_d-1, timelag_model-1,
									ntimelag, dat_rph_time_d[ndat_rph], &timelag, &jtimelag,
									&error);
						dat_rph_time_d[ndat_rph] -= timelag;
						}

					/* increment counter */
					ndat_rph++;
					}
				}
			}

	   	/* save primary heading data */
		if (status == MB_SUCCESS
			&& istore->type == EM3_HEADING
		    	&& istore->kind == heading_source)
			{
			/* get heading structure */
			headingr = (struct mbsys_simrad3_heading_struct *) istore->heading;

			/* get heading time */
			time_i[0] = headingr->hed_date / 10000;
			time_i[1] = (headingr->hed_date % 10000) / 100;
			time_i[2] = headingr->hed_date % 100;
			time_i[3] = headingr->hed_msec / 3600000;
			time_i[4] = (headingr->hed_msec % 3600000) / 60000;
			time_i[5] = (headingr->hed_msec % 60000) / 1000;
			time_i[6] = (headingr->hed_msec % 1000) * 1000;
			mb_get_time(verbose, time_i, &time_d);

			if (MBKONSBERGPREPROCESS_TIMESTAMPLIST == MB_YES)
				fprintf(stderr,"Record time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d nrec_0x48_heading:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],nrec_0x48_heading);

			/* allocate memory for position arrays if needed */
			if (ndat_heading + headingr->hed_ndata >= ndat_heading_alloc)
				{
				ndat_heading_alloc +=  MBKONSBERGPREPROCESS_ALLOC_CHUNK;
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_heading_alloc*sizeof(double),(void **)&dat_heading_time_d,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_heading_alloc*sizeof(double),(void **)&dat_heading_heading,&error);
				if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
					    program_name);
					exit(error);
					}
				}

			/* store the heading data */
			if (ndat_heading == 0 || dat_heading_time_d[ndat_heading-1] < time_d)
				{
				for (i=0;i<headingr->hed_ndata;i++)
					{
					dat_heading_time_d[ndat_heading] = (double)(time_d + 0.001 * headingr->hed_time[i]);
					dat_heading_heading[ndat_heading] = (double)(0.01 * headingr->hed_heading[i]);

					/* apply time lag correction if specified */
					if (timelagmode == MBKONSBERGPREPROCESS_TIMELAG_CONSTANT)
						{
						dat_heading_time_d[ndat_heading] -= timelagconstant;
						}
					else if (timelagmode == MBKONSBERGPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
						{
						interp_status = mb_linear_interp(verbose,
									timelag_time_d-1, timelag_model-1,
									ntimelag, dat_heading_time_d[ndat_heading], &timelag, &jtimelag,
									&error);
						dat_heading_time_d[ndat_heading] -= timelag;
						}

					/* increment counter */
					ndat_heading++;
					}
				}
			}

	   	/* handle unknown data */
		else  if (status == MB_SUCCESS)
			{
/*fprintf(stderr,"DATA TYPE UNKNOWN: status:%d error:%d kind:%d\n",status,error,kind);*/
			}

	   	/* handle read error */
		else
			{
/*fprintf(stderr,"READ FAILURE: status:%d error:%d kind:%d\n",status,error,kind);*/
			}

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",
				program_name);
			fprintf(stderr,"dbg2       kind:           %d\n",kind);
			fprintf(stderr,"dbg2       error:          %d\n",error);
			fprintf(stderr,"dbg2       status:         %d\n",status);
			}
		}

	/* close the swath file */
	status = mb_close(verbose,&imbio_ptr,&error);

	/* output counts */
	if (output_counts == MB_YES)
		{
		fprintf(stdout, "\nData records read from: %s\n", ifile);
		fprintf(stdout, "     nrec_0x30_parameter_stop:         %d\n", nrec_0x30_parameter_stop);
		fprintf(stdout, "     nrec_0x31_parameter_off:          %d\n", nrec_0x31_parameter_off);
		fprintf(stdout, "     nrec_0x32_parameter_on:           %d\n", nrec_0x32_parameter_on);
		fprintf(stdout, "     nrec_0x33_parameter_extra:        %d\n", nrec_0x33_parameter_extra);
		fprintf(stdout, "     nrec_0x41_attitude:               %d\n", nrec_0x41_attitude);
		fprintf(stdout, "     nrec_0x43_clock:                  %d\n", nrec_0x43_clock);
		fprintf(stdout, "     nrec_0x44_bathymetry:             %d\n", nrec_0x44_bathymetry);
		fprintf(stdout, "     nrec_0x45_singlebeam:             %d\n", nrec_0x45_singlebeam);
		fprintf(stdout, "     nrec_0x46_rawbeamF:               %d\n", nrec_0x46_rawbeamF);
		fprintf(stdout, "     nrec_0x47_surfacesoundspeed2:     %d\n", nrec_0x47_surfacesoundspeed2);
		fprintf(stdout, "     nrec_0x48_heading:                %d\n", nrec_0x48_heading);
		fprintf(stdout, "     nrec_0x49_parameter_start:        %d\n", nrec_0x49_parameter_start);
		fprintf(stdout, "     nrec_0x4A_tilt:                   %d\n", nrec_0x4A_tilt);
		fprintf(stdout, "     nrec_0x4B_echogram:               %d\n", nrec_0x4B_echogram);
		fprintf(stdout, "     nrec_0x4E_rawbeamN:               %d\n", nrec_0x4E_rawbeamN);
		fprintf(stdout, "     nrec_0x50_pos:                    %d\n", nrec_0x50_pos);
		fprintf(stdout, "     nrec_0x52_runtime:                %d\n", nrec_0x52_runtime);
		fprintf(stdout, "     nrec_0x53_sidescan:               %d\n", nrec_0x53_sidescan);
		fprintf(stdout, "     nrec_0x54_tide:                   %d\n", nrec_0x54_tide);
		fprintf(stdout, "     nrec_0x55_svp2:                   %d\n", nrec_0x55_svp2);
		fprintf(stdout, "     nrec_0x56_svp:                    %d\n", nrec_0x56_svp);
		fprintf(stdout, "     nrec_0x57_surfacesoundspeed:      %d\n", nrec_0x57_surfacesoundspeed);
		fprintf(stdout, "     nrec_0x58_bathymetry2:            %d\n", nrec_0x58_bathymetry2);
		fprintf(stdout, "     nrec_0x59_sidescan2:              %d\n", nrec_0x59_sidescan2);
		fprintf(stdout, "     nrec_0x66_rawbeamf:               %d\n", nrec_0x66_rawbeamf);
		fprintf(stdout, "     nrec_0x68_height:                 %d\n", nrec_0x68_height);
		fprintf(stdout, "     nrec_0x69_parameter_stop:         %d\n", nrec_0x69_parameter_stop);
		fprintf(stdout, "     nrec_0x6B_water_column:           %d\n", nrec_0x6B_water_column);
		fprintf(stdout, "     nrec_0x6E_network_attitude:       %d\n", nrec_0x6E_network_attitude);
		fprintf(stdout, "     nrec_0x70_parameter:              %d\n", nrec_0x70_parameter);
		fprintf(stdout, "     nrec_0x73_surface_sound_speed:    %d\n", nrec_0x73_surface_sound_speed);
		fprintf(stdout, "     nrec_0xE1_bathymetry_mbari57:     %d\n", nrec_0xE1_bathymetry_mbari57);
		fprintf(stdout, "     nrec_0xE2_sidescan_mbari57:       %d\n", nrec_0xE2_sidescan_mbari57);
		fprintf(stdout, "     nrec_0xE3_bathymetry_mbari59:     %d\n", nrec_0xE3_bathymetry_mbari59);
		fprintf(stdout, "     nrec_0xE4_sidescan_mbari59:       %d\n", nrec_0xE4_sidescan_mbari59);
		fprintf(stdout, "     nrec_0xE5_bathymetry_mbari59:     %d\n", nrec_0xE5_bathymetry_mbari59);

		nrec_0x30_parameter_stop_tot += nrec_0x30_parameter_stop;
		nrec_0x31_parameter_off_tot += nrec_0x31_parameter_off;
		nrec_0x32_parameter_on_tot += nrec_0x32_parameter_on;
		nrec_0x33_parameter_extra_tot += nrec_0x33_parameter_extra;
		nrec_0x41_attitude_tot += nrec_0x41_attitude;
		nrec_0x43_clock_tot += nrec_0x43_clock;
		nrec_0x44_bathymetry_tot += nrec_0x44_bathymetry;
		nrec_0x45_singlebeam_tot += nrec_0x45_singlebeam;
		nrec_0x46_rawbeamF_tot += nrec_0x46_rawbeamF;
		nrec_0x47_surfacesoundspeed2_tot += nrec_0x47_surfacesoundspeed2;
		nrec_0x48_heading_tot += nrec_0x48_heading;
		nrec_0x49_parameter_start_tot += nrec_0x49_parameter_start;
		nrec_0x4A_tilt_tot += nrec_0x4A_tilt;
		nrec_0x4B_echogram_tot += nrec_0x4B_echogram;
		nrec_0x4E_rawbeamN_tot += nrec_0x4E_rawbeamN;
		nrec_0x50_pos_tot += nrec_0x50_pos;
		nrec_0x52_runtime_tot += nrec_0x52_runtime;
		nrec_0x53_sidescan_tot += nrec_0x53_sidescan;
		nrec_0x54_tide_tot += nrec_0x54_tide;
		nrec_0x55_svp2_tot += nrec_0x55_svp2;
		nrec_0x56_svp_tot += nrec_0x56_svp;
		nrec_0x57_surfacesoundspeed_tot += nrec_0x57_surfacesoundspeed;
		nrec_0x58_bathymetry2_tot += nrec_0x58_bathymetry2;
		nrec_0x59_sidescan2_tot += nrec_0x59_sidescan2;
		nrec_0x66_rawbeamf_tot += nrec_0x66_rawbeamf;
		nrec_0x68_height_tot += nrec_0x68_height;
		nrec_0x69_parameter_stop_tot += nrec_0x69_parameter_stop;
		nrec_0x6B_water_column_tot += nrec_0x6B_water_column;
		nrec_0x6E_network_attitude_tot += nrec_0x6E_network_attitude;
		nrec_0x70_parameter_tot += nrec_0x70_parameter;
		nrec_0x73_surface_sound_speed_tot += nrec_0x73_surface_sound_speed;
		nrec_0xE1_bathymetry_mbari57_tot += nrec_0xE1_bathymetry_mbari57;
		nrec_0xE2_sidescan_mbari57_tot += nrec_0xE2_sidescan_mbari57;
		nrec_0xE3_bathymetry_mbari59_tot += nrec_0xE3_bathymetry_mbari59;
		nrec_0xE4_sidescan_mbari59_tot += nrec_0xE4_sidescan_mbari59;
		nrec_0xE5_bathymetry_mbari59_tot += nrec_0xE5_bathymetry_mbari59;
		}

	/* figure out whether and what to read next */
        if (read_datalist == MB_YES)
                {
		if ((status = mb_datalist_read(verbose,datalist,
			    ifile,&format,&file_weight,&error))
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

	/* output asynchronous navigation and attitude data */
	if (verbose > 0 || mode == MBKONSBERGPREPROCESS_TIMESTAMPLIST)
		fprintf(stdout, "\nTotal navigation data read: %d\n", ndat_nav);
	if (mode == MBKONSBERGPREPROCESS_TIMESTAMPLIST)
		for (i=0;i<ndat_nav;i++)
			{
			fprintf(stdout, "  NAV: %5d %17.6f %11.6f %10.6f\n",
				i, dat_nav_time_d[i], dat_nav_lon[i], dat_nav_lat[i]);
			}
	if (verbose > 0 || mode == MBKONSBERGPREPROCESS_TIMESTAMPLIST)
		fprintf(stdout, "\nTotal heading data read: %d\n", ndat_heading);
	if (mode == MBKONSBERGPREPROCESS_TIMESTAMPLIST)
		for (i=0;i<ndat_heading;i++)
			{
			fprintf(stdout, "  HDG: %5d %17.6f %8.3f\n",
				i, dat_heading_time_d[i], dat_heading_heading[i]);
			}
	if (verbose > 0 || mode == MBKONSBERGPREPROCESS_TIMESTAMPLIST)
		fprintf(stdout, "\nTotal attitude data read: %d\n", ndat_rph);
	if (mode == MBKONSBERGPREPROCESS_TIMESTAMPLIST)
		for (i=0;i<ndat_rph;i++)
			{
			fprintf(stdout, "  HCP: %5d %17.6f %8.3f %8.3f %8.3f\n",
				i, dat_rph_time_d[i], dat_rph_roll[i], dat_rph_pitch[i], dat_rph_heave[i]);
			}

	/* output counts */
	if (output_counts == MB_YES)
		{
		fprintf(stdout, "\nTotal data records read from: %s\n", read_file);
		fprintf(stdout, "     nrec_0x30_parameter_stop_tot:     %d\n", nrec_0x30_parameter_stop_tot);
		fprintf(stdout, "     nrec_0x31_parameter_off_tot:      %d\n", nrec_0x31_parameter_off_tot);
		fprintf(stdout, "     nrec_0x32_parameter_on_tot:       %d\n", nrec_0x32_parameter_on_tot);
		fprintf(stdout, "     nrec_0x33_parameter_extra_tot:    %d\n", nrec_0x33_parameter_extra_tot);
		fprintf(stdout, "     nrec_0x41_attitude_tot:           %d\n", nrec_0x41_attitude_tot);
		fprintf(stdout, "     nrec_0x43_clock_tot:              %d\n", nrec_0x43_clock_tot);
		fprintf(stdout, "     nrec_0x44_bathymetry_tot:         %d\n", nrec_0x44_bathymetry_tot);
		fprintf(stdout, "     nrec_0x45_singlebeam_tot:         %d\n", nrec_0x45_singlebeam_tot);
		fprintf(stdout, "     nrec_0x46_rawbeamF_tot:           %d\n", nrec_0x46_rawbeamF_tot);
		fprintf(stdout, "     nrec_0x47_surfacesoundspeed2_tot: %d\n", nrec_0x47_surfacesoundspeed2_tot);
		fprintf(stdout, "     nrec_0x48_heading_tot:            %d\n", nrec_0x48_heading_tot);
		fprintf(stdout, "     nrec_0x49_parameter_start_tot:    %d\n", nrec_0x49_parameter_start_tot);
		fprintf(stdout, "     nrec_0x4A_tilt_tot:               %d\n", nrec_0x4A_tilt_tot);
		fprintf(stdout, "     nrec_0x4B_echogram_tot:           %d\n", nrec_0x4B_echogram_tot);
		fprintf(stdout, "     nrec_0x4E_rawbeamN_tot:           %d\n", nrec_0x4E_rawbeamN_tot);
		fprintf(stdout, "     nrec_0x50_pos_tot:                %d\n", nrec_0x50_pos_tot);
		fprintf(stdout, "     nrec_0x52_runtime_tot:            %d\n", nrec_0x52_runtime_tot);
		fprintf(stdout, "     nrec_0x53_sidescan_tot:           %d\n", nrec_0x53_sidescan_tot);
		fprintf(stdout, "     nrec_0x54_tide_tot:               %d\n", nrec_0x54_tide_tot);
		fprintf(stdout, "     nrec_0x55_svp2_tot:               %d\n", nrec_0x55_svp2_tot);
		fprintf(stdout, "     nrec_0x56_svp_tot:                %d\n", nrec_0x56_svp_tot);
		fprintf(stdout, "     nrec_0x57_surfacesoundspeed_tot:  %d\n", nrec_0x57_surfacesoundspeed_tot);
		fprintf(stdout, "     nrec_0x58_bathymetry2_tot:        %d\n", nrec_0x58_bathymetry2_tot);
		fprintf(stdout, "     nrec_0x59_sidescan2_tot:          %d\n", nrec_0x59_sidescan2_tot);
		fprintf(stdout, "     nrec_0x66_rawbeamf_tot:           %d\n", nrec_0x66_rawbeamf_tot);
		fprintf(stdout, "     nrec_0x68_height_tot:             %d\n", nrec_0x68_height_tot);
		fprintf(stdout, "     nrec_0x69_parameter_stop_tot:     %d\n", nrec_0x69_parameter_stop_tot);
		fprintf(stdout, "     nrec_0x6B_water_column_tot:       %d\n", nrec_0x6B_water_column_tot);
		fprintf(stdout, "     nrec_0x6E_network_attitude_tot:   %d\n", nrec_0x6E_network_attitude_tot);
		fprintf(stdout, "     nrec_0x70_parameter_tot:          %d\n", nrec_0x70_parameter_tot);
		fprintf(stdout, "     nrec_0x73_surface_sound_speed_tot:%d\n", nrec_0x73_surface_sound_speed_tot);
		fprintf(stdout, "     nrec_0xE1_bathymetry_mbari57_tot: %d\n", nrec_0xE1_bathymetry_mbari57_tot);
		fprintf(stdout, "     nrec_0xE2_sidescan_mbari57_tot:   %d\n", nrec_0xE2_sidescan_mbari57_tot);
		fprintf(stdout, "     nrec_0xE3_bathymetry_mbari59_tot: %d\n", nrec_0xE3_bathymetry_mbari59_tot);
		fprintf(stdout, "     nrec_0xE4_sidescan_mbari59_tot:   %d\n", nrec_0xE4_sidescan_mbari59_tot);
		fprintf(stdout, "     nrec_0xE5_bathymetry_mbari59_tot: %d\n", nrec_0xE5_bathymetry_mbari59_tot);
		}
	nrec_0x30_parameter_stop_tot = 0;
	nrec_0x31_parameter_off_tot = 0;
	nrec_0x32_parameter_on_tot = 0;
	nrec_0x33_parameter_extra_tot = 0;
	nrec_0x41_attitude_tot = 0;
	nrec_0x43_clock_tot = 0;
	nrec_0x44_bathymetry_tot = 0;
	nrec_0x45_singlebeam_tot = 0;
	nrec_0x46_rawbeamF_tot = 0;
	nrec_0x47_surfacesoundspeed2_tot = 0;
	nrec_0x48_heading_tot = 0;
	nrec_0x49_parameter_start_tot = 0;
	nrec_0x4A_tilt_tot = 0;
	nrec_0x4B_echogram_tot = 0;
	nrec_0x4E_rawbeamN_tot = 0;
	nrec_0x50_pos_tot = 0;
	nrec_0x52_runtime_tot = 0;
	nrec_0x53_sidescan_tot = 0;
	nrec_0x54_tide_tot = 0;
	nrec_0x55_svp2_tot = 0;
	nrec_0x56_svp_tot = 0;
	nrec_0x57_surfacesoundspeed_tot = 0;
	nrec_0x58_bathymetry2_tot = 0;
	nrec_0x59_sidescan2_tot = 0;
	nrec_0x66_rawbeamf_tot = 0;
	nrec_0x68_height_tot = 0;
	nrec_0x69_parameter_stop_tot = 0;
	nrec_0x6B_water_column_tot = 0;
	nrec_0x6E_network_attitude_tot = 0;
	nrec_0x70_parameter_tot = 0;
	nrec_0x73_surface_sound_speed_tot = 0;
	nrec_0xE1_bathymetry_mbari57_tot = 0;
	nrec_0xE2_sidescan_mbari57_tot = 0;
	nrec_0xE3_bathymetry_mbari59_tot = 0;
	nrec_0xE4_sidescan_mbari59_tot = 0;
	nrec_0xE5_bathymetry_mbari59_tot = 0;

	/* now read the data files again, this time interpolating nav and attitude
		into the multibeam records and fixing other problems found in the
		data */
	if (mode == MBKONSBERGPREPROCESS_PROCESS)
	{

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
	    if ((status = mb_datalist_read(verbose,datalist,
			    ifile,&format,&file_weight,&error))
			    == MB_SUCCESS)
		read_data = MB_YES;
	    else
		read_data = MB_NO;
	    }
	/* else copy single filename to be read */
	else
	    {
	    strcpy(ifile, read_file);
	    read_data = MB_YES;
	    }

	/* loop over all files to be read */
	while (read_data == MB_YES && (format == MBF_EM710RAW || format == MBF_EM710MBA))
	{
	/* figure out the output file name if not specified */
	if (ofile_set == MB_NO)
		{
		status = mb_get_format(verbose, ifile, fileroot, &testformat, &error);
		if (format == MBF_EM710MBA
			&& strncmp(".mb59",&ifile[strlen(ifile)-5],5) == 0)
			sprintf(ofile, "%sf.mb%d", fileroot, MBF_EM710MBA);
		else
			sprintf(ofile, "%s.mb%d", fileroot, MBF_EM710MBA);
		}

	/* if output directory was set by user, reset file path */
	if (odir_set == MB_YES && odir!=NULL)
		{
		strcpy(buffer, odir);
		if (buffer[strlen(odir)-1] != '/')
			strcat(buffer, "/");
		if (strrchr(ofile, '/') != NULL)
			filenameptr = strrchr(ofile, '/') + 1;
		else
			filenameptr = ofile;
		strcat(buffer, filenameptr);
		strcpy(ofile,buffer);

		}

	/* initialize reading the input swath file */
	if ((status = mb_read_init(
		verbose,ifile,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&imbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",ifile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	nfile_read++;

	/* if ofile has been set then there is only one output file, otherwise there
		is an output file for each input file */
	if (ofile_set == MB_NO || nfile_write == 0)
		{
		/* initialize writing the output swath sonar file */
		if ((status = mb_write_init(
			verbose,ofile,MBF_EM710MBA,&ombio_ptr,
			&obeams_bath,&obeams_amp,&opixels_ss,&error)) != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error returned from function <mb_write_init>:\n%s\n",message);
			fprintf(stderr,"\nMultibeam File <%s> not initialized for writing\n",ofile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		nfile_write++;

		/* initialize asynchronous sonardepth output file */
		sprintf(atsfile,"%s.ats",ofile);
		if ((atsfp = fopen(atsfile, "w")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open asynchronous sonardepth data file <%s> for writing\n",atsfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		/* initialize synchronous attitude output file */
		sprintf(stafile,"%s.sta",ofile);
		if ((stafp = fopen(stafile, "w")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open synchronous attitude data file <%s> for writing\n",stafile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		}

	/* get pointers to data storage */
	imb_io_ptr = (struct mb_io_struct *) imbio_ptr;
	istore_ptr = imb_io_ptr->store_data;
	istore = (struct mbsys_simrad3_struct *) istore_ptr;

	if (error == MB_ERROR_NO_ERROR)
		{
		beamflag = NULL;
		bath = NULL;
		amp = NULL;
		bathacrosstrack = NULL;
		bathalongtrack = NULL;
		ss = NULL;
		ssacrosstrack = NULL;
		ssalongtrack = NULL;
		}
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(char), (void **)&beamflag, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bath, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE,
						sizeof(double), (void **)&amp, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bathacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bathalongtrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
						sizeof(double), (void **)&ss, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
						sizeof(double), (void **)&ssacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
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

	/* reset file record counters */
	nrec_0x30_parameter_stop = 0;
	nrec_0x31_parameter_off = 0;
	nrec_0x32_parameter_on = 0;
	nrec_0x33_parameter_extra = 0;
	nrec_0x41_attitude = 0;
	nrec_0x43_clock = 0;
	nrec_0x44_bathymetry = 0;
	nrec_0x45_singlebeam = 0;
	nrec_0x46_rawbeamF = 0;
	nrec_0x47_surfacesoundspeed2 = 0;
	nrec_0x48_heading = 0;
	nrec_0x49_parameter_start = 0;
	nrec_0x4A_tilt = 0;
	nrec_0x4B_echogram = 0;
	nrec_0x4E_rawbeamN = 0;
	nrec_0x50_pos = 0;
	nrec_0x52_runtime = 0;
	nrec_0x53_sidescan = 0;
	nrec_0x54_tide = 0;
	nrec_0x55_svp2 = 0;
	nrec_0x56_svp = 0;
	nrec_0x57_surfacesoundspeed = 0;
	nrec_0x58_bathymetry2 = 0;
	nrec_0x59_sidescan2 = 0;
	nrec_0x66_rawbeamf = 0;
	nrec_0x68_height = 0;
	nrec_0x69_parameter_stop = 0;
	nrec_0x6B_water_column = 0;
	nrec_0x6E_network_attitude = 0;
	nrec_0x70_parameter = 0;
	nrec_0x73_surface_sound_speed = 0;
	nrec_0xE1_bathymetry_mbari57 = 0;
	nrec_0xE2_sidescan_mbari57 = 0;
	nrec_0xE3_bathymetry_mbari59 = 0;
	nrec_0xE4_sidescan_mbari59 = 0;
	nrec_0xE5_bathymetry_mbari59 = 0;

	/* read and write data */
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* reset error */
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;

		/* read next data record */
		status = mb_get_all(verbose,imbio_ptr,&istore_ptr,&kind,
				    time_i,&time_d,&navlon,&navlat,
				    &speed,&heading,
				    &distance,&altitude,&sonardepth,
				    &beams_bath,&beams_amp,&pixels_ss,
				    beamflag,bath,amp,bathacrosstrack,bathalongtrack,
				    ss,ssacrosstrack,ssalongtrack,
				    comment,&error);

		/* some nonfatal errors do not matter */
		if (error < MB_ERROR_NO_ERROR && error > MB_ERROR_UNINTELLIGIBLE)
			{
			error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}

		/* keep track of starting and ending time of sonar data for this file */
		if (status == MB_SUCCESS && kind == MB_DATA_DATA)
			{
			if (nrec_0xE5_bathymetry_mbari59 == 0)
				start_time_d = time_d;
			end_time_d = time_d;
			}

		/* count the record that was just read */
		if (status == MB_SUCCESS && kind == MB_DATA_DATA)
			{
			/* get survey data structure */
			ping = (struct mbsys_simrad3_ping_struct *) istore->ping;

			nrec_0xE5_bathymetry_mbari59++;
			if (ping->png_raw4_read == MB_YES)
				nrec_0x4E_rawbeamN++;
			if (ping->png_ss2_read == MB_YES)
				nrec_0x59_sidescan2++;
			}
		else if (status == MB_SUCCESS)
			{
			if (istore->type == EM3_STOP2)
				nrec_0x30_parameter_stop++;
			if (istore->type == EM3_OFF)
				nrec_0x31_parameter_off++;
			if (istore->type == EM3_ON)
				nrec_0x32_parameter_on++;
			if (istore->type == EM3_ATTITUDE)
				nrec_0x41_attitude++;
			if (istore->type == EM3_CLOCK)
				nrec_0x43_clock++;
			if (istore->type == EM3_BATH)
				nrec_0x44_bathymetry++;
			if (istore->type == EM3_SBDEPTH)
				nrec_0x45_singlebeam++;
			if (istore->type == EM3_RAWBEAM)
				nrec_0x46_rawbeamF++;
			if (istore->type == EM3_SSV)
				nrec_0x47_surfacesoundspeed2++;
			if (istore->type == EM3_HEADING)
				nrec_0x48_heading++;
			if (istore->type == EM3_START)
				nrec_0x49_parameter_start++;
			if (istore->type == EM3_TILT)
				nrec_0x4A_tilt++;
			if (istore->type == EM3_CBECHO)
				nrec_0x4B_echogram++;
			if (istore->type == EM3_RAWBEAM4)
				nrec_0x4E_rawbeamN++;
			if (istore->type == EM3_POS)
				nrec_0x50_pos++;
			if (istore->type == EM3_RUN_PARAMETER)
				nrec_0x52_runtime++;
			if (istore->type == EM3_SS)
				nrec_0x53_sidescan++;
			if (istore->type == EM3_TIDE)
				nrec_0x54_tide++;
			if (istore->type == EM3_SVP2)
				nrec_0x55_svp2++;
			if (istore->type == EM3_SVP)
				nrec_0x56_svp++;
			if (istore->type == EM3_SSPINPUT)
				nrec_0x57_surfacesoundspeed++;
			if (istore->type == EM3_BATH2)
				nrec_0x58_bathymetry2++;
			if (istore->type == EM3_SS2)
				nrec_0x59_sidescan2++;
			if (istore->type == EM3_RAWBEAM3)
				nrec_0x66_rawbeamf++;
			if (istore->type == EM3_HEIGHT)
				nrec_0x68_height++;
			if (istore->type == EM3_STOP)
				nrec_0x69_parameter_stop++;
			if (istore->type == EM3_WATERCOLUMN)
				nrec_0x6B_water_column++;
			if (istore->type == EM3_NETATTITUDE)
				nrec_0x6E_network_attitude++;
			if (istore->type == EM3_REMOTE)
				nrec_0x70_parameter++;
			if (istore->type == EM3_SSP)
				nrec_0x73_surface_sound_speed++;
			}

	   	/* handle multibeam data */
		if (status == MB_SUCCESS && kind == MB_DATA_DATA)
			{
			/* get survey data structure */
			ping = (struct mbsys_simrad3_ping_struct *) istore->ping;

			/* merge navigation from best available source */
			if (ndat_nav > 0)
				{
				interp_status = mb_linear_interp_degrees(verbose,
							dat_nav_time_d-1, dat_nav_lon-1,
							ndat_nav, time_d, &navlon, &jnav,
							&error);
				if (interp_status == MB_SUCCESS)
				interp_status = mb_linear_interp_degrees(verbose,
							dat_nav_time_d-1, dat_nav_lat-1,
							ndat_nav, time_d, &navlat, &jnav,
							&error);
				}
			else
				{
				navlon = 0.0;
				navlat = 0.0;
				speed = 0.0;
				}

			/* merge heading from best available source */
			if (ndat_heading > 0)
				{
				interp_status = mb_linear_interp_degrees(verbose,
							dat_heading_time_d-1, dat_heading_heading-1,
							ndat_heading, time_d, &heading, &jheading,
							&error);
				}
			else
				{
				heading = 0.0;
				}

			/* get attitude from best available source */
			if (ndat_rph > 0)
				{
				interp_status = mb_linear_interp(verbose,
							dat_rph_time_d-1, dat_rph_roll-1,
							ndat_rph, time_d, &roll, &jattitude,
							&error);
				if (interp_status == MB_SUCCESS)
				interp_status = mb_linear_interp(verbose,
							dat_rph_time_d-1, dat_rph_pitch-1,
							ndat_rph, time_d, &pitch, &jattitude,
							&error);
				if (interp_status == MB_SUCCESS)
				interp_status = mb_linear_interp(verbose,
							dat_rph_time_d-1, dat_rph_heave-1,
							ndat_rph, time_d, &heave, &jattitude,
							&error);
				}
			else
				{
				roll = 0.0;
				pitch = 0.0;
				}

			/* insert navigation */
			if (navlon < -180.0)
				navlon += 360.0;
			else if (navlon > 180.0)
				navlon -= 360.0;
			ping->png_longitude = 10000000 * navlon;
			ping->png_latitude = 20000000 * navlat;

			/* insert heading */
			ping->png_heading = (int) rint(heading * 100);

			/* insert roll pitch and heave */
			ping->png_roll = (int) rint(roll / 0.01);
			ping->png_pitch = (int) rint(pitch / 0.01);
			ping->png_heave = (int) rint(heave / 0.01);

			/* output asynchronous sonardepth (though really synchronous for Kongsberg data) */
			fprintf(atsfp, "%0.6f\t%0.3f\n", time_d, ping->png_xducer_depth);

			/* output synchronous attitude */
			fprintf(stafp, "%0.6f\t%0.3f\t%0.3f\n",time_d, roll, pitch);

			/* recalculate beam angles if desired */
			if (recalculate_beam_angles == MB_YES && imb_io_ptr->saveptr1 != NULL)
				{
				/* estimate effective heave using sonar parameters this ought to work but isn't quite right */
				heave_ping = 0.5 * (istore->par_s1z + istore->par_s2z) - istore->par_wlz - ping->png_xducer_depth;

				/* make first cut at angles */
				/* calculate corrected ranges, angles, and bathymetry */
				theta_nadir = 90.0;
				inadir = 0;
				for (i=0;i<ping->png_nbeams;i++)
					{
					/* get attitude and heave at ping and receive time */
					transmit_time_d = ptime_d + (double) ping->png_raw_txoffset[ping->png_raw_rxsector[i]];
					mb_hedint_interp(verbose, imbio_ptr, transmit_time_d,
								&transmit_heading, &error);
					mb_attint_interp(verbose, imbio_ptr, transmit_time_d,
								&transmit_heave, &transmit_roll, &transmit_pitch, &error);
					receive_time_d = transmit_time_d + ping->png_raw_rxrange[i];
					mb_hedint_interp(verbose, imbio_ptr, receive_time_d,
								&receive_heading, &error);
					mb_attint_interp(verbose, imbio_ptr, receive_time_d,
								&receive_heave, &receive_roll, &receive_pitch, &error);

					/* alongtrack offset distance */
					transmit_alongtrack = (0.01 * ((double)ping->png_speed))
								* ((double) ping->png_raw_txoffset[ping->png_raw_rxsector[i]]);

					/* get corrected range */
					if (ping->png_ssv <= 0)
						ping->png_ssv = 150;
					soundspeed = 0.1 * ((double)ping->png_ssv);
					ping->png_range[i] = ping->png_raw_rxrange[i];
					heave_beam = 0.5 * (transmit_heave + receive_heave);
					ping->png_bheave[i] = receive_heave - transmit_heave;
					depth_offset_use = ping->png_xducer_depth - ping->png_bheave[i];

					/* calculate angles */
					alpha = (0.01 * (double)ping->png_raw_txtiltangle[ping->png_raw_rxsector[i]]) - transmit_pitch + istore->par_msp;
					beta = 90.0 - ((0.01 * (double)ping->png_raw_rxpointangle[i]) + receive_roll - istore->par_msr);
					mb_rollpitch_to_takeoff(
						verbose,
						alpha, beta,
						&theta, &phi,
						&error);

					/* apply yaw correction by rotating the azimuthal angle to reflect the difference between
						the ping heading and the heading at sector transmit time */
					phi -= transmit_heading - pheading;
					if (phi > 180.0) phi -= 360.0;
					if (phi < -180.0) phi += 360.0;

					/* get takeoff angles */
					ping->png_depression[i] = theta;
					ping->png_azimuth[i] = phi;

					/* check for most nadir beam */
					if (ping->png_clean[i] == 0 && theta < theta_nadir)
						{
						theta_nadir = theta;
						inadir = i;
						}
					}

				/* Unfortunately, the above code is not succeeding in calculating angles that, after
					raytracing, replicate the sounding positions reported by the sonar. I am
					probably missing some aspect of the calculation of attitude compensation, or
					I've just got something wrong.
					To get bathymetry recalculation close to right, I will estimate the azimuthal
					angle phi using the originally reported beam positions. I will then estimate
					the takeoff angle theta by the following three steps:
						1. Iteratively raytrace the most vertical beam to find the angle
							reproducing the original position. Then take the difference
							between the calculated and originally reported depth and
							treat that as an effective heave offset that is added to all beams.
						2. For each beam iteratively raytrace to match the originally reported position.
							Also iteratively raytrace to match the originally reported depth.
						3. Estimate takeoff angle for the beam as a weighted average of the position
							and depth matching takeoff angles, where the weighting for the position
							matching estimate is cos(theta)**2 and the weighting for the depth-matching
							estimate is (1 - cos(theta)**2). */

				/* estimate effective heave by raytracing the most-vertical beam and matching the position to within
					1 mm. Add the depth difference to all beams as an effective heave offset */
				heave_offset = 0.0;
				dt = 0.0;

				/* get attitude and heave at ping and receive time */
				transmit_time_d = ptime_d + (double) ping->png_raw_txoffset[ping->png_raw_rxsector[inadir]];
				mb_hedint_interp(verbose, imbio_ptr, transmit_time_d,
							&transmit_heading, &error);
				mb_attint_interp(verbose, imbio_ptr, transmit_time_d,
							&transmit_heave, &transmit_roll, &transmit_pitch, &error);
				receive_time_d = transmit_time_d + ping->png_raw_rxrange[inadir];
				mb_hedint_interp(verbose, imbio_ptr, receive_time_d,
							&receive_heading, &error);
				mb_attint_interp(verbose, imbio_ptr, receive_time_d,
							&receive_heave, &receive_roll, &receive_pitch, &error);

				/* get range */
				if (ping->png_ssv <= 0)
					ping->png_ssv = 150;
				soundspeed = 0.1 * ((double)ping->png_ssv);
				ping->png_range[inadir] = ping->png_raw_rxrange[inadir];
				heave_beam = 0.5 * (transmit_heave + receive_heave);
				ping->png_bheave[inadir] = receive_heave - transmit_heave;

				/* get depth_offset_use and static_shift for raytracing */
				if (istore->svp_num > 0)
					svpdepthstart = 0.01 * istore->svp_depth_res * istore->svp_depth[0];
				else
					svpdepthstart = 0.0;
				depth_offset_use = ping->png_xducer_depth - ping->png_bheave[inadir];
				if (depth_offset_use < svpdepthstart)
					static_shift = depth_offset_use - svpdepthstart;
				else
					static_shift = 0.0;

				/* calculate angles */
				alpha = (0.01 * (double)ping->png_raw_txtiltangle[ping->png_raw_rxsector[inadir]]) - transmit_pitch + istore->par_msp;
				beta = 90.0 - ((0.01 * (double)ping->png_raw_rxpointangle[inadir]) + receive_roll - istore->par_msr);
				mb_rollpitch_to_takeoff(
					verbose,
					alpha, beta,
					&theta, &phi,
					&error);

				/* obtain lever arm offset for sonar relative to the position sensor */
				mb_lever(verbose, istore->par_s1y, istore->par_s1x, istore->par_s1z - istore->par_wlz,
						istore->par_p1y, istore->par_p1x, istore->par_p1z,
						istore->par_msy, istore->par_msx, istore->par_msz,
						-transmit_pitch + istore->par_msp, -receive_roll + istore->par_msr,
						&lever_x, &lever_y, &lever_z, &error);

				/* obtain position offset for beam */
				offset_x = istore->par_s1y - istore->par_p1y + lever_x;
				offset_y = istore->par_s1x - istore->par_p1x + lever_y;
				offset_z =  receive_heave - transmit_heave + lever_z;

				/* apply yaw correction by rotating the azimuthal angle to reflect the difference between
					the ping heading and the heading at sector transmit time */
				phi -= transmit_heading - pheading;
				if (phi > 180.0) phi -= 360.0;
				if (phi < -180.0) phi += 360.0;

				/* alongtrack offset distance */
				transmit_alongtrack = (0.01 * ((double)ping->png_speed))
							* ((double) ping->png_raw_txoffset[ping->png_raw_rxsector[inadir]]);

				/* corrected lateral distance */
				xxx = ping->png_acrosstrack[inadir] - offset_x;
				yyy = ping->png_alongtrack[inadir] - offset_y - transmit_alongtrack;
				xx = sqrt(xxx * xxx + yyy * yyy);
				zz = ping->png_depth[inadir] + ping->png_xducer_depth;
				mb_xyz_to_takeoff(verbose,-xxx, yyy, ping->png_depth[inadir],
							&theta_bath,&phi_bath,&error);
				phi = phi_bath;

				/* find vertical takeoff angle that matches the position to within 1 mm */
				iterx = 0;
				iterz = 0;
				theta_x = theta;
				thetamin = 0.0;
				thetamax = 90.0;
				dtheta = 0.0;
				dx = zz;
				dz = zz;
				zzcalc = zz;
				zzcalc_old = 0.0;
				done = MB_NO;
				while (iterx < 3 || done == MB_NO)
					{
					theta_old = theta_x;
					xxcalc_old = xxcalc;
					zzcalc_old = zzcalc;
					if (theta_x + dtheta > thetamin && theta_x + dtheta < thetamax)
						theta_x += dtheta;
					else if (dtheta < 0.0)
						theta_x = theta_x - 0.5 * (theta_x - thetamin);
					else if (dtheta > 0.0)
						theta_x = theta_x + 0.5 * (thetamax - theta_x);
					tt = 0.5 * ping->png_range[inadir];

					mb_rt(verbose, (void *) imb_io_ptr->saveptr1,
						depth_offset_use,
						theta_x, tt,
						MBKONSBERGPREPROCESS_BATH_RECALC_ANGLEMODE, soundspeed, 0.0,
						0, NULL, NULL, NULL,
						&xxcalc, &zzcalc, &ttt, &ray_stat,&error);
					zzcalc += static_shift;
					dx = xx - xxcalc;
					dz = zz - zzcalc;
					if (xxcalc > xx)
						thetamax = MIN(thetamax, theta_x);
					if (xxcalc < xx)
						thetamin = MAX(thetamin, theta_x);
					if (iterx == 0)
						{
						if (xxcalc > xx)
							{
							dtheta = -0.01;
							thetamax = MIN(thetamax, theta_x);
							}
						else
							{
							dtheta = 0.01;
							thetamin = MAX(thetamin, theta_x);
							}
						}
					else if (fabs(dx) < MBKONSBERGPREPROCESS_BATH_RECALC_PRECISION)
						{
						dtheta = 0.0;
						done = MB_YES;
						}
					else if (fabs(xxcalc - xxcalc_old) < MBKONSBERGPREPROCESS_BATH_RECALC_PRECISION)
						{
						dtheta = 0.0;
						done = MB_YES;
						}
					else
						{
						dtheta = (xx - xxcalc) * (theta_x - theta_old) / (xxcalc - xxcalc_old);
						}

					iterx++;
					if (iterx >= MBKONSBERGPREPROCESS_BATH_RECALC_NCALCMAX)
						done = MB_YES;
					}

				/* set the heave offset that will be added to all beams */
				heave_offset = -dz;

				/* calculate ranges, angles, and bathymetry */
				for (i=0;i<ping->png_nbeams;i++)
					{
					/* only work on beams with good travel times */
					detection_mask = (mb_u_char) ping->png_raw_rxdetection[i];
					if (ping->png_range[i] > 0.0
						|| (((detection_mask & 128) == 128) && (((detection_mask & 32) == 32) || ((detection_mask & 24) == 24))))
						{
						/* get attitude and heave at ping and receive time */
						transmit_time_d = ptime_d + (double) ping->png_raw_txoffset[ping->png_raw_rxsector[i]];
						mb_hedint_interp(verbose, imbio_ptr, transmit_time_d,
									&transmit_heading, &error);
						mb_attint_interp(verbose, imbio_ptr, transmit_time_d,
									&transmit_heave, &transmit_roll, &transmit_pitch, &error);
						receive_time_d = transmit_time_d + ping->png_raw_rxrange[i];
						mb_hedint_interp(verbose, imbio_ptr, receive_time_d,
									&receive_heading, &error);
						mb_attint_interp(verbose, imbio_ptr, receive_time_d,
									&receive_heave, &receive_roll, &receive_pitch, &error);

						/* get range */
						if (ping->png_ssv <= 0)
							ping->png_ssv = 150;
						soundspeed = 0.1 * ((double)ping->png_ssv);
						ping->png_range[i] = ping->png_raw_rxrange[i];
						heave_beam = 0.5 * (transmit_heave + receive_heave);
						ping->png_bheave[i] = receive_heave - transmit_heave + heave_offset;
						depth_offset_use = ping->png_xducer_depth - ping->png_bheave[i];

						/* calculate angles */
						alpha = (0.01 * (double)ping->png_raw_txtiltangle[ping->png_raw_rxsector[i]]) - transmit_pitch + istore->par_msp;
						beta = 90.0 - ((0.01 * (double)ping->png_raw_rxpointangle[i]) + receive_roll - istore->par_msr);
						mb_rollpitch_to_takeoff(
							verbose,
							alpha, beta,
							&theta, &phi,
							&error);

						/* apply yaw correction by rotating the azimuthal angle to reflect the difference between
							the ping heading and the heading at sector transmit time */
						phi -= transmit_heading - pheading;
						if (phi > 180.0) phi -= 360.0;
						if (phi < -180.0) phi += 360.0;

						/* alongtrack offset distance */
						transmit_alongtrack = (0.01 * ((double)ping->png_speed))
									* ((double) ping->png_raw_txoffset[ping->png_raw_rxsector[i]]);

						/* corrected lateral distance */
						xxx = ping->png_acrosstrack[i] - offset_x;
						yyy = ping->png_alongtrack[i] - offset_y - transmit_alongtrack;
						xx = sqrt(xxx * xxx + yyy * yyy);
						zz = ping->png_depth[i] + ping->png_xducer_depth;
						mb_xyz_to_takeoff(verbose,-xxx, yyy, ping->png_depth[i],
									&theta_bath,&phi_bath,&error);
						phi = phi_bath;

						/* get depth_offset_use and static_shift for raytracing */
						if (istore->svp_num > 0)
							svpdepthstart = 0.01 * istore->svp_depth_res * istore->svp_depth[0];
						else
							svpdepthstart = 0.0;
						depth_offset_use = ping->png_xducer_depth - ping->png_bheave[i];
						if (depth_offset_use < svpdepthstart)
							static_shift = depth_offset_use - svpdepthstart;
						else
							static_shift = 0.0;

						/* find vertical takeoff angle that matches the position to within 1 mm */
						iterx = 0;
						iterz = 0;
						theta_x = theta;
						thetamin = 0.0;
						thetamax = 90.0;
						dtheta = 0.0;
						dx = zz;
						dz = zz;
						zzcalc = zz;
						zzcalc_old = 0.0;
						done = MB_NO;
						while (iterx < 3 || done == MB_NO)
							{
							theta_old = theta_x;
							xxcalc_old = xxcalc;
							zzcalc_old = zzcalc;
							if (theta_x + dtheta > thetamin && theta_x + dtheta < thetamax)
								theta_x += dtheta;
							else if (dtheta < 0.0)
								theta_x = theta_x - 0.5 * (theta_x - thetamin);
							else if (dtheta > 0.0)
								theta_x = theta_x + 0.5 * (thetamax - theta_x);
							tt = 0.5 * ping->png_range[i];

							mb_rt(verbose, (void *) imb_io_ptr->saveptr1,
								(depth_offset_use - static_shift),
								theta_x, tt,
								MBKONSBERGPREPROCESS_BATH_RECALC_ANGLEMODE, soundspeed, 0.0,
								0, NULL, NULL, NULL,
								&xxcalc, &zzcalc, &ttt, &ray_stat,&error);
							zzcalc += static_shift;
							dx = xx - xxcalc;
							dz = zz - zzcalc;
							if (xxcalc > xx)
								thetamax = MIN(thetamax, theta_x);
							if (xxcalc < xx)
								thetamin = MAX(thetamin, theta_x);
							if (iterx == 0)
								{
								if (xxcalc > xx)
									{
									dtheta = -0.01;
									thetamax = MIN(thetamax, theta_x);
									}
								else
									{
									dtheta = 0.01;
									thetamin = MAX(thetamin, theta_x);
									}
								}
							else if (fabs(dx) < MBKONSBERGPREPROCESS_BATH_RECALC_PRECISION)
								{
								dtheta = 0.0;
								done = MB_YES;
								}
							else if (fabs(xxcalc - xxcalc_old) < MBKONSBERGPREPROCESS_BATH_RECALC_PRECISION)
								{
								dtheta = 0.0;
								done = MB_YES;
								}
							else
								{
								dtheta = (xx - xxcalc) * (theta_x - theta_old) / (xxcalc - xxcalc_old);
								}

							iterx++;
							if (iterx >= MBKONSBERGPREPROCESS_BATH_RECALC_NCALCMAX)
								done = MB_YES;
							}

						/* find vertical takeoff angle that matches the depth to within 1 mm */
						iterx = 0;
						iterz = 0;
						theta_z = theta_x;
						thetamin = 0.0;
						thetamax = 90.0;
						dtheta = 0.0;
						dx = zz;
						dz = zz;
						zzcalc = zz;
						zzcalc_old = 0.0;
						done = MB_NO;
						while (iterz < 3 || done == MB_NO)
							{
							theta_old = theta_z;
							xxcalc_old = xxcalc;
							zzcalc_old = zzcalc;
							if (theta_z + dtheta > thetamin && theta_z + dtheta < thetamax)
								theta_z += dtheta;
							else if (dtheta < 0.0)
								theta_z = theta_z - 0.5 * (theta_z - thetamin);
							else if (dtheta > 0.0)
								theta_z = theta_z + 0.5 * (thetamax - theta_z);
							tt = 0.5 * ping->png_range[i];

							mb_rt(verbose, (void *) imb_io_ptr->saveptr1,
								(depth_offset_use - static_shift),
								theta_z, tt,
								MBKONSBERGPREPROCESS_BATH_RECALC_ANGLEMODE, soundspeed, 0.0,
								0, NULL, NULL, NULL,
								&xxcalc, &zzcalc, &ttt, &ray_stat,&error);
							zzcalc += static_shift;
							dx = xx - xxcalc;
							dz = zz - zzcalc;
							if (zzcalc < zz)
								thetamax = MIN(thetamax, theta_z);
							if (zzcalc > zz)
								thetamin = MAX(thetamin, theta_z);
							if (iterz == 0)
								{
								if (zzcalc < zz)
									{
									dtheta = -0.01;
									thetamax = MIN(thetamax, theta_z);
									}
								else
									{
									dtheta = 0.01;
									thetamin = MAX(thetamin, theta_z);
									}
								}
							else if (fabs(dz) < MBKONSBERGPREPROCESS_BATH_RECALC_PRECISION)
								{
								dtheta = 0.0;
								done = MB_YES;
								}
							else if (fabs(zzcalc - zzcalc_old) < MBKONSBERGPREPROCESS_BATH_RECALC_PRECISION)
								{
								dtheta = 0.0;
								done = MB_YES;
								}
							else
								{
								dtheta = (zz - zzcalc) * (theta_z - theta_old) / (zzcalc - zzcalc_old);
								}

							iterz++;
							if (iterz >= MBKONSBERGPREPROCESS_BATH_RECALC_NCALCMAX)
								done = MB_YES;
							}

						weight = cos(DTR * theta) * cos (DTR * theta);
						theta_new = weight * theta_x + (1.0 - weight) * theta_z;

						ping->png_depression[i] = theta_new;
						ping->png_azimuth[i] = phi;
						ping->png_range[i] += dt;
						}

					/* handle beams with zero travel times */
					else
						{
						ping->png_beamflag[i] = MB_FLAG_NULL;
						ping->png_depression[i] = 0.0;
						ping->png_azimuth[i] = 0.0;
						ping->png_range[i] = 0.0;
						}
					}

				}
			}

	   	/* handle unknown data */
		else  if (status == MB_SUCCESS)
			{
/*fprintf(stderr,"DATA TYPE OTHER THAN SURVEY: status:%d error:%d kind:%d\n",status,error,kind);*/
			}

	   	/* handle read error */
		else
			{
/*fprintf(stderr,"READ FAILURE: status:%d error:%d kind:%d\n",status,error,kind);*/
			}

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",
				program_name);
			fprintf(stderr,"dbg2       kind:           %d\n",kind);
			fprintf(stderr,"dbg2       error:          %d\n",error);
			fprintf(stderr,"dbg2       status:         %d\n",status);
			}

	/*--------------------------------------------
	  write the processed data
	  --------------------------------------------*/

		/* write some data */
		if (error == MB_ERROR_NO_ERROR)
			{
			status = mb_put_all(verbose,ombio_ptr,
					istore_ptr,MB_NO,kind,
					time_i,time_d,
					navlon,navlat,speed,heading,
					obeams_bath,obeams_amp,opixels_ss,
					beamflag,bath,amp,bathacrosstrack,bathalongtrack,
					ss,ssacrosstrack,ssalongtrack,
					comment,&error);
			if (status != MB_SUCCESS)
				{
				mb_error(verbose,error,&message);
				fprintf(stderr,"\nMBIO Error returned from function <mb_put>:\n%s\n",message);
				fprintf(stderr,"\nMultibeam Data Not Written To File <%s>\n",ofile);
				fprintf(stderr,"\nProgram <%s> Terminated\n",
					program_name);
				exit(error);
				}
			}
		}

	/* output counts */
	if(output_counts == MB_YES)
		{
		fprintf(stdout, "\nData records written to: %s\n", ofile);
		fprintf(stdout, "     nrec_0x30_parameter_stop:         %d\n", nrec_0x30_parameter_stop);
		fprintf(stdout, "     nrec_0x31_parameter_off:          %d\n", nrec_0x31_parameter_off);
		fprintf(stdout, "     nrec_0x32_parameter_on:           %d\n", nrec_0x32_parameter_on);
		fprintf(stdout, "     nrec_0x33_parameter_extra:        %d\n", nrec_0x33_parameter_extra);
		fprintf(stdout, "     nrec_0x41_attitude:               %d\n", nrec_0x41_attitude);
		fprintf(stdout, "     nrec_0x43_clock:                  %d\n", nrec_0x43_clock);
		fprintf(stdout, "     nrec_0x44_bathymetry:             %d\n", nrec_0x44_bathymetry);
		fprintf(stdout, "     nrec_0x45_singlebeam:             %d\n", nrec_0x45_singlebeam);
		fprintf(stdout, "     nrec_0x46_rawbeamF:               %d\n", nrec_0x46_rawbeamF);
		fprintf(stdout, "     nrec_0x47_surfacesoundspeed2:     %d\n", nrec_0x47_surfacesoundspeed2);
		fprintf(stdout, "     nrec_0x48_heading:                %d\n", nrec_0x48_heading);
		fprintf(stdout, "     nrec_0x49_parameter_start:        %d\n", nrec_0x49_parameter_start);
		fprintf(stdout, "     nrec_0x4A_tilt:                   %d\n", nrec_0x4A_tilt);
		fprintf(stdout, "     nrec_0x4B_echogram:               %d\n", nrec_0x4B_echogram);
		fprintf(stdout, "     nrec_0x4E_rawbeamN:               %d\n", nrec_0x4E_rawbeamN);
		fprintf(stdout, "     nrec_0x50_pos:                    %d\n", nrec_0x50_pos);
		fprintf(stdout, "     nrec_0x52_runtime:                %d\n", nrec_0x52_runtime);
		fprintf(stdout, "     nrec_0x53_sidescan:               %d\n", nrec_0x53_sidescan);
		fprintf(stdout, "     nrec_0x54_tide:                   %d\n", nrec_0x54_tide);
		fprintf(stdout, "     nrec_0x55_svp2:                   %d\n", nrec_0x55_svp2);
		fprintf(stdout, "     nrec_0x56_svp:                    %d\n", nrec_0x56_svp);
		fprintf(stdout, "     nrec_0x57_surfacesoundspeed:      %d\n", nrec_0x57_surfacesoundspeed);
		fprintf(stdout, "     nrec_0x58_bathymetry2:            %d\n", nrec_0x58_bathymetry2);
		fprintf(stdout, "     nrec_0x59_sidescan2:              %d\n", nrec_0x59_sidescan2);
		fprintf(stdout, "     nrec_0x66_rawbeamf:               %d\n", nrec_0x66_rawbeamf);
		fprintf(stdout, "     nrec_0x68_height:                 %d\n", nrec_0x68_height);
		fprintf(stdout, "     nrec_0x69_parameter_stop:         %d\n", nrec_0x69_parameter_stop);
		fprintf(stdout, "     nrec_0x6B_water_column:           %d\n", nrec_0x6B_water_column);
		fprintf(stdout, "     nrec_0x6E_network_attitude:       %d\n", nrec_0x6E_network_attitude);
		fprintf(stdout, "     nrec_0x70_parameter:              %d\n", nrec_0x70_parameter);
		fprintf(stdout, "     nrec_0x73_surface_sound_speed:    %d\n", nrec_0x73_surface_sound_speed);
		fprintf(stdout, "     nrec_0xE1_bathymetry_mbari57:     %d\n", nrec_0xE1_bathymetry_mbari57);
		fprintf(stdout, "     nrec_0xE2_sidescan_mbari57:       %d\n", nrec_0xE2_sidescan_mbari57);
		fprintf(stdout, "     nrec_0xE3_bathymetry_mbari59:     %d\n", nrec_0xE3_bathymetry_mbari59);
		fprintf(stdout, "     nrec_0xE4_sidescan_mbari59:       %d\n", nrec_0xE4_sidescan_mbari59);
		fprintf(stdout, "     nrec_0xE5_bathymetry_mbari59:     %d\n", nrec_0xE5_bathymetry_mbari59);
		}

	nrec_0x30_parameter_stop_tot += nrec_0x30_parameter_stop;
	nrec_0x31_parameter_off_tot += nrec_0x31_parameter_off;
	nrec_0x32_parameter_on_tot += nrec_0x32_parameter_on;
	nrec_0x33_parameter_extra_tot += nrec_0x33_parameter_extra;
	nrec_0x41_attitude_tot += nrec_0x41_attitude;
	nrec_0x43_clock_tot += nrec_0x43_clock;
	nrec_0x44_bathymetry_tot += nrec_0x44_bathymetry;
	nrec_0x45_singlebeam_tot += nrec_0x45_singlebeam;
	nrec_0x46_rawbeamF_tot += nrec_0x46_rawbeamF;
	nrec_0x47_surfacesoundspeed2_tot += nrec_0x47_surfacesoundspeed2;
	nrec_0x48_heading_tot += nrec_0x48_heading;
	nrec_0x49_parameter_start_tot += nrec_0x49_parameter_start;
	nrec_0x4A_tilt_tot += nrec_0x4A_tilt;
	nrec_0x4B_echogram_tot += nrec_0x4B_echogram;
	nrec_0x4E_rawbeamN_tot += nrec_0x4E_rawbeamN;
	nrec_0x50_pos_tot += nrec_0x50_pos;
	nrec_0x52_runtime_tot += nrec_0x52_runtime;
	nrec_0x53_sidescan_tot += nrec_0x53_sidescan;
	nrec_0x54_tide_tot += nrec_0x54_tide;
	nrec_0x55_svp2_tot += nrec_0x55_svp2;
	nrec_0x56_svp_tot += nrec_0x56_svp;
	nrec_0x57_surfacesoundspeed_tot += nrec_0x57_surfacesoundspeed;
	nrec_0x58_bathymetry2_tot += nrec_0x58_bathymetry2;
	nrec_0x59_sidescan2_tot += nrec_0x59_sidescan2;
	nrec_0x66_rawbeamf_tot += nrec_0x66_rawbeamf;
	nrec_0x68_height_tot += nrec_0x68_height;
	nrec_0x69_parameter_stop_tot += nrec_0x69_parameter_stop;
	nrec_0x6B_water_column_tot += nrec_0x6B_water_column;
	nrec_0x6E_network_attitude_tot += nrec_0x6E_network_attitude;
	nrec_0x70_parameter_tot += nrec_0x70_parameter;
	nrec_0x73_surface_sound_speed_tot += nrec_0x73_surface_sound_speed;
	nrec_0xE1_bathymetry_mbari57_tot += nrec_0xE1_bathymetry_mbari57;
	nrec_0xE2_sidescan_mbari57_tot += nrec_0xE2_sidescan_mbari57;
	nrec_0xE3_bathymetry_mbari59_tot += nrec_0xE3_bathymetry_mbari59;
	nrec_0xE4_sidescan_mbari59_tot += nrec_0xE4_sidescan_mbari59;
	nrec_0xE5_bathymetry_mbari59_tot += nrec_0xE5_bathymetry_mbari59;

	/* figure out whether and what to read next */
        if (read_datalist == MB_YES)
                {
		if ((status = mb_datalist_read(verbose,datalist,
			    ifile,&format,&file_weight,&error))
			    == MB_SUCCESS)
                        read_data = MB_YES;
                else
                        read_data = MB_NO;
                }
        else
                {
                read_data = MB_NO;
                }

	/* close the input swath file */
	status = mb_close(verbose,&imbio_ptr,&error);

	/* close the output swath file if necessary */
	if (ofile_set == MB_NO || read_data == MB_NO)
		{
		status = mb_close(verbose,&ombio_ptr,&error);
		fclose(tfp);

		/* open up start and end times by two minutes */
		start_time_d -= 120.0;
		end_time_d += 120.0;

		/* output asynchronous heading output file */
		sprintf(athfile,"%s.ath",ofile);
		if ((athfp = fopen(athfile, "w")) != NULL)
			{
			for (i=0;i<ndat_heading;i++)
				{
				if (dat_heading_time_d[i] > start_time_d && dat_heading_time_d[i] < end_time_d)
					fprintf(athfp, "%0.6f\t%7.3f\n",dat_heading_time_d[i], dat_heading_heading[i]);
				}
			fclose(athfp);
			}
		else
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open asynchronous heading data file <%s> for writing\n",athfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		/* output asynchronous attitude output file */
		sprintf(atafile,"%s.ata",ofile);
		if ((atafp = fopen(atafile, "w")) != NULL)
			{
			for (i=0;i<ndat_rph;i++)
				{
				if (dat_rph_time_d[i] > start_time_d && dat_rph_time_d[i] < end_time_d)
					fprintf(atafp, "%0.6f\t%0.3f\t%0.3f\n",dat_rph_time_d[i], dat_rph_roll[i], dat_rph_pitch[i]);
				}
			fclose(atafp);
			}
		else
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open asynchronous attitude data file <%s> for writing\n",atafile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		/* close ats and sta files */
		fclose(atsfp);
		fclose(stafp);

		/* generate inf fnv and fbt files */
		if (status == MB_SUCCESS)
			{
			status = mb_make_info(verbose, MB_YES, ofile, MBF_EM710MBA, &error);
			}
		}

	/* end loop over files in list */
	}
	if (read_datalist == MB_YES)
		mb_datalist_close(verbose,&datalist,&error);

	/* output counts */
	if(output_counts == MB_YES)
		{
		fprintf(stdout, "\nTotal files read:  %d\n", nfile_read);
		fprintf(stdout, "Total files written: %d\n", nfile_write);
		fprintf(stdout, "\nTotal data records written from: %s\n", read_file);
		fprintf(stdout, "     nrec_0x30_parameter_stop_tot:     %d\n", nrec_0x30_parameter_stop_tot);
		fprintf(stdout, "     nrec_0x31_parameter_off_tot:      %d\n", nrec_0x31_parameter_off_tot);
		fprintf(stdout, "     nrec_0x32_parameter_on_tot:       %d\n", nrec_0x32_parameter_on_tot);
		fprintf(stdout, "     nrec_0x33_parameter_extra_tot:    %d\n", nrec_0x33_parameter_extra_tot);
		fprintf(stdout, "     nrec_0x41_attitude_tot:           %d\n", nrec_0x41_attitude_tot);
		fprintf(stdout, "     nrec_0x43_clock_tot:              %d\n", nrec_0x43_clock_tot);
		fprintf(stdout, "     nrec_0x44_bathymetry_tot:         %d\n", nrec_0x44_bathymetry_tot);
		fprintf(stdout, "     nrec_0x45_singlebeam_tot:         %d\n", nrec_0x45_singlebeam_tot);
		fprintf(stdout, "     nrec_0x46_rawbeamF_tot:           %d\n", nrec_0x46_rawbeamF_tot);
		fprintf(stdout, "     nrec_0x47_surfacesoundspeed2_tot: %d\n", nrec_0x47_surfacesoundspeed2_tot);
		fprintf(stdout, "     nrec_0x48_heading_tot:            %d\n", nrec_0x48_heading_tot);
		fprintf(stdout, "     nrec_0x49_parameter_start_tot:    %d\n", nrec_0x49_parameter_start_tot);
		fprintf(stdout, "     nrec_0x4A_tilt_tot:               %d\n", nrec_0x4A_tilt_tot);
		fprintf(stdout, "     nrec_0x4B_echogram_tot:           %d\n", nrec_0x4B_echogram_tot);
		fprintf(stdout, "     nrec_0x4E_rawbeamN_tot:           %d\n", nrec_0x4E_rawbeamN_tot);
		fprintf(stdout, "     nrec_0x50_pos_tot:                %d\n", nrec_0x50_pos_tot);
		fprintf(stdout, "     nrec_0x52_runtime_tot:            %d\n", nrec_0x52_runtime_tot);
		fprintf(stdout, "     nrec_0x53_sidescan_tot:           %d\n", nrec_0x53_sidescan_tot);
		fprintf(stdout, "     nrec_0x54_tide_tot:               %d\n", nrec_0x54_tide_tot);
		fprintf(stdout, "     nrec_0x55_svp2_tot:               %d\n", nrec_0x55_svp2_tot);
		fprintf(stdout, "     nrec_0x56_svp_tot:                %d\n", nrec_0x56_svp_tot);
		fprintf(stdout, "     nrec_0x57_surfacesoundspeed_tot:  %d\n", nrec_0x57_surfacesoundspeed_tot);
		fprintf(stdout, "     nrec_0x58_bathymetry2_tot:        %d\n", nrec_0x58_bathymetry2_tot);
		fprintf(stdout, "     nrec_0x59_sidescan2_tot:          %d\n", nrec_0x59_sidescan2_tot);
		fprintf(stdout, "     nrec_0x66_rawbeamf_tot:           %d\n", nrec_0x66_rawbeamf_tot);
		fprintf(stdout, "     nrec_0x68_height_tot:             %d\n", nrec_0x68_height_tot);
		fprintf(stdout, "     nrec_0x69_parameter_stop_tot:     %d\n", nrec_0x69_parameter_stop_tot);
		fprintf(stdout, "     nrec_0x6B_water_column_tot:       %d\n", nrec_0x6B_water_column_tot);
		fprintf(stdout, "     nrec_0x6E_network_attitude_tot:   %d\n", nrec_0x6E_network_attitude_tot);
		fprintf(stdout, "     nrec_0x70_parameter_tot:          %d\n", nrec_0x70_parameter_tot);
		fprintf(stdout, "     nrec_0x73_surface_sound_speed_tot:%d\n", nrec_0x73_surface_sound_speed_tot);
		fprintf(stdout, "     nrec_0xE1_bathymetry_mbari57_tot: %d\n", nrec_0xE1_bathymetry_mbari57_tot);
		fprintf(stdout, "     nrec_0xE2_sidescan_mbari57_tot:   %d\n", nrec_0xE2_sidescan_mbari57_tot);
		fprintf(stdout, "     nrec_0xE3_bathymetry_mbari59_tot: %d\n", nrec_0xE3_bathymetry_mbari59_tot);
		fprintf(stdout, "     nrec_0xE4_sidescan_mbari59_tot:   %d\n", nrec_0xE4_sidescan_mbari59_tot);
		fprintf(stdout, "     nrec_0xE5_bathymetry_mbari59_tot: %d\n", nrec_0xE5_bathymetry_mbari59_tot);
		}
	}

	/* deallocate navigation arrays */
	if (ndat_nav > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_nav_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_nav_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_nav_lon,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_nav_lat,&error);
		}
	if (ndat_heading > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_heading_heading,&error);
		}
	if (ndat_rph > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_rph_roll,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_rph_pitch,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_rph_heave,&error);
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

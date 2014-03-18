/*--------------------------------------------------------------------
 *    The MB-system:	mbhysweeppreprocess.c	1/1/2012
 *    $Id$
 *
 *    Copyright (c) 2013-2014 by
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
 * mbhysweeppreprocess reads a HYSWEEP HSX format file, interpolates the
 * asynchronous navigation and attitude onto the multibeam data,
 * and writes a new HSX file with that information correctly embedded
 * in the multibeam data. This program can also fix various problems
 * with the data, including sensor offsets.
 *
 * Author:	D. W. Caress
 * Date:	January 1, 2012
 *
 * $Log: mbhysweeppreprocess.c,v $
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
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_io.h"
#include "mb_aux.h"
#include "mbsys_hysweep.h"

#define MBHYSWEEPPREPROCESS_ALLOC_CHUNK 1000
#define MBHYSWEEPPREPROCESS_PROCESS		1
#define MBHYSWEEPPREPROCESS_TIMESTAMPLIST	2
#define	MBHYSWEEPPREPROCESS_TIMELAG_OFF	0
#define	MBHYSWEEPPREPROCESS_TIMELAG_CONSTANT	1
#define	MBHYSWEEPPREPROCESS_TIMELAG_MODEL	2

#define MBHYSWEEPPREPROCESS_SONAR_OFFSET_NONE		0
#define MBHYSWEEPPREPROCESS_SONAR_OFFSET_SONAR		1
#define MBHYSWEEPPREPROCESS_SONAR_OFFSET_MRU		2
#define MBHYSWEEPPREPROCESS_SONAR_OFFSET_NAVIGATION	3

#define MBHYSWEEPPREPROCESS_OFFSET_MAX	12

#define MBHYSWEEPPREPROCESS_NAVFORMAT_NONE	0
#define MBHYSWEEPPREPROCESS_NAVFORMAT_OFG	1

static char rcs_id[] = "$Id$";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char program_name[] = "mbhysweeppreprocess";
	char help_message[] =  "mbhysweeppreprocess reads a Hysweep HSX format file, interpolates the\nasynchronous navigation and attitude onto the multibeam data, \nand writes a new HSX file with that information correctly embedded\nin the multibeam data.";
	char usage_message[] = "mbhysweeppreprocess [-Aoffsettype/x/y/z/t -Brollbias/pitchbias/headingbias -Dsonardepthfile -Idatalist -Jprojection -L -Mnavformat -Nnavfile -Ttimelag -H -V]";
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
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	int	obeams_bath;
	int	obeams_amp;
	int	opixels_ss;

	/* MBIO read values */
	void	*imbio_ptr = NULL;
	struct mb_io_struct *imb_io_ptr = NULL;
	void	*istore_ptr = NULL;
	struct mbsys_hysweep_struct *istore = NULL;
	struct mbsys_hysweep_device_struct *device;
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
	int	mode = MBHYSWEEPPREPROCESS_PROCESS;

	/* counting variables */
	int	nrec_POS = 0;
	int	nrec_POSunused = 0;
	int	nrec_GYR = 0;
	int	nrec_HCP = 0;
	int	nrec_EC1 = 0;
	int	nrec_DFT = 0;
	int	nrec_RMB = 0;
	int	nrec_other = 0;
	int	nrec_POS_tot = 0;
	int	nrec_POSunused_tot = 0;
	int	nrec_GYR_tot = 0;
	int	nrec_HCP_tot = 0;
	int	nrec_EC1_tot = 0;
	int	nrec_DFT_tot = 0;
	int	nrec_RMB_tot = 0;
	int	nrec_other_tot = 0;

	/* projection */
	int	projection_set = MB_NO;
	mb_path	proj4command;
	void	*pjptr = NULL;

	/* merge navigation data file */
	char	navfile[MB_PATH_MAXLINE];
	int	navdata = MB_NO;
	int	navformat = MBHYSWEEPPREPROCESS_NAVFORMAT_OFG;
	int	nnav = 0;
	double	*nav_time_d = NULL;
	double	*nav_lon = NULL;
	double	*nav_lat = NULL;
	double	*nav_heading = NULL;
	double	*nav_sonardepth = NULL;
	double	*nav_altitude = NULL;

	/* merge sonardepth from separate data file */
	char	sonardepthfile[MB_PATH_MAXLINE];
	int	sonardepthdata = MB_NO;
	int	nsonardepth = 0;
	double	*sonardepth_time_d = NULL;
	double	*sonardepth_sonardepth = NULL;

	/* asynchronous navigation, heading, attitude data */
	int	ndat_nav = 0;
	int	ndat_nav_alloc = 0;
	double	*dat_nav_time_d = NULL;
	double	*dat_nav_lon = NULL;
	double	*dat_nav_lat = NULL;

	int	ndat_sonardepth = 0;
	int	ndat_sonardepth_alloc = 0;
	double	*dat_sonardepth_time_d = NULL;
	double	*dat_sonardepth_sonardepth = NULL;

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

	int	ndat_altitude = 0;
	int	ndat_altitude_alloc = 0;
	double	*dat_altitude_time_d = NULL;
	double	*dat_altitude_altitude = NULL;

	/* timelag parameters */
	int	timelagmode = MBHYSWEEPPREPROCESS_TIMELAG_OFF;
	double	timelag = 0.0;
	double	timelagm = 0.0;
	double	timelagconstant = 0.0;
	char	timelagfile[MB_PATH_MAXLINE];
	int	ntimelag = 0;
	double	*timelag_time_d = NULL;
	double	*timelag_model = NULL;

	/* sensor offset parameters */
	double	offset_sonar_roll = 0.0;
	double	offset_sonar_pitch = 0.0;
	double	offset_sonar_heading = 0.0;
	double	offset_sonar_x = 0.0;
	double	offset_sonar_y = 0.0;
	double	offset_sonar_z = 0.0;
	double	offset_sonar_t = 0.0;
	double	offset_mru_x = 0.0;
	double	offset_mru_y = 0.0;
	double	offset_mru_z = 0.0;
	double	offset_mru_t = 0.0;
	double	offset_nav_x = 0.0;
	double	offset_nav_y = 0.0;
	double	offset_nav_z = 0.0;
	double	offset_nav_t = 0.0;

	/* processing kluge modes */
	int	klugemode;

	int	interp_status;
	double	alpha, beta, theta, phi;
	double	rr, xx, zz;

	FILE	*tfp = NULL;
	struct stat file_status;
	int	fstat;
	char	buffer[MB_PATH_MAXLINE];
	char	*result;
	int	read_data;
	char	fileroot[MB_PATH_MAXLINE];
	int	nscan;
	int	year, month, day, hour, minute;
	double	second, yearsecond;
	double	easting, northing;
	int	testformat;
	int	type;
	double	offset_roll, offset_pitch, offset_heading;
	double	offset_x, offset_y, offset_z, offset_t;
	double	lever_x, lever_y, lever_z;
	int	i, j;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to datalist.mb-1 */
	strcpy (read_file, "datalist.mb-1");

	/* process argument list */
	while ((c = getopt(argc, argv, "A:a:B:b:D:d:F:f:I:i:J:j:K:k:LlM:m:N:n:O:o:T:t:VvHh")) != -1)
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
			nscan = sscanf (optarg,"%d/%lf/%lf/%lf/%lf",
					&type, &offset_x, &offset_y, &offset_z, &offset_t);
			if (nscan == 5)
				{
				if (type == MBHYSWEEPPREPROCESS_SONAR_OFFSET_SONAR)
					{
					offset_sonar_x = offset_x;
					offset_sonar_y = offset_y;
					offset_sonar_z = offset_z;
					offset_sonar_t = offset_t;
					}
				else if (type == MBHYSWEEPPREPROCESS_SONAR_OFFSET_MRU)
					{
					offset_mru_x = offset_x;
					offset_mru_y = offset_y;
					offset_mru_z = offset_z;
					offset_mru_t = offset_t;
					}
				else if (type == MBHYSWEEPPREPROCESS_SONAR_OFFSET_NAVIGATION)
					{
					offset_nav_x = offset_x;
					offset_nav_y = offset_y;
					offset_nav_z = offset_z;
					offset_nav_t = offset_t;
					}
				}
			flag++;
			break;
		case 'B':
		case 'b':
			nscan = sscanf (optarg,"%lf/%lf/%lf",
					&offset_roll, &offset_pitch, &offset_heading);
			if (nscan == 3)
				{
				offset_sonar_roll = offset_roll;
				offset_sonar_pitch = offset_pitch;
				offset_sonar_heading = offset_heading;
				}
			flag++;
			break;
		case 'D':
		case 'd':
			sscanf (optarg,"%s", buffer);
			if ((fstat = stat(buffer, &file_status)) == 0
				&& (file_status.st_mode & S_IFMT) != S_IFDIR)
				{
				sonardepthdata  = MB_YES;
				strcpy(sonardepthfile,buffer);
				}
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
		case 'J':
		case 'j':
			sscanf (optarg,"%s", proj4command);
			projection_set = MB_YES;
			flag++;
			break;
		case 'K':
		case 'k':
			sscanf (optarg,"%d", &klugemode);
			flag++;
			break;
		case 'L':
		case 'l':
			mode = MBHYSWEEPPREPROCESS_TIMESTAMPLIST;
			flag++;
			break;
		case 'M':
		case 'm':
			sscanf (optarg,"%d", &navformat);
			flag++;
			break;
		case 'N':
		case 'n':
			sscanf (optarg,"%s", buffer);
			if ((fstat = stat(buffer, &file_status)) == 0
				&& (file_status.st_mode & S_IFMT) != S_IFDIR)
				{
				navdata  = MB_YES;
				strcpy(navfile,buffer);
				}
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", ofile);
			ofile_set  = MB_YES;
			flag++;
			break;
		case 'T':
		case 't':
			sscanf (optarg,"%s", timelagfile);
			if ((fstat = stat(timelagfile, &file_status)) == 0
				&& (file_status.st_mode & S_IFMT) != S_IFDIR)
					{
					timelagmode = MBHYSWEEPPREPROCESS_TIMELAG_MODEL;
					}
			else
				{
				sscanf (optarg,"%lf", &timelagconstant);
				timelagmode = MBHYSWEEPPREPROCESS_TIMELAG_CONSTANT;
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
		fprintf(stderr,"dbg2       projection_set:      %d\n",projection_set);
		fprintf(stderr,"dbg2       proj4command:        %s\n",proj4command);
		fprintf(stderr,"dbg2       navfile:             %s\n",navfile);
		fprintf(stderr,"dbg2       navdata:             %d\n",navdata);
		fprintf(stderr,"dbg2       sonardepthfile:      %s\n",sonardepthfile);
		fprintf(stderr,"dbg2       sonardepthdata:      %d\n",sonardepthdata);
		fprintf(stderr,"dbg2       timelagmode:         %d\n",timelagmode);
		if (timelagmode == MBHYSWEEPPREPROCESS_TIMELAG_MODEL)
			{
			fprintf(stderr,"dbg2       timelagfile:         %s\n",timelagfile);
			fprintf(stderr,"dbg2       ntimelag:            %d\n",ntimelag);
			for (i=0;i<ntimelag;i++)
				fprintf(stderr,"dbg2       timelag[%d]:         %f %f\n",
					i, timelag_time_d[i], timelag_model[i]);
			}
		else
			{
			fprintf(stderr,"dbg2       timelag:             %f\n",timelag);
			}
		fprintf(stderr,"dbg2       offset_sonar_roll:   %f\n",offset_sonar_roll);
		fprintf(stderr,"dbg2       offset_sonar_pitch:  %f\n",offset_sonar_pitch);
		fprintf(stderr,"dbg2       offset_sonar_heading:%f\n",offset_sonar_heading);
		fprintf(stderr,"dbg2       offset_sonar_x:      %f\n",offset_sonar_x);
		fprintf(stderr,"dbg2       offset_sonar_y:      %f\n",offset_sonar_y);
		fprintf(stderr,"dbg2       offset_sonar_z:      %f\n",offset_sonar_z);
		fprintf(stderr,"dbg2       offset_sonar_t:      %f\n",offset_sonar_t);
		fprintf(stderr,"dbg2       offset_mru_x:        %f\n",offset_mru_x);
		fprintf(stderr,"dbg2       offset_mru_y:        %f\n",offset_mru_y);
		fprintf(stderr,"dbg2       offset_mru_z:        %f\n",offset_mru_z);
		fprintf(stderr,"dbg2       offset_mru_t:        %f\n",offset_mru_t);
		fprintf(stderr,"dbg2       offset_nav_x:        %f\n",offset_nav_x);
		fprintf(stderr,"dbg2       offset_nav_y:        %f\n",offset_nav_y);
		fprintf(stderr,"dbg2       offset_nav_z:        %f\n",offset_nav_z);
		fprintf(stderr,"dbg2       offset_nav_t:        %f\n",offset_nav_t);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* set projection for nav data */
	if (projection_set == MB_YES)
		{
		mb_proj_init(verbose, proj4command, &pjptr, &error);
		}

	/* read navigation data from file if specified */
	if (navdata == MB_YES)
		{
		/* count the data points in the nav file */
		if ((tfp = fopen(navfile, "r")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open nav data file <%s> for reading\n",navfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		/* count the data records
			then rewind the file to the start */
		nnav = 0;
		while (fgets(buffer, MB_PATH_MAXLINE, tfp) != NULL)
			{
			nnav++;
			}
		rewind(tfp);

		/* allocate arrays for ins data */
		if (nnav > 0)
		    {
		    status = mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&nav_time_d,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&nav_lon,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&nav_lat,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&nav_heading,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&nav_sonardepth,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&nav_altitude,&error);
		    if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating nav data arrays:\n%s\n",message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		    }

		/* if no nav data then quit */
		else
		    {
		    error = MB_ERROR_BAD_DATA;
		    fprintf(stderr,"\nUnable to read data from nav file <%s>\n",navfile);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }

		/* read the data points in the nav file */
		nnav = 0;
		while (fgets(buffer, MB_PATH_MAXLINE, tfp) != NULL)
			{
			if (buffer[0] != '#' && buffer[0] != 'O')
				{
				nscan = sscanf(buffer, "%d:%d:%d:%d:%d:%lf %lf %lf %lf %lf %lf %lf",
						&year, &month, &day, &hour, &minute, &second, &yearsecond,
						&northing, &easting, &sonardepth, &altitude, &heading);
				if (nscan == 12)
					{
					time_i[0] = year;
					time_i[1] = month;
					time_i[2] = day;
					time_i[3] = hour;
					time_i[4] = minute;
					time_i[5] = (int)floor(second);
					time_i[6] = (int)((second - time_i[5]) * 1000000.0);
					mb_get_time(verbose, time_i, &time_d);

					if (projection_set == MB_YES)
						{
						mb_proj_inverse(verbose, pjptr, easting, northing,
									&navlon, &navlat, &error);
						}
					else
						{
						navlon = easting;
						navlat = northing;
						}

					nav_time_d[nnav] = time_d;
					nav_lon[nnav] = navlon;
					nav_lat[nnav] = navlat;
					nav_heading[nnav] = heading;
					nav_sonardepth[nnav] = sonardepth;
					nav_altitude[nnav] = altitude;
					nnav++;
					}
				}
			}
		fclose(tfp);
		}

	/* set projection for nav data */
	if (projection_set == MB_YES)
		{
		mb_proj_free(verbose, &pjptr, &error);
		}

	/* read sonardepth data from separate file if specified */
	if (sonardepthdata == MB_YES)
		{
		/* count the data points in the sonardepth file */
		if ((tfp = fopen(sonardepthfile, "r")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open sonardepth data file <%s> for reading\n",sonardepthfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		/* count the data records then rewind the file to the start of the binary data */
		nsonardepth = 0;
		while (fgets(buffer, MB_PATH_MAXLINE, tfp) != NULL)
			{
			nsonardepth++;
			}
		rewind(tfp);

		/* allocate arrays for sonardepth data */
		if (nsonardepth > 0)
		    {
		    status = mb_mallocd(verbose, __FILE__, __LINE__, nsonardepth * sizeof(double), (void **)&sonardepth_time_d,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nsonardepth * sizeof(double), (void **)&sonardepth_sonardepth,&error);
		    if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating sonardepth data arrays:\n%s\n",message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		    }

		/* if no sonardepth data then quit */
		else
		    {
		    error = MB_ERROR_BAD_DATA;
		    fprintf(stderr,"\nUnable to read data from sonardepth file <%s>\n",sonardepthfile);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }

		/* read the data points in the separate file */
		nsonardepth = 0;
		while (fgets(buffer, MB_PATH_MAXLINE, tfp) != NULL)
			{
			if (buffer[0] != '#' && buffer[0] != 'O')
				{
				nscan = sscanf(buffer, "%d:%d:%d:%d:%d:%lf %lf",
						&year, &month, &day, &hour, &minute, &second,
						&sonardepth);
				if (nscan == 7)
					{
					time_i[0] = year;
					time_i[1] = month;
					time_i[2] = day;
					time_i[3] = hour;
					time_i[4] = minute;
					time_i[5] = (int)floor(second);
					time_i[6] = (int)((second - time_i[5]) * 1000000.0);
					mb_get_time(verbose, time_i, &time_d);

					sonardepth_time_d[nsonardepth] = time_d;
					sonardepth_sonardepth[nsonardepth] = sonardepth;
					nsonardepth++;
					}
				}
			nsonardepth++;
			}
		fclose(tfp);
		}

	/* get time lag model if specified */
	if (timelagmode == MBHYSWEEPPREPROCESS_TIMELAG_MODEL)
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
		}

	/* output counts */
	fprintf(stdout, "\nData available for merging:\n");
	fprintf(stdout, "     Navigation (northing easting sonardepth altitude heading): %d\n", nnav);
	fprintf(stdout, "     Sonar depth (sonardepth):                                  %d\n", nsonardepth);
	fprintf(stdout, "     Time lag:                                                  %d\n", ntimelag);
	fprintf(stdout, "\nOffsets to be applied:\n");
	fprintf(stdout, "     Roll bias:    %8.3f\n", offset_sonar_roll);
	fprintf(stdout, "     Pitch bias:   %8.3f\n", offset_sonar_pitch);
	fprintf(stdout, "     Heading bias: %8.3f\n", offset_sonar_heading);
	fprintf(stdout, "               X (m)   Y (m)   Z (m)   T (sec)\n");
	fprintf(stdout, "     Sonar: %8.3f %8.3f %8.3f %8.3f\n",
				offset_sonar_x, offset_sonar_y,
				offset_sonar_z, offset_sonar_t);
	fprintf(stdout, "     MRU:   %8.3f %8.3f %8.3f %8.3f\n",
				offset_mru_x, offset_mru_y,
				offset_mru_z, offset_mru_t);
	fprintf(stdout, "     Nav:   %8.3f %8.3f %8.3f %8.3f\n",
				offset_nav_x, offset_nav_y,
				offset_nav_z, offset_nav_t);

	nrec_POS_tot = 0;
	nrec_POSunused_tot = 0;
	nrec_GYR_tot = 0;
	nrec_HCP_tot = 0;
	nrec_EC1_tot = 0;
	nrec_DFT_tot = 0;
	nrec_RMB_tot = 0;
	nrec_other_tot = 0;

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
	while (read_data == MB_YES && format == MBF_HYSWEEP1)
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
	istore = (struct mbsys_hysweep_struct *) istore_ptr;

	/* set projection if specified */
	if (projection_set == MB_YES)
		{
		strcpy(istore->PRJ_proj4_command,proj4command);
		}

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
	nrec_POS = 0;
	nrec_POSunused = 0;
	nrec_GYR = 0;
	nrec_HCP = 0;
	nrec_EC1 = 0;
	nrec_DFT = 0;
	nrec_RMB = 0;
	nrec_other = 0;

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

	   	/* handle multibeam data */
		if (status == MB_SUCCESS && kind == MB_DATA_DATA)
			{
			nrec_RMB++;
			}

	   	/* save primary navigation data */
		else if (status == MB_SUCCESS
			&& (istore->kind == MB_DATA_NAV
				|| istore->kind == MB_DATA_NAV1
				|| istore->kind == MB_DATA_NAV2))
			{
			/* check device for being enabled */
			device = (struct mbsys_hysweep_device_struct *)&(istore->devices[istore->POS_device_number]);
			if (device->DV2_enabled == MB_YES)
				{
				nrec_POS++;

				/* add latest fix */
				if (imb_io_ptr->projection_initialized == MB_YES)
					{
					mb_proj_inverse(verbose, imb_io_ptr->pjptr,
									istore->POS_x,
									istore->POS_y,
									&navlon, &navlat,
									&error);
					}
				else
					{
					navlon = istore->POS_x;
					navlat = istore->POS_y;
					}

				if (MBHYSWEEPPREPROCESS_TIMESTAMPLIST == MB_YES)
					fprintf(stderr,"Record time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d POS record:%d\n",
						istore->time_i[0],istore->time_i[1],istore->time_i[2],
						istore->time_i[3],istore->time_i[4],istore->time_i[5],istore->time_i[6],nrec_POS);

				/* allocate memory for position arrays if needed */
				if (ndat_nav + 1 >= ndat_nav_alloc)
					{
					ndat_nav_alloc +=  MBHYSWEEPPREPROCESS_ALLOC_CHUNK;
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
				if (ndat_nav == 0 || dat_nav_time_d[ndat_nav-1] < istore->time_d)
					{
					dat_nav_time_d[ndat_nav] = istore->time_d;
					dat_nav_lon[ndat_nav] = navlon;
					dat_nav_lat[ndat_nav] = navlat;
					ndat_nav++;
					}
				}
			else
				{
				nrec_POSunused++;
				}

			}

	   	/* save primary attitude data */
		if (status == MB_SUCCESS && kind == MB_DATA_ATTITUDE)
			{
			/* check device for being enabled */
			device = (struct mbsys_hysweep_device_struct *)&(istore->devices[istore->HCP_device_number]);
			if (device->DV2_enabled == MB_YES)
				{
				nrec_HCP++;


				if (MBHYSWEEPPREPROCESS_TIMESTAMPLIST == MB_YES)
					fprintf(stderr,"Record time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d HCP record:%d\n",
						istore->time_i[0],istore->time_i[1],istore->time_i[2],
						istore->time_i[3],istore->time_i[4],istore->time_i[5],istore->time_i[6],nrec_HCP);

				/* allocate memory for position arrays if needed */
				if (ndat_rph + 1 >= ndat_rph_alloc)
					{
					ndat_rph_alloc +=  MBHYSWEEPPREPROCESS_ALLOC_CHUNK;
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
				if (ndat_rph == 0 || dat_rph_time_d[ndat_rph-1] < istore->time_d)
					{
					dat_rph_time_d[ndat_rph] = istore->time_d;
					dat_rph_roll[ndat_rph] = -istore->HCP_roll;
					dat_rph_pitch[ndat_rph] = istore->HCP_pitch;
					dat_rph_heave[ndat_rph] = -istore->HCP_heave;
					ndat_rph++;
					}
				}
			}

	   	/* save primary heading data */
		if (status == MB_SUCCESS && kind == MB_DATA_HEADING)
			{
			/* check device for being enabled */
			device = (struct mbsys_hysweep_device_struct *)&(istore->devices[istore->GYR_device_number]);
			if (device->DV2_enabled == MB_YES)
				{
				nrec_GYR++;

				if (MBHYSWEEPPREPROCESS_TIMESTAMPLIST == MB_YES)
					fprintf(stderr,"Record time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d GYR record:%d\n",
						istore->time_i[0],istore->time_i[1],istore->time_i[2],
						istore->time_i[3],istore->time_i[4],istore->time_i[5],istore->time_i[6],nrec_GYR);

				/* allocate memory for position arrays if needed */
				if (ndat_heading + 1 >= ndat_heading_alloc)
					{
					ndat_heading_alloc +=  MBHYSWEEPPREPROCESS_ALLOC_CHUNK;
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
				if (ndat_heading == 0 || dat_heading_time_d[ndat_heading-1] < istore->time_d)
					{
					dat_heading_time_d[ndat_heading] = istore->time_d;
					dat_heading_heading[ndat_heading] = istore->GYR_heading;
					ndat_heading++;
					}
				}
			}

	   	/* save primary sonardepth data */
		if (status == MB_SUCCESS && kind == MB_DATA_SONARDEPTH)
			{
			/* check device for being enabled */
			device = (struct mbsys_hysweep_device_struct *)&(istore->devices[istore->DFT_device_number]);
			if (device->DV2_enabled == MB_YES)
				{
				nrec_DFT++;

				if (MBHYSWEEPPREPROCESS_TIMESTAMPLIST == MB_YES)
					fprintf(stderr,"Record time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d DFT record:%d\n",
						istore->time_i[0],istore->time_i[1],istore->time_i[2],
						istore->time_i[3],istore->time_i[4],istore->time_i[5],istore->time_i[6],nrec_DFT);

				/* allocate memory for position arrays if needed */
				if (ndat_sonardepth + 1 >= ndat_sonardepth_alloc)
					{
					ndat_sonardepth_alloc +=  MBHYSWEEPPREPROCESS_ALLOC_CHUNK;
					status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_sonardepth_alloc*sizeof(double),(void **)&dat_sonardepth_time_d,&error);
					status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_sonardepth_alloc*sizeof(double),(void **)&dat_sonardepth_sonardepth,&error);
					if (error != MB_ERROR_NO_ERROR)
						{
						mb_error(verbose,error,&message);
						fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
						fprintf(stderr,"\nProgram <%s> Terminated\n",
						    program_name);
						exit(error);
						}
					}

				/* store the sonardepth data */
				if (ndat_sonardepth == 0 || dat_sonardepth_time_d[ndat_sonardepth-1] < istore->time_d)
					{
					dat_sonardepth_time_d[ndat_sonardepth] = istore->time_d;
					dat_sonardepth_sonardepth[ndat_sonardepth] = istore->DFT_draft;
					ndat_sonardepth++;
					}
				}
			}

	   	/* save primary altitude data */
		if (status == MB_SUCCESS && kind == MB_DATA_ALTITUDE)
			{
			/* check device for being enabled */
			device = (struct mbsys_hysweep_device_struct *)&(istore->devices[istore->GYR_device_number]);
			if (device->DV2_enabled == MB_YES)
				{
				nrec_EC1++;

				if (MBHYSWEEPPREPROCESS_TIMESTAMPLIST == MB_YES)
					fprintf(stderr,"Record time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d EC1 record:%d\n",
						istore->time_i[0],istore->time_i[1],istore->time_i[2],
						istore->time_i[3],istore->time_i[4],istore->time_i[5],istore->time_i[6],nrec_EC1);

				/* allocate memory for position arrays if needed */
				if (ndat_altitude + 1 >= ndat_altitude_alloc)
					{
					ndat_altitude_alloc +=  MBHYSWEEPPREPROCESS_ALLOC_CHUNK;
					status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_altitude_alloc*sizeof(double),(void **)&dat_altitude_time_d,&error);
					status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_altitude_alloc*sizeof(double),(void **)&dat_altitude_altitude,&error);
					if (error != MB_ERROR_NO_ERROR)
						{
						mb_error(verbose,error,&message);
						fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
						fprintf(stderr,"\nProgram <%s> Terminated\n",
						    program_name);
						exit(error);
						}
					}

				/* store the altitude data */
				if (ndat_altitude == 0 || dat_altitude_time_d[ndat_altitude-1] < istore->time_d)
					{
					dat_altitude_time_d[ndat_altitude] = istore->time_d;
					dat_altitude_altitude[ndat_altitude] = istore->EC1_rawdepth;
					ndat_altitude++;
					}
				}
			}

	   	/* handle unknown data */
		else  if (status == MB_SUCCESS)
			{
/*fprintf(stderr,"DATA TYPE UNKNOWN: status:%d error:%d kind:%d\n",status,error,kind);*/
			nrec_other++;
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
	fprintf(stdout, "\nData records read from: %s\n", ifile);
	fprintf(stdout, "     Positions (POS):                   %d\n", nrec_POS);
	fprintf(stdout, "     Positions ignored (POS):           %d\n", nrec_POSunused);
	fprintf(stdout, "     Heading (GYR):                     %d\n", nrec_GYR);
	fprintf(stdout, "     Attitude (HCP):                    %d\n", nrec_HCP);
	fprintf(stdout, "     Echosounder (altitude) (EC1):      %d\n", nrec_EC1);
	fprintf(stdout, "     Dynamic draft (DFT):               %d\n", nrec_DFT );
	fprintf(stdout, "     Raw multibeam (RMB):               %d\n", nrec_RMB);
	fprintf(stdout, "     Other:                             %d\n", nrec_other);
	nrec_POS_tot += nrec_POS;
	nrec_POSunused_tot += nrec_POSunused;
	nrec_GYR_tot += nrec_GYR;
	nrec_HCP_tot += nrec_HCP;
	nrec_EC1_tot += nrec_EC1;
	nrec_DFT_tot += nrec_DFT;
	nrec_RMB_tot += nrec_RMB;
	nrec_other_tot += nrec_other;

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

	/* apply time lag to all relevant data
		timelag value calculated either from model imported from file (timelagmode == MBHYSWEEPPREPROCESS_TIMELAG_MODEL)
			or by a constant offset (timelagmode == MBHYSWEEPPREPROCESS_TIMELAG_CONSTANT) */
	if (timelagmode != MBHYSWEEPPREPROCESS_TIMELAG_OFF)
		{
		/* correct time of navigation, heading, attitude, sonardepth, altitude
			read from asynchronous records in files */
fprintf(stderr,"Applying timelag to %d nav data\n", ndat_nav);
		j = 0;
		for (i=0;i<ndat_nav;i++)
			{
			/* get timelag value */
			timelag = offset_nav_t;
			if (timelagmode == MBHYSWEEPPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MBHYSWEEPPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose,
							timelag_time_d-1, timelag_model-1,
							ntimelag, dat_nav_time_d[i], &timelagm, &j,
							&error);
				timelag -= timelagm;
				}
			dat_nav_time_d[i] += timelag;
			}
fprintf(stderr,"Applying timelag to %d heading data\n", ndat_heading);
		j = 0;
		for (i=0;i<ndat_heading;i++)
			{
			/* get timelag value */
			timelag = offset_nav_t;
			if (timelagmode == MBHYSWEEPPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MBHYSWEEPPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose,
							timelag_time_d-1, timelag_model-1,
							ntimelag, dat_heading_time_d[i], &timelagm, &j,
							&error);
				timelag -= timelagm;
				}
			dat_heading_time_d[i] += timelag;
			}
fprintf(stderr,"Applying timelag to %d attitude data\n", ndat_rph);
		j = 0;
		for (i=0;i<ndat_rph;i++)
			{
			/* get timelag value */
			timelag = offset_mru_t;
			if (timelagmode == MBHYSWEEPPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MBHYSWEEPPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose,
							timelag_time_d-1, timelag_model-1,
							ntimelag, dat_rph_time_d[i], &timelagm, &j,
							&error);
				timelag -= timelagm;
				}
			dat_rph_time_d[i] += timelag;
			}
fprintf(stderr,"Applying timelag to %d sonardepth data\n", ndat_sonardepth);
		j = 0;
		for (i=0;i<ndat_sonardepth;i++)
			{
			/* get timelag value */
			timelag = offset_nav_t;
			if (timelagmode == MBHYSWEEPPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MBHYSWEEPPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose,
							timelag_time_d-1, timelag_model-1,
							ntimelag, dat_sonardepth_time_d[i], &timelagm, &j,
							&error);
				timelag -= timelagm;
				}
			dat_sonardepth_time_d[i] += timelag;
			}
fprintf(stderr,"Applying timelag to %d altitude data\n", ndat_altitude);
		j = 0;
		for (i=0;i<ndat_altitude;i++)
			{
			/* get timelag value */
			timelag = offset_nav_t;
			if (timelagmode == MBHYSWEEPPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MBHYSWEEPPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose,
							timelag_time_d-1, timelag_model-1,
							ntimelag, dat_altitude_time_d[i], &timelagm, &j,
							&error);
				timelag -= timelagm;
				}
			dat_altitude_time_d[i] += timelag;
			}

		/* correct time of nav data read from separate file */
fprintf(stderr,"Applying timelag to %d INS data\n", nnav);
		for (i=0;i<nnav;i++)
			{
			/* get timelag value */
			timelag = offset_mru_t;
			if (timelagmode == MBHYSWEEPPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MBHYSWEEPPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose,
							timelag_time_d-1, timelag_model-1,
							ntimelag, nav_time_d[i], &timelagm, &j,
							&error);
				timelag -= timelagm;
				}
			nav_time_d[i] += timelag;
			}

		/* correct time of sonar depth data read from separate file */
fprintf(stderr,"Applying timelag to %d sonardepth nav data\n", nsonardepth);
		for (i=0;i<nsonardepth;i++)
			{
			/* get timelag value */
			timelag = offset_nav_t;
			if (timelagmode == MBHYSWEEPPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MBHYSWEEPPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose,
							timelag_time_d-1, timelag_model-1,
							ntimelag, sonardepth_time_d[i], &timelagm, &j,
							&error);
				timelag -= timelagm;
				}
			sonardepth_time_d[i] += timelag;
			}
		}

	/* apply roll bias */
	if (offset_sonar_roll != 0.0)
		{
		for (i=0;i<ndat_rph;i++)
			{
			dat_rph_roll[i] += offset_sonar_roll;
			}
		}

	/* apply pitch bias */
	if (offset_sonar_pitch != 0.0)
		{
		for (i=0;i<ndat_rph;i++)
			{
			dat_rph_pitch[i] += offset_sonar_pitch;
			}
		}

	/* apply heading bias */
	if (offset_sonar_heading != 0.0)
		{
		for (i=0;i<ndat_heading;i++)
			{
			dat_heading_heading[i] += offset_sonar_heading;
			if (dat_heading_heading[i] >= 360.0)
				dat_heading_heading[i] -= 360.0;
			else if (dat_heading_heading[i] < 0.0)
				dat_heading_heading[i] += 360.0;
			}
		for (i=0;i<nnav;i++)
			{
			nav_heading[i] += offset_sonar_heading;
			if (nav_heading[i] >= 360.0)
				nav_heading[i] -= 360.0;
			else if (nav_heading[i] < 0.0)
				nav_heading[i] += 360.0;
			}
		}

	/* output navigation data */
	if (nnav > 0 && (verbose > 0 || mode == MBHYSWEEPPREPROCESS_TIMESTAMPLIST))
		fprintf(stdout, "\nTotal navigation data read: %d\n", nnav);
	if (nnav > 0 && (mode == MBHYSWEEPPREPROCESS_TIMESTAMPLIST))
		{
		fprintf(stdout, "\nTotal navigation data read: %d\n", nnav);
		for (i=0;i<nnav;i++)
			{
			fprintf(stdout, "  NAVIGATION: %12d %17.6f %11.6f %10.6f %8.3f %7.3f %6.3f\n",
				i, nav_time_d[i], nav_lon[i], nav_lat[i], nav_heading[i],
				nav_sonardepth[i], nav_altitude[i]);
			}
		}

	/* output sonardepth data */
	if (nsonardepth > 0 && (verbose >= 0 || mode == MBHYSWEEPPREPROCESS_TIMESTAMPLIST))
		fprintf(stdout, "\nTotal sonardepth data read: %d\n", nsonardepth);
	if (nsonardepth > 0 && (mode == MBHYSWEEPPREPROCESS_TIMESTAMPLIST))
		{
		fprintf(stdout, "\nTotal sonardepth data read: %d\n", nsonardepth);
		for (i=0;i<nnav;i++)
			{
			fprintf(stdout, "  SONARDEPTH: %12d %8.3f %8.3f\n",
				i, sonardepth_time_d[i], sonardepth_sonardepth[i]);
			}
		}

	/* output asynchronous navigation and attitude data */
	if (verbose > 0 || mode == MBHYSWEEPPREPROCESS_TIMESTAMPLIST)
		fprintf(stdout, "\nTotal navigation data read: %d\n", ndat_nav);
	if (mode == MBHYSWEEPPREPROCESS_TIMESTAMPLIST)
		for (i=0;i<ndat_nav;i++)
			{
			fprintf(stdout, "  NAV: %5d %17.6f %11.6f %10.6f\n",
				i, dat_nav_time_d[i], dat_nav_lon[i], dat_nav_lat[i]);
			}
	if (verbose > 0 || mode == MBHYSWEEPPREPROCESS_TIMESTAMPLIST)
		fprintf(stdout, "\nTotal heading data read: %d\n", ndat_heading);
	if (mode == MBHYSWEEPPREPROCESS_TIMESTAMPLIST)
		for (i=0;i<ndat_heading;i++)
			{
			fprintf(stdout, "  HDG: %5d %17.6f %8.3f\n",
				i, dat_heading_time_d[i], dat_heading_heading[i]);
			}
	if (verbose > 0 || mode == MBHYSWEEPPREPROCESS_TIMESTAMPLIST)
		fprintf(stdout, "\nTotal sonardepth data read: %d\n", ndat_sonardepth);
	if (mode == MBHYSWEEPPREPROCESS_TIMESTAMPLIST)
		for (i=0;i<ndat_sonardepth;i++)
			{
			fprintf(stdout, "  DEP: %5d %17.6f %8.3f\n",
				i, dat_sonardepth_time_d[i], dat_sonardepth_sonardepth[i]);
			}
	if (verbose > 0 || mode == MBHYSWEEPPREPROCESS_TIMESTAMPLIST)
		fprintf(stdout, "\nTotal altitude data read: %d\n", ndat_altitude);
	if (mode == MBHYSWEEPPREPROCESS_TIMESTAMPLIST)
		for (i=0;i<ndat_altitude;i++)
			{
			fprintf(stdout, "  ALT: %5d %17.6f %8.3f\n",
				i, dat_altitude_time_d[i], dat_altitude_altitude[i]);
			}
	if (verbose > 0 || mode == MBHYSWEEPPREPROCESS_TIMESTAMPLIST)
		fprintf(stdout, "\nTotal attitude data read: %d\n", ndat_rph);
	if (mode == MBHYSWEEPPREPROCESS_TIMESTAMPLIST)
		for (i=0;i<ndat_rph;i++)
			{
			fprintf(stdout, "  HCP: %5d %17.6f %8.3f %8.3f %8.3f\n",
				i, dat_rph_time_d[i], dat_rph_roll[i], dat_rph_pitch[i], dat_rph_heave[i]);
			}

	/* output counts */
	fprintf(stdout, "\nTotal data records read from: %s\n", read_file);
	fprintf(stdout, "     Positions (POS):                   %d\n", nrec_POS_tot);
	fprintf(stdout, "     Positions ignored (POS):           %d\n", nrec_POSunused_tot);
	fprintf(stdout, "     Heading (GYR):                     %d\n", nrec_GYR_tot);
	fprintf(stdout, "     Attitude (HCP):                    %d\n", nrec_HCP_tot);
	fprintf(stdout, "     Echosounder (altitude) (EC1):      %d\n", nrec_EC1_tot);
	fprintf(stdout, "     Dynamic draft (DFT):               %d\n", nrec_DFT_tot);
	fprintf(stdout, "     Raw multibeam (RMB):               %d\n", nrec_RMB_tot);
	fprintf(stdout, "     Other:                             %d\n", nrec_other_tot);
	nrec_POS_tot = 0;
	nrec_POSunused_tot = 0;
	nrec_GYR_tot = 0;
	nrec_HCP_tot = 0;
	nrec_EC1_tot = 0;
	nrec_DFT_tot = 0;
	nrec_RMB_tot = 0;
	nrec_other_tot = 0;

	/* now read the data files again, this time interpolating nav and attitude
		into the multibeam records and fixing other problems found in the
		data */
	if (mode == MBHYSWEEPPREPROCESS_PROCESS)
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
	while (read_data == MB_YES && format == MBF_HYSWEEP1)
	{
	/* figure out the output file name */
	status = mb_get_format(verbose, ifile, fileroot, &testformat, &error);
	if (testformat == MBF_HYSWEEP1
		&& strncmp(".HSX",&ifile[strlen(ifile)-4],4) == 0)
		sprintf(ofile, "%s.mb%d", fileroot, testformat);
	else if (testformat == MBF_HYSWEEP1
		&& strncmp(".hsx",&ifile[strlen(ifile)-4],4) == 0)
		sprintf(ofile, "%s.mb%d", fileroot, testformat);
	else if (testformat == MBF_HYSWEEP1)
		sprintf(ofile, "%s.mb%d", ifile, testformat);

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

	/* initialize writing the output swath sonar file */
	if ((status = mb_write_init(
		verbose,ofile,format,&ombio_ptr,
		&obeams_bath,&obeams_amp,&opixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_write_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for writing\n",ofile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* get pointers to data storage */
	imb_io_ptr = (struct mb_io_struct *) imbio_ptr;
	istore_ptr = imb_io_ptr->store_data;
	istore = (struct mbsys_hysweep_struct *) istore_ptr;

	/* set projection if specified */
	if (projection_set == MB_YES)
		{
		strcpy(istore->PRJ_proj4_command,proj4command);
		}

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
	nrec_POS = 0;
	nrec_POSunused = 0;
	nrec_GYR = 0;
	nrec_HCP = 0;
	nrec_EC1 = 0;
	nrec_DFT = 0;
	nrec_RMB = 0;
	nrec_other = 0;

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

	   	/* handle multibeam data */
		if (status == MB_SUCCESS && kind == MB_DATA_DATA)
			{
			nrec_RMB++;

			/* print debug statements */
			if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  Multibeam bathymetry read in by MB-System program <%s>\n",program_name);
				fprintf(stderr,"dbg4       RMB_device_number:                 %d\n", istore->RMB_device_number);
				fprintf(stderr,"dbg4       RMB_time:                          %f\n", istore->RMB_time);
				fprintf(stderr,"dbg4       RMB_sonar_type:                    %x\n", istore->RMB_sonar_type);
				fprintf(stderr,"dbg4       RMB_sonar_flags:                   %x\n", istore->RMB_sonar_flags);
				fprintf(stderr,"dbg4       RMB_beam_data_available:           %x\n", istore->RMB_beam_data_available);
				fprintf(stderr,"dbg4       RMB_num_beams:                     %d\n", istore->RMB_num_beams);
				fprintf(stderr,"dbg4       RMB_num_beams_alloc:               %d\n", istore->RMB_num_beams_alloc);
				fprintf(stderr,"dbg4       RMB_sound_velocity:                %f\n", istore->RMB_sound_velocity);
				fprintf(stderr,"dbg4       RMB_ping_number:                   %d\n", istore->RMB_ping_number);
				for (i=0;i<istore->RMB_num_beams;i++)
					{
					fprintf(stderr,"dbg4       beam:%4d", i);

					if (istore->RMB_beam_data_available & 0x0001)
					fprintf(stderr," mbrng:%f", istore->RMB_beam_ranges[i]);

					if (istore->RMB_beam_data_available & 0x0002)
					fprintf(stderr," mtrng:%f", istore->RMB_multi_ranges[i]);

					if (istore->RMB_beam_data_available & 0x0004)
					fprintf(stderr," est:%f", istore->RMB_sounding_eastings[i]);

					if (istore->RMB_beam_data_available & 0x0004)
					fprintf(stderr," nor:%f", istore->RMB_sounding_northings[i]);

					if (istore->RMB_beam_data_available & 0x0008)
					fprintf(stderr," dep:%f", istore->RMB_sounding_depths[i]);

					if (istore->RMB_beam_data_available & 0x0010)
					fprintf(stderr," ltr:%f", istore->RMB_sounding_along[i]);

					if (istore->RMB_beam_data_available & 0x0020)
					fprintf(stderr," atr:%f", istore->RMB_sounding_across[i]);

					if (istore->RMB_beam_data_available & 0x0040)
					fprintf(stderr," pth:%f", istore->RMB_sounding_pitchangles[i]);

					if (istore->RMB_beam_data_available & 0x0080)
					fprintf(stderr," rll:%f", istore->RMB_sounding_rollangles[i]);

					if (istore->RMB_beam_data_available & 0x0100)
					fprintf(stderr," toa:%f", istore->RMB_sounding_takeoffangles[i]);

					if (istore->RMB_beam_data_available & 0x0200)
					fprintf(stderr," azi:%f", istore->RMB_sounding_azimuthalangles[i]);

					if (istore->RMB_beam_data_available & 0x0400)
					fprintf(stderr," tim:%d", istore->RMB_sounding_timedelays[i]);

					if (istore->RMB_beam_data_available & 0x0800)
					fprintf(stderr," int:%d", istore->RMB_sounding_intensities[i]);

					if (istore->RMB_beam_data_available & 0x1000)
					fprintf(stderr," qua:%d", istore->RMB_sounding_quality[i]);

					if (istore->RMB_beam_data_available & 0x2000)
					fprintf(stderr," flg:%d", istore->RMB_sounding_flags[i]);

					fprintf(stderr,"\n");
					}
				}

			/* merge navigation from best available source */
			if (nnav > 0)
				{
				interp_status = mb_linear_interp_longitude(verbose,
							nav_time_d-1, nav_lon-1,
							nnav, time_d, &navlon, &j,
							&error);
				if (interp_status == MB_SUCCESS)
				interp_status = mb_linear_interp_latitude(verbose,
							nav_time_d-1, nav_lat-1,
							nnav, time_d, &navlat, &j,
							&error);
				}
			else if (ndat_nav > 0)
				{
				interp_status = mb_linear_interp_longitude(verbose,
							dat_nav_time_d-1, dat_nav_lon-1,
							ndat_nav, time_d, &navlon, &j,
							&error);
				if (interp_status == MB_SUCCESS)
				interp_status = mb_linear_interp_latitude(verbose,
							dat_nav_time_d-1, dat_nav_lat-1,
							ndat_nav, time_d, &navlat, &j,
							&error);
				}
			else
				{
				navlon = 0.0;
				navlat = 0.0;
				speed = 0.0;
				}

			/* merge heading from best available source */
			if (nnav > 0)
				{
				interp_status = mb_linear_interp_heading(verbose,
							nav_time_d-1, nav_heading-1,
							nnav, time_d, &heading, &j,
							&error);
				}
			else if (ndat_heading > 0)
				{
				interp_status = mb_linear_interp_heading(verbose,
							dat_heading_time_d-1, dat_heading_heading-1,
							ndat_heading, time_d, &heading, &j,
							&error);
				}
			else
				{
				heading = 0.0;
				}
			if (heading < 0.0)
				heading += 360.0;
			else if (heading >= 360.0)
				heading -= 360.0;

			/* merge sonardepth from best available source */
			if (nsonardepth > 0)
				{
				interp_status = mb_linear_interp(verbose,
							sonardepth_time_d-1, sonardepth_sonardepth-1,
							nsonardepth, time_d, &sonardepth, &j,
							&error);
				}
			else if (nnav > 0)
				{
				interp_status = mb_linear_interp(verbose,
							nav_time_d-1, nav_sonardepth-1,
							nnav, time_d, &sonardepth, &j,
							&error);
				}
			else if (ndat_sonardepth > 0)
				{
				interp_status = mb_linear_interp(verbose,
							dat_sonardepth_time_d-1, dat_sonardepth_sonardepth-1,
							ndat_sonardepth, time_d, &sonardepth, &j,
							&error);
				}
			else
				{
				sonardepth = 0.0;
				}

			/* merge altitude from best available source */
			if (nnav > 0)
				{
				interp_status = mb_linear_interp(verbose,
							nav_time_d-1, nav_altitude-1,
							nnav, time_d, &altitude, &j,
							&error);
				}
			else if (ndat_altitude > 0)
				{
				interp_status = mb_linear_interp(verbose,
							dat_altitude_time_d-1, dat_altitude_altitude-1,
							ndat_altitude, time_d, &altitude, &j,
							&error);
				}
			else
				{
				altitude = 0.0;
				}

			/* get attitude from best available source */
			if (ndat_rph > 0)
				{
				interp_status = mb_linear_interp(verbose,
							dat_rph_time_d-1, dat_rph_roll-1,
							ndat_rph, time_d, &roll, &j,
							&error);
				if (interp_status == MB_SUCCESS)
				interp_status = mb_linear_interp(verbose,
							dat_rph_time_d-1, dat_rph_pitch-1,
							ndat_rph, time_d, &pitch, &j,
							&error);
				}
			else
				{
				roll = 0.0;
				pitch = 0.0;
				}

			/* do lever arm calculation with sensor offsets */
			mb_lever(verbose, offset_sonar_x, offset_sonar_y, offset_sonar_z,
					offset_mru_x, offset_mru_y, offset_mru_z,
					offset_nav_x, offset_nav_y, offset_nav_z,
					roll, pitch,
					&lever_x, &lever_y, &lever_z, &error);

			/* set values at sonar ping time */
			istore->RMBint_lon = navlon;
			istore->RMBint_lat = navlat;
			if (imb_io_ptr->projection_initialized == MB_YES)
				{
				mb_proj_forward(verbose, imb_io_ptr->pjptr,
								istore->RMBint_lon,
								istore->RMBint_lat,
								&(istore->RMBint_x),
								&(istore->RMBint_y),
								&error);
				}
			else
				{
				istore->RMBint_x = istore->RMBint_lon;
				istore->RMBint_y = istore->RMBint_lat;
				}
			istore->RMBint_heave = 0.0;
			istore->RMBint_roll = roll;
			istore->RMBint_pitch = pitch;
			istore->RMBint_heading = heading;
			istore->RMBint_draft = sonardepth - lever_z;

			/* get mapping sonar device pointer */
			device = (struct mbsys_hysweep_device_struct *)&(istore->devices[istore->RMB_device_number]);

			/* deal with case of multibeam sonar - recalculate bathymetry if possible */
			if (istore->RMB_beam_data_available & 0x0001)
				{
				/* handle data that starts with beam angles in roll and pitch coordinates */
				if (istore->RMB_sonar_type == 1 || istore->RMB_sonar_type == 2)
					{
					/* get beam roll angles from sonar parameters if necessary */
					if (!(istore->RMB_beam_data_available & 0x0080))
						{
						for (i=0;i<istore->RMB_num_beams;i++)
							{
							istore->RMB_sounding_rollangles[i]
								= device->MBI_first_beam_angle + i * device->MBI_angle_increment;
							}
						istore->RMB_beam_data_available = istore->RMB_beam_data_available | 0x0080;
						}

					/* set zero beam pitch angles if necessary */
					if (!(istore->RMB_beam_data_available & 0x0040))
						{
						for (i=0;i<istore->RMB_num_beams;i++)
							{
							istore->RMB_sounding_pitchangles[i] = 0.0;
							}
						istore->RMB_beam_data_available = istore->RMB_beam_data_available | 0x0040;
						}

					/* get beam takeoff and azimuthal angles */
					for (i=0;i<istore->RMB_num_beams;i++)
						{
						alpha = istore->RMB_sounding_pitchangles[i];
						beta = 90.0 + istore->RMB_sounding_rollangles[i];

						/* correct alpha for pitch if necessary */
						if (!(device->MBI_sonar_flags & 0x0002))
							alpha += istore->RMBint_pitch;

						/* correct beta for roll if necessary */
						if (!(device->MBI_sonar_flags & 0x0001))
							beta -= istore->RMBint_roll;

						mb_rollpitch_to_takeoff(
							verbose,
							alpha, beta,
							&theta, &phi,
							&error);
						istore->RMB_sounding_takeoffangles[i] = theta;
						istore->RMB_sounding_azimuthalangles[i] = 90.0 - phi;
						}
					istore->RMB_beam_data_available = istore->RMB_beam_data_available | 0x0300;
					}

				/* recalculate beam bathymetry if beam takeoff and azimuthal angles are available */
				if ((istore->RMB_beam_data_available & 0x0300))
					{
					for (i=0;i<istore->RMB_num_beams;i++)
						{
						rr = istore->RMB_beam_ranges[i];
						theta = istore->RMB_sounding_takeoffangles[i];
						phi = 90.0 - istore->RMB_sounding_azimuthalangles[i];
						xx = rr * sin(DTR * theta);
						zz = rr * cos(DTR * theta);
						istore->RMB_sounding_across[i] = xx * cos(DTR * phi);
						istore->RMB_sounding_along[i] = xx * sin(DTR * phi);
						istore->RMB_sounding_depths[i] = zz + istore->RMBint_draft - istore->RMBint_heave;
						}
					istore->RMB_beam_data_available = istore->RMB_beam_data_available | 0x0038;
					}

				/* get beam flags if necessary */
				if (!(istore->RMB_beam_data_available & 0x2000))
					{
					for (i=0;i<istore->RMB_num_beams;i++)
						{
						istore->RMB_sounding_flags[i] = MB_FLAG_NONE;
						}
					istore->RMB_beam_data_available = istore->RMB_beam_data_available | 0x2000;

					/* incorporate quality values */
					if ((istore->RMB_beam_data_available & 0x1000)
						&& strncmp(device->DEV_device_name, "Reson Seabat 8", 14) == 0)
						{
						for (i=0;i<istore->RMB_num_beams;i++)
							{
							if (istore->RMB_sounding_quality[i] < 2)
								istore->RMB_sounding_flags[i] = MB_FLAG_FLAG + MB_FLAG_SONAR;
							}
						}

					/* check for null ranges */
					if ((istore->RMB_beam_data_available & 0x0001))
						{
						for (i=0;i<istore->RMB_num_beams;i++)
							{
							if (istore->RMB_beam_ranges[i] <= 0.0)
								istore->RMB_sounding_flags[i] = MB_FLAG_FLAG + MB_FLAG_SONAR;
							}
						}
					}
				}

			/* deal with case of multiple transducer sonar */
			if (istore->RMB_beam_data_available & 0x0002)
				{
				/* get beam roll angles if necessary */
				if (!(istore->RMB_beam_data_available & 0x0080))
					{
					for (i=0;i<istore->RMB_num_beams;i++)
						{
						istore->RMB_sounding_rollangles[i] = 0.0;
						}
					istore->RMB_beam_data_available = istore->RMB_beam_data_available | 0x0080;
					}

				/* correct beam roll angles for roll if necessary */
				if (!(device->MBI_sonar_flags & 0x0001))
					{
					for (i=0;i<istore->RMB_num_beams;i++)
						{
						istore->RMB_sounding_rollangles[i] += istore->RMBint_roll;
						}
					}

				/* get beam pitch angles if necessary */
				if (!(istore->RMB_beam_data_available & 0x0040))
					{
					if (!(device->MBI_sonar_flags & 0x0002))
						{
						for (i=0;i<istore->RMB_num_beams;i++)
							{
							istore->RMB_sounding_pitchangles[i] = istore->RMBint_pitch;
							}
						}
					else
						{
						for (i=0;i<istore->RMB_num_beams;i++)
							{
							istore->RMB_sounding_pitchangles[i] = 0.0;
							}
						}
					istore->RMB_beam_data_available = istore->RMB_beam_data_available | 0x0040;
					}

				/* get beam takeoff and azimuthal angles if necessary */
				if (!(istore->RMB_beam_data_available & 0x0100)
					|| !(istore->RMB_beam_data_available & 0x0200))
					{
					for (i=0;i<istore->RMB_num_beams;i++)
						{
						alpha = istore->RMB_sounding_pitchangles[i];
						beta = 90.0 - istore->RMB_sounding_rollangles[i];
						mb_rollpitch_to_takeoff(
							verbose,
							alpha, beta,
							&theta, &phi,
							&error);
						istore->RMB_sounding_takeoffangles[i] = theta;
						istore->RMB_sounding_azimuthalangles[i] = 90.0 - phi;
						}
					istore->RMB_beam_data_available = istore->RMB_beam_data_available | 0x0300;
					}

				/* get beam bathymetry if necessary */
				if (!(istore->RMB_beam_data_available & 0x0004)
					|| !(istore->RMB_beam_data_available & 0x0008)
					|| !(istore->RMB_beam_data_available & 0x0010)
					|| !(istore->RMB_beam_data_available & 0x0020))
					{
					for (i=0;i<istore->RMB_num_beams;i++)
						{
						rr = istore->RMB_multi_ranges[i];
						theta = istore->RMB_sounding_takeoffangles[i];
						phi = 90.0 - istore->RMB_sounding_azimuthalangles[i];
						xx = rr * sin(DTR * theta);
						zz = rr * cos(DTR * theta);
						istore->RMB_sounding_across[i] = xx * cos(DTR * phi);
						istore->RMB_sounding_along[i] = xx * sin(DTR * phi);
						istore->RMB_sounding_depths[i] = zz + istore->RMBint_draft - istore->RMBint_heave;
						}
					istore->RMB_beam_data_available = istore->RMB_beam_data_available | 0x003C;
					}

				/* get beam flags if necessary */
				if (!(istore->RMB_beam_data_available & 0x2000))
					{
					for (i=0;i<istore->RMB_num_beams;i++)
						{
						istore->RMB_sounding_flags[i] = MB_FLAG_NONE;
						}
					istore->RMB_beam_data_available = istore->RMB_beam_data_available | 0x2000;
					}
				}

			/* print debug statements */
			if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  Multibeam bathymetry calculated by MB-System program <%s>\n",program_name);
				fprintf(stderr,"dbg4       RMB_device_number:                 %d\n", istore->RMB_device_number);
				fprintf(stderr,"dbg4       RMB_time:                          %f\n", istore->RMB_time);
				fprintf(stderr,"dbg4       RMB_sonar_type:                    %x\n", istore->RMB_sonar_type);
				fprintf(stderr,"dbg4       RMB_sonar_flags:                   %x\n", istore->RMB_sonar_flags);
				fprintf(stderr,"dbg4       RMB_beam_data_available:           %x\n", istore->RMB_beam_data_available);
				fprintf(stderr,"dbg4       RMB_num_beams:                     %d\n", istore->RMB_num_beams);
				fprintf(stderr,"dbg4       RMB_num_beams_alloc:               %d\n", istore->RMB_num_beams_alloc);
				fprintf(stderr,"dbg4       RMB_sound_velocity:                %f\n", istore->RMB_sound_velocity);
				fprintf(stderr,"dbg4       RMB_ping_number:                   %d\n", istore->RMB_ping_number);
				for (i=0;i<istore->RMB_num_beams;i++)
					{
					fprintf(stderr,"dbg4       beam:%4d", i);

					if (istore->RMB_beam_data_available & 0x0001)
					fprintf(stderr," mbrng:%f", istore->RMB_beam_ranges[i]);

					if (istore->RMB_beam_data_available & 0x0002)
					fprintf(stderr," mtrng:%f", istore->RMB_multi_ranges[i]);

					if (istore->RMB_beam_data_available & 0x0004)
					fprintf(stderr," est:%f", istore->RMB_sounding_eastings[i]);

					if (istore->RMB_beam_data_available & 0x0004)
					fprintf(stderr," nor:%f", istore->RMB_sounding_northings[i]);

					if (istore->RMB_beam_data_available & 0x0008)
					fprintf(stderr," dep:%f", istore->RMB_sounding_depths[i]);

					if (istore->RMB_beam_data_available & 0x0010)
					fprintf(stderr," ltr:%f", istore->RMB_sounding_along[i]);

					if (istore->RMB_beam_data_available & 0x0020)
					fprintf(stderr," atr:%f", istore->RMB_sounding_across[i]);

					if (istore->RMB_beam_data_available & 0x0040)
					fprintf(stderr," pth:%f", istore->RMB_sounding_pitchangles[i]);

					if (istore->RMB_beam_data_available & 0x0080)
					fprintf(stderr," rll:%f", istore->RMB_sounding_rollangles[i]);

					if (istore->RMB_beam_data_available & 0x0100)
					fprintf(stderr," toa:%f", istore->RMB_sounding_takeoffangles[i]);

					if (istore->RMB_beam_data_available & 0x0200)
					fprintf(stderr," azi:%f", istore->RMB_sounding_azimuthalangles[i]);

					if (istore->RMB_beam_data_available & 0x0400)
					fprintf(stderr," tim:%d", istore->RMB_sounding_timedelays[i]);

					if (istore->RMB_beam_data_available & 0x0800)
					fprintf(stderr," int:%d", istore->RMB_sounding_intensities[i]);

					if (istore->RMB_beam_data_available & 0x1000)
					fprintf(stderr," qua:%d", istore->RMB_sounding_quality[i]);

					if (istore->RMB_beam_data_available & 0x2000)
					fprintf(stderr," flg:%d", istore->RMB_sounding_flags[i]);

					fprintf(stderr,"\n");
					}
				}
			}

	   	/* handle navigation data */
		else if (status == MB_SUCCESS
			&& (kind == MB_DATA_NAV
				|| kind == MB_DATA_NAV1
				|| kind == MB_DATA_NAV2))
			{
			/* check device for being enabled */
			device = (struct mbsys_hysweep_device_struct *)&(istore->devices[istore->POS_device_number]);
			if (device->DV2_enabled == MB_YES)
				nrec_POS++;
			else
				nrec_POSunused++;
			}

	   	/* handle attitude data */
		else if (status == MB_SUCCESS && kind == MB_DATA_ATTITUDE)
			{
			nrec_HCP++;
			}

	   	/* handle heading data */
		else if (status == MB_SUCCESS && kind == MB_DATA_HEADING)
			{
			nrec_GYR++;
			}

	   	/* handle altitude data */
		else if (status == MB_SUCCESS && kind == MB_DATA_ALTITUDE)
			{
			nrec_EC1++;
			}

	   	/* handle sonar depth data */
		else if (status == MB_SUCCESS && kind == MB_DATA_SONARDEPTH)
			{
			nrec_DFT++;
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

	/*--------------------------------------------
	  write the processed data
	  --------------------------------------------*/

		/* write some data */
		if (error == MB_ERROR_NO_ERROR && (nnav < 1 || kind != MB_DATA_NAV2))
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

	/* close the swath file */
	status = mb_close(verbose,&imbio_ptr,&error);
	status = mb_close(verbose,&ombio_ptr,&error);

	/* output counts */
	fprintf(stdout, "\nData records written to: %s\n", ofile);
	fprintf(stdout, "     Positions (POS):                   %d\n", nrec_POS);
	fprintf(stdout, "     Positions ignored (POS):           %d\n", nrec_POSunused);
	fprintf(stdout, "     Heading (GYR):                     %d\n", nrec_GYR);
	fprintf(stdout, "     Attitude (HCP):                    %d\n", nrec_HCP);
	fprintf(stdout, "     Echosounder (altitude) (EC1):      %d\n", nrec_EC1);
	fprintf(stdout, "     Dynamic draft (DFT):               %d\n", nrec_DFT );
	fprintf(stdout, "     Raw multibeam (RMB):               %d\n", nrec_RMB);
	fprintf(stdout, "     Other:                             %d\n", nrec_other);
	nrec_POS_tot += nrec_POS;
	nrec_POSunused_tot += nrec_POSunused;
	nrec_GYR_tot += nrec_GYR;
	nrec_HCP_tot += nrec_HCP;
	nrec_EC1_tot += nrec_EC1;
	nrec_DFT_tot += nrec_DFT;
	nrec_RMB_tot += nrec_RMB;
	nrec_other_tot += nrec_other;

	/* generate inf fnv and fbt files */
	if (status == MB_SUCCESS)
		{
		status = mb_make_info(verbose, MB_YES, ofile, format, &error);
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

	/* output counts */
	fprintf(stdout, "\nTotal data records written from: %s\n", read_file);
	fprintf(stdout, "     Positions (POS):                   %d\n", nrec_POS_tot);
	fprintf(stdout, "     Positions ignored (POS):           %d\n", nrec_POSunused_tot);
	fprintf(stdout, "     Heading (GYR):                     %d\n", nrec_GYR_tot);
	fprintf(stdout, "     Attitude (HCP):                    %d\n", nrec_HCP_tot);
	fprintf(stdout, "     Echosounder (altitude) (EC1):      %d\n", nrec_EC1_tot);
	fprintf(stdout, "     Dynamic draft (DFT):               %d\n", nrec_DFT_tot);
	fprintf(stdout, "     Raw multibeam (RMB):               %d\n", nrec_RMB_tot);
	fprintf(stdout, "     Other:                             %d\n", nrec_other_tot);
	}

	/* deallocate navigation arrays */
	if (ndat_nav > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_nav_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_nav_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_nav_lon,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_nav_lat,&error);
		}
	if (ndat_sonardepth > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_sonardepth_sonardepth,&error);
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
	if (ndat_altitude > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_altitude_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_altitude_altitude,&error);
		}
	if (nnav > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nav_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nav_lon,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nav_lat,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nav_heading,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nav_sonardepth,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nav_altitude,&error);
		}
	if (nsonardepth > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&sonardepth_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&sonardepth_sonardepth,&error);
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

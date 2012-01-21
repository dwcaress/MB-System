/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


/*--------------------------------------------------------------------
 *    The MB-system:	mbsm20002gsf.c	5/15/2009
 *
 *    $Id: mbsm20002gsf.c 1891 2011-05-04 23:46:30Z caress $
 *
 *    Copyright (c) 2009-2011 by
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
 * mbsm20002gsf translates native SM2000 MPB format data to GSF (format 121) 
 * while merging navigation from a separate file.q
 *
 * Author:	D. W. Caress
 * Location:	Trapped at the airport in Sydney Australia
 * Date:	May 15, 2009
 *
 * $Log: mbsm20002gsf.c,v $
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>

#ifdef WIN32
#include <winsock2.h>
#endif

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_aux.h"

/* local defines */
#define	SM2000_NUM_RETURNS	4
#define	SM2000_NUM_BEAMS	128
#define	SM2000_HEADER_SIZE	36
#define	SM2000_BEAM_SIZE	2
#define	BUFFERSIZE		256
struct sm2000_return_struct 
	{
	int	range[SM2000_NUM_BEAMS];
	int	quality[SM2000_NUM_BEAMS];
	int	amplitude[SM2000_NUM_BEAMS];
	
	double	traveltimes[SM2000_NUM_BEAMS];
	double	angles[SM2000_NUM_BEAMS];
	double	angles_forward[SM2000_NUM_BEAMS];
	
	char	beamflag[SM2000_NUM_BEAMS];
	double	bath[SM2000_NUM_BEAMS];
	double	bathacrosstrack[SM2000_NUM_BEAMS];
	double	bathalongtrack[SM2000_NUM_BEAMS];
	double	amp[SM2000_NUM_BEAMS];
	};
struct sm2000_ping_struct 
	{
	int	sync1;
	int	sync2;
	int	type;
	int	year;
	int	month;
	int	day;
	int	hour;
	int	minute;
	int	second;
	int	ping_number;
	int	alt_time;	/* 50 usec */
	int	agc;		/* dB? */
	int	sound_speed;	/* m/sec */
	int	time_latency;	/* msec */
	int	sample_rate;	/* Hz */
	int	swath_width;	/* degrees (60, 90, 120, 150, 180) */
	int	num_beams_tot;	/* 1-128 */
	int	start_beam;	/* 1-128 */
	int	num_beams;	/* 1-128 */
	int	num_returns;	/* 1-4 */
	int	beam_width;	/* alongtrack beam width (0.1 degrees) */
	int	sonar_range;	/* range (cm) */
	struct sm2000_return_struct returns[SM2000_NUM_RETURNS];
	};

static char rcs_id[] = "$Id: mbsm20002gsf.c 1891 2011-05-04 23:46:30Z caress $";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	/* id variables */
	char program_name[] = "mbsm20002gsf";
	char help_message[] =  "mbsm20002gsf translates native SM2000 mpb format data to GSF (format 121) while merging navigation from a separate file.";
	char usage_message[] = "mbsm20002gsf -Impbfile -Mnavformat -Nnavfile -Ogsffile [-V]";

	/* parsing variables */
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

	/* MBIO write control parameters */
	int	format = MBF_GSFGENMB;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	speedmin;
	double	timegap;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	void	*ombio_ptr = NULL;

	int	navfile_specified = MB_NO;
	int	gsffile_specified = MB_NO;
	char	mpbfile[MB_PATH_MAXLINE];
	char	gsffile[MB_PATH_MAXLINE];
	char	navfile[MB_PATH_MAXLINE];
	FILE	*mpbfp;
	FILE	*nfp;

	/* sm2000 read values */
	struct sm2000_ping_struct ping;
	int	swap = MB_YES;

	/* mbio write values */
	void	*store_ptr;
	int	kind;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	sonardepth;
	double	roll;
	double	pitch;
	int	idata = 0;
	int	odata = 0;
	int	onav = 0;
	int	ocomment = 0;
	char	comment[MB_COMMENT_MAXLINE];
	
	/* time, user, host variables */
	time_t	right_now;
	char	date[25], user[128], *user_ptr, host[128];

	/* navigation handling variables */
	int	navformat = 5;
	int	nnav;
	double	*ntime = NULL;
	double	*nlon = NULL;
	double	*nlat = NULL;
	double	*nheading = NULL;
	double	*nsonardepth = NULL;
	double	*nroll = NULL;
	double	*npitch = NULL;
	
	int	nav_ok;
	int	time_j[5], hr;
	double	sec;
	int	stime_i[7], ftime_i[7];
	int	itime;
	double	mtodeglon, mtodeglat;
	double	del_time, dx, dy, dist;
	int	intstat;
	short	short_val;
	
	double	soundspeed;
	double	alpha, beta, theta, phi;
	double	rr, xx, zz;

	int	nchar;
	char	buffer[BUFFERSIZE], *result;
	int	nget, read_len, index;
	int	i, j;

	char	*ctime();
	char	*getenv();

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input and output */
	strcpy (mpbfile, "stdin");
	strcpy (gsffile, "\0");
	strcpy (navfile, "\0");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhI:i:L:l:M:m:N:n:O:o:")) != -1)
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
		case 'I':
		case 'i':
			sscanf (optarg,"%s", mpbfile);
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
			flag++;
			break;
		case 'M':
		case 'm':
			sscanf (optarg,"%d", &navformat);
			flag++;
			break;
		case 'N':
		case 'n':
			sscanf (optarg,"%s", navfile);
			navfile_specified = MB_YES;
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", gsffile);
			gsffile_specified = MB_YES;
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
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       help:            %d\n",help);
		fprintf(stderr,"dbg2       format:          %d\n",format);
		fprintf(stderr,"dbg2       input file:      %s\n",mpbfile);
		fprintf(stderr,"dbg2       output file:     %s\n",gsffile);
		fprintf(stderr,"dbg2       navigation file: %s\n",navfile);
		fprintf(stderr,"dbg2       nav format:      %d\n",navformat);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}
		
	/* first read the navigation if any */
	if (navfile_specified == MB_YES)
		{		
		/* set max number of characters to be read at a time */
		nchar = 128;

		/* count the nav points */
		nnav = 0;
		if ((nfp = fopen(navfile, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to Open Navigation File <%s> for reading\n",navfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		while ((result = fgets(buffer,nchar,nfp)) == buffer)
			nnav++;
		fclose(nfp);

		/* allocate space for the nav points */
		status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&ntime,&error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&nlon,&error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&nlat,&error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&nheading,&error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&nsonardepth,&error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&nroll,&error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&npitch,&error);

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		/* read in nav points */
		nnav = 0;
		if ((nfp = fopen(navfile, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to Open Navigation File <%s> for reading\n",navfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		strncpy(buffer,"\0",sizeof(buffer));
		while ((result = fgets(buffer,nchar,nfp)) == buffer)
			{
			/* deal with nav in form: time_d lon lat heading sonardepth roll pitch */
			nav_ok = MB_NO;
			if (navformat == 1)
				{
				nget = sscanf(buffer,"%lf %lf %lf %lf %lf %lf %lf",
						&ntime[nnav],&nlon[nnav],&nlat[nnav],
						&nheading[nnav],&nsonardepth[nnav],
						&nroll[nnav],&npitch[nnav]);
				if (nget == 7)
					nav_ok = MB_YES;
				}

			/* deal with nav in form: yr mon day hour min sec lon lat heading sonardepth roll pitch */
			else if (navformat == 2)
				{
				nget = sscanf(buffer,"%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf",
						&time_i[0],&time_i[1],&time_i[2],
						&time_i[3],&time_i[4],&sec,
						&nlon[nnav],&nlat[nnav],
						&nheading[nnav],&nsonardepth[nnav],
						&nroll[nnav],&npitch[nnav]);
				time_i[5] = (int) sec;
				time_i[6] = 1000000*(sec - time_i[5]);
				mb_get_time(verbose,time_i,&time_d);
				ntime[nnav] = time_d;
				if (nget == 12)
					nav_ok = MB_YES;
				}

			/* deal with nav in form: yr jday hour min sec lon lat */
			else if (navformat == 3)
				{
				nget = sscanf(buffer,"%d %d %d %d %lf %lf %lf %lf %lf %lf %lf",
					&time_j[0],&time_j[1],&hr,
					&time_j[2],&sec,
					&nlon[nnav],&nlat[nnav],
						&nheading[nnav],&nsonardepth[nnav],
						&nroll[nnav],&npitch[nnav]);
				time_j[2] = time_j[2] + 60*hr;
				time_j[3] = (int) sec;
				time_j[4] = 1000000*(sec - time_j[3]);
				mb_get_itime(verbose,time_j,time_i);
				mb_get_time(verbose,time_i,&time_d);
				ntime[nnav] = time_d;
				if (nget == 7)
					nav_ok = MB_YES;
				}

			/* deal with nav in form: yr jday daymin sec lon lat */
			else if (navformat == 4)
				{
				nget = sscanf(buffer,"%d %d %d %lf %lf %lf %lf %lf %lf %lf",
						&time_j[0],&time_j[1],&time_j[2],
						&sec,
						&nlon[nnav],&nlat[nnav],
						&nheading[nnav],&nsonardepth[nnav],
						&nroll[nnav],&npitch[nnav]);
				time_j[3] = (int) sec;
				time_j[4] = 1000000*(sec - time_j[3]);
				mb_get_itime(verbose,time_j,time_i);
				mb_get_time(verbose,time_i,&time_d);
				ntime[nnav] = time_d;
				if (nget == 10)
					nav_ok = MB_YES;
				}


			/* deal with nav in form: yr mon day hour min sec time_d lon lat */
			else if (navformat == 9)
				{
				nget = sscanf(buffer,"%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf",
						&time_i[0],&time_i[1],&time_i[2],
						&time_i[3],&time_i[4],&sec,
						&ntime[nnav],
						&nlon[nnav],&nlat[nnav],
						&nheading[nnav],&nsonardepth[nnav],
						&nroll[nnav],&npitch[nnav]);
				if (nget == 13)
					nav_ok = MB_YES;
				}


			/* make sure longitude is defined according to lonflip */
			if (nav_ok == MB_YES)
				{
				if (lonflip == -1 && nlon[nnav] > 0.0)
					nlon[nnav] = nlon[nnav] - 360.0;
				else if (lonflip == 0 && nlon[nnav] < -180.0)
					nlon[nnav] = nlon[nnav] + 360.0;
				else if (lonflip == 0 && nlon[nnav] > 180.0)
					nlon[nnav] = nlon[nnav] - 360.0;
				else if (lonflip == 1 && nlon[nnav] < 0.0)
					nlon[nnav] = nlon[nnav] + 360.0;
				}

			/* output some debug values */
			if (verbose >= 5 && nav_ok == MB_YES)
				{
				fprintf(stderr,"\ndbg5  New navigation point read in program <%s>\n",program_name);
				fprintf(stderr,"dbg5       nav[%d]: %f %f %f %f %f %f %f\n",
					nnav,ntime[nnav],nlon[nnav],nlat[nnav],
						nheading[nnav],nsonardepth[nnav],
						nroll[nnav],npitch[nnav]);
				}
			else if (verbose >= 5)
				{
				fprintf(stderr,"\ndbg5  Error parsing line in navigation file in program <%s>\n",program_name);
				fprintf(stderr,"dbg5       line: %s\n",buffer);
				}

			/* check for reverses or repeats in time */
			if (nav_ok == MB_YES)
				{
				if (nnav == 0)
					nnav++;
				else if (ntime[nnav] > ntime[nnav-1])
					nnav++;
				else if (nnav > 0 && ntime[nnav] <= ntime[nnav-1] 
					&& verbose >= 5)
					{
					fprintf(stderr,"\ndbg5  Navigation time error in program <%s>\n",program_name);
					fprintf(stderr,"dbg5       nav[%d]: %f %f %f\n",
						nnav-1,ntime[nnav-1],nlon[nnav-1],
						nlat[nnav-1]);
					fprintf(stderr,"dbg5       nav[%d]: %f %f %f\n",
						nnav,ntime[nnav],nlon[nnav],
						nlat[nnav]);
				}
				}
			strncpy(buffer,"\0",sizeof(buffer));
			}

		/* check for nav */
		if (nnav < 2)
			{
			fprintf(stderr,"\nNo navigation read from file <%s>\n",navfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		/* get start and finish times of nav */
		mb_get_date(verbose,ntime[0],stime_i);
		mb_get_date(verbose,ntime[nnav-1],ftime_i);

		/* give the statistics */
		if (verbose >= 1)
			{
			fprintf(stderr,"\n%d navigation records read\n",nnav);
			fprintf(stderr,"Nav start time: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
				stime_i[0],stime_i[1],stime_i[2],stime_i[3],
				stime_i[4],stime_i[5],stime_i[6]);
			fprintf(stderr,"Nav end time:   %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
				ftime_i[0],ftime_i[1],ftime_i[2],ftime_i[3],
				ftime_i[4],ftime_i[5],ftime_i[6]);
			}
		}

	/* initialize reading the input SM2000 mpb file */
	if ((mpbfp = fopen(mpbfile, "r")) == NULL) 
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to Open SM2000 mpb File <%s> for reading\n",mpbfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* initialize writing the output swath sonar file */
	if ((status = mb_write_init(
		verbose,gsffile,MBF_GSFGENMB,&ombio_ptr,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_write_init>:\n%s\n",message);
		fprintf(stderr,"\nSwath Sonar File <%s> not initialized for writing\n",gsffile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* write comments to beginning of output file */
	kind = MB_DATA_COMMENT;
	sprintf(comment,"SM2000 multibeam bathymetry translated to GSF by program %s version %s",
		program_name,rcs_id);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	sprintf(comment,"MB-system Version %s",MB_VERSION);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(date,"\0",25);
	right_now = time((time_t *)0);
	strncpy(date,ctime(&right_now),24);
	if ((user_ptr = getenv("USER")) == NULL)
		user_ptr = getenv("LOGNAME");
	if (user_ptr != NULL)
		strcpy(user,user_ptr);
	else
		strcpy(user, "unknown");
	gethostname(host,128);
	sprintf(comment,"Run by user <%s> on cpu <%s> at <%s>",
		user,host,date);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	sprintf(comment,"Control Parameters:");
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	sprintf(comment,"  Input SM2000 mpb file:    %s",mpbfile);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	sprintf(comment,"  Output GSF file:          %s",gsffile);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	sprintf(comment,"  Navigation/Attitude file: %s",navfile);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	sprintf(comment," ");
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;

	/* read and write */
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* read some data */
		error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
				
		/* read the next header */
		read_len = fread(buffer,1,SM2000_HEADER_SIZE,mpbfp);
		
		if (read_len == SM2000_HEADER_SIZE)
			{
			mb_get_binary_short(swap, &buffer[0], &short_val); 
				ping.sync1 = (int) (unsigned short) short_val;
			mb_get_binary_short(swap, &buffer[2], &short_val); 
				ping.sync2 = (int) (unsigned short) short_val;
			mb_get_binary_short(swap, &buffer[4], &short_val); 
				ping.type = (int) short_val;
			ping.year = (mb_u_char) buffer[6];
			ping.hour = (mb_u_char) buffer[7];
			ping.minute = (mb_u_char) buffer[8];
			ping.second = (mb_u_char) buffer[9];
			mb_get_binary_short(swap, &buffer[10], &short_val); 
				ping.ping_number = (int) short_val;
			mb_get_binary_short(swap, &buffer[12], &short_val); 
				ping.agc = (int) short_val;
			ping.month = buffer[14];
			mb_get_binary_int(swap, &buffer[15], &ping.alt_time); 
			ping.day = (mb_u_char) buffer[19];
			mb_get_binary_short(swap, &buffer[20], &short_val); 
				ping.sound_speed = (int) short_val;
			mb_get_binary_short(swap, &buffer[22], &short_val); 
				ping.time_latency = (int) short_val;
			mb_get_binary_short(swap, &buffer[24], &short_val); 
				ping.sample_rate = (int) short_val;
			ping.swath_width = (mb_u_char) buffer[26];
			ping.num_beams_tot = (mb_u_char) buffer[27];
			ping.start_beam = (mb_u_char) buffer[28];
			ping.num_beams = (mb_u_char) buffer[29];
			ping.num_returns = (mb_u_char) buffer[30];
			ping.beam_width = (mb_u_char) buffer[31];
			mb_get_binary_int(swap, &buffer[32], &ping.sonar_range); 
			}
		else
			{
			status = MB_FAILURE;
			error = MB_ERROR_EOF;
			}
				
		/* read the beam data */
		read_len = fread(buffer,1,ping.num_beams_tot * SM2000_BEAM_SIZE,mpbfp);
		index = 0;
		for (j=0;j<ping.num_returns;j++)
		for (i=0;i<ping.num_beams_tot;i++)
			{
			ping.returns[j].quality[i] = (int) (((mb_u_char)buffer[index]) >> 6);
			mb_get_binary_short(swap, &buffer[index], &short_val); 
				ping.returns[j].range[i] = (int) (short_val & 0x3FFF);
			ping.returns[j].amplitude[i] = 0;
			index += SM2000_BEAM_SIZE;
			}
			
		/* output some debug info */
		if (status == MB_SUCCESS && verbose >= 0)
			{
			fprintf(stderr,"dbg2  SM2000 Header Values Read:\n");
			fprintf(stderr,"dbg2       sync1:           %d\n",ping.sync1);
			fprintf(stderr,"dbg2       sync2:           %d\n",ping.sync2);
			fprintf(stderr,"dbg2       type:            %d\n",ping.type);
			fprintf(stderr,"dbg2       year:            %d\n",ping.year);
			fprintf(stderr,"dbg2       month:           %d\n",ping.month);
			fprintf(stderr,"dbg2       day:             %d\n",ping.day);
			fprintf(stderr,"dbg2       hour:            %d\n",ping.hour);
			fprintf(stderr,"dbg2       minute:          %d\n",ping.minute);
			fprintf(stderr,"dbg2       second:          %d\n",ping.second);
			fprintf(stderr,"dbg2       alt_time:        %d\n",ping.alt_time);
			fprintf(stderr,"dbg2       agc:             %d\n",ping.agc);
			fprintf(stderr,"dbg2       sound_speed:     %d\n",ping.sound_speed);
			fprintf(stderr,"dbg2       time_latency:    %d\n",ping.time_latency);
			fprintf(stderr,"dbg2       sample_rate:     %d\n",ping.sample_rate);
			fprintf(stderr,"dbg2       swath_width:     %d\n",ping.swath_width);
			fprintf(stderr,"dbg2       num_beams_tot:   %d\n",ping.num_beams_tot);
			fprintf(stderr,"dbg2       start_beam:      %d\n",ping.start_beam);
			fprintf(stderr,"dbg2       num_beams:       %d\n",ping.num_beams);
			fprintf(stderr,"dbg2       num_returns:     %d\n",ping.num_returns);
			fprintf(stderr,"dbg2       beam_width:      %d\n",ping.beam_width);
			fprintf(stderr,"dbg2       sonar_range:     %d\n",ping.sonar_range);
			for (j=0;j<ping.num_returns;j++)
			for (i=0;i<ping.num_beams;i++)
				{
				fprintf(stderr,"dbg2      return %d beam %d   quality:%d range:%d  amplitude:%d\n",
					j,i,ping.returns[j].quality[i],
					ping.returns[j].range[i],
					ping.returns[j].amplitude[i]);
				}
			}
		/* interpolate the navigation and attitude */
		if (error == MB_ERROR_NO_ERROR && nnav > 2)
			{
			intstat = mb_linear_interp(verbose, 
				    ntime-1, nlon-1,
				    nnav, time_d, &navlon, &itime, 
				    &error);
			intstat = mb_linear_interp(verbose, 
				    ntime-1, nlat-1,
				    nnav, time_d, &navlat, &itime, 
				    &error);
			intstat = mb_linear_interp(verbose, 
				    ntime-1, nlat-1,
				    nnav, time_d, &heading, &itime, 
				    &error);
			intstat = mb_linear_interp(verbose, 
				    ntime-1, nlat-1,
				    nnav, time_d, &sonardepth, &itime, 
				    &error);
			intstat = mb_linear_interp(verbose, 
				    ntime-1, nlat-1,
				    nnav, time_d, &roll, &itime, 
				    &error);
			intstat = mb_linear_interp(verbose, 
				    ntime-1, nlat-1,
				    nnav, time_d, &pitch, &itime, 
				    &error);
			}

		/* calculate speed */
		if (error == MB_ERROR_NO_ERROR && nnav > 2)
			{
			if (itime == nnav - 1)
				itime = nnav - 2;
			if (itime == 0) 
				itime = 1;
			mb_coor_scale(verbose,nlat[itime-1],&mtodeglon,&mtodeglat);
			del_time = ntime[itime] - ntime[itime-1];
			dx = (nlon[itime] - nlon[itime-1])/mtodeglon;
			dy = (nlat[itime] - nlat[itime-1])/mtodeglat;
			dist = sqrt(dx*dx + dy*dy);
			if (del_time > 0.0)
				speed = 3.6*dist/del_time;
			else
				speed = 0.0;
			}

		/* calculate bathymetry */
		if (error <= MB_ERROR_NO_ERROR 
			&& kind == MB_DATA_DATA)
			{
			/* increment counter */
			idata++;
			
			/* fix some occasionally missing values */
			if (ping.sound_speed == 0)
				ping.sound_speed = 1500;
			if (ping.sample_rate == 0)
				ping.sample_rate = 11428;
			if (ping.swath_width == 0)
				ping.swath_width = 120;
			if (ping.beam_width == 0)
				ping.beam_width = 15;
			
			/* calculate travel times and angles */
			soundspeed = 0.001 * (double) ping.sound_speed;
			for (j=0;j<ping.num_returns;j++)
			for (i=0;i<ping.num_beams;i++)
				{
				ping.returns[j].angles[i] = (180.0 - (double)ping.swath_width) / 2.0
								+ ((double)ping.swath_width) 
									/ ((double)ping.num_beams - 1.0)
									* (ping.start_beam + i);
				ping.returns[j].traveltimes[i] = ((double)ping.returns[j].range[i]) 
									/ ((double)ping.sample_rate);
				alpha = pitch;
				beta = (180.0 - (double)ping.swath_width) / 2.0
						+ ((double)ping.swath_width) / ((double)ping.num_beams - 1.0)
									* (ping.start_beam + i)
						+ roll;
				mb_rollpitch_to_takeoff(verbose, alpha, beta, &theta, &phi, &error);
				if (phi < 0.0)
					phi += 360.0;
				if (phi > 360.0)
					phi -= 360.0;
				ping.returns[j].angles[i] = theta;
				ping.returns[j].angles_forward[i] = phi;
				rr = 0.5 * ping.returns[j].traveltimes[i] * ((double)ping.sound_speed);
				xx = rr * sin(DTR * theta);
				zz = rr * cos(DTR * theta);
				
				
				ping.returns[j].beamflag[i] = MB_FLAG_NONE;
				ping.returns[j].bath[i] = zz + sonardepth;
				ping.returns[j].bathacrosstrack[i] = xx * cos(DTR * phi);
				ping.returns[j].bathalongtrack[i] = xx * sin(DTR * phi);
				}			
			}

		/* give error message */
		if ((verbose >= 1 && error == MB_ERROR_NO_ERROR 
			&& (kind == MB_DATA_DATA
			    || kind == MB_DATA_NAV)
			&& (time_d < ntime[0] 
			    || time_d > ntime[nnav-1])))
			{
			fprintf(stderr,"\nNavigation extrapolated!\n");
			fprintf(stderr,"Data time lies outside the bounds of the input navigation...\n");
			fprintf(stderr,"Data time: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
				time_i[0],time_i[1],time_i[2],time_i[3],
				time_i[4],time_i[5],time_i[6]);
			}

		/* write some data */
		if (error == MB_ERROR_NO_ERROR)
			{
			/* loop over each individual return */
			for (j=0;j<ping.num_returns;j++)
				{
				status = mb_put_all(verbose,ombio_ptr,
						store_ptr,MB_YES,kind,
						time_i,time_d,
						navlon,navlat,speed,heading,
						ping.num_beams, 0, 0,
						ping.returns[j].beamflag,
						ping.returns[j].bath,
						ping.returns[j].amp,
						ping.returns[j].bathacrosstrack,
						ping.returns[j].bathalongtrack,
						NULL, NULL, NULL,
						comment,&error);
				if (status == MB_SUCCESS)
					{
					if (kind == MB_DATA_DATA)
						odata++;
					else if (kind == MB_DATA_NAV)
						onav++;
					else if (kind == MB_DATA_COMMENT)
						ocomment++;
					}
				else if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error returned from function <mb_put_all>:\n%s\n",message);
					fprintf(stderr,"\nSwath Sonar Data Not Written To File <%s>\n",gsffile);
					fprintf(stderr,"Output Record: %d\n",odata+1);
					fprintf(stderr,"Time: %d %d %d %d %d %d %d\n",
						time_i[0],time_i[1],time_i[2],
						time_i[3],time_i[4],time_i[5],
						time_i[6]);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
						program_name);
					exit(error);
					}
				}
			}
		}

	/* close the files */
	fclose(mpbfp);
	status = mb_close(verbose,&ombio_ptr,&error);

	/* deallocate memory for data arrays */
	mb_freed(verbose,__FILE__,__LINE__,(void **)&ntime,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&nlon,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&nlat,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&nheading,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&nsonardepth,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&nroll,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&npitch,&error);

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* give the statistics */
	if (verbose >= 1)
		{
		fprintf(stderr,"\n%d input navigation records\n",nnav);
		fprintf(stderr,"%d input data records\n",idata);
		fprintf(stderr,"%d output ping records\n",odata);
		fprintf(stderr,"%d output comment records\n",ocomment);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/

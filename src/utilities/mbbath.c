/*--------------------------------------------------------------------
 *    The MB-system:	mbbath.c	3/31/93
 *    $Id: mbbath.c,v 4.4 1994-07-29 19:02:56 caress Exp $
 *
 *    Copyright (c) 1993, 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBBATH is a tool for processing multibeam data.  This program
 * calculates bathymetry from the travel time data by raytracing
 * through a layered water velocity model.
 * The default input and output streams are stdin and stdout.
 *
 * Author:	D. W. Caress
 * Date:	March 31, 1993
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.3  1994/04/12  18:58:00  caress
 * Fixed fprintf statements on lines 689 and 691.
 *
 * Revision 4.2  1994/04/12  17:01:29  caress
 * First cut at mbbath.  Will need to deal with alongtrack angles
 * as well as acrosstrack angles.
 *
 * Revision 4.1  1994/03/12  01:44:37  caress
 * Added declarations of ctime and/or getenv for compatability
 * with SGI compilers.
 *
 * Revision 4.0  1994/03/06  00:13:22  caress
 * First cut at version 4.0
 *
 * Revision 4.0  1994/03/01  18:59:27  caress
 * First cut at new version. Any changes are associated with
 * support of three data types (beam bathymetry, beam amplitude,
 * and sidescan) instead of two (bathymetry and backscatter).
 *
 * Revision 3.6  1993/11/05  18:13:20  caress
 * Fixed some minor details, e.g. updated the comments written
 * into the output data stream, and updated the manual page.
 *
 * Revision 3.5  1993/11/03  21:17:12  caress
 * Added a capability to read in and apply static corrections
 * as a function of beam number.
 * Also fixed a few typos in comment lines.
 *
 * Revision 3.4  1993/08/16  22:22:25  caress
 * Don't know what the changes are exactly.
 *
 * Revision 3.3  1993/06/21  01:25:06  caress
 * Added ability to read in and use roll corrections.
 *
 * Revision 3.2  1993/06/09  08:38:09  caress
 * Changed way takeoff angles of beams are calculated. Now the
 * beam angle spacing is taken from mbsys_hsds.h file. The
 * Hydrosweep documentation gives the beam spacing as 1.525 degrees
 * but a close examination of some data (6/9/93) shows that Hydrosweep
 * must use a value more like 1.510 degrees internally.
 * Also removed velocity bias option as the creation of HSVELOCITYTOOL
 * obviates the need for that option.
 *
 * Revision 3.1  1993/05/25  04:55:58  caress
 * Added velocity bias option.
 *
 * Revision 3.0  1993/05/04  22:26:11  dale
 * Initial version
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <strings.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mbsys_hsds.h"

/* DTR define */
#ifndef M_PI
#define	M_PI	3.14159265358979323846
#endif
#define DTR	(M_PI/180.)

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
	/* id variables */
	static char rcs_id[] = "$Id: mbbath.c,v 4.4 1994-07-29 19:02:56 caress Exp $";
	static char program_name[] = "MBBATH";
	static char help_message[] =  "MBBATH calculates bathymetry from \
the travel time data by raytracing \nthrough a layered water velocity \
model. The depths may be saved as \ncalculated by raytracing (corrected \
meters) or adjusted as if the \nvertical water velocity is 1500 m/s \
(uncorrected meters). The default \ninput and output streams are stdin \
and stdout.";
	static char usage_message[] = "mbbath [-Aangle -Brollbias \
-Ddraught -Fformat  \n\t-Iinfile -Ooutfile -Ppitch_bias -Rrollfile \
\n\t-Sstaticfile -U -Wvelfile -V -H]";

	/* parsing variables */
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

	/* MBIO read and write control parameters */
	int	format = 0;
	int	format_num;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[6];
	int	etime_i[6];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	char	ifile[128];
	char	*imbio_ptr;
	char	ofile[128];
	char	*ombio_ptr;

	/* mbio read and write values */
	char	*store_ptr;
	int	kind;
	int	time_i[6];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	int	nbath;
	int	namp;
	int	nss;
	int	*bath;
	int	*bathacrosstrack;
	int	*bathalongtrack;
	int	*amp;
	int	*ss;
	int	*ssacrosstrack;
	int	*ssalongtrack;
	int	idata = 0;
	int	icomment = 0;
	int	odata = 0;
	int	ocomment = 0;
	char	comment[256];

	/* time, user, host variables */
	long int	right_now;
	char	date[25], user[128], host[128];

	/* velocity profile handling variables */
	char	vfile[128];
	double	roll_bias;
	double	pitch_bias;
	double	dangle;
	double	draught;
	int	uncorrected;
	FILE	*vfp;
	int	nvel;
	double	*vel;
	double	*velraw;
	double	*dep;
	double	*vsum;

	/* roll error correction handling variables */
	char	rfile[128];
	int	nroll;
	double	*roll_time;
	double	*roll_corr;
	double	roll_correction;

	/* static bathymetry correction handling variables */
	char	sfile[128];
	int	nbath_corr;
	int	*bath_corr;

	/* survey and calibrate ping raytracing arrays */
	double	*s_angle;
	double	*s_p;
	double	**s_ttime_tab;
	double	**s_dist_tab;
	double	*c_angle;
	double	*c_p;
	double	**c_ttime_tab;
	double	**c_dist_tab;
	double	*ttime;
	double	*dist;
	double	*ttimes;
	double	*angles;
	int	*flags;

	char	buffer[128], tmp[128], *result;
	int	size;
	double	dummy;
	double	dr, dx;
	int	nbeams;
	int	center_beam;
	double	tt, factor, zz, xx, vavg;
	int	setup;
	int	i, j, k, l, m;

	char	*ctime();
	char	*getenv();

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* reset all defaults */
	format = MBF_HSATLRAW;
	pings = 1;
	lonflip = 0;
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
	etime_i[0] = 2062;
	etime_i[1] = 2;
	etime_i[2] = 21;
	etime_i[3] = 10;
	etime_i[4] = 30;
	etime_i[5] = 0;
	speedmin = 0.0;
	timegap = 1000000000.0;

	/* set default input and output */
	strcpy (ifile, "stdin");
	strcpy (ofile, "stdout");
	strcpy (vfile, "\0");
	strcpy (rfile, "\0");
	strcpy (sfile, "\0");

	/* set default control parameters */
	roll_bias = 0.0;
	pitch_bias = 0.0;
	dangle = 0.0;
	draught = 5.5; /* set for R/V Ewing data */
	uncorrected = MB_NO;

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhA:a:B:b:D:d:F:f:I:i:O:o:P:p:R:r:S:s:UuW:w:")) != -1)
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
			sscanf (optarg,"%lf", &dangle);
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			flag++;
			break;
		case 'W':
		case 'w':
			sscanf (optarg,"%s", vfile);
			flag++;
			break;
		case 'B':
		case 'b':
			sscanf (optarg,"%lf", &roll_bias);
			flag++;
			break;
		case 'R':
		case 'r':
			sscanf (optarg,"%s", rfile);
			flag++;
			break;
		case 'S':
		case 's':
			sscanf (optarg,"%s", sfile);
			flag++;
			break;
		case 'P':
		case 'p':
			sscanf (optarg,"%lf", &pitch_bias);
			flag++;
			break;
		case 'D':
		case 'd':
			sscanf (optarg,"%lf", &draught);
			flag++;
			break;
		case 'U':
		case 'u':
			uncorrected = MB_YES;
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", ifile);
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", ofile);
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
		exit(MB_FAILURE);
		}

	/* print starting message */
	if (verbose == 1)
		{
		fprintf(stderr,"\nProgram %s\n",program_name);
		fprintf(stderr,"Version %s\n",rcs_id);
		fprintf(stderr,"MB-System Version %s\n",MB_VERSION);
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
		fprintf(stderr,"dbg2       pings:           %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:         %d\n",lonflip);
		fprintf(stderr,"dbg2       bounds[0]:       %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:       %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:       %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:       %f\n",bounds[3]);
		fprintf(stderr,"dbg2       btime_i[0]:      %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]:      %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]:      %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]:      %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]:      %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]:      %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       etime_i[0]:      %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:      %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:      %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:      %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:      %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:      %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       speedmin:        %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:         %f\n",timegap);
		fprintf(stderr,"dbg2       input file:      %s\n",ifile);
		fprintf(stderr,"dbg2       output file:     %s\n",ofile);
		fprintf(stderr,"dbg2       velocity file:   %s\n",vfile);
		fprintf(stderr,"dbg2       roll bias:       %f\n",roll_bias);
		fprintf(stderr,"dbg2       pitch bias:      %f\n",pitch_bias);
		fprintf(stderr,"dbg2       beam angle:      %f\n",dangle);
		fprintf(stderr,"dbg2       draught:         %f\n",draught);
		fprintf(stderr,"dbg2       roll file:       %s\n",rfile);
		fprintf(stderr,"dbg2       statics file:    %s\n",sfile);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(MB_ERROR_NO_ERROR);
		}

	/* check for format with travel time data */
	status = mb_format(verbose,&format,&format_num,&error);
	if (mb_traveltime_table[format_num] != MB_YES)
		{
		fprintf(stderr,"\nProgram <%s> requires travel time data.\n",program_name);
		fprintf(stderr,"Format %d is unacceptable because it does not inlude travel time data.\n",format);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_ERROR_BAD_FORMAT);
		}

	/* if a velocity profile file has been specified then
		count the velocity points */
	if (strlen(vfile) > 0)
		{
		nvel = 0;
		if ((vfp = fopen(vfile, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to Open Velocity Profile File <%s> for reading\n",vfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		while ((result = fgets(buffer,128,vfp)) == buffer)
			if (buffer[0] != '#')
				nvel++;
		fclose(vfp);
		}		
	/* else assume a 1500 m/s half space */
	else
		nvel = 2;

	/* allocate space for the velocity profile and raytracing tables */
	size = (nvel+1)*sizeof(double);
	status = mb_malloc(verbose,size,&vel,&error);
	status = mb_malloc(verbose,size,&velraw,&error);
	status = mb_malloc(verbose,size,&dep,&error);
	status = mb_malloc(verbose,size,&vsum,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* if a velocity profile file has been specified then
		read in velocity profile data */
	if (strlen(vfile) > 0)
		{
		nvel = 0;
		if ((vfp = fopen(vfile, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to Open Velocity Profile File <%s> for reading\n",vfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		strncpy(buffer,"\0",sizeof(buffer));
		while ((result = fgets(buffer,128,vfp)) == buffer)
		  {
		  if (buffer[0] != '#')
			{
			sscanf(buffer,"%lf %lf",&dep[nvel],&velraw[nvel]);

			/* output some debug values */
			if (verbose >= 5)
				{
				fprintf(stderr,"\ndbg5  New velocity value read in program <%s>\n",program_name);
				fprintf(stderr,"dbg5       dep[%d]: %f  vel[%d]: %f\n",
					nvel,dep[nvel],nvel,velraw[nvel]);
				}
			nvel++;
			}
		  strncpy(buffer,"\0",sizeof(buffer));
		  }
		}
	/* else assume a 1500 m/s half space */
	else
		{
		nvel = 2;
		velraw[0] = 1500.0;
		velraw[1] = 1500.0;
		dep[0] = 0.0;
		dep[1] = 12000.0;
		}

	/* if velocity profile doesn't extend to 12000 m depth
		extend it to that depth */
	if (dep[nvel-1] < 12000.0)
		{
		dep[nvel] = 12000.0;
		velraw[nvel] = velraw[nvel-1];
		nvel++;
		}

	/* construct layered velocity model from discrete model */
	for (i=0;i<nvel-1;i++)
		{
		vel[i] = 0.5*(velraw[i] + velraw[i+1]);
		}
	vel[nvel-1] = 0.0;

	/* output some debug values */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Discrete and layered velocity models in program <%s>\n",program_name);
		for (i=0;i<nvel;i++)
			fprintf(stderr,"dbg5       %d  depth:%f  raw vel:%f  layer vel:%f\n",
					i,dep[i],velraw[i],vel[i]);
		}

	/* if a roll correction file has been specified then
		count the roll correction points */
	if (strlen(rfile) > 0)
		{
		nroll = 0;
		if ((vfp = fopen(rfile, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to Open Roll Correction File <%s> for reading\n",vfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		while ((result = fgets(buffer,128,vfp)) == buffer)
			{
			if (buffer[0] != '#')
				nroll++;
			}
		fclose(vfp);
		}

	/* allocate space for the roll correction arrays */
	status = mb_malloc(verbose,nroll*sizeof(double),&roll_time,&error);
	status = mb_malloc(verbose,nroll*sizeof(double),&roll_corr,&error);

	/* if a roll correction file has been specified then
		read in roll correction data */
	if (strlen(rfile) > 0)
		{
		nroll = 0;
		if ((vfp = fopen(rfile, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to Open Roll Correction File <%s> for reading\n",vfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		strncpy(buffer,"\0",sizeof(buffer));
		while ((result = fgets(buffer,128,vfp)) == buffer)
		  {
		  if (buffer[0] != '#')
			{
			sscanf(buffer,"%lf %lf",&roll_time[nroll],&roll_corr[nroll]);

			/* output some debug values */
			if (verbose >= 5)
				{
				fprintf(stderr,"\ndbg5  New roll correction value read in program <%s>\n",program_name);
				fprintf(stderr,"dbg5       time[%d]: %f  roll[%d]: %f\n",
					nroll,roll_time[nroll],nroll,roll_corr[nroll]);
				}
			nroll++;
			}
		  strncpy(buffer,"\0",sizeof(buffer));
		  }
		fclose(vfp);
		}
	if (verbose > 0 && nroll > 0)
		fprintf(stderr,"\n%d roll correction records read\n",nroll);

	/* calculate velocity sums for uncorrecting depths */
	vsum[0] = 0.0;
	for (i=1;i<nvel;i++)
		vsum[i] = vsum[i-1] + vel[i-1]*(dep[i] - dep[i-1]);

	/* initialize reading the input multibeam file */
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

	/* initialize writing the output multibeam file */
	if ((status = mb_write_init(
		verbose,ofile,format,&ombio_ptr,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_write_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for writing\n",ofile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* allocate memory for data arrays */
	status = mb_malloc(verbose,beams_bath*sizeof(int),&bath,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(int),
				&bathacrosstrack,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(int),
				&bathalongtrack,&error);
	status = mb_malloc(verbose,beams_amp*sizeof(int),&amp,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(int),&ss,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(int),&ssacrosstrack,
				&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(int),&ssalongtrack,
				&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),&s_angle,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),&s_p,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),&c_angle,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),&c_p,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double *),
				&s_ttime_tab,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double *),
				&s_dist_tab,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double *),
				&c_ttime_tab,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double *),
				&c_dist_tab,&error);
	size = (nvel+1)*sizeof(double);
	for (i=0;i<beams_bath;i++)
		{
		status = mb_malloc(verbose,size,&s_ttime_tab[i],&error);
		status = mb_malloc(verbose,size,&s_dist_tab[i],&error);
		status = mb_malloc(verbose,size,&c_ttime_tab[i],&error);
		status = mb_malloc(verbose,size,&c_dist_tab[i],&error);
		}
	status = mb_malloc(verbose,beams_bath*sizeof(double),&ttimes,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),&angles,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(int),&flags,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* if specified get static beam depth corrections */
	if (strlen(sfile) > 0)
		{
		if ((vfp = fopen(sfile, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to Open Static Beam Correction File <%s> for reading\n",sfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		status = mb_malloc(verbose,beams_bath*sizeof(int),&bath_corr,&error);
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,message);
			fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		nbath_corr = 0;
		while (nbath_corr < beams_bath &&
			(result = fgets(buffer,128,vfp)) == buffer)
			{
			if (buffer[0] != '#')
				{
				sscanf(buffer,"%d %lf",&i,&dummy);
				bath_corr[nbath_corr] = (int) dummy;
				}
			nbath_corr++;
			}
		if (nbath_corr != beams_bath)
			{
			fprintf(stderr,"\nRequire %d static beam depth corrections but only read %d from <%s>\n",
				beams_bath,nbath_corr,sfile);
			fprintf(stderr,"No static beam depth corrections applied to data\n");
			nbath_corr = 0;
			}
		fclose(vfp);
		}

	/* write comments to beginning of output file */
	kind = MB_DATA_COMMENT;
	strncpy(comment,"\0",256);
	sprintf(comment,"Bathymetry data generated by program %s",program_name);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"Version %s",rcs_id);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"MB-system Version %s",MB_VERSION);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	right_now = time((long *)0);
	strncpy(date,"\0",25);
	right_now = time((long *)0);
	strncpy(date,ctime(&right_now),24);
	strcpy(user,getenv("USER"));
	gethostname(host,128);
	strncpy(comment,"\0",256);
	sprintf(comment,"Run by user <%s> on cpu <%s> at <%s>",
		user,host,date);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;

	strncpy(comment,"\0",256);
	sprintf(comment,"Depths and crosstrack distances calculated from travel times");
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  by raytracing through a water velocity profile specified");
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  by the user.  The depths have been saved in units of");
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	if (uncorrected)
		sprintf(comment,"  uncorrected meters (the depth values are adjusted to be");
	else
		sprintf(comment,"  corrected meters (the depth values obtained by");
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	if (uncorrected)
		sprintf(comment,"  consistent with a vertical water velocity of 1500 m/s).");
	else
		sprintf(comment,"  raytracing are not adjusted further).");
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"Control Parameters:");
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  MBIO data format:   %d",format);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  Input file:         %s",ifile);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  Output file:        %s",ofile);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  Velocity file:      %s",vfile);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;

	strncpy(comment,"\0",256);
	sprintf(comment,"  Input nodal water velocity profile:");
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"    depth (m)   velocity (m/s)");
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	for (i=0;i<nvel-1;i++)
		{
		strncpy(comment,"\0",256);
		sprintf(comment,"     %10.2f     %10.2f",
			dep[i],velraw[i]);
		status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}

	strncpy(comment,"\0",256);
	sprintf(comment,"  Water velocity profile used for raytracing:");
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"    layer   top and bottom depths (m)   velocity (m/s)");
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	for (i=0;i<nvel-1;i++)
		{
		strncpy(comment,"\0",256);
		sprintf(comment,"     %2d    %10.2f   %10.2f     %10.2f",
			i,dep[i],dep[i+1],vel[i]);
		status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}

	strncpy(comment,"\0",256);
	sprintf(comment,"  Roll bias:    %f degrees (starboard: -, port: +)",
		roll_bias);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  Pitch bias:   %f degrees (aft: -, forward: +)",
		pitch_bias);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  Draught:      %f meters",
		draught);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  Roll correction file:      %s",rfile);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  Static depth correction file:      %s",sfile);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	if (nbath_corr == beams_bath)
		{
		strncpy(comment,"\0",256);
		sprintf(comment,"  Static beam depth corrections:");
		status = mb_put(verbose,ombio_ptr,kind,
				time_i,time_d,
				navlon,navlat,speed,heading,
				beams_bath,beams_amp,pixels_ss,
				bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		for (i=0;i<beams_bath;i++)
			{
			strncpy(comment,"\0",256);
			sprintf(comment,"    %2d  %5d",i,bath_corr[i]);
			status = mb_put(verbose,ombio_ptr,kind,
					time_i,time_d,
					navlon,navlat,speed,heading,
					beams_bath,beams_amp,pixels_ss,
					bath,amp,bathacrosstrack,bathalongtrack,
					ss,ssacrosstrack,ssalongtrack,
					comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		}
	strncpy(comment,"\0",256);
	sprintf(comment," ");
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;

	/* read and write */
	setup = MB_NO;
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* read some data */
		error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		status = mb_get_all(verbose,imbio_ptr,&store_ptr,&kind,
				time_i,&time_d,&navlon,&navlat,&speed,
				&heading,&distance,
				&nbath,&namp,&nss,
				bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,&error);

		/* increment counter */
		if (error <= MB_ERROR_NO_ERROR 
			&& kind == MB_DATA_DATA)
			idata = idata + pings;
		else if (error <= MB_ERROR_NO_ERROR 
			&& kind == MB_DATA_COMMENT)
			icomment++;

		/* time gaps do not matter to mbbath */
		if (error == MB_ERROR_TIME_GAP)
			{
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* non-survey data do not matter to mbbath */
		if (error == MB_ERROR_OTHER)
			{
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* output error messages */
		if (verbose >= 1 && error == MB_ERROR_COMMENT)
			{
			if (icomment == 1)
				fprintf(stderr,"\nComments in Input:\n");
			fprintf(stderr,"%s\n",comment);
			}
		else if (verbose >= 1 && error < MB_ERROR_NO_ERROR
			&& error > MB_ERROR_OTHER)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nNonfatal MBIO Error:\n%s\n",message);
			fprintf(stderr,"Input Record: %d\n",idata);
			fprintf(stderr,"Time: %d %d %d %d %d %d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5]);
			}
		else if (verbose >= 1 && error < MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nNonfatal MBIO Error:\n%s\n",message);
			fprintf(stderr,"Input Record: %d\n",idata);
			}
		else if (verbose >= 1 && error != MB_ERROR_NO_ERROR 
			&& error != MB_ERROR_EOF)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nFatal MBIO Error:\n%s\n",message);
			fprintf(stderr,"Last Good Time: %d %d %d %d %d %d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5]);
			}

		/* if survey or calibrate data encountered, 
			get the bathymetry */
		if (error == MB_ERROR_NO_ERROR
			&& (kind == MB_DATA_DATA 
			|| kind == MB_DATA_CALIBRATE))
			{
			/* if needed do setup */
			if (setup != MB_YES )
				{
				/* set up the raytracing for 
					both survey and calibrate pings */
				status = setup_raytracing(verbose,
					imbio_ptr,store_ptr,
					nbeams,ttimes,angles,flags,
					roll_bias,dangle,
					nvel,vel,dep,
					s_angle,s_p,s_ttime_tab,s_dist_tab,
					&error);
				status = setup_raytracing(verbose,
					imbio_ptr,store_ptr,
					nbeams,ttimes,angles,flags,
					pitch_bias,dangle,
					nvel,vel,dep,
					c_angle,c_p,c_ttime_tab,c_dist_tab,
					&error);
				setup = MB_YES;
				}

			/* if needed get roll correction */
			if (nroll > 0 && kind == MB_DATA_DATA)
				{
				status = get_roll_correction(verbose,
					nroll,roll_time,roll_corr,
					time_d,&roll_correction,&error);

				/* set up the raytracing for survey pings */
				status = setup_raytracing(verbose,
					imbio_ptr,store_ptr,
					nbeams,ttimes,angles,flags,
					roll_bias+roll_correction,dangle,
					nvel,vel,dep,
					s_angle,s_p,s_ttime_tab,s_dist_tab,
					&error);
				}

			/* extract travel times */
			status = mb_ttimes(verbose,imbio_ptr,
				store_ptr,&kind,&nbeams,
				ttimes,angles,flags,&error);

			/* loop over the beams */
			for (i=0;i<beams_bath;i++)
			  {
			  /* use the right raytracing tables */
			  if (kind == MB_DATA_DATA)
				{
				ttime = s_ttime_tab[i];
				dist = s_dist_tab[i];
				}
			  else
				{
				ttime = c_ttime_tab[i];
				dist = c_dist_tab[i];
				}

			  /* calculate the depths 
				and crosstrack distances */ 
			  bath[i] = 0;
			  bathacrosstrack[i] = 0;
			  bathalongtrack[i] = 0;
			  if (ttimes[i] > 0.0)
				{
				for (j=0;j<nvel-1;j++)
				  if (ttimes[i] > ttime[j] 
					&& ttimes[i] <= ttime[j+1])
					{
					factor = (ttimes[i] - ttime[j])
						/(ttime[j+1] - ttime[j]);
					zz = dep[j] 
						+ factor*(dep[j+1] - dep[j])
						+ draught;
					xx = dist[j]
						+ factor*(dist[j+1] - dist[j]);
					if (uncorrected)
					  {
					  vavg = (vsum[j] 
						+ vel[j]*(zz - dep[j]))/zz;
					  zz = zz*1500./vavg;
					  }
					bathalongtrack[i] = (int) (xx + 0.5);
					bath[i] = (int) (zz + 0.5);
					if (nbath_corr == beams_bath)
						bath[i] -= bath_corr[i];
					if (flags[i] == MB_YES)
						bath[i] = -bath[i];

					/* output some debug messages */
					if (verbose >= 5)
						{
						fprintf(stderr,"\ndbg5  Depth value calculated in program <%s>:\n",program_name);
						fprintf(stderr,"dbg5       kind:  %d\n",kind);
						fprintf(stderr,"dbg5       beam:  %d\n",i);
						fprintf(stderr,"dbg5       tt:     %f\n",ttimes[i]);
						fprintf(stderr,"dbg5       ttime[%d]: %d\n",j,ttime[j]);
						fprintf(stderr,"dbg5       ttime[%d]: %d\n",j+1,ttime[j+1]);
						fprintf(stderr,"dbg5       factor: %f\n",factor);
						fprintf(stderr,"dbg5       xx:     %f\n",xx);
						fprintf(stderr,"dbg5       zz:     %f\n",zz);
						fprintf(stderr,"dbg5       vavg:   %f\n",vavg);
						fprintf(stderr,"dbg5       dist:   %d\n",bathalongtrack[i]);
						fprintf(stderr,"dbg5       depth:  %d\n",bath[i]);
						}
					}
				}
			  }

			/* output some debug messages */
			if (verbose >= 5)
				{
				fprintf(stderr,"\ndbg5  Depth values calculated in program <%s>:\n",program_name);
				fprintf(stderr,"dbg5       kind:  %d\n",kind);
				fprintf(stderr,"dbg5      beam    time      depth        dist\n");	
				for (i=0;i<MBSYS_HSDS_BEAMS;i++)
					fprintf(stderr,"dbg5       %2d   %6.0f   %6d   %6d\n",
						i,ttimes[i],
						bath[i],
						bathalongtrack[i]);
				}
			}

		/* write some data */
		if (error == MB_ERROR_NO_ERROR
			|| kind == MB_DATA_COMMENT)
			{
			status = mb_put_all(verbose,ombio_ptr,
					store_ptr,MB_NO,kind,
					time_i,time_d,
					navlon,navlat,speed,heading,
					beams_bath,beams_amp,pixels_ss,
					bath,amp,bathacrosstrack,bathalongtrack,
					ss,ssacrosstrack,ssalongtrack,
					comment,&error);
			if (status == MB_SUCCESS)
				{
				if (kind == MB_DATA_DATA)
					odata++;
				else if (kind == MB_DATA_COMMENT)
					ocomment++;
				}
			else
				{
				mb_error(verbose,error,&message);
				fprintf(stderr,"\nMBIO Error returned from function <mb_put>:\n%s\n",message);
				fprintf(stderr,"\nMultibeam Data Not Written To File <%s>\n",ofile);
				fprintf(stderr,"Output Record: %d\n",odata+1);
				fprintf(stderr,"Time: %d %d %d %d %d %d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5]);
				fprintf(stderr,"\nProgram <%s> Terminated\n",
					program_name);
				exit(error);
				}
			}
		}

	/* close the files */
	status = mb_close(verbose,imbio_ptr,&error);
	status = mb_close(verbose,ombio_ptr,&error);

	/* deallocate memory for data arrays */
	mb_free(verbose,vel,&error);
	mb_free(verbose,dep,&error);
	mb_free(verbose,vsum,&error);
	mb_free(verbose,roll_time,&error);
	mb_free(verbose,roll_corr,&error);
	mb_free(verbose,bath_corr,&error);
	mb_free(verbose,s_angle,&error);
	mb_free(verbose,s_p,&error);
	mb_free(verbose,c_angle,&error);
	mb_free(verbose,c_p,&error);
	for (i=0;i<beams_bath;i++)
		{
		mb_free(verbose,s_ttime_tab[i],&error);
		mb_free(verbose,s_dist_tab[i],&error);
		mb_free(verbose,c_ttime_tab[i],&error);
		mb_free(verbose,c_dist_tab[i],&error);
		}
	mb_free(verbose,s_ttime_tab,&error);
	mb_free(verbose,s_dist_tab,&error);
	mb_free(verbose,c_ttime_tab,&error);
	mb_free(verbose,c_dist_tab,&error);
	mb_free(verbose,ttimes,&error);
	mb_free(verbose,angles,&error);
	mb_free(verbose,flags,&error);
	mb_free(verbose,bath,&error); 
	mb_free(verbose,bathacrosstrack,&error); 
	mb_free(verbose,bathalongtrack,&error); 
	mb_free(verbose,amp,&error); 
	mb_free(verbose,ss,&error); 
	mb_free(verbose,ssacrosstrack,&error); 
	mb_free(verbose,ssalongtrack,&error); 

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* give the statistics */
	if (verbose >= 1)
		{
		fprintf(stderr,"\n%d input data records\n",idata);
		fprintf(stderr,"%d input comment records\n",icomment);
		fprintf(stderr,"%d output data records\n",odata);
		fprintf(stderr,"%d output comment records\n",ocomment);
		}

	/* end it all */
	exit(status);
}
/*--------------------------------------------------------------------*/
int setup_raytracing(verbose,mbio_ptr,store_ptr,
	nbeams,ttimes,angles,flags,
	angle_bias,dangle,nvel,vel,dep,
	angle,p,ttime_tab,dist_tab,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	nbeams;
double	*ttimes;
double	*angles;
int	*flags;
double	angle_bias;
double	dangle;
int	nvel;
double	*dep;
double	*vel;
double	*angle;
double	*p;
double	**ttime_tab;
double	**dist_tab;
int	*error;
{
	char	*function_name = "setup_raytracing";
	int	status = MB_SUCCESS;
	double	*ttime;
	double	*dist;
	int	kind;
	int	center_beam;
	double	dr, dx;
	int	i, j;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		fprintf(stderr,"dbg2       ttimes:     %d\n",ttimes);
		fprintf(stderr,"dbg2       angles:     %d\n",angles);
		fprintf(stderr,"dbg2       flags:      %d\n",flags);
		fprintf(stderr,"dbg2       angle_bias: %f\n",angle_bias);
		fprintf(stderr,"dbg2       dangle:     %f\n",dangle);
		fprintf(stderr,"dbg2       nvel:       %d\n",nvel);
		for (i=0;i<nvel;i++)
			fprintf(stderr,"dbg2       dep[%d]:%f  vel[%d]:%f\n",
				i,dep[i],i,vel[i]);
		fprintf(stderr,"dbg2       angle:      %d\n",angle);
		fprintf(stderr,"dbg2       p:          %d\n",p);
		fprintf(stderr,"dbg2       ttime_tab:  %d\n",ttime_tab);
		fprintf(stderr,"dbg2       dist_tab:   %d\n",dist_tab);
		}

	/* set the angle increment between survey ping beams */
	if (dangle > 0.0)
		{
		center_beam = nbeams/2;
		for (i=0;i<nbeams;i++)
			{
			angle[i] = (i-center_beam)*dangle + angle_bias;
			p[i] = sin(DTR*angle[i])/vel[0];
			}
		}
	else
		status = mb_ttimes(verbose,mbio_ptr,
				store_ptr,&kind,&nbeams,
				ttimes,angles,flags,error);
	for (i=0;i<nbeams;i++)
		{
		angle[i] = angles[i];
		p[i] = sin(DTR*angle[i])/vel[0];
		}

	/* set up the raytracing tables for survey pings */
	for (i=0;i<nbeams;i++)
		{
		ttime = ttime_tab[i];
		dist = dist_tab[i];
		ttime[0] = 0.0;
		dist[0] = 0.0;
		for (j=0;j<nvel-1;j++)
			{
			dr = (dep[j+1] - dep[j])
				/sqrt(1. - p[i]*p[i]*vel[j]*vel[j]);
			dx = dr*p[i]*vel[j];
			ttime[j+1] = ttime[j] + 2.*dr/vel[j];
			dist[j+1] = dist[j] + dx;
			}

		/* output some debug values */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  Raytracing table created for survey beam %d in function <%s>:\n",i,function_name);
			fprintf(stderr,"dbg5       angle: %f\n",angle[i]);
			fprintf(stderr,"dbg5       p:     %f\n",p[i]);
			fprintf(stderr,"dbg5      beam    depth      vel        time      dist     vsum\n",j,dep[j],vel[j],ttime[j],dist[j]);
			for (j=0;j<nvel;j++)
				fprintf(stderr,"dbg5       %2d   %8.2f   %7.2f   %8.2f  %9.2f\n",j,dep[j],vel[j],ttime[j],dist[j]);
			}
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBATH function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int get_roll_correction(verbose,nroll,roll_time,roll_corr,time_d,
		roll_correction,error)
int	verbose;
int	nroll;
double	*roll_time;
double	*roll_corr;
double	time_d;
double	*roll_correction;
int	*error;
{
	char	*function_name = "get_roll_correction";
	int	status = MB_SUCCESS;
	int	iroll, found;
	int	i, j;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       nroll:      %d\n",nroll);
		fprintf(stderr,"dbg2       roll_time:  %d\n",roll_time);
		fprintf(stderr,"dbg2       roll_corr:  %d\n",roll_corr);
		fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
		}

	/* search for the current time */
	found = MB_NO;
	for (i=0;i<nroll-1;i++)
		if (time_d >= roll_time[i] && time_d <= roll_time[i+1])
			{
			found = MB_YES;
			iroll = i;
			}

	/* set the correction */
	if (found == MB_YES)
		{
		*roll_correction = roll_corr[iroll] 
			+ (roll_corr[iroll+1] - roll_corr[iroll])
			*(time_d - roll_time[iroll])
			/(roll_time[i+1] - roll_time[iroll]);
		}
	else
		*roll_correction = 0.0;

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBATH function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       roll_correction: %f\n",
						*roll_correction);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/

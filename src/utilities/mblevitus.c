/*--------------------------------------------------------------------
 *    The MB-system:	mblevitus.c	3.00	4/15/93
 *    $Id: mblevitus.c,v 3.5 1993-11-05 16:13:40 caress Exp $
 *
 *    Copyright (c) 1993 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBLEVITUS generates an average water velocity profile for a 
 * specified location from the Levitus temperature and salinity 
 * database.
 *
 * Author:	D. W. Caress
 * Date:	April 15, 1993
 *
 * $Log: not supported by cvs2svn $
 * Revision 3.4  1993/06/30  21:50:13  caress
 * Set for LDEO location of Levitus annual database.
 *
 * Revision 3.3  1993/06/30  02:53:06  caress
 * *** empty log message ***
 *
 * Revision 3.2  1993/06/30  02:52:06  caress
 * Changed location of levitus database to
 * /home/hs/packages/Levitus/levitus.annual
 * This is another temporary fix.
 *
 * Revision 3.1  1993/05/16  18:18:10  caress
 * Changed location of Levitus annual file to
 * /usr/local/lib - this is a temporary fix.
 *
 * Revision 3.0  1993/05/04  22:38:44  dale
 * Inital version.
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <strings.h>

/* mbio include files */
#include "../../include/mb_status.h"

/* global defines */
#define	DTR	M_PI/180.0
#define	NO_DATA	-1000000000.0
#define	NDEPTH_MAX	46

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
	static char rcs_id[] = "$Id: mblevitus.c,v 3.5 1993-11-05 16:13:40 caress Exp $";
	static char program_name[] = "MBLEVITUS";
	static char help_message[] = "MBLEVITUS generates an average water velocity profile for a \nspecified location from the Levitus temperature and salinity database.";
	static char usage_message[] = "mblevitus [-Rlon/lat -Ooutfile -V -H]";
	extern char *optarg;
	extern int optkind;
	int	errflg = 0;
	int	c;
	int	status;
	int	verbose = 0;
	int	help = 0;
	int	flag = 0;
	int	error = MB_ERROR_NO_ERROR;

	/* input file set in include file */
#include "levitus.h"

	/* control parameters */
	char	ofile[128];
	FILE	*ifp, *ofp, *outfp;
	int	record_size;
	long int location;
	double	longitude = 0.0;
	double	latitude = 0.0;
	double	lon_actual;
	double	lat_actual;
	int	ilon;
	int	ilat;
	int	nvelocity;
	int	nvelocity_tot;
	float	temperature[33][180];
	float	salinity[33][180];
	float	velocity[NDEPTH_MAX];
	static float	depth[NDEPTH_MAX] = {
		    0.,   10.,   20.,   30.,   50., 
		   75.,  100.,  125.,  150.,  200.,
		  250.,  300.,  400.,  500.,  600.,
		  700.,  800.,  900., 1000., 1100.,
		 1200., 1300., 1400., 1500., 1750.,
		 2000., 2500., 3000., 3500., 4000.,
		 4500., 5000., 5500., 6000., 6500.,
		 7000., 7500., 8000., 8500., 9000.,
		 9500.,10000.,10500.,11000.,11500.,
		12000. }; /* 46 depth values */
	double	theta, sine_theta, sine_two_theta;
	double	geoid_latitude;
	double	d1, d2, depth_pressure, pressure;
	double	sm, pkc, c01, c10, c11, c20, b0, c00;
	double	b1, b2, b3;
	double	gradient;
	double	zero = 0.0;

	/* time, user, host variables */
	long int	right_now;
	char	date[25], user[128], host[128];

	int	i, j, k;

	/* set default output */
	strcpy(ofile,"velocity");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhR:r:O:o:")) != -1)
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
		case 'R':
		case 'r':
			sscanf (optarg,"%lf/%lf", &longitude,&latitude);
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
		exit(MB_FAILURE);
		}

	/* set output stream */
	if (verbose <= 1)
		outfp = stdout;
	else
		outfp = stderr;

	/* print starting message */
	if (verbose == 1)
		{
		fprintf(outfp,"\nProgram %s\n",program_name);
		fprintf(outfp,"Version %s\n",rcs_id);
		fprintf(outfp,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(outfp,"\ndbg2  Program <%s>\n",program_name);
		fprintf(outfp,"dbg2  Version %s\n",rcs_id);
		fprintf(outfp,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(outfp,"dbg2  Control Parameters:\n");
		fprintf(outfp,"dbg2       verbose:    %d\n",verbose);
		fprintf(outfp,"dbg2       help:       %d\n",help);
		fprintf(outfp,"dbg2       ifile:      %s\n",ifile);
		fprintf(outfp,"dbg2       ofile:      %s\n",ofile);
		fprintf(outfp,"dbg2       longitude:  %f\n",longitude);
		fprintf(outfp,"dbg2       latitude:   %f\n",latitude);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(outfp,"\n%s\n",help_message);
		fprintf(outfp,"\nusage: %s\n", usage_message);
		exit(MB_SUCCESS);
		}

	/* open the data file */
	if ((ifp = fopen(ifile, "r")) == NULL)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to Open Levitus database file <%s> for reading\n",ifile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
			exit(error);
		}

	/* check for sensible location */
	if (longitude < -360.0 || longitude > 360.0
		|| latitude < -90.0 || latitude > 90.0)
		{
		error = MB_ERROR_BAD_PARAMETER;
		fprintf(stderr,"\nInvalid location specified:  longitude: %f  latitude: %f\n",longitude,latitude);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* get the longitude and latitude indices */
	if (longitude < 0.0)
		ilon = (int) (longitude + 360.0);
	else if (longitude >= 360.0)
		ilon = (int) (longitude - 360.0);
	else
		ilon = (int) longitude;
	lon_actual = ilon + 0.5;
	ilat = (int) (latitude + 90.0);
	lat_actual = ilat - 89.5;
	fprintf(outfp,"\nLocation for mean annual water velocity profile:\n");
	fprintf(outfp,"  Requested:  %6.4f longitude   %6.4f latitude\n",
		longitude,latitude);
	fprintf(outfp,"  Used:       %6.4f longitude   %6.4f latitude\n",
		lon_actual,lat_actual);

	/* read the temperature */
	record_size = sizeof(float)*33*180;
	location = ilon*record_size;
	status = fseek(ifp,location,0);
	if ((status = fread(&temperature[0][0],1,record_size,ifp)) 
		== record_size)
		{
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
		}
	else
		{
		status = MB_FAILURE;
		error = MB_ERROR_EOF;
		}

	/* read the salinity */
	location = location + 360*record_size;
	status = fseek(ifp,location,0);
	if ((status = fread(&salinity[0][0],1,record_size,ifp)) 
		== record_size)
		{
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
		}
	else
		{
		status = MB_FAILURE;
		error = MB_ERROR_EOF;
		}

	/* close input file */
	fclose(ifp);

	/* get geoid latitude */
	theta = DTR*latitude;
	sine_theta = sin(theta);
	sine_two_theta = sin(2.0*theta);
	geoid_latitude = 978.0309 
		+ 5.18552*sine_theta*sine_theta 
		- 0.0057*sine_two_theta*sine_two_theta;

	/* calculate velocity from temperature and salinity */
	nvelocity = 0;
	for (i=0;i<33;i++)
	  {
	  if (salinity[i][ilat] > NO_DATA)
		{
		/* set counter */
		nvelocity++;

		/* get depth for a given pressure 
			as a function of latitude */
		d1 = (-3.434e-12*depth[i] + 1.113e-07)*depth[i] + 0.712953;
		d2 = d1*depth[i] + 14190.7*log(1 + 1.83e-05*depth[i]);
		depth_pressure = 1000.*(d2/(geoid_latitude 
					+ 0.0001113*depth[i]));
		if (fabs(depth_pressure - depth[i]) >= 0.5)
			pressure = depth[i]*depth[i]/depth_pressure;
		else
			pressure = depth[i];

		/* calculate sound velocity using 
			Wilson's October 1960 formula */
		sm = salinity[i][ilat] - 35.0;
		pkc = pressure * 0.1019716 + 1.03323;
		c00 = (((7.9851e-06*temperature[i][ilat] - 2.6054e-04)
			*temperature[i][ilat] - 0.044532)
			*temperature[i][ilat] + 4.5721)
			*temperature[i][ilat] + 1449.14;
		c01 = (7.7711e-07*temperature[i][ilat] - 0.011244)
			*temperature[i][ilat] + 1.39799;
		c10 = ((4.5283e-08*temperature[i][ilat] + 7.4812e-06)
			*temperature[i][ilat] - 1.8607e-04)
			*temperature[i][ilat] + 0.16027;
		c11 = (1.579e-09*temperature[i][ilat] + 3.158e-08)
			*temperature[i][ilat] + 7.7016e-05;
		c20 = (1.8563e-09*temperature[i][ilat] - 2.5294e-07)
			*temperature[i][ilat] + 1.0268e-05;
		b0 =  (1.69202E-03*sm + c01)*sm + c00;
		b1 =  c11*sm + c10;
		b2 =  -1.2943E-07*sm + c20;
		b3 =  -1.9646E-10*temperature[i][ilat] + 3.5216E-09;
		velocity[i] =  (((-3.3603E-12*pkc + b3)*pkc + b2)
			*pkc + b1)*pkc + b0;
		}
	  else
		velocity[i] = salinity[i][ilat];
	  }
	nvelocity_tot = nvelocity;

	/* check for existence of water velocity profile */
	if (nvelocity < 1)
		{
		error = MB_ERROR_BAD_PARAMETER;
		fprintf(stderr,"\nNo water velocity profile available for specified location.\n");
		fprintf(stderr,"This place is probably subaerial!\n");
		fprintf(stderr,"No output file created.\n");
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* extrapolate to depth of 12000 meters */
	if (nvelocity > 1)
		{
		gradient = (velocity[nvelocity-1] - velocity[nvelocity-2])
			/(depth[nvelocity-1] - depth[nvelocity-2]);
		for (i=nvelocity;i<NDEPTH_MAX;i++)
			{
			velocity[i] = velocity[i-1] 
				+ gradient*(depth[i] - depth[i-1]);
			}
		nvelocity_tot = NDEPTH_MAX;
		}

	/* open the output file */
	if ((ofp = fopen(ofile, "w")) == NULL)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to Open output file <%s> for writing\n",ofile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
			exit(error);
		}

	/* print it out */
	fprintf(ofp,"# Water velocity profile created by program %s version %s\n",
		program_name,rcs_id);
	fprintf(ofp,"# MB-system Version %s\n",MB_VERSION);
	right_now = time((long *)0);
	strncpy(date,"\0",25);
	right_now = time((long *)0);
	strncpy(date,ctime(&right_now),24);
	strcpy(user,getenv("USER"));
	gethostname(host,128);
	fprintf(ofp,"# Run by user <%s> on cpu <%s> at <%s>\n",
		user,host,date);
	fprintf(ofp,"# Water velocity profile derived from Levitus\n");
	fprintf(ofp,"# temperature and salinity database.  This profile\n");
	fprintf(ofp,"# represents the annual average water velocity\n");
	fprintf(ofp,"# structure for a 1 degree X 1 degree area centered\n");
	fprintf(ofp,"# at %6.4f longitude and %6.4f latitude.\n",
		lon_actual, lat_actual);
	fprintf(ofp,"# This water velocity profile is in the form\n");
	fprintf(ofp,"# of discrete (depth, velocity) points where\n");
	fprintf(ofp,"# the depth is in meters and the velocity in\n");
	fprintf(ofp,"# meters/second.\n");
	fprintf(ofp,"# The first %d velocity values are defined using the\n",
		nvelocity);
	fprintf(ofp,"# Levitus database; the remaining %d points are\n",
		nvelocity_tot-nvelocity);
	fprintf(ofp,"# extrapolated using a constant velocity gradient\n");
	fprintf(ofp,"# obtained from the two deepest known points.\n");

	for (i=0;i<nvelocity_tot;i++)
		fprintf(ofp,"%f %f\n",depth[i],velocity[i]);
	fprintf(outfp,"Velocity points defined by Levitus database: %d\n",
		nvelocity);
	fprintf(outfp,"Velocity points extrapolated to depth:       %d\n",
		nvelocity_tot-nvelocity);
	fprintf(outfp,"Velocity points written:                     %d\n",
		nvelocity_tot);
	fprintf(outfp,"Output file: %s\n",ofile);
	if (verbose >= 1)
		{
		fprintf(outfp,"\nMean annual water column profile:\n");
		fprintf(outfp,"     Depth Temperature Salinity   Velocity\n");
		for (i=0;i<nvelocity_tot;i++)
			{
			if (i<nvelocity)
			fprintf(outfp,"%10.4f %9.4f %9.4f   %9.4f\n",
				depth[i],temperature[i][ilat],
				salinity[i][ilat],velocity[i]);
			else
			fprintf(outfp,"%10.4f %9.4f %9.4f   %9.4f\n",
				depth[i],zero,
				zero,velocity[i]);
			}
		}

	/* close the output file */
	fclose(ofp);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(outfp,"\ndbg2  Program <%s> completed\n",
			program_name);
		fprintf(outfp,"dbg2  Ending status:\n");
		fprintf(outfp,"dbg2       status:  %d\n",status);
		}

	/* end it all */
	exit(status);
}

/*--------------------------------------------------------------------
 *    The MB-system:	mbmerge.c	2/20/93
 *
 *    $Id: mbmerge.c,v 4.6 1994-11-01 21:52:08 caress Exp $
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
 * MBMERGE merges new navigation with multibeam data for an input file 
 * and then writes the merged data to an output multibeam data file.
 * The input navigation must be in the L-DEO shipboard processing 
 * format. The default input and output streams are stdin and stdout.
 *
 * Author:	D. W. Caress
 * Date:	February 20, 1993
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.5  1994/10/21  13:02:31  caress
 * Release V4.0
 *
 * Revision 4.4  1994/05/11  19:35:20  caress
 * Added ability to check for time repeats or reverses in
 * navigation that could mess up the spline interpolation.
 *
 * Revision 4.3  1994/05/02  03:35:14  caress
 * Set mbmerge to ignore records with times outside the
 * input navigation times.
 *
 * Revision 4.2  1994/04/29  18:00:51  caress
 * Added ability to read navigation in five different formats.
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
 * Revision 3.1  1993/06/01  21:26:40  caress
 * Changed exit codes so that exits with MB_SUCCESS=1 if
 * successful and with MB_FAILURE=0 if failed.
 *
 * Revision 3.0  1993/05/04  22:10:41  dale
 * Initial version
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <strings.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"

/* DTR define */
#ifndef M_PI
#define	M_PI	3.14159265358979323846
#endif
#define DTR	(M_PI/180.)
#define RTD (180./M_PI)

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
	/* id variables */
	static char rcs_id[] = "$Id: mbmerge.c,v 4.6 1994-11-01 21:52:08 caress Exp $";
	static char program_name[] = "MBMERGE";
	static char help_message[] =  "MBMERGE merges new navigation with multibeam data from an \ninput file and then writes the merged data to an output \nmultibeam data file. The default input \nand output streams are stdin and stdout.";
	static char usage_message[] = "mbmerge [-Fformat -Llonflip -V -H  -Iinfile -Ooutfile -Mnavformat -Nnavfile]";

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
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	char	ifile[128];
	char	*imbio_ptr = NULL;
	char	ofile[128];
	char	*ombio_ptr = NULL;
	char	nfile[128];
	int	nformat = 5;
	FILE	*nfp;

	/* mbio read and write values */
	char	*store_ptr;
	int	kind;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	int	nbath;
	int	namp;
	int	nss;
	double	*bath = NULL;
	double	*bathacrosstrack = NULL;
	double	*bathalongtrack = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*ssacrosstrack = NULL;
	double	*ssalongtrack = NULL;
	int	idata = 0;
	int	icomment = 0;
	int	odata = 0;
	int	ocomment = 0;
	char	comment[256];

	/* time, user, host variables */
	long int	right_now;
	char	date[25], user[128], host[128];

	/* navigation handling variables */
	int	nnav;
	double	*ntime = NULL;
	double	*nlon = NULL;
	double	*nlat = NULL;
	double	*nlonspl = NULL;
	double	*nlatspl = NULL;
	int	nav_ok;
	int	make_heading = MB_NO;
	int	nget;
	int	time_j[5], hr;
	double	sec;
	char	NorS[2], EorW[2];
	double	mlon, llon, mlat, llat;
	int	degree, minute;
	double	second;
	double	splineflag;
	int	stime_i[7], ftime_i[7];
	int	itime;
	double	mtodeglon, mtodeglat;
	double	del_time, dx, dy, dist;

	char	buffer[128], tmp[128], *result;
	int	i, j, k, l, m;

	char	*ctime();
	char	*getenv();

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* reset all defaults but the format and lonflip */
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

	/* set default input and output */
	strcpy (ifile, "stdin");
	strcpy (ofile, "stdout");
	strcpy (nfile, "\0");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhF:f:L:l:I:i:O:o:M:m:N:n:Zz")) != -1)
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
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
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
		case 'M':
		case 'm':
			sscanf (optarg,"%d", &nformat);
			flag++;
			break;
		case 'N':
		case 'n':
			sscanf (optarg,"%s", nfile);
			flag++;
			break;
		case 'Z':
		case 'z':
			make_heading = MB_YES;
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
		fprintf(stderr,"dbg2       btime_i[6]:      %d\n",btime_i[6]);
		fprintf(stderr,"dbg2       etime_i[0]:      %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:      %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:      %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:      %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:      %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:      %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       etime_i[6]:      %d\n",etime_i[6]);
		fprintf(stderr,"dbg2       speedmin:        %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:         %f\n",timegap);
		fprintf(stderr,"dbg2       input file:      %s\n",ifile);
		fprintf(stderr,"dbg2       output file:     %s\n",ofile);
		fprintf(stderr,"dbg2       navigation file: %s\n",nfile);
		fprintf(stderr,"dbg2       nav format:      %d\n",nformat);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(MB_SUCCESS);
		}

	/* count the nav points */
	nnav = 0;
	if ((nfp = fopen(nfile, "r")) == NULL) 
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to Open Navigation File <%s> for reading\n",nfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_FAILURE);
		}
	while ((result = fgets(buffer,128,nfp)) == buffer)
		nnav++;
	fclose(nfp);

	/* allocate space for the nav points */
	status = mb_malloc(verbose,nnav*sizeof(double),&ntime,&error);
	status = mb_malloc(verbose,nnav*sizeof(double),&nlon,&error);
	status = mb_malloc(verbose,nnav*sizeof(double),&nlat,&error);
	status = mb_malloc(verbose,nnav*sizeof(double),&nlonspl,&error);
	status = mb_malloc(verbose,nnav*sizeof(double),&nlatspl,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_FAILURE);
		}

	/* read in nav points */
	nnav = 0;
	if ((nfp = fopen(nfile, "r")) == NULL) 
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to Open Navigation File <%s> for reading\n",nfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_FAILURE);
		}
	strncpy(buffer,"\0",sizeof(buffer));
	while ((result = fgets(buffer,128,nfp)) == buffer)
		{
		/* deal with nav in form: time_d lon lat */
		nav_ok = MB_NO;
		if (nformat == 1)
			{
			nget = sscanf(buffer,"%lf %lf %lf",
				&ntime[nnav],&nlon[nnav],&nlat[nnav]);
			if (nget == 3)
				nav_ok = MB_YES;
			}

		/* deal with nav in form: yr mon day hour min sec lon lat */
		else if (nformat == 2)
			{
			nget = sscanf(buffer,"%d %d %d %d %d %lf %lf %lf",
				&time_i[0],&time_i[1],&time_i[2],
				&time_i[3],&time_i[4],&sec,
				&nlon[nnav],&nlat[nnav]);
			time_i[5] = (int) sec;
			time_i[6] = 1000000*(sec - time_i[5]);
			mb_get_time(verbose,time_i,&time_d);
			ntime[nnav] = time_d;
			if (nget == 8)
				nav_ok = MB_YES;
			}

		/* deal with nav in form: yr jday hour min sec lon lat */
		else if (nformat == 3)
			{
			nget = sscanf(buffer,"%d %d %d %d %lf %lf %lf",
				&time_j[0],&time_j[1],&hr,
				&time_j[2],&sec,
				&nlon[nnav],&nlat[nnav]);
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
		else if (nformat == 4)
			{
			nget = sscanf(buffer,"%d %d %d %lf %lf %lf",
				&time_j[0],&time_j[1],&time_j[2],
				&sec,
				&nlon[nnav],&nlat[nnav]);
			time_j[3] = (int) sec;
			time_j[4] = 1000000*(sec - time_j[3]);
			mb_get_itime(verbose,time_j,time_i);
			mb_get_time(verbose,time_i,&time_d);
			ntime[nnav] = time_d;
			if (nget == 6)
				nav_ok = MB_YES;
			}

		/* deal with nav in L-DEO processed nav format */
		else if (nformat == 5)
			{
			strncpy(tmp,"\0",128);
			time_j[0] = atoi(strncpy(tmp,buffer,2)) + 1900;
			strncpy(tmp,"\0",128);
			time_j[1] = atoi(strncpy(tmp,buffer+3,3));
			strncpy(tmp,"\0",128);
			hr = atoi(strncpy(tmp,buffer+7,2));
			strncpy(tmp,"\0",128);
			time_j[2] = atoi(strncpy(tmp,buffer+10,2))
				+ 60*hr;
			strncpy(tmp,"\0",128);
			time_j[3] = atof(strncpy(tmp,buffer+13,3));
			time_j[4] = 0;
			mb_get_itime(verbose,time_j,time_i);
			mb_get_time(verbose,time_i,&time_d);
			ntime[nnav] = time_d;

			strncpy(NorS,"\0",sizeof(NorS));
			strncpy(NorS,buffer+20,1);
			strncpy(tmp,"\0",128);
			mlat = atof(strncpy(tmp,buffer+21,3));
			strncpy(tmp,"\0",128);
			llat = atof(strncpy(tmp,buffer+24,8));
			strncpy(EorW,"\0",sizeof(EorW));
			strncpy(EorW,buffer+33,1);
			strncpy(tmp,"\0",128);
			mlon = atof(strncpy(tmp,buffer+34,4));
			strncpy(tmp,"\0",128);
			llon = atof(strncpy(tmp,buffer+38,8));
			nlon[nnav] = mlon + llon/60.;
			if (strncmp(EorW,"W",1) == 0) 
				nlon[nnav] = -nlon[nnav];
			nlat[nnav] = mlat + llat/60.;
			if (strncmp(NorS,"S",1) == 0) 
				nlat[nnav] = -nlat[nnav];
			nav_ok = MB_YES;
			}

		/* deal with nav in ISAH format */
		else if (nformat == 6)
			{
			if (strncmp(buffer,"$PQDAT",6) == 0)
				{
				strncpy(tmp,"\0",128);
				time_i[0] = atoi(strncpy(tmp,buffer+7,4));
				time_i[1] = atoi(strncpy(tmp,buffer+11,2));
				time_i[2] = atoi(strncpy(tmp,buffer+13,2));
				}
			else if (strncmp(buffer,"$PQGLL",6) == 0)
				{
				strncpy(tmp,"\0",128);
				degree = atoi(strncpy(tmp,buffer+7,2));
				strncpy(tmp,"\0",128);
				minute = atoi(strncpy(tmp,buffer+9,2));
				strncpy(tmp,"\0",128);
				second = atof(strncpy(tmp,buffer+11,5));
				strncpy(NorS,"\0",sizeof(NorS));
				strncpy(NorS,buffer+17,1);
				nlat[nnav] = degree + minute/60. + second/3600.;
				if (strncmp(NorS,"S",1) == 0) 
					nlat[nnav] = -nlat[nnav];
				strncpy(tmp,"\0",128);
				degree = atoi(strncpy(tmp,buffer+19,3));
				strncpy(tmp,"\0",128);
				minute = atoi(strncpy(tmp,buffer+22,2));
				strncpy(tmp,"\0",128);
				second = atof(strncpy(tmp,buffer+24,5));
				strncpy(EorW,"\0",sizeof(EorW));
				strncpy(EorW,buffer+30,1);
				nlon[nnav] = degree + minute/60. + second/3600.;
				if (strncmp(EorW,"W",1) == 0) 
					nlon[nnav] = -nlon[nnav];
				strncpy(tmp,"\0",128);
				time_i[3] = atoi(strncpy(tmp,buffer+32,2));
				strncpy(tmp,"\0",128);
				time_i[4] = atoi(strncpy(tmp,buffer+34,2));
				strncpy(tmp,"\0",128);
				time_i[5] = atoi(strncpy(tmp,buffer+36,2));
				strncpy(tmp,"\0",128);
				time_i[6] = 10000*atoi(strncpy(tmp,buffer+38,2));
				mb_get_time(verbose,time_i,&time_d);
				ntime[nnav] = time_d;
				nav_ok = MB_YES;
				}
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
			fprintf(stderr,"dbg5       nav[%d]: %f %f %f\n",
				nnav,ntime[nnav],nlon[nnav],nlat[nnav]);
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

	/* set up spline interpolation of nav points */
	splineflag = 1.0e30;
	spline(ntime-1,nlon-1,nnav,splineflag,splineflag,nlonspl-1);
	spline(ntime-1,nlat-1,nnav,splineflag,splineflag,nlatspl-1);

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
		exit(MB_FAILURE);
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
		exit(MB_FAILURE);
		}

	/* allocate memory for data arrays */
	status = mb_malloc(verbose,beams_bath*sizeof(double),&bath,&error);
	status = mb_malloc(verbose,beams_amp*sizeof(double),&amp,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			&bathacrosstrack,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			&bathalongtrack,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&ss,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ssacrosstrack,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ssalongtrack,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_FAILURE);
		}

	/* write comments to beginning of output file */
	kind = MB_DATA_COMMENT;
	strncpy(comment,"\0",256);
	sprintf(comment,"This data merged with navigation by program %s version %s",
		program_name,rcs_id);
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
	sprintf(comment,"  Navigation file:    %s",nfile);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  Longitude flip:     %d",lonflip);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
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
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* read some data */
		error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		status = mb_get_all(verbose,imbio_ptr,&store_ptr,&kind,
				time_i,&time_d,&navlon,&navlat,&speed,
				&heading,&distance,
				&beams_bath,&beams_amp,&pixels_ss,
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

		/* time gaps do not matter to mbmerge */
		if (error == MB_ERROR_TIME_GAP)
			{
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* non-survey data do not matter to mbmerge */
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
			fprintf(stderr,"Time: %d %d %d %d %d %d %d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],
				time_i[6]);
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
			fprintf(stderr,"Last Good Time: %d %d %d %d %d %d %d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],
				time_i[6]);
			}

		/* interpolate the navigation */
		if (error == MB_ERROR_NO_ERROR
			|| kind == MB_DATA_COMMENT)
			{
			status = splint(ntime-1,nlon-1,nlonspl-1,
				nnav,time_d,&navlon,&itime);
			status = splint(ntime-1,nlat-1,nlatspl-1,
				nnav,time_d,&navlat,&itime);
			}

		/* make up heading and speed if required */
		if (error == MB_ERROR_NO_ERROR
			&& make_heading == MB_YES)
			{
			mb_coor_scale(verbose,nlat[itime],&mtodeglon,&mtodeglat);
			del_time = ntime[itime+1] - ntime[itime];
			dx = (nlon[itime+1] - nlon[itime])/mtodeglon;
			dy = (nlat[itime+1] - nlat[itime])/mtodeglat;
			dist = sqrt(dx*dx + dy*dy);
			if (del_time > 0.0)
				speed = 3.6*dist/del_time;
			else
				speed = 0.0;
			heading = RTD*atan2(dx/dist,dy/dist);
			}

		/* give error message */
		if ((verbose >= 1 && error == MB_ERROR_NO_ERROR 
			&& (time_d < ntime[0] || time_d > ntime[nnav-1]))
			&& kind != MB_DATA_COMMENT)
			{
			fprintf(stderr,"\nData record not written!\n");
			fprintf(stderr,"Data time lies outside the bounds of the input navigation...\n");
			fprintf(stderr,"Data time: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
				time_i[0],time_i[1],time_i[2],time_i[3],
				time_i[4],time_i[5],time_i[6]);
			}

		/* write some data */
		if ((error == MB_ERROR_NO_ERROR 
			&& (time_d >= ntime[0] && time_d <= ntime[nnav-1]))
			|| kind == MB_DATA_COMMENT)
			{
			status = mb_put_all(verbose,ombio_ptr,
					store_ptr,MB_YES,kind,
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
				fprintf(stderr,"Time: %d %d %d %d %d %d %d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],
					time_i[6]);
				fprintf(stderr,"\nProgram <%s> Terminated\n",
					program_name);
				exit(MB_FAILURE);
				}
			}
		}

	/* close the files */
	status = mb_close(verbose,&imbio_ptr,&error);
	status = mb_close(verbose,&ombio_ptr,&error);

	/* deallocate memory for data arrays */
	mb_free(verbose,&ntime,&error);
	mb_free(verbose,&nlon,&error);
	mb_free(verbose,&nlat,&error);
	mb_free(verbose,&nlonspl,&error);
	mb_free(verbose,&nlatspl,&error);
	mb_free(verbose,&bath,&error); 
	mb_free(verbose,&amp,&error); 
	mb_free(verbose,&bathacrosstrack,&error); 
	mb_free(verbose,&bathalongtrack,&error); 
	mb_free(verbose,&ss,&error); 
	mb_free(verbose,&ssacrosstrack,&error); 
	mb_free(verbose,&ssalongtrack,&error); 

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* give the statistics */
	if (verbose >= 1)
		{
		fprintf(stderr,"\n%d input navigation records\n",nnav);
		fprintf(stderr,"%d input data records\n",idata);
		fprintf(stderr,"%d input comment records\n",icomment);
		fprintf(stderr,"%d output data records\n",odata);
		fprintf(stderr,"%d output comment records\n",ocomment);
		}

	/* end it all */
	exit(status);
}
/*--------------------------------------------------------------------*/
int spline(x,y,n,yp1,ypn,y2)
/* From Numerical Recipies */
double x[],y[],yp1,ypn,y2[];
int n;
{
	int i,k;
	double p,qn,sig,un,*u,*vector();
	void free_vector();

	u=vector(1,n-1);
	if (yp1 > 0.99e30)
		y2[1]=u[1]=0.0;
	else {
		y2[1] = -0.5;
		u[1]=(3.0/(x[2]-x[1]))*((y[2]-y[1])/(x[2]-x[1])-yp1);
	}
	for (i=2;i<=n-1;i++) {
		sig=(x[i]-x[i-1])/(x[i+1]-x[i-1]);
		p=sig*y2[i-1]+2.0;
		y2[i]=(sig-1.0)/p;
		u[i]=(y[i+1]-y[i])/(x[i+1]-x[i]) - (y[i]-y[i-1])/(x[i]-x[i-1]);
		u[i]=(6.0*u[i]/(x[i+1]-x[i-1])-sig*u[i-1])/p;
	}
	if (ypn > 0.99e30)
		qn=un=0.0;
	else {
		qn=0.5;
		un=(3.0/(x[n]-x[n-1]))*(ypn-(y[n]-y[n-1])/(x[n]-x[n-1]));
	}
	y2[n]=(un-qn*u[n-1])/(qn*y2[n-1]+1.0);
	for (k=n-1;k>=1;k--)
		y2[k]=y2[k]*y2[k+1]+u[k];
	free_vector(u,1,n-1);
	return(0);
}
/*--------------------------------------------------------------------*/
int splint(xa,ya,y2a,n,x,y,i)
/* From Numerical Recipies */
double xa[],ya[],y2a[],x,*y;
int n;
int *i;
{
	int klo,khi,k;
	double h,b,a;

	klo=1;
	khi=n;
	while (khi-klo > 1) {
		k=(khi+klo) >> 1;
		if (xa[k] > x) khi=k;
		else klo=k;
	}
	h=xa[khi]-xa[klo];
	if (h == 0.0) 
		{
		fprintf(stderr,"ERROR: interpolation time out of nav bounds\n");
		return(-1);
		}
	a=(xa[khi]-x)/h;
	b=(x-xa[klo])/h;
	*y=a*ya[klo]+b*ya[khi]+((a*a*a-a)*y2a[klo]
		+(b*b*b-b)*y2a[khi])*(h*h)/6.0;
	*i=klo;
	return(0);
}
/*--------------------------------------------------------------------*/
double *vector(nl,nh)
int nl,nh;
{
	double *v;
	v = (double *) malloc ((unsigned) (nh-nl+1)*sizeof(double));
	if (!v) fprintf(stderr,"allocation failure in vector()");
	return v-nl;
}
/*--------------------------------------------------------------------*/
void free_vector(v,nl,nh)
double *v;
int nl,nh;
{
	free((char*) (v+nl));
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
 *    The MB-system:	mbps.c	11/4/93
 *    $Id: mbps.c,v 4.4 1994-10-21 13:02:31 caress Exp $
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
 * MBPS is a program that plots an almost correct perspective view
 * of a piece of multibeam data. Input is some multibeam data
 * file; output is PostScript code.
 *
 * Authors:	Russ Alexander, UCSB
 *		Alberto Malinverno, L-DEO
 * Date:	September 15, 1993 (version 3)
 * Date:	August 31, 1991 (original version)
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.3  1994/07/29  19:02:56  caress
 * Changes associated with supporting byte swapped Lynx OS and
 * using unix second time base.
 *
 * Revision 4.2  1994/03/08  12:51:05  caress
 * Really fixed mb_format_inf call.
 * l
 *
 * Revision 4.1  1994/03/08  12:44:33  caress
 * Fixed mb_format_info call.
 *
 * Revision 4.0  1994/03/06  00:13:22  caress
 * First cut at version 4.0
 *
 * Revision 4.0  1994/03/01  18:59:27  caress
 * First cut at new version. Any changes are associated with
 * support of three data types (beam bathymetry, beam amplitude,
 * and sidescan) instead of two (bathymetry and backscatter).
 *
 * Revision 1.2  1993/11/04  19:32:28  caress
 * Fixed some details.  PSLIB calls now correct for GMT v 2.1.4
 * and gmtdefs now used in part.  Will need some more cleaning
 * up later.
 *
 * Revision 1.1  1993/11/04  18:09:06  caress
 * Initial revision
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <strings.h>

/* MBIO include files */
#include "../../include/mb_status.h"

/* GMT include files */
#include "gmt.h"

/* DTR define */
#ifndef M_PI
#define	M_PI	3.14159265358979323846
#endif
#define DTR	(M_PI/180.)

/*--------------------------------------------------------------------*/

/* Global defines */
#define 	BEAMS_MAX	200
#define		PINGS_MAX	1000
#define		PINGS_READ	1
#define 	DY_DEF		(0.1)	/* km */
#define		DZ_DEF		(-50)	/* m */
#define		PLOT_XMAX_DEF	5.0
#define		PLOT_YMAX_DEF	8.0
#define		VIEWDIR_DEF	'S'
#define		ALPHA_DEF	70.0
#define		ETA_DEF		45.0
#define		BAD		(-32000.0)
#define		VE_DEF		5.0
#define		MBPS_MAXPINGS 50

struct ping
	{
	double	*bath;
	double	*bathacrosstrack;
	double	*bathalongtrack;
	double	*amp;
	double	*ss;
	double	*ssacrosstrack;
	double	*ssalongtrack;
	};

main (argc, argv)
int argc;
char **argv; 
{

	static char rcs_id[] = "$Id: mbps.c,v 4.4 1994-10-21 13:02:31 caress Exp $";
	static char program_name[] = "MBPS";
	static char help_message[] =  "MBPS reads a multibeam bathymetry data file and creates a postscript 3-d mesh plot";
	static char usage_message[] = "mbps [-Iinfile -Fformat -Byr/mo/da/hr/mn/sc -Eyr/mo/da/hr/mn/sc -Aalpha -Keta -Dviewdir -Xvertexag -T\"title\" -Wmetersperinch -Sspeedmin -Ggap -Ydisplay_stats -Zdisplay_scales -V -H]";
	extern char *optarg;
	extern int optkind;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/*ALBERTO definitions */
	int ib, jb, g, gap=1;
	int xp[PINGS_MAX][BEAMS_MAX],yp[PINGS_MAX][BEAMS_MAX];
	double xl[PINGS_MAX*BEAMS_MAX], yl[PINGS_MAX*BEAMS_MAX], xc[4], yc[4], bottom_yl;
	double plot_xmax=PLOT_XMAX_DEF, plot_ymax=PLOT_YMAX_DEF, alpha=ALPHA_DEF;
	double eta=ETA_DEF, sin_eta, cos_eta;
	double sin_alpha, cos_alpha, track_length, xscale, zscale, zscale_inch;
	double mean_xp=0.0, mean_yp=0.0, min_xp, max_xp, min_yp, max_yp;
	double scaling, scaling_xp, scaling_yp, ve=VE_DEF, x_off, y_off;
	double min_z, max_z, range_z, meters_per_inch=(-1.0);
	double mean_lat=0.0, mean_lon=0.0, mean_latmin, mean_lonmin, mean_hdg=0.0;
	int n_pings, done, mean_knt=0;
	int orient;
	char viewdir=VIEWDIR_DEF;
	char label[100];

	int a,b,num_pts,rotate;
	int cnt[1];	
	double x,y,z;
	int display_stats=1;	/* 1 if displaying stats (eg view angle, VE, Scale, ...) else 0 */
	int display_scales=1;	/* 1 if displaying scales, ship direction arrow, and coor. axes else 0 */

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	*message;

	/* MBIO read control parameters */
	int	format;
	int	format_num;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	char	file[128];
	int	pings_get = 1;
	int	pings = 1;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;

	/* MBIO read values */
	char	*mbio_ptr = NULL;
	int	kind;
	struct ping *data[MBPS_MAXPINGS];
	struct ping *datacur;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	*bath = NULL;
	double	*bathacrosstrack = NULL;
	double	*bathalongtrack = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*ssacrosstrack = NULL;
	double	*ssalongtrack = NULL;
	char	comment[256];
	int	icomment = 0;
	int	comments = MB_NO;

	/* limit variables */
	double	lonmin = 0.0;
	double	lonmax = 0.0;
	double	latmin = 0.0;
	double	latmax = 0.0;
	double	bathmin = 0.0;
	double	bathmax = 0.0;
	double	backmin = 0.0;
	double	backmax = 0.0;
	double	bathbeg = 0.0;
	double	lonbeg = 0.0;
	double	latbeg = 0.0;
	double	bathend = 0.0;
	double	lonend = 0.0;
	double	latend = 0.0;
	double	spdbeg = 0.0;
	double	hdgbeg = 0.0;
	double	spdend = 0.0;
	double	hdgend = 0.0;
	double	timbeg = 0.0;
	double	timend = 0.0;
	int	timbeg_i[7];
	int	timend_i[7];
	int	timbeg_j[5];
	int	timend_j[5];
	double	distot = 0.0;
	double	timtot = 0.0;
	double	spdavg = 0.0;
	int	irec = 0;
	int	ngdbeams = 0;
	int	nzdbeams = 0;
	int	nfdbeams = 0;
	int	ngbbeams = 0;
	int	nzbbeams = 0;
	int	nfbbeams = 0;
	int	begin = 0;
	int	nread = 0;

	struct EPS *eps;
	char title[128];
	FILE	*output;
	int i, j, k, l, m;

	/* initialize some time variables */
	for (i=0;i<7;i++)
		{
		timbeg_i[i] = 0;
		timend_i[i] = 0;
		}
	/* get current default values */
	status = mb_defaults(verbose,&format,&pings_get,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to stdin */
	strcpy (file, "stdin");


	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhF:f:B:b:E:e:S:s:T:t:I:i:A:a:X:x:K:k:D:d:M:m:W:w:G:g:Y:y:Z:z")) != -1)
	    switch (c) {
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
		case 'B':
		case 'b':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&btime_i[0],&btime_i[1],&btime_i[2],
				&btime_i[3],&btime_i[4],&btime_i[5]);
			btime_i[6] = 0;
			flag++;
			break;
		case 'E':
		case 'e':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&etime_i[0],&etime_i[1],&etime_i[2],
				&etime_i[3],&etime_i[4],&etime_i[5]);
			etime_i[6] = 0;
			flag++;
			break;
		case 'S':
		case 's':
			sscanf (optarg,"%lf", &speedmin);
			flag++;
			break;
		case 'T':
		case 't':
			sscanf (optarg,"%s", title);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", file);
			flag++;
			break;
		case 'A':
		case 'a':
			sscanf (optarg, "%lf", &alpha);
			flag++;
			break;
		case 'X':
		case 'x':
			sscanf (optarg, "%lf", &ve);
			flag++;
			break;
		case 'K':
		case 'k':
			sscanf (optarg, "%lf", &eta);
			flag++;
			break;
		case 'D':
		case 'd':
			sscanf (optarg, "%c", &viewdir);
			flag++;
			break;
		case 'W':
		case 'w':
			sscanf (optarg, "%lf", &meters_per_inch);
			flag++;
			break;
		case 'Y':
		case 'y':
			sscanf (optarg, "%d", &display_stats);
			flag++;
			break;
		case 'Z':
		case 'z':
			sscanf (optarg, "%d", &display_scales);
			flag++;
			break;
		case 'G':
		case 'g':
			sscanf (optarg, "%d", &gap);
			flag++;
			break;
		case '?':
			errflg++;
			break;
	    } /* switch */

		
	/* Process the title of the plot */
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-'&& ((argv[i][1]=='T')||(argv[i][1]=='t')) ) {
			strcpy(title,argv[i]);
			title[0]=' ';
			title[1]=' ';
		}
	}


	/* check that otions are allowed */
	if ((viewdir!='P') && (viewdir!='S') && (viewdir!='B') && 
            (viewdir!='p') && (viewdir!='s') && (viewdir!='b'))
	{
		fprintf(stderr,"viewdir must be either P/p (port) S/s (stbd) or B/b (back)\n");
		errflg++;
	}

	/* set output stream */
	if (verbose <= 1)
		output = stdout;
	else
		output = stderr;

	/* if error flagged then print it and exit */
	if (errflg) {
		fprintf(output,"usage: %s\n", usage_message);
		fprintf(output,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_FAILURE);
	}

	/* print starting message */
	if (verbose == 1) {
		fprintf(output,"\nProgram %s\n",program_name);
		fprintf(output,"Version %s\n",rcs_id);
		fprintf(output,"MB-system Version %s\n",MB_VERSION);
	}

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(output,"\ndbg2  Program <%s>\n",program_name);
		fprintf(output,"dbg2  Version %s\n",rcs_id);
		fprintf(output,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(output,"dbg2  Control Parameters:\n");
		fprintf(output,"dbg2       verbose:    %d\n",verbose);
		fprintf(output,"dbg2       help:       %d\n",help);
		fprintf(output,"dbg2       format:     %d\n",format);
		fprintf(output,"dbg2       btime_i[0]: %d\n",btime_i[0]);
		fprintf(output,"dbg2       btime_i[1]: %d\n",btime_i[1]);
		fprintf(output,"dbg2       btime_i[2]: %d\n",btime_i[2]);
		fprintf(output,"dbg2       btime_i[3]: %d\n",btime_i[3]);
		fprintf(output,"dbg2       btime_i[4]: %d\n",btime_i[4]);
		fprintf(output,"dbg2       btime_i[5]: %d\n",btime_i[5]);
		fprintf(output,"dbg2       btime_i[6]: %d\n",btime_i[6]);
		fprintf(output,"dbg2       etime_i[0]: %d\n",etime_i[0]);
		fprintf(output,"dbg2       etime_i[1]: %d\n",etime_i[1]);
		fprintf(output,"dbg2       etime_i[2]: %d\n",etime_i[2]);
		fprintf(output,"dbg2       etime_i[3]: %d\n",etime_i[3]);
		fprintf(output,"dbg2       etime_i[4]: %d\n",etime_i[4]);
		fprintf(output,"dbg2       etime_i[5]: %d\n",etime_i[5]);
		fprintf(output,"dbg2       etime_i[6]: %d\n",etime_i[6]);
		fprintf(output,"dbg2       speedmin:   %f\n",speedmin);
		fprintf(output,"dbg2       file:       %s\n",file);
	}

	/* if help desired then print it and exit */
	if (help) {
		fprintf(output,"\n%s\n",help_message);
		fprintf(output,"\nusage: %s\n", usage_message);
		exit(MB_ERROR_NO_ERROR);
	}

	/* initialize reading the multibeam file */
	if ((status = mb_read_init(
		verbose,file,format,pings_get,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&mbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS) {
		mb_error(verbose,error,&message);
		fprintf(output,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(output,"\nMultibeam File <%s> not initialized for reading\n",file);
		fprintf(output,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
	}


	/* allocate memory for data arrays */
	for (i=0;i<pings;i++) {
		data[i] = NULL;
		status = mb_malloc(verbose,pings*sizeof(struct ping),
					&data[i],&error);
		if (error == MB_ERROR_NO_ERROR) {
			datacur = data[i];
			datacur->bath = NULL;
			datacur->amp = NULL;
			datacur->bathacrosstrack = NULL;
			datacur->bathalongtrack = NULL;
			datacur->ss = NULL;
			datacur->ssacrosstrack = NULL;
			datacur->ssalongtrack = NULL;
			status = mb_malloc(verbose,beams_bath*sizeof(double),
					&datacur->bath,&error);
			status = mb_malloc(verbose,beams_amp*sizeof(double),
					&datacur->amp,&error);
			status = mb_malloc(verbose,beams_bath*sizeof(double),
					&datacur->bathacrosstrack,&error);
			status = mb_malloc(verbose,beams_bath*sizeof(double),
					&datacur->bathalongtrack,&error);
			status = mb_malloc(verbose,pixels_ss*sizeof(double),
					&datacur->ss,&error);
			status = mb_malloc(verbose,pixels_ss*sizeof(double),
					&datacur->ssacrosstrack,&error);
			status = mb_malloc(verbose,pixels_ss*sizeof(double),
					&datacur->ssalongtrack,&error);
		} /* if data[i] */
	} 	 /* for i */	  


	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
		fprintf(output,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(output,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* printf out file and format */
	status = mb_format(verbose,&format,&format_num,&error);
	mb_format_inf(verbose,format_num,&message);
	fprintf(stderr,"\nMultibeam Data File:  %s\n",file);
	fprintf(stderr,"MBIO Data Format ID:  %d\n",format);
	fprintf(stderr,"%s",message);


	/* READ AND PROCESS DATA */
	while (error <= MB_ERROR_NO_ERROR)
		{
		nread = 0;
		error = MB_ERROR_NO_ERROR;
		while (nread < PINGS_READ && error == MB_ERROR_NO_ERROR) {

			/* read a ping of data */
			datacur = data[nread];
			bath = datacur->bath;
			bathacrosstrack = datacur->bathacrosstrack;
			bathalongtrack = datacur->bathalongtrack;
			amp = datacur->amp;
			ss = datacur->ss;
			ssacrosstrack = datacur->ssacrosstrack;
			ssalongtrack = datacur->ssalongtrack;
			status = mb_get(verbose,mbio_ptr,&kind,&pings,
				time_i,&time_d,
				&navlon,&navlat,&speed,&heading,&distance,
				&beams_bath,&beams_amp,&pixels_ss,
				bath,amp,bathacrosstrack,
				bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,&error);

			/* increment counters */
			if (error == MB_ERROR_NO_ERROR 
				|| error == MB_ERROR_TIME_GAP)
				{
				irec++;
				nread++;
				}

			/* output error messages */
			if (error == MB_ERROR_COMMENT)
				{
				/* do nothing */
				}
			else if (verbose >= 1 && error < MB_ERROR_NO_ERROR
				&& error >= MB_ERROR_OTHER)
				{
				mb_error(verbose,error,&message);
				fprintf(output,"\nNonfatal MBIO Error:\n%s\n",
					message);
				fprintf(output,"Time: %d %d %d %d %d %d %d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],
					time_i[6]);
				}
			else if (verbose >= 1 && error < MB_ERROR_NO_ERROR)
				{
				mb_error(verbose,error,&message);
				fprintf(output,"\nNonfatal MBIO Error:\n%s\n",
					message);
				fprintf(output,"Number of good records so far: %d\n",irec);
				}
			else if (verbose >= 1 && error > MB_ERROR_NO_ERROR 
				&& error != MB_ERROR_EOF)
				{
				mb_error(verbose,error,&message);
				fprintf(output,"\nFatal MBIO Error:\n%s\n",
					message);
				fprintf(output,"Last Good Time: %d %d %d %d %d %d %d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],
					time_i[6]);
				}

		} /* end reading of data, 2'nd while under read/process data */


		/* print debug statements */
		if (verbose >= 2) {
			fprintf(output,"\ndbg2  Reading loop finished in program <%s>\n",
			program_name);
			fprintf(output,"dbg2       status:     %d\n",status);
			fprintf(output,"dbg2       error:      %d\n",error);
			fprintf(output,"dbg2       nread:      %d\n",nread);
			fprintf(output,"dbg2       pings: %d\n",pings);
		}


		i=done=0;
		sin_alpha=sin(alpha*DTR);
		cos_alpha=cos(alpha*DTR);
		sin_eta=sin(eta*DTR);
		cos_eta=cos(eta*DTR);
		min_z=0.0;
		max_z= -9999.0;
			if (status==MB_SUCCESS) {
				distot+=distance*1000.0;	/* dist in meters */
				for (j=0; j<beams_bath; j++) {
					if (bath[j]>0.0) {
					/* bath[] > 0 for unflagged data */
					    if (viewdir=='S' || viewdir=='s') {
						xp[irec-1][j]=distot+bathacrosstrack[j]*
							sin_eta*cos_alpha;
						yp[irec-1][j]= (-bath[j]*cos_eta*ve)-
							bathacrosstrack[j]*sin_eta*sin_alpha;
					    }
					    else if (viewdir=='P' || viewdir=='p') {
						xp[irec-1][j]= -distot-bathacrosstrack[j]*
							sin_eta*cos_alpha;
						yp[irec-1][j]= (-bath[j]*cos_eta*ve)+
							bathacrosstrack[j]*sin_eta*sin_alpha;
					    }
					    else if (viewdir=='B' || viewdir=='b') {
						xp[irec-1][j]= bathacrosstrack[j]+distot*
							sin_eta*cos_alpha;
						yp[irec-1][j]= (-bath[j]*cos_eta*ve)+
							distot*sin_eta*sin_alpha;
					    }
					    mean_lat+=navlat;
					    mean_lon+=navlon;
					    mean_hdg+=heading;
					    mean_xp+=xp[irec-1][j];
					    mean_yp+=yp[irec-1][j];
					    mean_knt++;

					    if (-bath[j] < min_z) min_z= -bath[j];
					    if (-bath[j] > max_z) max_z= -bath[j];
					}
					else {
						xp[irec-1][j]=BAD;
						yp[irec-1][j]=BAD;
					}
				} /* for j=0 ... */


				if ((irec-1)==0)
					for (k=0; k<7; k++)
						timbeg_i[k]=time_i[k];
				else
					for (k=0; k<7; k++)
						timend_i[k]=time_i[k];
				i++;
				if (i>=(PINGS_MAX-3)) {
					fprintf(stderr,
					"%s: WARNING: Too many pings\n",
					program_name);
					done=1;
				}

			}	/* if status==MB_SUCCESS */

		}  /* end of processing data, 1'st while under read/process data */


		/* print debug statements */
	if (verbose >= 2) {
		fprintf(output,"\ndbg2  Processing loop finished in program <%s>\n",
				program_name);
		fprintf(output,"dbg2       status:     %d\n",status);
		fprintf(output,"dbg2       error:      %d\n",error);
		fprintf(output,"dbg2       nread:      %d\n",nread);
		fprintf(output,"dbg2       pings: %d\n",pings);
	}


	track_length=distot;	
	/* total track length in m */
	mean_lat/=mean_knt;
	mean_latmin=fabs(mean_lat-(int)mean_lat)*60.0;
	mean_lon/=mean_knt;
	mean_lonmin=fabs(mean_lon-(int)mean_lon)*60.0;
	mean_hdg/=mean_knt;
	mean_xp/=mean_knt;
	mean_yp/=mean_knt;


		/* rescale xp[],yp[] to zero mean; get min and max */
	max_yp=min_yp=max_xp=min_xp=0.0;
	for (i=0; i<irec; i++) {
		for (j=0; j< beams_bath; j++) {
			if (yp[i][j]!=BAD) {
				yp[i][j] -= mean_yp;
				if (yp[i][j]<min_yp) min_yp=yp[i][j];
				if (yp[i][j]>max_yp) max_yp=yp[i][j];
				xp[i][j] -= mean_xp;
				if (xp[i][j]<min_xp) min_xp=xp[i][j];
				if (xp[i][j]>max_xp) max_xp=xp[i][j];
			} /* if yp[][] */
		} 	  /* for j */
	} 		  /* for i */



		/* get page orientation, scaling(in/m) factor and startup plot */
	if ((viewdir=='P') || (viewdir=='S') || (viewdir=='p') || (viewdir=='s')) {
		orient=0;		/* Landscape */
		if (meters_per_inch > 0.0) {
			scaling=1.0/meters_per_inch;
			x_off=11./2;
			y_off=-8.5/2.;
		} else {
			if ( (5.2/(max_yp-min_yp))<(8.5/(max_xp-min_xp)) )
				scaling = (5.2/(max_yp-min_yp));
			else
				scaling = (8.5/(max_xp-min_xp));			
		x_off=(-(max_xp+min_xp)*scaling/2.0)+(11./2);
		y_off=(-(max_yp+min_yp)*scaling/2.0)-(8.5/2)-.2;
		}
	} else {
		orient=1;		/* Potrait */
		if (meters_per_inch > 0.0) {
			scaling=1.0/meters_per_inch;
			x_off=8.5/2.0;
			y_off=11./2.0;
		} else {
			if ( (8./(max_yp-min_yp))<(6.5/(max_xp-min_xp)) )
				scaling = (8./(max_yp-min_yp));
			else
				scaling = (6.5/(max_xp-min_xp));
		x_off=(-(max_xp+min_xp)*scaling/2.0)+(8.5/2);
		y_off=(-(max_yp+min_yp)*scaling/2.0)+(11./2)-.2;
		}
	}


/* ps_plotinit (plotfile, overlay, mode, xoff, yoff, xscl,
          yscl, ncopies, dpi, unit, pagewidth, eps)
          char *plotfile; "" or NULL for stdout
          int overlay, mode, ncopies, dpi, unit;
          double xoff, yoff, xscl, yscl, pagewidth;
          struct EPS * eps;
*/

	ps_plotinit(NULL,0,orient,x_off,y_off,1.0,1.0,1,300,1,
		gmtdefs.paper_width, gmtdefs.page_rgb, eps);



		/* PLOT POLYGONS  */

		/* Plot a filled polygon underneath plot to show data gaps */
	if (gap==1) {
		 Polygon_Fill(xl,yl,xp,yp,irec,beams_bath,cnt,scaling);
	}

	for (j=0; j<beams_bath; j++) {
		if ((viewdir=='S') || (viewdir=='s')) jb=j;
		else if ((viewdir=='P') || (viewdir=='p')) jb=beams_bath-1-j;
		else if ((viewdir=='B') || (viewdir=='b')) {
			if (alpha<90.0) jb=j;
			else jb=beams_bath-1-j;
		}

		for (i=0; i<irec; i++) {
			Good_Polygon(xl,yl,xp,yp,i,jb,irec,beams_bath,scaling);
		}			/* for i=k=0 ... */
	}			/* for (j=0; j<beams_bath; j++) */


		/* TITLES AND SUCH */
	ps_setline(2);	/* set line width */




	if (!display_stats) {
			/* plot a title */
		xl[0]=0;
		yl[0]=max_yp*scaling+.6;
		sprintf(label,"%s",title);
		ps_text(xl[0],yl[0],20,label,0.,6,0);
	 } else {
			/* plot a title */
		xl[0]=0;
		yl[0]=max_yp*scaling+1.3;
		sprintf(label,"%s",title);
		ps_text(xl[0],yl[0],20,label,0.,6,0);

		xl[0]-=3.25;
		yl[0]-=0.3;
		sprintf(label,"Mean Lat.: %3d@+o@+ %4.1f'   Mean Lon.: %4d@+o @+%4.1f'   Heading: %.1lf@+o @+",(int)mean_lat, mean_latmin, (int)mean_lon, mean_lonmin, mean_hdg);
		ps_text(xl[0],yl[0],15,label,0.,4,0);

		yl[0]-=0.3;
		sprintf(label,"View Angle: %.1lf@+o @+  V.E.: %.1lfX   Scale: %.0lf m/inch   Track Length: %.1lf km",eta,ve,1.0/scaling,track_length/1000.0);
		ps_text(xl[0],yl[0],15,label,0.,4,0);

		yl[0]-=0.3;
		sprintf(label,
		"From %.4d/%.2d/%.2d %.2d:%.2d:%.2d   to  %.4d/%.2d/%.2d %.2d:%.2d:%.2d",
		timbeg_i[0],timbeg_i[1],timbeg_i[2],timbeg_i[3],
		timbeg_i[4],timbeg_i[5],timend_i[0],timend_i[1],
		timend_i[2],timend_i[3],timend_i[4],timend_i[5]);
		ps_text(xl[0],yl[0],15,label,0.,4,0);
	} /* else after if display_stats */


	if (display_scales) {
			/* plot the x-scale */
		xscale=10000;		/* x scale in m */
		if (track_length < 50000) xscale=5000;
		if (track_length < 20000) xscale=2000;
		if (track_length < 10000) xscale=1000;
		xl[0]=xl[1]= (-xscale*scaling/2.0);
		xl[2]=xl[3]= (-xl[0]);
		xl[0]+=2.;xl[1]+=2.;xl[2]+=2.;xl[3]+=2.;
		yl[1]=yl[2]= min_yp*scaling-1.;
		yl[0]=yl[3]= yl[1]+0.1;
	
		ps_line(xl,yl,4,3,0);
		sprintf(label,"%.0f km",xscale/1000.0);
		ps_text(xl[0]+.5,yl[0]+.05,15,label,0.,6,0);

	
			/* plot the z-scale */
		range_z=(max_z-min_z);
		zscale=2000;		/* z scale in m */
		if (range_z < 3000) zscale=1000;
		if (range_z < 1000) zscale=500;
		if (range_z < 500) zscale=200;
		if (range_z < 250) zscale=100;
		zscale_inch= zscale*scaling*cos_eta*ve;
		xl[1]=xl[2]+0.5;
		xl[2]=xl[1];
		xl[0]=xl[3]= xl[1]+.1;
		yl[0]=yl[1]= min_yp*scaling-1.;
		yl[2]=yl[3]= yl[0]+zscale_inch;

		ps_line(xl,yl,4,3,0); 
		sprintf(label,"%.0f m",zscale);
		ps_text(xl[0]+0.3,yl[0]+zscale_inch/2.0,15,label,0.,6,0);


			/* plot an arrow in the ship's direction */
		a=0;
		b=beams_bath/2;
		while (xp[a++][b]==BAD) {}
		xl[0]=xp[--a][b];
		yl[0]=yp[a][b];
		a=irec-1;
		while (xp[a--][b]==BAD) {}
		xl[1]=xp[++a][b];
		yl[1]=yp[a][b];
		xl[1]=((xl[1]-xl[0])/distot/2)+.6;
		yl[1]=((yl[1]-yl[0])/distot/2)+min_yp*scaling-1.;
		xl[0]=0.+.6; 
		yl[0]=0.+min_yp*scaling-0.85;
		ps_vector(xl[0],yl[0],xl[1],yl[1],0.01,0.25,0.1,1.0,0,0,0,0);
		ps_text(xl[0]-1.7,yl[0]+.2,15,"ship heading",0.,1,0);
		ps_text(xl[0]-1.7,yl[0],15,"direction",0.,1,0);


			/* plot the three axes */
		for (i=0;i<3;i++) {
			xl[0]=0.;	/* point in center of page */
			yl[0]=0.;
			rotate=0;	/* set to 1 if arrow is rotated below */
			if (i==0) {	
				x=1.;		/* x-axis */
				y=0;	
				z=0;
			} else if (i==1) {	/* y-axis */
				x=0;	
				y=1.;	
				z=0;
			} else if (i==2) {	/* z-axis */
				x=0;
				y=0;	
				z=-1.;
			}

			if (viewdir=='P' || viewdir=='p') {
				xl[1]=-y-x*sin_eta*cos_alpha+xl[0];
				yl[1]= -z*cos_eta+x*sin_eta*sin_alpha+yl[0];
			} else if (viewdir=='B' || viewdir=='b') {
				xl[1]=(x+y*sin_eta*cos_alpha)+xl[0];
				yl[1]=-z*cos_eta+y*sin_eta*sin_alpha+yl[0];
			} else if (viewdir=='S' || viewdir=='s') {
				xl[1]=y+x*sin_eta*cos_alpha+xl[0];
				yl[1]=z*cos_eta-x*sin_eta*sin_alpha+yl[0];
			}

			if (yl[1]<yl[0]) {	/* rotate arrows 180 if facing downward */
				xl[1]=-xl[1];
				yl[1]=-yl[1];
				rotate=1;
			}

			xl[0]=(-3.);		/* move arrows from center to lower left corner */
			yl[0]=(min_yp*scaling-1.);
			xl[1]=xl[0]+xl[1];
			yl[1]=yl[0]+yl[1];

			ps_vector(xl[0],yl[0],xl[1],yl[1],0.01,0.25,0.1,1.0,0,0,0,0);

			if (i==0&&rotate==0)
				ps_text(xl[1],yl[1]+.15,15,"x",0.,6,0);
			else if (i==1&&rotate==0)
				ps_text(xl[1],yl[1]+.15,15,"y",0.,6,0);
			else if (i==2&&rotate==0)
				ps_text(xl[1],yl[1]+.15,15,"z",0.,6,0);
			else if (i==0&&rotate==1)
				ps_text(xl[1],yl[1]+.15,15,"-x",0.,6,0);
			else if (i==1&&rotate==1)
				ps_text(xl[1],yl[1]+.15,15,"-y",0.,6,0);
			else if (i==2&&rotate==1)
				ps_text(xl[1],yl[1]+.15,15,"z",0.,6,0);

		} /* (i=0;i<3;i++) */
	} /* if display_scales */

	
	ps_plotend(1);



	/* close the multibeam file */
	status = mb_close(verbose,&mbio_ptr,&error);

	/* deallocate memory */
	for (i=0;i<pings;i++) {
		if (error == MB_ERROR_NO_ERROR) {
			datacur = data[i];
			mb_free(verbose,&datacur->bath,&error);
			mb_free(verbose,&datacur->amp,&error);
			mb_free(verbose,&datacur->bathacrosstrack,&error);
			mb_free(verbose,&datacur->bathalongtrack,&error);
			mb_free(verbose,&datacur->ss,&error);
			mb_free(verbose,&datacur->ssacrosstrack,&error);
			mb_free(verbose,&datacur->ssalongtrack,&error);
		} /* if data[i] */
		mb_free(verbose,pings*sizeof(struct ping),
					&data[i],&error);
	} 	 /* for i */	  

}	/* main */





Polygon_Fill(xl,yl,xp,yp,irec,beams_bath,cnt,scaling)
	/* Plots a black background upon which the good polygons will be drawn */
	/* Data gaps will show through to this polygon fill */
double xl[PINGS_MAX*BEAMS_MAX], yl[PINGS_MAX*BEAMS_MAX];
int xp[PINGS_MAX][BEAMS_MAX],yp[PINGS_MAX][BEAMS_MAX];
int *cnt;
int irec,beams_bath;
double scaling;
{
	int a,b,i,pt_found;
	int beam_limit_min,beam_limit_max;
	double miny, maxy;

/* Plot the underlying black fill as 4 small pieces and as 1 big piece. */
/* This will cause better coverage of the area but gaps may still occur */

  for (i=0;i<=4;i++) { 
  
  *cnt=0;

	if (i==0) {
		beam_limit_min=0;
		beam_limit_max=beams_bath/4;
	} else if (i==1) {
		beam_limit_min=beam_limit_max-1;
		beam_limit_max=beams_bath/2;
	} else if (i==2) {
		beam_limit_min=beam_limit_max-1;
		beam_limit_max=beams_bath*3/4;
	} else if (i==3) {
		beam_limit_min=beam_limit_max-1;
		beam_limit_max=beams_bath;
	}else if (i==4) {
		beam_limit_min=0;
		beam_limit_max=beams_bath;
	}

	pt_found=0;
	b=beam_limit_min;
	while (!(pt_found)) {
		for(a=0;a<irec;a++) {
			if (xp[a][b]!=BAD) {
				pt_found++;
				xl[(*cnt)]=xp[a][b]*scaling;
				yl[(*cnt)++]=yp[a][b]*scaling;
			}
		}
		b++;
	}

	pt_found=0;
	a=irec-1;
	while (!(pt_found)) {
		for(b=beam_limit_min+1;b<beam_limit_max;b++) {
			if (xp[a][b]!=BAD) {
				pt_found++;
				xl[(*cnt)]=xp[a][b]*scaling;
				yl[(*cnt)++]=yp[a][b]*scaling;
			}
		}
		a--;
	}
	
	pt_found=0;
	b=beam_limit_max-1;
	while (!(pt_found)) {
		for(a=irec-2;a>=0;a--) {
			if (xp[a][b]!=BAD) {
				pt_found++;
				xl[(*cnt)]=xp[a][b]*scaling;
				yl[(*cnt)++]=yp[a][b]*scaling;
			}
		}
		b--;
	}

	pt_found=0;
	a=0;
	while (!(pt_found)) {
		for(b=beam_limit_max-2;b>beam_limit_min;b--) {
			if (xp[a][b]!=BAD) {
				pt_found++;
				xl[(*cnt)]=xp[a][b]*scaling;
				yl[(*cnt)++]=yp[a][b]*scaling;
			}
		}
		a++;
	}

  ps_polygon(xl,yl,*cnt,0,0,0,1);

  }  /* for loop */

		
} /* Poloygon_Fill */




Good_Polygon(xl,yl,xp,yp,i,jb,irec,beams_bath,scaling)
	/* DRAW THE 8 TRIANGULAR POLYGONS AROUND THE POINT IF 3 VERTICES ARE GOOD */
int xp[PINGS_MAX][BEAMS_MAX],yp[PINGS_MAX][BEAMS_MAX];
double xl[PINGS_MAX*BEAMS_MAX], yl[PINGS_MAX*BEAMS_MAX];
int i,jb,irec,beams_bath;
double scaling;
{

/* Polygon 1 */
	xl[0]=xp[i][jb]*scaling;		yl[0]=yp[i][jb]*scaling;
	xl[1]=xp[i-1][jb-1]*scaling;		yl[1]=yp[i-1][jb-1]*scaling;
	xl[2]=xp[i][jb-1]*scaling;		yl[2]=yp[i][jb-1]*scaling;
	if ((i-1)>=0&&(jb-1)>=0&&xp[i][jb]!=BAD&&xp[i-1][jb-1]!=BAD&&xp[i][jb-1]!=BAD) {
		ps_polygon(xl,yl,3,255,255,255,1);
	} 

/* Polygon 2 */
	xl[0]=xp[i][jb]*scaling;		yl[0]=yp[i][jb]*scaling;
	xl[1]=xp[i-1][jb]*scaling;		yl[1]=yp[i-1][jb]*scaling;
	xl[2]=xp[i-1][jb-1]*scaling;		yl[2]=yp[i-1][jb-1]*scaling;
	if ((i-1)>=0&&(jb-1)>=0&&xp[i][jb]!=BAD&&xp[i-1][jb]!=BAD&&xp[i-1][jb-1]!=BAD) {
		ps_polygon(xl,yl,3,255,255,255,1);
	} 

/* Polygon 3 */
	xl[0]=xp[i][jb]*scaling;		yl[0]=yp[i][jb]*scaling;
	xl[1]=xp[i-1][jb+1]*scaling;		yl[1]=yp[i-1][jb+1]*scaling;
	xl[2]=xp[i-1][jb]*scaling;		yl[2]=yp[i-1][jb]*scaling;
	if ((i-1)>=0&&(jb+1)<beams_bath&&xp[i][jb]!=BAD&&xp[i-1][jb+1]!=BAD&&xp[i-1][jb]!=BAD) {
		ps_polygon(xl,yl,3,255,255,255,1);
	} 

/* Polygon 4 */
	xl[0]=xp[i][jb]*scaling;		yl[0]=yp[i][jb]*scaling;
	xl[1]=xp[i][jb+1]*scaling;		yl[1]=yp[i][jb+1]*scaling;
	xl[2]=xp[i-1][jb+1]*scaling;		yl[2]=yp[i-1][jb+1]*scaling;
	if ((i-1)>=0&&(jb+1)<beams_bath&&xp[i][jb]!=BAD&&xp[i][jb+1]!=BAD&&xp[i-1][jb+1]!=BAD) {
		ps_polygon(xl,yl,3,255,255,255,1);
	}

/* Polygon 5 */
	xl[0]=xp[i][jb]*scaling;		yl[0]=yp[i][jb]*scaling;
	xl[1]=xp[i+1][jb+1]*scaling;		yl[1]=yp[i+1][jb+1]*scaling;
	xl[2]=xp[i][jb+1]*scaling;		yl[2]=yp[i][jb+1]*scaling;
	if ((i+1)<irec&&(jb+1)<beams_bath&&xp[i][jb]!=BAD&&xp[i+1][jb+1]!=BAD&&xp[i][jb+1]!=BAD) {
		ps_polygon(xl,yl,3,255,255,255,1);
	} 

/* Polygon 6 */
	xl[0]=xp[i][jb]*scaling;		yl[0]=yp[i][jb]*scaling;
	xl[1]=xp[i+1][jb]*scaling;		yl[1]=yp[i+1][jb]*scaling;
	xl[2]=xp[i+1][jb+1]*scaling;		yl[2]=yp[i+1][jb+1]*scaling;
	if ((i+1)<irec&&(jb+1)<beams_bath&&xp[i][jb]!=BAD&&xp[i+1][jb]!=BAD&&xp[i+1][jb+1]!=BAD) {
		ps_polygon(xl,yl,3,255,255,255,1);
	}

/* Polygon 7 */
	xl[0]=xp[i][jb]*scaling;		yl[0]=yp[i][jb]*scaling;
	xl[1]=xp[i+1][jb-1]*scaling;		yl[1]=yp[i+1][jb-1]*scaling;
	xl[2]=xp[i+1][jb]*scaling;		yl[2]=yp[i+1][jb]*scaling;
	if ((i+1)<irec&&(jb-1)>=0&&xp[i][jb]!=BAD&&xp[i+1][jb-1]!=BAD&&xp[i+1][jb]!=BAD) {
		ps_polygon(xl,yl,3,255,255,255,1);
	} 

/* Polygon 8 */
	xl[0]=xp[i][jb]*scaling;		yl[0]=yp[i][jb]*scaling;
	xl[1]=xp[i][jb-1]*scaling;		yl[1]=yp[i][jb-1]*scaling;
	xl[2]=xp[i+1][jb-1]*scaling;		yl[2]=yp[i+1][jb-1]*scaling;
	if ((i+1)<irec&&(jb-1)>=0&&xp[i][jb]!=BAD&&xp[i][jb-1]!=BAD&&xp[i+1][jb-1]!=BAD) {
		ps_polygon(xl,yl,3,255,255,255,1);
	} 
 
}	/* Good_Polygon */
 

/*--------------------------------------------------------------------
 *    The MB-system:	mbswath.c	5/30/93
 *    $Id: mbswath.c,v 4.10 1995-03-06 19:40:17 caress Exp $
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
 * MBSWATH is a GMT compatible utility which creates a color postscript
 * image of multibeam swath bathymetry or backscatter data.  The image
 * may be shaded relief as well.  Complete maps are made by using
 * MBSWATH in conjunction with the usual GMT programs.  The modes
 * of operation are:
 *   Mode 1:  Bathymetry
 *   Mode 2:  Bathymetry shaded by illumination
 *   Mode 3:  Bathymetry shaded by amplitude
 *   Mode 4:  amplitude
 *   Mode 5:  sidescan
 *
 * Author:	D. W. Caress
 * Date:	May 30, 1993
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.9  1995/01/18  22:14:49  caress
 * Fixed bug regarding the calculation of footprints for
 * sidescan data.
 *
 * Revision 4.8  1995/01/10  15:46:08  caress
 * Made plotting using the fore-aft beam width the default.
 *
 * Revision 4.7  1995/01/05  23:59:20  caress
 * Made it possible to read data from a single file or
 * from datalists.
 *
 * Revision 4.6  1994/12/21  20:23:30  caress
 * Allows the alongtrack beam footprint to be determined
 * by alongtrack beam width in degrees. The alongtrack
 * beam width is set using a negative argument in the
 * -A option. This will be made the standard mode soon,
 * but is an undocumented feature for now.
 *
 * Revision 4.5  1994/10/25  12:32:41  caress
 * Fixed memory overwrite that causing early exits.
 *
 * Revision 4.4  1994/10/21  16:08:22  caress
 * Release V4.0
 *
 * Revision 4.3  1994/10/21  11:34:20  caress
 * Release V4.0
 *
 * Revision 4.2  1994/07/29  19:04:31  caress
 * Changes associated with supporting byte swapped Lynx OS and
 * >> using unix second time base.
 *
 * Revision 4.1  1994/06/13  18:39:15  caress
 * Made it possible to read data from stdin.
 *
 * Revision 4.0  1994/03/05  23:46:48  caress
 * First cut at version 4.0
 *
 * Revision 4.2  1994/03/03  03:48:58  caress
 * Fixed copyright message.
 *
 * Revision 4.1  1994/03/02  22:21:45  caress
 * Added capability of plotting bathymetry shaded by amplitude,
 * amplitude, or sidescan data.
 *
 * Revision 4.0  1994/02/26  23:07:51  caress
 * First cut at new version.
 *
 * Revision 3.3  1993/11/27  03:03:37  caress
 * Fixed major bug in shaded relief plotting.  Before, only the
 * along track derivative was being used because the drvx and drvy
 * x and y derivative components were set to zero after the
 * across-track derivative was calculated.
 * ..
 *
 * Revision 3.2  1993/11/03  21:09:10  caress
 * Changed ps_plotinit call to agree with current version
 * of GMT (v2.1.4).
 *
 * Revision 3.1  1993/06/20  23:08:30  caress
 * Fixed help message.
 *
 * Revision 3.0  1993/06/19  01:16:43  caress
 * Initial version.
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"

/* GMT include files */
#include "gmt.h"

/* min max defines */
#define      min(A, B)       ((A) < (B) ? (A) : (B))
#define      max(A, B)       ((A) > (B) ? (A) : (B))

/* MBSWATH MODE DEFINES */
#define	MBSWATH_BATH		1
#define	MBSWATH_BATH_RELIEF	2
#define	MBSWATH_BATH_AMP	3
#define	MBSWATH_AMP		4
#define	MBSWATH_SS		5
#define MBSWATH_FOOTPRINT_REAL	1
#define MBSWATH_FOOTPRINT_FAKE	2

/* DTR define */
#ifndef M_PI
#define	M_PI	3.14159265358979323846
#endif
#define DTR	(M_PI/180.)

/* global structure definitions */
#define MAXPINGS 50
struct	footprint
	{
	double	x[4];
	double	y[4];
	};
struct	ping
	{
	int	pings;
	int	kind;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	*bath;
	double	*bathlon;
	double	*bathlat;
	double	*amp;
	double	*ss;
	double	*sslon;
	double	*sslat;
	char	comment[256];
	double	lonaft;
	double	lataft;
	double	lonfor;
	double	latfor;
	int	*bathflag;
	struct footprint *bathfoot;
	int	*ssflag;
	struct footprint *ssfoot;
	double	*bathshade;
	};
struct swath
	{
	int	npings;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	struct ping data[MAXPINGS];
	};

/* image type defines */
#define	MBSWATH_IMAGE_VECTOR	1
#define	MBSWATH_IMAGE_8		2
#define	MBSWATH_IMAGE_24	3

/* global image variables and defines */
#define YIQ(r,g,b)	rint(0.299*(r) + 0.587*(b) + 0.114*(b))
int	image = MBSWATH_IMAGE_VECTOR;
unsigned char *bitimage;
int	dpi = 100;
double	x_inch, y_inch;
double	xo, yo;
int	nx, ny, nm, nm2;
unsigned char r, g, b, gray;

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
	static char rcs_id[] = "$Id: mbswath.c,v 4.10 1995-03-06 19:40:17 caress Exp $";
	static char program_name[] = "MBSWATH";
	static char help_message[] =  "MBSWATH is a GMT compatible utility which creates a color postscript \nimage of multibeam swath bathymetry or backscatter data.  The image \nmay be shaded relief as well.  Complete maps are made by using \nMBSWATH in conjunction with the usual GMT programs.";
	static char usage_message[] = "mbswath -Ccptfile -Jparameters -Rwest/east/south/north [-Afactor -Btickinfo -byr/mon/day/hour/min/sec -Dmode/ampscale/ampmin/ampmax -Eyr/mon/day/hour/min/sec -fformat -Fred/green/blue -Gmagnitude/azimuth -Idatalist -K -M -O -P -ppings -Qdpi -U -Xx-shift -Yy-shift -Zmode -#copies -V -H]";

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
	char	*message = NULL;

	/* MBIO read control parameters */
	char	read_file[128];
        int     read_datalist = MB_NO;
	int	read_data;
	FILE	*fp;
	int	format;
	int	format_num;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	char	file[128];
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	char	*mbio_ptr = NULL;

	/* mbio read values */
	struct swath *swath_plot = NULL;
	struct ping *pingcur = NULL;
	double	*bath = NULL;
	double	*bathlon = NULL;
	double	*bathlat = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*sslon = NULL;
	double	*sslat = NULL;
	int	*bathflag = NULL;
	double	*bathfoot = NULL;
	int	*ssflag = NULL;
	double	*ssfoot = NULL;

	/* gmt control variables */
	double	borders[4];
	char	cptfile[128];
	double	magnitude = 1.0;
	double	azimuth = 90.0;
	int	ampscale_mode = 0;
	double	ampscale = 1.0;
	double	ampmin = 0.0;
	double	ampmax = 1.0;
	int	monochrome = MB_NO;
	int	footprint_mode = MBSWATH_FOOTPRINT_REAL;
	double	rawfactor = 1.0;
	double	factor;
	double	default_depth = 3000.0;
	int	mode = MBSWATH_BATH;
	int	start;
	int	plot;
	int	done;
	int	flush;
	int	save_new;
	int	first;
	int	*npings;
	int	nping_read;
	int	nplot;
	double	amplog;
	double	mtodeglon, mtodeglat;
	double	clipx[4], clipy[4];

	/* other variables */
	int	i, j;
	char	line[128];

	/* get current mb default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to stdin */
	strcpy (read_file, "stdin");

	/* initialize some values */
	format = -1;
        read_datalist = MB_NO;
	strcpy (cptfile,"cpt");
	borders[0] = 0.0;
	borders[1] = 0.0;
	borders[2] = 0.0;
	borders[3] = 0.0;

	/* deal with gmt options */
	gmt_begin (argc, argv);
	for (i = 1; i < argc; i++) 
		{
		if (argv[i][0] == '-') 
			{
			switch (argv[i][1]) 
				{
				/* Common parameters */
			
				case 'B':
				case 'J':
				case 'K':
				case 'O':
				case 'P':
				case 'R':
				case 'U':
				case 'V':
				case 'X':
				case 'x':
				case 'Y':
				case 'y':
				case '#':
				case '\0':
					errflg += get_common_args (argv[i], 
						&borders[0], &borders[1], 
						&borders[2], &borders[3]);
					break;
				
				/* Supplemental parameters */
			
				case 'F':
					sscanf (&argv[i][2], "%d/%d/%d",&gmtdefs.basemap_frame_rgb[0],
						&gmtdefs.basemap_frame_rgb[1], &gmtdefs.basemap_frame_rgb[2]);
					break;
				case 'C':
					strcpy(cptfile,&argv[i][2]);
					break;
				case 'M':
					monochrome = MB_YES;
					break;
				case 'Q':
					image = MBSWATH_IMAGE_24;
					if (argv[i][2])
					  dpi = atoi (&argv[i][2]);
					break;
				case '0':
					gmtdefs.color_image = 0;
					break;
				case '1':
					gmtdefs.color_image = 1;
					break;
				case '2':
					gmtdefs.color_image = 2;
					break;
				case '3':
					gmtdefs.color_image = 3;
					break;
			}
			}
		}

	/* if error flagged then print it and exit */
	if (errflg)
		{
		fprintf(stderr,"usage: %s\n", usage_message);
		fprintf(stderr,"GMT option error\n");
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_FAILURE);
		}

	/* deal with mb options */
	while ((c = getopt(argc, argv, "A:a:B:b:C:D:d:E:e:F:f:G:g:HhI:i:J:KL:l:MOPp:Q:R:S:s:T:t:UVvX:x:Y:y:Z:z:#:0123")) != -1)
	  switch (c) 
		{
		case 'A':
		case 'a':
			sscanf (optarg,"%lf/%d/%lf", &factor, 
				&footprint_mode, &default_depth);
			flag++;
			break;
		case 'b':
			sscanf (optarg, "%d/%d/%d/%d/%d/%d",
				&btime_i[0],&btime_i[1],&btime_i[2],
				&btime_i[3],&btime_i[4],&btime_i[5]);
			btime_i[6] = 0;
			break;
		case 'D':
		case 'd':
			sscanf (optarg, "%d/%lf/%lf/%lf",
				&ampscale_mode,&ampscale,&ampmin,&ampmax);
			break;
		case 'E':
		case 'e':
			sscanf (optarg, "%d/%d/%d/%d/%d/%d",
				&etime_i[0],&etime_i[1],&etime_i[2],
				&etime_i[3],&etime_i[4],&etime_i[5]);
			etime_i[6] = 0;
			break;
                case 'f':
                        sscanf (optarg, "%d",&format);
                        break;
		case 'G':
		case 'g':
			sscanf (optarg,"%lf/%lf", &magnitude,&azimuth);
			flag++;
			break;
		case 'H':
		case 'h':
			help++;
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
		case 'p':
			sscanf (optarg,"%d", &pings);
			flag++;
			break;
		case 'S':
		case 's':
			sscanf (optarg, "%lf", &speedmin);
			break;
		case 'T':
		case 't':
			sscanf (optarg,"%lf", &timegap);
			flag++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'Z':
		case 'z':
			sscanf (optarg,"%d", &mode);
			flag++;
			break;
		case 'B':
		case 'C':
		case 'F':
		case 'J':
		case 'K':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'U':
		case 'X':
		case 'x':
		case 'Y':
		case 'y':
		case '#':
		case '0':
		case '1':
		case '2':
		case '3':
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
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		fprintf(stderr,"dbg2       help:             %d\n",help);
		fprintf(stderr,"dbg2       format:           %d\n",format);
		fprintf(stderr,"dbg2       pings:            %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:          %d\n",lonflip);
		fprintf(stderr,"dbg2       btime_i[0]:       %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]:       %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]:       %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]:       %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]:       %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]:       %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       btime_i[6]:       %d\n",btime_i[6]);
		fprintf(stderr,"dbg2       etime_i[0]:       %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:       %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:       %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:       %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:       %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:       %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       etime_i[6]:       %d\n",etime_i[6]);
		fprintf(stderr,"dbg2       speedmin:         %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:          %f\n",timegap);
		fprintf(stderr,"dbg2       input file:       %s\n",read_file);
		fprintf(stderr,"dbg2       borders[0]:       %f\n",borders[0]);
		fprintf(stderr,"dbg2       borders[1]:       %f\n",borders[1]);
		fprintf(stderr,"dbg2       borders[2]:       %f\n",borders[2]);
		fprintf(stderr,"dbg2       borders[3]:       %f\n",borders[3]);
		fprintf(stderr,"dbg2       shade magnitude:  %f\n",magnitude);
		fprintf(stderr,"dbg2       shade azimuth:    %f\n",azimuth);
		fprintf(stderr,"dbg2       amp scale mode:   %d\n",ampscale_mode);
		fprintf(stderr,"dbg2       amplitude scale:  %f\n",ampscale);
		fprintf(stderr,"dbg2       amplitude minimum:%f\n",ampmin);
		fprintf(stderr,"dbg2       amplitude maximum:%f\n",ampmax);
		fprintf(stderr,"dbg2       footprint mode:   %f\n",footprint_mode);
		fprintf(stderr,"dbg2       footprint factor: %f\n",factor);
		fprintf(stderr,"dbg2       default depth:    %f\n",default_depth);
		fprintf(stderr,"dbg2       mode:             %d\n",mode);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(MB_ERROR_NO_ERROR);
		}

	/* if borders not specified then quit */
	if (borders[0] >= borders[1] || borders[2] >= borders[3]
		|| borders[2] <= -90.0 || borders[3] >= 90.0)
		{
		fprintf(stderr,"\nRegion borders not properly specified:\n\t%f %f %f %f\n",borders[0],borders[1],borders[2],borders[3]);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_ERROR_BAD_PARAMETER);
		}

	/* set bounds for multibeam reading larger than
		map borders */
	bounds[0] = borders[0] - 0.25*(borders[1] - borders[0]);
	bounds[1] = borders[1] + 0.25*(borders[1] - borders[0]);
	bounds[2] = borders[2] - 0.25*(borders[3] - borders[2]);
	bounds[3] = borders[3] + 0.25*(borders[3] - borders[2]);

	/* set up map */
	map_setup(borders[0],borders[1],borders[2],borders[3]);

	/* get scaling from degrees to km */
	mb_coor_scale(verbose,0.5*(borders[2] + borders[3]),
			&mtodeglon,&mtodeglat);

	/* get color palette file */
	read_cpt(cptfile);
	if (gmt_b_and_w || monochrome == MB_YES) 
		image = MBSWATH_IMAGE_8;
	if (gmt_n_colors <= 0)
		{
		fprintf(stderr,"\nColor pallette table not properly specified:\n");
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_ERROR_BAD_PARAMETER);
		}

	/* initialize plotting */
	ps_plotinit (NULL, gmtdefs.overlay, gmtdefs.page_orientation, 
		gmtdefs.x_origin, gmtdefs.y_origin,
		gmtdefs.global_x_scale, gmtdefs.global_y_scale, 
		gmtdefs.n_copies, gmtdefs.dpi, gmtdefs.measure_unit, 
		gmtdefs.paper_width, gmtdefs.page_rgb, gmt_epsinfo (argv[0]));
	echo_command (argc, argv);
	if (gmtdefs.unix_time) 
		timestamp (TIME_STAMP_X, TIME_STAMP_Y, argc, argv);

	/* set clip path */
	geo_to_xy(borders[0],borders[2],&clipx[0],&clipy[0]);
	geo_to_xy(borders[1],borders[2],&clipx[1],&clipy[1]);
	geo_to_xy(borders[1],borders[3],&clipx[2],&clipy[2]);
	geo_to_xy(borders[0],borders[3],&clipx[3],&clipy[3]);
	ps_clipon(clipx,clipy,4,-1,-1,-1,3);

	/* if plot is made using an image operator
		set up the image */
	if (image == MBSWATH_IMAGE_8 || image == MBSWATH_IMAGE_24)
		{
		x_inch = clipx[1] - clipx[0];
		y_inch = clipy[2] - clipy[1];
		xo = 0.0;
		yo = 0.0;
		nx = x_inch*dpi;
		ny = y_inch*dpi;
		nm = nx*ny;
		nm2 = 2*nm;

		if (image == MBSWATH_IMAGE_8)
			{
			/* allocate image */
			status = mb_malloc(verbose,nm*sizeof(char),
					&bitimage,&error);

			/* set image to background color */
			gray = YIQ (gmtdefs.page_rgb[0], 
				gmtdefs.page_rgb[1], 
				gmtdefs.page_rgb[2]);
			for (j=0;j<nm;j++)
				bitimage[j] = gray;
			}
		else
			{
			/* allocate image */
			status = mb_malloc(verbose,3*nm*sizeof(char),
					&bitimage,&error);

			/* set image to background color */
			j = 0;
			while (j < 3*nm) 
				{
				bitimage[j++] = gmtdefs.page_rgb[0];
				bitimage[j++] = gmtdefs.page_rgb[1];
				bitimage[j++] = gmtdefs.page_rgb[2];
				}
			}
		}

	/* determine whether to read one file or a list of files */
	if (format < 0)
		read_datalist = MB_YES;

	/* open file list */
	if (read_datalist == MB_YES)
	    {
	    if ((fp = fopen(read_file,"r")) == NULL)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open data list file: %s\n",
			read_file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	    if (fgets(line,128,fp) != NULL
		&& sscanf(line,"%s %d",file,&format) == 2)
		read_data = MB_YES;
	    else
		read_data = MB_NO;
	    }
	else
	    {
	    strcpy(file,read_file);
	    read_data = MB_YES;
	    }

	/* loop over files in file list */
	if (verbose == 1) 
		fprintf(stderr,"\n");
	while (read_data == MB_YES)
	{
	/* initialize reading the multibeam file */
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

	/* get fore-aft beam_width */
	status = mb_format(verbose,&format,&format_num,&error);
	if (footprint_mode == MBSWATH_FOOTPRINT_REAL)
		factor = rawfactor*mb_foreaft_beamwidth_table[format_num];
	else
		factor = rawfactor;

	/* allocate memory for data arrays */
	status = mb_malloc(verbose,sizeof(struct swath),
			&swath_plot,&error);
	npings = &swath_plot->npings;
	for (i=0;i<MAXPINGS;i++)
		{
		pingcur = &(swath_plot->data[i]);
		pingcur->bath = NULL;
		pingcur->amp = NULL;
		pingcur->bathlon = NULL;
		pingcur->bathlat = NULL;
		pingcur->ss = NULL;
		pingcur->sslon = NULL;
		pingcur->sslat = NULL;
		pingcur->bathflag = NULL;
		pingcur->bathfoot = NULL;
		pingcur->ssflag = NULL;
		pingcur->ssfoot = NULL;
		pingcur->bathshade = NULL;
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&(pingcur->bath),&error);
		status = mb_malloc(verbose,beams_amp*sizeof(double),
			&(pingcur->amp),&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&(pingcur->bathlon),&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&(pingcur->bathlat),&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&(pingcur->ss),&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&(pingcur->sslon),&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&(pingcur->sslat),&error);
		status = mb_malloc(verbose,beams_bath*sizeof(int),
			&(pingcur->bathflag),&error);
		status = mb_malloc(verbose,
			(beams_bath)*sizeof(struct footprint),
			&(pingcur->bathfoot),&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(int),
			&(pingcur->ssflag),&error);
		status = mb_malloc(verbose,
			(pixels_ss)*sizeof(struct footprint),
			&(pingcur->ssfoot),&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&(pingcur->bathshade),&error);
		}

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* print message */
	if (verbose >= 2) 
		fprintf(stderr,"\n");
	if (verbose >= 1)
		fprintf(stderr,"processing data in %s...\n",file);

	/* loop over reading */
	*npings = 0;
	start = MB_YES;
	done = MB_NO;
	while (done == MB_NO)
		{
		pingcur = &swath_plot->data[*npings];
		bath = pingcur->bath;
		amp = pingcur->amp;
		bathlon = pingcur->bathlon;
		bathlat = pingcur->bathlat;
		ss = pingcur->ss;
		sslon = pingcur->sslon;
		sslat = pingcur->sslat;
		status = mb_read(verbose,mbio_ptr,&(pingcur->kind),
			&(pingcur->pings),pingcur->time_i,&(pingcur->time_d),
			&(pingcur->navlon),&(pingcur->navlat),&(pingcur->speed),
			&(pingcur->heading),&(pingcur->distance),
			&beams_bath,&beams_amp,&pixels_ss,
			bath,amp,bathlon,bathlat,
			ss,sslon,sslat,
			pingcur->comment,&error);
		swath_plot->beams_bath = beams_bath;
		swath_plot->beams_amp = beams_amp;
		swath_plot->pixels_ss = pixels_ss;

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",
				program_name);
			fprintf(stderr,"dbg2       kind:           %d\n",
				pingcur->kind);
			fprintf(stderr,"dbg2       beams_bath:     %d\n",
				beams_bath);
			fprintf(stderr,"dbg2       beams_amp:      %d\n",
					beams_amp);
			fprintf(stderr,"dbg2       pixels_ss:      %d\n",
					pixels_ss);
			fprintf(stderr,"dbg2       error:          %d\n",
				error);
			fprintf(stderr,"dbg2       status:         %d\n",
				status);
			}

		/* update bookkeeping */
		if (error == MB_ERROR_NO_ERROR)
			{
			nping_read += pingcur->pings;
			(*npings)++;
			}

		/* scale amplitudes if necessary */
		if (error == MB_ERROR_NO_ERROR
			&& (mode == MBSWATH_BATH_AMP
			|| mode == MBSWATH_AMP)
			&& ampscale_mode > 0)
			{
			for (i=0;i<beams_amp;i++)
				{
				if (amp[i] > 0 && ampscale_mode == 1)
				    {
				    amp[i] = ampscale*(amp[i] - ampmin)
					/(ampmax - ampmin);
				    }
				else if (amp[i] > 0 && ampscale_mode == 2)
				    {
				    amp[i] = min(amp[i],ampmax);
				    amp[i] = max(amp[i],ampmin);
				    amp[i] = ampscale*(amp[i] - ampmin)
					/(ampmax - ampmin);
				    }
				else if (amp[i] > 0 && ampscale_mode == 3)
				    {
				    amplog = 20.0*log10(amp[i]);
				    amp[i] = ampscale*(amplog - ampmin)
					/(ampmax - ampmin);
				    }
				else if (amp[i] > 0 && ampscale_mode == 4)
				    {
				    amplog = 20.0*log10(amp[i]);
				    amplog = min(amplog,ampmax);
				    amplog = max(amplog,ampmin);
				    amp[i] = ampscale*(amplog - ampmin)
					   /(ampmax - ampmin);
				    }
				}
			}

		/* scale sidescan if necessary */
		if (error == MB_ERROR_NO_ERROR
			&& mode == MBSWATH_SS
			&& ampscale_mode > 0)
			{
			for (i=0;i<pixels_ss;i++)
				{
				if (ss[i] > 0 && ampscale_mode == 1)
				    {
				    ss[i] = ampscale*(ss[i] - ampmin)
					/(ampmax - ampmin);
				    }
				else if (ss[i] > 0 && ampscale_mode == 2)
				    {
				    ss[i] = min(ss[i],ampmax);
				    ss[i] = max(ss[i],ampmin);
				    ss[i] = ampscale*(ss[i] - ampmin)
					/(ampmax - ampmin);
				    }
				else if (ss[i] > 0 && ampscale_mode == 3)
				    {
				    amplog = 20.0*log10(ss[i]);
				    ss[i] = ampscale*(amplog - ampmin)
					/(ampmax - ampmin);
				    }
				else if (ss[i] > 0 && ampscale_mode == 4)
				    {
				    amplog = 20.0*log10(ss[i]);
				    amplog = min(amplog,ampmax);
				    amplog = max(amplog,ampmin);
				    ss[i] = ampscale*(amplog - ampmin)
					   /(ampmax - ampmin);
				    }
				}
			}

		/* decide whether to plot, whether to 
			save the new ping, and if done */
		plot = MB_NO; 
		flush = MB_NO;
		if (*npings >= MAXPINGS)
			plot = MB_YES;
		if (*npings > 0 
			&& (error > MB_ERROR_NO_ERROR
			|| error == MB_ERROR_TIME_GAP
			|| error == MB_ERROR_OUT_BOUNDS
			|| error == MB_ERROR_OUT_TIME
			|| error == MB_ERROR_SPEED_TOO_SMALL))
			{
			plot = MB_YES;
			flush = MB_YES;
			}
		save_new = MB_NO;
		if (error == MB_ERROR_TIME_GAP)
			save_new = MB_YES;
		if (error > MB_ERROR_NO_ERROR)
			done = MB_YES;

		/* if enough pings read in, plot them */
		if (plot == MB_YES)
			{
			/* get footprint locations */
			status = get_footprints(verbose,mode,
				footprint_mode,factor,default_depth, 
				swath_plot,mtodeglon,mtodeglat,&error);

			/* get shading */
			if (mode == MBSWATH_BATH_RELIEF 
				|| mode == MBSWATH_BATH_AMP)
				status = get_shading(verbose,mode,swath_plot,
					mtodeglon,mtodeglat,
					magnitude,azimuth,&error);

			/* plot data */
			if (start == MB_YES)
				{
				first = 0;
				start = MB_NO;
				}
			else
				first = 1;
			if (done == MB_YES)
				nplot = *npings - first;
			else
				nplot = *npings - first - 1;
			status = plot_data(verbose,mode,swath_plot,
				first,nplot,&error);

			/* let the world know */
			/*if (verbose >= 1)
			    fprintf(stderr,"plotted %d pings...\n",nplot);*/

			/* reorganize data */
			if (flush == MB_YES && save_new == MB_YES)
				{
				status = ping_copy(verbose,0,*npings,
					swath_plot,&error);
				*npings = 1;
				start = MB_YES;
				}
			else if (flush == MB_YES)
				{
				*npings = 0;
				start = MB_YES;
				}
			else if (*npings > 1)
				{
				for (i=0;i<2;i++)
					status = ping_copy(verbose,i,
						*npings-2+i,
						swath_plot,&error);
				*npings = 2;
				}

			}
		}
	status = mb_close(verbose,&mbio_ptr,&error);

	/* deallocate memory for data arrays */
	for (i=0;i<MAXPINGS;i++)
		{
		pingcur = &swath_plot->data[i];
		mb_free(verbose,&pingcur->bath,&error);
		mb_free(verbose,&pingcur->amp,&error);
		mb_free(verbose,&pingcur->bathlon,&error);
		mb_free(verbose,&pingcur->bathlat,&error);
		mb_free(verbose,&pingcur->ss,&error);
		mb_free(verbose,&pingcur->sslon,&error);
		mb_free(verbose,&pingcur->sslat,&error);
		mb_free(verbose,&pingcur->bathflag,&error);
		mb_free(verbose,&pingcur->bathfoot,&error);
		mb_free(verbose,&pingcur->ssflag,&error);
		mb_free(verbose,&pingcur->ssfoot,&error);
		mb_free(verbose,&pingcur->bathshade,&error);
		}
	mb_free(verbose,&swath_plot,&error);

	/* figure out whether and what to read next */
        if (read_datalist == MB_YES)
                {
                if (fgets(line,128,fp) != NULL
                        && sscanf(line,"%s %d",file,&format) == 2)
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
		fclose (fp);

	/* turn off clipping */
	ps_clipoff();

	/* plot image if one is used */
	if (image == MBSWATH_IMAGE_8)
		{
		ps_image (0., 0., x_inch, y_inch, bitimage, nx, ny, 8);
		}
	else if (image == MBSWATH_IMAGE_24)
		{
		color_image (0., 0., x_inch, y_inch, bitimage, nx, ny);
		}
		
	/* plot basemap if required */
	if (frame_info.plot) 
		{
		ps_setpaint (gmtdefs.basemap_frame_rgb[0], 
			gmtdefs.basemap_frame_rgb[1], 
			gmtdefs.basemap_frame_rgb[2]);
		map_basemap ();
		ps_setpaint (0, 0, 0);
		}

	/* end the plot */
	ps_plotend (gmtdefs.last_page);

	/* deallocate image */
	if (image == MBSWATH_IMAGE_8 || image == MBSWATH_IMAGE_24)
		mb_free(verbose,&bitimage,&error);

	/* check memory */
	if (verbose >= 2)
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
	gmt_end(argc, argv);
}
/*--------------------------------------------------------------------*/
int get_footprints(verbose,mode,fp_mode,factor,depth_def,
			swath,mtodeglon,mtodeglat,error)
int	verbose;
int	mode;
int	fp_mode;
double	factor;
double	depth_def;
struct swath *swath;
double	mtodeglon;
double	mtodeglat;
int	*error;
{
	char	*function_name = "get_footprints";
	int	status = MB_SUCCESS;

	struct ping	*pingcur;
	struct footprint	*print;
	int	dobath, doss;
	double	headingx, headingy;
	double	dx, dy, r, dlon1, dlon2, dlat1, dlat2, tt, x, y;
	double	ddlonx, ddlaty, rfactor;
	double	dddepth = 0.0;
	int	setprint;
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:     %d\n",verbose);
		fprintf(stderr,"dbg2       mode:        %d\n",mode);
		fprintf(stderr,"dbg2       fp mode:     %d\n",fp_mode);
		fprintf(stderr,"dbg2       factor:      %f\n",factor);
		fprintf(stderr,"dbg2       depth_def:   %f\n",depth_def);
		fprintf(stderr,"dbg2       swath:       %d\n",swath);
		fprintf(stderr,"dbg2       mtodeglon:   %f\n",mtodeglon);
		fprintf(stderr,"dbg2       mtodeglat:   %f\n",mtodeglat);
		fprintf(stderr,"dbg2       pings:       %d\n",swath->npings);
		}

	/* set mode of operation */
	if (mode != MBSWATH_SS)
		{
		dobath = MB_YES;
		doss = MB_NO;
		}
	else
		{
		dobath = MB_NO;
		doss = MB_YES;
		}

	/* set all footprint flags to zero */
	for (i=0;i<swath->npings;i++)
		{
		pingcur = &swath->data[i];
		for (j=0;j<swath->beams_bath;j++)
			pingcur->bathflag[j] = MB_NO;
		for (j=0;j<swath->pixels_ss;j++)
			pingcur->ssflag[j] = MB_NO;
		}

	/* get fore-aft components of beam footprints */
	if (swath->npings > 1 && fp_mode == MBSWATH_FOOTPRINT_FAKE)
	  {
	  for (i=0;i<swath->npings;i++)
		{
		/* initialize */
		pingcur = &swath->data[i];
		pingcur->lonaft = 0.0;
		pingcur->lataft = 0.0;
		pingcur->lonfor = 0.0;
		pingcur->latfor = 0.0;

		/* get aft looking
		if (i > 0)
			{
			headingx = sin(pingcur->heading*DTR);
			headingy = cos(pingcur->heading*DTR);
			dx = (swath->data[i-1].navlon - pingcur->navlon)
				/mtodeglon;
			dy = (swath->data[i-1].navlat - pingcur->navlat)
				/mtodeglat;
			r = sqrt(dx*dx + dy*dy);
			pingcur->lonaft = factor*r*headingx*mtodeglon;
			pingcur->lataft = factor*r*headingy*mtodeglat;
			}

		/* get forward looking */
		if (i < swath->npings - 1)
			{
			headingx = sin(pingcur->heading*DTR);
			headingy = cos(pingcur->heading*DTR);
			dx = (swath->data[i+1].navlon - pingcur->navlon)
				/mtodeglon;
			dy = (swath->data[i+1].navlat - pingcur->navlat)
				/mtodeglat;
			r = sqrt(dx*dx + dy*dy);
			pingcur->lonfor = factor*r*headingx*mtodeglon;
			pingcur->latfor = factor*r*headingy*mtodeglat;
			}

		/* take care of first ping */
		if (i == 0)
			{
			pingcur->lonaft = -pingcur->lonfor;
			pingcur->lataft = -pingcur->latfor;
			}

		/* take care of last ping */
		if (i == swath->npings - 1)
			{
			pingcur->lonfor = -pingcur->lonaft;
			pingcur->latfor = -pingcur->lataft;
			}
		}
	  }

	/* take care of just one ping with nonzero center beam */
	else if (swath->npings == 1 && fp_mode == MBSWATH_FOOTPRINT_FAKE
		&& swath->data[0].bath[swath->beams_bath/2] > 0.0)
	  {
	  pingcur = &swath->data[0];
	  headingx = sin(pingcur->heading*DTR);
	  headingy = cos(pingcur->heading*DTR);
	  tt = pingcur->bath[swath->beams_bath/2]/750.0; /* in s */
	  r = tt * pingcur->speed * 0.55555556; /* in m */
	  pingcur->lonaft = -factor*r*headingx*mtodeglon;
	  pingcur->lataft = -factor*r*headingy*mtodeglat;
	  pingcur->lonfor = factor*r*headingx*mtodeglon;
	  pingcur->latfor = factor*r*headingy*mtodeglat;
	  }

	/* else get rfactor if using fore-aft beam width */
	else if (fp_mode == MBSWATH_FOOTPRINT_REAL)
	  {
	  rfactor = 0.5*sin(DTR*factor);
	  }

	/* loop over the inner beams and get 
		the obvious footprint boundaries */
	for (i=0;i<swath->npings;i++)
		{
		pingcur = &swath->data[i];

		/* get heading if using fore-aft beam width */
		if (fp_mode == MBSWATH_FOOTPRINT_REAL)
			{
			headingx = sin(pingcur->heading*DTR);
			headingy = cos(pingcur->heading*DTR);
			}

		/* do bathymetry */
		if (dobath == MB_YES)
		  for (j=1;j<swath->beams_bath-1;j++)
		    if (pingcur->bath[j] > 0.0)
			{
			x = pingcur->bathlon[j];
			y = pingcur->bathlat[j];
			setprint = MB_NO;
			if (pingcur->bath[j-1] > 0.0 
				&& pingcur->bath[j+1] > 0.0)
				{
				setprint = MB_YES;
				dlon1 = pingcur->bathlon[j-1] 
					- pingcur->bathlon[j];
				dlat1 = pingcur->bathlat[j-1] 
					- pingcur->bathlat[j];
				dlon2 = pingcur->bathlon[j+1] 
					- pingcur->bathlon[j];
				dlat2 = pingcur->bathlat[j+1] 
					- pingcur->bathlat[j];
				}
			else if (pingcur->bath[j-1] > 0.0)
				{
				setprint = MB_YES;
				dlon1 = pingcur->bathlon[j-1] 
					- pingcur->bathlon[j];
				dlat1 = pingcur->bathlat[j-1] 
					- pingcur->bathlat[j];
				dlon2 = -dlon1;
				dlat2 = -dlat1;
				}
			else if (pingcur->bath[j+1] > 0.0)
				{
				setprint = MB_YES;
				dlon2 = pingcur->bathlon[j+1] 
					- pingcur->bathlon[j];
				dlat2 = pingcur->bathlat[j+1] 
					- pingcur->bathlat[j];
				dlon1 = -dlon2;
				dlat1 = -dlat2;
				}

			/* do it using fore-aft beam width */
			if (setprint == MB_YES 
				&& fp_mode == MBSWATH_FOOTPRINT_REAL)
				{
				print = &pingcur->bathfoot[j];
				pingcur->bathflag[j] = MB_YES;
				ddlonx = (pingcur->bathlon[j] 
					- pingcur->navlon)/mtodeglon;
				ddlaty = (pingcur->bathlat[j] 
					- pingcur->navlat)/mtodeglat;
				r = rfactor*sqrt(ddlonx*ddlonx 
					+ ddlaty*ddlaty 
					+ pingcur->bath[j]*pingcur->bath[j]);
				pingcur->lonaft = -r*headingx*mtodeglon;
				pingcur->lataft = -r*headingy*mtodeglat;
				pingcur->lonfor = r*headingx*mtodeglon;
				pingcur->latfor = r*headingy*mtodeglat;
				print->x[0] = x + dlon1 + pingcur->lonaft;
				print->y[0] = y + dlat1 + pingcur->lataft;
				print->x[1] = x + dlon2 + pingcur->lonaft;
				print->y[1] = y + dlat2 + pingcur->lataft;
				print->x[2] = x + dlon2 + pingcur->lonfor;
				print->y[2] = y + dlat2 + pingcur->latfor;
				print->x[3] = x + dlon1 + pingcur->lonfor;
				print->y[3] = y + dlat1 + pingcur->latfor;
				}

			/* do it using ping nav separation */
			else if (setprint == MB_YES)
				{
				print = &pingcur->bathfoot[j];
				pingcur->bathflag[j] = MB_YES;
				print->x[0] = x + dlon1 + pingcur->lonaft;
				print->y[0] = y + dlat1 + pingcur->lataft;
				print->x[1] = x + dlon2 + pingcur->lonaft;
				print->y[1] = y + dlat2 + pingcur->lataft;
				print->x[2] = x + dlon2 + pingcur->lonfor;
				print->y[2] = y + dlat2 + pingcur->latfor;
				print->x[3] = x + dlon1 + pingcur->lonfor;
				print->y[3] = y + dlat1 + pingcur->latfor;
				}
			}

		/* do sidescan */
		if (doss == MB_YES)
		  for (j=1;j<swath->pixels_ss-1;j++)
		    if (pingcur->ss[j] > 0.0)
			{
			x = pingcur->sslon[j];
			y = pingcur->sslat[j];
			setprint = MB_NO;
			if (pingcur->ss[j-1] > 0.0 
				&& pingcur->ss[j+1] > 0.0)
				{
				setprint = MB_YES;
				dlon1 = pingcur->sslon[j-1] 
					- pingcur->sslon[j];
				dlat1 = pingcur->sslat[j-1] 
					- pingcur->sslat[j];
				dlon2 = pingcur->sslon[j+1] 
					- pingcur->sslon[j];
				dlat2 = pingcur->sslat[j+1] 
					- pingcur->sslat[j];
				}
			else if (pingcur->ss[j-1] > 0.0)
				{
				setprint = MB_YES;
				dlon1 = pingcur->sslon[j-1] 
					- pingcur->sslon[j];
				dlat1 = pingcur->sslat[j-1] 
					- pingcur->sslat[j];
				dlon2 = -dlon1;
				dlat2 = -dlat1;
				}
			else if (pingcur->ss[j+1] > 0.0)
				{
				setprint = MB_YES;
				dlon2 = pingcur->sslon[j+1] 
					- pingcur->sslon[j];
				dlat2 = pingcur->sslat[j+1] 
					- pingcur->sslat[j];
				dlon1 = -dlon2;
				dlat1 = -dlat2;
				}

			/* do it using fore-aft beam width */
			if (setprint == MB_YES 
				&& fp_mode == MBSWATH_FOOTPRINT_REAL)
				{
				print = &pingcur->ssfoot[j];
				pingcur->ssflag[j] = MB_YES;
				ddlonx = (pingcur->sslon[j] 
					- pingcur->navlon)/mtodeglon;
				ddlaty = (pingcur->sslat[j] 
					- pingcur->navlat)/mtodeglat;
				if (swath->beams_bath > 0 
					&& pingcur->bath[swath->beams_bath] > 0.0)
					dddepth = pingcur->bath[swath->beams_bath];
				else if (dddepth <= 0.0)
					dddepth = depth_def;
				r = rfactor*sqrt(ddlonx*ddlonx 
					+ ddlaty*ddlaty + dddepth*dddepth);
				pingcur->lonaft = -r*headingx*mtodeglon;
				pingcur->lataft = -r*headingy*mtodeglat;
				pingcur->lonfor = r*headingx*mtodeglon;
				pingcur->latfor = r*headingy*mtodeglat;
				print->x[0] = x + dlon1 + pingcur->lonaft;
				print->y[0] = y + dlat1 + pingcur->lataft;
				print->x[1] = x + dlon2 + pingcur->lonaft;
				print->y[1] = y + dlat2 + pingcur->lataft;
				print->x[2] = x + dlon2 + pingcur->lonfor;
				print->y[2] = y + dlat2 + pingcur->latfor;
				print->x[3] = x + dlon1 + pingcur->lonfor;
				print->y[3] = y + dlat1 + pingcur->latfor;
				}

			/* do it using ping nav separation */
			else if (setprint == MB_YES)
				{
				print = &pingcur->ssfoot[j];
				pingcur->ssflag[j] = MB_YES;
				print->x[0] = x + dlon1 + pingcur->lonaft;
				print->y[0] = y + dlat1 + pingcur->lataft;
				print->x[1] = x + dlon2 + pingcur->lonaft;
				print->y[1] = y + dlat2 + pingcur->lataft;
				print->x[2] = x + dlon2 + pingcur->lonfor;
				print->y[2] = y + dlat2 + pingcur->latfor;
				print->x[3] = x + dlon1 + pingcur->lonfor;
				print->y[3] = y + dlat1 + pingcur->latfor;
				}
			}
		}

	/* loop over the outer beams and get 
		the obvious footprint boundaries */
	for (i=0;i<swath->npings;i++)
		{
		pingcur = &swath->data[i];

		/* get heading if using fore-aft beam width */
		if (fp_mode == MBSWATH_FOOTPRINT_REAL)
			{
			headingx = sin(pingcur->heading*DTR);
			headingy = cos(pingcur->heading*DTR);
			}

		/* do bathymetry */
		if (dobath == MB_YES)
		  {
		  j = 0;
		  if (pingcur->bath[j] > 0.0 && pingcur->bath[j+1] > 0)
			{
			x = pingcur->bathlon[j];
			y = pingcur->bathlat[j];
			dlon2 = pingcur->bathlon[j+1] 
				- pingcur->bathlon[j];
			dlat2 = pingcur->bathlat[j+1] 
				- pingcur->bathlat[j];
			dlon1 = -dlon2;
			dlat1 = -dlat2;
			print = &pingcur->bathfoot[j];
			pingcur->bathflag[j] = MB_YES;

			/* using fore-aft beam width */
			if (fp_mode == MBSWATH_FOOTPRINT_REAL)
				{
				ddlonx = (pingcur->bathlon[j] 
					- pingcur->navlon)/mtodeglon;
				ddlaty = (pingcur->bathlat[j] 
					- pingcur->navlat)/mtodeglat;
				r = rfactor*sqrt(ddlonx*ddlonx 
					+ ddlaty*ddlaty 
					+ pingcur->bath[j]*pingcur->bath[j]);
				pingcur->lonaft = -r*headingx*mtodeglon;
				pingcur->lataft = -r*headingy*mtodeglat;
				pingcur->lonfor = r*headingx*mtodeglon;
				pingcur->latfor = r*headingy*mtodeglat;
				}

			print->x[0] = x + dlon1 + pingcur->lonaft;
			print->y[0] = y + dlat1 + pingcur->lataft;
			print->x[1] = x + dlon2 + pingcur->lonaft;
			print->y[1] = y + dlat2 + pingcur->lataft;
			print->x[2] = x + dlon2 + pingcur->lonfor;
			print->y[2] = y + dlat2 + pingcur->latfor;
			print->x[3] = x + dlon1 + pingcur->lonfor;
			print->y[3] = y + dlat1 + pingcur->latfor;
			}
		  j = swath->beams_bath-1;
		  if (pingcur->bath[j] > 0.0 && pingcur->bath[j-1] > 0)
			{
			x = pingcur->bathlon[j];
			y = pingcur->bathlat[j];
			dlon1 = pingcur->bathlon[j-1] 
				- pingcur->bathlon[j];
			dlat1 = pingcur->bathlat[j-1] 
				- pingcur->bathlat[j];
			dlon2 = -dlon2;
			dlat2 = -dlat2;
			print = &pingcur->bathfoot[j];
			pingcur->bathflag[j] = MB_YES;

			/* using fore-aft beam width */
			if (fp_mode == MBSWATH_FOOTPRINT_REAL)
				{
				ddlonx = (pingcur->bathlon[j] 
					- pingcur->navlon)/mtodeglon;
				ddlaty = (pingcur->bathlat[j] 
					- pingcur->navlat)/mtodeglat;
				r = rfactor*sqrt(ddlonx*ddlonx 
					+ ddlaty*ddlaty 
					+ pingcur->bath[j]*pingcur->bath[j]);
				pingcur->lonaft = -r*headingx*mtodeglon;
				pingcur->lataft = -r*headingy*mtodeglat;
				pingcur->lonfor = r*headingx*mtodeglon;
				pingcur->latfor = r*headingy*mtodeglat;
				}

			print->x[0] = x + dlon1 + pingcur->lonaft;
			print->y[0] = y + dlat1 + pingcur->lataft;
			print->x[1] = x + dlon2 + pingcur->lonaft;
			print->y[1] = y + dlat2 + pingcur->lataft;
			print->x[2] = x + dlon2 + pingcur->lonfor;
			print->y[2] = y + dlat2 + pingcur->latfor;
			print->x[3] = x + dlon1 + pingcur->lonfor;
			print->y[3] = y + dlat1 + pingcur->latfor;
			}
		  }

		/* do sidescan */
		if (doss == MB_YES)
		  {
		  j = 0;
		  if (pingcur->ss[j] > 0.0 && pingcur->ss[j+1] > 0)
			{
			x = pingcur->sslon[j];
			y = pingcur->sslat[j];
			dlon2 = pingcur->sslon[j+1] 
				- pingcur->sslon[j];
			dlat2 = pingcur->sslat[j+1] 
				- pingcur->sslat[j];
			dlon1 = -dlon2;
			dlat1 = -dlat2;
			print = &pingcur->ssfoot[j];
			pingcur->ssflag[j] = MB_YES;

			/* using fore-aft beam width */
			if (fp_mode == MBSWATH_FOOTPRINT_REAL)
				{
				ddlonx = (pingcur->sslon[j] 
					- pingcur->navlon)/mtodeglon;
				ddlaty = (pingcur->sslat[j] 
					- pingcur->navlat)/mtodeglat;
				r = rfactor*sqrt(ddlonx*ddlonx 
					+ ddlaty*ddlaty 
					+ pingcur->ss[j]*pingcur->ss[j]);
				pingcur->lonaft = -r*headingx*mtodeglon;
				pingcur->lataft = -r*headingy*mtodeglat;
				pingcur->lonfor = r*headingx*mtodeglon;
				pingcur->latfor = r*headingy*mtodeglat;
				}

			print->x[0] = x + dlon1 + pingcur->lonaft;
			print->y[0] = y + dlat1 + pingcur->lataft;
			print->x[1] = x + dlon2 + pingcur->lonaft;
			print->y[1] = y + dlat2 + pingcur->lataft;
			print->x[2] = x + dlon2 + pingcur->lonfor;
			print->y[2] = y + dlat2 + pingcur->latfor;
			print->x[3] = x + dlon1 + pingcur->lonfor;
			print->y[3] = y + dlat1 + pingcur->latfor;
			}
		  j = swath->pixels_ss-1;
		  if (pingcur->ss[j] > 0.0 && pingcur->ss[j-1] > 0)
			{
			x = pingcur->sslon[j];
			y = pingcur->sslat[j];
			dlon1 = pingcur->sslon[j-1] 
				- pingcur->sslon[j];
			dlat1 = pingcur->sslat[j-1] 
				- pingcur->sslat[j];
			dlon2 = -dlon2;
			dlat2 = -dlat2;
			print = &pingcur->ssfoot[j];
			pingcur->ssflag[j] = MB_YES;

			/* using fore-aft beam width */
			if (fp_mode == MBSWATH_FOOTPRINT_REAL)
				{
				ddlonx = (pingcur->sslon[j] 
					- pingcur->navlon)/mtodeglon;
				ddlaty = (pingcur->sslat[j] 
					- pingcur->navlat)/mtodeglat;
				if (swath->beams_bath > 0 
					&& pingcur->bath[swath->beams_bath] > 0.0)
					dddepth = pingcur->bath[swath->beams_bath];
				else if (dddepth <= 0.0)
					dddepth = depth_def;
				r = rfactor*sqrt(ddlonx*ddlonx 
					+ ddlaty*ddlaty + dddepth*dddepth);
				pingcur->lonaft = -r*headingx*mtodeglon;
				pingcur->lataft = -r*headingy*mtodeglat;
				pingcur->lonfor = r*headingx*mtodeglon;
				pingcur->latfor = r*headingy*mtodeglat;
				}

			print->x[0] = x + dlon1 + pingcur->lonaft;
			print->y[0] = y + dlat1 + pingcur->lataft;
			print->x[1] = x + dlon2 + pingcur->lonaft;
			print->y[1] = y + dlat2 + pingcur->lataft;
			print->x[2] = x + dlon2 + pingcur->lonfor;
			print->y[2] = y + dlat2 + pingcur->latfor;
			print->x[3] = x + dlon1 + pingcur->lonfor;
			print->y[3] = y + dlat1 + pingcur->latfor;
			}
		  }
		}

	/* print debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Beam footprints found in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2       npings:         %d\n",
			swath->npings);
		fprintf(stderr,"dbg2       error:          %d\n",
			*error);
		fprintf(stderr,"dbg2       status:         %d\n",
			status);
		for (i=0;i<swath->npings;i++)
			{
			fprintf(stderr,"dbg2\ndbg2       ping:           %d\n",i);
			pingcur = &swath->data[i];
			if (dobath == MB_YES)
			  for (j=0;j<swath->beams_bath;j++)
				{
				print = &pingcur->bathfoot[j];
				fprintf(stderr,"dbg2       %d  %d %g %g   ",
					j,pingcur->bathflag[j],
					pingcur->bathlon[j],
					pingcur->bathlat[j]);
				for (k=0;k<4;k++)
					fprintf(stderr,"  %g %g",
						print->x[k],
						print->y[k]);
				fprintf(stderr,"\n");
				}
			if (doss == MB_YES)
			  for (j=0;j<swath->pixels_ss;j++)
				{
				print = &pingcur->ssfoot[j];
				fprintf(stderr,"dbg2       %d  %d %g %g   ",
					j,pingcur->ssflag[j],
					pingcur->sslon[j],
					pingcur->sslat[j]);
				for (k=0;k<4;k++)
					fprintf(stderr,"  %g %g",
						print->x[k],
						print->y[k]);
				fprintf(stderr,"\n");
				}
			}
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> completed\n",
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
int get_shading(verbose,mode,swath,mtodeglon,
		mtodeglat,magnitude,azimuth,error)
int	verbose;
int	mode;
struct swath *swath;
double	mtodeglon;
double	mtodeglat;
double	magnitude;
double	azimuth;
int	*error;
{
	char	*function_name = "get_shading";
	int	status = MB_SUCCESS;
	struct ping	*ping0;
	struct ping	*ping1;
	struct ping	*ping2;
	int	drvcount;
	double	dx, dy, dd;
	double	dst2;
	double	drvx, drvy;
	double	sinx,cosy;
	double	median;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mode:       %d\n",mode);
		fprintf(stderr,"dbg2       swath:      %d\n",swath);
		fprintf(stderr,"dbg2       pings:      %d\n",swath->npings);
		fprintf(stderr,"dbg2       mtodeglon:  %f\n",mtodeglon);
		fprintf(stderr,"dbg2       mtodeglat:  %f\n",mtodeglat);
		fprintf(stderr,"dbg2       magnitude:  %f\n",magnitude);
		fprintf(stderr,"dbg2       azimuth:    %f\n",azimuth);
		}

	/* get shading from directional bathymetric gradient */
	if (mode == MBSWATH_BATH_RELIEF)
	  {
	  /* get directional factors */
	  sinx = sin(DTR*azimuth);
	  cosy = cos(DTR*azimuth);

	  /* loop over the pings and beams */
	  for (i=0;i<swath->npings;i++)
	    {
	    if (i > 0) ping0 = &swath->data[i-1];
	    ping1 = &swath->data[i];
	    if (i < swath->npings - 1) ping2 = &swath->data[i+1];
	    for (j=0;j<swath->beams_bath;j++)
	      if (ping1->bath[j] > 0.0)
		{
		/* do across track components */
		drvcount = 0;
		dx = 0.0;
		dy = 0.0;
		dd = 0.0;
		drvx = 0.0;
		drvy = 0.0;
		if (j > 0 && j < swath->beams_bath - 1
			&& ping1->bath[j-1] > 0.0
			&& ping1->bath[j+1] > 0.0)
			{
			dx = (ping1->bathlon[j+1] - ping1->bathlon[j-1])
				/mtodeglon;
			dy = (ping1->bathlat[j+1] - ping1->bathlat[j-1])
				/mtodeglat;
			dd = ping1->bath[j+1] - ping1->bath[j-1];
			}
		else if (j < swath->beams_bath - 1
			&& ping1->bath[j] > 0.0
			&& ping1->bath[j+1] > 0.0)
			{
			dx = (ping1->bathlon[j+1] - ping1->bathlon[j])
				/mtodeglon;
			dy = (ping1->bathlat[j+1] - ping1->bathlat[j])
				/mtodeglat;
			dd = ping1->bath[j+1] - ping1->bath[j];
			}
		else if (j > 0
			&& ping1->bath[j-1] > 0.0
			&& ping1->bath[j] > 0.0)
			{
			dx = (ping1->bathlon[j] - ping1->bathlon[j-1])
				/mtodeglon;
			dy = (ping1->bathlat[j] - ping1->bathlat[j-1])
				/mtodeglat;
			dd = ping1->bath[j] - ping1->bath[j-1];
			}
		dst2 = dx*dx + dy*dy;
		if (dst2 > 0.0)
			{
			drvx = dd*dx/dst2;
			drvy = dd*dy/dst2;
			drvcount++;
			}

		/* do along track components */
		dx = 0.0;
		dy = 0.0;
		dd = 0.0;
		if (i > 0 && i < swath->npings - 1
			&& ping0->bath[j] > 0.0
			&& ping2->bath[j] > 0.0)
			{
			dx = (ping2->bathlon[j] - ping0->bathlon[j])
				/mtodeglon;
			dy = (ping2->bathlat[j] - ping0->bathlat[j])
				/mtodeglat;
			dd = ping2->bath[j] - ping0->bath[j];
			}
		else if (i < swath->npings - 1
			&& ping1->bath[j] > 0.0
			&& ping2->bath[j] > 0.0)
			{
			dx = (ping2->bathlon[j] - ping1->bathlon[j])
				/mtodeglon;
			dy = (ping2->bathlat[j] - ping1->bathlat[j])
				/mtodeglat;
			dd = ping2->bath[j] - ping1->bath[j];
			}
		else if (i > 0
			&& ping0->bath[j] > 0.0
			&& ping1->bath[j] > 0.0)
			{
			dx = (ping1->bathlon[j] - ping0->bathlon[j])
				/mtodeglon;
			dy = (ping1->bathlat[j] - ping0->bathlat[j])
				/mtodeglat;
			dd = ping1->bath[j] - ping0->bath[j];
			}
		dst2 = dx*dx + dy*dy;
		if (dst2 > 0.0)
			{
			drvx = drvx + dd*dx/dst2;
			drvy = drvy + dd*dy/dst2;
			drvcount++;
			}

		/* calculate directional derivative */
		if (drvcount == 2)
			ping1->bathshade[j] = magnitude*(drvx*sinx + drvy*cosy);
		else
			ping1->bathshade[j] = 0.0;

		}
	    }
	  }

	/* get shading from amplitude data */
	if (mode == MBSWATH_BATH_AMP)
	  {
	  /* get median value from value entered as azimuth */
	  median = azimuth;

	  /* loop over the pings and beams */
	  for (i=0;i<swath->npings;i++)
	    {
	    if (i > 0) ping0 = &swath->data[i-1];
	    ping1 = &swath->data[i];
	    if (i < swath->npings - 1) ping2 = &swath->data[i+1];
	    for (j=0;j<swath->beams_bath;j++)
	      if (ping1->bath[j] > 0.0)
		{

		/* calculate directional derivative */
		if (ping1->amp[j] > 0)
			ping1->bathshade[j] = magnitude*(ping1->amp[j] - median);
		else
			ping1->bathshade[j] = 0.0;

		}
	    }
	  }

	/* print debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Shading values in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2       npings:         %d\n",
			swath->npings);
		fprintf(stderr,"dbg2       error:          %d\n",
			*error);
		fprintf(stderr,"dbg2       status:         %d\n",
			status);
		for (i=0;i<swath->npings;i++)
			{
			fprintf(stderr,"dbg2\ndbg2       ping:           %d\n",i);
			ping1 = &swath->data[i];
			for (j=0;j<swath->beams_bath;j++)
				{
				fprintf(stderr,"dbg2       %d  %d  %g  %g\n",
					j,ping1->bathflag[j],
					ping1->bath[j],
					ping1->bathshade[j]);
				}
			}
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> completed\n",
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
int plot_data(verbose,mode,swath,first,nplot,error)
int	verbose;
int	mode;
struct swath *swath;
int	first;
int	nplot;
int	*error;
{
	char	*function_name = "plot_data";
	int	status = MB_SUCCESS;
	struct ping	*pingcur;
	struct footprint	*print;
	double	*x, *y;
	double	xx[4], yy[4];
	int	red, green, blue;
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mode:       %d\n",mode);
		fprintf(stderr,"dbg2       swath:      %d\n",swath);
		fprintf(stderr,"dbg2       pings:      %d\n",swath->npings);
		fprintf(stderr,"dbg2       first:      %d\n",first);
		fprintf(stderr,"dbg2       nplot:      %d\n",nplot);
		}

	if (mode == MBSWATH_BATH || mode == MBSWATH_BATH_RELIEF
		|| mode == MBSWATH_BATH_AMP)
		{
		/* loop over all pings and beams and plot the good ones */
		for (i=first;i<first+nplot;i++)
			{
			pingcur = &swath->data[i];
			for (j=0;j<swath->beams_bath;j++)
			  if (pingcur->bathflag[j] == MB_YES)
				{
				print = &pingcur->bathfoot[j];
				x = &(print->x[0]);
				y = &(print->y[0]);
				for (k=0;k<4;k++)
					geo_to_xy(x[k],y[k],&xx[k],&yy[k]);
				get_rgb24(pingcur->bath[j],&red,&green,&blue);
				if (mode == MBSWATH_BATH_RELIEF 
					|| mode == MBSWATH_BATH_AMP)
					illuminate(pingcur->bathshade[j],
						&red,&green,&blue);
				status = plot_box(verbose,xx,yy,
						red,green,blue,error);
				}
			}
		}
	else if (mode == MBSWATH_AMP)
		{
		/* loop over all pings and beams and plot the good ones */
		for (i=first;i<first+nplot;i++)
			{
			pingcur = &swath->data[i];
			for (j=0;j<swath->beams_amp;j++)
			  if (pingcur->bathflag[j] == MB_YES)
				{
				print = &pingcur->bathfoot[j];
				x = &(print->x[0]);
				y = &(print->y[0]);
				for (k=0;k<4;k++)
					geo_to_xy(x[k],y[k],&xx[k],&yy[k]);
				get_rgb24(pingcur->amp[j],&red,&green,&blue);
				status = plot_box(verbose,xx,yy,
						red,green,blue,error);
				}
			}
		}
	else if (mode == MBSWATH_SS)
		{
		/* loop over all pings and beams and plot the good ones */
		for (i=first;i<first+nplot;i++)
			{
			pingcur = &swath->data[i];
			for (j=0;j<swath->pixels_ss;j++)
			  if (pingcur->ssflag[j] == MB_YES)
				{
				print = &pingcur->ssfoot[j];
				x = &(print->x[0]);
				y = &(print->y[0]);
				for (k=0;k<4;k++)
					geo_to_xy(x[k],y[k],&xx[k],&yy[k]);
				get_rgb24(pingcur->ss[j],&red,&green,&blue);
				status = plot_box(verbose,xx,yy,
						red,green,blue,error);
				}
			}
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> completed\n",
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
int plot_box(verbose,x,y,red,green,blue,error)
int	verbose;
double	*x;
double	*y;
int	red;
int	green;
int	blue;
int	*error;
{
	char	*function_name = "plot_box";
	int	status = MB_SUCCESS;
	int	ix[5], iy[5];
	int	ixmin, ixmax, iymin, iymax;
	int	ixx, iyy;
	int	ixx1, ixx2;
	double	dx, dy;
	int	ncross, xcross[10];
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       x[0]:       %f\n",x[0]);
		fprintf(stderr,"dbg2       y[0]:       %f\n",y[0]);
		fprintf(stderr,"dbg2       x[1]:       %f\n",x[1]);
		fprintf(stderr,"dbg2       y[1]:       %f\n",y[1]);
		fprintf(stderr,"dbg2       x[2]:       %f\n",x[2]);
		fprintf(stderr,"dbg2       y[2]:       %f\n",y[2]);
		fprintf(stderr,"dbg2       x[3]:       %f\n",x[3]);
		fprintf(stderr,"dbg2       y[3]:       %f\n",y[3]);
		fprintf(stderr,"dbg2       red:        %d\n",red);
		fprintf(stderr,"dbg2       green:      %d\n",green);
		fprintf(stderr,"dbg2       blue:       %d\n",blue);
		}

	/* if simple case just plot polygon */
	if (image == MBSWATH_IMAGE_VECTOR)
		ps_polygon(x,y,4,red,green,blue,0);

	/* if image plot then rasterize the box */
	else if (image == MBSWATH_IMAGE_8 || image == MBSWATH_IMAGE_24)
		{
		/* get bounds of box in pixels */
		for (i=0;i<4;i++)
			{
			ix[i] = nx*x[i]/x_inch;
			iy[i] = ny*y[i]/y_inch;
			}
		ix[4] = ix[0];
		iy[4] = iy[0];

		/* get min max values of bounding corners in pixels */
		ixmin = ix[0];
		ixmax = ix[0];
		iymin = iy[0];
		iymax = iy[0];
		for (i=1;i<4;i++)
			{
			if (ix[i] < ixmin)
				ixmin = ix[i];
			if (ix[i] > ixmax)
				ixmax = ix[i];
			if (iy[i] < iymin)
				iymin = iy[i];
			if (iy[i] > iymax)
				iymax = iy[i];
			}
		if (ixmin < 0) ixmin = 0;
		if (ixmax > nx-1) ixmax = nx - 1;
		if (iymin < 1) iymin = 1;
		if (iymax > ny-1) iymax = ny - 1;

		/* loop over all y values */
		for (iyy=iymin;iyy<=iymax;iyy++)
		  {
		  /* find crossings */
		  ncross = 0;
		  for (i=0;i<4;i++)
		    {
		    if ((iy[i] <= iyy && iy[i+1] >= iyy)
		      || (iy[i] >= iyy && iy[i+1] <= iyy))
		      {
		      if (iy[i] == iy[i+1])
		        {
		        xcross[ncross] = ix[i];
		        ncross++;
		        xcross[ncross] = ix[i+1];
		        ncross++;
		        }
		      else
		        {
		        dy = iy[i+1] - iy[i];
		        dx = ix[i+1] - ix[i];
		        xcross[ncross] = (int) ((iyy - iy[i])*dx/dy + ix[i]);
		        ncross++;
		        }
		      }
		    }		

		  /* plot lines between crossings */
		  for (j=0;j<ncross-1;j++)
		    {
		    if (xcross[j] < xcross[j+1])
		      {
		      ixx1 = xcross[j];
		      ixx2 = xcross[j+1];
		      }
		    else 
		      {
		      ixx1 = xcross[j+1];
		      ixx2 = xcross[j];
		      }
		    if ((ixx1 < ixmin && ixx2 < ixmin) 
		      || (ixx1 > ixmax && ixx2 > ixmax))
		      ixx2 = ixx1 - 1; /* disable plotting */
		    else
		      {
		      if (ixx1 < ixmin) ixx1 = ixmin;
		      if (ixx2 > ixmax) ixx2 = ixmax;
		      }
		    for (ixx=ixx1;ixx<=ixx2;ixx++)
		      {
/*			fprintf(stderr,"plot %d %d\n",ixx,iyy);*/
		      if (image == MBSWATH_IMAGE_8)
			{
			k = nx*(ny - iyy) + ixx;
			bitimage[k] = (unsigned char) YIQ (red, green, blue);
			}
		      else if (gmtdefs.color_image == 3)
			{
			k = nx*(ny - iyy) + ixx;
			bitimage[k] = red;
			bitimage[k+nm] = green;
			bitimage[k+nm2] = blue;
			}
		      else
			{
			k = 3*(nx*(ny - iyy) + ixx);
			bitimage[k] = red;
			bitimage[k+1] = green;
			bitimage[k+2] = blue;
			}
		      }
		    }
		  }

		/* plot simple minded wrong box */
/*		for (iyy=iymin;iyy<=iymax;iyy++)
		  for (ixx=ixmin;ixx<=ixmax;ixx++)
		    {
		    if (image == MBSWATH_IMAGE_8)
			{
			k = nx*(ny - iyy) + ixx;
			bitimage[k] = (unsigned char) YIQ (red, green, blue);
			}
		    else if (gmtdefs.color_image == 3)
			{
			k = nx*(ny - iyy) + ixx;
			bitimage[k] = red;
			bitimage[k+nm] = green;
			bitimage[k+nm2] = blue;
			}
		    else
			{
			k = 3*(nx*(ny - iyy) + ixx);
			bitimage[k] = red;
			bitimage[k+1] = green;
			bitimage[k+2] = blue;
			}
		    }*/
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> completed\n",
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
int ping_copy(verbose,one,two,swath,error)
int	verbose;
int	one;
int	two;
struct swath *swath;
int	*error;
{
	char	*function_name = "ping_copy";
	int	status = MB_SUCCESS;

	struct ping	*ping1;
	struct ping	*ping2;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       one:        %d\n",one);
		fprintf(stderr,"dbg2       two:        %d\n",two);
		fprintf(stderr,"dbg2       swath:      %d\n",swath);
		fprintf(stderr,"dbg2       pings:      %d\n",swath->npings);
		}

	/* copy things */
	ping1 = &swath->data[one];
	ping2 = &swath->data[two];
	ping1->pings = ping2->pings;
	ping1->kind = ping2->kind;
	for (i=0;i<7;i++)
		ping1->time_i[i] = ping2->time_i[i];
	ping1->time_d = ping2->time_d;
	ping1->navlon = ping2->navlon;
	ping1->navlat = ping2->navlat;
	ping1->speed = ping2->speed;
	ping1->heading = ping2->heading;
	ping1->distance = ping2->distance;
	strcpy(ping1->comment,ping2->comment);
	for (i=0;i<swath->beams_bath;i++)
		{
		ping1->bath[i] = ping2->bath[i];
		ping1->bathlon[i] = ping2->bathlon[i];
		ping1->bathlat[i] = ping2->bathlat[i];
		ping1->bathflag[i] = ping2->bathflag[i];
		for (j=0;j<4;j++)
			{
			ping1->bathfoot[i].x[j] = ping2->bathfoot[i].x[j];
			ping1->bathfoot[i].y[j] = ping2->bathfoot[i].y[j];
			}
		}
	for (i=0;i<swath->beams_amp;i++)
		{
		ping1->amp[i] = ping2->amp[i];
		}
	for (i=0;i<swath->pixels_ss;i++)
		{
		ping1->ss[i] = ping2->ss[i];
		ping1->sslon[i] = ping2->sslon[i];
		ping1->sslat[i] = ping2->sslat[i];
		ping1->ssflag[i] = ping2->ssflag[i];
		for (j=0;j<4;j++)
			{
			ping1->ssfoot[i].x[j] = ping2->ssfoot[i].x[j];
			ping1->ssfoot[i].y[j] = ping2->ssfoot[i].y[j];
			}
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> completed\n",
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

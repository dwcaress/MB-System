/*--------------------------------------------------------------------
 *    The MB-system:	mbswath.c	5/30/93
 *    $Id$
 *
 *    Copyright (c) 1993-2009 by
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
 * MBSWATH is a GMT compatible utility which creates a color postscript
 * image of swath bathymetry or backscatter data.  The image
 * may be shaded relief as well.  Complete maps are made by using
 * MBSWATH in conjunction with the usual GMT programs.  The modes
 * of operation are:
 *   Mode 1:  Bathymetry
 *   Mode 2:  Bathymetry shaded by illumination
 *   Mode 3:  Bathymetry shaded by amplitude
 *   Mode 4:  amplitude
 *   Mode 5:  sidescan
 *   Mode 6:  Bathymetry shaded by amplitude using cpt gray data
 *
 * Author:	D. W. Caress
 * Date:	May 30, 1993
 *
 * $Log: mbswath.c,v $
 * Revision 5.23  2009/03/02 18:59:05  caress
 * Moving towards 5.1.2beta1.
 *
 * Revision 5.22  2008/09/27 03:27:10  caress
 * Working towards release 5.1.1beta24
 *
 * Revision 5.21  2008/09/13 06:08:09  caress
 * Updates to apply suggested patches to segy handling. Also fixes to remove compiler warnings.
 *
 * Revision 5.20  2007/11/02 22:38:52  caress
 * Fixed handling of time gap errors in mbswath.
 *
 * Revision 5.19  2007/10/17 20:35:53  caress
 * Release 5.1.1beta11
 *
 * Revision 5.18  2007/07/04 03:00:19  caress
 * Now works with xyz data.
 *
 * Revision 5.17  2006/12/15 21:42:49  caress
 * Incremental CVS update.
 *
 * Revision 5.16  2006/11/26 09:42:01  caress
 * Making distribution 5.1.0.
 *
 * Revision 5.15  2006/11/10 22:36:04  caress
 * Working towards release 5.1.0
 *
 * Revision 5.14  2006/11/07 20:19:29  dale
 * Changed line 646 from: 	GMT_pen_syntax ('F');
 *
 * to: GMT_pen_syntax ('F'," ");
 *
 * to accomodate GMT4.1.4
 * -Dale
 *
 * Revision 5.13  2006/06/22 04:45:42  caress
 * Working towards 5.1.0
 *
 * Revision 5.12  2006/06/02 03:00:31  caress
 * Put in ifdefs to handle new GMT version.
 *
 * Revision 5.11  2006/01/11 07:25:53  caress
 * Working towards 5.0.8
 *
 * Revision 5.10  2005/11/04 20:50:19  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.9  2004/05/21 23:13:35  caress
 * Changes to support GMT 4.0
 *
 * Revision 5.8  2003/04/17 20:43:37  caress
 * Release 5.0.beta30
 *
 * Revision 5.7  2002/10/02 23:52:37  caress
 * Release 5.0.beta24
 *
 * Revision 5.6  2002/04/06 02:45:59  caress
 * Release 5.0.beta16
 *
 * Revision 5.5  2001/08/10 22:40:02  dcaress
 * Release 5.0.beta07
 *
 * Revision 5.4  2001-07-19 17:29:41-07  caress
 * Release 5.0.beta03
 *
 * Revision 5.3  2001/06/08 21:42:53  caress
 * Version 5.0.beta01
 *
 * Revision 5.2  2001/03/22 21:03:31  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.1  2001/01/22  05:03:25  caress
 * Release 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:52:16  caress
 * First cut at Version 5.0.
 *
 * Revision 4.33  2000/10/11  00:53:45  caress
 * Converted to ANSI C
 *
 * Revision 4.32  2000/09/30  06:52:17  caress
 * Snapshot for Dale.
 *
 * Revision 4.31  2000/09/11  20:09:14  caress
 * Linked to new datalist parsing functions. Now supports recursive datalists
 * and comments in datalists.
 *
 * Revision 4.30  1999/04/16  01:24:27  caress
 * Final version 4.6 release?
 *
 * Revision 4.29  1999/02/04  23:41:29  caress
 * MB-System version 4.6beta7
 *
 * Revision 4.28  1998/12/17  22:53:13  caress
 * MB-System version 4.6beta4
 *
 * Revision 4.27  1998/10/28  21:32:29  caress
 * Fixed handling of data with variable numbers of beams.
 *
 * Revision 4.26  1998/10/04  04:18:07  caress
 * MB-System version 4.6beta
 *
 * Revision 4.25  1997/09/15  19:03:27  caress
 * Real Version 4.5
 *
 * Revision 4.24  1997/04/21  16:53:56  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.24  1997/04/17  15:05:49  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.23  1996/09/05  13:58:27  caress
 * Added feature to check data bounds in ".inf" files.
 *
 * Revision 4.22  1996/08/26  17:31:55  caress
 * Release 4.4 revision.
 *
 * Revision 4.21  1996/08/13  18:32:53  caress
 * Fixed bug in getting footprints of outer beams.
 *
 * Revision 4.20  1996/08/13  16:10:07  caress
 * Fixed bug in plotting the first sidescan pixel.
 *
 * Revision 4.19  1996/07/26  21:03:26  caress
 * Fixed code to handle variable numbers of beams and pixels properly.
 *
 * Revision 4.18  1996/04/22  13:20:25  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.17  1995/11/28  21:06:16  caress
 * Fixed scaling for meters to feet.
 *
 * Revision 4.16  1995/11/22  22:13:02  caress
 * Now handles bathymetry in feet with -W option.
 *
 * Revision 4.15  1995/11/15  22:32:55  caress
 * Now handles non-region bounds (lower left point
 * + upper right point) properly.
 *
 * Revision 4.14  1995/08/17  14:49:26  caress
 * Revision for release 4.3.
 *
 * Revision 4.13  1995/08/07  17:31:39  caress
 * Moved to GMT V3.
 *
 * Revision 4.12  1995/07/13  19:15:53  caress
 * Fixed problems with footprints for sidescan.
 *
 * Revision 4.11  1995/05/12  17:19:02  caress
 * Made exit status values consistent with Unix convention.
 * 0: ok  nonzero: error
 *
 * Revision 4.10  1995/03/06  19:40:17  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
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
#include "../../include/mb_define.h"

/* GMT include files */
#include "gmt.h"
#include "pslib.h"

/* GMT argument handling define */
#define MBSWATH_GMT_ARG_MAX     128

/* MBSWATH MODE DEFINES */
#define	MBSWATH_BATH		1
#define	MBSWATH_BATH_RELIEF	2
#define	MBSWATH_BATH_AMP	3
#define	MBSWATH_AMP		4
#define	MBSWATH_SS		5
#define	MBSWATH_BATH_AMP_FILTER	6
#define	MBSWATH_AMP_FILTER	7
#define	MBSWATH_SS_FILTER	8
#define MBSWATH_FOOTPRINT_REAL	1
#define MBSWATH_FOOTPRINT_FAKE	2
#define MBSWATH_FOOTPRINT_POINT	3
#define MBSWATH_FILTER_NONE	0
#define MBSWATH_FILTER_AMP	1
#define MBSWATH_FILTER_SIDESCAN	2

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
	double	altitude;
	double	sonardepth;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	char	*beamflag;
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
#ifndef YIQ
#define YIQ(rgb) irint (0.299 * (rgb[0]) + 0.587 * (rgb[1]) + 0.114 * (rgb[2]))
#endif
int	image = MBSWATH_IMAGE_24;
unsigned char *bitimage;
int	dpi = 100;
double	x_inch, y_inch;
double	xo, yo;
int	nx, ny, nm, nm2;
unsigned char r, g, b, gray;

int get_footprints(int verbose, int mode, int fp_mode,
		double factor, double depth_def,
		struct swath *swath, 
		double mtodeglon, double mtodeglat, int *error);
int get_shading(int verbose, int mode, int ampshademode, struct swath *swath,
		double mtodeglon, double mtodeglat,
		double magnitude, double azimuth,
		int nshadelevel, double *shadelevel, 
		int *shadelevelgray, int *error);
int plot_data_footprint(int verbose, int mode,
		struct swath *swath, int first, int nplot, int *error);
int plot_data_point(int verbose, int mode, 
		struct swath *swath, int first, int nplot, int *error);
int plot_box(int verbose, double *x, double *y, int *rgb, int *error);
int plot_point(int verbose, double x, double y, int *rgb, int *error);
int ping_copy(int verbose, int one, int two, struct swath *swath, int *error);

static char rcs_id[] = "$Id$";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char program_name[] = "MBSWATH";
	char help_message[] =  "MBSWATH is a GMT compatible utility which creates a color postscript \nimage of swath bathymetry or backscatter data.  The image \nmay be shaded relief as well.  Complete maps are made by using \nMBSWATH in conjunction with the usual GMT programs.";
	char usage_message[] = "mbswath -Ccptfile -Jparameters -Rwest/east/south/north \n\t[-Afactor -Btickinfo -byr/mon/day/hour/min/sec \n\t-ccopies -Dmode/ampscale/ampmin/ampmax \n\t-Eyr/mon/day/hour/min/sec -fformat \n\t-Fred/green/blue -Gmagnitude/azimuth -Idatalist \n\t-K -Ncptfile -O -P -ppings -Qdpi -Ttimegap -U -W -Xx-shift -Yy-shift \n\t-Zmode[F] -V -H]";

	extern char *optarg;
	int     argc_gmt = 0;
	char    *argv_gmt[MBSWATH_GMT_ARG_MAX];
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
	char	read_file[MB_PATH_MAXLINE];
        int     read_datalist = MB_NO;
	int	read_data;
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	double	file_weight;
	FILE	*fp;
	int	format;
	double	beamwidth_xtrack;
	double	beamwidth_ltrack;
	int	pings;
	int	lonflip;
	int	lonflip_set = MB_NO;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	char	file[MB_PATH_MAXLINE];
	int	file_in_bounds;
	int	beams_bath_max;
	int	beams_amp_max;
	int	pixels_ss_max;
	void	*mbio_ptr = NULL;

	/* mbio read values */
	struct swath *swath_plot = NULL;
	struct ping *pingcur = NULL;
	char	*beamflag = NULL;
	double	*bath = NULL;
	double	*bathlon = NULL;
	double	*bathlat = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*sslon = NULL;
	double	*sslat = NULL;

	/* gmt control variables */
	double	borders[4];
	double	borders_use[4];
	char	cptfile[MB_PATH_MAXLINE];
	char	cptshadefile[MB_PATH_MAXLINE];
	int	ampshademode = 0;
	int	nshadelevel = 0;
	double	*shadelevel;
	int	*shadelevelgray;
	double	magnitude = 1.0;
	double	azimuth = 90.0;
	int	ampscale_mode = 0;
	double	ampscale = 1.0;
	double	ampmin = 0.0;
	double	ampmax = 1.0;
	int	footprint_mode = MBSWATH_FOOTPRINT_REAL;
	double	rawfactor = 1.0;
	double	factor;
	double	default_depth = 0.0;
	double	default_depth_use;
	int	mode = MBSWATH_BATH;
	int	usefiltered = MB_NO;
	int	filtermode = MBSWATH_FILTER_NONE;
	int	bathy_in_feet = MB_NO;
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
	int	rgb[3];
	int	r1, g1, b1, r2, g2, b2;
	int	count;
	int	i, j;
	char	line[MB_PATH_MAXLINE];

	/* get current mb default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to stdin */
	strcpy (read_file, "stdin");

	/* initialize some values */
	format = -1;
        read_datalist = MB_NO;
	strcpy (cptfile,"cpt");
	strcpy (cptshadefile,"\0");
	borders[0] = 0.0;
	borders[1] = 0.0;
	borders[2] = 0.0;
	borders[3] = 0.0;

	/* get GMT options into separate argv */
	argv_gmt[0] = argv[0];
	argc_gmt = 1;
	for (i=1;i<argc;i++)
	  {
	  if (argv[i][0] == '-')
	    {
	    switch (argv[i][1])
	        {
		case 'B':
       		case 'C':
		case 'c':
		case 'F':
		case 'J':
		case 'j':
		case 'K':
		case 'k':
		case 'O':
		case 'o':
		case 'P':
		case 'R':
		case 'r':
		case 'U':
		case 'u':
		case 'V':
		case 'v':
		case 'X':
		case 'x':
		case 'Y':
		case 'y':
		case '0':
		case '1':
		case '2':
		case '#':
		case '\0':
		        if (argc_gmt < MBSWATH_GMT_ARG_MAX)
			  {
			  argv_gmt[argc_gmt] = argv[i];
			  argc_gmt++;
			  break;
			  }    
		}
	    }
	  }

	/* deal with mb options */
	while ((c = getopt(argc, argv, "A:a:B:b:C:c:D:d:E:e:F:f:G:g:HhI:i:J:j:KkL:l:MN:n:OPp:Q:R:r:S:s:T:t:UuVvWwX:x:Y:y:Z:z:012")) != -1)
	  switch (c) 
		{
		case 'A':
		case 'a':
			sscanf (optarg,"%lf/%d/%lf", &rawfactor, 
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
			lonflip_set = MB_YES;
			flag++;
			break;
		case 'N':
		case 'n':
			sscanf (optarg,"%s", cptshadefile);
			ampshademode = 1;
			flag++;
			break;
		case 'p':
			sscanf (optarg,"%d", &pings);
			flag++;
			break;
		case 'Q':
		case 'q':
			sscanf (optarg,"%d", &dpi);
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
		case 'W':
		case 'w':
			bathy_in_feet = MB_YES;
			break;
		case 'Z':
		case 'z':
			sscanf (optarg,"%d", &mode);
                        if (optarg[1] == 'f' || optarg[1] == 'F')
				usefiltered = MB_YES;
			flag++;
			break;
		case 'B':
		case 'C':
		case 'c':
		case 'F':
		case 'J':
		case 'j':
		case 'K':
		case 'k':
		case 'O':
		case 'o':
		case 'P':
		case 'R':
		case 'r':
		case 'U':
		case 'u':
		case 'X':
		case 'x':
		case 'Y':
		case 'y':
		case '#':
		case '0':
		case '1':
		case '2':
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

	/* deal with gmt options */
	argc = GMT_begin (argc_gmt, argv_gmt);
	for (i = 1; i < argc_gmt; i++) 
		{
		if (argv_gmt[i][0] == '-') 
			{
			switch (argv_gmt[i][1]) 
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
				case 'c':
				case '\0':
					errflg += GMT_get_common_args (argv_gmt[i], 
						&borders[0], &borders[1], 
						&borders[2], &borders[3]);
					break;
				
				/* Supplemental parameters */
			
				case 'C':
					strcpy(cptfile,&argv_gmt[i][2]);
					break;
				case 'F':
					if (GMT_getrgb (&argv_gmt[i][2], 
						gmtdefs.basemap_frame_rgb)) 
						{
#ifdef GMT_4_1_2
						GMT_pen_syntax ('F');
#else
						GMT_pen_syntax ('F'," ");
#endif
						}
				case '0':
					gmtdefs.color_image = 0;
					image = MBSWATH_IMAGE_24;
					break;
				case '1':
					gmtdefs.color_image = 1;
					image = MBSWATH_IMAGE_VECTOR;
					break;
				case '2':
					gmtdefs.color_image = 2;
					image = MBSWATH_IMAGE_24;
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
		fprintf(stderr,"dbg2       cptfile:          %s\n",cptfile);
		fprintf(stderr,"dbg2       cptshadefile:     %s\n",cptshadefile);
		fprintf(stderr,"dbg2       shade magnitude:  %f\n",magnitude);
		fprintf(stderr,"dbg2       shade azimuth:    %f\n",azimuth);
		fprintf(stderr,"dbg2       amp scale mode:   %d\n",ampscale_mode);
		fprintf(stderr,"dbg2       amplitude scale:  %f\n",ampscale);
		fprintf(stderr,"dbg2       amplitude minimum:%f\n",ampmin);
		fprintf(stderr,"dbg2       amplitude maximum:%f\n",ampmax);
		fprintf(stderr,"dbg2       footprint mode:   %d\n",footprint_mode);
		fprintf(stderr,"dbg2       footprint factor: %f\n",rawfactor);
		fprintf(stderr,"dbg2       default depth:    %f\n",default_depth);
		fprintf(stderr,"dbg2       mode:             %d\n",mode);
		fprintf(stderr,"dbg2       usefiltered:      %d\n",usefiltered);
		fprintf(stderr,"dbg2       bathy_in_feet:    %d\n",bathy_in_feet);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}
	
	/* turn on looking for filtered amp or sidescan if needed */
	if (usefiltered == MB_YES)
		{
		if (mode == MBSWATH_BATH_AMP)
			filtermode = MBSWATH_FILTER_AMP;
		else if (mode == MBSWATH_AMP)
			filtermode = MBSWATH_FILTER_AMP;
		else if (mode == MBSWATH_SS)
			filtermode = MBSWATH_FILTER_SIDESCAN;
		}

	/* copy borders in correct order for use by this program */
	if (project_info.region == MB_YES)
		{
		borders_use[0] = borders[0];
		borders_use[1] = borders[1];
		borders_use[2] = borders[2];
		borders_use[3] = borders[3];
		}
	else
		{
		borders_use[0] = borders[0];
		borders_use[1] = borders[2];
		borders_use[2] = borders[1];
		borders_use[3] = borders[3];
		}

	/* if borders not specified then quit */
	if (borders_use[0] >= borders_use[1] 
		|| borders_use[2] >= borders_use[3]
		|| borders_use[2] <= -90.0 
		|| borders_use[3] >= 90.0)
		{
		fprintf(stderr,"\nRegion borders not properly specified:\n\t%f %f %f %f\n",borders[0],borders[1],borders[2],borders[3]);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_PARAMETER;
		exit(error);
		}
	
	/* set lonflip if possible */
	if (lonflip_set == MB_NO)
		{
		if (borders_use[0] < -180.0)
			lonflip = -1;
		else if (borders_use[1] > 180.0)
			lonflip = 1;
		else if (lonflip == -1 && borders_use[1] > 0.0)
			lonflip = 0;
		else if (lonflip == 1 && borders_use[0] < 0.0)
			lonflip = 0;
		}

	/* set bounds for data reading larger than
		map borders */
	bounds[0] = borders_use[0] 
		- 0.25*(borders_use[1] - borders_use[0]);
	bounds[1] = borders_use[1] 
		+ 0.25*(borders_use[1] - borders_use[0]);
	bounds[2] = borders_use[2] 
		- 0.25*(borders_use[3] - borders_use[2]);
	bounds[3] = borders_use[3] 
		+ 0.25*(borders_use[3] - borders_use[2]);
	
	/* set up map */
	GMT_map_setup(borders[0],borders[1],borders[2],borders[3]);

	/* get scaling from degrees to km */
	mb_coor_scale(verbose,0.5*(borders_use[2] + borders_use[3]),
			&mtodeglon,&mtodeglat);

	/* get color palette file */
	GMT_read_cpt(cptfile);
	if (GMT_gray || GMT_b_and_w) 
		image = MBSWATH_IMAGE_8;
	if (GMT_n_colors <= 0)
		{
		fprintf(stderr,"\nColor pallette table not properly specified:\n");
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_PARAMETER;
		exit(error);
		}

	/* if overlaying amplitude on bathymetry and cptshadefile specified
		read in grayscale cpt to modulate shading */
	if (ampshademode == 1 && mode == MBSWATH_BATH_AMP)
		{
		/* open shade control cpt file */
		if ((fp = fopen(cptshadefile,"r")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open shading control cpt file: %s\n",
				cptshadefile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		/* count lines in file */
		nshadelevel = 0;
		while (fgets(line,MB_PATH_MAXLINE,fp) != NULL)
			nshadelevel++;
		nshadelevel++;
		fclose(fp);

		/* allocate memory */
		status = mb_mallocd(verbose, __FILE__, __LINE__, nshadelevel*sizeof(double), (void **)&shadelevel, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nshadelevel*sizeof(int), (void **)&shadelevelgray, &error);

		/* reopen shade control cpt file */
		if ((fp = fopen(cptshadefile,"r")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open shading control cpt file: %s\n",
				cptshadefile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		/* read shading levels from file */
		nshadelevel = 0;
		while (fgets(line,MB_PATH_MAXLINE,fp) != NULL)
			{
			count = sscanf(line,"%lf %d %d %d %lf %d %d %d",
				&shadelevel[nshadelevel], &r1, &g1, &b1, 
				&shadelevel[nshadelevel+1], &r2, &g2, &b2);
			shadelevelgray[nshadelevel] = (int)(r1 + g1 + b1)/3;
			shadelevelgray[nshadelevel] = 
				MIN(255, shadelevelgray[nshadelevel]);
			shadelevelgray[nshadelevel] = 
				MAX(0, shadelevelgray[nshadelevel]);
			shadelevelgray[nshadelevel+1] = (int)(r2 + g2 + b2)/3;
			shadelevelgray[nshadelevel+1] = 
				MIN(255, shadelevelgray[nshadelevel+1]);
			shadelevelgray[nshadelevel+1] = 
				MAX(0, shadelevelgray[nshadelevel+1]);
			if (count == 8)
				nshadelevel++;
			}
		if (nshadelevel > 0)
			nshadelevel++;
		fclose(fp);

		/* check if cpt data was read */
		if (nshadelevel < 2)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to read proper shading control cpt data from file: %s\n",
				cptshadefile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		}

	/* initialize plotting */
	GMT_plotinit (argc, argv);

	/* set clip path */
	GMT_geo_to_xy(borders_use[0],borders_use[2],&clipx[0],&clipy[0]);
	GMT_geo_to_xy(borders_use[1],borders_use[2],&clipx[1],&clipy[1]);
	GMT_geo_to_xy(borders_use[1],borders_use[3],&clipx[2],&clipy[2]);
	GMT_geo_to_xy(borders_use[0],borders_use[3],&clipx[3],&clipy[3]);
	GMT_map_clip_on (GMT_no_rgb, 3);

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
			status = mb_mallocd(verbose, __FILE__, __LINE__, nm*sizeof(char),
					(void **)&bitimage, &error);

			/* set image to background color */
			gray = YIQ (gmtdefs.page_rgb); 
			for (j=0;j<nm;j++)
				bitimage[j] = gray;
			}
		else
			{
			/* allocate image */
			status = mb_mallocd(verbose, __FILE__, __LINE__, 3*nm*sizeof(char),
					(void **)&bitimage, &error);

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
			    file,&format,&file_weight,&error))
			    == MB_SUCCESS)
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
	    /* check for mbinfo file - get file bounds if possible */
	    status = mb_check_info(verbose, file, lonflip, borders_use, 
			    &file_in_bounds, &error);
	    if (status == MB_FAILURE)
		    {
		    file_in_bounds = MB_YES;
		    status = MB_SUCCESS;
		    error = MB_ERROR_NO_ERROR;
		    }

	    /* read if data may be in bounds */
	    if (file_in_bounds == MB_YES)
		{
		/* check for "fast bathymetry" or "fbt" file */
		if (mode == MBSWATH_BATH
		    || mode == MBSWATH_BATH_RELIEF)
		    {
		    mb_get_fbt(verbose, file, &format, &error);
		    }
		    
		/* check for filtered amplitude or sidescan file */
		if (filtermode == MBSWATH_FILTER_AMP)
		    {
		    if ((status = mb_get_ffa(verbose, file, &format, &error)) != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error returned from function <mb_get_ffa>:\n%s\n",message);
			fprintf(stderr,"Requested filtered amplitude file missing\n");
			fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",file);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		    }
		else if (filtermode == MBSWATH_FILTER_SIDESCAN)
		    {
		    if ((status = mb_get_ffs(verbose, file, &format, &error)) != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error returned from function <mb_get_ffs>:\n%s\n",message);
			fprintf(stderr,"Requested filtered sidescan file missing\n");
			fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",file);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		    }
	    
		/* call mb_read_init() */
		if ((status = mb_read_init(
		    verbose,file,format,pings,lonflip,bounds,
		    btime_i,etime_i,speedmin,timegap,
		    &mbio_ptr,&btime_d,&etime_d,
		    &beams_bath_max,&beams_amp_max,&pixels_ss_max,&error)) != MB_SUCCESS)
		    {
		    mb_error(verbose,error,&message);
		    fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		    fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",file);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }
    
		/* get fore-aft beam_width */
		status = mb_format_beamwidth(verbose, &format, 
				&beamwidth_xtrack, &beamwidth_ltrack, 
				&error);
		if (beamwidth_ltrack <= 0.0)
			beamwidth_ltrack = 2.0;
		if (footprint_mode == MBSWATH_FOOTPRINT_REAL)
		    factor = rawfactor*pings*beamwidth_ltrack;
		else
		    factor = rawfactor;
		    
		/* set default depth, checking for deep-tow data */
		if ((format == 111 || format == 112)
		    && default_depth <= 0.0)
		    default_depth_use = 100.0;
		else
		    default_depth_use = default_depth;
    
		/* allocate memory for data arrays */
		status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct swath),
				(void **)&swath_plot, &error);
		npings = &swath_plot->npings;
		swath_plot->beams_bath = beams_bath_max;
		swath_plot->beams_amp = beams_amp_max;
		swath_plot->pixels_ss = pixels_ss_max;
		for (i=0;i<MAXPINGS;i++)
		    {
		    pingcur = &(swath_plot->data[i]);
		    pingcur->beamflag = NULL;
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
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(char), (void **)&(pingcur->beamflag), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&(pingcur->bath), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							    sizeof(double), (void **)&(pingcur->amp), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&(pingcur->bathlon), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&(pingcur->bathlat), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
							    sizeof(double), (void **)&(pingcur->ss), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
							    sizeof(double), (void **)&(pingcur->sslon), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
							    sizeof(double), (void **)&(pingcur->sslat), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, 
							    sizeof(int), (void **)&(pingcur->bathflag), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, 
							    sizeof(struct footprint), (void **)&(pingcur->bathfoot), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
							    sizeof(int), (void **)&(pingcur->ssflag), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
							    sizeof(struct footprint), (void **)&(pingcur->ssfoot), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&(pingcur->bathshade), &error);
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
		    status = mb_read(verbose,mbio_ptr,&(pingcur->kind),
			    &(pingcur->pings),pingcur->time_i,&(pingcur->time_d),
			    &(pingcur->navlon),&(pingcur->navlat),
			    &(pingcur->speed),&(pingcur->heading),
			    &(pingcur->distance),&(pingcur->altitude),
			    &(pingcur->sonardepth),
			    &(pingcur->beams_bath),
			    &(pingcur->beams_amp),
			    &(pingcur->pixels_ss),
			    pingcur->beamflag,pingcur->bath,pingcur->amp,
			    pingcur->bathlon,pingcur->bathlat,
			    pingcur->ss,pingcur->sslon,pingcur->sslat,
			    pingcur->comment,&error);
		    beamflag = pingcur->beamflag;
		    bath = pingcur->bath;
		    amp = pingcur->amp;
		    bathlon = pingcur->bathlon;
		    bathlat = pingcur->bathlat;
		    ss = pingcur->ss;
		    sslon = pingcur->sslon;
		    sslat = pingcur->sslat;

		    /* print debug statements */
		    if (verbose >= 2)
			    {
			    fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",
				    program_name);
			    fprintf(stderr,"dbg2       kind:           %d\n",
				    pingcur->kind);
			    fprintf(stderr,"dbg2       beams_bath:     %d\n",
				    pingcur->beams_bath);
			    fprintf(stderr,"dbg2       beams_amp:      %d\n",
				    pingcur->beams_amp);
			    fprintf(stderr,"dbg2       pixels_ss:      %d\n",
				    pingcur->pixels_ss);
			    fprintf(stderr,"dbg2       error:          %d\n",
				    error);
			    fprintf(stderr,"dbg2       status:         %d\n",
				    status);
			    for (i=0;i<pingcur->beams_bath;i++)
				    {
				    fprintf(stderr, "bath[%4d]:  %3d  %f  %f  %f\n", 
					    i, beamflag[i], bath[i], bathlon[i], bathlat[i]);
				    }
			    for (i=0;i<pingcur->beams_amp;i++)
				    {
				    fprintf(stderr, "amp[%4d]:  %f  %f  %f\n", 
					    i, amp[i], bathlon[i], bathlat[i]);
				    }
			    for (i=0;i<pingcur->pixels_ss;i++)
				    {
				    fprintf(stderr, "ss[%4d]:  %f  %f  %f\n", 
					    i, ss[i], sslon[i], sslat[i]);
				    }
			    }
			    
		    /* ignore time gaps */
		    if (error == MB_ERROR_TIME_GAP)
		    	    {
			    error = MB_ERROR_NO_ERROR;
			    status = MB_SUCCESS;
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
			    for (i=0;i<pingcur->beams_amp;i++)
				    {
				    if (mb_beam_ok(beamflag[i]) && ampscale_mode == 1)
					{
					amp[i] = ampscale*(amp[i] - ampmin)
					    /(ampmax - ampmin);
					}
				    else if (mb_beam_ok(beamflag[i]) && ampscale_mode == 2)
					{
					amp[i] = MIN(amp[i],ampmax);
					amp[i] = MAX(amp[i],ampmin);
					amp[i] = ampscale*(amp[i] - ampmin)
					    /(ampmax - ampmin);
					}
				    else if (mb_beam_ok(beamflag[i]) && ampscale_mode == 3)
					{
					amplog = 20.0*log10(amp[i]);
					amp[i] = ampscale*(amplog - ampmin)
					    /(ampmax - ampmin);
					}
				    else if (mb_beam_ok(beamflag[i]) && ampscale_mode == 4)
					{
					amplog = 20.0*log10(amp[i]);
					amplog = MIN(amplog,ampmax);
					amplog = MAX(amplog,ampmin);
					amp[i] = ampscale*(amplog - ampmin)
					       /(ampmax - ampmin);
					}
				    }
			    }
    
		    /* scale bathymetry if necessary */
		    if (error == MB_ERROR_NO_ERROR
			    && bathy_in_feet == MB_YES)
			    {
			    for (i=0;i<pingcur->beams_bath;i++)
				    {
				    bath[i] = 3.2808399 * bath[i];
				    }
			    }
    
		    /* scale sidescan if necessary */
		    if (error == MB_ERROR_NO_ERROR
			    && mode == MBSWATH_SS
			    && ampscale_mode > 0)
			    {
			    for (i=0;i<pingcur->pixels_ss;i++)
				    {
				    if (ss[i] > MB_SIDESCAN_NULL && ampscale_mode == 1)
					{
					ss[i] = ampscale*(ss[i] - ampmin)
					    /(ampmax - ampmin);
					}
				    else if (ss[i] > MB_SIDESCAN_NULL && ampscale_mode == 2)
					{
					ss[i] = MIN(ss[i],ampmax);
					ss[i] = MAX(ss[i],ampmin);
					ss[i] = ampscale*(ss[i] - ampmin)
					    /(ampmax - ampmin);
					}
				    else if (ss[i] > MB_SIDESCAN_NULL && ampscale_mode == 3)
					{
					amplog = 20.0*log10(ss[i]);
					ss[i] = ampscale*(amplog - ampmin)
					    /(ampmax - ampmin);
					}
				    else if (ss[i] > MB_SIDESCAN_NULL && ampscale_mode == 4)
					{
					amplog = 20.0*log10(ss[i]);
					amplog = MIN(amplog,ampmax);
					amplog = MAX(amplog,ampmin);
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
			    if (footprint_mode != MBSWATH_FOOTPRINT_POINT)
			    status = get_footprints(verbose,mode,
				    footprint_mode,factor,default_depth_use, 
				    swath_plot,mtodeglon,mtodeglat,&error);


			    /* get shading */
			    if (mode == MBSWATH_BATH_RELIEF 
				    || mode == MBSWATH_BATH_AMP)
				    status = get_shading(verbose,
				    	    mode,ampshademode,swath_plot,
					    mtodeglon,mtodeglat,
					    magnitude,azimuth,
					    nshadelevel, shadelevel, 
					    shadelevelgray,
					    &error);
    
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

			    if (footprint_mode == MBSWATH_FOOTPRINT_POINT)
				    status = plot_data_point(verbose,
					    mode,swath_plot,
					    first,nplot,&error);
			    else 
				    status = plot_data_footprint(verbose,
					    mode,swath_plot,
					    first,nplot,&error);


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
		mb_freed(verbose,__FILE__, __LINE__, (void **)&swath_plot, &error);
		} /* end if file in bounds */
		
	    /* figure out whether and what to read next */
	    if (read_datalist == MB_YES)
                {
		if ((status = mb_datalist_read(verbose,datalist,
			    file,&format,&file_weight,&error))
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

	/* turn off clipping */
	ps_clipoff();

	/* plot image if one is used */
	if (image == MBSWATH_IMAGE_8)
		{
		ps_image (0., 0., x_inch, y_inch, bitimage, nx, ny, 8);
		}
	else if (image == MBSWATH_IMAGE_24)
		{
		GMT_color_image (0., 0., x_inch, y_inch, bitimage, nx, ny, 24);
		}
		
	/* plot basemap if required */
	if (frame_info.plot) 
		{
		ps_setpaint (gmtdefs.basemap_frame_rgb);
		GMT_map_basemap ();
		rgb[0] = 0;
		rgb[1] = 0;
		rgb[2] = 0;
		ps_setpaint (rgb);
		}

	/* end the plot */
	GMT_plotend ();

	/* deallocate image */
	if (image == MBSWATH_IMAGE_8 || image == MBSWATH_IMAGE_24)
		mb_freed(verbose,__FILE__, __LINE__, (void **)&bitimage, &error);
	if (ampshademode == 1 && mode == MBSWATH_BATH_AMP)
		{
		mb_freed(verbose,__FILE__, __LINE__, (void **)&shadelevel, &error);
		mb_freed(verbose,__FILE__, __LINE__, (void **)&shadelevelgray, &error);
		}

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
	GMT_end(argc, argv);
	exit(status);
}
/*--------------------------------------------------------------------*/
int get_footprints(int verbose, int mode, int fp_mode,
		double factor, double depth_def,
		struct swath *swath, 
		double mtodeglon, double mtodeglat, int *error)
{
	char	*function_name = "get_footprints";
	int	status = MB_SUCCESS;

	struct ping	*pingcur;
	struct footprint	*print;
	int	dobath, doss;
	double	headingx, headingy;
	double	dx, dy, r, dlon1, dlon2, dlat1, dlat2, tt, x, y;
	double	ddlonx, ddlaty, rfactor;
	static double	dddepth = 0.0;
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
		fprintf(stderr,"dbg2       swath:       %ld\n",(size_t)swath);
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
		for (j=0;j<pingcur->beams_bath;j++)
			pingcur->bathflag[j] = MB_NO;
		for (j=0;j<pingcur->pixels_ss;j++)
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

		/* get aft looking */
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
		&& mb_beam_ok(swath->data[0].beamflag[pingcur->beams_bath/2])
		&& depth_def <= 0.0)
	  {
	  pingcur = &swath->data[0];
	  headingx = sin(pingcur->heading*DTR);
	  headingy = cos(pingcur->heading*DTR);
	  tt = pingcur->bath[pingcur->beams_bath/2]/750.0; /* in s */
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
		  for (j=1;j<pingcur->beams_bath-1;j++)
		    if (mb_beam_ok(pingcur->beamflag[j]))
			{
			x = pingcur->bathlon[j];
			y = pingcur->bathlat[j];
			setprint = MB_NO;
			if (mb_beam_ok(pingcur->beamflag[j-1])
				&& mb_beam_ok(pingcur->beamflag[j+1]))
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
			else if (mb_beam_ok(pingcur->beamflag[j-1]))
				{
				setprint = MB_YES;
				dlon1 = pingcur->bathlon[j-1] 
					- pingcur->bathlon[j];
				dlat1 = pingcur->bathlat[j-1] 
					- pingcur->bathlat[j];
				dlon2 = -dlon1;
				dlat2 = -dlat1;
				}
			else if (mb_beam_ok(pingcur->beamflag[j+1]))
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
				&& fp_mode == MBSWATH_FOOTPRINT_REAL
				&& depth_def <= 0.0)
				{
				print = &pingcur->bathfoot[j];
				pingcur->bathflag[j] = MB_YES;
				ddlonx = (pingcur->bathlon[j] 
					- pingcur->navlon)/mtodeglon;
				ddlaty = (pingcur->bathlat[j] 
					- pingcur->navlat)/mtodeglat;
				if (depth_def > 0.0)
					dddepth = depth_def;
				else if (pingcur->altitude > 0.0)
					dddepth = pingcur->altitude;
				else
					dddepth = pingcur->bath[j];
				r = rfactor*sqrt(ddlonx*ddlonx 
					+ ddlaty*ddlaty 
					+ dddepth*dddepth);
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
		  for (j=1;j<pingcur->pixels_ss-1;j++)
		    if (pingcur->ss[j] > MB_SIDESCAN_NULL)
			{
			x = pingcur->sslon[j];
			y = pingcur->sslat[j];
			setprint = MB_NO;
			if (pingcur->ss[j-1] > MB_SIDESCAN_NULL 
				&& pingcur->ss[j+1] > MB_SIDESCAN_NULL)
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
			else if (pingcur->ss[j-1] > MB_SIDESCAN_NULL)
				{
				setprint = MB_YES;
				dlon1 = pingcur->sslon[j-1] 
					- pingcur->sslon[j];
				dlat1 = pingcur->sslat[j-1] 
					- pingcur->sslat[j];
				dlon2 = -dlon1;
				dlat2 = -dlat1;
				}
			else if (pingcur->ss[j+1] > MB_SIDESCAN_NULL)
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
				if (depth_def > 0.0)
					dddepth = depth_def;
				else if (pingcur->altitude > 0.0)
					dddepth = pingcur->altitude;
				else if (pingcur->beams_bath > 0 
					&& mb_beam_ok(pingcur->beamflag[pingcur->beams_bath/2]))
					dddepth = pingcur->bath[pingcur->beams_bath/2];
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

		/* do bathymetry with more than 2 soundings */
		if (dobath == MB_YES && pingcur->beams_bath > 2)
		  {
		  j = 0;
		  if (mb_beam_ok(pingcur->beamflag[j]) 
			&& mb_beam_ok(pingcur->beamflag[j+1]))
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
				if (depth_def > 0.0)
					dddepth = depth_def;
				else if (pingcur->altitude > 0.0)
					dddepth = pingcur->altitude;
				else
					dddepth = pingcur->bath[j];
				r = rfactor*sqrt(ddlonx*ddlonx 
					+ ddlaty*ddlaty 
					+ dddepth*dddepth);
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
		  j = pingcur->beams_bath-1;
		  if (mb_beam_ok(pingcur->beamflag[j]) 
			&& mb_beam_ok(pingcur->beamflag[j-1]))
			{
			x = pingcur->bathlon[j];
			y = pingcur->bathlat[j];
			dlon1 = pingcur->bathlon[j-1] 
				- pingcur->bathlon[j];
			dlat1 = pingcur->bathlat[j-1] 
				- pingcur->bathlat[j];
			dlon2 = -dlon1;
			dlat2 = -dlat1;
			print = &pingcur->bathfoot[j];
			pingcur->bathflag[j] = MB_YES;

			/* using fore-aft beam width */
			if (fp_mode == MBSWATH_FOOTPRINT_REAL)
				{
				ddlonx = (pingcur->bathlon[j] 
					- pingcur->navlon)/mtodeglon;
				ddlaty = (pingcur->bathlat[j] 
					- pingcur->navlat)/mtodeglat;
				if (depth_def > 0.0)
					dddepth = depth_def;
				else if (pingcur->altitude > 0.0)
					dddepth = pingcur->altitude;
				else
					dddepth = pingcur->bath[j];
				r = rfactor*sqrt(ddlonx*ddlonx 
					+ ddlaty*ddlaty 
					+ dddepth*dddepth);
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

		/* do bathymetry with 1 sounding */
		if (dobath == MB_YES && fp_mode == MBSWATH_FOOTPRINT_REAL && pingcur->beams_bath == 1)
		  {
		  if (mb_beam_ok(pingcur->beamflag[0]))
			{
			print = &pingcur->bathfoot[0];
			pingcur->bathflag[0] = MB_YES;
			ddlonx = (pingcur->bathlon[0] 
				- pingcur->navlon)/mtodeglon;
			ddlaty = (pingcur->bathlat[0] 
				- pingcur->navlat)/mtodeglat;
			if (depth_def > 0.0)
				dddepth = depth_def;
			else if (pingcur->altitude > 0.0)
				dddepth = pingcur->altitude;
			else
				dddepth = pingcur->bath[0];
			r = rfactor*sqrt(ddlonx*ddlonx 
				+ ddlaty*ddlaty 
				+ dddepth*dddepth);

			dlon2 = -r*headingy*mtodeglon;
			dlat2 = -r*headingx*mtodeglat;
			dlon1 = r*headingy*mtodeglon;
			dlat1 = r*headingx*mtodeglat;
			pingcur->lonaft = -r*headingx*mtodeglon;
			pingcur->lataft = -r*headingy*mtodeglat;
			pingcur->lonfor = r*headingx*mtodeglon;
			pingcur->latfor = r*headingy*mtodeglat;
			print->x[0] = pingcur->bathlon[0] + dlon1 + pingcur->lonaft;
			print->y[0] = pingcur->bathlat[0] + dlat1 + pingcur->lataft;
			print->x[1] = pingcur->bathlon[0] + dlon2 + pingcur->lonaft;
			print->y[1] = pingcur->bathlat[0] + dlat2 + pingcur->lataft;
			print->x[2] = pingcur->bathlon[0] + dlon2 + pingcur->lonfor;
			print->y[2] = pingcur->bathlat[0] + dlat2 + pingcur->latfor;
			print->x[3] = pingcur->bathlon[0] + dlon1 + pingcur->lonfor;
			print->y[3] = pingcur->bathlat[0] + dlat1 + pingcur->latfor;
			}
		  }

		/* do sidescan */
		if (doss == MB_YES && pingcur->pixels_ss > 2)
		  {
		  j = 0;
		  if (pingcur->ss[j] > MB_SIDESCAN_NULL && pingcur->ss[j+1] > MB_SIDESCAN_NULL)
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
				if (depth_def > 0.0)
					dddepth = depth_def;
				else if (pingcur->altitude > 0.0)
					dddepth = pingcur->altitude;
				else if (pingcur->beams_bath > 0 
					&& mb_beam_ok(pingcur->beamflag[pingcur->beams_bath/2]))
					dddepth = pingcur->bath[pingcur->beams_bath/2];
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

		  j = pingcur->pixels_ss-1;
		  if (pingcur->ss[j] > MB_SIDESCAN_NULL && pingcur->ss[j-1] > MB_SIDESCAN_NULL)
			{
			x = pingcur->sslon[j];
			y = pingcur->sslat[j];
			dlon1 = pingcur->sslon[j-1] 
				- pingcur->sslon[j];
			dlat1 = pingcur->sslat[j-1] 
				- pingcur->sslat[j];
			dlon2 = -dlon1;
			dlat2 = -dlat1;
			print = &pingcur->ssfoot[j];
			pingcur->ssflag[j] = MB_YES;

			/* using fore-aft beam width */
			if (fp_mode == MBSWATH_FOOTPRINT_REAL)
				{
				ddlonx = (pingcur->sslon[j] 
					- pingcur->navlon)/mtodeglon;
				ddlaty = (pingcur->sslat[j] 
					- pingcur->navlat)/mtodeglat;
				if (depth_def > 0.0)
					dddepth = depth_def;
				else if (pingcur->altitude > 0.0)
					dddepth = pingcur->altitude;
				else if (pingcur->beams_bath > 0 
					&& mb_beam_ok(pingcur->beamflag[pingcur->beams_bath/2]))
					dddepth = pingcur->bath[pingcur->beams_bath/2];
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
			  for (j=0;j<pingcur->beams_bath;j++)
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
			  for (j=0;j<pingcur->pixels_ss;j++)
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
int get_shading(int verbose, int mode, int ampshademode, struct swath *swath,
		double mtodeglon, double mtodeglat,
		double magnitude, double azimuth,
		int nshadelevel, double *shadelevel, 
		int *shadelevelgray, int *error)
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
	double	graylevel;
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mode:       %d\n",mode);
		fprintf(stderr,"dbg2       swath:      %ld\n",(size_t)swath);
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
	    for (j=0;j<ping1->beams_bath;j++)
	      if (mb_beam_ok(ping1->beamflag[j]))
		{
		/* do across track components */
		drvcount = 0;
		dx = 0.0;
		dy = 0.0;
		dd = 0.0;
		drvx = 0.0;
		drvy = 0.0;
		if (j > 0 && j < ping1->beams_bath - 1
			&& mb_beam_ok(ping1->beamflag[j-1])
			&& mb_beam_ok(ping1->beamflag[j+1]))
			{
			dx = (ping1->bathlon[j+1] - ping1->bathlon[j-1])
				/mtodeglon;
			dy = (ping1->bathlat[j+1] - ping1->bathlat[j-1])
				/mtodeglat;
			dd = ping1->bath[j+1] - ping1->bath[j-1];
			}
		else if (j < ping1->beams_bath - 1
			&& mb_beam_ok(ping1->beamflag[j])
			&& mb_beam_ok(ping1->beamflag[j+1]))
			{
			dx = (ping1->bathlon[j+1] - ping1->bathlon[j])
				/mtodeglon;
			dy = (ping1->bathlat[j+1] - ping1->bathlat[j])
				/mtodeglat;
			dd = ping1->bath[j+1] - ping1->bath[j];
			}
		else if (j > 0
			&& mb_beam_ok(ping1->beamflag[j-1])
			&& mb_beam_ok(ping1->beamflag[j]))
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
			&& mb_beam_ok(ping0->beamflag[j])
			&& mb_beam_ok(ping2->beamflag[j]))
			{
			dx = (ping2->bathlon[j] - ping0->bathlon[j])
				/mtodeglon;
			dy = (ping2->bathlat[j] - ping0->bathlat[j])
				/mtodeglat;
			dd = ping2->bath[j] - ping0->bath[j];
			}
		else if (i < swath->npings - 1
			&& mb_beam_ok(ping1->beamflag[j])
			&& mb_beam_ok(ping2->beamflag[j]))
			{
			dx = (ping2->bathlon[j] - ping1->bathlon[j])
				/mtodeglon;
			dy = (ping2->bathlat[j] - ping1->bathlat[j])
				/mtodeglat;
			dd = ping2->bath[j] - ping1->bath[j];
			}
		else if (i > 0
			&& mb_beam_ok(ping0->beamflag[j])
			&& mb_beam_ok(ping1->beamflag[j]))
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
	if (ampshademode == 1 && mode == MBSWATH_BATH_AMP)
	  {
	  /* get median value from value entered as azimuth */
	  median = azimuth;

	  /* loop over the pings and beams */
	  for (i=0;i<swath->npings;i++)
	    {
	    if (i > 0) ping0 = &swath->data[i-1];
	    ping1 = &swath->data[i];
	    if (i < swath->npings - 1) ping2 = &swath->data[i+1];
	    for (j=0;j<ping1->beams_bath;j++)
	      if (mb_beam_ok(ping1->beamflag[j]))
		{

		/* calculate shading */
		if (mb_beam_ok(ping1->beamflag[j]))
			{
			/* get shading value from cpt grayscale */
			if (ping1->amp[j] < shadelevel[0])
			    graylevel = (double) shadelevelgray[0];
			else if (ping1->amp[j] > shadelevel[nshadelevel-1])
			    graylevel = (double) shadelevelgray[nshadelevel-1];
			else 
			    {
			    for (k=0;k<nshadelevel-1;k++)
				{
				if (ping1->amp[j] > shadelevel[k]
				    && ping1->amp[j] <= shadelevel[k+1])
				    {
				    graylevel = (double) shadelevelgray[k] 
					+ (ping1->amp[j] - shadelevel[k])
					*((double)shadelevelgray[k+1] - (double)shadelevelgray[k])
					/(shadelevel[k+1] - shadelevel[k]);
				    }
				}
			    }
			ping1->bathshade[j] = magnitude*(graylevel - median)/128.;
			}
		else
			ping1->bathshade[j] = 0.0;

		}
	    }
	  }

	/* get shading from amplitude data */
	else if (mode == MBSWATH_BATH_AMP)
	  {
	  /* get median value from value entered as azimuth */
	  median = azimuth;

	  /* loop over the pings and beams */
	  for (i=0;i<swath->npings;i++)
	    {
	    if (i > 0) ping0 = &swath->data[i-1];
	    ping1 = &swath->data[i];
	    if (i < swath->npings - 1) ping2 = &swath->data[i+1];
	    for (j=0;j<ping1->beams_bath;j++)
	      if (mb_beam_ok(ping1->beamflag[j]))
		{

		/* calculate shading */
		if (mb_beam_ok(ping1->beamflag[j]))
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
			for (j=0;j<ping1->beams_bath;j++)
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
int plot_data_footprint(int verbose, int mode,
		struct swath *swath, int first, int nplot, int *error)
{
	char	*function_name = "plot_data_footprint";
	int	status = MB_SUCCESS;
	struct ping	*pingcur;
	struct footprint	*print;
	double	*x, *y;
	double	xx[4], yy[4];
	int	rgb[3];
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mode:       %d\n",mode);
		fprintf(stderr,"dbg2       swath:      %ld\n",(size_t)swath);
		fprintf(stderr,"dbg2       pings:      %d\n",swath->npings);
		fprintf(stderr,"dbg2       first:      %d\n",first);
		fprintf(stderr,"dbg2       nplot:      %d\n",nplot);
		}

	if (mode == MBSWATH_BATH 
		|| mode == MBSWATH_BATH_RELIEF
		|| mode == MBSWATH_BATH_AMP)
		{
		/* loop over all pings and beams and plot the good ones */
		for (i=first;i<first+nplot;i++)
			{
			pingcur = &swath->data[i];
			for (j=0;j<pingcur->beams_bath;j++)
			  if (pingcur->bathflag[j] == MB_YES)
				{
				print = &pingcur->bathfoot[j];
				x = &(print->x[0]);
				y = &(print->y[0]);
				for (k=0;k<4;k++)
					GMT_geo_to_xy(x[k],y[k],&xx[k],&yy[k]);
				GMT_get_rgb24(pingcur->bath[j],rgb);
				if (mode == MBSWATH_BATH_RELIEF 
					|| mode == MBSWATH_BATH_AMP)
					GMT_illuminate(pingcur->bathshade[j],rgb);
/*fprintf(stderr,"Calling plot_box ping:%d of %d   beam:%d of %d\n",
i,nplot,j,pingcur->beams_bath);*/
				status = plot_box(verbose,xx,yy,rgb,error);
				}
			}
		}
	else if (mode == MBSWATH_AMP)
		{
		/* loop over all pings and beams and plot the good ones */
		for (i=first;i<first+nplot;i++)
			{
			pingcur = &swath->data[i];
			for (j=0;j<pingcur->beams_amp;j++)
			  if (pingcur->bathflag[j] == MB_YES)
				{
				print = &pingcur->bathfoot[j];
				x = &(print->x[0]);
				y = &(print->y[0]);
				for (k=0;k<4;k++)
					GMT_geo_to_xy(x[k],y[k],&xx[k],&yy[k]);
				GMT_get_rgb24(pingcur->amp[j],rgb);
				status = plot_box(verbose,xx,yy,rgb,error);
				}
			}
		}
	else if (mode == MBSWATH_SS)
		{
		/* loop over all pings and beams and plot the good ones */
		for (i=first;i<first+nplot;i++)
			{
			pingcur = &swath->data[i];
			for (j=0;j<pingcur->pixels_ss;j++)
			  if (pingcur->ssflag[j] == MB_YES)
				{
				print = &pingcur->ssfoot[j];
				x = &(print->x[0]);
				y = &(print->y[0]);
				for (k=0;k<4;k++)
					GMT_geo_to_xy(x[k],y[k],&xx[k],&yy[k]);
				GMT_get_rgb24(pingcur->ss[j],rgb);
				status = plot_box(verbose,xx,yy,rgb,error);
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
int plot_data_point(int verbose, int mode, 
		struct swath *swath, int first, int nplot, int *error)
{
	char	*function_name = "plot_data_point";
	int	status = MB_SUCCESS;
	struct ping	*pingcur;
	double	xx, yy;
	int	rgb[3];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mode:       %d\n",mode);
		fprintf(stderr,"dbg2       swath:      %ld\n",(size_t)swath);
		fprintf(stderr,"dbg2       pings:      %d\n",swath->npings);
		fprintf(stderr,"dbg2       first:      %d\n",first);
		fprintf(stderr,"dbg2       nplot:      %d\n",nplot);
		}

	if (mode == MBSWATH_BATH 
		|| mode == MBSWATH_BATH_RELIEF
		|| mode == MBSWATH_BATH_AMP)
		{
		/* loop over all pings and beams and plot the good ones */
		for (i=first;i<first+nplot;i++)
			{
			pingcur = &swath->data[i];
			for (j=0;j<pingcur->beams_bath;j++)
			  if (mb_beam_ok(pingcur->beamflag[j]))
				{
				GMT_geo_to_xy(pingcur->bathlon[j], 
					pingcur->bathlat[j], 
					&xx, &yy);
				GMT_get_rgb24(pingcur->bath[j],rgb);
				if (mode == MBSWATH_BATH_RELIEF 
					|| mode == MBSWATH_BATH_AMP)
					GMT_illuminate(pingcur->bathshade[j],
						rgb);
				status = plot_point(verbose,xx,yy,rgb,error);
				}
			}
		}
	else if (mode == MBSWATH_AMP)
		{
		/* loop over all pings and beams and plot the good ones */
		for (i=first;i<first+nplot;i++)
			{
			pingcur = &swath->data[i];
			for (j=0;j<pingcur->beams_amp;j++)
			  if (mb_beam_ok(pingcur->beamflag[j]))
				{
				GMT_geo_to_xy(pingcur->bathlon[j], 
					pingcur->bathlat[j], 
					&xx, &yy);
				GMT_get_rgb24(pingcur->amp[j],rgb);
				status = plot_point(verbose,xx,yy,rgb,error);
				}
			}
		}
	else if (mode == MBSWATH_SS)
		{
		/* loop over all pings and beams and plot the good ones */
		for (i=first;i<first+nplot;i++)
			{
			pingcur = &swath->data[i];
			for (j=0;j<pingcur->pixels_ss;j++)
			  if (pingcur->ss[j] > MB_SIDESCAN_NULL)
				{
				GMT_geo_to_xy(pingcur->sslon[j], 
					pingcur->sslat[j], 
					&xx, &yy);
				GMT_get_rgb24(pingcur->ss[j],rgb);
				status = plot_point(verbose,xx,yy,rgb,error);
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
int plot_box(int verbose, double *x, double *y, int *rgb, int *error)
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
		fprintf(stderr,"dbg2       rgb[0]:     %d\n",rgb[0]);
		fprintf(stderr,"dbg2       rgb[1]:     %d\n",rgb[1]);
		fprintf(stderr,"dbg2       rgb[2]:     %d\n",rgb[2]);
		}

	/* if simple case just plot polygon */
	if (image == MBSWATH_IMAGE_VECTOR)
		{
		ps_polygon(x,y,4,rgb,0);
		}

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
			bitimage[k] = (unsigned char) YIQ (rgb);
			}
		      else if (gmtdefs.color_image == 2)
			{
			k = nx*(ny - iyy) + ixx;
			bitimage[k] = rgb[0];
			bitimage[k+nm] = rgb[1];
			bitimage[k+nm2] = rgb[2];
			}
		      else
			{
			k = 3*(nx*(ny - iyy) + ixx);
			bitimage[k] = rgb[0];
			bitimage[k+1] = rgb[1];
			bitimage[k+2] = rgb[2];
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
			bitimage[k] = (unsigned char) YIQ (rgb);
			}
		    else if (gmtdefs.color_image == 2)
			{
			k = nx*(ny - iyy) + ixx;
			bitimage[k] = rgb[0];
			bitimage[k+nm] = rgb[1];
			bitimage[k+nm2] = rgb[2];
			}
		    else
			{
			k = 3*(nx*(ny - iyy) + ixx);
			bitimage[k] = rgb[0];
			bitimage[k+1] = rgb[1];
			bitimage[k+2] = rgb[2];
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
int plot_point(int verbose, double x, double y, int *rgb, int *error)
{
	char	*function_name = "plot_point";
	int	status = MB_SUCCESS;
	int	ix, iy;
	int	k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       x:          %f\n",x);
		fprintf(stderr,"dbg2       y:          %f\n",y);
		fprintf(stderr,"dbg2       rgb[0]:     %d\n",rgb[0]);
		fprintf(stderr,"dbg2       rgb[1]:     %d\n",rgb[1]);
		fprintf(stderr,"dbg2       rgb[2]:     %d\n",rgb[1]);
		}

	/* if simple case just plot point */
	if (image == MBSWATH_IMAGE_VECTOR)
		{
		ps_setpaint (rgb);
		ps_cross (x, y, 0.005);
		}

	/* if image plot then plot pixel */
	else if (image == MBSWATH_IMAGE_8 || image == MBSWATH_IMAGE_24)
		{
		/* get pixel */
		ix = nx*x/x_inch;
		iy = ny*y/y_inch;

		/* plot pixel */
		if (image == MBSWATH_IMAGE_8)
			{
			k = nx*(ny - iy) + ix;
			bitimage[k] = (unsigned char ) YIQ(rgb);
			}
		else if (gmtdefs.color_image == 2)
			{
			k = nx*(ny - iy) + ix;
			bitimage[k] = rgb[0];
			bitimage[k+nm] = rgb[1];
			bitimage[k+nm2] = rgb[2];
			}
		else
			{
			k = 3*(nx*(ny - iy) + ix);
			bitimage[k] = rgb[0];
			bitimage[k+1] = rgb[1];
			bitimage[k+2] = rgb[2];
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
int ping_copy(int verbose, int one, int two, struct swath *swath, int *error)
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
		fprintf(stderr,"dbg2       swath:      %ld\n",(size_t)swath);
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
	ping1->altitude = ping2->altitude;
	ping1->sonardepth = ping2->sonardepth;
	strcpy(ping1->comment,ping2->comment);
	ping1->beams_bath = ping2->beams_bath;
	ping1->beams_amp = ping2->beams_amp;
	ping1->pixels_ss = ping2->pixels_ss;
	for (i=0;i<ping1->beams_bath;i++)
		{
		ping1->beamflag[i] = ping2->beamflag[i];
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
	for (i=0;i<ping1->beams_amp;i++)
		{
		ping1->amp[i] = ping2->amp[i];
		}
	for (i=0;i<ping1->pixels_ss;i++)
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

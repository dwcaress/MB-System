/*--------------------------------------------------------------------
 *    The MB-system:	mbcontour.c	3.00	6/4/93
 *    $Id  $
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
 * MBCONTOUR is a GMT compatible utility which creates a color postscript
 * contour map of multibeam swath bathymetry or backscatter data. 
 * Complete maps are made by using MBCONTOUR in conjunction with the 
 * usual GMT programs.  The contour levels and colors can be controlled
 * directly or set implicitly using contour and color change intervals.
 * Contours can also be set to have ticks pointing downhill.
 *
 * Author:	D. W. Caress
 * Date:	June 4, 1993
 *
 * $Log: not supported by cvs2svn $
 * Revision 3.0  1993/06/19  01:17:26  caress
 * Initial version
 * ls
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <time.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"

/* GMT include files */
#include "gmt.h"

/* local define */
#define DTR (M_PI/180.)

/* global structure definitions */
#define MAXPINGS 1000
struct	ping
	{
	int	pings;
	int	kind;
	int	time_i[6];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	*bath;
	double	*bathlon;
	double	*bathlat;
	double	*back;
	double	*backlon;
	double	*backlat;
	char	comment[256];
	};
struct swath
	{
	int	npings;
	int	beams_bath;
	int	beams_back;
	struct ping data[MAXPINGS];
	};

/* global pen variables */
int	ncolor;
int	nlevel;
double	*level;
int	*red;
int	*green;
int	*blue;
int	*label;
int	*tick;

/* global inch to map scale */
double	inchtolon;

main (argc, argv) 
int argc;
char **argv; 
{
	static char rcs_id[] = "$Id: mbcontour.c,v 3.1 1993-06-21 00:04:52 caress Exp $";
	static char program_name[] = "MBCONTOUR";
	static char help_message[] =  "MBCONTOUR is a GMT compatible utility which creates a color postscript \ncontour map of multibeam swath bathymetry or backscatter data.  \nComplete maps are made by using MBCONTOUR in conjunction with the  \nusual GMT programs.  The contour levels and colors can be controlled \ndirectly or set implicitly using contour and color change intervals. \nContours can also be set to have ticks pointing downhill.";
	static char usage_message[] = "mbcontour -Idatalist -Jparameters -Rwest/east/south/north [-Acontour_int/color_int/tick_int/label_int/tick_len/label_hgt -Btickinfo -Ccontourfile -Fred/green/blue -K -Llonflip -M -O -P -ppings -U -Xx-shift -Yy-shift -#copies -V -H]";

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

	/* MBIO read control parameters */
	char	filelist[128];
	FILE	*fp;
	int	format;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[6];
	int	etime_i[6];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	char	file[128];
	int	beams_bath;
	int	beams_back;
	char	*mbio_ptr;

	/* mbio read values */
	struct swath *swath_plot;
	struct ping *pingcur;
	double	*bath;
	double	*bathlon;
	double	*bathlat;
	double	*back;
	double	*backlon;
	double	*backlat;

	/* gmt control variables */
	char	contourfile[128];
	int	monochrome = MB_NO;
	int	plot;
	int	done;
	int	flush;
	int	save_new;
	int	first;
	int	*npings;
	int	nping_read;
	int	nplot;
	int	set_contours;
	double	cont_int;
	double	col_int;
	double	tick_int;
	double	label_int;
	double	tick_len;
	double	label_hgt;
	double	tick_len_map;
	double	label_hgt_map;
	double	clipx[4], clipy[4];

	/* other variables */
	double	x1, y1, x2, y2, xx1, yy1, xx2, yy2;
	char	line[128];
	char	labelstr[128], tickstr[128];
	int	count;
	int	setcolors;
	int	i;

	/* get current mb default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* initialize some values */
	strcpy (file, "stdin");
	strcpy (contourfile,"\0");
	set_contours = MB_NO;
	bounds[0] = 0.0;
	bounds[1] = 0.0;
	bounds[2] = 0.0;
	bounds[3] = 0.0;
	nplot = 20;
	cont_int = 25.;
	col_int = 100.;
	tick_int = 100.;
	label_int = 100.;
	label_hgt = 0.1;
	tick_len = 0.05;
	ncolor = 4;
	nlevel = 0;

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
						&bounds[0], &bounds[1], 
						&bounds[2], &bounds[3]);
					break;
				
				/* Supplemental parameters */
			
				case 'F':
					sscanf (&argv[i][2], "%d/%d/%d",&gmtdefs.basemap_frame_rgb[0],
						&gmtdefs.basemap_frame_rgb[1], &gmtdefs.basemap_frame_rgb[2]);
					break;
				case 'M':
					monochrome = MB_YES;
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
	while ((c = getopt(argc, argv, "VvHhp:L:l:b:E:e:S:s:T:t:I:i:A:a:C:c:N:n:B:MJ:KOPR:UX:x:Y:y:#:")) != -1)
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
		case 'p':
			sscanf (optarg,"%d", &pings);
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
			flag++;
			break;
		case 'b':
			sscanf (optarg, "%d/%d/%d/%d/%d/%d",
				&btime_i[0],&btime_i[1],&btime_i[2],
				&btime_i[3],&btime_i[4],&btime_i[5]);
			break;
		case 'E':
		case 'e':
			sscanf (optarg, "%d/%d/%d/%d/%d/%d",
				&etime_i[0],&etime_i[1],&etime_i[2],
				&etime_i[3],&etime_i[4],&etime_i[5]);
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
		case 'I':
		case 'i':
			sscanf (optarg,"%s", filelist);
			flag++;
			break;
		case 'A':
		case 'a':
			sscanf (optarg, "%lf/%lf/%lf/%lf/%lf/%lf",
				&cont_int,&col_int,
				&tick_int,&label_int,
				&tick_len,&label_hgt);
			break;
		case 'C':
		case 'c':
			sscanf (optarg,"%s", contourfile);
			set_contours = MB_YES;
			break;
		case 'N':
		case 'n':
			sscanf (optarg,"%d", &nplot);
			if (nplot < 3) nplot = 3;
			break;
		case 'B':
		case 'J':
		case 'K':
		case 'O':
		case 'P':
		case 'R':
		case 'U':
		case 'X':
		case 'x':
		case 'Y':
		case 'y':
		case '#':
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
		fprintf(stderr,"dbg2       pings:            %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:          %d\n",lonflip);
		fprintf(stderr,"dbg2       btime_i[0]:       %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]:       %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]:       %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]:       %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]:       %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]:       %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       etime_i[0]:       %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:       %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:       %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:       %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:       %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:       %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       speedmin:         %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:          %f\n",timegap);
		fprintf(stderr,"dbg2       file list:        %s\n",filelist);
		fprintf(stderr,"dbg2       bounds[0]:        %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:        %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:        %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:        %f\n",bounds[3]);
		fprintf(stderr,"dbg2       contour interval: %f\n",cont_int);
		fprintf(stderr,"dbg2       color interval:   %f\n",col_int);
		fprintf(stderr,"dbg2       tick interval:    %f\n",tick_int);
		fprintf(stderr,"dbg2       label interval:   %f\n",label_int);
		fprintf(stderr,"dbg2       tick length:      %f\n",tick_len);
		fprintf(stderr,"dbg2       label height:     %f\n",label_hgt);
		fprintf(stderr,"dbg2       number contoured: %d\n",nplot);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(MB_ERROR_NO_ERROR);
		}

	/* if bounds not specified then quit */
	if (bounds[0] >= bounds[1] || bounds[2] >= bounds[3]
		|| bounds[2] <= -90.0 || bounds[3] >= 90.0)
		{
		fprintf(stderr,"\nRegion bounds not properly specified:\n\t%f %f %f %f\n",bounds[0],bounds[1],bounds[2],bounds[3]);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_ERROR_BAD_PARAMETER);
		}

	/* read contours from file */
	if (set_contours == MB_YES)
		{
		/* open contour file */
		if ((fp = fopen(contourfile,"r")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open contour file: %s\n",
				contourfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		/* count lines in file */
		nlevel = 0;
		while (fgets(line,128,fp) != NULL)
			nlevel++;
		fclose(fp);

		/* set number of colors equal to levels */
		ncolor = nlevel;

		/* allocate memory */
		status = mb_malloc(verbose,nlevel*sizeof(double),&level,&error);
		status = mb_malloc(verbose,nlevel*sizeof(int),&label,&error);
		status = mb_malloc(verbose,nlevel*sizeof(int),&tick,&error);
		status = mb_malloc(verbose,ncolor*sizeof(int),&red,&error);
		status = mb_malloc(verbose,ncolor*sizeof(int),&green,&error);
		status = mb_malloc(verbose,ncolor*sizeof(int),&blue,&error);

		/* reopen contour file */
		if ((fp = fopen(contourfile,"r")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open contour file: %s\n",
				contourfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		/* read contour levels from file */
		nlevel = 0;
		while (fgets(line,128,fp) != NULL)
			{
			count = sscanf(line,"%lf %s %s %d %d %d",
				&level[nlevel],labelstr,tickstr,
				&red[nlevel],&green[nlevel],&blue[nlevel]);
			setcolors = MB_YES;
			if (count >= 2 && labelstr[0] == 'a')
				label[nlevel] = 1;
			else if (count >= 2 && labelstr[0] == 'n')
				label[nlevel] = 0;
			else
				{
				label[nlevel] = 0;
				setcolors = MB_NO;
				}
			if (count >= 3 && tickstr[0] == 't')
				tick[nlevel] = 1;
			else if (count >= 3 && tickstr[0] == 'n')
				tick[nlevel] = 1;
			else
				{
				tick[nlevel] = 0;
				setcolors = MB_NO;
				}
			if (count < 6 || setcolors == MB_NO)
				{
				red[nlevel] = 0;
				green[nlevel] = 0;
				blue[nlevel] = 0;
				}
			if (count > 0) nlevel++;
			}
		fclose(fp);
		}

	/* else set default colors and use contour intervals */
	else
		{
		/* set defaults */
		nlevel = 0;
		ncolor = 4;

		/* allocate memory */
		status = mb_malloc(verbose,ncolor*sizeof(int),&red,&error);
		status = mb_malloc(verbose,ncolor*sizeof(int),&green,&error);
		status = mb_malloc(verbose,ncolor*sizeof(int),&blue,&error);

		/* set colors */
		red[0] =   0; green[0] =   0; blue[0] =   0; /* black */
		red[1] = 255; green[1] =   0; blue[1] =   0; /* red */
		red[2] =   0; green[2] = 200; blue[2] =   0; /* green */
		red[3] =   0; green[3] =   0; blue[3] = 255; /* blue */
		}

	/* set up map */
	map_setup(bounds[0],bounds[1],bounds[2],bounds[3]);

	/* initialize plotting */
	ps_plotinit (NULL, gmtdefs.overlay, gmtdefs.page_orientation, 
		gmtdefs.x_origin, gmtdefs.y_origin,
		gmtdefs.global_x_scale, gmtdefs.global_y_scale, 
		gmtdefs.n_copies, gmtdefs.dpi, gmtdefs.measure_unit, 
		gmtdefs.paper_width, gmt_epsinfo (argv[0]));
	echo_command (argc, argv);
	if (gmtdefs.unix_time) 
		timestamp (TIME_STAMP_X, TIME_STAMP_Y, argc, argv);

	/* set clip path */
	geo_to_xy(bounds[0],bounds[2],&clipx[0],&clipy[0]);
	geo_to_xy(bounds[1],bounds[2],&clipx[1],&clipy[1]);
	geo_to_xy(bounds[1],bounds[3],&clipx[2],&clipy[2]);
	geo_to_xy(bounds[0],bounds[3],&clipx[3],&clipy[3]);
	ps_clipon(clipx,clipy,4,-1,-1,-1,3);

	/* get inches to longitude scale */
	x1 = 0.0;
	y1 = 0.0;
	x2 = 1.0;
	y2 = 0.0;
	xy_to_geo(&xx1,&yy1,x1,y1);
	xy_to_geo(&xx2,&yy2,x2,y2);
	inchtolon = xx2 - xx1;
	label_hgt_map = inchtolon*label_hgt;
	tick_len_map = inchtolon*tick_len;

	/* open file list */
	nping_read = 0;
	if ((fp = fopen(filelist,"r")) == NULL)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open data list file: %s\n",
			filelist);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* loop over files in file list */
	if (verbose == 1) 
		fprintf(stderr,"\n");
	while (fgets(line,128,fp) != NULL
		&& sscanf(line,"%s %d",file,&format) == 2)
	{

	/* initialize reading the multibeam file */
	if ((status = mb_read_init(
		verbose,file,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&mbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_back,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* allocate memory for data arrays */
	status = mb_malloc(verbose,sizeof(struct swath),
			&swath_plot,&error);
	npings = &swath_plot->npings;
	for (i=0;i<MAXPINGS;i++)
		{
		pingcur = &(swath_plot->data[i]);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&(pingcur->bath),&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&(pingcur->bathlon),&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&(pingcur->bathlat),&error);
		status = mb_malloc(verbose,beams_back*sizeof(double),
			&(pingcur->back),&error);
		status = mb_malloc(verbose,beams_back*sizeof(double),
			&(pingcur->backlon),&error);
		status = mb_malloc(verbose,beams_back*sizeof(double),
			&(pingcur->backlat),&error);
		}

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
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
	swath_plot->beams_bath = beams_bath;
	swath_plot->beams_back = beams_back;
	done = MB_NO;
	while (done == MB_NO)
		{
		pingcur = &swath_plot->data[*npings];
		bath = pingcur->bath;
		bathlon = pingcur->bathlon;
		bathlat = pingcur->bathlat;
		back = pingcur->back;
		backlon = pingcur->backlon;
		backlat = pingcur->backlat;
		status = mb_read(verbose,mbio_ptr,&(pingcur->kind),
			&(pingcur->pings),pingcur->time_i,&(pingcur->time_d),
			&(pingcur->navlon),&(pingcur->navlat),&(pingcur->speed),
			&(pingcur->heading),&(pingcur->distance),
			&beams_bath,bath,bathlon,bathlat,
			&beams_back,back,backlon,backlat,
			pingcur->comment,&error);

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",
				program_name);
			fprintf(stderr,"dbg2       kind:           %d\n",
				pingcur->kind);
			fprintf(stderr,"dbg2       beams_bath:     %d\n",
				beams_bath);
			fprintf(stderr,"dbg2       beams_back:     %d\n",
				beams_back);
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

		/* decide whether to plot, whether to 
			save the new ping, and if done */
		plot = MB_NO; 
		flush = MB_NO;
		if (*npings >= nplot)
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

			/* print debug statements */
			if (verbose >= 2)
				{
				fprintf(stderr,"\ndbg2  Plotting %d pings in program <%s>\n",
					*npings,program_name);
				for (i=0;i<*npings;i++)
					{
					pingcur = &swath_plot->data[i];
					fprintf(stderr,"dbg2       %4d  %4d %2d %2d %2d %2d %2d\n",
						i,pingcur->time_i[0],
						pingcur->time_i[1],
						pingcur->time_i[2],
						pingcur->time_i[3],
						pingcur->time_i[4],
						pingcur->time_i[5]);
					}
				}

			/* plot data */
			mb_contour(verbose,swath_plot,cont_int,col_int,
				tick_int,label_int,
				tick_len_map,label_hgt_map,
				ncolor,nlevel,level,label,tick,&error);


			/* reorganize data */
			if (flush == MB_YES && save_new == MB_YES)
				{
				status = ping_copy(verbose,0,*npings,
					swath_plot,&error);
				*npings = 1;
				}
			else if (flush == MB_YES)
				{
				*npings = 0;
				}
			else if (*npings > 1)
				{
				status = ping_copy(verbose,0,*npings-1,
						swath_plot,&error);
				*npings = 1;
				}

			}
		}
	status = mb_close(verbose,mbio_ptr,&error);

	/* deallocate memory for data arrays */
	for (i=0;i<*npings;i++)
		{
		pingcur = &swath_plot->data[i];
		mb_free(verbose,pingcur->bath,&error);
		mb_free(verbose,pingcur->bathlon,&error);
		mb_free(verbose,pingcur->bathlat,&error);
		mb_free(verbose,pingcur->back,&error);
		mb_free(verbose,pingcur->backlon,&error);
		mb_free(verbose,pingcur->backlat,&error);
		}
	mb_free(verbose,swath_plot,&error);
	/* end loop over files in list */
	}
	fclose (fp);

	/* turn off clipping */
	ps_clipoff();

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

	/* deallocate memory for data arrays */
	mb_free(verbose,level,&error);
	mb_free(verbose,label,&error);
	mb_free(verbose,tick,&error);
	mb_free(verbose,red,&error);
	mb_free(verbose,green,&error);
	mb_free(verbose,blue,&error);

	/* print ending info */
	if (verbose >= 1)
		fprintf(stderr,"\n%d pings read and plotted\n",
			nping_read);

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
	for (i=0;i<6;i++)
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
		}
	for (i=0;i<swath->beams_back;i++)
		{
		ping1->back[i] = ping2->back[i];
		ping1->backlon[i] = ping2->backlon[i];
		ping1->backlat[i] = ping2->backlat[i];
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
void plot(x,y,ipen)
double x,y;
int ipen;
{
	double	xx, yy;
	geo_to_xy(x,y,&xx,&yy);
	ps_plot(xx,yy,ipen);
	return;
}
/*--------------------------------------------------------------------*/
void newpen(ipen)
int ipen;
{
	if (ipen > -1 && ipen < ncolor)
		ps_setpaint(red[ipen],green[ipen],blue[ipen]);
	return;
}
/*--------------------------------------------------------------------*/
void justify_string(height,string,s)
double	height;
char	*string;
double	*s;
{
	int	len;

	len = strlen(string);
	s[0] = 0.0;
	s[1] = 0.25*height*len;
	s[2] = 0.5*len*height;
	s[3] = 0.5*len*height;

	return;
}
/*--------------------------------------------------------------------*/
void plot_string(x,y,hgt,angle,label)
double	x;
double	y;
double	hgt;
double	angle;
char	*label;
{
	int	point;
	double	height;
	double	xx, yy;

	height = hgt/inchtolon;
	point = height*72.;
	geo_to_xy(x,y,&xx,&yy);
	ps_text(xx,yy,point,label,angle,5,0);

	return;
}
/*--------------------------------------------------------------------*/

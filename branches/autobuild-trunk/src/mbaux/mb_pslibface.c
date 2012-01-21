/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


/*--------------------------------------------------------------------
 *    The MB-system:	mb_pslibface.c	5/15/94
 *    $Id: mb_pslibface.c 1891 2011-05-04 23:46:30Z caress $
 *
 *    Copyright (c) 1993-2011 by
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
 * MB_PSLIBFACE is a set of functions which provide an interface
 * between the program MBCONTOUR and the PSLIB Postscript plotting 
 * library from GMT.  This code is separated from MBCONTOUR so that
 * a similar set of interface functions for pen plotting can be linked
 * to the same source code.
 *
 * Author:	D. W. Caress
 * Date:	May 15, 1994
 *
 * $Log: mb_pslibface.c,v $
 * Revision 5.7  2008/07/10 06:43:40  caress
 * Preparing for 5.1.1beta20
 *
 * Revision 5.6  2006/12/15 21:42:49  caress
 * Incremental CVS update.
 *
 * Revision 5.5  2006/06/22 04:45:42  caress
 * Working towards 5.1.0
 *
 * Revision 5.4  2006/06/02 03:01:30  caress
 * Put in ifdefs to handle new GMT version.
 *
 * Revision 5.3  2006/01/18 15:11:05  caress
 * Had to change ps_text calls to work with pslib from GMT 4.1.
 *
 * Revision 5.2  2006/01/11 07:33:01  caress
 * Working towards 5.0.8
 *
 * Revision 5.1  2004/05/21 23:19:26  caress
 * Changes to support GMT 4.0
 *
 * Revision 5.0  2000/12/01 22:53:59  caress
 * First cut at Version 5.0.
 *
 * Revision 4.14  2000/10/11  01:01:26  caress
 * Convert to ANSI C
 *
 * Revision 4.13  2000/10/11  01:00:12  caress
 * Convert to ANSI C
 *
 * Revision 4.12  2000/09/30  06:54:58  caress
 * Snapshot for Dale.
 *
 * Revision 4.11  1999/04/16  01:24:27  caress
 * Final version 4.6 release?
 *
 * Revision 4.10  1999/02/04  23:41:29  caress
 * MB-System version 4.6beta7
 *
 * Revision 4.9  1998/10/04  04:18:07  caress
 * MB-System version 4.6beta
 *
 * Revision 4.8  1997/09/15  19:03:27  caress
 * Real Version 4.5
 *
 * Revision 4.7  1997/04/21  16:53:56  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.6  1996/04/22  13:18:44  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.6  1996/04/22  13:18:44  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.5  1995/11/15  22:34:22  caress
 * Now handles non-region bounds (lower left point
 * + upper right point) properly.
 *
 * Revision 4.4  1995/08/07  17:31:39  caress
 * Moved to GMT V3.
 *
 * Revision 4.3  1995/03/06  19:39:52  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.2  1994/07/29  19:04:31  caress
 * Changes associated with supporting byte swapped Lynx OS and
 * >> using unix second time base.
 *
 * Revision 4.1  1994/05/25  15:08:49  caress
 * Fixed plot_exit.
 *
 * Revision 4.0  1994/05/16  22:14:24  caress
 * First cut at new contouring scheme.
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* MBIO include files */
#include "mb_status.h"
#include "mb_define.h"
#include "mb_aux.h"

/* GMT include files */
#include "gmt.h"
#include "pslib.h"

/* global variables */
int	argc_save;
char	**argv_save;
double	inchtolon;
int	ncolor;
int	*red;
int	*green;
int	*blue;
int	rgb[3];

static char rcs_id[]="$Id: mb_pslibface.c 1891 2011-05-04 23:46:30Z caress $";

/*--------------------------------------------------------------------------*/
/* 	function plot_init initializes the GMT plotting. */
int plot_init(	int	verbose, 
		int	argc, 
		char	**argv, 
		double	*bounds_use, 
		double	*scale, 
		double	*inch2lon, 
		int	*error)
{
	char	*function_name = "plot_init";
	int	status = MB_SUCCESS;
	int	errflg = 0;
	double	bounds[4];
	double	x1, y1, x2, y2, xx1, yy1, xx2, yy2;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBA function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		fprintf(stderr,"dbg2       argc:             %d\n",argc);
		fprintf(stderr,"dbg2       argv:             %lu\n",(size_t)argv);
		fprintf(stderr,"dbg2       bounds_use[0]:    %f\n",bounds_use[0]);
		fprintf(stderr,"dbg2       bounds_use[1]:    %f\n",bounds_use[1]);
		fprintf(stderr,"dbg2       bounds_use[2]:    %f\n",bounds_use[2]);
		fprintf(stderr,"dbg2       bounds_use[3]:    %f\n",bounds_use[3]);
		fprintf(stderr,"dbg2       scale:            %f\n",*scale);
		}

	/* save argc and argv */
	argc_save = argc;
	argv_save = argv;

	/* deal with gmt options */
	GMT_begin (argc, argv);
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
				case 'c':
				case '\0':
					errflg += GMT_get_common_args (argv[i], 
						&bounds[0], &bounds[1], 
						&bounds[2], &bounds[3]);
					break;
				
				/* Supplemental parameters */
			
				case 'F':
					sscanf (&argv[i][2], "%d/%d/%d",
						&gmtdefs.basemap_frame_rgb[0],
						&gmtdefs.basemap_frame_rgb[1],
						&gmtdefs.basemap_frame_rgb[2]);
					break;
				}
			}
		}

	/* if error flagged then return */
	if (errflg)
		{
		status = MB_FAILURE;
		return(status);
		}

	/* set up map */
	GMT_map_setup(bounds[0],bounds[1],bounds[2],bounds[3]);

	/* initialize plotting */
	GMT_plotinit (argc, argv);

	/* copy bounds in correct order for use by this program */
	if (project_info.region == MB_YES)
		{
		bounds_use[0] = bounds[0];
		bounds_use[1] = bounds[1];
		bounds_use[2] = bounds[2];
		bounds_use[3] = bounds[3];
		}
	else
		{
		bounds_use[0] = bounds[0];
		bounds_use[1] = bounds[2];
		bounds_use[2] = bounds[1];
		bounds_use[3] = bounds[3];
		}

	/* set clip path */
	GMT_map_clip_on (GMT_no_rgb, 3);

	/* get inches to longitude scale */
	x1 = 0.0;
	y1 = 0.0;
	x2 = 1.0;
	y2 = 0.0;
	GMT_xy_to_geo(&xx1,&yy1,x1,y1);
	GMT_xy_to_geo(&xx2,&yy2,x2,y2);
	*inch2lon = xx2 - xx1;
	inchtolon = *inch2lon;

	/* set line width */
	ps_setline(0);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bounds[0]:  %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:  %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:  %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:  %f\n",bounds[3]);
		fprintf(stderr,"dbg2       scale:      %f\n",*scale);
		fprintf(stderr,"dbg2       inchtolon:  %f\n",*inch2lon);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------------*/
/* 	function plot_end ends the GMT plotting. */
int plot_end(int verbose, int *error)
{
	char	*function_name = "plot_end";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBA function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		}

	/* turn off clipping */
	ps_clipoff();

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

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int plot_exit(int argc, char **argv)
{
	int	status = MB_SUCCESS;

	GMT_end(argc, argv);
	
	return(status);
}

/*--------------------------------------------------------------------*/
void set_colors(int ncol, int *rd, int *gn, int *bl)
{
	ncolor = ncol;
	red = rd;
	green = gn;
	blue = bl;
	return;
}

/*--------------------------------------------------------------------*/
void plot(double x, double y, int ipen)
{
	double	xx, yy;
	GMT_geo_to_xy(x,y,&xx,&yy);
	ps_plot(xx,yy,ipen);
	return;
}
/*--------------------------------------------------------------------*/
void setline(int linewidth)
{
        ps_setline(linewidth);
        return;
}
/*--------------------------------------------------------------------*/
void newpen(int ipen)
{
	if (ipen > -1 && ipen < ncolor)
		{
		rgb[0] = red[ipen];
		rgb[1] = green[ipen];
		rgb[2] = blue[ipen];
		ps_setpaint(rgb);
		}
	return;
}
/*--------------------------------------------------------------------*/
void justify_string(double height, char *string, double *s)
{
	int	len;

	len = strlen(string);
	s[0] = 0.0;
	s[1] = 0.185*height*len;
	s[2] = 0.37*len*height;
	s[3] = 0.37*len*height;

	return;
}
/*--------------------------------------------------------------------*/
void plot_string(double x, double y, double hgt, double angle, char *label)
{
	double	point;
	double	height;
	double	xx, yy;

	height = hgt/inchtolon;
	point = height*72.;
	GMT_geo_to_xy(x,y,&xx,&yy);
	ps_text(xx,yy,point,label,angle,5,0);

	return;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
 *    The MB-system:	mb_track.c	8/15/93
 *    $Id: mb_track.c,v 5.0 2000-12-01 22:53:59 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000 by
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
 * mb_track.c plots the shiptrack of swath sonar data.
 *
 * Author:	D. W. Caress
 * Date:	August, 1993
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.8  2000/10/11  00:54:20  caress
 * Converted to ANSI C
 *
 * Revision 4.7  2000/09/30  06:52:17  caress
 * Snapshot for Dale.
 *
 * Revision 4.6  1998/10/04  04:18:07  caress
 * MB-System version 4.6beta
 *
 * Revision 4.5  1997/04/21  16:53:56  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.4  1996/04/22  13:18:44  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.4  1996/04/22  13:18:44  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.3  1995/03/06  19:39:52  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.2  1994/10/21  11:34:20  caress
 * Release V4.0
 *
 * Revision 4.1  1994/07/29  19:04:31  caress
 * Changes associated with supporting byte swapped Lynx OS and
 * >> using unix second time base.
 *
 * Revision 4.0  1994/05/16  22:09:29  caress
 * First cut at new contouring scheme
 *
 * Revision 4.0  1994/03/01  20:50:45  caress
 * First cut at new version.
 *
 * Revision 3.1  1993/11/05  18:58:09  caress
 * Not sure if there are any changes.
 *
 * Revision 3.0  1993/08/26  00:59:59  caress
 * Initial version.
 *
 *
 */

/* standard global include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_contour.h"
#include "../../include/mb_define.h"

/* global array dimensions etc */
#define IUP 3
#define IDN 2
#define IOR -3

/*--------------------------------------------------------------------------*/
/* 	function mb_track plots the shiptrack of multibeam data. */
void mb_track(int verbose, struct swath *data, int *error)
{
  	static char rcs_id[]="$Id: mb_track.c,v 5.0 2000-12-01 22:53:59 caress Exp $";
	char	*function_name = "mb_track";
	int	status = MB_SUCCESS;
	int	time_tick, time_annot, date_annot;
	double	hour0, hour1;
	int	time_j[5];
	double	x, y, x1, y1, x2, y2, x3, y3, x4, y4;
	double	dx, dy;
	double	angle;
	char	label[25];
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:            %d\n",verbose);
		fprintf(stderr,"dbg2       swath:              %d\n",data);
		fprintf(stderr,"dbg2       time tick interval: %f\n",
			data->time_tick_int);
		fprintf(stderr,"dbg2       time interval:      %f\n",
			data->time_annot_int);
		fprintf(stderr,"dbg2       date interval:      %f\n",
			data->date_annot_int);
		fprintf(stderr,"dbg2       time tick length:   %f\n",
			data->time_tick_len);
		}

	/* draw the shiptrack */
	setline(3);
	newpen(0);
	for (i=1;i<data->npings;i++)
		{
/*		boldline(data->pings[i-1].navlon,data->pings[i-1].navlat,
			data->pings[i].navlon,data->pings[i].navlat);*/
		plot(data->pings[i-1].navlon,data->pings[i-1].navlat,IUP);
		plot(data->pings[i].navlon,data->pings[i].navlat,IDN);
		}

	/* draw the time ticks */
	for (i=1;i<data->npings;i++)
		{
		/* get time of day */
		hour0 = data->pings[i-1].time_i[3] 
			+ data->pings[i-1].time_i[4]/60.0 
			+ data->pings[i-1].time_i[5]/3600.0;
		hour1 = data->pings[i].time_i[3] 
			+ data->pings[i].time_i[4]/60.0 
			+ data->pings[i].time_i[5]/3600.0;

		/* check for time tick */
		time_tick = MB_NO;
		if (floor(hour0/data->time_tick_int) 
			< floor(hour1/data->time_tick_int))
			time_tick = MB_YES;

		/* check for time annotation */
		time_annot = MB_NO;
		if (floor(hour0/data->time_annot_int) 
			< floor(hour1/data->time_annot_int))
			time_annot = MB_YES;

		/* check for date annotation */
		date_annot = MB_NO;
		if (floor(hour0/data->date_annot_int) 
			< floor(hour1/data->date_annot_int))
			date_annot = MB_YES;

		/* now get azimuth and location if needed */
		if (date_annot == MB_YES || time_annot == MB_YES 
			|| time_tick == MB_YES)
			{
			/* get azimuth from heading */
			angle = data->pings[i].heading + 90.0;
			if (angle > 360.0)
				angle = angle - 360.0;
			dx = sin(DTR*angle);
			dy = cos(DTR*angle);

			/* cheat and get location by averaging */
			x = 0.5*(data->pings[i-1].navlon 
				+ data->pings[i].navlon);
			y = 0.5*(data->pings[i-1].navlat 
				+ data->pings[i].navlat);
			}

		/* do date annotation if needed */
		if (date_annot == MB_YES)
			{
			x1 = x + 0.375*data->time_tick_len*(dx - dy);
			y1 = y + 0.375*data->time_tick_len*(dy + dx);
			x3 = x + 0.375*data->time_tick_len*(dx + dy);
			y3 = y + 0.375*data->time_tick_len*(dy - dx);
			x2 = x + 0.375*data->time_tick_len*(-dx + dy);
			y2 = y + 0.375*data->time_tick_len*(-dy - dx);
			x4 = x + 0.375*data->time_tick_len*(-dx - dy);
			y4 = y + 0.375*data->time_tick_len*(-dy + dx);
/*			boldline(x1,y1,x2,y2);
			boldline(x3,y3,x4,y4);*/
			plot(x1,y1,IUP);
			plot(x2,y2,IDN);
			plot(x3,y3,IUP);
			plot(x4,y4,IDN);
			mb_get_jtime(verbose,data->pings[i].time_i,time_j);
			sprintf(label," %2.2d:%2.2d/%3.3d",
				data->pings[i].time_i[3],
				data->pings[i].time_i[4],							time_j[1]);
			plot_string(x,y,data->time_tick_len,90.0-angle,label);
		}

		/* do time annotation if needed */
		else if (time_annot == MB_YES)
			{
			x1 = x + 0.375*data->time_tick_len*(dx - dy);
			y1 = y + 0.375*data->time_tick_len*(dy + dx);
			x3 = x + 0.375*data->time_tick_len*(dx + dy);
			y3 = y + 0.375*data->time_tick_len*(dy - dx);
			x2 = x + 0.375*data->time_tick_len*(-dx + dy);
			y2 = y + 0.375*data->time_tick_len*(-dy - dx);
			x4 = x + 0.375*data->time_tick_len*(-dx - dy);
			y4 = y + 0.375*data->time_tick_len*(-dy + dx);
/*			boldline(x1,y1,x2,y2);
			boldline(x3,y3,x4,y4);*/
			plot(x1,y1,IUP);
			plot(x2,y2,IDN);
			plot(x3,y3,IUP);
			plot(x4,y4,IDN);
			sprintf(label,"   %2.2d:%2.2d",
				data->pings[i].time_i[3],
				data->pings[i].time_i[4]);
			plot_string(x,y,data->time_tick_len,90.0-angle,label);
			}

		/* do time tick if needed */
		else if (time_tick == MB_YES)
			{
			x1 = x + 0.25*data->time_tick_len*(dx - dy);
			y1 = y + 0.25*data->time_tick_len*(dy + dx);
			x3 = x + 0.25*data->time_tick_len*(dx + dy);
			y3 = y + 0.25*data->time_tick_len*(dy - dx);
			x2 = x + 0.25*data->time_tick_len*(-dx + dy);
			y2 = y + 0.25*data->time_tick_len*(-dy - dx);
			x4 = x + 0.25*data->time_tick_len*(-dx - dy);
			y4 = y + 0.25*data->time_tick_len*(-dy + dx);
/*			boldline(x1,y1,x2,y2);
			boldline(x3,y3,x4,y4);*/
			plot(x1,y1,IUP);
			plot(x2,y2,IDN);
			plot(x3,y3,IUP);
			plot(x4,y4,IDN);
			}
		}

	/* reset line width */
	setline(0);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return;
}

/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------
 *    The MB-system:	mb_track.c	3.00	8/15/93
 *    $Id: mb_track.c,v 3.1 1993-11-05 18:58:09 caress Exp $
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
 * mb_track.c plots the shiptrack of multibeam bathymetry data.
 *
 * Author:	D. W. Caress
 * Date:	August, 1993
 *
 * $Log: not supported by cvs2svn $
 * Revision 3.0  1993/08/26  00:59:59  caress
 * Initial version.
 *
 *
 */

/* standard global include files */
#include <stdio.h>
#include <math.h>
#include <strings.h>

/* mbio include files */
#include "../../include/mb_status.h"

/* global array dimensions etc */
#define MAXPINGS 1000
#define MAXBEAMS 59
#define MAXHIS 30
#define MAXFLAG MAXPINGS*MAXBEAMS
#define MAXSAVE MAXFLAG
#define PI 3.1415926
#define DTR PI/180.
#define RTD 180./PI
#define IUP 3
#define IDN 2
#define IOR -3

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
struct pingflag
	{
	int	*flag;
	};
struct flagstruct
	{
	struct pingflag pflag[2][MAXPINGS];
	};

/* global shiptrack control variables */
int	npings, beams_bath, center;


/*--------------------------------------------------------------------------*/
/* 	function mb_track plots the shiptrack of multibeam data. */
void mb_track(verbose,swath,time_tick_int,time_annot_int,
				date_annot_int,time_tick_len,error)
int	verbose;
struct swath *swath;
double	time_tick_int,time_annot_int,date_annot_int,time_tick_len;
int	*error;
{
  	static char rcs_id[]="$Id: mb_track.c,v 3.1 1993-11-05 18:58:09 caress Exp $";
	char	*function_name = "mb_track";
	int	status = MB_SUCCESS;
	int	time_tick, time_annot, date_annot;
	double	hour0, hour1;
	int	time_j[4];
	double	x, y, x1, y1, x2, y2, x3, y3, x4, y4, x5, y5;
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
		fprintf(stderr,"dbg2       swath:              %d\n",swath);
		fprintf(stderr,"dbg2       time tick interval: %f\n",
			time_tick_int);
		fprintf(stderr,"dbg2       time interval:      %f\n",
			time_annot_int);
		fprintf(stderr,"dbg2       date interval:      %f\n",
			date_annot_int);
		fprintf(stderr,"dbg2       time tick length:   %f\n",
			time_tick_len);
		}

	/* draw the shiptrack */
	newpen(0);
	for (i=1;i<swath->npings;i++)
		{
		boldline(swath->data[i-1].navlon,swath->data[i-1].navlat,
			swath->data[i].navlon,swath->data[i].navlat);
/*		plot(swath->data[i-1].navlon,swath->data[i-1].navlat,IUP);
		plot(swath->data[i].navlon,swath->data[i].navlat,IDN);*/
		}

	/* draw the time ticks */
	for (i=1;i<swath->npings;i++)
		{
		/* get time of day */
		hour0 = swath->data[i-1].time_i[3] 
			+ swath->data[i-1].time_i[4]/60.0 
			+ swath->data[i-1].time_i[5]/3600.0;
		hour1 = swath->data[i].time_i[3] 
			+ swath->data[i].time_i[4]/60.0 
			+ swath->data[i].time_i[5]/3600.0;

		/* check for time tick */
		time_tick = MB_NO;
		if (floor(hour0/time_tick_int) < floor(hour1/time_tick_int))
			time_tick = MB_YES;

		/* check for time annotation */
		time_annot = MB_NO;
		if (floor(hour0/time_annot_int) < floor(hour1/time_annot_int))
			time_annot = MB_YES;

		/* check for date annotation */
		date_annot = MB_NO;
		if (floor(hour0/date_annot_int) < floor(hour1/date_annot_int))
			date_annot = MB_YES;

		/* now get azimuth and location if needed */
		if (date_annot == MB_YES || time_annot == MB_YES 
			|| time_tick == MB_YES)
			{
			/* get azimuth from heading */
			angle = swath->data[i].heading + 90.0;
			if (angle > 360.0)
				angle = angle - 360.0;
			dx = sin(DTR*angle);
			dy = cos(DTR*angle);

			/* cheat and get location by averaging */
			x = 0.5*(swath->data[i-1].navlon 
				+ swath->data[i].navlon);
			y = 0.5*(swath->data[i-1].navlat 
				+ swath->data[i].navlat);
			}

		/* do date annotation if needed */
		if (date_annot == MB_YES)
			{
			x1 = x + 1.5*time_tick_len*(dx - dy);
			y1 = y + 1.5*time_tick_len*(dy + dx);
			x3 = x + 1.5*time_tick_len*(dx + dy);
			y3 = y + 1.5*time_tick_len*(dy - dx);
			x2 = x + 1.5*time_tick_len*(-dx + dy);
			y2 = y + 1.5*time_tick_len*(-dy - dx);
			x4 = x + 1.5*time_tick_len*(-dx - dy);
			y4 = y + 1.5*time_tick_len*(-dy + dx);
			x5 = x + 2.0*dx*time_tick_len + dy*time_tick_len;
			y5 = y + 2.0*dy*time_tick_len + dx*time_tick_len;
			boldline(x1,y1,x2,y2);
			boldline(x3,y3,x4,y4);
			mb_get_jtime(verbose,swath->data[i].time_i,time_j);
			sprintf(label,"\\com\\ %2.2d:%2.2d/%3.3d\\sim\\",
				swath->data[i].time_i[3],
				swath->data[i].time_i[4],							time_j[1]);
			plot_string(x5,y5,time_tick_len,90.0-angle,label);
		}

		/* do time annotation if needed */
		else if (time_annot == MB_YES)
			{
			x1 = x + 1.5*time_tick_len*(dx - dy);
			y1 = y + 1.5*time_tick_len*(dy + dx);
			x3 = x + 1.5*time_tick_len*(dx + dy);
			y3 = y + 1.5*time_tick_len*(dy - dx);
			x2 = x + 1.5*time_tick_len*(-dx + dy);
			y2 = y + 1.5*time_tick_len*(-dy - dx);
			x4 = x + 1.5*time_tick_len*(-dx - dy);
			y4 = y + 1.5*time_tick_len*(-dy + dx);
			x5 = x + 2.0*dx*time_tick_len + dy*time_tick_len;
			y5 = y + 2.0*dy*time_tick_len + dx*time_tick_len;
			boldline(x1,y1,x2,y2);
			boldline(x3,y3,x4,y4);
			sprintf(label,"\\com\\ %2.2d:%2.2d\\sim\\",swath->data[i].time_i[3],
				swath->data[i].time_i[4]);
			plot_string(x5,y5,time_tick_len,90.0-angle,label);
			}

		/* do time tick if needed */
		else if (time_tick == MB_YES)
			{
			x1 = x + time_tick_len*(dx - dy);
			y1 = y + time_tick_len*(dy + dx);
			x3 = x + time_tick_len*(dx + dy);
			y3 = y + time_tick_len*(dy - dx);
			x2 = x + time_tick_len*(-dx + dy);
			y2 = y + time_tick_len*(-dy - dx);
			x4 = x + time_tick_len*(-dx - dy);
			y4 = y + time_tick_len*(-dy + dx);
			boldline(x1,y1,x2,y2);
			boldline(x3,y3,x4,y4);
			}
		}

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

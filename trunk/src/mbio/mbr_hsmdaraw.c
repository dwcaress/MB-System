/*--------------------------------------------------------------------
 *    The MB-system:	mbr_hsmdaraw.c	2/11/93
 *	$Header: /system/link/server/cvs/root/mbsystem/src/mbio/mbr_hsmdaraw.c,v 4.10 2000-09-30 06:34:20 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 1995, 2000 by
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
 * mbr_hsmdaraw.c contains the functions for reading and writing
 * multibeam data in the HSMDARAW format.  
 * These functions include:
 *   mbr_alm_hsmdaraw	- allocate read/write memory
 *   mbr_dem_hsmdaraw	- deallocate read/write memory
 *   mbr_rt_hsmdaraw	- read and translate data
 *   mbr_wt_hsmdaraw	- translate and write data
 *
 * Author:	Dale Chayes
 * Date:	August 11, 1995
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.9  1999/09/14  20:39:11  caress
 * Fixed bugs handling HSMD
 *
 * Revision 4.8  1999/03/31  18:11:35  caress
 * MB-System 4.6beta7
 *
 * Revision 4.7  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.6  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.5  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.4  1996/07/16  22:07:12  caress
 * Fixed port/starboard mixup and made null angles for raytracing 40 degrees to
 * reflect 40 degree tranducer array mounting.
 *
 * Revision 4.4  1996/07/16  22:07:12  caress
 * Fixed port/starboard mixup and made null angles for raytracing 40 degrees to
 * reflect 40 degree tranducer array mounting.
 *
 * Revision 4.3  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.2  1996/04/22  10:57:09  caress
 * DTR define now in mb_io.h
 *
 * Revision 4.1  1996/01/26  21:23:30  caress
 * Version 4.3 distribution
 *
 * Revision 4.0  1995/09/28  18:14:11  caress
 * First cut.
 *
 * Revision 1.1  1995/09/28  18:10:48  caress
 * Initial revision
 *
 * Revision 4.3  95/08/16  15:53:04  15:53:04  dale (Dale Chayes)
 * Works to a limited extent.
 * Does not handle writing type 101 data files yet.
 * 
 * Revision 4.2  95/08/14  20:55:17  20:55:17  dale (Dale Chayes)
 * HSMD is partially working, at least internally.
 * Still need to fix:
 *  - time for each ping
 *  - position for each ping
 * 
 * Revision 4.1  95/08/13  08:05:35  08:05:35  dale (Dale Chayes)
 * First attempt to compile. Still has a bunch of no longer appropriate
 * variables and does not yet have all the right stuff for HSMD, but 
 * less than 100 errors before the compiler quits.
 * 
 * Revision 4.0  95/08/11  14:09:12  14:09:12  dale (Dale Chayes)
 * First cut, copied from mbr_hsatlraw.c
 * reformated for emacs
 * converted hsds structures and defines to hsmd
 * Code not functionaly modified yet.
 * 
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mb_define.h"
#include "../../include/mbsys_hsmd.h"
#include "../../include/mbf_hsmdaraw.h"

/*--------------------------------------------------------------------*/
int mbr_alm_hsmdaraw(verbose,mbio_ptr,error)
int    verbose;
char   *mbio_ptr;
int    *error;
{
	static char res_id[]="$Header: /system/link/server/cvs/root/mbsystem/src/mbio/mbr_hsmdaraw.c,v 4.10 2000-09-30 06:34:20 caress Exp $";
	char	 *function_name = "mbr_alm_hsmdaraw";
	int	 status = MB_SUCCESS;
	int	 i;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_hsmdaraw_struct *data;
	char	 *data_ptr;
	double *FirstReftime;	/* time from the first header */
	int  *Header_count; /* number of header records encounterd */
	int  *Rev_count;	   /* Raw Event counter */
	int  *Nav_count;	   /* number of Nav records */
	int  *Angle_count;  /* etc....... */
	int  *Svp_count;
	int  *Raw_count;
	int  *MDevent_count;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}
  
	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
  
	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_hsmdaraw_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
		     &mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_hsmd_struct),
		     &mb_io_ptr->store_data,error);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_hsmdaraw_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	
	/* initialize saved values */
	FirstReftime = &mb_io_ptr->saved1;	/* time from the first header */
	Header_count = &mb_io_ptr->save1; /* number of header records encounterd */
	Rev_count = &mb_io_ptr->save2;	   /* Raw Event counter */
	Nav_count = &mb_io_ptr->save3;	   /* number of Nav records */
	Angle_count = &mb_io_ptr->save4;  /* etc....... */
	Svp_count = &mb_io_ptr->save5;
	Raw_count = &mb_io_ptr->save6;
	MDevent_count = &mb_io_ptr->save7;
	*FirstReftime = 0.0;	/* time from the first header */
	*Header_count = 0; /* number of header records encounterd */
	*Rev_count = 0;	   /* Raw Event counter */
	*Nav_count = 0;	   /* number of Nav records */
	*Angle_count = 0;  /* etc....... */
	*Svp_count = 0;
	*Raw_count = 0;
	*MDevent_count = 0;

	/* initialize everything to zeros */
	mbr_zero_hsmdaraw(verbose,data_ptr,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_dem_hsmdaraw(verbose,mbio_ptr,error)
int    verbose;
char   *mbio_ptr;
int    *error;
{
	char    *function_name = "mbr_dem_hsmdaraw";
	int	  status = MB_SUCCESS;
	struct  mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* deallocate memory for data descriptor */
	status = mb_free(verbose,&mb_io_ptr->raw_data,error);
	status = mb_free(verbose,&mb_io_ptr->store_data,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_zero_hsmdaraw(verbose,data_ptr,error)
int    verbose;
char   *data_ptr;
int    *error;
{
	char	 *function_name = "mbr_zero_hsmdaraw";
	int	 status = MB_SUCCESS;
	struct mbf_hsmdaraw_struct *data;
	int	 i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}


	/* get pointer to data descriptor */
	data = (struct mbf_hsmdaraw_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		for (i=0;i<4;i++)
			{
			data->scsid[i]= NULL;
			data->scsart[i]= NULL;
			}
		data->scslng = 0;
		data->scsext = 0;
		data->scsblcnt = 0;
		data->scsres1 = 0.0;
		data->transid = 0;	/* indicates what kind of data */
		data->reftime = -1.0;	/* unitialized */

		data->datuhr = -1.0;

		for (i=0;i<8;i++)
			data->mksysint[i]=NULL;

		for (i=0;i<84;i++)
			data->mktext[i]=NULL;

		data->navid = 0;
		data->year = 0;
		data->month = 0;
		data->day = 0;
		data->hour = 0;
		data->minute = 0;
		data->second = 0;
		data->millisecond = 0.0;

		data->lon = 0.0;
		data->lat = 0.0;


		data->ckeel = 0.0;
		data->cmean = 0.0;
		data->Port = 0;
		data->noho = 0;
		data->skals = 0;

		for (i=0;i<MBF_HSMDARAW_BEAMS_PING;i++)
			{
			data->spfb[i] = 0;
			data->angle[i] = mbf_hsmdaraw_beamangle[i];
			data->depth[i] = 0.0;
			data->distance[i] = 0.0;
			}

		data->ss_range = 0.0;
		for (i=0;i<MBF_HSMDARAW_PIXELS_PING;i++)
			data->ss[i] = 0;

		data->heading_tx = 0.0;
		for (i=0;i<5;i++)
			data->heading_rx[i] = 0.0;

		data->roll_tx = 0.0;
		for (i=0;i<5;i++)
			data->roll_rx[i] = 0.0;

		data->pitch_tx = 0.0;
		for (i=0;i<5;i++)
			data->pitch_rx[i] = 0.0;

		data->num_vel = 0;
		for (i=0;i<MBF_HSMDARAW_MAXVEL;i++)
			{
			data->vdepth[i] = 0.0;
			data->velocity[i] = 0.0;
			}

		}

	/* assume success */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_rt_hsmdaraw(verbose,mbio_ptr,store_ptr,error)
int    verbose;
char   *mbio_ptr;
char   *store_ptr;
int    *error;
{
	char	 *function_name = "mbr_rt_hsmdaraw";
	int	 status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_hsmdaraw_struct *data;
	struct mbsys_hsmd_struct *store;
	double	dd, dt, dx, dy;
	double	mtodeglon, mtodeglat;
	double	headingx, headingy;
	int	ifix;
	int	 i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_hsmdaraw_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_hsmd_struct *) store_ptr;

	/* reset values in mb_io_ptr */
	mb_io_ptr->new_kind = MB_DATA_NONE;
	mb_io_ptr->new_time_i[0] = 0;
	mb_io_ptr->new_time_i[1] = 0;
	mb_io_ptr->new_time_i[2] = 0;
	mb_io_ptr->new_time_i[3] = 0;
	mb_io_ptr->new_time_i[4] = 0;
	mb_io_ptr->new_time_i[5] = 0;
	mb_io_ptr->new_time_i[6] = 0;
	mb_io_ptr->new_time_d = 0.0;
	mb_io_ptr->new_lon = 0.0;
	mb_io_ptr->new_lat = 0.0;
	mb_io_ptr->new_heading = 0.0;
	mb_io_ptr->new_speed = 0.0;
	for (i=0;i<mb_io_ptr->beams_bath;i++)
		{
		mb_io_ptr->new_beamflag[i] = MB_FLAG_NULL;
		mb_io_ptr->new_bath[i] = 0.0;
		mb_io_ptr->new_bath_acrosstrack[i] = 0.0;
		mb_io_ptr->new_bath_alongtrack[i] = 0.0;
		}
	for (i=0;i<mb_io_ptr->pixels_ss;i++)
		{
		mb_io_ptr->new_ss[i] = 0.0;
		mb_io_ptr->new_ss_acrosstrack[i] = 0.0;
		mb_io_ptr->new_ss_alongtrack[i] = 0.0;
		}
  
	/* read next (record of) data from file */
	status = mbr_hsmdaraw_rd_data(verbose,mbio_ptr,error);

	/* print debug statements */
	if (verbose >= 5) 
		{
		fprintf(stderr,"dbg5: In function name:\t%s\n", function_name);
		fprintf(stderr,"dbg5:\t Returned from  mbr_hsmdaraw_rd_data()\n");
		fprintf(stderr,"dbg5:\t Status:\t%d\n", status);
		fprintf(stderr,"dbg5:\t data->kind:\t%d\n", data->kind);
		fprintf(stderr,"dbg5:\t store_ptr: \t%d\n",store_ptr);
		fprintf(stderr,"dbg5:\t Beams_Bath \t%d\n",mb_io_ptr->beams_bath);
		}

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* translate time and navigation values to current 
		ping variables in mbio descriptor structure */
	if (status == MB_SUCCESS
		&& data->kind != MB_DATA_COMMENT)
		{
		/* get time */
		mb_io_ptr->new_time_i[0] = data->year;
		mb_io_ptr->new_time_i[1] = data->month;
		mb_io_ptr->new_time_i[2] = data->day;
		mb_io_ptr->new_time_i[3] = data->hour;
		mb_io_ptr->new_time_i[4] = data->minute;
		mb_io_ptr->new_time_i[5] = data->second;
		mb_io_ptr->new_time_i[6] = 1000 * data->millisecond;
		mb_get_time(verbose, mb_io_ptr->new_time_i, 
			&mb_io_ptr->new_time_d);
		}
			
	/* save fix if nav data */
	if (data->kind == MB_DATA_NAV)
		{
		/* make room for latest fix */
		if (mb_io_ptr->nfix >= MB_NAV_SAVE_MAX)
			{
			for (i=0;i<mb_io_ptr->nfix-1;i++)
				{
				mb_io_ptr->fix_time_d[i]
				    = mb_io_ptr->fix_time_d[i+1];
				mb_io_ptr->fix_lon[i]
				    = mb_io_ptr->fix_lon[i+1];
				mb_io_ptr->fix_lat[i]
				    = mb_io_ptr->fix_lat[i+1];
				}
			mb_io_ptr->nfix--;
			}
		
		/* add latest fix */
		mb_io_ptr->fix_time_d[mb_io_ptr->nfix] 
			= mb_io_ptr->new_time_d;
		mb_io_ptr->fix_lon[mb_io_ptr->nfix] = data->lon;
		mb_io_ptr->fix_lat[mb_io_ptr->nfix] = data->lat;
		mb_io_ptr->nfix++;
		}

	/* copy comment to mbio descriptor structure */
	if (status == MB_SUCCESS
		&& data->kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		strncpy(mb_io_ptr->new_comment,data->comment,
			MBSYS_HSMD_COMMENT);

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  New ping read by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New ping values:\n");
			fprintf(stderr,"dbg4       error:      %d\n",
				mb_io_ptr->new_error);
			fprintf(stderr,"dbg4       comment:    %s\n",
				mb_io_ptr->new_comment);
			}
		}

	/* deal with position data */
	else if (status == MB_SUCCESS 
		&& data->kind == MB_DATA_NAV) 
		{
		mb_io_ptr->new_lon = data->lon;	/* Position  */
		mb_io_ptr->new_lat = data->lat;

		if (mb_io_ptr->lonflip < 0) 
			{
			if (mb_io_ptr->new_lon > 0.) 
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < -360.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}
		else if (mb_io_ptr->lonflip == 0)
			{
			if (mb_io_ptr->new_lon > 180.) 
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < -180.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}
		else 
			{
			if (mb_io_ptr->new_lon > 360.) 
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < 0.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}
		mb_io_ptr->new_speed = data->speed;

		/* print debug statements */
		if (verbose >= 5)	
			{
			fprintf(stderr,"\ndbg4 HSMD Navigation read by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New ping values:\n");
			fprintf(stderr,"dbg4       error:      %d\n",
				mb_io_ptr->new_error);
			fprintf(stderr,"dbg4       kind:       %d\n",
				mb_io_ptr->new_kind);
			fprintf(stderr,"dbg4       time_i[0]:  %d\n",
				mb_io_ptr->new_time_i[0]);
			fprintf(stderr,"dbg4       time_i[1]:  %d\n",
				mb_io_ptr->new_time_i[1]);
			fprintf(stderr,"dbg4       time_i[2]:  %d\n",
				mb_io_ptr->new_time_i[2]);
			fprintf(stderr,"dbg4       time_i[3]:  %d\n",
				mb_io_ptr->new_time_i[3]);
			fprintf(stderr,"dbg4       time_i[4]:  %d\n",
				mb_io_ptr->new_time_i[4]);
			fprintf(stderr,"dbg4       time_i[5]:  %d\n",
				mb_io_ptr->new_time_i[5]);
			fprintf(stderr,"dbg4       time_i[6]:  %d\n",
				mb_io_ptr->new_time_i[6]);
			fprintf(stderr,"dbg4       time_d:     %f\n",
				mb_io_ptr->new_time_d);
			fprintf(stderr,"dbg4       longitude:  %f\n",
				mb_io_ptr->new_lon);
			fprintf(stderr,"dbg4       latitude:   %f\n",
				mb_io_ptr->new_lat);
			}
		} 

	else if (status==MB_SUCCESS 
		&& data->kind==MB_DATA_DATA ) 
		{
		if (verbose >= 5)
			fprintf(stderr,"\ndbg5:\t DATA w/:Port == %d\n",data->Port);

		/* interpolate from saved nav if possible */
		if (mb_io_ptr->nfix > 1)
			{
			/* get speed */
			mb_coor_scale(verbose,
			    mb_io_ptr->fix_lat[mb_io_ptr->nfix-1],
			    &mtodeglon,&mtodeglat);
			dx = (mb_io_ptr->fix_lon[mb_io_ptr->nfix-1]
			    - mb_io_ptr->fix_lon[0])/mtodeglon;
			dy = (mb_io_ptr->fix_lat[mb_io_ptr->nfix-1]
			    - mb_io_ptr->fix_lat[0])/mtodeglat;
			dt = mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1]
			    - mb_io_ptr->fix_time_d[0];
			data->speed = sqrt(dx*dx + dy*dy) /dt / 3.6; /* km/hr */

			/* interpolation possible */
			if (mb_io_ptr->new_time_d 
				>= mb_io_ptr->fix_time_d[0]
			    && mb_io_ptr->new_time_d
				<= mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1])
			    {
			    ifix = 0;
			    while (mb_io_ptr->new_time_d
				> mb_io_ptr->fix_time_d[ifix+1])
				ifix++;
			    mb_io_ptr->new_lon = mb_io_ptr->fix_lon[ifix]
				+ (mb_io_ptr->fix_lon[ifix+1] 
				    - mb_io_ptr->fix_lon[ifix])
				* (mb_io_ptr->new_time_d
				    - mb_io_ptr->fix_time_d[ifix])
				/ (mb_io_ptr->fix_time_d[ifix+1]
				    - mb_io_ptr->fix_time_d[ifix]);
			    mb_io_ptr->new_lat = mb_io_ptr->fix_lat[ifix]
				+ (mb_io_ptr->fix_lat[ifix+1] 
				    - mb_io_ptr->fix_lat[ifix])
				* (mb_io_ptr->new_time_d
				    - mb_io_ptr->fix_time_d[ifix])
				/ (mb_io_ptr->fix_time_d[ifix+1]
				    - mb_io_ptr->fix_time_d[ifix]);
			    }
			
			/* extrapolate from first fix */
			else if (mb_io_ptr->new_time_d 
				< mb_io_ptr->fix_time_d[0])
			    {
			    dd = (mb_io_ptr->new_time_d 
				- mb_io_ptr->fix_time_d[0])
				* 3.6 * data->speed;
			    mb_coor_scale(verbose,mb_io_ptr->fix_lat[0],
				&mtodeglon,&mtodeglat);
			    headingx = sin(DTR*data->heading_tx);
			    headingy = cos(DTR*data->heading_tx);
			    mb_io_ptr->new_lon = mb_io_ptr->fix_lon[0] 
				+ headingx*mtodeglon*dd;
			    mb_io_ptr->new_lat = mb_io_ptr->fix_lat[0] 
				+ headingy*mtodeglat*dd;
			    }
			
			/* extrapolate from last fix */
			else if (mb_io_ptr->new_time_d 
				> mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1])
			    {
			    dd = (mb_io_ptr->new_time_d 
				- mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1])
				* 3.6 * data->speed;
			    mb_coor_scale(verbose,mb_io_ptr->fix_lat[mb_io_ptr->nfix-1],
				&mtodeglon,&mtodeglat);
			    headingx = sin(DTR*data->heading_tx);
			    headingy = cos(DTR*data->heading_tx);
			    mb_io_ptr->new_lon = mb_io_ptr->fix_lon[mb_io_ptr->nfix-1] 
				+ headingx*mtodeglon*dd;
			    mb_io_ptr->new_lat = mb_io_ptr->fix_lat[mb_io_ptr->nfix-1] 
				+ headingy*mtodeglat*dd;
			    }
			
			/* use last fix */
			else
			    {
			    mb_io_ptr->new_lon = mb_io_ptr->fix_lon[mb_io_ptr->nfix-1];
			    mb_io_ptr->new_lat = mb_io_ptr->fix_lat[mb_io_ptr->nfix-1];
			    }
			}
			
		/* else just take last position */
		else if (mb_io_ptr->nfix == 1)
			{
			mb_io_ptr->new_lon = mb_io_ptr->fix_lon[mb_io_ptr->nfix-1];
			mb_io_ptr->new_lat = mb_io_ptr->fix_lat[mb_io_ptr->nfix-1];
			mb_io_ptr->new_speed = 0.0;
			}
		else
			{
			mb_io_ptr->new_lon = 0.0;
			mb_io_ptr->new_lat = 0.0;
			mb_io_ptr->new_speed = 0.0;
			}
		mb_io_ptr->new_speed = data->speed;
    		mb_io_ptr->new_heading = data->heading_tx;
		/* get bathymetry */

		/* deal with a ping to port */
		if (data->Port == -1) 
			{
			for (i=0;i<MBF_HSMDARAW_BEAMS_PING;i++) 
			    {
			    j = MBF_HSMDARAW_BEAMS_PING - i - 1;
			    if (data->depth[i] > 0.0)
				{
				mb_io_ptr->new_beamflag[j] = MB_FLAG_NONE;
				mb_io_ptr->new_bath[j] = data->depth[i];
				}
			    else if (data->depth[i] < 0.0)
				{
				mb_io_ptr->new_beamflag[j] = 
				    MB_FLAG_MANUAL + MB_FLAG_FLAG;
				mb_io_ptr->new_bath[j] = -data->depth[i];
				}
			    else
				{
				mb_io_ptr->new_beamflag[j] = MB_FLAG_NULL;
				mb_io_ptr->new_bath[j] = data->depth[i];
				}
			    mb_io_ptr->new_bath_acrosstrack[j] = 
					    data->distance[i]; 
			    }
    			}

		/* deal with a ping to starboard */
		else 
			{
			for (i=0;i<MBF_HSMDARAW_BEAMS_PING;i++) 
			    {
			    j = i + MBF_HSMDARAW_BEAMS_PING - 1;
			    if (data->depth[i] > 0.0)
				{
				mb_io_ptr->new_beamflag[j] = MB_FLAG_NONE;
				mb_io_ptr->new_bath[j] = data->depth[i];
				}
			    else if (data->depth[i] < 0.0)
				{
				mb_io_ptr->new_beamflag[j] = 
				    MB_FLAG_MANUAL + MB_FLAG_FLAG;
				mb_io_ptr->new_bath[j] = data->depth[i];
				}
			    else
				{
				mb_io_ptr->new_beamflag[j] = MB_FLAG_NULL;
				mb_io_ptr->new_bath[j] = data->depth[i];
				}
			    mb_io_ptr->new_bath_acrosstrack[j] = 
					    data->distance[i]; 
			    }
    			}

		/* Deal with the sidescan */

		/* deal with a ping to port */
		if (data->Port == -1) 
			{
			for (i=0;i<MBF_HSMDARAW_PIXELS_PING;i++) 
				{
				j = MBF_HSMDARAW_PIXELS_PING - i - 1;
				mb_io_ptr->new_ss[j] = data->ss[i];
				mb_io_ptr->new_ss_acrosstrack[j] = 
					-data->ss_range * i 
					/ ((double)(MBF_HSMDARAW_PIXELS_PING - 1));
      				}
    			}

		/* deal with a ping to starboard */
		else 
			{
			for (i=0;i<MBF_HSMDARAW_PIXELS_PING;i++) 
				{
				j = i + MBF_HSMDARAW_PIXELS_PING - 1;
				mb_io_ptr->new_ss[j] = data->ss[i];
				mb_io_ptr->new_ss_acrosstrack[j] = 
					data->ss_range * i 
					/ ((double)(MBF_HSMDARAW_PIXELS_PING - 1));
      				}
    			}
    		}

	/* translate values to data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
    		{
		/* type of data record */
		store->kind = data->kind;

		/* header values */
		for (i=0;i<4;i++)
			{
			store->scsid[i] = data->scsid[i];
			store->scsart[i] = data->scsart[i];
			}
		store->scslng = data->scslng;
		store->scsext = data->scsext;
		store->scsblcnt = data->scsblcnt;
		store->scsres1 = data->scsres1;
		store->transid = data->transid;
		store->reftime = data->reftime;

		/* event data */
		store->datuhr = data->datuhr;
		for (i=0;i<8;i++)
			store->mksysint[i] = data->mksysint[i];
		for (i=0;i<84;i++)
			store->mktext[i] = data->mktext[i];

		/* navigation data */
		store->navid = data->navid;
		store->year = data->year;
		store->month = data->month;
		store->day = data->day;
		store->hour = data->hour;
		store->minute = data->minute;
		store->second = data->second;
		store->secf = data->secf;
		store->millisecond = data->millisecond;
		store->PingTime = data->PingTime;
		store->lon = data->lon;
		store->lat = data->lat;
		store->pos_sens[0] = data->pos_sens[0];
		store->pos_sens[1] = data->pos_sens[1];

		/* travel time, bathymetry and sidescan data */
		store->ckeel = data->ckeel;
		store->cmean = data->cmean;
		store->Port = data->Port;
		store->noho = data->noho;
		store->skals = data->skals;
		for (i=0;i<MBF_HSMDARAW_BEAMS_PING;i++)
			{
	  		store->spfb[i] = data->spfb[i];
	 		store->depth[i] = data->depth[i];
	  		store->distance[i] = data->distance[i];
	  		store->angle[i] = data->angle[i];
			}
		store->ss_range = data->ss_range;
		for (i=0;i<MBF_HSMDARAW_PIXELS_PING;i++)
			{
			store->ss[i] = data->ss[i];
			}
		store->heading_tx = data->heading_tx;
		store->roll_tx = data->roll_tx;
		store->pitch_tx = data->pitch_tx;
		for (i=0;i<5;i++)
			{
			store->heading_rx[i] = data->heading_rx[i];
			store->pitch_rx[i] = data->pitch_rx[i];
			store->roll_rx[i] = data->roll_rx[i];
			}

		/* MD event data */
		store->evid = data->evid;
		for (i=0;i<84;i++)
			store->evtext[i] = data->evtext[i];

		store->num_vel = data->num_vel;
		for (i=0;i<data->num_vel;i++)
			{
			store->vdepth[i] = data->vdepth[i];
			store->velocity[i] = data->velocity[i];
			}

		/* comment */
		strncpy(store->comment,data->comment,MBSYS_HSMD_COMMENT);
		store->heave = data->heave;
		store->speed = data->speed;
    		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_wt_hsmdaraw(verbose,mbio_ptr,store_ptr,error)
int    verbose;
char   *mbio_ptr;
char   *store_ptr;
int    *error;
{
	char	*function_name = "mbr_wt_hsmdaraw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_hsmdaraw_struct *data;
	char	*data_ptr;
	struct mbsys_hsmd_struct *store;
	int	first;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_hsmdaraw_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	store = (struct mbsys_hsmd_struct *) store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL)
    		{
		/* type of data record */
		data->kind = store->kind;

		/* header values */
		for (i=0;i<4;i++)
			{
			data->scsid[i] = store->scsid[i];
			data->scsart[i] = store->scsart[i];
			}
		data->scslng = store->scslng;
		data->scsext = store->scsext;
		data->scsblcnt = store->scsblcnt;
		data->scsres1 = store->scsres1;
		data->transid = store->transid;
		data->reftime = store->reftime;

		/* event data */
		data->datuhr = store->datuhr;
		for (i=0;i<8;i++)
			data->mksysint[i] = store->mksysint[i];
		for (i=0;i<84;i++)
			data->mktext[i] = store->mktext[i];

		/* navigation data */
		data->navid = store->navid;
		data->year = store->year;
		data->month = store->month;
		data->day = store->day;
		data->hour = store->hour;
		data->minute = store->minute;
		data->second = store->second;
		data->secf = store->secf;
		data->millisecond = store->millisecond;
		data->PingTime = store->PingTime;
		data->lon = store->lon;
		data->lat = store->lat;
		data->pos_sens[0] = store->pos_sens[0];
		data->pos_sens[1] = store->pos_sens[1];

		/* travel time, bathymetry and sidescan data */
		data->ckeel = store->ckeel;
		data->cmean = store->cmean;
		data->Port = store->Port;
		data->noho = store->noho;
		data->skals = store->skals;
		for (i=0;i<MBF_HSMDARAW_BEAMS_PING;i++)
			{
	  		data->spfb[i] = store->spfb[i];
	 		data->depth[i] = store->depth[i];
	  		data->distance[i] = store->distance[i];
	  		data->angle[i] = store->angle[i];
			}
		data->ss_range = store->ss_range;
		for (i=0;i<MBF_HSMDARAW_PIXELS_PING;i++)
			{
			data->ss[i] = store->ss[i];
			}
		data->heading_tx = store->heading_tx;
		data->roll_tx = store->roll_tx;
		data->pitch_tx = store->pitch_tx;
		for (i=0;i<5;i++)
			{
			data->heading_rx[i] = store->heading_rx[i];
			data->pitch_rx[i] = store->pitch_rx[i];
			data->roll_rx[i] = store->roll_rx[i];
			}

		/* MD event data */
		data->evid = store->evid;
		for (i=0;i<84;i++)
			data->evtext[i] = store->evtext[i];

		data->num_vel = store->num_vel;
		for (i=0;i<store->num_vel;i++)
			{
			data->vdepth[i] = store->vdepth[i];
			data->velocity[i] = store->velocity[i];
			}

		/* comment */
		strncpy(data->comment,store->comment,MBSYS_HSMD_COMMENT);
		data->heave = store->heave;
		data->speed = store->speed;
    		}

	/* set kind from current ping */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR)
		data->kind = mb_io_ptr->new_kind;

	/* check for comment to be copied from mb_io_ptr */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_COMMENT)
    		{
		strncpy(data->comment,mb_io_ptr->new_comment,
			MBSYS_HSMD_COMMENT);
		
		/* put in some reasonable header values */
		strncpy(data->scsid, "DXT", 4);
		strncpy(data->scsart, "REI", 4);
		data->scslng = 140;
		data->scsblcnt = 0;
		data->scsres1 = 0;
		data->transid = 7;
		data->reftime = 0;
    		}

	/* else check for ping data to be copied from mb_io_ptr */
	else if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		/* get time */
		data->year = mb_io_ptr->new_time_i[0];
		data->month = mb_io_ptr->new_time_i[1];
		data->day = mb_io_ptr->new_time_i[2];
		data->hour = mb_io_ptr->new_time_i[3];
		data->minute = mb_io_ptr->new_time_i[4];
		data->second = mb_io_ptr->new_time_i[5];
		data->millisecond = mb_io_ptr->new_time_i[6] / 1000;

		/* get navigation */
		data->lon = mb_io_ptr->new_lon;
		data->lat = mb_io_ptr->new_lat;

		/* get speed (convert km/hr to m/s) */
		data->speed = mb_io_ptr->new_speed;

		/* figure out if port or starboard ping */
		first = -1;
		for (i=0;i<mb_io_ptr->beams_bath;i++) 
			{
			if (first == -1 && mb_io_ptr->new_bath[i] != 0.0)
				first = i;
			}
		if (first >= MBF_HSMDARAW_BEAMS_PING - 1)
			data->Port = 1;
		else
			data->Port = -1;

		/* put distance and depth values 
			into hsmdaraw data structure */

		/* deal with a ping to port */
		if (data->Port == -1) 
			{
			for (i=0;i<MBF_HSMDARAW_BEAMS_PING;i++) 
				{
				j = MBF_HSMDARAW_BEAMS_PING - i - 1;
				if (mb_beam_check_flag(mb_io_ptr->new_beamflag[j]))
	  			    data->depth[i] = -mb_io_ptr->new_bath[j];
				else
	  			    data->depth[i] = mb_io_ptr->new_bath[j];
				data->distance[i] = mb_io_ptr->new_bath_acrosstrack[j];
      				}
    			}

		/* deal with a ping to starboard */
		else 
			{
			for (i=0;i<MBF_HSMDARAW_BEAMS_PING;i++) 
				{
				j = i + MBF_HSMDARAW_BEAMS_PING - 1;
				if (mb_beam_check_flag(mb_io_ptr->new_beamflag[j]))
	  			    data->depth[i] = -mb_io_ptr->new_bath[j];
				else
	  			    data->depth[i] = mb_io_ptr->new_bath[j];
				data->distance[i] = mb_io_ptr->new_bath_acrosstrack[j];
      				}
    			}

		/* put sidescan values into hsmdaraw data structure */

		/* deal with a ping to port */
		if (data->Port == -1) 
			{
			if (data->ss_range <= 0.0)
				data->ss_range = 
					fabs(mb_io_ptr->new_ss_acrosstrack[0]);
			for (i=0;i<MBF_HSMDARAW_PIXELS_PING;i++) 
				{
				j = MBF_HSMDARAW_PIXELS_PING - i - 1;
	  			data->ss[i] = mb_io_ptr->new_ss[j];
      				}
    			}

		/* deal with a ping to starboard */
		else 
			{
			if (data->ss_range <= 0.0)
				data->ss_range = 
					mb_io_ptr->new_ss_acrosstrack[MBF_HSMDARAW_PIXELS_PING - 1];
			for (i=0;i<MBF_HSMDARAW_PIXELS_PING;i++) 
				{
				j = i + MBF_HSMDARAW_PIXELS_PING - 1;
	  			data->ss[i] = mb_io_ptr->new_ss[j];
      				}
    			}
		}

	/* write next data to file */
	status = mbr_hsmdaraw_wr_data(verbose,mbio_ptr,data_ptr,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_hsmdaraw_rd_data(verbose,mbio_ptr,error)
int    verbose;
char   *mbio_ptr;
int    *error;
{
	char    *function_name = "mbr_hsmdaraw_rd_data";
	int     status = MB_SUCCESS;
	struct  mb_io_struct *mb_io_ptr;
	struct  mbf_hsmdaraw_struct *data;
	char    *data_ptr;
	FILE    *mbfp;
	char	*xdrs;		/* xdr i/o pointer */
	int     i;
	int     time_i[7];
	double	scale;
	double	PingTime;	/* Synthesised time of this ping 
				 		PingTime = Base_time 
				 		+ (current.datuhr 
						- FirstReftime) */

	double *FirstReftime;	/* time from the first header */
	int  *Header_count; /* number of header records encounterd */
	int  *Rev_count;	   /* Raw Event counter */
	int  *Nav_count;	   /* number of Nav records */
	int  *Angle_count;  /* etc....... */
	int  *Svp_count;
	int  *Raw_count;
	int  *MDevent_count;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_hsmdaraw_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;	/* The data structure pointer */
	mbfp = mb_io_ptr->mbfp;		/* The file pointer */
	xdrs = mb_io_ptr->xdrs;
	FirstReftime = &mb_io_ptr->saved1;	/* time from the first header */
	Header_count = &mb_io_ptr->save1; /* number of header records encounterd */
	Rev_count = &mb_io_ptr->save2;	   /* Raw Event counter */
	Nav_count = &mb_io_ptr->save3;	   /* number of Nav records */
	Angle_count = &mb_io_ptr->save4;  /* etc....... */
	Svp_count = &mb_io_ptr->save5;
	Raw_count = &mb_io_ptr->save6;
	MDevent_count = &mb_io_ptr->save7;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* Start reading an HSMD Header structure */
	/* read the first four bytes */
	for (i=0;i<4;i++)
		status = xdr_char(xdrs, &data->scsid[i]);
		
	/* loop until the beginning of a record is found */
	while (status == MB_SUCCESS
		&& strncmp(data->scsid, "DXT", 3) != 0)
		{
		if (data->scsid[1] == 'D'
		    || data->scsid[2] == 'D'
		    || data->scsid[3] == 'D')
		    {
		    for (i=0;i<3;i++)
			    data->scsid[i] = data->scsid[i+1];		    
		    status = xdr_char(xdrs, &data->scsid[3]);		
		    }
		else
		    {
		    while (status == MB_SUCCESS
			&& data->scsid[0] != 'D')
			{
			if ((status = fread(&data->scsid[0],
				1,1,mb_io_ptr->mbfp)) != 1)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}
			}
		    if (status == MB_SUCCESS)
			{
			for (i=1;i<4;i++)
			    status = xdr_char(xdrs, &data->scsid[i]);
			}	
		    }
		}
		
	/* now read the rest of the record */
	if (status == MB_SUCCESS)
		for (i=0;i<4;i++)
			status = xdr_char(xdrs, &data->scsart[i]);
	if (status == MB_SUCCESS)
		status = xdr_long(xdrs, &data->scslng);
	if (status == MB_SUCCESS)
		status = xdr_long(xdrs, &data->scsext);
	if (status == MB_SUCCESS)
		status = xdr_long(xdrs, &data->scsblcnt);
	if (status == MB_SUCCESS)
		status = xdr_double(xdrs, &data->scsres1);
	if (status == MB_SUCCESS)
		status = xdr_long(xdrs, &data->transid);

	/* get first time and initialize the time base */
	if (status == MB_SUCCESS)
		status = xdr_double(xdrs, &data->reftime);
	if (status == MB_SUCCESS
		&& data->transid != MBF_HSMDARAW_COM)
		{
		(*Header_count)++;
		if (*Header_count == 1)	
			*FirstReftime = data->reftime;
		}

	/* check status */
	if (status == MB_SUCCESS)
	      	*error = MB_ERROR_NO_ERROR;
	else
	      	*error = MB_ERROR_EOF;

	/* print out some debug messages */
	if (verbose >= 2 && status == MB_SUCCESS)
		{
		fprintf(stderr,"\ndbg2: ========================== \n");
		fprintf(stderr,"dbg2: HED (0) # %d\t%.3lf\t%.3lf \n", 
				*Header_count,data->reftime, data->reftime-*FirstReftime);
		}
	if (verbose >= 5 && status == MB_SUCCESS)
		{
		fprintf(stderr,"dbg5: data  From Header:\n");
		fprintf(stderr,"dbg5: \t->scsid : \t%s\n",data->scsid);
		fprintf(stderr,"dbg5: \t->scsart: \t%s\n",data->scsart);
		fprintf(stderr,"dbg5: \t->scslng: \t%ld\t0x%0X\n",
		  		data->scslng,data->scslng);
		fprintf(stderr,"dbg5: \t->scsext:  \t%ld\n",
				data->scsext);
		fprintf(stderr,"dbg5: \t->scsblcnt:\t%ld\n",
				data->scsblcnt);
		fprintf(stderr,"dbg5: \t->scsres1: \t%lf\n",
				data->scsres1);
		fprintf(stderr,"dgb5: \t->transid: \t%ld\n",
				data->transid);
		fprintf(stderr,"dgb5: \t->reftime: \t%lf\n",
				data->reftime);
		}

     	/* done reading the header part of this data record 
		- now read the rest*/

	/* read the appropriate data records */
	if (status == MB_SUCCESS ) 
		{ 
		switch(data->transid)
	  		{
	  		case (MBF_HSMDARAW_RAW):	/* 1, Raw data record */
	    			{
				data->kind = MB_DATA_DATA; 

	      			(*Raw_count)++;	/* the number of this kind of record */

				/* get water velocity and travel time data */
	      			if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->ckeel);
	      			if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->cmean);
	      			if (status == MB_SUCCESS)
					status = xdr_long(xdrs, &data->Port);
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs, &data->noho);
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs, &data->skals);
				if (status == MB_SUCCESS)
					for (i=0;i<MBF_HSMDARAW_BEAMS_PING;i++)
						status = xdr_long(xdrs, &data->spfb[i]);

				/* Check for bad beams - broken records produce
				    bogus data - it happens with HSMD systems!!! */
				if (data->skals)
					scale = 0.00015;
				else
					scale = 0.000015;
				if (status == MB_SUCCESS)
				for (i=0; i<MBF_HSMDARAW_BEAMS_PING; i++ )
		    			{
	 				if (data->spfb[i] < -65535
					    || data->spfb[i] > 65535)
					    {
					    data->spfb[i] = 0;
					    }
					}

				/* Calculate bathymetry.
					The travel times are scaled to 
					seconds, then adjusted for the mean sound
					speed, then do the simple geometry to
					calculate depth and cross-track. */
				if (data->skals)
					scale = 0.00015;
				else
					scale = 0.000015;
				if (status == MB_SUCCESS)
				for (i=0; i<MBF_HSMDARAW_BEAMS_PING; i++ )
		    			{
	 				data->depth[i] = 
						(fabs(scale * data->spfb[i]) 
						* 0.5 * data->cmean)
	    					* cos(data->angle[i] * DTR);
	 				data->distance[i] = 
	    					data->depth[i] 
						* tan(data->angle[i] * DTR );
	  				if (data->spfb[i] < 0)
	   					data->depth[i] = 
							-data->depth[i];
					if (data->Port == -1)
						data->distance[i] = 
							-data->distance[i];
					}

				/* get sidescan data */
				if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->ss_range);
				if (status == MB_SUCCESS)
					for (i=0;i<MBF_HSMDARAW_PIXELS_PING;i++)
						status = xdr_char(xdrs, &data->ss[i]);

				/* get attitude data */
				if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->heading_tx);
				if (status == MB_SUCCESS)
					for (i=0;i<5;i++)
						status = xdr_double(xdrs, &data->heading_rx[i]);
				if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->roll_tx);
				if (status == MB_SUCCESS)
					for (i=0;i<5;i++)
						status = xdr_double(xdrs, &data->roll_rx[i]);
				if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->pitch_tx); 
				if (status == MB_SUCCESS)
					for (i=0;i<5;i++)
						status = xdr_double(xdrs, &data->pitch_rx[i]);

				/* Establish the time of day for this
				 * ping. "Raw" (travel time) data records 
				 * do not contain time of day, only the
				 * internal Reference time. The interrupt 
				 * records contain a unix epoch time used 
				 * to convert to UTC. */
				if (status == MB_SUCCESS)
					{
					PingTime = data->datuhr + (data->reftime 
						- *FirstReftime);
					status = mb_get_date(verbose,PingTime,time_i);

					data->PingTime 		= PingTime;
					data->year 		= time_i[0];
					data->month 		= time_i[1];
					data->day 		= time_i[2];
					data->hour 		= time_i[3];
					data->minute 		= time_i[4];
					data->second 		= time_i[5];
					data->millisecond 	= time_i[6] / 1000;
					}

	      			/* output some debug messages */
				if (verbose >= 2 && status == MB_SUCCESS)
					{
					fprintf(stderr,"\ndgb2: Setting time of Ping in RAW\n:");
					fprintf(stderr,"dbg2: \t->year:   \t%4d\n",data->year);
					fprintf(stderr,"dbg2: \t->month:  \t%2d\n",data->month);
					fprintf(stderr,"dgb2: \t->day:    \t%2d\n",data->day);
					fprintf(stderr,"dgb2: \t->hour:   \t%2d\n",data->hour);
					fprintf(stderr,"dbg2: \t->minute: \t%2d\n",data->minute);
					fprintf(stderr,"dbg2: \t->second: \t%2d\n",data->second);
					fprintf(stderr,"dbg2: \t->millisecond: \t%3d\n",data->millisecond);
					fprintf(stderr,"\ndbg2: \t->Lat:   \t%.4lf\n",data->lat);
					fprintf(stderr,"\ndbg2: \t->Lon:   \t%.4lf\n",data->lon);
					}
				if (verbose >= 2 && status == MB_SUCCESS)
					fprintf(stderr,"\ndbg2: RAW (1) \t%3d\t%4d %2d %2d %2d:%2d:%2d.%3d\n", 
						data->Port, data->year, data->month, data->day, 
						data->hour, data->minute, data->second, data->millisecond);
				if (verbose >= 2 && status == MB_SUCCESS)
					{
					fprintf(stderr,"\ndgb2: Raw\n");
					fprintf(stderr,"dbg2: \tckeel\t%8.2lf\n", data->ckeel);
					fprintf(stderr,"dbg2: \tcmean\t%8.2lf\n", data->cmean);
					fprintf(stderr,"dgb2: \tPort\t%ld\n", data->Port);
					fprintf(stderr,"\tnoho\t%ld\n", data->noho);
					fprintf(stderr,"\tskals\t%ld\n", data->skals);
					fprintf(stderr,"\tspfbs\n");
					for (i=0;i<MBF_HSMDARAW_BEAMS_PING;i=i+4)
		    				{
						fprintf(stderr,"\t(%02d) %10d (%02d) %10d (%02d) %10d (%02d) %10d\n",
			      				i,data->spfb[i], 
							i+1,data->spfb[i+1], 
							i+2, data->spfb[i+2], 
							i+3, data->spfb[i+3]);
		   				}
					fprintf(stderr,"\tss_range\t%lf\n", data->ss_range);
					fprintf(stderr,"\tampl\n");	       
					for (i=0;i<MBF_HSMDARAW_PIXELS_PING;i=i+4)
		    				{
						fprintf(stderr,"\t%d\t%d\t%d\t%d\n",
							data->ss[i], 
							data->ss[i+1], 
							data->ss[i+2],
			      				data->ss[i+3]);
		    				}
		  
					fprintf(stderr,"\theading_tx\t%8.3lf\n", data->heading_tx);
					fprintf(stderr,"\theading_rx:\t");
					fprintf(stderr,"%8.3lf %8.3lf %8.3lf %8.3lf %8.3lf\n",
						data->heading_rx[0],
						data->heading_rx[1],
						data->heading_rx[2],
						data->heading_rx[3],
						data->heading_rx[4]);
		  
					fprintf(stderr,"\troll_tx\t%8.3lf\n", data->roll_tx);
					fprintf(stderr,"\troll_rx:\t");
					fprintf(stderr,"%8.3lf %8.3lf %8.3lf %8.3lf %8.3lf\n",
						data->roll_rx[0],
						data->roll_rx[1],
						data->roll_rx[2],
						data->roll_rx[3],
						data->roll_rx[4]);
		  
					fprintf(stderr,"\tpitch_tx\t%8.3lf\n", data->pitch_tx);
					fprintf(stderr,"\tpitch_rx:\t");
					fprintf(stderr,"%8.3lf %8.3lf %8.3lf %8.3lf %8.3lf\n",
						data->pitch_rx[0],
						data->pitch_rx[1],
						data->pitch_rx[2],
						data->pitch_rx[3],
						data->pitch_rx[4]);
					}

				/* check status */
				if (status == MB_SUCCESS)
					{
	      				*error = MB_ERROR_NO_ERROR;
					}
				else
	      				*error = MB_ERROR_EOF;
	     			break;
	    			}
	      
	  		case (MBF_HSMDARAW_BAT):	/* 8, LDEO bath data record */
	    			{
				data->kind = MB_DATA_DATA; 

	      			(*Raw_count)++;	/* the number of this kind of record */

				/* get time and position */
	      			if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->PingTime);
	      			if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->lon);
	      			if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->lat);

				/* get water velocity and travel time data */
	      			if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->ckeel);
	      			if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->cmean);
	      			if (status == MB_SUCCESS)
					status = xdr_long(xdrs, &data->Port);
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs, &data->noho);
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs, &data->skals);
				if (status == MB_SUCCESS)
					for (i=0;i<MBF_HSMDARAW_BEAMS_PING;i++)
						status = xdr_long(xdrs, &data->spfb[i]);
				if (status == MB_SUCCESS)
					for (i=0;i<MBF_HSMDARAW_BEAMS_PING;i++)
						status = xdr_double(xdrs, &data->depth[i]);
				if (status == MB_SUCCESS)
					for (i=0;i<MBF_HSMDARAW_BEAMS_PING;i++)
						status = xdr_double(xdrs, &data->distance[i]);

				/* get sidescan data */
				if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->ss_range);
				if (status == MB_SUCCESS)
					for (i=0;i<MBF_HSMDARAW_PIXELS_PING;i++)
						status = xdr_char(xdrs, &data->ss[i]);

				/* get attitude data */
				if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->heading_tx);
				if (status == MB_SUCCESS)
					for (i=0;i<5;i++)
						status = xdr_double(xdrs, &data->heading_rx[i]);
				if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->roll_tx);
				if (status == MB_SUCCESS)
					for (i=0;i<5;i++)
						status = xdr_double(xdrs, &data->roll_rx[i]);
				if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->pitch_tx); 
				if (status == MB_SUCCESS)
					for (i=0;i<5;i++)
						status = xdr_double(xdrs, &data->pitch_rx[i]);

				/* Establish the time of day for this
				 * ping. "Raw" (travel time) data records 
				 * do not contain time of day, only the
				 * internal Reference time. The interrupt 
				 * records contain a unix epoch time used 
				 * to convert to UTC. */
				if (status == MB_SUCCESS)
					{
					status = mb_get_date(verbose,
						    data->PingTime,time_i);

					data->year 		= time_i[0];
					data->month 		= time_i[1];
					data->day 		= time_i[2];
					data->hour 		= time_i[3];
					data->minute 		= time_i[4];
					data->second 		= time_i[5];
					data->millisecond 	= time_i[6] / 1000;
					}

	      			/* output some debug messages */
				if (verbose >= 2 && status == MB_SUCCESS)
					{
					fprintf(stderr,"\ndgb2: Setting time of Ping in RAW\n:");
					fprintf(stderr,"dbg2: \t->year:   \t%4d\n",data->year);
					fprintf(stderr,"dbg2: \t->month:  \t%2d\n",data->month);
					fprintf(stderr,"dgb2: \t->day:    \t%2d\n",data->day);
					fprintf(stderr,"dgb2: \t->hour:   \t%2d\n",data->hour);
					fprintf(stderr,"dbg2: \t->minute: \t%2d\n",data->minute);
					fprintf(stderr,"dbg2: \t->second: \t%2d\n",data->second);
					fprintf(stderr,"dbg2: \t->millisecond: \t%3d\n",data->millisecond);
					fprintf(stderr,"\ndbg2: \t->Lat:   \t%.4lf\n",data->lat);
					fprintf(stderr,"\ndbg2: \t->Lon:   \t%.4lf\n",data->lon);
					}
				if (verbose >= 2 && status == MB_SUCCESS)
					fprintf(stderr,"\ndbg2: RAW (1) \t%3d\t%4d %2d %2d %2d:%2d:%2d.%3d\n", 
						data->Port, data->year, data->month, data->day, 
						data->hour, data->minute, data->second, data->millisecond);
				if (verbose >= 2 && status == MB_SUCCESS)
					{
					fprintf(stderr,"\ndgb2: Raw\n");
					fprintf(stderr,"dbg2: \tckeel\t%8.2lf\n", data->ckeel);
					fprintf(stderr,"dbg2: \tcmean\t%8.2lf\n", data->cmean);
					fprintf(stderr,"dgb2: \tPort\t%ld\n", data->Port);
					fprintf(stderr,"\tnoho\t%ld\n", data->noho);
					fprintf(stderr,"\tskals\t%ld\n", data->skals);
					fprintf(stderr,"\tspfbs\n");
					for (i=0;i<MBF_HSMDARAW_BEAMS_PING;i=i+4)
		    				{
						fprintf(stderr,"\t(%02d) %10d (%02d) %10d (%02d) %10d (%02d) %10d\n",
			      				i,data->spfb[i], 
							i+1,data->spfb[i+1], 
							i+2, data->spfb[i+2], 
							i+3, data->spfb[i+3]);
		   				}
					fprintf(stderr,"\tss_range\t%lf\n", data->ss_range);
					fprintf(stderr,"\tampl\n");	       
					for (i=0;i<MBF_HSMDARAW_PIXELS_PING;i=i+4)
		    				{
						fprintf(stderr,"\t%d\t%d\t%d\t%d\n",
							data->ss[i], 
							data->ss[i+1], 
							data->ss[i+2],
			      				data->ss[i+3]);
		    				}
		  
					fprintf(stderr,"\theading_tx\t%8.3lf\n", data->heading_tx);
					fprintf(stderr,"\theading_rx:\t");
					fprintf(stderr,"%8.3lf %8.3lf %8.3lf %8.3lf %8.3lf\n",
						data->heading_rx[0],
						data->heading_rx[1],
						data->heading_rx[2],
						data->heading_rx[3],
						data->heading_rx[4]);
		  
					fprintf(stderr,"\troll_tx\t%8.3lf\n", data->roll_tx);
					fprintf(stderr,"\troll_rx:\t");
					fprintf(stderr,"%8.3lf %8.3lf %8.3lf %8.3lf %8.3lf\n",
						data->roll_rx[0],
						data->roll_rx[1],
						data->roll_rx[2],
						data->roll_rx[3],
						data->roll_rx[4]);
		  
					fprintf(stderr,"\tpitch_tx\t%8.3lf\n", data->pitch_tx);
					fprintf(stderr,"\tpitch_rx:\t");
					fprintf(stderr,"%8.3lf %8.3lf %8.3lf %8.3lf %8.3lf\n",
						data->pitch_rx[0],
						data->pitch_rx[1],
						data->pitch_rx[2],
						data->pitch_rx[3],
						data->pitch_rx[4]);
		  
					}

				/* check status */
				if (status == MB_SUCCESS)
					{
	      				*error = MB_ERROR_NO_ERROR;
					}
				else
	      				*error = MB_ERROR_EOF;
	     			break;
	    			}
	      
	  		case (MBF_HSMDARAW_NAV): /* 2, Navigation data record */
	    			{
				(*Nav_count)++;
				data->kind = MB_DATA_NAV;

				/* get nav data */
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs, &data->navid);
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs,&data->year);
				if (status == MB_SUCCESS)
					mb_fix_y2k(verbose, data->year, &data->year);
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs,&data->month);
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs,&data->day);
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs,&data->hour);
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs,&data->minute);
				if (status == MB_SUCCESS)
					status = xdr_double(xdrs,&data->secf);
				if (status == MB_SUCCESS)
					{
					/* break decimal seconds into integer
						seconds and msec */
					data->second = (int) data->secf;
					data->millisecond = (int) (1000 * (data->secf - data->second));
					}

				/* get position */
				if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->lat);
				if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->lon);
				if (status == MB_SUCCESS)
					status = xdr_char(xdrs, &data->pos_sens[0]);
				if (status == MB_SUCCESS)
					status = xdr_char(xdrs, &data->pos_sens[1]);

				/* Establish the time of day for this
				 * nav record. Nav data records 
				 * do contain time of day, but the values
				 * seem unreliable. Thus, we use the
				 * internal Reference time. The interrupt 
				 * records contain a unix epoch time used 
				 * to convert to UTC. */
				if (status == MB_SUCCESS)
					{
					PingTime = data->datuhr + (data->reftime 
						- *FirstReftime);
					status = mb_get_date(verbose,PingTime,time_i);

					data->PingTime 		= PingTime;
					data->year 		= time_i[0];
					data->month 		= time_i[1];
					data->day 		= time_i[2];
					data->hour 		= time_i[3];
					data->minute 		= time_i[4];
					data->second 		= time_i[5];
					data->millisecond 	= time_i[6] / 1000;
					}

	      			/* output some debug messages */
				if (verbose >= 2 && status == MB_SUCCESS)
					fprintf(stderr,"\ndbg2: NAV (2) # %3d\t%4d %2d %2d %2d:%2d:%2d.%3d\n", 
						*Nav_count, data->year, data->month, data->day, 
						data->hour, data->minute, data->second, data->millisecond);
				if (verbose >= 2 && status == MB_SUCCESS)
					{
					fprintf(stderr,"dbg2: \nNav\n");
					fprintf(stderr,"dbg2: \t->navid:  \t%ld\n",data->navid);
					fprintf(stderr,"dbg2: \t->year:   \t%4d\n",data->year);
					fprintf(stderr,"dbg2: \t->month:  \t%2d\n",data->month);
					fprintf(stderr,"dbg2: \t->day:    \t%2d\n",data->day);
					fprintf(stderr,"dbg2: \t->hour:   \t%2d\n",data->hour);
					fprintf(stderr,"dbg2: \t->minute: \t%2d\n",data->minute);
					fprintf(stderr,"dbg2: \t->second: \t%2d\n",data->second);
					fprintf(stderr,"dbg2: \t->millisec::\t%.3f\n",data->secf);
		  		    
					fprintf(stderr,"dbg2: \t->lat:    \t%lf\n",data->lat);
					fprintf(stderr,"dbg2: \t->lon:    \t%lf\n",data->lon);
					fprintf(stderr,"dbg2: \t->pos_sens:\t%s\n", data->pos_sens);
					}
				if ( verbose >= 2 && status == MB_SUCCESS)
					{
					fprintf(stderr,"dbg2: %4ld %2ld %3ld %2ld %2ld %2ld %d %10.5lf %10.5lf %2ld %2s \n",
					data->year,
					data->month,
					data->day,
					data->hour,
					data->minute,
					data->second,
					data->millisecond,
					data->lat,
					data->lon,
					data->navid,
					data->pos_sens);
					}

				/* check status */
				if (status == MB_SUCCESS)
	      				*error = MB_ERROR_NO_ERROR;
				else
	      				*error = MB_ERROR_EOF;
	     			break;
	    			}

			case (MBF_HSMDARAW_MDE):		/* 3, MD Event */
	    			{
				(*MDevent_count)++;
				data->kind = MB_DATA_EVENT; 
	      
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs, &data->evid);
	      
				if (status == MB_SUCCESS)
					for (i=0;i<84;i++)
						status = xdr_char(xdrs, &data->evtext[i]);

				/* Establish the time of day for this
				 * record. Event data records 
				 * do not contain time of day, only the
				 * internal Reference time. The interrupt 
				 * records contain a unix epoch time used 
				 * to convert to UTC. */
				if (status == MB_SUCCESS)
					{
					PingTime = data->datuhr + (data->reftime 
						- *FirstReftime);
					status = mb_get_date(verbose,
						    PingTime,time_i);

					data->PingTime 		= PingTime;
					data->year 		= time_i[0];
					data->month 		= time_i[1];
					data->day 		= time_i[2];
					data->hour 		= time_i[3];
					data->minute 		= time_i[4];
					data->second 		= time_i[5];
					data->millisecond 	= time_i[6] / 1000;
					}

				/* output some debug messages */
				if (verbose >= 2 && status == MB_SUCCESS)
					{
					fprintf(stderr,"MDE (3) # %d\n", *MDevent_count);
					}
	      			if (verbose >= 2 && status == MB_SUCCESS)
					{
					fprintf(stderr,"MDE Event->\n");
					fprintf(stderr,"\t->evid:\t%ld\n", data->evid);
					fprintf(stderr,"\t->evtxt:\t%s\n", data->evtext);
					}

				/* check status */
				if (status == MB_SUCCESS)
	      				*error = MB_ERROR_NO_ERROR;
				else
	      				*error = MB_ERROR_EOF;
	     			break;
	    			}

			case (MBF_HSMDARAW_ANG): /* Transid == 4, Beam Angles */
	    			{
				(*Angle_count)++;
				data->kind = MB_DATA_ANGLE;
	      
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs, &data->noho);
	      
				if (status == MB_SUCCESS && status == MB_SUCCESS)
					for (i=0;i<MBF_HSMDARAW_BEAMS_PING;i++)
						{
						status = xdr_double(xdrs, &data->angle[i]);
						mbf_hsmdaraw_beamangle[i] = data->angle[i];
						}

				/* Establish the time of day for this
				 * record. Angle data records 
				 * do not contain time of day, only the
				 * internal Reference time. The interrupt 
				 * records contain a unix epoch time used 
				 * to convert to UTC. */
				if (status == MB_SUCCESS)
					{
					PingTime = data->datuhr + (data->reftime 
						- *FirstReftime);
					status = mb_get_date(verbose,
						    PingTime,time_i);

					data->PingTime 		= PingTime;
					data->year 		= time_i[0];
					data->month 		= time_i[1];
					data->day 		= time_i[2];
					data->hour 		= time_i[3];
					data->minute 		= time_i[4];
					data->second 		= time_i[5];
					data->millisecond 	= time_i[6] / 1000;
					}

				/* output some debug messages */
				if (verbose >= 2 && status == MB_SUCCESS)
					{
					fprintf(stderr,"\ndbg2: ANG (4) # %d\n", *Angle_count);
					}
				if ( verbose >= 5 && status == MB_SUCCESS)
					{
					fprintf(stderr,"\ndgb5: Ang");
					fprintf(stderr,"dbg5:\tnoho:\t%ld\n", data->noho);
		  			for (i=0;i<MBF_HSMDARAW_BEAMS_PING;i=i+4)
		   				{
		      				fprintf(stderr,"\t%02d: %8.3lf\t%02d: %8.3lf\t%02d: %8.3lf\t%02d: %8.3lf\n",
						i,data->angle[i], 
						i+1,data->angle[i+1], 
						i+2, data->angle[i+2], 
						i+3, data->angle[i+3]);
		    				}
					}

				/* check status */
				if (status == MB_SUCCESS)
	      				*error = MB_ERROR_NO_ERROR;
				else
	      				*error = MB_ERROR_EOF;
	     			break;
	    			}
	    
			case (MBF_HSMDARAW_SVP): /* 5, Sound Velocity Profile */
	    			{
				(*Svp_count)++;
				data->kind = MB_DATA_VELOCITY_PROFILE;
				
				data->num_vel = 20;
				for (i=0;i<data->num_vel;i++)
					{
					status = xdr_double(xdrs, &data->vdepth[i]);
					status = xdr_double(xdrs, &data->velocity[i]);
					}

				/* Establish the time of day for this
				 * record. SVP data records 
				 * do not contain time of day, only the
				 * internal Reference time. The interrupt 
				 * records contain a unix epoch time used 
				 * to convert to UTC. */
				if (status == MB_SUCCESS)
					{
					PingTime = data->datuhr + (data->reftime 
						- *FirstReftime);
					status = mb_get_date(verbose,
						    PingTime,time_i);

					data->PingTime 		= PingTime;
					data->year 		= time_i[0];
					data->month 		= time_i[1];
					data->day 		= time_i[2];
					data->hour 		= time_i[3];
					data->minute 		= time_i[4];
					data->second 		= time_i[5];
					data->millisecond 	= time_i[6] / 1000;
					}

				/* output some debug messages */
				if (verbose >= 2 && status == MB_SUCCESS)
					{
					fprintf(stderr,"\ndbg2: SVP (5) # %d\n",*Svp_count);
					}

				/* check status */
				if (status == MB_SUCCESS)
	      				*error = MB_ERROR_NO_ERROR;
				else
	      				*error = MB_ERROR_EOF;
	     			break;
	    			}
	    
	    
			case (MBF_HSMDARAW_REV):	/* 6, An Interrupt event? */
	    			{
				(*Rev_count)++;
	      
				if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->datuhr);
				if (status == MB_SUCCESS)
					for (i=0;i<8;i++)
						status = xdr_char(xdrs, &data->mksysint[i]);
				if (status == MB_SUCCESS)
					for (i=0;i<84;i++)
						status = xdr_char(xdrs, &data->mktext[i]);

				/* Establish the time of day for this
				 * record. Interrupt data records 
				 * contain a unix time which is used
				 * to get time of day. */
				if (status == MB_SUCCESS)
					{
					PingTime = data->datuhr + (data->reftime 
						- *FirstReftime);
					status = mb_get_date(verbose,
						    PingTime,time_i);

					data->PingTime 		= PingTime;
					data->year 		= time_i[0];
					data->month 		= time_i[1];
					data->day 		= time_i[2];
					data->hour 		= time_i[3];
					data->minute 		= time_i[4];
					data->second 		= time_i[5];
					data->millisecond 	= time_i[6] / 1000;
					}

				/* output some debug messages */
	      			if (verbose >= 2 && status == MB_SUCCESS)
					{
					fprintf(stderr,"dbg2:\n REV (6) # %d\t%.3lf",
						*Rev_count, data->datuhr );
					}
				if (verbose >= 5 && status == MB_SUCCESS)
					{
					fprintf(stderr,"\nIntevent");
					fprintf(stderr,"->datuhr:  \t%lf\n",data->datuhr);
					fprintf(stderr,"\t->mksysint:\t%s\n",data->mksysint);
					fprintf(stderr,"\t->mktext:  \t%s\n",data->mktext);
					}
	      
				/* Check to see if this Raw Event is
				 * indicating the start or end of the file. */
				if (status == MB_SUCCESS 
					&& strncmp(data->mksysint, "STOP",4) == 0)
					{
					data->kind = MB_DATA_STOP;
		  			*error = MB_ERROR_NO_ERROR;
					}
				else if (status == MB_SUCCESS)
					{
					data->kind = MB_DATA_START;
		  			*error = MB_ERROR_NO_ERROR;
					}
				else
					{
					*error = MB_ERROR_EOF;
					}
				break;
	    			}

			case (MBF_HSMDARAW_COM):	/* 7, Comment */
	    			{
				data->kind = MB_DATA_COMMENT; 

				if (status == MB_SUCCESS)
					for (i=0;i<MBF_HSMDARAW_COMMENT;i++)
						status = xdr_char(xdrs, &data->comment[i]);
		
				/* Check status. */
				if (status == MB_SUCCESS)
		  			*error = MB_ERROR_NO_ERROR;
				else
					*error = MB_ERROR_EOF;
				break;
	    			}

	  		default:
	    			{			
				/* Should never get here, so fail! */
				status = MB_FAILURE;
	      			*error = MB_ERROR_UNINTELLIGIBLE;
	      
				if (verbose >=2)
					{
					fprintf(stderr,"dbg2: data->transid=%d not parsed\n",
						data->transid);
					}
				break;
	    			}
	  		} 
      		}
	
	/* get file position */
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);

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

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
/*
 * This function actually does the writing of raw data 
 */

int mbr_hsmdaraw_wr_data(verbose,mbio_ptr,data_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*data_ptr;
int	*error;
{
	char	  *function_name = "mbr_hsmdaraw_wr_data";
	int	  status = MB_SUCCESS;
	struct  mb_io_struct *mb_io_ptr;
	struct  mbf_hsmdaraw_struct *data;
	FILE	  *mbfp;
	char	*xdrs;		/* xdr i/o pointer */
	int     i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_hsmdaraw_struct *) data_ptr;
	mbfp = mb_io_ptr->mbfp;
	xdrs = mb_io_ptr->xdrs;

	/* make sure transid is correct */
	if (data->transid == MBF_HSMDARAW_BAT)
		data->transid = MBF_HSMDARAW_RAW;

	/* Start writing an HSMD Header structure */
	for (i=0;i<4;i++)
		status = xdr_char(xdrs, &data->scsid[i]);
	if (status == MB_SUCCESS)
		for (i=0;i<4;i++)
			status = xdr_char(xdrs, &data->scsart[i]);
	if (status == MB_SUCCESS)
		status = xdr_long(xdrs, &data->scslng);
	if (status == MB_SUCCESS)
		status = xdr_long(xdrs, &data->scsext);
	if (status == MB_SUCCESS)
		status = xdr_long(xdrs, &data->scsblcnt);
	if (status == MB_SUCCESS)
		status = xdr_double(xdrs, &data->scsres1);
	if (status == MB_SUCCESS)
		status = xdr_long(xdrs, &data->transid);
	if (status == MB_SUCCESS)
		status = xdr_double(xdrs, &data->reftime);

	/* write the appropriate data record */
	if (status == MB_SUCCESS ) 
		{ 
		switch(data->transid)
	  		{
	  		case (MBF_HSMDARAW_RAW):	/* 1, Raw data record */
	    			{
				/* make sure transid is correct */
				data->transid = MBF_HSMDARAW_RAW;
	      
				/* First make sure bathymetry edits are 
					carried over into travel times */
				for (i=0; i<MBF_HSMDARAW_BEAMS_PING; i++ )
		    			{
					if (data->depth[i] < 0.0 &&
						data->spfb[i] > 0.0)
						data->spfb[i] = -data->spfb[i];
					else if (data->depth[i] > 0.0 &&
						data->spfb[i] < 0.0)
						data->spfb[i] = -data->spfb[i];
		    			}

	      			/* output some debug messages */
				if (verbose >= 2 && status == MB_SUCCESS)
					{
					fprintf(stderr,"\ndgb2: Setting time of Ping in RAW\n:");
					fprintf(stderr,"dbg2: \t->year:   \t%4d\n",data->year);
					fprintf(stderr,"dbg2: \t->month:  \t%2d\n",data->month);
					fprintf(stderr,"dgb2: \t->day:    \t%2d\n",data->day);
					fprintf(stderr,"dgb2: \t->hour:   \t%2d\n",data->hour);
					fprintf(stderr,"dbg2: \t->minute: \t%2d\n",data->minute);
					fprintf(stderr,"dbg2: \t->second: \t%2d\n",data->second);
					fprintf(stderr,"dbg2: \t->millisecond: \t%3d\n",data->millisecond);
					fprintf(stderr,"\ndbg2: \t->Lat:   \t%.4lf\n",data->lat);
					fprintf(stderr,"\ndbg2: \t->Lon:   \t%.4lf\n",data->lon);
					}
				if (verbose >= 2 && status == MB_SUCCESS)
					fprintf(stderr,"\ndbg2: RAW (1) \t%3d\t%4d %2d %2d %2d:%2d:%2d.%3d\n", 
						data->Port, data->year, data->month, data->day, 
						data->hour, data->minute, data->second, data->millisecond);
				if (verbose >= 2 && status == MB_SUCCESS)
					{
					fprintf(stderr,"\ndgb2: Raw\n");
					fprintf(stderr,"dbg2: \tckeel\t%8.2lf\n", data->ckeel);
					fprintf(stderr,"dbg2: \tcmean\t%8.2lf\n", data->cmean);
					fprintf(stderr,"dgb2: \tPort\t%ld\n", data->Port);
					fprintf(stderr,"\tnoho\t%ld\n", data->noho);
					fprintf(stderr,"\tskals\t%ld\n", data->skals);
					fprintf(stderr,"\tspfbs\n");
					for (i=0;i<MBF_HSMDARAW_BEAMS_PING;i=i+4)
		    				{
						fprintf(stderr,"\t(%02d) %10d (%02d) %10d (%02d) %10d (%02d) %10d\n",
			      				i,data->spfb[i], 
							i+1,data->spfb[i+1], 
							i+2, data->spfb[i+2], 
							i+3, data->spfb[i+3]);
		   				}
					fprintf(stderr,"\tss_range\t%lf\n", data->ss_range);
					fprintf(stderr,"\tampl\n");	       
					for (i=0;i<MBF_HSMDARAW_PIXELS_PING;i=i+4)
		    				{
						fprintf(stderr,"\t%d\t%d\t%d\t%d\n",
							data->ss[i], 
							data->ss[i+1], 
							data->ss[i+2],
			      				data->ss[i+3]);
		    				}
		  
					fprintf(stderr,"\theading_tx\t%8.3lf\n", data->heading_tx);
					fprintf(stderr,"\theading_rx:\t");
					fprintf(stderr,"%8.3lf %8.3lf %8.3lf %8.3lf %8.3lf\n",
						data->heading_rx[0],
						data->heading_rx[1],
						data->heading_rx[2],
						data->heading_rx[3],
						data->heading_rx[4]);
		  
					fprintf(stderr,"\troll_tx\t%8.3lf\n", data->roll_tx);
					fprintf(stderr,"\troll_rx:\t");
					fprintf(stderr,"%8.3lf %8.3lf %8.3lf %8.3lf %8.3lf\n",
						data->roll_rx[0],
						data->roll_rx[1],
						data->roll_rx[2],
						data->roll_rx[3],
						data->roll_rx[4]);
		  
					fprintf(stderr,"\tpitch_tx\t%8.3lf\n", data->pitch_tx);
					fprintf(stderr,"\tpitch_rx:\t");
					fprintf(stderr,"%8.3lf %8.3lf %8.3lf %8.3lf %8.3lf\n",
						data->pitch_rx[0],
						data->pitch_rx[1],
						data->pitch_rx[2],
						data->pitch_rx[3],
						data->pitch_rx[4]);
		  
					}

				/* set water velocity and travel time data */
	      			if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->ckeel);
	      			if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->cmean);
	      			if (status == MB_SUCCESS)
					status = xdr_long(xdrs, &data->Port);
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs, &data->noho);
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs, &data->skals);
				if (status == MB_SUCCESS)
					for (i=0;i<MBF_HSMDARAW_BEAMS_PING;i++)
						status = xdr_long(xdrs, &data->spfb[i]);

				/* set sidescan data */
				if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->ss_range);
				if (status == MB_SUCCESS)
					for (i=0;i<MBF_HSMDARAW_PIXELS_PING;i++)
						status = xdr_char(xdrs, &data->ss[i]);

				/* set attitude data */
				if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->heading_tx);
				if (status == MB_SUCCESS)
					for (i=0;i<5;i++)
						status = xdr_double(xdrs, &data->heading_rx[i]);
				if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->roll_tx);
				if (status == MB_SUCCESS)
					for (i=0;i<5;i++)
						status = xdr_double(xdrs, &data->roll_rx[i]);
				if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->pitch_tx); 
				if (status == MB_SUCCESS)
					for (i=0;i<5;i++)
						status = xdr_double(xdrs, &data->pitch_rx[i]);

				/* check status */
				if (status == MB_SUCCESS)
	      				*error = MB_ERROR_NO_ERROR;
				else
	      				*error = MB_ERROR_WRITE_FAIL;
	     			break;
	    			}
	      
	  		case (MBF_HSMDARAW_NAV): /* 2, Navigation data record */
	    			{
				/* set nav data */
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs, &data->navid);
				if (status == MB_SUCCESS)
					mb_unfix_y2k(verbose, data->year, &data->year);
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs,&data->year);
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs,&data->month);
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs,&data->day);
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs,&data->hour);
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs,&data->minute);
				if (status == MB_SUCCESS)
					status = xdr_double(xdrs,&data->secf);

				/* set position */
				if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->lat);
				if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->lon);
				if (status == MB_SUCCESS)
					status = xdr_char(xdrs, &data->pos_sens[0]);
				if (status == MB_SUCCESS)
					status = xdr_char(xdrs, &data->pos_sens[1]);

				/* check status */
				if (status == MB_SUCCESS)
	      				*error = MB_ERROR_NO_ERROR;
				else
	      				*error = MB_ERROR_WRITE_FAIL;
	     			break;
	    			}

			case (MBF_HSMDARAW_MDE):		/* 3, MD Event */
	    			{
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs, &data->evid);
				if (status == MB_SUCCESS)
					for (i=0;i<84;i++)
						status = xdr_char(xdrs, &data->evtext[i]);

				/* check status */
				if (status == MB_SUCCESS)
	      				*error = MB_ERROR_NO_ERROR;
				else
	      				*error = MB_ERROR_WRITE_FAIL;
	     			break;
	    			}

			case (MBF_HSMDARAW_ANG): /* Transid == 4, Beam Angles */
	    			{
				if (status == MB_SUCCESS)
					status = xdr_long(xdrs, &data->noho);
				if (status == MB_SUCCESS && status == MB_SUCCESS)
					for (i=0;i<MBF_HSMDARAW_BEAMS_PING;i++)
						status = xdr_double(xdrs, &data->angle[i]);

				/* check status */
				if (status == MB_SUCCESS)
	      				*error = MB_ERROR_NO_ERROR;
				else
	      				*error = MB_ERROR_WRITE_FAIL;
	     			break;
	    			}
	    
			case (MBF_HSMDARAW_SVP): /* 5, Sound Velocity Profile */
	    			{
				
				data->num_vel = 20;
				for (i=0;i<data->num_vel;i++)
					{
					status = xdr_double(xdrs, &data->vdepth[i]);
					status = xdr_double(xdrs, &data->velocity[i]);
					}

				/* check status */
				if (status == MB_SUCCESS)
	      				*error = MB_ERROR_NO_ERROR;
				else
	      				*error = MB_ERROR_WRITE_FAIL;
	     			break;
	    			}

			case (MBF_HSMDARAW_REV):	/* 6, An Interrupt event? */
	    			{
				if (status == MB_SUCCESS)
					status = xdr_double(xdrs, &data->datuhr);
				if (status == MB_SUCCESS)
					for (i=0;i<8;i++)
						status = xdr_char(xdrs, &data->mksysint[i]);
				if (status == MB_SUCCESS)
					for (i=0;i<84;i++)
						status = xdr_char(xdrs, &data->mktext[i]);
	      
				/* Check status. */
				if (status == MB_SUCCESS)
		  			*error = MB_ERROR_NO_ERROR;
				else
					*error = MB_ERROR_WRITE_FAIL;
				break;
	    			}

			case (MBF_HSMDARAW_COM):	/* 7, Comment */
	    			{
				if (status == MB_SUCCESS)
					for (i=0;i<MBF_HSMDARAW_COMMENT;i++)
						status = xdr_char(xdrs, &data->comment[i]);
		
				/* Check status. */
				if (status == MB_SUCCESS)
		  			*error = MB_ERROR_NO_ERROR;
				else
					*error = MB_ERROR_WRITE_FAIL;
				break;
	    			}

	  		default:
	    			{			
				/* Should never get here, so fail! */
				status = MB_FAILURE;
	      			*error = MB_ERROR_UNINTELLIGIBLE;
				break;
	    			}
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
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/



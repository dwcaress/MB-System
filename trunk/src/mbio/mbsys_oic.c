/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_oic.c	3/1/99
 *	$Id: mbsys_oic.c,v 5.5 2002-09-18 23:32:59 caress Exp $
 *
 *    Copyright (c) 1999, 2000, 2002 by
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
 * mbsys_oic.c contains the functions for handling the data structure
 * used by MBIO functions to store swath sonar data derived 
 * from OIC systems:
 *      MBF_OICGEODA : MBIO ID 141
 *      MBF_OICMBARI : MBIO ID 142
 *
 * Author:	D. W. Caress
 * Date:	March 1, 1999
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.4  2001/08/25 00:54:13  caress
 * Adding beamwidth values to extract functions.
 *
 * Revision 5.3  2001/07/20  00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.2  2001/03/22  20:50:02  caress
 * Trying to make version 5.0.beta0
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.2  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.1  2000/09/30  06:32:52  caress
 * Snapshot for Dale.
 *
 * Revision 4.0  1999/03/31  18:29:20  caress
 * MB-System 4.6beta7
 *
 * Revision 1.1  1999/03/31  18:11:35  caress
 * Initial revision
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
#include "../../include/mbsys_oic.h"

/*--------------------------------------------------------------------*/
int mbsys_oic_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
 static char res_id[]="$Id: mbsys_oic.c,v 5.5 2002-09-18 23:32:59 caress Exp $";
	char	*function_name = "mbsys_oic_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_oic_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* allocate memory for data structure */
	status = mb_malloc(verbose,sizeof(struct mbsys_oic_struct),
				store_ptr,error);

	/* get pointer to data structure */
	store = (struct mbsys_oic_struct *) *store_ptr;

	/* initialize values in structure */
	store->kind = MB_DATA_NONE;
	store->type = 0;
	store->proc_status = 0;
	store->data_size = 0;
	store->client_size = 0;
	store->fish_status = 0;
	store->nav_used = 0;
	store->nav_type = 0;
	store->utm_zone = 0;
	store->ship_x = 0.0;
	store->ship_y = 0.0;
	store->ship_course = 0.0;
	store->ship_speed = 0.0;
	store->sec = 0;
	store->usec = 0;
	store->spare_gain = 0.0;
	store->fish_heading = 0.0;
	store->fish_depth = 0.0;
	store->fish_range = 0.0;
	store->fish_pulse_width = 0.0;
	store->gain_c0 = 0.0;
	store->gain_c1 = 0.0;
	store->gain_c2 = 0.0;
	store->fish_pitch = 0.0;
	store->fish_roll = 0.0;
	store->fish_yaw = 0.0;
	store->fish_x = 0.0;
	store->fish_y = 0.0;
	store->fish_layback = 0.0;
	store->fish_altitude = 0.0;
	store->fish_altitude_samples = 0;
	store->fish_ping_period = 0.0;
	store->sound_velocity = 0.0;
	store->num_chan = 0;
	store->beams_bath = 0;
	store->beams_amp = 0;
	store->bath_chan_port = 0;
	store->bath_chan_stbd = 0;
	store->pixels_ss = 0;
	store->ss_chan_port = 0;
	store->ss_chan_stbd = 0;
	store->beamflag = NULL;
	store->bath = NULL;
	store->bathacrosstrack = NULL;
	store->bathalongtrack = NULL;
	store->tt = NULL;
	store->angle = NULL;
	store->amp = NULL;
	store->ss = NULL;
	store->ssacrosstrack = NULL;
	store->ssalongtrack = NULL;
	for (i=0;i<MBSYS_OIC_MAX_CHANNELS;i++)
	    {
	    store->channel[i].offset = 0;
	    store->channel[i].type = 0;
	    store->channel[i].side = 0;
	    store->channel[i].size = 0;
	    store->channel[i].empty = 0;
	    store->channel[i].frequency = 0;
	    store->channel[i].num_samples = 0;
	    }
	memset(store->client, 0, MBSYS_OIC_MAX_CLIENT);
	for (i=0;i<MBSYS_OIC_MAX_CHANNELS;i++)
	    {
	    store->rawsize[i] = 0;
	    store->raw[i] = NULL;
	    }
	store->beams_bath_alloc = 0;
	store->beams_amp_alloc = 0;
	store->pixels_ss_alloc = 0;
	store->beamflag = NULL;
	store->bath = NULL;
	store->amp = NULL;
	store->bathacrosstrack = NULL;
	store->bathalongtrack = NULL;
	store->tt = NULL;
	store->angle = NULL;
	store->ss = NULL;
	store->ssacrosstrack = NULL;
	store->ssalongtrack = NULL;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       store_ptr:  %d\n",*store_ptr);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_oic_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
	char	*function_name = "mbsys_oic_deall";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_oic_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",*store_ptr);
		}

	/* get pointer to data structure */
	store = (struct mbsys_oic_struct *) *store_ptr;

	/* deallocate memory for data structures */
	for (i=0;i<store->num_chan;i++)
	    status = mb_free(verbose,&(store->raw[i]),error);
	status = mb_free(verbose,&store->beamflag,error);
	status = mb_free(verbose,&store->bath,error);
	status = mb_free(verbose,&store->bathacrosstrack,error);
	status = mb_free(verbose,&store->bathalongtrack,error);
	status = mb_free(verbose,&store->tt,error);
	status = mb_free(verbose,&store->angle,error);
	status = mb_free(verbose,&store->amp,error);
	status = mb_free(verbose,&store->ss,error);
	status = mb_free(verbose,&store->ssacrosstrack,error);
	status = mb_free(verbose,&store->ssalongtrack,error);

	/* deallocate memory for data structure */
	status = mb_free(verbose,store_ptr,error);

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
int mbsys_oic_extract(int verbose, void *mbio_ptr, void *store_ptr, 
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_oic_extract";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_oic_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
		    function_name);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
	    fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
	    }

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_oic_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
	    {
	    /* get time */
	    *time_d = store->sec + 0.000001 * store->usec;
	    mb_get_date(verbose,*time_d,time_i);

	    /* get navigation */
	    if (store->nav_type == OIC_NAV_LONLAT)
		{
		*navlon = store->fish_x;
		*navlat = store->fish_y;
		}
	    else
		{
		*navlon = 0.0;
		*navlat = 0.0;
		}

	    /* get heading */
	    *heading = store->fish_heading;

	    /* get speed */
	    *speed = 3.6 * store->ship_speed;
			
	    /* set beamwidths in mb_io structure */
	    mb_io_ptr->beamwidth_ltrack = 2.0;
	    mb_io_ptr->beamwidth_xtrack = 0.2;

	    /* read distance, depth, and backscatter 
		    values into storage arrays */
	    *nbath = store->beams_bath;
	    *namp = store->beams_amp;
	    *nss = store->pixels_ss;
	    for (i=0;i<*nbath;i++)
		{
		beamflag[i] = store->beamflag[i];
		bath[i] = store->bath[i];
		bathacrosstrack[i] = store->bathacrosstrack[i];
		bathalongtrack[i] = store->bathalongtrack[i];
		}
	    for (i=0;i<*namp;i++)
		{
		amp[i] = store->amp[i];
		}
	    for (i=0;i<*nss;i++)
		{
		ss[i] = store->ss[i];
		ssacrosstrack[i] = store->ssacrosstrack[i];
		ssalongtrack[i] = store->ssalongtrack[i];
		}

	    /* print debug statements */
	    if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg4  Data extracted by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4  Extracted values:\n");
		fprintf(stderr,"dbg4       kind:       %d\n",
			*kind);
		fprintf(stderr,"dbg4       error:      %d\n",
			*error);
		fprintf(stderr,"dbg4       time_i[0]:  %d\n",
			time_i[0]);
		fprintf(stderr,"dbg4       time_i[1]:  %d\n",
			time_i[1]);
		fprintf(stderr,"dbg4       time_i[2]:  %d\n",
			time_i[2]);
		fprintf(stderr,"dbg4       time_i[3]:  %d\n",
			time_i[3]);
		fprintf(stderr,"dbg4       time_i[4]:  %d\n",
			time_i[4]);
		fprintf(stderr,"dbg4       time_i[5]:  %d\n",
			time_i[5]);
		fprintf(stderr,"dbg4       time_i[6]:  %d\n",
			time_i[6]);
		fprintf(stderr,"dbg4       time_d:     %f\n",
			*time_d);
		fprintf(stderr,"dbg4       longitude:  %f\n",
			*navlon);
		fprintf(stderr,"dbg4       latitude:   %f\n",
			*navlat);
		fprintf(stderr,"dbg4       speed:      %f\n",
			*speed);
		fprintf(stderr,"dbg4       heading:    %f\n",
			*heading);
		fprintf(stderr,"dbg4       nbath:      %d\n",
			*nbath);
		for (i=0;i<*nbath;i++)
		  fprintf(stderr,"dbg4       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
			i,beamflag[i],bath[i],bathacrosstrack[i],bathalongtrack[i]);
		fprintf(stderr,"dbg4        namp:     %d\n",
			*namp);
		for (i=0;i<*namp;i++)
		  fprintf(stderr,"dbg4        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n",
			i,amp[i],bathacrosstrack[i],bathalongtrack[i]);
		fprintf(stderr,"dbg4        nss:      %d\n",
			*nss);
		for (i=0;i<*nss;i++)
		  fprintf(stderr,"dbg4        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
			i,ss[i],ssacrosstrack[i],ssalongtrack[i]);
		}

	    /* done translating values */

	    }

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT)
	    {
	    /* copy comment */
	    strncpy(comment,store->client,MBSYS_OIC_MAX_COMMENT);

	    /* print debug statements */
	    if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  New ping read by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4  New ping values:\n");
		fprintf(stderr,"dbg4       error:      %d\n",
			error);
		fprintf(stderr,"dbg4       comment:    %s\n",
			comment);
		}
	    }

	/* print output debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
		    function_name);
	    fprintf(stderr,"dbg2  Return values:\n");
	    fprintf(stderr,"dbg2       kind:       %d\n",*kind);
	    }
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR 
	    && *kind == MB_DATA_COMMENT)
	    {
	    fprintf(stderr,"dbg2       comment:     \ndbg2       %s\n",
		    comment);
	    }
	else if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR 
	    && *kind != MB_DATA_COMMENT)
	    {
	    fprintf(stderr,"dbg2       time_i[0]:     %d\n",time_i[0]);
	    fprintf(stderr,"dbg2       time_i[1]:     %d\n",time_i[1]);
	    fprintf(stderr,"dbg2       time_i[2]:     %d\n",time_i[2]);
	    fprintf(stderr,"dbg2       time_i[3]:     %d\n",time_i[3]);
	    fprintf(stderr,"dbg2       time_i[4]:     %d\n",time_i[4]);
	    fprintf(stderr,"dbg2       time_i[5]:     %d\n",time_i[5]);
	    fprintf(stderr,"dbg2       time_i[6]:     %d\n",time_i[6]);
	    fprintf(stderr,"dbg2       time_d:        %f\n",*time_d);
	    fprintf(stderr,"dbg2       longitude:     %f\n",*navlon);
	    fprintf(stderr,"dbg2       latitude:      %f\n",*navlat);
	    fprintf(stderr,"dbg2       speed:         %f\n",*speed);
	    fprintf(stderr,"dbg2       heading:       %f\n",*heading);
	    }
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR 
	    && *kind == MB_DATA_DATA)
	    {
	    fprintf(stderr,"dbg2       nbath:      %d\n",
		    *nbath);
	    for (i=0;i<*nbath;i++)
	      fprintf(stderr,"dbg2       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
		    i,beamflag[i],bath[i],
		    bathacrosstrack[i],bathalongtrack[i]);
	    fprintf(stderr,"dbg2        namp:     %d\n",
		    *namp);
	    for (i=0;i<*namp;i++)
	      fprintf(stderr,"dbg2       beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n",
		    i,amp[i],bathacrosstrack[i],bathalongtrack[i]);
	    fprintf(stderr,"dbg2        nss:      %d\n",
		    *nss);
	    for (i=0;i<*nss;i++)
	      fprintf(stderr,"dbg2        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
		    i,ss[i],ssacrosstrack[i],ssalongtrack[i]);
	    }
	if (verbose >= 2)
	    {
	    fprintf(stderr,"dbg2       error:      %d\n",*error);
	    fprintf(stderr,"dbg2  Return status:\n");
	    fprintf(stderr,"dbg2       status:     %d\n",status);
	    }

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_oic_insert(int verbose, void *mbio_ptr, void *store_ptr, 
		int kind, int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_oic_insert";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_oic_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
		    function_name);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
	    fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
	    fprintf(stderr,"dbg2       kind:       %d\n",kind);
	    }
	if (verbose >= 2 && (kind == MB_DATA_DATA || kind == MB_DATA_NAV))
	    {
	    fprintf(stderr,"dbg2       time_i[0]:  %d\n",time_i[0]);
	    fprintf(stderr,"dbg2       time_i[1]:  %d\n",time_i[1]);
	    fprintf(stderr,"dbg2       time_i[2]:  %d\n",time_i[2]);
	    fprintf(stderr,"dbg2       time_i[3]:  %d\n",time_i[3]);
	    fprintf(stderr,"dbg2       time_i[4]:  %d\n",time_i[4]);
	    fprintf(stderr,"dbg2       time_i[5]:  %d\n",time_i[5]);
	    fprintf(stderr,"dbg2       time_i[6]:  %d\n",time_i[6]);
	    fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
	    fprintf(stderr,"dbg2       navlon:     %f\n",navlon);
	    fprintf(stderr,"dbg2       navlat:     %f\n",navlat);
	    fprintf(stderr,"dbg2       speed:      %f\n",speed);
	    fprintf(stderr,"dbg2       heading:    %f\n",heading);
	    }
	if (verbose >= 2 && kind == MB_DATA_DATA)
	    {
	    fprintf(stderr,"dbg2       nbath:      %d\n",
		    nbath);
	    if (verbose >= 3) 
	     for (i=0;i<nbath;i++)
	      fprintf(stderr,"dbg3       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
		    i,beamflag[i],bath[i],
		    bathacrosstrack[i],bathalongtrack[i]);
	    fprintf(stderr,"dbg2       namp:       %d\n",namp);
	    if (verbose >= 3) 
	     for (i=0;i<namp;i++)
	      fprintf(stderr,"dbg3        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n",
		    i,amp[i],bathacrosstrack[i],bathalongtrack[i]);
	    fprintf(stderr,"dbg2        nss:       %d\n",nss);
	    if (verbose >= 3) 
	     for (i=0;i<nss;i++)
	      fprintf(stderr,"dbg3        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
		    i,ss[i],ssacrosstrack[i],ssalongtrack[i]);
	    }
	if (verbose >= 2 && kind == MB_DATA_COMMENT)
	    {
	    fprintf(stderr,"dbg2       comment:     \ndbg2       %s\n",
			comment);
	    }

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_oic_struct *) store_ptr;

	/* set data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
	    {
	    /* get time */
	    store->sec = (int) time_d;
	    store->usec = (int) (1000000 * (time_d - store->sec));

	    /* get navigation */
	    if (navlon < 0.0) navlon = navlon + 360.0;
	    store->nav_type = OIC_NAV_LONLAT;
	    store->fish_x = navlon;
	    store->fish_y = navlat;

	    /* get heading */
	    store->fish_heading = heading;

	    /* get speed */
	    store->ship_speed = speed / 3.6;

	    /* set numbers of beams and sidescan */
	    store->beams_bath = nbath;
	    store->beams_amp = namp;
	    store->pixels_ss = nss;

	    /* get bath and sidescan */
	    if (store->beams_bath > store->beams_bath_alloc
		|| store->beamflag == NULL
		|| store->bath == NULL
		|| store->bathacrosstrack == NULL
		|| store->bathalongtrack == NULL
		|| store->tt == NULL
		|| store->angle == NULL)
		{
		store->beams_bath_alloc = store->beams_bath;
		if (store->beamflag != NULL)
		    status = mb_free(verbose, &(store->beamflag), error);
		if (store->bath != NULL)
		    status = mb_free(verbose, &(store->bath), error);
		if (store->bathacrosstrack != NULL)
		    status = mb_free(verbose, &(store->bathacrosstrack), error);
		if (store->bathalongtrack != NULL)
		    status = mb_free(verbose, &(store->bathalongtrack), error);
		if (store->tt != NULL)
		    status = mb_free(verbose, &(store->bathalongtrack), error);
		if (store->angle != NULL)
		    status = mb_free(verbose, &(store->bathalongtrack), error);
		status = mb_malloc(verbose,store->beams_bath_alloc * sizeof(char), 
				    &(store->beamflag),error);
		status = mb_malloc(verbose,store->beams_bath_alloc * sizeof(float), 
				    &(store->bath),error);
		status = mb_malloc(verbose,store->beams_bath_alloc * sizeof(float), 
				    &(store->bathacrosstrack),error);
		status = mb_malloc(verbose,store->beams_bath_alloc * sizeof(float), 
				    &(store->bathalongtrack),error);
		status = mb_malloc(verbose,store->beams_bath_alloc * sizeof(float), 
				    &(store->tt),error);
		status = mb_malloc(verbose,store->beams_bath_alloc * sizeof(float), 
				    &(store->angle),error);
		}
	    if (store->beams_amp > store->beams_amp_alloc
		|| store->amp == NULL)
		{
		store->beams_amp_alloc = store->beams_amp;
		if (store->amp != NULL)
		    status = mb_free(verbose, &(store->amp), error);
		status = mb_malloc(verbose,store->beams_bath_alloc * sizeof(char), 
				    &(store->amp),error);
		}
	    if (store->pixels_ss > store->pixels_ss_alloc
		|| store->ss == NULL
		|| store->ssacrosstrack == NULL
		|| store->ssalongtrack == NULL)
		{
		store->pixels_ss_alloc = store->pixels_ss;
		if (store->ss != NULL)
		    status = mb_free(verbose, &(store->ss), error);
		if (store->ssacrosstrack != NULL)
		    status = mb_free(verbose, &(store->ssacrosstrack), error);
		if (store->ssalongtrack != NULL)
		    status = mb_free(verbose, &(store->ssalongtrack), error);
		status = mb_malloc(verbose,store->pixels_ss_alloc * sizeof(float), 
				    &(store->ss),error);
		status = mb_malloc(verbose,store->pixels_ss_alloc * sizeof(float), 
				    &(store->ssacrosstrack),error);
		status = mb_malloc(verbose,store->pixels_ss_alloc * sizeof(float), 
				    &(store->ssalongtrack),error);
		}
	    for (i=0;i<store->beams_bath;i++)
		{
		store->beamflag[i] = beamflag[i];
		store->bath[i] = bath[i];
		store->bathacrosstrack[i] = bathacrosstrack[i];
		store->bathalongtrack[i] = bathalongtrack[i];
		}
	    for (i=0;i<store->beams_amp;i++)
		{
		store->amp[i] = amp[i];
		}
	    for (i=0;i<store->pixels_ss;i++)
		{
		store->ss[i] = ss[i];
		store->ssacrosstrack[i] = ssacrosstrack[i];
		store->ssalongtrack[i] = ssalongtrack[i];
		}
	    }

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT)
	    {
	    strncpy(store->client,comment,MBSYS_OIC_MAX_COMMENT);
	    store->client_size = strlen(comment) + 1;
	    store->type = OIC_ID_COMMENT;
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_oic_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams,
	double *ttimes, double *angles, 
	double *angles_forward, double *angles_null,
	double *heave, double *alongtrack_offset, 
	double *draft, double *ssv, int *error)
{
	char	*function_name = "mbsys_oic_ttimes";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_oic_struct *store;
	double	alpha, beta;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
		    function_name);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
	    fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
	    fprintf(stderr,"dbg2       ttimes:     %d\n",ttimes);
	    fprintf(stderr,"dbg2       angles_xtrk:%d\n",angles);
	    fprintf(stderr,"dbg2       angles_ltrk:%d\n",angles_forward);
	    fprintf(stderr,"dbg2       angles_null:%d\n",angles_null);
	    fprintf(stderr,"dbg2       heave:      %d\n",heave);
	    fprintf(stderr,"dbg2       ltrk_off:   %d\n",alongtrack_offset);
	    }

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_oic_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
	    {
	    /* get nbeams */
	    *nbeams = store->beams_bath;

	    /* initialize */
	    for (i=0;i<store->beams_bath;i++)
		{
		ttimes[i] = store->tt[i];
		beta = store->angle[i];
		alpha = store->fish_pitch;
		status = mb_rollpitch_to_takeoff(verbose,
				    alpha, beta,
				    &angles[i],
				    &angles_forward[i],error);
		angles_null[i] = 0.0;
		heave[i] = 0.0;
		alongtrack_offset[i] = 0.0;
		}

	    /* get ssv */
	    *ssv = store->sound_velocity;
	    
	    /* get draft */
	    *draft = store->fish_depth;

	    /* set status */
	    *error = MB_ERROR_NO_ERROR;
	    status = MB_SUCCESS;

	    /* done translating values */

	    }

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT)
	    {
	    /* set status */
	    *error = MB_ERROR_COMMENT;
	    status = MB_FAILURE;
	    }

	/* deal with other record type */
	else
	    {
	    /* set status */
	    *error = MB_ERROR_OTHER;
	    status = MB_FAILURE;
	    }

	/* print output debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
		    function_name);
	    fprintf(stderr,"dbg2  Return values:\n");
	    fprintf(stderr,"dbg2       kind:       %d\n",*kind);
	    }
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR)
	    {
	    fprintf(stderr,"dbg2       draft:      %f\n",*draft);
	    fprintf(stderr,"dbg2       ssv:        %f\n",*ssv);
	    fprintf(stderr,"dbg2       nbeams:     %d\n",*nbeams);
	    for (i=0;i<*nbeams;i++)
		    fprintf(stderr,"dbg2       beam %d: tt:%f  angle_xtrk:%f angle_ltrk:%f  angle_null:%f  depth_off:%f  ltrk_off:%f\n",
			    i,ttimes[i],angles[i],
			    angles_forward[i],angles_null[i],
			    heave[i],alongtrack_offset[i]);
	    }
	if (verbose >= 2)
	    {
	    fprintf(stderr,"dbg2       error:      %d\n",*error);
	    fprintf(stderr,"dbg2  Return status:\n");
	    fprintf(stderr,"dbg2       status:     %d\n",status);
	    }

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_oic_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, double *transducer_depth, double *altitude, 
	int *error)
{
	char	*function_name = "mbsys_oic_extract_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_oic_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
		    function_name);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
	    fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
	    }

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_oic_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
	    {
	    *transducer_depth = store->fish_depth;
	    *altitude = store->fish_altitude;
	    }

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT)
	    {
	    /* set status */
	    *error = MB_ERROR_COMMENT;
	    status = MB_FAILURE;
	    }

	/* deal with other record type */
	else
	    {
	    /* set status */
	    *error = MB_ERROR_OTHER;
	    status = MB_FAILURE;
	    }

	/* print output debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
		    function_name);
	    fprintf(stderr,"dbg2  Return values:\n");
	    fprintf(stderr,"dbg2       kind:              %d\n",*kind);
	    fprintf(stderr,"dbg2       transducer_depth:  %f\n",*transducer_depth);
	    fprintf(stderr,"dbg2       altitude:          %f\n",*altitude);
	    fprintf(stderr,"dbg2       error:             %d\n",*error);
	    fprintf(stderr,"dbg2  Return status:\n");
	    fprintf(stderr,"dbg2       status:            %d\n",status);
	    }

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_oic_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	double transducer_depth, double altitude, 
	int *error)
{
	char	*function_name = "mbsys_oic_insert_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_oic_struct *store;
	double	depthscale;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
		    function_name);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
	    fprintf(stderr,"dbg2       mb_ptr:            %d\n",mbio_ptr);
	    fprintf(stderr,"dbg2       store_ptr:         %d\n",store_ptr);
	    fprintf(stderr,"dbg2       transducer_depth:  %f\n",transducer_depth);
	    fprintf(stderr,"dbg2       altitude:          %f\n",altitude);
	    }

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_oic_struct *) store_ptr;

	/* insert data into structure */
	if (store->kind == MB_DATA_DATA)
	    {
	    store->fish_depth = transducer_depth;
	    store->fish_altitude = altitude;
	    }

	/* deal with comment */
	else if (store->kind == MB_DATA_COMMENT)
	    {
	    /* set status */
	    *error = MB_ERROR_COMMENT;
	    status = MB_FAILURE;
	    }

	/* deal with other record type */
	else
	    {
	    /* set status */
	    *error = MB_ERROR_OTHER;
	    status = MB_FAILURE;
	    }

	/* print output debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
		    function_name);
	    fprintf(stderr,"dbg2  Return values:\n");
	    fprintf(stderr,"dbg2       error:             %d\n",*error);
	    fprintf(stderr,"dbg2  Return status:\n");
	    fprintf(stderr,"dbg2       status:            %d\n",status);
	    }

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_oic_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft, 
		double *roll, double *pitch, double *heave, 
		int *error)
{
	char	*function_name = "mbsys_oic_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_oic_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
		    function_name);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
	    fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
	    }

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_oic_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
	    {
	    /* get time */
	    *time_d = store->sec + 0.000001 * store->usec;
	    mb_get_date(verbose,*time_d,time_i);

	    /* get navigation */
	    if (store->nav_type == OIC_NAV_LONLAT)
		{
		*navlon = store->fish_x;
		*navlat = store->fish_y;
		}
	    else
		{
		*navlon = 0.0;
		*navlat = 0.0;
		}

	    /* get heading */
	    *heading = store->fish_heading;

	    /* get speed */
	    *speed = 3.6 * store->ship_speed;
	    
	    /* get draft */
	    *draft = store->fish_depth;
	    
	    /* get roll pitch and heave */
	    *roll = store->fish_roll;
	    *pitch = store->fish_pitch;
	    *heave = 0.0;

	    /* print debug statements */
	    if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg4  Data extracted by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4  Extracted values:\n");
		fprintf(stderr,"dbg4       kind:       %d\n",
			*kind);
		fprintf(stderr,"dbg4       error:      %d\n",
			*error);
		fprintf(stderr,"dbg4       time_i[0]:  %d\n",
			time_i[0]);
		fprintf(stderr,"dbg4       time_i[1]:  %d\n",
			time_i[1]);
		fprintf(stderr,"dbg4       time_i[2]:  %d\n",
			time_i[2]);
		fprintf(stderr,"dbg4       time_i[3]:  %d\n",
			time_i[3]);
		fprintf(stderr,"dbg4       time_i[4]:  %d\n",
			time_i[4]);
		fprintf(stderr,"dbg4       time_i[5]:  %d\n",
			time_i[5]);
		fprintf(stderr,"dbg4       time_i[6]:  %d\n",
			time_i[6]);
		fprintf(stderr,"dbg4       time_d:     %f\n",
			*time_d);
		fprintf(stderr,"dbg4       longitude:  %f\n",
			*navlon);
		fprintf(stderr,"dbg4       latitude:   %f\n",
			*navlat);
		fprintf(stderr,"dbg4       speed:      %f\n",
			*speed);
		fprintf(stderr,"dbg4       heading:    %f\n",
			*heading);
		fprintf(stderr,"dbg4       draft:      %f\n",
			*draft);
		fprintf(stderr,"dbg4       roll:       %f\n",
			*roll);
		fprintf(stderr,"dbg4       pitch:      %f\n",
			*pitch);
		fprintf(stderr,"dbg4       heave:      %f\n",
			*heave);
		}

	    /* done translating values */

	    }

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT)
	    {
	    /* set status */
	    *error = MB_ERROR_COMMENT;
	    status = MB_FAILURE;
	    }

	/* deal with other record type */
	else
	    {
	    /* set status */
	    *error = MB_ERROR_OTHER;
	    status = MB_FAILURE;
	    }

	/* print output debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
		    function_name);
	    fprintf(stderr,"dbg2  Return values:\n");
	    fprintf(stderr,"dbg2       kind:       %d\n",*kind);
	    }
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR 
		&& *kind == MB_DATA_DATA)
	    {
	    fprintf(stderr,"dbg2       time_i[0]:     %d\n",time_i[0]);
	    fprintf(stderr,"dbg2       time_i[1]:     %d\n",time_i[1]);
	    fprintf(stderr,"dbg2       time_i[2]:     %d\n",time_i[2]);
	    fprintf(stderr,"dbg2       time_i[3]:     %d\n",time_i[3]);
	    fprintf(stderr,"dbg2       time_i[4]:     %d\n",time_i[4]);
	    fprintf(stderr,"dbg2       time_i[5]:     %d\n",time_i[5]);
	    fprintf(stderr,"dbg2       time_i[6]:     %d\n",time_i[6]);
	    fprintf(stderr,"dbg2       time_d:        %f\n",*time_d);
	    fprintf(stderr,"dbg2       longitude:     %f\n",*navlon);
	    fprintf(stderr,"dbg2       latitude:      %f\n",*navlat);
	    fprintf(stderr,"dbg2       speed:         %f\n",*speed);
	    fprintf(stderr,"dbg2       heading:       %f\n",*heading);
	    fprintf(stderr,"dbg2       draft:         %f\n",*draft);
	    fprintf(stderr,"dbg2       roll:          %f\n",*roll);
	    fprintf(stderr,"dbg2       pitch:         %f\n",*pitch);
	    fprintf(stderr,"dbg2       heave:         %f\n",*heave);
	    }
	if (verbose >= 2)
	    {
	    fprintf(stderr,"dbg2       error:      %d\n",*error);
	    fprintf(stderr,"dbg2  Return status:\n");
	    fprintf(stderr,"dbg2       status:     %d\n",status);
	    }

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_oic_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading, double draft, 
		double roll, double pitch, double heave,
		int *error)
{
	char	*function_name = "mbsys_oic_insert_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_oic_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
		    function_name);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
	    fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
	    fprintf(stderr,"dbg2       time_i[0]:  %d\n",time_i[0]);
	    fprintf(stderr,"dbg2       time_i[1]:  %d\n",time_i[1]);
	    fprintf(stderr,"dbg2       time_i[2]:  %d\n",time_i[2]);
	    fprintf(stderr,"dbg2       time_i[3]:  %d\n",time_i[3]);
	    fprintf(stderr,"dbg2       time_i[4]:  %d\n",time_i[4]);
	    fprintf(stderr,"dbg2       time_i[5]:  %d\n",time_i[5]);
	    fprintf(stderr,"dbg2       time_i[6]:  %d\n",time_i[6]);
	    fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
	    fprintf(stderr,"dbg2       navlon:     %f\n",navlon);
	    fprintf(stderr,"dbg2       navlat:     %f\n",navlat);
	    fprintf(stderr,"dbg2       speed:      %f\n",speed);
	    fprintf(stderr,"dbg2       heading:    %f\n",heading);
	    fprintf(stderr,"dbg2       draft:      %f\n",draft);
	    fprintf(stderr,"dbg2       roll:       %f\n",roll);
	    fprintf(stderr,"dbg2       pitch:      %f\n",pitch);
	    fprintf(stderr,"dbg2       heave:      %f\n",heave);
	    }

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_oic_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
	    {
	    /* get time */
	    store->sec = (int) time_d;
	    store->usec = (int) (1000000 * (time_d - store->sec));

	    /* get navigation */
	    if (navlon < 0.0) navlon = navlon + 360.0;
	    store->nav_type = OIC_NAV_LONLAT;
	    store->fish_x = navlon;
	    store->fish_y = navlat;

	    /* get heading */
	    store->fish_heading = heading;

	    /* get speed */
	    store->ship_speed = speed / 3.6;

	    /* get draft */
	    store->fish_depth = draft;

	    /* get roll pitch and heave */
	    store->fish_roll = roll;
	    store->fish_pitch = pitch;
	    }

	/* print output debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
		    function_name);
	    fprintf(stderr,"dbg2  Return value:\n");
	    fprintf(stderr,"dbg2       error:      %d\n",*error);
	    fprintf(stderr,"dbg2  Return status:\n");
	    fprintf(stderr,"dbg2       status:  %d\n",status);
	    }

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_oic_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error)
{
	char	*function_name = "mbsys_oic_copy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_oic_struct *store;
	struct mbsys_oic_struct *copy;
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
		fprintf(stderr,"dbg2       copy_ptr:   %d\n",copy_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointers */
	store = (struct mbsys_oic_struct *) store_ptr;
	copy = (struct mbsys_oic_struct *) copy_ptr;
	
	/* copy the basic header data */
	if (store != NULL && copy != NULL)
	    {
	    /* type of data record */
	    copy->kind = store->kind;
	    copy->type = store->type;

	    /* status and size */
	    copy->proc_status = store->proc_status;
	    copy->data_size = store->data_size;
	    copy->client_size = store->client_size;
	    copy->fish_status = store->fish_status;
	    copy->type = store->type;
	    copy->type = store->type;

	    /* nav */
	    copy->nav_used = store->nav_used;
	    copy->nav_type = store->nav_type;
	    copy->utm_zone = store->utm_zone;
	    copy->ship_x = store->ship_x;
	    copy->ship_y = store->ship_y;
	    copy->ship_course = store->ship_course;
	    copy->ship_speed = store->ship_speed;
	    copy->ship_x = store->ship_x;

	    /* time stamp */
	    copy->sec = store->sec;
	    copy->usec = store->usec;

	    /* more stuff */
	    copy->spare_gain = store->spare_gain;
	    copy->fish_heading = store->fish_heading;
	    copy->fish_depth = store->fish_depth;
	    copy->fish_range = store->fish_range;
	    copy->fish_pulse_width = store->fish_pulse_width;
	    copy->gain_c0 = store->gain_c0;
	    copy->gain_c1 = store->gain_c1;
	    copy->gain_c2 = store->gain_c2;
	    copy->fish_pitch = store->fish_pitch;
	    copy->fish_roll = store->fish_roll;
	    copy->fish_yaw = store->fish_yaw;
	    copy->fish_x = store->fish_x;
	    copy->fish_y = store->fish_y;
	    copy->fish_layback = store->fish_layback;
	    copy->fish_altitude = store->fish_altitude;
	    copy->fish_altitude_samples = store->fish_altitude_samples;
	    copy->fish_ping_period = store->fish_ping_period;
	    copy->sound_velocity = store->sound_velocity;

	    /* numbers of beams and scaling */
	    copy->num_chan = store->num_chan;
	    copy->beams_bath = store->beams_bath;
	    copy->beams_amp = store->beams_amp;
	    copy->bath_chan_port = store->bath_chan_port;
	    copy->bath_chan_stbd = store->bath_chan_stbd;
	    copy->pixels_ss = store->pixels_ss;
	    copy->ss_chan_port = store->ss_chan_port;
	    copy->ss_chan_stbd = store->ss_chan_stbd;
	    }
	
	/* allocate the raw data */
	if (store != NULL && copy != NULL)
	    {
	    for (i=0;i<copy->num_chan;i++)
		{
		/* copy channel info */
		copy->channel[i].offset = store->channel[i].offset;
		copy->channel[i].type = store->channel[i].type;
		copy->channel[i].side = store->channel[i].side;
		copy->channel[i].size = store->channel[i].size;
		copy->channel[i].empty = store->channel[i].empty;
		copy->channel[i].frequency = store->channel[i].frequency;
		copy->channel[i].num_samples = store->channel[i].num_samples;

		/* allocate data if needed */
		if (store->rawsize[i] > copy->rawsize[i] 
		    || copy->raw[i] == NULL)
		    {
		    if (copy->raw[i] != NULL)
			status = mb_free(verbose, &(copy->raw[i]), error);
		    copy->rawsize[i] = store->rawsize[i];
		    status = mb_malloc(verbose, copy->rawsize[i], &(copy->raw[i]),error);
		    }

		/* copy the raw data */
		for (j=0;j<copy->rawsize[i];j++)
		    {
		    copy->raw[i][j] = store->raw[i][j];		    
		    }
		}
	    }
	
	/* allocate the depths and sidescan */
	if (status == MB_SUCCESS 
	    && store != NULL && copy != NULL)
	    {
	    if (store->beams_bath > copy->beams_bath_alloc
		|| copy->beamflag == NULL
		|| copy->bath == NULL
		|| copy->bathacrosstrack == NULL
		|| copy->bathalongtrack == NULL
		|| copy->tt == NULL
		|| copy->angle == NULL)
		{
		copy->beams_bath_alloc = store->beams_bath;
		if (copy->beamflag != NULL)
		    status = mb_free(verbose, &(copy->beamflag), error);
		if (copy->bath != NULL)
		    status = mb_free(verbose, &(copy->bath), error);
		if (copy->bathacrosstrack != NULL)
		    status = mb_free(verbose, &(copy->bathacrosstrack), error);
		if (copy->bathalongtrack != NULL)
		    status = mb_free(verbose, &(copy->bathalongtrack), error);
		if (copy->tt != NULL)
		    status = mb_free(verbose, &(copy->tt), error);
		if (copy->angle != NULL)
		    status = mb_free(verbose, &(copy->angle), error);
		status = mb_malloc(verbose,copy->beams_bath_alloc * sizeof(char), 
				    &(copy->beamflag),error);
		status = mb_malloc(verbose,copy->beams_bath_alloc * sizeof(float), 
				    &(copy->bath),error);
		status = mb_malloc(verbose,copy->beams_bath_alloc * sizeof(float), 
				    &(copy->bathacrosstrack),error);
		status = mb_malloc(verbose,copy->beams_bath_alloc * sizeof(float), 
				    &(copy->bathalongtrack),error);
		status = mb_malloc(verbose,copy->beams_bath_alloc * sizeof(float), 
				    &(copy->tt),error);
		status = mb_malloc(verbose,copy->beams_bath_alloc * sizeof(float), 
				    &(copy->angle),error);
		}
	    if (store->beams_amp > copy->beams_amp_alloc
		|| copy->amp == NULL)
		{
		copy->beams_amp_alloc = store->beams_amp;
		if (copy->amp != NULL)
		    status = mb_free(verbose, &(copy->amp), error);
		status = mb_malloc(verbose,copy->beams_bath_alloc * sizeof(char), 
				    &(copy->amp),error);
		}
	    if (store->pixels_ss > copy->pixels_ss_alloc
		|| copy->ss == NULL
		|| copy->ssacrosstrack == NULL
		|| copy->ssalongtrack == NULL)
		{
		copy->pixels_ss_alloc = store->pixels_ss;
		if (copy->ss != NULL)
		    status = mb_free(verbose, &(copy->ss), error);
		if (copy->ssacrosstrack != NULL)
		    status = mb_free(verbose, &(copy->ssacrosstrack), error);
		if (copy->ssalongtrack != NULL)
		    status = mb_free(verbose, &(copy->ssalongtrack), error);
		status = mb_malloc(verbose,copy->pixels_ss_alloc * sizeof(float), 
				    &(copy->ss),error);
		status = mb_malloc(verbose,copy->pixels_ss_alloc * sizeof(float), 
				    &(copy->ssacrosstrack),error);
		status = mb_malloc(verbose,copy->pixels_ss_alloc * sizeof(float), 
				    &(copy->ssalongtrack),error);
		}
	    }
	
	/* copy the depths and sidescan */
	if (status == MB_SUCCESS 
	    && store != NULL && copy != NULL)
	    {
	    for (i=0;i<copy->beams_bath;i++)
		{
		copy->beamflag[i] = store->beamflag[i];
		copy->bath[i] = store->bath[i];
		copy->bathacrosstrack[i] = store->bathacrosstrack[i];
		copy->bathalongtrack[i] = store->bathalongtrack[i];
		copy->tt[i] = store->tt[i];
		copy->angle[i] = store->angle[i];
		}
	    for (i=0;i<copy->beams_amp;i++)
		{
		copy->amp[i] = store->amp[i];
		}
	    for (i=0;i<copy->pixels_ss;i++)
		{
		copy->ss[i] = store->ss[i];
		copy->ssacrosstrack[i] = store->ssacrosstrack[i];
		copy->ssalongtrack[i] = store->ssalongtrack[i];
		}
		    
	    /* client */
	    for (i=0;i<store->client_size;i++)
		copy->client[i] = store->client[i];
	    }

	/* deal with a memory allocation failure */
	if (status == MB_FAILURE)
	    {
	    for (i=0;i<copy->num_chan;i++)
		{
		if (copy->raw[i] != NULL)
		    status = mb_free(verbose, &(copy->raw[i]), error);
		}
	    if (copy->beamflag != NULL)
		status = mb_free(verbose, &copy->beamflag, error);
	    if (copy->bath != NULL)
		status = mb_free(verbose, &copy->bath, error);
	    if (copy->bathacrosstrack != NULL)
		status = mb_free(verbose, &copy->bathacrosstrack, error);
	    if (copy->bathalongtrack != NULL)
		status = mb_free(verbose, &copy->bathalongtrack, error);
	    if (copy->tt != NULL)
		status = mb_free(verbose, &copy->tt, error);
	    if (copy->angle != NULL)
		status = mb_free(verbose, &copy->angle, error);
	    if (copy->amp != NULL)
		status = mb_free(verbose, &copy->amp, error);
	    if (copy->ss != NULL)
		status = mb_free(verbose, &copy->ss, error);
	    if (copy->ssacrosstrack != NULL)
		status = mb_free(verbose, &copy->ssacrosstrack, error);
	    if (copy->ssalongtrack != NULL)
		status = mb_free(verbose, &copy->ssalongtrack, error);
	    if (copy->beamflag != NULL)
		status = MB_FAILURE;
	    *error = MB_ERROR_MEMORY_FAIL;
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

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/

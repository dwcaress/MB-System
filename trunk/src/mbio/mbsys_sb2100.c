/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_sb2100.c	3/2/94
 *	$Id: mbsys_sb2100.c,v 4.17 2000-09-30 06:32:52 caress Exp $
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
 * mbsys_sb2100.c contains the functions for handling the data structure
 * used by MBIO functions to store data from the SeaBeam 2100 and 
 * SeaBeam 1000 series multibeam sonar systems.
 * The data formats which are commonly used to store SeaBeam 1000/2100
 * data in files include
 *      MBF_SB2100RW : MBIO ID 41
 *      MBF_SB2100B1 : MBIO ID 42
 *      MBF_SB2100B2 : MBIO ID 43
 * These functions include:
 *   mbsys_sb2100_alloc   - allocate memory for mbsys_sb2100_struct structure
 *   mbsys_sb2100_deall   - deallocate memory for mbsys_sb2100_struct structure
 *   mbsys_sb2100_extract - extract basic data from mbsys_sb2100_struct structure
 *   mbsys_sb2100_insert  - insert basic data into mbsys_sb2100_struct structure
 *   mbsys_sb2100_ttimes  - extract travel time and beam angle data from
 *                          mbsys_sb2100_struct structure
 *   mbsys_sb2100_extract_nav - extract navigation data from
 *                          mbsys_sb2100_struct structure
 *   mbsys_sb2100_insert_nav - insert navigation data into
 *                          mbsys_sb2100_struct structure
 *   mbsys_sb2100_copy	  - copy data in one mbsys_sb2100_struct structure
 *   				into another mbsys_sb2100_struct structure
 *
 * Author:	D. W. Caress
 * Date:	March 2, 1994
 * $Log: not supported by cvs2svn $
 * Revision 4.16  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.15  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.14  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.14  1997/04/17  15:07:36  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.13  1996/06/05  21:06:27  caress
 * Fixed problem handling gain of sidescan and amplitude data.
 * Previously transmit attenuation was treated as a gain value
 * (positive) rather than an attenuation value.
 *
 * Revision 4.12  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.11  1996/01/26  21:23:30  caress
 * Version 4.3 distribution
 *
 * Revision 4.10  1995/09/28  18:10:48  caress
 * Various bug fixes working toward release 4.3.
 *
 * Revision 4.9  1995/08/17  14:41:09  caress
 * Revision for release 4.3.
 *
 * Revision 4.8  1995/07/13  19:13:36  caress
 * Intermediate check-in during major bug-fixing flail.
 *
 * Revision 4.7  1995/06/03  03:25:13  caress
 * Fixes to handling of changes to vendor SB2100 format
 *
 * Revision 4.6  1995/05/08  21:26:28  caress
 * Made changes consistent with new i/o spec for SB2100 data.
 *
 * Revision 4.5  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.4  1994/11/09  21:40:34  caress
 * Changed ttimes extraction routines to handle forward beam angles
 * so that alongtrack distances can be calculated.
 *
 * Revision 4.3  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.2  1994/04/11  23:34:41  caress
 * Added function to extract travel time and beam angle data
 * from multibeam data in an internal data structure.
 *
 * Revision 4.2  1994/04/11  23:34:41  caress
 * Added function to extract travel time and beam angle data
 * from multibeam data in an internal data structure.
 *
 * Revision 4.1  1994/04/09  15:49:21  caress
 * Altered to fit latest iteration of SeaBeam 2100 vendor format.
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.0  1994/03/05  02:12:07  caress
 * First cut for SeaBeam 2100 i/o.
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
#include "../../include/mbsys_sb2100.h"

/*--------------------------------------------------------------------*/
int mbsys_sb2100_alloc(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	**store_ptr;
int	*error;
{
 static char res_id[]="$Id: mbsys_sb2100.c,v 4.17 2000-09-30 06:32:52 caress Exp $";
	char	*function_name = "mbsys_sb2100_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_sb2100_struct *store;

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
	status = mb_malloc(verbose,sizeof(struct mbsys_sb2100_struct),
				store_ptr,error);

	/* get store structure pointer */
	store = (struct mbsys_sb2100_struct *) *store_ptr;
				
	/* set comment pointer */
	store->comment = (char *) &(store->roll_bias_port);

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
int mbsys_sb2100_deall(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbsys_sb2100_deall";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_sb2100_struct *store;

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
int mbsys_sb2100_extract(verbose,mbio_ptr,store_ptr,kind,
		time_i,time_d,navlon,navlat,speed,heading,
		nbath,namp,nss,
		beamflag,bath,amp,
		bathacrosstrack,bathalongtrack,
		ss,ssacrosstrack,ssalongtrack,
		comment,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*kind;
int	time_i[7];
double	*time_d;
double	*navlon;
double	*navlat;
double	*speed;
double	*heading;
int	*nbath;
int	*namp;
int	*nss;
char	*beamflag;
double	*bath;
double	*amp;
double	*bathacrosstrack;
double	*bathalongtrack;
double	*ss;
double	*ssacrosstrack;
double	*ssalongtrack;
char	*comment;
int	*error;
{
	char	*function_name = "mbsys_sb2100_extract";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_sb2100_struct *store;
	int	time_j[5];
	double	gain_db;
	double	gain_factor;
	int	center_pixel;
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
	store = (struct mbsys_sb2100_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		time_j[0] = store->year;
		time_j[1] = store->jday;
		time_j[2] = 60*store->hour + store->minute;
		time_j[3] = store->sec;
		time_j[4] = 1000 * store->msec;
		mb_get_itime(verbose,time_j,time_i);
		mb_get_time(verbose,time_i,time_d);

		/* get navigation */
		*navlon = store->longitude;
		*navlat = store->latitude;
		if (mb_io_ptr->lonflip < 0)
			{
			if (*navlon > 0.) 
				*navlon = *navlon - 360.;
			else if (*navlon < -360.)
				*navlon = *navlon + 360.;
			}
		else if (mb_io_ptr->lonflip == 0)
			{
			if (*navlon > 180.) 
				*navlon = *navlon - 360.;
			else if (*navlon < -180.)
				*navlon = *navlon + 360.;
			}
		else
			{
			if (*navlon > 360.) 
				*navlon = *navlon - 360.;
			else if (*navlon < 0.)
				*navlon = *navlon + 360.;
			}

		/* get heading */
		*heading = store->heading;

		/* get speed */
		*speed = 0.18553167*store->speed;

		/* read beam and pixel values into storage arrays */
		*nbath = store->nbeams;
		*namp = store->nbeams;
		*nss = store->npixels;
		center_pixel = store->npixels / 2;
		gain_db = store->ping_gain 
			- store->transmitter_attenuation
			+ 10.0 * log10( store->ping_pulse_width / 5.0)
			- 30.0;
		gain_factor = pow(10.0, (-gain_db / 20.0));
		for (i=0;i<*nbath;i++)
			{
			if (store->beams[i].quality == ' ')
			    beamflag[i] = MB_FLAG_NONE;
			else if (store->beams[i].quality == '0')
			    beamflag[i] = MB_FLAG_NULL;
			else if (store->beams[i].quality == 'Q')
			    beamflag[i] = MB_FLAG_SONAR + MB_FLAG_FLAG;
			else if (store->beams[i].quality == 'E')
			    beamflag[i] = MB_FLAG_MANUAL + MB_FLAG_FLAG;
			else if (store->beams[i].quality == 'F')
			    beamflag[i] = MB_FLAG_FILTER + MB_FLAG_FLAG;
			bath[i] = store->beams[i].depth;
			bathacrosstrack[i] = store->beams[i].acrosstrack;
			bathalongtrack[i] = store->beams[i].alongtrack;
			}
		for (i=0;i<*namp;i++)
			{
			amp[i] = 0.25 * store->beams[i].amplitude - gain_db;
			}
		for (i=0;i<*nss;i++)
			{
			ss[i] = gain_factor * store->pixels[i].amplitude;
			ssacrosstrack[i] = store->pixel_size
				* (i - center_pixel);
			ssalongtrack[i] = store->pixels[i].alongtrack;
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
		strcpy(comment,store->comment);

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
int mbsys_sb2100_insert(verbose,mbio_ptr,store_ptr,
		time_i,time_d,navlon,navlat,speed,heading,
		nbath,namp,nss,
		beamflag,bath,amp,
		bathacrosstrack,bathalongtrack,
		ss,ssacrosstrack,ssalongtrack,
		comment,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	time_i[7];
double	time_d;
double	navlon;
double	navlat;
double	speed;
double	heading;
int	nbath;
int	namp;
int	nss;
char	*beamflag;
double	*bath;
double	*amp;
double	*bathacrosstrack;
double	*bathalongtrack;
double	*ss;
double	*ssacrosstrack;
double	*ssalongtrack;
char	*comment;
int	*error;
{
	char	*function_name = "mbsys_sb2100_insert";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_sb2100_struct *store;
	int	kind;
	int	time_j[5];
	int	set_pixel_size;
	double	gain_db;
	double	gain_factor;
	int	center_pixel;
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
		fprintf(stderr,"dbg2       nbath:      %d\n",nbath);
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
		fprintf(stderr,"dbg2       comment:    %s\n",comment);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_sb2100_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get time */
		mb_get_jtime(verbose,time_i,time_j);
		store->year = time_j[0];
		store->jday = time_j[1];
		store->hour = time_j[2]/60;
		store->minute = time_j[2] - 60*store->hour;
		store->sec = time_j[3];
		store->msec = (int) (0.001 * time_j[4]);

		/* get number of beams and pixels */
		store->nbeams = nbath;
		store->npixels = nss;
		center_pixel = store->npixels / 2;

		/* get navigation */
		if (navlon < 0.0) navlon = navlon + 360.0;
		store->longitude = navlon;
		store->latitude = navlat;

		/* get heading */
		store->heading = heading;

		/* get speed */
		store->speed = 5.3899155 * speed;

		/* put beam and pixel values 
			into data structure */
		gain_db = store->ping_gain 
			- store->transmitter_attenuation
			+ 10.0 * log10( store->ping_pulse_width / 5.0)
			- 30.0;
		gain_factor = pow(10.0, (gain_db / 20.0));
		for (i=0;i<nbath;i++)
			{
			if (mb_beam_check_flag(beamflag[i]))
			    {
			    if (mb_beam_check_flag_null(beamflag[i]))
				store->beams[i].quality = '0';
			    else if (mb_beam_check_flag_manual(beamflag[i]))
				store->beams[i].quality = 'E';
			    else if (mb_beam_check_flag_filter(beamflag[i]))
				store->beams[i].quality = 'F';
			    else if (mb_beam_check_flag_sonar(beamflag[i]))
				store->beams[i].quality = 'Q';
			    }
			else 
			    store->beams[i].quality = ' ';
			store->beams[i].depth = bath[i];
			store->beams[i].acrosstrack = bathacrosstrack[i];
			store->beams[i].alongtrack = bathalongtrack[i];
			}
		for (i=0;i<namp;i++)
			store->beams[i].amplitude = 4.0 * (amp[i] + gain_db);
		if (store->pixel_size <= 0.0)
			set_pixel_size = MB_YES;
		else
			set_pixel_size = MB_NO;
		for (i=0;i<nss;i++)
			{
			store->pixels[i].amplitude = gain_factor * ss[i];
			store->pixels[i].alongtrack = ssalongtrack[i];
			if (set_pixel_size == MB_YES
				&& ssacrosstrack[i] > 0)
				{
				store->pixel_size = ssacrosstrack[i]/
					(i - center_pixel);
				set_pixel_size = MB_NO;
				}
			}
		}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT)
		{
		strcpy(store->comment,comment);
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
int mbsys_sb2100_ttimes(verbose,mbio_ptr,store_ptr,kind,nbeams,
	ttimes,angles,angles_forward,angles_null,
	heave,alongtrack_offset,draft,ssv,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*kind;
int	*nbeams;
double	*ttimes;
double	*angles;
double	*angles_forward;
double	*angles_null;
double	*heave;
double	*alongtrack_offset;
double	*draft;
double	*ssv;
int	*error;
{
	char	*function_name = "mbsys_sb2100_ttimes";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_sb2100_struct *store;
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
	store = (struct mbsys_sb2100_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get nbeams */
		*nbeams = store->nbeams;
		/* ignore draft because the values arrive intermittently */
		*draft = 0.0;
		*ssv = store->ssv;

		/* get travel times, angles */
		for (i=0;i<*nbeams;i++)
			{
			ttimes[i] = store->beams[i].range;
			angles[i] = store->beams[i].angle_across;
			angles_forward[i] = store->beams[i].angle_forward;
			if (angles[i] < 0.0)
				{
				angles[i] = -angles[i];
				angles_forward[i] = angles_forward[i]
					+ 180.0;
				}
			angles_null[i] = 0.0;
			heave[i] = store->heave;
			alongtrack_offset[i] = 0.0;
			}

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
			fprintf(stderr,"dbg2       beam %d: tt:%f  angle_xtrk:%f  angle_ltrk:%f  angle_null:%f  depth_off:%f  ltrk_off:%f\n",
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
int mbsys_sb2100_altitude(verbose,mbio_ptr,store_ptr,
	kind,transducer_depth,altitude,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*kind;
double	*transducer_depth;
double	*altitude;
int	*error;
{
	char	*function_name = "mbsys_sb2100_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_sb2100_struct *store;
	double	bath_best;
	double	xtrack_min;
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
	store = (struct mbsys_sb2100_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		bath_best = 0.0;
		if (store->beams[store->nbeams/2].depth > 0.0)
		    bath_best = store->beams[store->nbeams/2].depth;
		else
		    {
		    xtrack_min = 99999999.9;
		    for (i=0;i<store->nbeams;i++)
			{
			if (store->beams[i].depth > 0.0
			    && fabs(store->beams[i].acrosstrack) < xtrack_min)
			    {
			    xtrack_min = fabs(store->beams[i].acrosstrack);
			    bath_best = store->beams[i].depth;
			    }
			}		
		    }
		if (bath_best <= 0.0)
		    {
		    xtrack_min = 99999999.9;
		    for (i=0;i<store->nbeams;i++)
			{
			if (store->beams[i].depth < 0.0
			    && fabs(store->beams[i].acrosstrack) < xtrack_min)
			    {
			    xtrack_min = fabs(store->beams[i].acrosstrack);
			    bath_best = -store->beams[i].depth;
			    }
			}		
		    }
		*transducer_depth = 0.0;
		*altitude = bath_best - *transducer_depth;

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
int mbsys_sb2100_extract_nav(verbose,mbio_ptr,store_ptr,kind,
		time_i,time_d,navlon,navlat,speed,heading,draft, 
		roll,pitch,heave,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*kind;
int	time_i[7];
double	*time_d;
double	*navlon;
double	*navlat;
double	*speed;
double	*heading;
double	*draft;
double	*roll;
double	*pitch;
double	*heave;
int	*error;
{
	char	*function_name = "mbsys_sb2100_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_sb2100_struct *store;
	int	time_j[5];
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
	store = (struct mbsys_sb2100_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		time_j[0] = store->year;
		time_j[1] = store->jday;
		time_j[2] = 60*store->hour + store->minute;
		time_j[3] = store->sec;
		time_j[4] = 1000 * store->msec;
		mb_get_itime(verbose,time_j,time_i);
		mb_get_time(verbose,time_i,time_d);

		/* get navigation */
		*navlon = store->longitude;
		*navlat = store->latitude;
		if (mb_io_ptr->lonflip < 0)
			{
			if (*navlon > 0.) 
				*navlon = *navlon - 360.;
			else if (*navlon < -360.)
				*navlon = *navlon + 360.;
			}
		else if (mb_io_ptr->lonflip == 0)
			{
			if (*navlon > 180.) 
				*navlon = *navlon - 360.;
			else if (*navlon < -180.)
				*navlon = *navlon + 360.;
			}
		else
			{
			if (*navlon > 360.) 
				*navlon = *navlon - 360.;
			else if (*navlon < 0.)
				*navlon = *navlon + 360.;
			}

		/* get heading */
		*heading = store->heading;

		/* get speed */
		*speed = 0.18553167*store->speed;

		/* get draft */
		*draft = store->ship_draft;

		/* get roll pitch and heave */
		*roll = store->roll;
		*pitch = store->pitch;
		*heave = store->heave;

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
int mbsys_sb2100_insert_nav(verbose,mbio_ptr,store_ptr,
		time_i,time_d,navlon,navlat,speed,heading,draft, 
		roll,pitch,heave,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	time_i[7];
double	time_d;
double	navlon;
double	navlat;
double	speed;
double	heading;
double	draft;
double	roll;
double	pitch;
double	heave;
int	*error;
{
	char	*function_name = "mbsys_sb2100_insert_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_sb2100_struct *store;
	int	kind;
	int	time_j[5];
	double	scale, pixel_size;
	int	set_pixel_size;
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
	store = (struct mbsys_sb2100_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get time */
		mb_get_jtime(verbose,time_i,time_j);
		store->year = time_j[0];
		store->jday = time_j[1];
		store->hour = time_j[2]/60;
		store->minute = time_j[2] - 60*store->hour;
		store->sec = time_j[3];
		store->msec = (int) (0.001 * time_j[4]);

		/* get navigation */
		if (navlon < 0.0) navlon = navlon + 360.0;
		store->longitude = navlon;
		store->latitude = navlat;

		/* get heading */
		store->heading = heading;

		/* get speed */
		store->speed = 5.3899155*speed;

		/* get draft */
		store->ship_draft = draft;

		/* get roll pitch and heave */
		store->roll = roll;
		store->pitch = pitch;
		store->heave = heave;
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
int mbsys_sb2100_copy(verbose,mbio_ptr,store_ptr,copy_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
char	*copy_ptr;
int	*error;
{
	char	*function_name = "mbsys_sb2100_copy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_sb2100_struct *store;
	struct mbsys_sb2100_struct *copy;

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
	store = (struct mbsys_sb2100_struct *) store_ptr;
	copy = (struct mbsys_sb2100_struct *) copy_ptr;

	/* copy the data */
	*copy = *store;
	
	/* reset the comment pointer */
	copy->comment = (char *) &(copy->roll_bias_port);

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

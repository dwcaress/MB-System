/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_gsf.c	3.00	8/20/94
 *	$Id: mbsys_gsf.c,v 5.1 2001-01-22 07:43:34 caress Exp $
 *
 *    Copyright (c) 1994, 2000 by
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
 * mbsys_gsf.c contains the functions for handling the data structure
 * used by MBIO functions to store data 
 * from Gsf BottomChart Mark II multibeam sonar systems.
 * The data formats which are commonly used to store Gsf 
 * data in files include
 *      MBF_ELUNB : MBIO ID 92
 *
 * Author:	D. W. Caress
 * Date:	March 5, 1998
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.6  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.5  2000/09/30  06:32:52  caress
 * Snapshot for Dale.
 *
 * Revision 4.4  2000/06/08  22:20:16  caress
 * Handled Reson 8101 SAIC data more properly - get angles
 * correctly even when not signed.
 *
 * Revision 4.3  2000/03/06  21:54:21  caress
 * Distribution 4.6.10
 *
 * Revision 4.2  1999/07/16  19:24:15  caress
 * Yet another version.
 *
 * Revision 4.1  1999/05/05  22:48:29  caress
 * Disabled handling of ping flags in GSF data.
 *
 * Revision 4.0  1998/10/05  18:30:03  caress
 * MB-System version 4.6beta
 *
 * Revision 1.1  1998/10/05  18:22:40  caress
 * Initial revision
 *
 * Revision 1.1  1998/10/05  17:46:15  caress
 * Initial revision
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mb_define.h"
#include "../../include/gsf.h"
#include "../../include/mbsys_gsf.h"

/*--------------------------------------------------------------------*/
int mbsys_gsf_alloc(int verbose, char *mbio_ptr, char **store_ptr, 
			int *error)
{
 static char res_id[]="$Id: mbsys_gsf.c,v 5.1 2001-01-22 07:43:34 caress Exp $";
	char	*function_name = "mbsys_gsf_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	int	i, j;

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
	status = mb_malloc(verbose,sizeof(struct mbsys_gsf_struct),
				store_ptr,error);
	memset(*store_ptr, 0, sizeof(struct mbsys_gsf_struct));

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
int mbsys_gsf_deall(int verbose, char *mbio_ptr, char **store_ptr, 
			int *error)
{
	char	*function_name = "mbsys_gsf_deall";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfRecords	    *records;

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

	/* deallocate memory for data structure */
	store =  (struct mbsys_gsf_struct *) *store_ptr;
	records = &(store->records);
	gsfFree(records);
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
int mbsys_gsf_extract(int verbose, char *mbio_ptr, char *store_ptr, 
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_gsf_extract";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSwathBathyPing   *mb_ping;
	int	i, j;

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
	store = (struct mbsys_gsf_struct *) store_ptr;
	dataID = &(store->dataID);
	records = &(store->records);

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		mb_ping = &(records->mb_ping);

		/* get time */
		*time_d = mb_ping->ping_time.tv_sec 
				+ 0.000000001 * mb_ping->ping_time.tv_nsec;
		mb_get_date(verbose,*time_d,time_i);

		/* get navigation */
		*navlon = mb_ping->longitude;
		*navlat = mb_ping->latitude;
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
		*heading = mb_ping->heading;

		/* get speed */
		*speed = 1.852 * mb_ping->speed;

		/* get numbers of beams and pixels */
		if (mb_ping->depth != NULL)
		    *nbath = mb_ping->number_beams;
		else
		    *nbath = 0;
		if (mb_ping->mc_amplitude != NULL
		    || mb_ping->mr_amplitude != NULL)
		    *namp = mb_ping->number_beams;
		else
		    *namp = 0;
		*nss = 0;

		/* read depth and beam location values into storage arrays */
		for (i=0;i<*nbath;i++)
			{
			/* set null beam flag if required */
			if (mb_ping->depth[i] == 0.0
			    && mb_ping->across_track[i] == 0.0
			    && mb_ping->beam_flags[i] != MB_FLAG_NULL)
			    mb_ping->beam_flags[i] = MB_FLAG_NULL;

			beamflag[i] = mb_ping->beam_flags[i];
			bath[i] = mb_ping->depth[i];
			bathacrosstrack[i] = mb_ping->across_track[i];
			bathalongtrack[i] = mb_ping->along_track[i];
			}

		/* set beamflags if ping flag set */
		if (mb_ping->ping_flags != 0)
		    {
		    for (i=0;i<*nbath;i++)
			if (mb_beam_ok(beamflag[i]))
			    beamflag[i] 
				= mb_beam_set_flag_manual(beamflag[i]);
		    }

		/* read amplitude values into storage arrays */
		if (mb_ping->mc_amplitude != NULL)
		for (i=0;i<*namp;i++)
			{
			amp[i] = mb_ping->mc_amplitude[i];
			}
		else if (mb_ping->mr_amplitude != NULL)
		for (i=0;i<*namp;i++)
			{
			amp[i] = mb_ping->mr_amplitude[i];
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
			}

		/* done translating values */

		}

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		strcpy(comment, records->comment.comment);

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  Comment extracted by MBIO function <%s>\n",
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
int mbsys_gsf_insert(int verbose, char *mbio_ptr, char *store_ptr, 
		int kind, int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_gsf_insert";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSwathBathyPing   *mb_ping;
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
		}
	if (verbose >= 2 && kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg2       comment:     \ndbg2       %s\n",
			comment);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_gsf_struct *) store_ptr;
	records = &(store->records);
	dataID = &(store->dataID);
	mb_ping = &(records->mb_ping);

	/* set data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		dataID->recordID = GSF_RECORD_SWATH_BATHYMETRY_PING;
		mb_ping = &(records->mb_ping);

		/* get time */
		mb_ping->ping_time.tv_sec = (int) time_d;
		mb_ping->ping_time.tv_nsec 
			= (int) (1000000000 
			    * (time_d 
				    - mb_ping->ping_time.tv_sec));

		/* get navigation */
		mb_ping->longitude = navlon;
		mb_ping->latitude = navlat;

		/* get heading */
		mb_ping->heading = heading;

		/* get speed */
		mb_ping->speed = speed / 1.852;

		/* get numbers of beams */
		mb_ping->number_beams = nbath;
		
		/* allocate memory in arrays if required */
		if (nbath > 0)
		    {
		    mb_ping->beam_flags 
			= (unsigned char *) 
			    realloc(mb_ping->beam_flags,
					nbath * sizeof(char));
		    mb_ping->depth 
			= (double *) 
			    realloc(mb_ping->depth,
					nbath * sizeof(double));
		    mb_ping->across_track 
			= (double *) 
			    realloc(mb_ping->across_track,
					nbath * sizeof(double));
		    mb_ping->along_track 
			= (double *) 
			    realloc(mb_ping->along_track,
					nbath * sizeof(double));
		    if (mb_ping->beam_flags == NULL
			|| mb_ping->depth == NULL
			|| mb_ping->across_track == NULL
			|| mb_ping->along_track == NULL)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_MEMORY_FAIL;
			}
		    }
		if (namp > 0
		    && mb_ping->mc_amplitude != NULL)
		    {
		    mb_ping->mc_amplitude 
			= (double *) 
			    realloc(mb_ping->mc_amplitude,
					namp * sizeof(double));
		    if (mb_ping->mc_amplitude == NULL)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_MEMORY_FAIL;
			}
		    }
		else if (namp > 0)
		    {
		    mb_ping->mr_amplitude 
			= (double *) 
			    realloc(mb_ping->mr_amplitude,
					namp * sizeof(double));
		    if (mb_ping->mr_amplitude == NULL)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_MEMORY_FAIL;
			}
		    }

		/* if ping flag set check for any unset
		    beam flags - unset ping flag if any
		    good beams found */
		if (mb_ping->ping_flags != 0)
		    {
		    for (i=0;i<nbath;i++)
			{
			if (mb_beam_ok(beamflag[i]))
			    mb_ping->ping_flags = 0;
			}
		    }

		/* read depth and beam location values into storage arrays */
		for (i=0;i<nbath;i++)
			{
			mb_ping->beam_flags[i] = beamflag[i];
			mb_ping->depth[i] = bath[i];
			mb_ping->across_track[i] = bathacrosstrack[i];
			mb_ping->along_track[i] = bathalongtrack[i];
			}

		/* read amplitude values into storage arrays */
		if (mb_ping->mc_amplitude != NULL)
		for (i=0;i<namp;i++)
			{
			mb_ping->mc_amplitude[i] = amp[i];
			}
		else if (mb_ping->mr_amplitude != NULL)
		for (i=0;i<namp;i++)
			{
			mb_ping->mr_amplitude[i] = amp[i];
			}
		}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT)
		{
		dataID->recordID = GSF_RECORD_COMMENT;
		if (records->comment.comment_length < strlen(comment) + 1)
		    {
		    if ((records->comment.comment 
				= (char *) 
				    realloc(records->comment.comment,
					strlen(comment)+1))
					    == NULL) 
			{
			status = MB_FAILURE;
			*error = MB_ERROR_MEMORY_FAIL;
			records->comment.comment_length = 0;
			}
		    }
		if (status = MB_SUCCESS && records->comment.comment != NULL)
		    {
		    strcpy(records->comment.comment, comment);
		    records->comment.comment_length = strlen(comment)+1;
		    records->comment.comment_time.tv_sec = (int) time_d;
		    records->comment.comment_time.tv_nsec 
			    = (int) (1000000000 
				* (time_d 
					- records->comment.comment_time.tv_sec));
		    }
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
int mbsys_gsf_ttimes(int verbose, char *mbio_ptr, char *store_ptr,
	int *kind, int *nbeams,
	double *ttimes, double *angles, 
	double *angles_forward, double *angles_null,
	double *heave, double *alongtrack_offset, 
	double *draft, double *ssv, int *error)
{
	char	*function_name = "mbsys_gsf_ttimes";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSwathBathyPing   *mb_ping;
	double	alpha, beta;
	int	i, j;

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
	store = (struct mbsys_gsf_struct *) store_ptr;
	records = &(store->records);
	dataID = &(store->dataID);
	mb_ping = &(records->mb_ping);

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
	    {
	    /* get nbeams */
	    *nbeams = mb_ping->number_beams;

	    /* get travel times, angles */
	    if (mb_ping->travel_time != NULL
		&& mb_ping->beam_angle != NULL)
		{
		if (mb_ping->beam_angle_forward != NULL)
		    {
		    for (i=0;i<*nbeams;i++)
			{
			ttimes[i] = mb_ping->travel_time[i];
			angles[i] = fabs(mb_ping->beam_angle[i]);
			angles_forward[i] = mb_ping->beam_angle_forward[i];
			heave[i] = mb_ping->heave;
			alongtrack_offset[i] = 0.0;
			}
		    }
		else
		    {
		    for (i=0;i<*nbeams;i++)
			{
			ttimes[i] = mb_ping->travel_time[i];
			angles[i] = fabs(mb_ping->beam_angle[i]);
			if (mb_ping->across_track[i] < 0.0
			    || mb_ping->beam_angle[i] < 0.0)
				angles_forward[i] = 180.0;
			else
				angles_forward[i] = 0.0;
			heave[i] = mb_ping->heave;
			alongtrack_offset[i] = 0.0;
			}
		    }
		
		/* get surface sound velocity and 
		    effective array mount angles */
		if (mb_ping->sensor_id 
		    == GSF_SWATH_BATHY_SUBRECORD_SEABEAM_SPECIFIC)
		    {
		    *ssv = 1500.0;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			angles_null[i] = 0.0;
		    }
		else if (mb_ping->sensor_id 
		    == GSF_SWATH_BATHY_SUBRECORD_EM100_SPECIFIC)
		    {
		    *ssv = 1500.0;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			    angles_null[i] = angles[i];
		    }
		else if (mb_ping->sensor_id 
		    == GSF_SWATH_BATHY_SUBRECORD_EM950_SPECIFIC)
		    {
		    *ssv = mb_ping->sensor_data.gsfEM950Specific.surface_velocity;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			    angles_null[i] = angles[i];
		    }
		else if (mb_ping->sensor_id 
		    == GSF_SWATH_BATHY_SUBRECORD_EM121A_SPECIFIC)
		    {
		    *ssv = mb_ping->sensor_data.gsfEM121ASpecific.surface_velocity;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			angles_null[i] = 0.0;
		    }
		else if (mb_ping->sensor_id 
		    == GSF_SWATH_BATHY_SUBRECORD_EM121_SPECIFIC)
		    {
		    *ssv = mb_ping->sensor_data.gsfEM121Specific.surface_velocity;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			angles_null[i] = 0.0;
		    }
		else if (mb_ping->sensor_id 
		    == GSF_SWATH_BATHY_SUBRECORD_SEAMAP_SPECIFIC)
		    {
		    *ssv = 1500.0;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			    angles_null[i] = 50.0;
		    }
		else if (mb_ping->sensor_id 
		    == GSF_SWATH_BATHY_SUBRECORD_SEABAT_SPECIFIC)
		    {
		    *ssv = mb_ping->sensor_data.gsfSeaBatSpecific.surface_velocity;
		    *draft = mb_ping->depth_corrector;
		    if (mb_ping->beam_angle_forward == NULL)
			{
			for (i=0;i<*nbeams;i++)
			    {
			    if (mb_ping->across_track[i] < 0.0
				&& mb_ping->beam_angle[i] > 0.0)
				beta = 90.0 + mb_ping->beam_angle[i];
			    else
				beta = 90.0 - mb_ping->beam_angle[i];
			    alpha = mb_ping->pitch;
			    mb_rollpitch_to_takeoff(verbose, 
				alpha, beta, &angles[i], 
				&angles_forward[i], error);
			    }
			}
		    for (i=0;i<*nbeams;i++)
			    angles_null[i] = angles[i];
		    }
		else if (mb_ping->sensor_id 
		    == GSF_SWATH_BATHY_SUBRECORD_EM1000_SPECIFIC)
		    {
		    *ssv = mb_ping->sensor_data.gsfEM1000Specific.surface_velocity;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			    angles_null[i] = angles[i];
		    }
		else if (mb_ping->sensor_id 
		    == GSF_SWATH_BATHY_SUBRECORD_TYPEIII_SEABEAM_SPECIFIC)
		    {
		    *ssv = 1500.0;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			angles_null[i] = 0.0;
		    }
		else if (mb_ping->sensor_id 
		    == GSF_SWATH_BATHY_SUBRECORD_SB_AMP_SPECIFIC)
		    {
		    *ssv = 1500.0;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			angles_null[i] = 0.0;
		    }
		else if (mb_ping->sensor_id 
		    == GSF_SWATH_BATHY_SUBRECORD_SEABAT_II_SPECIFIC)
		    {
		    *ssv = mb_ping->sensor_data.gsfSeaBatIISpecific.surface_velocity;
		    *draft = mb_ping->depth_corrector;
		    if (mb_ping->beam_angle_forward == NULL)
			{
			for (i=0;i<*nbeams;i++)
			    {
			    if (mb_ping->across_track[i] < 0.0
				&& mb_ping->beam_angle[i] > 0.0)
				beta = 90.0 + mb_ping->beam_angle[i];
			    else
				beta = 90.0 - mb_ping->beam_angle[i];
			    alpha = mb_ping->pitch;
			    mb_rollpitch_to_takeoff(verbose, 
				alpha, beta, &angles[i], 
				&angles_forward[i], error);
			    }
			}
		    for (i=0;i<*nbeams;i++)
			    angles_null[i] = angles[i];
		    }
		else if (mb_ping->sensor_id 
		    == GSF_SWATH_BATHY_SUBRECORD_SEABAT_8101_SPECIFIC)
		    {
		    *ssv = mb_ping->sensor_data.gsfSeaBat8101Specific.surface_velocity;
		    *draft = mb_ping->depth_corrector;
		    if (mb_ping->beam_angle_forward == NULL)
			{
			for (i=0;i<*nbeams;i++)
			    {
			    if (mb_ping->across_track[i] < 0.0
				&& mb_ping->beam_angle[i] > 0.0)
				beta = 90.0 + mb_ping->beam_angle[i];
			    else
				beta = 90.0 - mb_ping->beam_angle[i];
			    alpha = mb_ping->pitch;
			    mb_rollpitch_to_takeoff(verbose, 
				alpha, beta, &angles[i], 
				&angles_forward[i], error);
			    }
			}
		    for (i=0;i<*nbeams;i++)
			    angles_null[i] = angles[i];
		    }
		else if (mb_ping->sensor_id 
		    == GSF_SWATH_BATHY_SUBRECORD_SEABEAM_2112_SPECIFIC)
		    {
		    *ssv = mb_ping->sensor_data.gsfSeaBeam2112Specific.surface_velocity;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			angles_null[i] = 0.0;
		    }
		else if (mb_ping->sensor_id 
		    == GSF_SWATH_BATHY_SUBRECORD_ELAC_MKII_SPECIFIC)
		    {
		    *ssv = mb_ping->sensor_data.gsfElacMkIISpecific.sound_vel;
		    *draft = mb_ping->depth_corrector;
		    if (mb_ping->beam_angle_forward == NULL)
			{
			for (i=0;i<*nbeams;i++)
			    {
			    beta = 90.0 - mb_ping->beam_angle[i];
			    alpha = mb_ping->pitch;
			    mb_rollpitch_to_takeoff(verbose, 
				alpha, beta, &angles[i], 
				&angles_forward[i], error);
			    }
			}
		    for (i=0;i<*nbeams;i++)
			    angles_null[i] = 37.5;
		    }
		else
		    {
		    *ssv = 1500.0;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			angles_null[i] = 0.0;
		    }
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
			fprintf(stderr,"dbg2       beam %d: tt:%f  angle_xtrk:%f  angle_ltrk:%f  angle_null:%f  heave:%f  ltrk_off:%f\n",
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
int mbsys_gsf_extract_altitude(int verbose, char *mbio_ptr, char *store_ptr,
	int *kind, double *transducer_depth, double *altitude, 
	int *error)
{
	char	*function_name = "mbsys_gsf_extract_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSwathBathyPing   *mb_ping;
	double	bath_best;
	double	xtrack_min;
	int	i, j;

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
	store = (struct mbsys_gsf_struct *) store_ptr;
	records = &(store->records);
	dataID = &(store->dataID);
	mb_ping = &(records->mb_ping);

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
	    {
	    *transducer_depth = mb_ping->depth_corrector;

	    /* get altitude if available */
	    if (mb_ping->sensor_id 
		== GSF_SWATH_BATHY_SUBRECORD_SEAMAP_SPECIFIC)
		{
		*altitude = mb_ping->sensor_data.gsfSeamapSpecific.altitude;
		}
		
	    /* else get altitude from depth */
	    else if (mb_ping->depth != NULL)
		{
		bath_best = 0.0;
		if (mb_ping->depth[mb_ping->number_beams/2] > 0.0)
		    bath_best = mb_ping->depth[mb_ping->number_beams/2];
		else
		    {
		    xtrack_min = 99999999.9;
		    for (i=0;i<mb_ping->number_beams;i++)
			{
			if (mb_beam_check_flag(mb_ping->beam_flags[i])
			    && fabs(mb_ping->across_track[i]) < xtrack_min)
			    {
			    xtrack_min = fabs(mb_ping->across_track[i]);
			    bath_best = mb_ping->depth[i];
			    }
			}		
		    }
		if (bath_best <= 0.0)
		    {
		    xtrack_min = 99999999.9;
		    for (i=0;i<mb_ping->number_beams;i++)
			{
			if (!mb_beam_check_flag(mb_ping->beam_flags[i])
			    && fabs(mb_ping->across_track[i]) < xtrack_min)
			    {
			    xtrack_min = fabs(mb_ping->across_track[i]);
			    bath_best = mb_ping->depth[i];
			    }
			}		
		    }
		*altitude = bath_best - *transducer_depth;
		}
	    
	    /* else no info */
	    else
		{
		*altitude = 0.0;
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
int mbsys_gsf_insert_altitude(int verbose, char *mbio_ptr, char *store_ptr,
	double transducer_depth, double altitude, 
	int *error)
{
	char	*function_name = "mbsys_gsf_insert_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSwathBathyPing   *mb_ping;
	int	i, j;

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
	store = (struct mbsys_gsf_struct *) store_ptr;
	records = &(store->records);
	dataID = &(store->dataID);
	mb_ping = &(records->mb_ping);

	/* insert data into structure */
	if (store->kind == MB_DATA_DATA)
	    {
	    mb_ping->depth_corrector = transducer_depth;

	    /* set altitude if possible */
	    if (mb_ping->sensor_id 
		== GSF_SWATH_BATHY_SUBRECORD_SEAMAP_SPECIFIC)
		{
		mb_ping->sensor_data.gsfSeamapSpecific.altitude = altitude;
		}

	    /* set status */
	    *error = MB_ERROR_NO_ERROR;
	    status = MB_SUCCESS;

	    /* done translating values */

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
int mbsys_gsf_extract_nav(int verbose, char *mbio_ptr, char *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft, 
		double *roll, double *pitch, double *heave, 
		int *error)
{
	char	*function_name = "mbsys_gsf_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSwathBathyPing   *mb_ping;
	int	i, j;

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
	store = (struct mbsys_gsf_struct *) store_ptr;
	records = &(store->records);
	dataID = &(store->dataID);
	mb_ping = &(records->mb_ping);

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		*time_d = mb_ping->ping_time.tv_sec 
				+ 0.000000001 * mb_ping->ping_time.tv_nsec;
		mb_get_date(verbose,*time_d,time_i);

		/* get navigation */
		*navlon = mb_ping->longitude;
		*navlat = mb_ping->latitude;
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
		*heading = mb_ping->heading;

		/* get speed */
		*speed = 1.852 * mb_ping->speed;

		/* get draft */
		*draft = mb_ping->depth_corrector;

		/* get roll pitch and heave */
		*roll = mb_ping->roll;
		*pitch = mb_ping->pitch;
		*heave = mb_ping->heave;

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
int mbsys_gsf_insert_nav(int verbose, char *mbio_ptr, char *store_ptr,
		int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading, double draft, 
		double roll, double pitch, double heave,
		int *error)
{
	char	*function_name = "mbsys_gsf_insert_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSwathBathyPing   *mb_ping;
	int	kind;
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
	store = (struct mbsys_gsf_struct *) store_ptr;
	records = &(store->records);
	dataID = &(store->dataID);
	mb_ping = &(records->mb_ping);

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{

		/* get time */
		mb_ping->ping_time.tv_sec = (int) time_d;
		mb_ping->ping_time.tv_nsec 
			= (int) (1000000000 
			    * (time_d 
				    - mb_ping->ping_time.tv_sec));

		/* get navigation */
		mb_ping->longitude = navlon;
		mb_ping->latitude = navlat;

		/* get heading */
		mb_ping->heading = heading;

		/* get speed */
		mb_ping->speed = speed / 1.852;

		/* get draft */
		mb_ping->depth_corrector = draft;

		/* get roll pitch and heave */
		mb_ping->roll = roll;
		mb_ping->pitch = pitch;
		mb_ping->heave = heave;
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
int mbsys_gsf_extract_svp(int verbose, char *mbio_ptr, char *store_ptr,
		int *kind, int *nsvp,
		double *depth, double *velocity,
		int *error)
{
	char	*function_name = "mbsys_gsf_extract_svp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSVP		    *svp;
	int	i, j;

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
	store = (struct mbsys_gsf_struct *) store_ptr;
	records = &(store->records);
	dataID = &(store->dataID);
	svp = &(records->svp);

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_VELOCITY_PROFILE)
		{
		/* get number of depth-velocity pairs */
		*nsvp = svp->number_points;
		
		/* get profile */
		for (i=0;i<*nsvp;i++)
			{
			depth[i] = svp->depth[i];
			velocity[i] = svp->sound_speed[i];
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
		fprintf(stderr,"dbg2       kind:              %d\n",*kind);
		fprintf(stderr,"dbg2       nsvp:              %d\n",*nsvp);
		for (i=0;i<*nsvp;i++)
		    fprintf(stderr,"dbg2       depth[%d]: %f   velocity[%d]: %f\n",i, depth[i], i, velocity[i]);
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_gsf_insert_svp(int verbose, char *mbio_ptr, char *store_ptr,
		int nsvp,
		double *depth, double *velocity,
		int *error)
{
	char	*function_name = "mbsys_gsf_insert_svp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSVP		    *svp;
	int	kind;
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
		fprintf(stderr,"dbg2       nsvp:       %d\n",nsvp);
		for (i=0;i<nsvp;i++)
		    fprintf(stderr,"dbg2       depth[%d]: %f   velocity[%d]: %f\n",i, depth[i], i, velocity[i]);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_gsf_struct *) store_ptr;
	records = &(store->records);
	dataID = &(store->dataID);
	svp = &(records->svp);

	/* insert data in structure */
	if (store->kind == MB_DATA_VELOCITY_PROFILE)
		{
		/* allocate memory if required */
		if (nsvp > svp->number_points
		    || svp->depth == NULL
		    || svp->sound_speed == NULL)
		    {
		    svp->depth = (double *) realloc(svp->depth, nsvp * sizeof(double));
		    svp->sound_speed = (double *) realloc(svp->sound_speed, nsvp * sizeof(double));
		    }

		/* get number of depth-velocity pairs */
		svp->number_points = nsvp;
		
		/* get profile */
		for (i=0;i<svp->number_points;i++)
			{
			svp->depth[i] = depth[i];
			svp->sound_speed[i] = velocity[i];
			}
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
int mbsys_gsf_copy(int verbose, char *mbio_ptr, 
			char *store_ptr, char *copy_ptr,
			int *error)
{
	char	*function_name = "mbsys_gsf_copy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	struct mbsys_gsf_struct *copy;

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
	store = (struct mbsys_gsf_struct *) store_ptr;
	copy = (struct mbsys_gsf_struct *) copy_ptr;
	
	/* clear copy structure */
	gsfFree(&(copy->records));
	gsfCopyRecords(&(copy->records), &(store->records));
	copy->kind = store->kind;
	copy->dataID = store->dataID;

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

/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_hsmd.c	Aug 10, 1995
 *	$Header: /system/link/server/cvs/root/mbsystem/src/mbio/mbsys_hsmd.c,v 5.2 2001-07-20 00:32:54 caress Exp $
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
 * mbsys_hsmd.c contains the functions for handling the data structure
 * used by MBIO functions to store data from the 40/80-beam Hydrosweep M
 * multibeam sonar systems.
 * The data formats which are commonly used to store Hydrosweep MD
 * Medium Depth data in files include:
 *
 *      MBF_HSMDRAW : MBIO ID 101
 *
 * Author:	Dale Chayes
 * Date:	August 10, 1995
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.9  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.8  2000/09/30  06:32:52  caress
 * Snapshot for Dale.
 *
 * Revision 4.7  1999/09/14  20:39:11  caress
 * Fixed bugs handling HSMD
 *
 * Revision 4.6  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.5  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.4  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.3  1996/07/16  22:07:12  caress
 * Fixed port/starboard mixup and made null angles for raytracing 40 degrees to
 * reflect 40 degree tranducer array mounting.
 *
 * Revision 4.3  1996/07/16  22:07:12  caress
 * Fixed port/starboard mixup and made null angles for raytracing 40 degrees to
 * reflect 40 degree tranducer array mounting.
 *
 * Revision 4.2  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
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
 * Revision 4.2  95/08/16  07:08:05  07:08:05  dale (Dale Chayes)
 * Appears to work for some things.
 *  - does not write data correctly yet
 *  - beams may not come out on the correct side/offset 
 * 
 * Revision 4.1  95/08/14  20:56:02  20:56:02  dale (Dale Chayes)
 * HSMD is sort of working.... needs work and testing.
 * 
 * Revision 4.0  95/08/10  15:58:00  15:58:00  dale (Dale Chayes)
 * Adjust the version number, 1.1 was an error
 * 
 * Revision 1.1  95/08/10  15:56:08  15:56:08  dale (Dale Chayes)
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
#include "../../include/mbsys_hsmd.h"

/*--------------------------------------------------------------------*/
int mbsys_hsmd_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
	static char res_id[]="$Id: mbsys_hsmd.c,v 5.2 2001-07-20 00:32:54 caress Exp $";
	char	*function_name = "mbsys_hsmd_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
  
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
	status = mb_malloc(verbose,sizeof(struct mbsys_hsmd_struct),
		     store_ptr,error);

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
int mbsys_hsmd_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
	char	*function_name = "mbsys_hsmd_deall";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hsmd_struct *store;

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
int mbsys_hsmd_extract(int verbose, void *mbio_ptr, void *store_ptr, 
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_hsmd_extract";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hsmd_struct *store;
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
	store = (struct mbsys_hsmd_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		time_i[0] = store->year;
		time_i[1] = store->month;
		time_i[2] = store->day;
		time_i[3] = store->hour;
		time_i[4] = store->minute;
		time_i[5] = store->second;
		time_i[6] = store->millisecond;
		mb_get_time(verbose,time_i,time_d);
      
		/* get navigation */
		*navlon = store->lon;
		*navlat = store->lat;
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
		*heading = store->heading_tx;
      
		/* set speed to zero */
		*speed = store->speed;

		/* zero bathymetry and sidescan */
		for (i=0;i<MBSYS_HSMD_BEAMS;i++)
			{
			beamflag[i] = MB_FLAG_NULL;
			bath[i] = 0.0;
			bathacrosstrack[i] = 0.0;
			bathalongtrack[i] = 0.0;
			}
		for (i=0;i<MBSYS_HSMD_PIXELS;i++)
			{
			ss[i] = 0.0;
			ssacrosstrack[i] = 0.0;
			ssalongtrack[i] = 0.0;
			}

		/* get bathymetry */
		*nbath = MBSYS_HSMD_BEAMS;

		/* deal with a ping to port */
		if (store->Port == -1) 
			{
			for (i=0;i<MBSYS_HSMD_BEAMS_PING;i++) 
				{
				j = MBSYS_HSMD_BEAMS_PING - i - 1;
				if (store->depth[i] > 0.0)
				    {
				    beamflag[j] = MB_FLAG_NONE;
				    bath[j] = store->depth[i];
				    }
				else if (store->depth[i] < 0.0)
				    {
				    beamflag[j] = 
					MB_FLAG_MANUAL + MB_FLAG_FLAG;
				    bath[j] = -store->depth[i];
				    }
				else
				    {
				    beamflag[j] = MB_FLAG_NULL;
				    bath[j] = store->depth[i];
				    }
				bathacrosstrack[j] = store->distance[i]; 
	 			bathalongtrack[j] = 0.0;
     	 			}
    			}

		/* deal with a ping to starboard */
		else 
			{
			for (i=0;i<MBSYS_HSMD_BEAMS_PING;i++) 
				{
				j = i + MBSYS_HSMD_BEAMS_PING - 1;
				if (store->depth[i] > 0.0)
				    {
				    beamflag[j] = MB_FLAG_NONE;
				    bath[j] = store->depth[i];
				    }
				else if (store->depth[i] < 0.0)
				    {
				    beamflag[j] = 
					MB_FLAG_MANUAL + MB_FLAG_FLAG;
				    bath[j] = -store->depth[i];
				    }
				else
				    {
				    beamflag[j] = MB_FLAG_NULL;
				    bath[j] = store->depth[i];
				    }
	 			bathacrosstrack[j] = 
	    					store->distance[i]; 
	 			bathalongtrack[j] = 0.0;
      				}
    			}

		/* Deal with the sidescan */
		*nss = MBSYS_HSMD_PIXELS;

		/* deal with a ping to port */
		if (store->Port == -1) 
			{
			for (i=0;i<MBSYS_HSMD_PIXELS_PING;i++) 
				{
				j = MBSYS_HSMD_PIXELS_PING - i - 1;
				ss[j] = store->ss[i];
				ssacrosstrack[j] = 
					-store->ss_range * i 
					/ ((double)(MBSYS_HSMD_PIXELS_PING - 1));
	 			ssalongtrack[j] = 0.0;
      				}
    			}

		/* deal with a ping to starboard */
		else 
			{
			for (i=0;i<MBSYS_HSMD_PIXELS_PING;i++) 
				{
				j = i + MBSYS_HSMD_PIXELS_PING - 1;
				ss[j] = store->ss[i];
				ssacrosstrack[j] = 
					store->ss_range * i 
					/ ((double)(MBSYS_HSMD_PIXELS_PING - 1));
	 			ssalongtrack[j] = 0.0;
      				}
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
				i,beamflag[i],bath[i],
				bathacrosstrack[i],bathalongtrack[i]);
			fprintf(stderr,"dbg4        namp:      %d\n",
				*namp);
			for (i=0;i<*nss;i++)
			  fprintf(stderr,"dbg4        ss[%d]: %f  ssdist[%d]:%f\n",
				i,ss[i],i,ssacrosstrack[i]);
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
		fprintf(stderr,"dbg2         nbath:         %d\n",*nbath);
		for (i=0;i<*nbath;i++)
		  fprintf(stderr,"dbg2       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
			i,beamflag[i],bath[i],
			bathacrosstrack[i],bathalongtrack[i]);
		fprintf(stderr,"dbg2         nss:           %d\n",*nss);
		for (i=0;i<*nss;i++)
		  fprintf(stderr,"dbg2       ss[%d]:   %f  ssdist[%d]:   %f\n",
			i,ss[i],i,ssacrosstrack[i]);
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
int mbsys_hsmd_insert(int verbose, void *mbio_ptr, void *store_ptr, 
		int kind, int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_hsmd_insert";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
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
		  fprintf(stderr,"dbg3        amp[%d]: %f\n",
			i,amp[i]);
		fprintf(stderr,"dbg2        nss:       %d\n",nss);
		if (verbose >= 3) 
		 for (i=0;i<nss;i++)
		  fprintf(stderr,"dbg3        ss[%d]: %f    ssdist[%d]: %f\n",
			i,ss[i],i,ssacrosstrack[i]);
		}
	if (verbose >= 2 && kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg2       comment:     \ndbg2       %s\n",
			comment);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hsmd_struct *) store_ptr;

	/* set data kind */
	store->kind = kind;
  
	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
    		{
		/* get time */
		store->year = time_i[0];
		store->month = time_i[1];
		store->day = time_i[2];
		store->hour = time_i[3];
		store->minute = time_i[4];
		store->second = time_i[5];

		/* get navigation */
		if (navlon < 0.0) navlon = navlon + 360.0;
		store->lon = navlon;
		store->lat = navlat;
      
		/* get heading */
		store->heading_tx = heading;

		/* figure out if port or starboard ping */
		first = -1;
		for (i=0;i<nbath;i++) 
			{
			if (first == -1 && bath[i] != 0.0)
				first = i;
			}
		if (first >= MBSYS_HSMD_BEAMS_PING - 1)
			store->Port = 1;
		else
			store->Port = -1;

		/* get bathymetry */

		/* deal with a ping to port */
		if (store->Port == -1) 
			{
			for (i=0;i<MBSYS_HSMD_BEAMS_PING;i++) 
				{
				j = MBSYS_HSMD_BEAMS_PING - i - 1;
				if (mb_beam_check_flag_null(beamflag[j]))
				    store->depth[i] = 0.0;
				else if (mb_beam_check_flag(beamflag[j]))
				    store->depth[i] = -bath[j];
				else
				    store->depth[i] = bath[j];
	 			store->distance[i] = 
	    					bathacrosstrack[j]; 
     	 			}
    			}

		/* deal with a ping to starboard */
		else 
			{
			for (i=0;i<MBSYS_HSMD_BEAMS_PING;i++) 
				{
				j = i + MBSYS_HSMD_BEAMS_PING - 1;
				if (mb_beam_check_flag_null(beamflag[j]))
				    store->depth[i] = 0.0;
				else if (mb_beam_check_flag(beamflag[j]))
				    store->depth[i] = -bath[j];
				else
				    store->depth[i] = bath[j];
	 			store->distance[i] = 
	    					bathacrosstrack[j]; 
      				}
    			}

		/* Deal with the sidescan */

		/* deal with a ping to port */
		if (store->Port == -1) 
			{
			store->ss_range = fabs(ssacrosstrack[0]);
			for (i=0;i<MBSYS_HSMD_PIXELS_PING;i++) 
				{
				j = MBSYS_HSMD_PIXELS_PING - i - 1;
				store->ss[i] = ss[j];
      				}
    			}

		/* deal with a ping to starboard */
		else 
			{
			store->ss_range = 
				ssacrosstrack[MBSYS_HSMD_PIXELS_PING - 1];
			for (i=0;i<MBSYS_HSMD_PIXELS_PING;i++) 
				{
				j = i + MBSYS_HSMD_PIXELS_PING - 1;
				store->ss[i] = ss[j];
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
int mbsys_hsmd_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams,
	double *ttimes, double *angles, 
	double *angles_forward, double *angles_null,
	double *heave, double *alongtrack_offset, 
	double *draft, double *ssv, int *error)
{
	char	*function_name = "mbsys_hsmd_ttimes";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hsmd_struct *store;
	double	scale;
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
	store = (struct mbsys_hsmd_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get nbeams */
		*nbeams = MBSYS_HSMD_BEAMS;

		/* zero travel times, angles */
		for (i=0;i<*nbeams;i++)
			{
			ttimes[i] = 0.0;
			angles[i] = 0.0;
			angles_forward[i] = 0.0;
			angles_null[i] = 40.0;
			heave[i] = 0.0;
			alongtrack_offset[i] = 0.0;
			}

		/* get travel times, angles */
		if (store->skals)
			scale = 0.00015;
		else
			scale = 0.000015;

		/* deal with a ping to port */
		if (store->Port == -1) 
			{
			for (i=0;i<MBSYS_HSMD_BEAMS_PING;i++) 
				{
				j = MBSYS_HSMD_BEAMS_PING - i - 1;
				ttimes[j] = fabs(scale * store->spfb[i]);
				/* angle convention in raw data
				   is positive to port */
				if (store->angle[i] < 0.0)
					{
					angles[j] = -store->angle[i];
					angles_forward[j] = 0.0;
					}
				else
					{
					angles[j] = store->angle[i];
					angles_forward[j] = 180.0;
					}
				heave[j] = store->heave;
     	 			}
    			}

		/* deal with a ping to starboard */
		else 
			{
			for (i=0;i<MBSYS_HSMD_BEAMS_PING;i++) 
				{
				j = i + MBSYS_HSMD_BEAMS_PING - 1;
				ttimes[j] = fabs(scale * store->spfb[i]);
				angles[j] = store->angle[i];
				heave[j] = store->heave;
      				}
    			}

		/* get sound velocity at transducers */
		*ssv = store->ckeel;
		*draft = 0.0;

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
int mbsys_hsmd_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, double *transducer_depth, double *altitude, 
	int *error)
{
	char	*function_name = "mbsys_hsmd_extract_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hsmd_struct *store;
	double	bath_best;
	double	xtrack_min;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
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
	store = (struct mbsys_hsmd_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		bath_best = 0.0;
		if (store->depth[0] > 0.0)
		    bath_best = store->depth[0];
		else
		    {
		    xtrack_min = 99999999.9;
		    for (i=0;i<MBSYS_HSMD_BEAMS_PING;i++)
			{
			if (store->depth[i] > 0.0
			    && fabs(store->distance[i]) < xtrack_min)
			    {
			    xtrack_min = fabs(store->distance[i]);
			    bath_best = store->depth[i];
			    }
			}		
		    }
		if (bath_best <= 0.0)
		    {
		    xtrack_min = 99999999.9;
		    for (i=0;i<MBSYS_HSMD_BEAMS_PING;i++)
			{
			if (store->depth[i] < 0.0
			    && fabs(store->distance[i]) < xtrack_min)
			    {
			    xtrack_min = fabs(store->distance[i]);
			    bath_best = -store->depth[i];
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
int mbsys_hsmd_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft, 
		double *roll, double *pitch, double *heave, 
		int *error)
{
	char	*function_name = "mbsys_hsmd_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hsmd_struct *store;
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
	store = (struct mbsys_hsmd_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA
		|| *kind == MB_DATA_NAV)
		{
		/* get time */
		time_i[0] = store->year;
		time_i[1] = store->month;
		time_i[2] = store->day;
		time_i[3] = store->hour;
		time_i[4] = store->minute;
		time_i[5] = store->second;
		time_i[6] = store->millisecond;
		mb_get_time(verbose,time_i,time_d);
      
		/* get navigation */
		*navlon = store->lon;
		*navlat = store->lat;
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
		*heading = store->heading_tx;

		/* get speed */
		*speed = store->speed;

		/* get draft */
		*draft = 0.0;

		/* get roll pitch and heave */
		*roll = store->roll_tx;
		*pitch = store->pitch_tx;
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
		&& (*kind == MB_DATA_DATA
		    || *kind == MB_DATA_NAV))
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
int mbsys_hsmd_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading, double draft, 
		double roll, double pitch, double heave,
		int *error)
{
	char	*function_name = "mbsys_hsmd_insert_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hsmd_struct *store;
	int	kind;
	double	scalefactor;
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
	store = (struct mbsys_hsmd_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get time */
		store->year = time_i[0];
		store->month = time_i[1];
		store->day = time_i[2];
		store->hour = time_i[3];
		store->minute = time_i[4];
		store->second = time_i[5];

		/* get navigation */
		if (navlon < 0.0) navlon = navlon + 360.0;
		store->lon = navlon;
		store->lat = navlat;
      
		/* get heading */
		store->heading_tx = heading;

		/* get speed */
		store->speed = speed;

		/* get draft */

		/* get roll pitch and heave */
		store->roll_tx = roll;
		store->pitch_tx = pitch;
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
int mbsys_hsmd_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error)
{
	char	*function_name = "mbsys_hsmd_copy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hsmd_struct *store;
	struct mbsys_hsmd_struct *copy;

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
	store = (struct mbsys_hsmd_struct *) store_ptr;
	copy = (struct mbsys_hsmd_struct *) copy_ptr;

	/* copy the data */
	*copy = *store;

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

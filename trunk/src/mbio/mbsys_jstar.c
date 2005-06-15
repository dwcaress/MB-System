/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_jstar.c	10/4/94
 *	$Id: mbsys_jstar.c,v 5.1 2005-06-15 15:20:17 caress Exp $
 *
 *    Copyright (c) 2005 by
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
 * mbsys_jstar.c contains the functions for handling the data structure
 * used by MBIO functions to store data from Edgetech
 * subbottom and sidescan sonar systems.
 * The native data format used to store Edgetech
 * data is called Jstar and is supported by the format:
 *      MBF_EDGJSTAR : MBIO ID 132
 *
 * Author:	D. W. Caress
 * Date:	May 4, 2005
 * $Log: not supported by cvs2svn $
 * Revision 5.0  2005/06/04 04:11:35  caress
 * Support for Edgetech Jstar format (id 132 and 133).
 *
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
#include "../../include/mb_segy.h"
#include "../../include/mbsys_jstar.h"

/*--------------------------------------------------------------------*/
int mbsys_jstar_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
 static char res_id[]="$Id: mbsys_jstar.c,v 5.1 2005-06-15 15:20:17 caress Exp $";
	char	*function_name = "mbsys_jstar_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_jstar_struct *store;
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
	status = mb_malloc(verbose,sizeof(struct mbsys_jstar_struct),
				store_ptr,error);

	/* make sure trace pointers are null */
	store = (struct mbsys_jstar_struct *) mb_io_ptr->store_data;
	store->kind = MB_DATA_NONE;
	store->sbp.trace_alloc = 0;
	store->sbp.trace = NULL;
	store->ssport.trace_alloc = 0;
	store->ssport.trace = NULL;
	store->ssstbd.trace_alloc = 0;
	store->ssstbd.trace = NULL;
	store->comment.comment[0] = 0;

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
int mbsys_jstar_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
	char	*function_name = "mbsys_jstar_deall";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_jstar_struct *store;
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

	/* deallocate memory for data structure */
	if (*store_ptr != NULL)
		{
		store = (struct mbsys_jstar_struct *) *store_ptr;
		if (store->sbp.trace != NULL
			&& store->sbp.trace_alloc > 0)
			{
			status = mb_free(verbose,&(store->sbp.trace),error);
			}
		store->sbp.trace_alloc = 0;
		if (store->ssport.trace != NULL
			&& store->ssport.trace_alloc > 0)
			{
			status = mb_free(verbose,&(store->ssport.trace),error);
			}
		store->ssport.trace_alloc = 0;
		if (store->ssstbd.trace != NULL
			&& store->ssstbd.trace_alloc > 0)
			{
			status = mb_free(verbose,&(store->ssstbd.trace),error);
			}
		store->ssstbd.trace_alloc = 0;
		}
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
int mbsys_jstar_extract(int verbose, void *mbio_ptr, void *store_ptr, 
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_jstar_extract";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_jstar_struct *store;
	struct mbsys_jstar_channel_struct *sbp;
	struct mbsys_jstar_channel_struct *ssport;
	struct mbsys_jstar_channel_struct *ssstbd;
	int	time_j[5];
	double	rawpixelsize;
	double	pixelsize;
	double	weight;
	int	istart;
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
	store = (struct mbsys_jstar_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract subbottom data from structure */
	if (*kind == MB_DATA_SUBBOTTOM_SUBBOTTOM)
		{
		/* get channel */
		sbp = (struct mbsys_jstar_channel_struct *) &(store->sbp);
		
		/* get time */
		time_j[0] = sbp->year;
		time_j[1] = sbp->day;
		time_j[2] = 60 * sbp->hour + sbp->minute;
		time_j[3] = sbp->second;
		time_j[4] = (int)1000 * (sbp->millisecondsToday 
				- 1000 * floor(0.001 * ((double)sbp->millisecondsToday)));
		mb_get_itime(verbose,time_j,time_i);
		mb_get_time(verbose,time_i,time_d);

		/* get navigation */
		*navlon = sbp->sourceCoordX / 360000.0;
		*navlat = sbp->sourceCoordY / 360000.0;

		/* get heading */
		*heading = 0.01 * sbp->heading;

		/* get speed */
		*speed = 0.0;
			
		/* set beamwidths in mb_io structure */
		mb_io_ptr->beamwidth_ltrack = 20.0;
		mb_io_ptr->beamwidth_xtrack = 20.0;

		/* read distance and depth values into storage arrays */
		*nbath = 1;
		*namp = 0;
		*nss = 0;
		
		/* get nadir depth */
		if (ssport->depth > 0)
			{
			bath[0] = 0.001 * ssport->depth;
			beamflag[0] = MB_FLAG_NONE;
			}
		else if (ssport->depth < 0)
			{
			bath[0] = -0.001 * ssport->depth;
			beamflag[0] = MB_FLAG_MANUAL + MB_FLAG_FLAG;
			}
		else
			{
			bath[0] = 0.0;
			beamflag[0] = MB_FLAG_NULL;
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
			  fprintf(stderr,"dbg4       beam:%4d  flag:%3d  bath:%f  bathdist:%f\n",
				i,beamflag[i],bath[i],
				bathacrosstrack[i]);
			fprintf(stderr,"dbg4        nss:      %d\n",
				*nss);
			for (i=0;i<*nss;i++)
			  fprintf(stderr,"dbg4        beam:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
				i,ss[i],ssacrosstrack[i],ssalongtrack[i]);
			}

		/* done translating values */

		}
		
	/* extract sidescan data from structure */
	else if (*kind == MB_DATA_DATA
		|| *kind == MB_DATA_SIDESCAN2)
		{
		/* get channels */
		ssport = (struct mbsys_jstar_channel_struct *) &(store->ssport);
		ssstbd = (struct mbsys_jstar_channel_struct *) &(store->ssstbd);
		
		/* get time */
		time_j[0] = ssport->year;
		time_j[1] = ssport->day;
		time_j[2] = 60 * ssport->hour + ssport->minute;
		time_j[3] = ssport->second;
		time_j[4] = (int)1000 * (ssport->millisecondsToday 
				- 1000 * floor(0.001 * ((double)ssport->millisecondsToday)));
		mb_get_itime(verbose,time_j,time_i);
		mb_get_time(verbose,time_i,time_d);

		/* get navigation */
		*navlon = ssport->sourceCoordX / 360000.0;
		*navlat = ssport->sourceCoordY / 360000.0;

		/* get heading */
		*heading = 0.01 * ssport->heading;

		/* get speed */
		*speed = 0.0;
			
		/* set beamwidths in mb_io structure */
		mb_io_ptr->beamwidth_ltrack = 20.0;
		mb_io_ptr->beamwidth_xtrack = 20.0;

		/* read distance and depth values into storage arrays */
		/* average sidescan into a MBSYS_JSTAR_PIXELS_MAX pixel array */
		*nbath = 1;
		*namp = 0;
		*nss = MBSYS_JSTAR_PIXELS_MAX;
		
		/* get nadir depth */
		if (ssport->depth > 0)
			{
			bath[0] = 0.001 * ssport->depth;
			beamflag[0] = MB_FLAG_NONE;
			}
		else if (ssport->depth < 0)
			{
			bath[0] = -0.001 * ssport->depth;
			beamflag[0] = MB_FLAG_MANUAL + MB_FLAG_FLAG;
			}
		else
			{
			bath[0] = 0.0;
			beamflag[0] = MB_FLAG_NULL;
			}
		
		/* get pixel sizes and bottom arrival */
		rawpixelsize = store->ssport.sampleInterval * 0.00000075;
		pixelsize = (store->ssport.samples + store->ssstbd.samples) * rawpixelsize / *nss;
		istart = (0.001 * ssport->sonaraltitude) / rawpixelsize;
		istart = 0;
		
		/* zero the array */
		for (i=0;i<*nss;i++)
			{
			ss[i] = 0.0;
			ssacrosstrack[i] = pixelsize * (i - *nss / 2 + 0.5);
			ssalongtrack[i] = 0.0;
			}
		
		/* bin the data */
		weight = exp2((double)ssport->weightingFactor);
/*fprintf(stderr, "Subsystem: %d Weights: %d %f ",ssport->message.subsystem,ssport->weightingFactor,weight);*/
		for (i=istart;i<store->ssport.samples;i++)
			{
			j = (*nss / 2 - 1) - (int)((i - istart) * rawpixelsize / pixelsize);
			ss[j] += store->ssport.trace[i] / weight;
			ssalongtrack[j] += 1.0;
			}
		weight = exp2((double)ssstbd->weightingFactor);
/*fprintf(stderr, "   %d %f\n",ssstbd->weightingFactor,weight);*/
		for (i=istart;i<store->ssstbd.samples;i++)
			{
			j = (*nss / 2) + (int)((i - istart) * rawpixelsize / pixelsize);
			ss[j] += store->ssstbd.trace[i] / weight;
			ssalongtrack[j] += 1.0;
			}
		
		/* average the data in the bins */
		for (i=0;i<*nss;i++)
			{
			if (ssalongtrack[i] > 0.0)
				{
				ss[i] /= ssalongtrack[i];
				ssalongtrack[i] = 0.0;
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
			  fprintf(stderr,"dbg4       beam:%4d  flag:%3d  bath:%f  bathdist:%f\n",
				i,beamflag[i],bath[i],
				bathacrosstrack[i]);
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
		strcpy(comment,store->comment.comment);

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
		&& (*kind == MB_DATA_DATA
		|| *kind == MB_DATA_SIDESCAN2))
		{
		fprintf(stderr,"dbg2       nbath:         %d\n",*nbath);
		for (i=0;i<*nbath;i++)
		  fprintf(stderr,"dbg2       beam:%4d  flag:%3d  bath:%f  bathdist:%f\n",
			i,beamflag[i],bath[i],
			bathacrosstrack[i]);
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
int mbsys_jstar_insert(int verbose, void *mbio_ptr, void *store_ptr, 
		int kind, int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_jstar_insert";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_jstar_struct *store;
	struct mbsys_jstar_channel_struct *sbp;
	struct mbsys_jstar_channel_struct *ssport;
	struct mbsys_jstar_channel_struct *ssstbd;
	int	time_j[5];
	double	weight;
	int	shortspersample = 2;
	int	trace_size = shortspersample * nss / 2 * sizeof(short);
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
	if (verbose >= 2 && (kind == MB_DATA_DATA || kind == MB_DATA_SIDESCAN2 || kind == MB_DATA_NAV))
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
	if (verbose >= 2 && (kind == MB_DATA_DATA || kind == MB_DATA_SIDESCAN2))
		{
		fprintf(stderr,"dbg2       nbath:      %d\n",nbath);
		if (verbose >= 3) 
		 for (i=0;i<nbath;i++)
		  fprintf(stderr,"dbg3       beam:%4d  flag:%3d  bath:%f  bathdist:%f\n",
			i,beamflag[i],bath[i],
			bathacrosstrack[i]);
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
	store = (struct mbsys_jstar_struct *) store_ptr;

	/* set data kind */
	store->kind = kind;

	/* insert subbottom data into structure */
	if (kind == MB_DATA_SUBBOTTOM_SUBBOTTOM)
		{
		/* get channel */
		sbp = (struct mbsys_jstar_channel_struct *) &(store->sbp);
		
		/* set kind and subsystem */
		store->kind = MB_DATA_SUBBOTTOM_SUBBOTTOM;
		store->subsystem = MBSYS_JSTAR_SUBSYSTEM_SBP;

		/* get time */
		mb_get_jtime(verbose,time_i,time_j);
		sbp->year = time_i[0];
		sbp->day = time_j[1];
		sbp->hour = time_i[3];
		sbp->minute = time_i[4];
		sbp->second = time_i[5];
		sbp->millisecondsToday = 0.001 * time_i[6]
					+ 1000 * (time_i[5] 
						+ 60.0 * (time_i[4] 
							+ 60.0 * time_i[3]));

		/* get navigation */
		if (navlon < 180.0) navlon = navlon + 360.0;
		if (navlon > 180.0) navlon = navlon - 360.0;
		sbp->sourceCoordX = (int) (360000.0 * navlon);
		sbp->sourceCoordY = (int) (360000.0 * navlat);

		/* get heading */
		sbp->heading = (short) (100.0 * heading);

		/* read distance and depth values into storage arrays */
		
		}
		
	/* insert data in structure */
	else if (store->kind == MB_DATA_DATA || store->kind == MB_DATA_SIDESCAN2)
		{
		/* get channels */
		ssport = (struct mbsys_jstar_channel_struct *) &(store->ssport);
		ssstbd = (struct mbsys_jstar_channel_struct *) &(store->ssstbd);
		
		/* set kind and subsystem */
		if (ssport->message.subsystem == MBSYS_JSTAR_SUBSYSTEM_SSLOW
			&& ssstbd->message.subsystem == MBSYS_JSTAR_SUBSYSTEM_SSLOW)
			{
			store->subsystem = MBSYS_JSTAR_SUBSYSTEM_SSLOW;
			}
		else if (ssport->message.subsystem == MBSYS_JSTAR_SUBSYSTEM_SSHIGH
			&& ssstbd->message.subsystem == MBSYS_JSTAR_SUBSYSTEM_SSHIGH)
			{
			store->subsystem = MBSYS_JSTAR_SUBSYSTEM_SSHIGH;
			}
		else if (mb_io_ptr->format == MBF_EDGJSTAR)
			{
			if (store->kind == MB_DATA_DATA)
				{
				store->subsystem = MBSYS_JSTAR_SUBSYSTEM_SSLOW;
				ssport->message.subsystem = MBSYS_JSTAR_SUBSYSTEM_SSLOW;
				ssstbd->message.subsystem = MBSYS_JSTAR_SUBSYSTEM_SSLOW;
				}
			else
				{
				store->subsystem = MBSYS_JSTAR_SUBSYSTEM_SSHIGH;
				ssport->message.subsystem = MBSYS_JSTAR_SUBSYSTEM_SSHIGH;
				ssstbd->message.subsystem = MBSYS_JSTAR_SUBSYSTEM_SSHIGH;
				}
			}
		else if (mb_io_ptr->format == MBF_EDGJSTR2)
			{
			if (store->kind == MB_DATA_DATA)
				{
				store->subsystem = MBSYS_JSTAR_SUBSYSTEM_SSHIGH;
				ssport->message.subsystem = MBSYS_JSTAR_SUBSYSTEM_SSHIGH;
				ssstbd->message.subsystem = MBSYS_JSTAR_SUBSYSTEM_SSHIGH;
				}
			else
				{
				store->subsystem = MBSYS_JSTAR_SUBSYSTEM_SSLOW;
				ssport->message.subsystem = MBSYS_JSTAR_SUBSYSTEM_SSLOW;
				ssstbd->message.subsystem = MBSYS_JSTAR_SUBSYSTEM_SSLOW;
				}
			}
		else
			{
			ssport->message.subsystem = MBSYS_JSTAR_SUBSYSTEM_SSHIGH;
			ssstbd->message.subsystem = MBSYS_JSTAR_SUBSYSTEM_SSHIGH;
			}
		
		/* get time */
		mb_get_jtime(verbose,time_i,time_j);
		ssport->year = time_i[0];
		ssport->day = time_j[1];
		ssport->hour = time_i[3];
		ssport->minute = time_i[4];
		ssport->second = time_i[5];
		ssport->millisecondsToday = 0.001 * time_i[6]
					+ 1000 * (time_i[5] 
						+ 60.0 * (time_i[4] 
							+ 60.0 * time_i[3]));
		ssstbd->year = time_i[0];
		ssstbd->day = time_j[1];
		ssstbd->hour = time_i[3];
		ssstbd->minute = time_i[4];
		ssstbd->second = time_i[5];
		ssstbd->millisecondsToday = 0.001 * time_i[6]
					+ 1000 * (time_i[5] 
						+ 60.0 * (time_i[4] 
							+ 60.0 * time_i[3]));

		/* get navigation */
		if (navlon < 180.0) navlon = navlon + 360.0;
		if (navlon > 180.0) navlon = navlon - 360.0;
		ssport->sourceCoordX = 360000.0 * navlon;
		ssport->sourceCoordY = 360000.0 * navlat;
		ssstbd->sourceCoordX = 360000.0 * navlon;
		ssstbd->sourceCoordY = 360000.0 * navlat;

		/* get heading and speed */
		ssport->heading = (short) (100.0 * heading);
		ssstbd->heading = (short) (100.0 * heading);

		/* put distance and depth values 
			into data structure */
		
		/* get nadir depth */
		if (nbath > 0)
			{
			ssport->depth = 1000 * bath[nbath/2];
			if (beamflag[nbath/2] == MB_FLAG_NULL)
				ssport->depth = 0;
			else if (mb_beam_check_flag(beamflag[nbath/2]))
				ssport->depth = -ssport->depth;
			}
					
		/* allocate memory for the trace */
		ssport->dataFormat = 1;
		ssstbd->dataFormat = 1;
		shortspersample = 2;
		trace_size = shortspersample * nss / 2 * sizeof(short);
		if (ssport->trace_alloc < trace_size)
			{
			if ((status = mb_realloc(verbose, trace_size, &(ssport->trace), error))
				== MB_SUCCESS)
				{
				ssport->trace_alloc = trace_size;
				}
			}
		if (ssstbd->trace_alloc < trace_size)
			{
			if ((status = mb_realloc(verbose, trace_size, &(ssstbd->trace), error))
				== MB_SUCCESS)
				{
				ssstbd->trace_alloc = trace_size;
				}
			}

		/* put sidescan values 
			into data structure */
		ssport->samples = nss / 2;
		weight = exp2((double)ssport->weightingFactor);
		for (i=0;i<nss/2;i++)
			{
			j = nss/2 - i; 
			ssport->trace[i] = (short) (ss[j] / weight);
			}
		ssstbd->samples = nss / 2;
		weight = exp2((double)ssstbd->weightingFactor);
		for (i=0;i<nss/2;i++)
			{
			j = nss/2 + i; 
			ssport->trace[i] = (short) (ss[j] / weight);
			}
		}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT)
		{
		strcpy(store->comment.comment,comment);
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
int mbsys_jstar_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams,
	double *ttimes, double *angles, 
	double *angles_forward, double *angles_null,
	double *heave, double *alongtrack_offset, 
	double *draft, double *ssv, int *error)
{
	char	*function_name = "mbsys_jstar_ttimes";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_jstar_struct *store;
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
	store = (struct mbsys_jstar_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA
		|| *kind == MB_DATA_SIDESCAN2)
		{
		/* get nbeams */
		*nbeams = 0;

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
int mbsys_jstar_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, double *transducer_depth, double *altitude, 
	int *error)
{
	char	*function_name = "mbsys_jstar_extract_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_jstar_struct *store;
	struct mbsys_jstar_channel_struct *sbp;
	struct mbsys_jstar_channel_struct *ssport;
	struct mbsys_jstar_channel_struct *ssstbd;
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
	store = (struct mbsys_jstar_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_SUBBOTTOM_SUBBOTTOM)
		{
		/* get channel */
		sbp = (struct mbsys_jstar_channel_struct *) &(store->sbp);
		
		/* get transducer_depth */
		if (sbp->sonardepth > 0)
			*transducer_depth = 0.001 * sbp->sonardepth
						+ sbp->heaveCompensation * sbp->sampleInterval * 0.00000075;
		else
			*transducer_depth = sbp->startDepth * sbp->sampleInterval * 0.00000075
						+ sbp->heaveCompensation * sbp->sampleInterval * 0.00000075; 
		*altitude = 0.001 * sbp->sonaraltitude;
		}
		
	else if (*kind == MB_DATA_DATA
		|| *kind == MB_DATA_SIDESCAN2)
		{
		/* get channel */
		ssport = (struct mbsys_jstar_channel_struct *) &(store->ssport);
		
		/* get transducer_depth */
		if (ssport->sonardepth > 0)
			*transducer_depth = 0.001 * ssport->sonardepth
						+ ssport->heaveCompensation * ssport->sampleInterval * 0.00000075;
		else
			*transducer_depth = ssport->startDepth * ssport->sampleInterval * 0.00000075
						+ ssport->heaveCompensation * ssport->sampleInterval * 0.00000075; 
		*altitude = 0.001 * ssport->sonaraltitude;

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
int mbsys_jstar_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft, 
		double *roll, double *pitch, double *heave, 
		int *error)
{
	char	*function_name = "mbsys_jstar_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_jstar_struct *store;
	struct mbsys_jstar_channel_struct *sbp;
	struct mbsys_jstar_channel_struct *ssport;
	struct mbsys_jstar_channel_struct *ssstbd;
	int	time_j[5];
	int	id;
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
	store = (struct mbsys_jstar_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract subbottom data from structure */
	if (*kind == MB_DATA_SUBBOTTOM_SUBBOTTOM)
		{
		/* get channel */
		sbp = (struct mbsys_jstar_channel_struct *) &(store->sbp);
		
		/* get time */
		time_j[0] = sbp->year;
		time_j[1] = sbp->day;
		time_j[2] = 60 * sbp->hour + sbp->minute;
		time_j[3] = sbp->second;
		time_j[4] = (int)1000 * (sbp->millisecondsToday 
				- 1000 * floor(0.001 * ((double)sbp->millisecondsToday)));
		mb_get_itime(verbose,time_j,time_i);
		mb_get_time(verbose,time_i,time_d);

		/* get navigation */
		*navlon = sbp->sourceCoordX / 360000.0;
		*navlat = sbp->sourceCoordY / 360000.0;

		/* get heading */
		*heading = 0.01 * sbp->heading;

		/* get speed */
		*speed = 0.0;
		
		/* get draft */
		if (sbp->sonardepth > 0)
			*draft = 0.001 * sbp->sonardepth;
		else
			*draft = sbp->startDepth 
				* sbp->sampleInterval * 0.00000075; 
		
		/* get attitude */
		*roll = 0.01 * sbp->roll; 
		*pitch = 0.01 * sbp->pitch; 
		*heave = sbp->heaveCompensation 
				* sbp->sampleInterval * 0.00000075; 

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
		

	/* extract data from structure */
	else if (*kind == MB_DATA_DATA
		|| *kind == MB_DATA_SIDESCAN2)
		{
		/* get channel */
		ssport = (struct mbsys_jstar_channel_struct *) &(store->ssport);
		
		/* get time */
		time_j[0] = ssport->year;
		time_j[1] = ssport->day;
		time_j[2] = 60 * ssport->hour + ssport->minute;
		time_j[3] = ssport->second;
		time_j[4] = (int)1000 * (ssport->millisecondsToday 
				- 1000 * floor(0.001 * ((double)ssport->millisecondsToday)));
		mb_get_itime(verbose,time_j,time_i);
		mb_get_time(verbose,time_i,time_d);

		/* get navigation */
		*navlon = ssport->sourceCoordX / 360000.0;
		*navlat = ssport->sourceCoordY / 360000.0;

		/* get heading */
		*heading = 0.01 * ssport->heading;

		/* get speed */
		*speed = 0.0;
		
		/* get draft */
		if (ssport->sonardepth > 0)
			*draft = 0.001 * ssport->sonardepth;
		else
			*draft = ssport->startDepth 
				* ssport->sampleInterval * 0.00000075; 
		
		/* get attitude */
		*roll = 0.01 * ssport->roll; 
		*pitch = 0.01 * ssport->pitch; 
		*heave = ssport->heaveCompensation 
				* ssport->sampleInterval * 0.00000075; 

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
int mbsys_jstar_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading, double draft, 
		double roll, double pitch, double heave,
		int *error)
{
	char	*function_name = "mbsys_jstar_insert_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_jstar_struct *store;
	struct mbsys_jstar_channel_struct *sbp;
	struct mbsys_jstar_channel_struct *ssport;
	struct mbsys_jstar_channel_struct *ssstbd;
	int	kind;
	int	time_j[5];
	int	id;
	unsigned short *short_ptr;
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
	store = (struct mbsys_jstar_struct *) store_ptr;

	/* insert subbottom data into structure */
	if (kind == MB_DATA_SUBBOTTOM_SUBBOTTOM)
		{
		/* get channel */
		sbp = (struct mbsys_jstar_channel_struct *) &(store->sbp);
		
		/* set kind and subsystem */
		store->kind = MB_DATA_SUBBOTTOM_SUBBOTTOM;
		store->subsystem = MBSYS_JSTAR_SUBSYSTEM_SBP;

		/* get time */
		mb_get_jtime(verbose,time_i,time_j);
		sbp->year = time_i[0];
		sbp->day = time_j[1];
		sbp->hour = time_i[3];
		sbp->minute = time_i[4];
		sbp->second = time_i[5];
		sbp->millisecondsToday = 0.001 * time_i[6]
					+ 1000 * (time_i[5] 
						+ 60.0 * (time_i[4] 
							+ 60.0 * time_i[3]));

		/* get navigation */
		if (navlon < 180.0) navlon = navlon + 360.0;
		if (navlon > 180.0) navlon = navlon - 360.0;
		sbp->sourceCoordX = (int) (360000.0 * navlon);
		sbp->sourceCoordY = (int) (360000.0 * navlat);

		/* get heading */
		sbp->heading = (short) (100.0 * heading);
	
		/* get draft */
		sbp->startDepth = draft /
				sbp->sampleInterval / 0.00000075; 
		sbp->sonardepth = 1000 * draft;

		/* get attitude */
		sbp->roll = 0.01 * roll; 
		sbp->pitch = 0.01 * pitch; 
		sbp->heaveCompensation = heave /
				sbp->sampleInterval / 0.00000075; 
		
		}

	/* insert data in structure */
	else if (store->kind == MB_DATA_DATA)
		{
		/* get channels */
		ssport = (struct mbsys_jstar_channel_struct *) &(store->ssport);
		ssstbd = (struct mbsys_jstar_channel_struct *) &(store->ssstbd);
		
		
		/* set kind and subsystem */
		store->kind = MB_DATA_DATA;
		if (ssport->message.subsystem == MBSYS_JSTAR_SUBSYSTEM_SSLOW
			&& ssstbd->message.subsystem == MBSYS_JSTAR_SUBSYSTEM_SSLOW)
			{
			store->subsystem = MBSYS_JSTAR_SUBSYSTEM_SSLOW;
			}
		else if (ssport->message.subsystem == MBSYS_JSTAR_SUBSYSTEM_SSHIGH
			&& ssstbd->message.subsystem == MBSYS_JSTAR_SUBSYSTEM_SSHIGH)
			{
			store->subsystem = MBSYS_JSTAR_SUBSYSTEM_SSHIGH;
			}
		else if (store->subsystem != MBSYS_JSTAR_SUBSYSTEM_SSHIGH)
			{
			store->subsystem = MBSYS_JSTAR_SUBSYSTEM_SSLOW;
			ssport->message.subsystem = MBSYS_JSTAR_SUBSYSTEM_SSLOW;
			ssstbd->message.subsystem = MBSYS_JSTAR_SUBSYSTEM_SSLOW;
			}
		else
			{
			ssport->message.subsystem = MBSYS_JSTAR_SUBSYSTEM_SSHIGH;
			ssstbd->message.subsystem = MBSYS_JSTAR_SUBSYSTEM_SSHIGH;
			}
		
		/* get time */
		mb_get_jtime(verbose,time_i,time_j);
		ssport->year = time_i[0];
		ssport->day = time_j[1];
		ssport->hour = time_i[3];
		ssport->minute = time_i[4];
		ssport->second = time_i[5];
		ssport->millisecondsToday = 0.001 * time_i[6]
					+ 1000 * (time_i[5] 
						+ 60.0 * (time_i[4] 
							+ 60.0 * time_i[3]));
		ssstbd->year = time_i[0];
		ssstbd->day = time_j[1];
		ssstbd->hour = time_i[3];
		ssstbd->minute = time_i[4];
		ssstbd->second = time_i[5];
		ssstbd->millisecondsToday = 0.001 * time_i[6]
					+ 1000 * (time_i[5] 
						+ 60.0 * (time_i[4] 
							+ 60.0 * time_i[3]));

		/* get navigation */
		if (navlon < 180.0) navlon = navlon + 360.0;
		if (navlon > 180.0) navlon = navlon - 360.0;
		ssport->sourceCoordX = 360000.0 * navlon;
		ssport->sourceCoordY = 360000.0 * navlat;
		ssstbd->sourceCoordX = 360000.0 * navlon;
		ssstbd->sourceCoordY = 360000.0 * navlat;

		/* get heading and speed */
		ssport->heading = (short) (100.0 * heading);
		ssstbd->heading = (short) (100.0 * heading);
	
		/* get draft */
		ssport->startDepth = draft /
				ssport->sampleInterval / 0.00000075; 
		ssstbd->startDepth = draft /
				ssstbd->sampleInterval / 0.00000075; 
		ssport->sonardepth = 1000 * draft;
		ssstbd->sonardepth = 1000 * draft;
		
		/* get attitude */
		ssport->roll = 0.01 * roll; 
		ssport->pitch = 0.01 * pitch; 
		ssport->heaveCompensation = heave /
				ssport->sampleInterval / 0.00000075; 
		ssstbd->roll = 0.01 * roll; 
		ssstbd->pitch = 0.01 * pitch; 
		ssstbd->heaveCompensation = heave /
				ssstbd->sampleInterval / 0.00000075; 
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
int mbsys_jstar_extract_segytraceheader(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, void *segytraceheader_ptr, int *error)
{
	char	*function_name = "mbsys_jstar_extract_segytraceheader";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_jstar_struct *store;
	struct mbsys_jstar_channel_struct *sbp;
	struct mb_segytraceheader_struct *mb_segytraceheader_ptr;
	double	dsonardepth, dsonaraltitude, dwaterdepth;
	int	sonardepth, sonaraltitude, waterdepth;
	int	watersoundspeed;
	float	fwatertime;
	double	longitude, latitude;
	double	speed;
	int	time_i[7],time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:        %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:         %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:      %d\n",store_ptr);
		fprintf(stderr,"dbg2       kind:           %d\n",kind);
		fprintf(stderr,"dbg2       segytraceheader_ptr: %d\n",segytraceheader_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_jstar_struct *) store_ptr;
	mb_segytraceheader_ptr = (struct mb_segytraceheader_struct *) segytraceheader_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_SUBBOTTOM_SUBBOTTOM)
		{
		/* get channel */
		sbp = (struct mbsys_jstar_channel_struct *) &(store->sbp);

		/* get time */
		time_j[0] = sbp->year;
		time_j[1] = sbp->day;
		time_j[2] = 60 * sbp->hour + sbp->minute;
		time_j[3] = sbp->second;
		time_j[4] = (int)1000 * (sbp->millisecondsToday 
				- 1000 * floor(0.001 * ((double)sbp->millisecondsToday)));
		mb_get_itime(verbose,time_j,time_i);
		
		/* get needed values */
		/* get transducer_depth */
		if (sbp->sonardepth > 0)
			dsonardepth = 0.001 * sbp->sonardepth
						+ sbp->heaveCompensation * sbp->sampleInterval * 0.00000075;
		else
			dsonardepth = sbp->startDepth * sbp->sampleInterval * 0.00000075
						+ sbp->heaveCompensation * sbp->sampleInterval * 0.00000075; 
		dsonaraltitude = 0.001 * sbp->sonaraltitude;
		if (sbp->depth > 0)
			dwaterdepth = 0.001 * sbp->depth;
		else
			dwaterdepth = dsonardepth + dsonaraltitude;
		sonardepth = (int) (100 * dsonardepth);
		waterdepth = (int) (100 * dwaterdepth);
		watersoundspeed = 1500;
		fwatertime = 2.0 * dwaterdepth / ((double) watersoundspeed);

		/* get navigation */
		longitude = sbp->sourceCoordX / 360000.0;
		latitude = sbp->sourceCoordY / 360000.0;
			
		/* extract the data */
		mb_segytraceheader_ptr->seq_num 	= sbp->pingNum;
		mb_segytraceheader_ptr->seq_reel 	= sbp->pingNum;
		mb_segytraceheader_ptr->shot_num 	= sbp->pingNum;
		mb_segytraceheader_ptr->shot_tr		= 1;
		mb_segytraceheader_ptr->espn		= 0;
		mb_segytraceheader_ptr->rp_num		= sbp->pingNum;
		mb_segytraceheader_ptr->rp_tr		= 1;
		mb_segytraceheader_ptr->trc_id		= 1;
		mb_segytraceheader_ptr->num_vstk	= 0;
		mb_segytraceheader_ptr->cdp_fold	= 0;
		mb_segytraceheader_ptr->use		= sbp->dataFormat;
		mb_segytraceheader_ptr->range		= 0;
		mb_segytraceheader_ptr->grp_elev	= -sonardepth;
		mb_segytraceheader_ptr->src_elev	= -sonardepth;
		mb_segytraceheader_ptr->src_depth	= sonardepth;
		mb_segytraceheader_ptr->grp_datum	= 0;
		mb_segytraceheader_ptr->src_datum	= 0;
		mb_segytraceheader_ptr->src_wbd		= waterdepth;
		mb_segytraceheader_ptr->grp_wbd		= waterdepth;
		mb_segytraceheader_ptr->elev_scalar	= -100; 	/* 0.01 m precision for depths */
		mb_segytraceheader_ptr->coord_scalar	= -100;		/* 0.01 arc second precision for position
									= 0.3 m precision at equator */
		mb_segytraceheader_ptr->src_long	= (int)(longitude * 360000.0);
		mb_segytraceheader_ptr->src_lat		= (int)(latitude * 360000.0);
		mb_segytraceheader_ptr->grp_long	= (int)(longitude * 360000.0);
		mb_segytraceheader_ptr->grp_lat		= (int)(latitude * 360000.0);
		mb_segytraceheader_ptr->coord_units	= 2;
		mb_segytraceheader_ptr->wvel		= watersoundspeed;
		mb_segytraceheader_ptr->sbvel		= 0;
		mb_segytraceheader_ptr->src_up_vel	= 0;
		mb_segytraceheader_ptr->grp_up_vel	= 0;
		mb_segytraceheader_ptr->src_static	= 0;
		mb_segytraceheader_ptr->grp_static	= 0;
		mb_segytraceheader_ptr->tot_static	= 0;
		mb_segytraceheader_ptr->laga		= 0;
		mb_segytraceheader_ptr->delay_mils	= 0;
		mb_segytraceheader_ptr->smute_mils	= 0;
		mb_segytraceheader_ptr->emute_mils	= 0;
		mb_segytraceheader_ptr->nsamps		= sbp->samples;
		mb_segytraceheader_ptr->si_micros	= (short) (sbp->sampleInterval / 1000);
		for (i=0;i<19;i++)
			mb_segytraceheader_ptr->other_1[i]	= 0;
		mb_segytraceheader_ptr->year		= time_i[0];
		mb_segytraceheader_ptr->day_of_yr	= time_j[1];
		mb_segytraceheader_ptr->hour		= time_i[3];
		mb_segytraceheader_ptr->min		= time_i[4];
		mb_segytraceheader_ptr->sec		= time_i[5];
		mb_segytraceheader_ptr->mils		= time_i[6] / 1000;
		mb_segytraceheader_ptr->tr_weight	= 1;
		for (i=0;i<5;i++)
			mb_segytraceheader_ptr->other_2[i]	= 0;
		mb_segytraceheader_ptr->delay		= 0.0;
		mb_segytraceheader_ptr->smute_sec	= 0.0;
		mb_segytraceheader_ptr->emute_sec	= 0.0;
		mb_segytraceheader_ptr->si_secs		= 0.000000001 * ((float)sbp->sampleInterval);
		mb_segytraceheader_ptr->wbt_secs	= fwatertime;
		mb_segytraceheader_ptr->end_of_rp	= 0;
		mb_segytraceheader_ptr->dummy1		= 0.0;
		mb_segytraceheader_ptr->dummy2		= 0.0;
		mb_segytraceheader_ptr->dummy3		= 0.0;
		mb_segytraceheader_ptr->dummy4		= 0.0;
		mb_segytraceheader_ptr->dummy5		= 0.0;
		mb_segytraceheader_ptr->dummy6		= 0.0;
		mb_segytraceheader_ptr->dummy7		= 0.0;
		mb_segytraceheader_ptr->dummy8		= 0.0;
		mb_segytraceheader_ptr->dummy9		= 0.0;
		
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
		fprintf(stderr,"dbg2       seq_num:           %d\n",mb_segytraceheader_ptr->seq_num);
		fprintf(stderr,"dbg2       seq_reel:          %d\n",mb_segytraceheader_ptr->seq_reel);
		fprintf(stderr,"dbg2       shot_num:          %d\n",mb_segytraceheader_ptr->shot_num);
		fprintf(stderr,"dbg2       shot_tr:           %d\n",mb_segytraceheader_ptr->shot_tr);
		fprintf(stderr,"dbg2       espn:              %d\n",mb_segytraceheader_ptr->espn);
		fprintf(stderr,"dbg2       rp_num:            %d\n",mb_segytraceheader_ptr->rp_num);
		fprintf(stderr,"dbg2       rp_tr:             %d\n",mb_segytraceheader_ptr->rp_tr);
		fprintf(stderr,"dbg2       trc_id:            %d\n",mb_segytraceheader_ptr->trc_id);
		fprintf(stderr,"dbg2       num_vstk:          %d\n",mb_segytraceheader_ptr->num_vstk);
		fprintf(stderr,"dbg2       cdp_fold:          %d\n",mb_segytraceheader_ptr->cdp_fold);
		fprintf(stderr,"dbg2       use:               %d\n",mb_segytraceheader_ptr->use);
		fprintf(stderr,"dbg2       range:             %d\n",mb_segytraceheader_ptr->range);
		fprintf(stderr,"dbg2       grp_elev:          %d\n",mb_segytraceheader_ptr->grp_elev);
		fprintf(stderr,"dbg2       src_elev:          %d\n",mb_segytraceheader_ptr->src_elev);
		fprintf(stderr,"dbg2       src_depth:         %d\n",mb_segytraceheader_ptr->src_depth);
		fprintf(stderr,"dbg2       grp_datum:         %d\n",mb_segytraceheader_ptr->grp_datum);
		fprintf(stderr,"dbg2       src_datum:         %d\n",mb_segytraceheader_ptr->src_datum);
		fprintf(stderr,"dbg2       src_wbd:           %d\n",mb_segytraceheader_ptr->src_wbd);
		fprintf(stderr,"dbg2       grp_wbd:           %d\n",mb_segytraceheader_ptr->grp_wbd);
		fprintf(stderr,"dbg2       elev_scalar:       %d\n",mb_segytraceheader_ptr->elev_scalar);
		fprintf(stderr,"dbg2       coord_scalar:      %d\n",mb_segytraceheader_ptr->coord_scalar);
		fprintf(stderr,"dbg2       src_long:          %d\n",mb_segytraceheader_ptr->src_long);
		fprintf(stderr,"dbg2       src_lat:           %d\n",mb_segytraceheader_ptr->src_lat);
		fprintf(stderr,"dbg2       grp_long:          %d\n",mb_segytraceheader_ptr->grp_long);
		fprintf(stderr,"dbg2       grp_lat:           %d\n",mb_segytraceheader_ptr->grp_lat);
		fprintf(stderr,"dbg2       coord_units:       %d\n",mb_segytraceheader_ptr->coord_units);
		fprintf(stderr,"dbg2       wvel:              %d\n",mb_segytraceheader_ptr->wvel);
		fprintf(stderr,"dbg2       sbvel:             %d\n",mb_segytraceheader_ptr->sbvel);
		fprintf(stderr,"dbg2       src_up_vel:        %d\n",mb_segytraceheader_ptr->src_up_vel);
		fprintf(stderr,"dbg2       grp_up_vel:        %d\n",mb_segytraceheader_ptr->grp_up_vel);
		fprintf(stderr,"dbg2       src_static:        %d\n",mb_segytraceheader_ptr->src_static);
		fprintf(stderr,"dbg2       grp_static:        %d\n",mb_segytraceheader_ptr->grp_static);
		fprintf(stderr,"dbg2       tot_static:        %d\n",mb_segytraceheader_ptr->tot_static);
		fprintf(stderr,"dbg2       laga:              %d\n",mb_segytraceheader_ptr->laga);
		fprintf(stderr,"dbg2       delay_mils:        %d\n",mb_segytraceheader_ptr->delay_mils);
		fprintf(stderr,"dbg2       smute_mils:        %d\n",mb_segytraceheader_ptr->smute_mils);
		fprintf(stderr,"dbg2       emute_mils:        %d\n",mb_segytraceheader_ptr->emute_mils);
		fprintf(stderr,"dbg2       nsamps:            %d\n",mb_segytraceheader_ptr->nsamps);
		fprintf(stderr,"dbg2       si_micros:         %d\n",mb_segytraceheader_ptr->si_micros);
		for (i=0;i<19;i++)
		fprintf(stderr,"dbg2       other_1[%2d]:       %d\n",i,mb_segytraceheader_ptr->other_1[i]);
		fprintf(stderr,"dbg2       year:              %d\n",mb_segytraceheader_ptr->year);
		fprintf(stderr,"dbg2       day_of_yr:         %d\n",mb_segytraceheader_ptr->day_of_yr);
		fprintf(stderr,"dbg2       hour:              %d\n",mb_segytraceheader_ptr->hour);
		fprintf(stderr,"dbg2       min:               %d\n",mb_segytraceheader_ptr->min);
		fprintf(stderr,"dbg2       sec:               %d\n",mb_segytraceheader_ptr->sec);
		fprintf(stderr,"dbg2       mils:              %d\n",mb_segytraceheader_ptr->mils);
		fprintf(stderr,"dbg2       tr_weight:         %d\n",mb_segytraceheader_ptr->tr_weight);
		for (i=0;i<5;i++)
		fprintf(stderr,"dbg2       other_2[%2d]:       %d\n",i,mb_segytraceheader_ptr->other_2[i]);
		fprintf(stderr,"dbg2       delay:             %f\n",mb_segytraceheader_ptr->delay);
		fprintf(stderr,"dbg2       smute_sec:         %f\n",mb_segytraceheader_ptr->smute_sec);
		fprintf(stderr,"dbg2       emute_sec:         %f\n",mb_segytraceheader_ptr->emute_sec);
		fprintf(stderr,"dbg2       si_secs:           %f\n",mb_segytraceheader_ptr->si_secs);
		fprintf(stderr,"dbg2       wbt_secs:          %f\n",mb_segytraceheader_ptr->wbt_secs);
		fprintf(stderr,"dbg2       end_of_rp:         %d\n",mb_segytraceheader_ptr->end_of_rp);
		fprintf(stderr,"dbg2       dummy1:            %d\n",mb_segytraceheader_ptr->dummy1);
		fprintf(stderr,"dbg2       dummy2:            %d\n",mb_segytraceheader_ptr->dummy2);
		fprintf(stderr,"dbg2       dummy3:            %d\n",mb_segytraceheader_ptr->dummy3);
		fprintf(stderr,"dbg2       dummy4:            %d\n",mb_segytraceheader_ptr->dummy4);
		fprintf(stderr,"dbg2       dummy5:            %d\n",mb_segytraceheader_ptr->dummy5);
		fprintf(stderr,"dbg2       dummy6:            %d\n",mb_segytraceheader_ptr->dummy6);
		fprintf(stderr,"dbg2       dummy7:            %d\n",mb_segytraceheader_ptr->dummy7);
		fprintf(stderr,"dbg2       dummy8:            %d\n",mb_segytraceheader_ptr->dummy8);
		fprintf(stderr,"dbg2       dummy9:            %d\n",mb_segytraceheader_ptr->dummy9);
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_jstar_extract_segy(int verbose, void *mbio_ptr, void *store_ptr,
		int *sampleformat, int *kind, void *segyheader_ptr, float *segydata, int *error)
{
	char	*function_name = "mbsys_jstar_extract_segy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_jstar_struct *store;
	struct mbsys_jstar_channel_struct *sbp;
	struct mb_segytraceheader_struct *mb_segytraceheader_ptr;
	short	*shortptr;
	unsigned short	*ushortptr;
	double	weight;
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
		fprintf(stderr,"dbg2       sampleformat:      %d\n",sampleformat);
		fprintf(stderr,"dbg2       sampleformat:      %d\n",*sampleformat);
		fprintf(stderr,"dbg2       kind:              %d\n",kind);
		fprintf(stderr,"dbg2       segyheader_ptr:    %d\n",segyheader_ptr);
		fprintf(stderr,"dbg2       segydata:          %d\n",segydata);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_jstar_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_SUBBOTTOM_SUBBOTTOM)
		{
		/* get channel */
		sbp = (struct mbsys_jstar_channel_struct *) &(store->sbp);
		shortptr = (short *) sbp->trace;
		ushortptr = (unsigned short *) sbp->trace;
			
		/* extract segy header */
		status = mbsys_jstar_extract_segytraceheader(verbose, mbio_ptr, store_ptr,
								kind, segyheader_ptr, error);
								
		/* get the trace weight */
		weight = exp2((double)sbp->weightingFactor);
/*fprintf(stderr, "Subsystem: %d Weight: %d %f\n",sbp->message.subsystem,sbp->weightingFactor,weight);*/

		/* extract the data */
		if (sbp->dataFormat == MBSYS_JSTAR_TRACEFORMAT_ENVELOPE)
			{
			*sampleformat = MB_SEGY_SAMPLEFORMAT_ENVELOPE;
			for (i=0;i<sbp->samples;i++)
				{
				segydata[i] = (float) (((double)ushortptr[i]) / weight);
				}
			}
		else if (sbp->dataFormat == MBSYS_JSTAR_TRACEFORMAT_ANALYTIC)
			{
			/* if no format specified do envelope by default */
			if (*sampleformat == MB_SEGY_SAMPLEFORMAT_NONE)
				*sampleformat = MB_SEGY_SAMPLEFORMAT_ENVELOPE;
			
			/* convert analytic data to desired envelope */
			if (*sampleformat == MB_SEGY_SAMPLEFORMAT_ENVELOPE)
				{
				for (i=0;i<sbp->samples;i++)
					{
					segydata[i] = (float) (sqrt((double) (shortptr[2*i] * shortptr[2*i] 
								+ shortptr[2*i+1] * shortptr[2*i+1]))
								/ weight);
					}
				}
			
			/* else extract desired analytic data */
			else if (*sampleformat == MB_SEGY_SAMPLEFORMAT_ANALYTIC)
				{
				for (i=0;i<sbp->samples;i++)
					{
					segydata[2*i]   = (float) (((double)shortptr[2*i]) / weight);
					segydata[2*i+1] = (float) (((double)shortptr[2*i+1]) / weight);
					}
				}
			
			/* else extract desired real trace from analytic data */
			else if (*sampleformat == MB_SEGY_SAMPLEFORMAT_TRACE)
				{
				for (i=0;i<sbp->samples;i++)
					{
					segydata[i] = (float) (((double)shortptr[2*i]) / weight);
					}
				}
			}
		else if (sbp->dataFormat == MBSYS_JSTAR_TRACEFORMAT_RAW)
			{
			*sampleformat = MB_SEGY_SAMPLEFORMAT_TRACE;
			for (i=0;i<sbp->samples;i++)
				{
				segydata[i] = (float) (((double)ushortptr[i]) / weight);
				}
			}
		else if (sbp->dataFormat == MBSYS_JSTAR_TRACEFORMAT_REALANALYTIC)
			{
			*sampleformat = MB_SEGY_SAMPLEFORMAT_TRACE;
			for (i=0;i<sbp->samples;i++)
				{
				segydata[i] = (float) (((double)ushortptr[i]) / weight);
				}
			}
		else if (sbp->dataFormat == MBSYS_JSTAR_TRACEFORMAT_PIXEL)
			{
			*sampleformat = MB_SEGY_SAMPLEFORMAT_TRACE;
			for (i=0;i<sbp->samples;i++)
				{
				segydata[i] = (float) (((double)ushortptr[i]) / weight);
				}
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
		fprintf(stderr,"dbg2       sampleformat:      %d\n",*sampleformat);
		fprintf(stderr,"dbg2       kind:              %d\n",*kind);
		fprintf(stderr,"dbg2       seq_num:           %d\n",mb_segytraceheader_ptr->seq_num);
		fprintf(stderr,"dbg2       seq_reel:          %d\n",mb_segytraceheader_ptr->seq_reel);
		fprintf(stderr,"dbg2       shot_num:          %d\n",mb_segytraceheader_ptr->shot_num);
		fprintf(stderr,"dbg2       shot_tr:           %d\n",mb_segytraceheader_ptr->shot_tr);
		fprintf(stderr,"dbg2       espn:              %d\n",mb_segytraceheader_ptr->espn);
		fprintf(stderr,"dbg2       rp_num:            %d\n",mb_segytraceheader_ptr->rp_num);
		fprintf(stderr,"dbg2       rp_tr:             %d\n",mb_segytraceheader_ptr->rp_tr);
		fprintf(stderr,"dbg2       trc_id:            %d\n",mb_segytraceheader_ptr->trc_id);
		fprintf(stderr,"dbg2       num_vstk:          %d\n",mb_segytraceheader_ptr->num_vstk);
		fprintf(stderr,"dbg2       cdp_fold:          %d\n",mb_segytraceheader_ptr->cdp_fold);
		fprintf(stderr,"dbg2       use:               %d\n",mb_segytraceheader_ptr->use);
		fprintf(stderr,"dbg2       range:             %d\n",mb_segytraceheader_ptr->range);
		fprintf(stderr,"dbg2       grp_elev:          %d\n",mb_segytraceheader_ptr->grp_elev);
		fprintf(stderr,"dbg2       src_elev:          %d\n",mb_segytraceheader_ptr->src_elev);
		fprintf(stderr,"dbg2       src_depth:         %d\n",mb_segytraceheader_ptr->src_depth);
		fprintf(stderr,"dbg2       grp_datum:         %d\n",mb_segytraceheader_ptr->grp_datum);
		fprintf(stderr,"dbg2       src_datum:         %d\n",mb_segytraceheader_ptr->src_datum);
		fprintf(stderr,"dbg2       src_wbd:           %d\n",mb_segytraceheader_ptr->src_wbd);
		fprintf(stderr,"dbg2       grp_wbd:           %d\n",mb_segytraceheader_ptr->grp_wbd);
		fprintf(stderr,"dbg2       elev_scalar:       %d\n",mb_segytraceheader_ptr->elev_scalar);
		fprintf(stderr,"dbg2       coord_scalar:      %d\n",mb_segytraceheader_ptr->coord_scalar);
		fprintf(stderr,"dbg2       src_long:          %d\n",mb_segytraceheader_ptr->src_long);
		fprintf(stderr,"dbg2       src_lat:           %d\n",mb_segytraceheader_ptr->src_lat);
		fprintf(stderr,"dbg2       grp_long:          %d\n",mb_segytraceheader_ptr->grp_long);
		fprintf(stderr,"dbg2       grp_lat:           %d\n",mb_segytraceheader_ptr->grp_lat);
		fprintf(stderr,"dbg2       coord_units:       %d\n",mb_segytraceheader_ptr->coord_units);
		fprintf(stderr,"dbg2       wvel:              %d\n",mb_segytraceheader_ptr->wvel);
		fprintf(stderr,"dbg2       sbvel:             %d\n",mb_segytraceheader_ptr->sbvel);
		fprintf(stderr,"dbg2       src_up_vel:        %d\n",mb_segytraceheader_ptr->src_up_vel);
		fprintf(stderr,"dbg2       grp_up_vel:        %d\n",mb_segytraceheader_ptr->grp_up_vel);
		fprintf(stderr,"dbg2       src_static:        %d\n",mb_segytraceheader_ptr->src_static);
		fprintf(stderr,"dbg2       grp_static:        %d\n",mb_segytraceheader_ptr->grp_static);
		fprintf(stderr,"dbg2       tot_static:        %d\n",mb_segytraceheader_ptr->tot_static);
		fprintf(stderr,"dbg2       laga:              %d\n",mb_segytraceheader_ptr->laga);
		fprintf(stderr,"dbg2       delay_mils:        %d\n",mb_segytraceheader_ptr->delay_mils);
		fprintf(stderr,"dbg2       smute_mils:        %d\n",mb_segytraceheader_ptr->smute_mils);
		fprintf(stderr,"dbg2       emute_mils:        %d\n",mb_segytraceheader_ptr->emute_mils);
		fprintf(stderr,"dbg2       nsamps:            %d\n",mb_segytraceheader_ptr->nsamps);
		fprintf(stderr,"dbg2       si_micros:         %d\n",mb_segytraceheader_ptr->si_micros);
		for (i=0;i<19;i++)
		fprintf(stderr,"dbg2       other_1[%2d]:       %d\n",i,mb_segytraceheader_ptr->other_1[i]);
		fprintf(stderr,"dbg2       year:              %d\n",mb_segytraceheader_ptr->year);
		fprintf(stderr,"dbg2       day_of_yr:         %d\n",mb_segytraceheader_ptr->day_of_yr);
		fprintf(stderr,"dbg2       hour:              %d\n",mb_segytraceheader_ptr->hour);
		fprintf(stderr,"dbg2       min:               %d\n",mb_segytraceheader_ptr->min);
		fprintf(stderr,"dbg2       sec:               %d\n",mb_segytraceheader_ptr->sec);
		fprintf(stderr,"dbg2       mils:              %d\n",mb_segytraceheader_ptr->mils);
		fprintf(stderr,"dbg2       tr_weight:         %d\n",mb_segytraceheader_ptr->tr_weight);
		for (i=0;i<5;i++)
		fprintf(stderr,"dbg2       other_2[%2d]:       %d\n",i,mb_segytraceheader_ptr->other_2[i]);
		fprintf(stderr,"dbg2       delay:             %f\n",mb_segytraceheader_ptr->delay);
		fprintf(stderr,"dbg2       smute_sec:         %f\n",mb_segytraceheader_ptr->smute_sec);
		fprintf(stderr,"dbg2       emute_sec:         %f\n",mb_segytraceheader_ptr->emute_sec);
		fprintf(stderr,"dbg2       si_secs:           %f\n",mb_segytraceheader_ptr->si_secs);
		fprintf(stderr,"dbg2       wbt_secs:          %f\n",mb_segytraceheader_ptr->wbt_secs);
		fprintf(stderr,"dbg2       end_of_rp:         %d\n",mb_segytraceheader_ptr->end_of_rp);
		fprintf(stderr,"dbg2       dummy1:            %d\n",mb_segytraceheader_ptr->dummy1);
		fprintf(stderr,"dbg2       dummy2:            %d\n",mb_segytraceheader_ptr->dummy2);
		fprintf(stderr,"dbg2       dummy3:            %d\n",mb_segytraceheader_ptr->dummy3);
		fprintf(stderr,"dbg2       dummy4:            %d\n",mb_segytraceheader_ptr->dummy4);
		fprintf(stderr,"dbg2       dummy5:            %d\n",mb_segytraceheader_ptr->dummy5);
		fprintf(stderr,"dbg2       dummy6:            %d\n",mb_segytraceheader_ptr->dummy6);
		fprintf(stderr,"dbg2       dummy7:            %d\n",mb_segytraceheader_ptr->dummy7);
		fprintf(stderr,"dbg2       dummy8:            %d\n",mb_segytraceheader_ptr->dummy8);
		fprintf(stderr,"dbg2       dummy9:            %d\n",mb_segytraceheader_ptr->dummy9);
		for (i=0;i<mb_segytraceheader_ptr->nsamps;i++)
			fprintf(stderr,"dbg2       segydata[%d]:      %f\n",i,segydata[i]);
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_jstar_insert_segy(int verbose, void *mbio_ptr, void *store_ptr,
		int kind, void *segyheader_ptr, float *segydata, int *error)
{
	char	*function_name = "mbsys_jstar_insert_segy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_jstar_struct *store;
	struct mbsys_jstar_channel_struct *sbp;
	struct mb_segytraceheader_struct *mb_segytraceheader_ptr;
	double	dsonardepth, dsonaraltitude, dwaterdepth;
	int	sonardepth, sonaraltitude, waterdepth;
	int	watersoundspeed;
	float	fwatertime;
	int	time_i[7];
	int	time_j[5];
	float	factor;
	float	datamax;
	double	weight;
	int	data_size;
	short	*shortptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:        %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:         %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:      %d\n",store_ptr);
		fprintf(stderr,"dbg2       kind:           %d\n",kind);
		fprintf(stderr,"dbg2       segyheader_ptr: %d\n",segyheader_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_jstar_struct *) store_ptr;

	/* get data kind */
	store->kind = kind;

	/* insert data to structure */
	if (store->kind == MB_DATA_SUBBOTTOM_SUBBOTTOM)
		{
		/* get channel */
		sbp = (struct mbsys_jstar_channel_struct *) &(store->sbp);
		shortptr = (short *) sbp->trace;
			
		/* extract the data */
		if (mb_segytraceheader_ptr->shot_num != 0)
			sbp->pingNum = mb_segytraceheader_ptr->shot_num;
		else if (mb_segytraceheader_ptr->seq_reel != 0)
			sbp->pingNum = mb_segytraceheader_ptr->seq_reel;
		else if (mb_segytraceheader_ptr->seq_num != 0)
			sbp->pingNum = mb_segytraceheader_ptr->seq_num;
		else if (mb_segytraceheader_ptr->rp_num != 0)
			sbp->pingNum = mb_segytraceheader_ptr->rp_num;
		else
			sbp->pingNum = 0;
		sbp->dataFormat = mb_segytraceheader_ptr->use;
		if (mb_segytraceheader_ptr->grp_elev != 0)
			sonardepth = -mb_segytraceheader_ptr->grp_elev;
		else if (mb_segytraceheader_ptr->src_elev != 0)
			sonardepth = -mb_segytraceheader_ptr->src_elev;
		else if (mb_segytraceheader_ptr->src_depth != 0)
			sonardepth = mb_segytraceheader_ptr->src_depth;
		else
			sonardepth = 0;
		if (mb_segytraceheader_ptr->elev_scalar < 0)
			factor = 1.0 / ((float) (-mb_segytraceheader_ptr->elev_scalar));
		else
			factor = (float) mb_segytraceheader_ptr->elev_scalar;
		if (mb_segytraceheader_ptr->src_wbd != 0)
			waterdepth = -mb_segytraceheader_ptr->grp_elev;
		else if (mb_segytraceheader_ptr->grp_wbd != 0)
			waterdepth = -mb_segytraceheader_ptr->src_elev;
		else
			waterdepth = 0;
		if (mb_segytraceheader_ptr->coord_scalar < 0)
			factor = 1.0 / ((float) (-mb_segytraceheader_ptr->coord_scalar)) / 3600.0;
		else
			factor = (float) mb_segytraceheader_ptr->coord_scalar / 3600.0;
		sbp->samples = mb_segytraceheader_ptr->nsamps;
		sbp->sampleInterval = 1000 * mb_segytraceheader_ptr->si_micros;
		time_j[0] = mb_segytraceheader_ptr->year;
		time_j[1] = mb_segytraceheader_ptr->day_of_yr;
		time_j[2] = 60 * mb_segytraceheader_ptr->hour + mb_segytraceheader_ptr->min;
		time_j[3] = mb_segytraceheader_ptr->sec;
		time_j[4] = 1000 * mb_segytraceheader_ptr->mils;
		mb_get_itime(verbose,time_j,time_i);
		sbp->year = time_j[0];
		sbp->day = time_j[1];
		sbp->second = 0.000001 * time_i[6] + time_i[5];
		sbp->hour = time_i[3];
		sbp->minute = time_i[4];
		sbp->millisecondsToday = 0.001 * time_i[6]
					+ 1000 * (time_i[5] 
						+ 60.0 * (time_i[4] 
							+ 60.0 * time_i[3]));

		sbp->depth = 1000 * waterdepth;
		sbp->sonardepth = 1000 * sonardepth;
		sbp->sonaraltitude = 1000 * (waterdepth - sonardepth);
		if (sbp->sonaraltitude < 0)
			sbp->sonaraltitude = 0;
		
		/* get max data value */
		datamax = 0.0;
		for (i=0;i<mb_segytraceheader_ptr->nsamps;i++)
			{
			if (fabs(segydata[i]) > datamax)
				datamax = fabs(segydata[i]);
			}
		if (datamax > 0.0)
			{
			sbp->weightingFactor = (short) log2(datamax) - 15;
			}
		else
			sbp->weightingFactor = 0;
		weight = pow(2.0, (double)sbp->weightingFactor);
		
		/* make sure enough memory is allocated for channel data */
		data_size = sizeof(short) * sbp->samples;
		if (sbp->trace_alloc < data_size)
			{
			status = mb_realloc(verbose, data_size, &(sbp->trace), error);
			if (status == MB_SUCCESS)
				{
				sbp->trace_alloc = data_size;
				}
			else
				{
				sbp->trace_alloc = 0;
				sbp->samples = 0;
				}
			}

		/* copy over the data */
		if (sbp->trace_alloc >= data_size)
			{
			shortptr = (short *) sbp->trace;
			for (i=0;i<sbp->samples;i++)
				{
				shortptr[i] = (short) (segydata[i] * weight);
				}
			}
		
		/* done translating values */

		}

	/* deal with comment */
	else if (kind == MB_DATA_COMMENT)
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
		fprintf(stderr,"dbg2       seq_num:           %d\n",mb_segytraceheader_ptr->seq_num);
		fprintf(stderr,"dbg2       seq_reel:          %d\n",mb_segytraceheader_ptr->seq_reel);
		fprintf(stderr,"dbg2       shot_num:          %d\n",mb_segytraceheader_ptr->shot_num);
		fprintf(stderr,"dbg2       shot_tr:           %d\n",mb_segytraceheader_ptr->shot_tr);
		fprintf(stderr,"dbg2       espn:              %d\n",mb_segytraceheader_ptr->espn);
		fprintf(stderr,"dbg2       rp_num:            %d\n",mb_segytraceheader_ptr->rp_num);
		fprintf(stderr,"dbg2       rp_tr:             %d\n",mb_segytraceheader_ptr->rp_tr);
		fprintf(stderr,"dbg2       trc_id:            %d\n",mb_segytraceheader_ptr->trc_id);
		fprintf(stderr,"dbg2       num_vstk:          %d\n",mb_segytraceheader_ptr->num_vstk);
		fprintf(stderr,"dbg2       cdp_fold:          %d\n",mb_segytraceheader_ptr->cdp_fold);
		fprintf(stderr,"dbg2       use:               %d\n",mb_segytraceheader_ptr->use);
		fprintf(stderr,"dbg2       range:             %d\n",mb_segytraceheader_ptr->range);
		fprintf(stderr,"dbg2       grp_elev:          %d\n",mb_segytraceheader_ptr->grp_elev);
		fprintf(stderr,"dbg2       src_elev:          %d\n",mb_segytraceheader_ptr->src_elev);
		fprintf(stderr,"dbg2       src_depth:         %d\n",mb_segytraceheader_ptr->src_depth);
		fprintf(stderr,"dbg2       grp_datum:         %d\n",mb_segytraceheader_ptr->grp_datum);
		fprintf(stderr,"dbg2       src_datum:         %d\n",mb_segytraceheader_ptr->src_datum);
		fprintf(stderr,"dbg2       src_wbd:           %d\n",mb_segytraceheader_ptr->src_wbd);
		fprintf(stderr,"dbg2       grp_wbd:           %d\n",mb_segytraceheader_ptr->grp_wbd);
		fprintf(stderr,"dbg2       elev_scalar:       %d\n",mb_segytraceheader_ptr->elev_scalar);
		fprintf(stderr,"dbg2       coord_scalar:      %d\n",mb_segytraceheader_ptr->coord_scalar);
		fprintf(stderr,"dbg2       src_long:          %d\n",mb_segytraceheader_ptr->src_long);
		fprintf(stderr,"dbg2       src_lat:           %d\n",mb_segytraceheader_ptr->src_lat);
		fprintf(stderr,"dbg2       grp_long:          %d\n",mb_segytraceheader_ptr->grp_long);
		fprintf(stderr,"dbg2       grp_lat:           %d\n",mb_segytraceheader_ptr->grp_lat);
		fprintf(stderr,"dbg2       coord_units:       %d\n",mb_segytraceheader_ptr->coord_units);
		fprintf(stderr,"dbg2       wvel:              %d\n",mb_segytraceheader_ptr->wvel);
		fprintf(stderr,"dbg2       sbvel:             %d\n",mb_segytraceheader_ptr->sbvel);
		fprintf(stderr,"dbg2       src_up_vel:        %d\n",mb_segytraceheader_ptr->src_up_vel);
		fprintf(stderr,"dbg2       grp_up_vel:        %d\n",mb_segytraceheader_ptr->grp_up_vel);
		fprintf(stderr,"dbg2       src_static:        %d\n",mb_segytraceheader_ptr->src_static);
		fprintf(stderr,"dbg2       grp_static:        %d\n",mb_segytraceheader_ptr->grp_static);
		fprintf(stderr,"dbg2       tot_static:        %d\n",mb_segytraceheader_ptr->tot_static);
		fprintf(stderr,"dbg2       laga:              %d\n",mb_segytraceheader_ptr->laga);
		fprintf(stderr,"dbg2       delay_mils:        %d\n",mb_segytraceheader_ptr->delay_mils);
		fprintf(stderr,"dbg2       smute_mils:        %d\n",mb_segytraceheader_ptr->smute_mils);
		fprintf(stderr,"dbg2       emute_mils:        %d\n",mb_segytraceheader_ptr->emute_mils);
		fprintf(stderr,"dbg2       nsamps:            %d\n",mb_segytraceheader_ptr->nsamps);
		fprintf(stderr,"dbg2       si_micros:         %d\n",mb_segytraceheader_ptr->si_micros);
		for (i=0;i<19;i++)
		fprintf(stderr,"dbg2       other_1[%2d]:       %d\n",i,mb_segytraceheader_ptr->other_1[i]);
		fprintf(stderr,"dbg2       year:              %d\n",mb_segytraceheader_ptr->year);
		fprintf(stderr,"dbg2       day_of_yr:         %d\n",mb_segytraceheader_ptr->day_of_yr);
		fprintf(stderr,"dbg2       hour:              %d\n",mb_segytraceheader_ptr->hour);
		fprintf(stderr,"dbg2       min:               %d\n",mb_segytraceheader_ptr->min);
		fprintf(stderr,"dbg2       sec:               %d\n",mb_segytraceheader_ptr->sec);
		fprintf(stderr,"dbg2       mils:              %d\n",mb_segytraceheader_ptr->mils);
		fprintf(stderr,"dbg2       tr_weight:         %d\n",mb_segytraceheader_ptr->tr_weight);
		for (i=0;i<5;i++)
		fprintf(stderr,"dbg2       other_2[%2d]:       %d\n",i,mb_segytraceheader_ptr->other_2[i]);
		fprintf(stderr,"dbg2       delay:             %f\n",mb_segytraceheader_ptr->delay);
		fprintf(stderr,"dbg2       smute_sec:         %f\n",mb_segytraceheader_ptr->smute_sec);
		fprintf(stderr,"dbg2       emute_sec:         %f\n",mb_segytraceheader_ptr->emute_sec);
		fprintf(stderr,"dbg2       si_secs:           %f\n",mb_segytraceheader_ptr->si_secs);
		fprintf(stderr,"dbg2       wbt_secs:          %f\n",mb_segytraceheader_ptr->wbt_secs);
		fprintf(stderr,"dbg2       end_of_rp:         %d\n",mb_segytraceheader_ptr->end_of_rp);
		fprintf(stderr,"dbg2       dummy1:            %d\n",mb_segytraceheader_ptr->dummy1);
		fprintf(stderr,"dbg2       dummy2:            %d\n",mb_segytraceheader_ptr->dummy2);
		fprintf(stderr,"dbg2       dummy3:            %d\n",mb_segytraceheader_ptr->dummy3);
		fprintf(stderr,"dbg2       dummy4:            %d\n",mb_segytraceheader_ptr->dummy4);
		fprintf(stderr,"dbg2       dummy5:            %d\n",mb_segytraceheader_ptr->dummy5);
		fprintf(stderr,"dbg2       dummy6:            %d\n",mb_segytraceheader_ptr->dummy6);
		fprintf(stderr,"dbg2       dummy7:            %d\n",mb_segytraceheader_ptr->dummy7);
		fprintf(stderr,"dbg2       dummy8:            %d\n",mb_segytraceheader_ptr->dummy8);
		fprintf(stderr,"dbg2       dummy9:            %d\n",mb_segytraceheader_ptr->dummy9);
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_jstar_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error)
{
	char	*function_name = "mbsys_jstar_copy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_jstar_struct *store;
	struct mbsys_jstar_struct *copy;
	unsigned int	sbp_trace_alloc;
	unsigned short	*sbp_trace;
	unsigned int	ssport_trace_alloc;
	unsigned short	*ssport_trace;
	unsigned int	ssstbd_trace_alloc;
	unsigned short	*ssstbd_trace;
	int	shortspersample;
	int	trace_size;
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
		fprintf(stderr,"dbg2       copy_ptr:   %d\n",copy_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointers */
	store = (struct mbsys_jstar_struct *) store_ptr;
	copy = (struct mbsys_jstar_struct *) copy_ptr;
	
	/* save existing trace pointers in copy */
	sbp_trace_alloc = copy->sbp.trace_alloc;
	sbp_trace = copy->sbp.trace;
	ssport_trace_alloc = copy->ssport.trace_alloc;
	ssport_trace = copy->ssport.trace;
	ssstbd_trace_alloc = copy->ssstbd.trace_alloc;
	ssstbd_trace = copy->ssstbd.trace;

	/* copy the data */
	*copy = *store;
	
	/* restore the original trace pointers */
	copy->sbp.trace_alloc = sbp_trace_alloc;
	copy->sbp.trace = sbp_trace;
	copy->ssport.trace_alloc = ssport_trace_alloc;
	copy->ssport.trace = ssport_trace;
	copy->ssstbd.trace_alloc = ssstbd_trace_alloc;
	copy->ssstbd.trace = ssstbd_trace;
	
	/* allocate memory and copy each trace */
					
	/* allocate memory for the subbottom trace */
	if (copy->sbp.dataFormat == 1)
		shortspersample = 2;
	else
		shortspersample = 1;
	trace_size = shortspersample * copy->sbp.samples * sizeof(short);
	if (copy->sbp.trace_alloc < trace_size)
		{
		if ((status = mb_realloc(verbose, trace_size, &(copy->sbp.trace), error))
			== MB_SUCCESS)
			{
			copy->sbp.trace_alloc = trace_size;
			}
		}
	if (copy->sbp.trace_alloc >= trace_size)
		{
		for (i=0;i<shortspersample * copy->sbp.samples;i++)
			{
			copy->sbp.trace[i] = store->sbp.trace[i];
			}
		}
					
	/* allocate memory for the port sidescan trace */
	if (copy->ssport.dataFormat == 1)
		shortspersample = 2;
	else
		shortspersample = 1;
	trace_size = shortspersample * copy->ssport.samples * sizeof(short);
	if (copy->ssport.trace_alloc < trace_size)
		{
		if ((status = mb_realloc(verbose, trace_size, &(copy->ssport.trace), error))
			== MB_SUCCESS)
			{
			copy->ssport.trace_alloc = trace_size;
			}
		}
	if (copy->ssport.trace_alloc >= trace_size)
		{
		for (i=0;i<shortspersample * copy->ssport.samples;i++)
			{
			copy->ssport.trace[i] = store->ssport.trace[i];
			}
		}
					
	/* allocate memory for the starboard sidescan trace */
	if (copy->ssstbd.dataFormat == 1)
		shortspersample = 2;
	else
		shortspersample = 1;
	trace_size = shortspersample * copy->ssstbd.samples * sizeof(short);
	if (copy->ssstbd.trace_alloc < trace_size)
		{
		if ((status = mb_realloc(verbose, trace_size, &(copy->ssstbd.trace), error))
			== MB_SUCCESS)
			{
			copy->ssstbd.trace_alloc = trace_size;
			}
		}
	if (copy->ssstbd.trace_alloc >= trace_size)
		{
		for (i=0;i<shortspersample * copy->ssstbd.samples;i++)
			{
			copy->ssstbd.trace[i] = store->ssstbd.trace[i];
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

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/

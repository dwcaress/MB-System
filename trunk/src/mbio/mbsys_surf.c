/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_surf.c	3.00	6/25/01
 *	$Id$
 *
 *    Copyright (c) 2001-2015 by
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
 * mbsys_surf.c contains the MBIO functions for handling data in
 * the SURF format from STN Atlas Marine Electronics.
 * The relevant sonars include all SAM Electronics swath mapping
 * sonars: Hydrosweep DS, Hydrosweep DS2, Hydrosweep MD,
 * Hydrosweep MD2, Fansweep 10, Fansweep 20.
 * The data format associated with SURF data is:
 *    MBSYS_SURF format (code in mbsys_surf.c and mbsys_surf.h):
 *      MBF_SAMESURF : MBIO ID 181 - Vendor processing format
 *
 * Author:	D. W. Caress
 * Author:	D. N. Chayes
 * Date:	June 20, 2002
 *
 * $Log: mbsys_surf.c,v $
 * Revision 5.15  2005/11/05 00:48:04  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.14  2003/12/04 23:10:24  caress
 * Fixed problems with format 54 EM12DARW due to old code assuming how internal structure was packed. Also changed handling of beamflags for formats that don't support beamflags. Now flagged beams will always be nulled in such cases.
 *
 * Revision 5.13  2003/11/24 21:09:09  caress
 * Implement Reinhard Holtkamp's suggested mods for better SURF format support.
 *
 * Revision 5.12  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.11  2003/03/06 00:14:52  caress
 * Put in Reinhard Holtkamp's mod's to support SVP data.
 *
 * Revision 5.10  2003/02/27 04:33:33  caress
 * Fixed handling of SURF format data.
 *
 * Revision 5.9  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.8  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 *
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"
#include "../surf/mb_sapi.h"
#include "mbsys_surf.h"

double mbsys_get_depth(	SurfMultiBeamDepth* 			MultiBeamDepth,
						SurfTransducerParameterTable	TransducerTable,
						float	heave,
						int		n );

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbsys_surf_alloc(int verbose, void *mbio_ptr, void **store_ptr,
			int *error)
{
	char	*function_name = "mbsys_surf_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_surf_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* allocate memory for data structure */
	status = mb_malloc(verbose,sizeof(struct mbsys_surf_struct),
				store_ptr,error);
	if (status == MB_SUCCESS)
		{
		/* initialize everything */
		memset(*store_ptr, 0, sizeof(struct mbsys_surf_struct));

		/* get data structure pointer */
		store = (struct mbsys_surf_struct *) *store_ptr;

		/* MBIO data record kind */
		store->kind = MB_DATA_NONE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)*store_ptr);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_surf_deall(int verbose, void *mbio_ptr, void **store_ptr,
			int *error)
{
	char	*function_name = "mbsys_surf_deall";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)*store_ptr);
		}

	/* deallocate memory for data structure */
	status = mb_free(verbose,store_ptr,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_surf_dimensions(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nbath, int *namp, int *nss, int *error)
{
	char	*function_name = "mbsys_surf_dimensions";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_surf_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_surf_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get beam and pixel numbers */
		*nbath = store->NrBeams;
		*namp = store->NrAmplitudes;
		*nss = store->NrSidescan;
		}
	else
		{
		/* get beam and pixel numbers */
		*nbath = 0;
		*namp = 0;
		*nss = 0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		fprintf(stderr,"dbg2       nbath:         %d\n",*nbath);
		fprintf(stderr,"dbg2       namp:          %d\n",*namp);
		fprintf(stderr,"dbg2       nss:           %d\n",*nss);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_surf_extract(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_surf_extract";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_surf_struct *store;
	double	v0, tlx, tly, tlz, z0, t0, t2, tn, dt, y;
	int		i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_surf_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		*time_d = store->AbsoluteStartTimeOfProfile
				+ (double) store->SoundingData.relTime;
		mb_get_date(verbose, *time_d, time_i);

		/* get navigation */
		*navlon = RTD * ((double) store->CenterPosition[0].centerPositionX
					+ store->GlobalData.referenceOfPositionX);
		*navlat = RTD * ((double) store->CenterPosition[0].centerPositionY
					+ store->GlobalData.referenceOfPositionY);

		/* get heading */
		*heading = RTD * store->SoundingData.headingWhileTransmitting;

		/* get speed  */
		*speed = 3.6 * store->CenterPosition[0].speed;

		/* set beamwidths in mb_io structure */
		mb_io_ptr->beamwidth_ltrack = 2.3;
		mb_io_ptr->beamwidth_xtrack = 2.3;

		/* transducer location */
		tlx = store->ActualTransducerTable.transducerPositionAhead;
		tly = store->ActualTransducerTable.transducerPositionStar;
		tlz = store->ActualTransducerTable.transducerDepth
			- store->SoundingData.heaveWhileTransmitting;

		/* reset storage arrays */
		for (i=0;i<MBSYS_SURF_MAXBEAMS;i++)
			{
			bath[i] = 0.0;
			beamflag[i] = MB_FLAG_NULL;
			amp[i] = 0.0;
			bathacrosstrack[i] = 0.0;
			bathalongtrack[i] = 0.0;
			}
		/* read distance and depth values into storage arrays */
		*nbath = store->NrBeams;
		for (i = 0; i < store->NrBeams; i ++)
			{
			if ((store->MultiBeamDepth[i].depthFlag & SB_DELETED) != 0)
				beamflag[i] = MB_FLAG_NULL;
			else if ((store->MultiBeamDepth[i].depthFlag &
					 (SB_DEPTH_SUPPRESSED | SB_REDUCED_FAN)) != 0)
				beamflag[i] = MB_FLAG_FLAG + MB_FLAG_MANUAL;
			else
				beamflag[i] = MB_FLAG_NONE;
			bath[i] = (double) store->MultiBeamDepth[i].depth;
			if (bath[i] < tlz)
				beamflag[i] = MB_FLAG_FLAG + MB_FLAG_MANUAL;
			bathacrosstrack[i] = (double) store->MultiBeamDepth[i].beamPositionStar;
			bathalongtrack[i] = (double) store->MultiBeamDepth[i].beamPositionAhead;
			}

		/* get beam amplitudes */
		*namp = store->NrAmplitudes;
		for (i=0;i<store->NrAmplitudes;i++)
			{
			amp[i] = (double) store->MultibeamBeamAmplitudes[i].beamAmplitude;
			}

		/* get single beam if not multibeam file */
		if (store->NrBeams == 0
			&& store->GlobalData.typeOfSounder == 'V')
			{
			if (store->SingleBeamDepth.depthHFreq > 0.0)
				{
				*nbath = 1;
				bath[0] = (double) store->SingleBeamDepth.depthHFreq;
				beamflag[0] = MB_FLAG_NONE;
				bathacrosstrack[0] = 0.0;
				bathalongtrack[0] = 0.0;
				amp[0] = 0.0;
				}
			else if (store->SingleBeamDepth.depthMFreq > 0.0)
				{
				*nbath = 1;
				bath[0] = (double) store->SingleBeamDepth.depthMFreq;
				beamflag[0] = MB_FLAG_NONE;
				bathacrosstrack[0] = 0.0;
				bathalongtrack[0] = 0.0;
				amp[0] = 0.0;
				}
			else if (store->SingleBeamDepth.depthLFreq > 0.0)
				{
				*nbath = 1;
				bath[0] = (double) store->SingleBeamDepth.depthLFreq;
				beamflag[0] = MB_FLAG_NONE;
				bathacrosstrack[0] = 0.0;
				bathalongtrack[0] = 0.0;
				amp[0] = 0.0;
				}
			if (*nbath == 1)
				{
				if ((int)(store->SingleBeamDepth.depthFlag & SB_DELETED) != 0)
					beamflag[0] = MB_FLAG_NULL;
				else if ((int)(store->SingleBeamDepth.depthFlag
					& (SB_DEPTH_SUPPRESSED + SB_REDUCED_FAN)) != 0)
					beamflag[0] = MB_FLAG_FLAG + MB_FLAG_MANUAL;
				else
					beamflag[0] = MB_FLAG_NONE;
				}
			}

		/* reset sidescan */
		*nss = 0;
		for (i = 0; i < store->NrSidescan; i++)
			{
			ss[i] = 0.0;
			ssacrosstrack[i] = 0.0;
			ssalongtrack[i] = 0.0;
			}

		/* get sidescan */
		v0 = (store->SoundingData.cMean > 0.0 ?
				store->SoundingData.cMean : 1500.0);
		z0 = mbsys_get_depth(store->MultiBeamDepth,
							 store->ActualTransducerTable,
							 store->SoundingData.heaveWhileTransmitting,
							 store->NrBeams);
		if ((store->NrSidescan > 0) && (z0 > tlz))
			{
			*nss = store->NrSidescan;
		    z0 -= tlz;
			t0 = z0/v0;
			t2 = t0*t0;
			/* read portside scan */
			if (store->SidescanData.actualNrOfSsDataPort > 1)
				{
				/* initialise */
				tn = store->SidescanData.minSsTimePort / 2;
				dt = (store->SidescanData.maxSsTimePort / 2 - tn)
					/ (store->SidescanData.actualNrOfSsDataPort - 1);
				/* start reading: */
				for (i = 0, j = store->SidescanData.actualNrOfSsDataPort; 0 < --j; i++)
					{
					y = (tn > t0 ? v0 * sqrt(tn*tn - t2) : 0.0);
					ssalongtrack[j]	 = tlx;
					ssacrosstrack[j] = tly - y;
					ss[j] = store->SidescanData.ssData[i];
					tn += dt;
					}
				}
			/* read starboard scan */
			if (store->SidescanData.actualNrOfSsDataStb > 1)
				{
				/* initialise */
				tn = store->SidescanData.minSsTimeStb / 2;
				dt = (store->SidescanData.maxSsTimeStb / 2 - tn)
					/ (store->SidescanData.actualNrOfSsDataStb - 1);
				for (j = store->SidescanData.actualNrOfSsDataPort; j < store->NrSidescan; j++)
					{
					y = (tn > t0 ? v0 * sqrt(tn*tn - t2) : 0.0);
					ssalongtrack[j]	 = tlx;
					ssacrosstrack[j] = tly + y;
					ss[j] = store->SidescanData.ssData[j];
					tn += dt;
					}
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
			fprintf(stderr,"dbg4        time_i[6]:  %d\n",
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
			fprintf(stderr,"dbg4        namp:     %d\n",
				*namp);
			for (i=0;i<*namp;i++)
			  fprintf(stderr,"dbg4        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n",
				i,amp[i],bathacrosstrack[i],bathalongtrack[i]);
			fprintf(stderr,"dbg4        nss:      %d\n",
				*nss);
			for (i=0;i<*nss;i++)
			  fprintf(stderr,"dbg4        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
				i,ss[i],ssacrosstrack[i ],ssalongtrack[i]);
			}

		/* done translating values */

		}

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT)
		{
		/* copy comment */
/*		strncpy(comment,store->comment,
			MBSYS_SURF_COMMENT_LENGTH);*/

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  New ping read by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New ping values:\n");
			fprintf(stderr,"dbg4       error:      %d\n",
				*error);
			fprintf(stderr,"dbg4       comment:    %s\n",
				comment);
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
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
int mbsys_surf_insert(int verbose, void *mbio_ptr, void *store_ptr,
		int kind, int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_surf_insert";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_surf_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		fprintf(stderr,"dbg2       kind:       %d\n",kind);
		}
	if (verbose >= 2 && kind == MB_DATA_DATA)
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
		fprintf(stderr,"dbg2       nbath:      %d\n",nbath);
		if (verbose >= 3)
		 for (i=0;i<nbath;i++)
		  fprintf(stderr,"dbg3       beam:%d  flag:%3d  bath:%f   acrosstrack:%f  alongtrack:%f\n",
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
		  fprintf(stderr,"dbg3        beam:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
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
	store = (struct mbsys_surf_struct *) store_ptr;

	/* set data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* set time */
		store->SoundingData.relTime = (float) (time_d
		    - store->AbsoluteStartTimeOfProfile);

		/* set navigation */
		store->CenterPosition[0].centerPositionX = (float)
			(DTR * navlon - store->GlobalData.referenceOfPositionX);
		store->CenterPosition[0].centerPositionY = (float)
			(DTR * navlat - store->GlobalData.referenceOfPositionY);

		/* set heading */
		store->SoundingData.headingWhileTransmitting = (float) (DTR * heading);

		/* set speed  */
		store->CenterPosition[0].speed = (float) (speed / 3.6);

		/* write distance and depth values into storage arrays */
		if (store->GlobalData.typeOfSounder == 'B'
			|| store->GlobalData.typeOfSounder == 'F')
			{
			store->NrBeams = nbath;
			for (i=0;i<store->NrBeams;i++)
				{
				store->MultiBeamDepth[i].depth = bath[i];
				if (beamflag[i] == MB_FLAG_NULL)
			    		store->MultiBeamDepth[i].depthFlag
						= store->MultiBeamDepth[i].depthFlag | SB_DELETED;
				else if (!mb_beam_check_flag(beamflag[i]))
			    		store->MultiBeamDepth[i].depthFlag
						= store->MultiBeamDepth[i].depthFlag
							& 2046;
				else
			    		store->MultiBeamDepth[i].depthFlag
						= store->MultiBeamDepth[i].depthFlag
							| SB_DEPTH_SUPPRESSED;
				store->MultiBeamDepth[i].beamPositionStar = (float) bathacrosstrack[i];
				store->MultiBeamDepth[i].beamPositionAhead = (float) bathalongtrack[i];
				}
			}
		else if (store->GlobalData.typeOfSounder == 'V')
			{
			store->NrBeams = 0;
  			if (store->SingleBeamDepth.depthHFreq > 0.0)
  				store->SingleBeamDepth.depthHFreq = bath[0];
			else if (store->SingleBeamDepth.depthMFreq > 0.0)
				store->SingleBeamDepth.depthMFreq = bath[0];
			else if (store->SingleBeamDepth.depthLFreq > 0.0)
				store->SingleBeamDepth.depthLFreq = bath[0];
			else
				store->SingleBeamDepth.depthMFreq = bath[0];
			if (beamflag[i] == MB_FLAG_NULL)
			   	store->SingleBeamDepth.depthFlag
					= store->SingleBeamDepth.depthFlag | SB_DELETED;
			else if (!mb_beam_check_flag(beamflag[i]))
			    	store->SingleBeamDepth.depthFlag
					= store->SingleBeamDepth.depthFlag & 2046;
			else
			    	store->SingleBeamDepth.depthFlag
					= store->SingleBeamDepth.depthFlag | SB_DEPTH_SUPPRESSED;
			}

		/* set beam amplitudes */
		store->NrAmplitudes = namp;
		for (i=0;i<store->NrAmplitudes;i++)
			{
			store->MultibeamBeamAmplitudes[i].beamAmplitude = (unsigned short) amp[i];
			}

		if (nss == store->SidescanData.actualNrOfSsDataPort
				 + store->SidescanData.actualNrOfSsDataStb)
			{
    		for (i=0;i<store->SidescanData.actualNrOfSsDataPort;i++)
    			{
    			store->SidescanData.ssData[i] = ss[store->SidescanData.actualNrOfSsDataPort - i - 1];
    			}
    		for (i=store->SidescanData.actualNrOfSsDataStb;i<nss;i++)
    			{
    			store->SidescanData.ssData[i] = ss[i];
    			}
    		}
		}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT)
		{
		/*strncpy(store->comment,comment,
			MBSYS_SURF_COMMENT_LENGTH);*/
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_surf_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams,
	double *ttimes, double *angles,
	double *angles_forward, double *angles_null,
	double *heave, double *alongtrack_offset,
	double *draft, double *ssv, int *error)
{
	char	*function_name = "mbsys_surf_ttimes";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_surf_struct *store;
	double pitch, angle;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		fprintf(stderr,"dbg2       ttimes:     %p\n",(void *)ttimes);
		fprintf(stderr,"dbg2       angles_xtrk:%p\n",(void *)angles);
		fprintf(stderr,"dbg2       angles_ltrk:%p\n",(void *)angles_forward);
		fprintf(stderr,"dbg2       angles_null:%p\n",(void *)angles_null);
		fprintf(stderr,"dbg2       heave:      %p\n",(void *)heave);
		fprintf(stderr,"dbg2       ltrk_off:   %p\n",(void *)alongtrack_offset);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_surf_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{

		/* get draft */
		*draft = store->ActualTransducerTable.transducerDepth;

		/* get ssv */
		*ssv = store->SoundingData.cKeel;

		/* get travel times */
		if (((store->GlobalData.typeOfSounder == 'B')
		|| 	 (store->GlobalData.typeOfSounder == 'F'))
		&&	(store->NrBeams == store->ActualAngleTable.actualNumberOfBeams)
		&&	(store->NrBeams == store->NrTravelTimes))
			{
			*nbeams = store->NrBeams;
			pitch = - RTD * store->SoundingData.pitchWhileTransmitting;
			for (i=0;i<store->NrBeams;i++)
				{
				ttimes[i] = 0.0;
				angles[i] = 0.0;
				angles_forward[i] = 0.0;
				angles_null[i] = 0.0;
				heave[i] = 0.0;
				alongtrack_offset[i] = 0.0;
				}
			for (i=0;i<store->NrBeams;i++)
				{
				ttimes[i] = 2*store->MultiBeamTraveltime[i].travelTimeOfRay;
				angle = 90. - RTD * store->ActualAngleTable.beamAngle[i];
				mb_rollpitch_to_takeoff(verbose,
					pitch, angle,
					&angles[i],&angles_forward[i], error);
				heave[i] =  -0.5 * (store->SoundingData.heaveWhileTransmitting
					+ store->MultiBeamReceiveParams[i].heaveWhileReceiving);
				}
			/* set status */
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else if (store->GlobalData.typeOfSounder == 'V')
			{
			*nbeams = 1;
			ttimes[0] = 2*store->SingleBeamDepth.travelTimeOfRay;
			angles_forward[0] = 0.0;
			angles_null[0] = 0.0;
			heave[0] = -0.5 * (store->SoundingData.heaveWhileTransmitting
					+ store->MultiBeamReceiveParams[0].heaveWhileReceiving);
			alongtrack_offset[0] = 0.0;

			/* set status */
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else
			{
			/* set status */
			*error = MB_ERROR_OTHER;
			status = MB_FAILURE;
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
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
int mbsys_surf_detects(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams, int *detects, int *error)
{
	char	*function_name = "mbsys_surf_detects";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_surf_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		fprintf(stderr,"dbg2       detects:    %p\n",(void *)detects);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_surf_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* amplitude detects from Atlas multibeams */
		if (store->GlobalData.typeOfSounder == 'B')
			{
			*nbeams = store->NrBeams;
			for (i=0;i<store->NrBeams;i++)
				{
				detects[i] = MB_DETECT_AMPLITUDE;
				}
			}

		/* phase detects from Atlas fansweeps */
		else if (store->GlobalData.typeOfSounder == 'F')
			{
			*nbeams = store->NrBeams;
			for (i=0;i<store->NrBeams;i++)
				{
				detects[i] = MB_DETECT_PHASE;
				}
			}
		else if (store->GlobalData.typeOfSounder == 'V')
			{
			*nbeams = 1;
			detects[0] = MB_DETECT_AMPLITUDE;
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR)
		{
		fprintf(stderr,"dbg2       nbeams:     %d\n",*nbeams);
		for (i=0;i<*nbeams;i++)
			fprintf(stderr,"dbg2       beam %d: detects:%d\n",
				i,detects[i]);
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
double mbsys_get_depth(	SurfMultiBeamDepth* 			MultiBeamDepth,
						SurfTransducerParameterTable	TransducerTable,
						float	heave,
						int		n )
{
	double	x, y, z, a, b, c, d, depth;
	int		i, N;

	a = b = c = d = 0.0;
	N = 0;
	/* include all beams with a lateral distance within ~15 % of the depth
	   and calculate the depth @ centre assuming a linear slope.
	 */
	for (i = 0; i < n; i++)
		if ((MultiBeamDepth[i].depthFlag &
			(SB_DELETED | SB_DEPTH_SUPPRESSED | SB_REDUCED_FAN)) == 0)
			{
			x = MultiBeamDepth[i].beamPositionAhead	-
				TransducerTable.transducerPositionAhead;
			y = MultiBeamDepth[i].beamPositionStar -
				TransducerTable.transducerPositionStar;
			z = MultiBeamDepth[i].depth + heave -
				TransducerTable.transducerDepth;
			if ((x*x + y*y) <= (z*z/50.))
				{
					a += y;
					b += y*y;
					c += z;
					d += z*y;
					N++;
				}
			}

	if (N > 0)
		depth = (b*c - a*d)/(N*b - a*a)	- heave + TransducerTable.transducerDepth;
	else
		depth = 0.0;

	return(depth);
}
/*----------------------------------------------------------------------
roll in MB-System is positive rotating starboard to down, same as in SURF
pitch in MB-System is positive rotating down to forward, opposite in SURF
heave in MB-System is positive down, which conforms to the cartesian
		coordinates convention, but seems rather a contradiction in terms
													     opposite in SURF
*/
int mbsys_surf_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, double *transducer_depth, double *altitude,
	int *error)
{
	char	*function_name = "mbsys_surf_extract_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_surf_struct *store;
	double	bath_best;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_surf_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get transducer depth and altitude */
		*transducer_depth = store->ActualTransducerTable.transducerDepth
					- store->SoundingData.heaveWhileTransmitting;
		bath_best = mbsys_get_depth(store->MultiBeamDepth,
									store->ActualTransducerTable,
									store->SoundingData.heaveWhileTransmitting,
									store->NrBeams);;
		if (bath_best > *transducer_depth)
			{
		    *altitude = bath_best - *transducer_depth;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else
			{
		    *altitude = 0.0;
			*error = MB_ERROR_OTHER;
			status = MB_FAILURE;
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
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
int mbsys_surf_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft,
		double *roll, double *pitch, double *heave,
		int *error)
{
	char	*function_name = "mbsys_surf_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_surf_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_surf_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from ping structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		*time_d = store->AbsoluteStartTimeOfProfile
			    + (double) store->SoundingData.relTime;
		mb_get_date(verbose, *time_d, time_i);

		/* get navigation */
		*navlon = RTD * ((double) store->CenterPosition[0].centerPositionX
					+ store->GlobalData.referenceOfPositionX);
		*navlat = RTD * ((double) store->CenterPosition[0].centerPositionY
					+ store->GlobalData.referenceOfPositionY);

		/* get draft */
		*draft = store->ActualTransducerTable.transducerDepth;

		/* get heading */
		*heading = RTD * store->SoundingData.headingWhileTransmitting;

		/* get speed  */
		*speed = 3.6 * store->CenterPosition[0].speed;

		/* get roll pitch and heave */
		*roll = RTD * store->SoundingData.rollWhileTransmitting;
		*pitch = -RTD * store->SoundingData.pitchWhileTransmitting;
		*heave = -store->SoundingData.heaveWhileTransmitting;

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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
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
int mbsys_surf_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading, double draft,
		double roll, double pitch, double heave,
		int *error)
{
	char	*function_name = "mbsys_surf_insert_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_surf_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
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
	store = (struct mbsys_surf_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get time */
		store->SoundingData.relTime = (float) (time_d
		    - store->AbsoluteStartTimeOfProfile);

		/* get navigation */
		store->CenterPosition[0].centerPositionX = (float)
			(DTR * navlon - store->GlobalData.referenceOfPositionX);
		store->CenterPosition[0].centerPositionY = (float)
			(DTR * navlat - store->GlobalData.referenceOfPositionY);

		/* get heading */
		store->SoundingData.headingWhileTransmitting = (float) (DTR * heading);

		/* get speed  */
		store->CenterPosition[0].speed = (float) (speed / 3.6);

		/* get draft  */
		store->ActualTransducerTable.transducerDepth = draft;

		/* get roll pitch and heave */
		store->SoundingData.rollWhileTransmitting = (float) (DTR * roll);
		store->SoundingData.pitchWhileTransmitting = (float) (-DTR * pitch);
		store->SoundingData.heaveWhileTransmitting = (float) (-heave);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_surf_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nsvp,
		double *depth, double *velocity,
		int *error)
{
	char	*function_name = "mbsys_surf_extract_svp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_surf_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_surf_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA &&
		store->ActualCProfileTable.numberOfActualValues > 0)
		{
		/* get number of depth-velocity pairs */
		*nsvp = store->ActualCProfileTable.numberOfActualValues;

		/* get profile */
		for (i=0;i<*nsvp;i++)
			{
			depth[i] = store->ActualCProfileTable.values[i].depth;
			velocity[i] = store->ActualCProfileTable.values[i].cValue;
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
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
int mbsys_surf_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
		int nsvp,
		double *depth, double *velocity,
		int *error)
{
	char	*function_name = "mbsys_surf_insert_svp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_surf_struct *store;
	int	kind;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		fprintf(stderr,"dbg2       nsvp:       %d\n",nsvp);
		for (i=0;i<nsvp;i++)
		    fprintf(stderr,"dbg2       depth[%d]: %f   velocity[%d]: %f\n",i, depth[i], i, velocity[i]);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_surf_struct *) store_ptr;

	/* get data kind */
	kind = store->kind;

	/* insert data in structure */
	if (kind == MB_DATA_DATA && nsvp > 0)
		{
		store->NrSoundvelocityProfiles++;

		/* set number of depth-velocity pairs */
		store->ActualCProfileTable.numberOfActualValues
			= MIN(nsvp, MBSYS_SURF_MAXCVALUES);

		/* set profile */
		for (i=0;i<store->ActualCProfileTable.numberOfActualValues;i++)
			{
			store->ActualCProfileTable.values[i].depth = depth[i];
			store->ActualCProfileTable.values[i].cValue = velocity[i];
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_surf_copy(int verbose, void *mbio_ptr,
			void *store_ptr, void *copy_ptr,
			int *error)
{
	char	*function_name = "mbsys_surf_copy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_surf_struct *store;
	struct mbsys_surf_struct *copy;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		fprintf(stderr,"dbg2       copy_ptr:   %p\n",(void *)copy_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointers */
	store = (struct mbsys_surf_struct *) store_ptr;
	copy = (struct mbsys_surf_struct *) copy_ptr;

	/* copy the main structure */
	*copy = *store;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/

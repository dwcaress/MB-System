/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_surf.c	3.00	6/25/01
 *	$Id: mbsys_surf.c,v 5.11 2003-03-06 00:14:52 caress Exp $
 *
 *    Copyright (c) 2001, 2002 by
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
 * $Log: not supported by cvs2svn $
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
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mb_define.h"
#include "../../include/mbsys_surf.h"

static char res_id[]="$Id: mbsys_surf.c,v 5.11 2003-03-06 00:14:52 caress Exp $";

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
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
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
int mbsys_surf_deall(int verbose, void *mbio_ptr, void **store_ptr,
			int *error)
{
	char	*function_name = "mbsys_surf_deall";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_surf_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
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
	double	range, tt, timeslice, ssdepth;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
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

		/* read distance and depth values into storage arrays */
		*nbath = store->NrBeams;
		for (i=0;i<MBSYS_SURF_MAXBEAMS;i++)
			{
			bath[i] = 0.0;
			beamflag[i] = MB_FLAG_NULL;
			amp[i] = 0.0;
			bathacrosstrack[i] = 0.0;
			bathalongtrack[i] = 0.0;
			}
		for (i=0;i<store->NrBeams;i++)
			{
			bath[i] = (double) store->MultiBeamDepth[i].depth;
			if ((int)(store->MultiBeamDepth[i].depthFlag & SB_DELETED) != 0)
			    beamflag[i] = MB_FLAG_NULL;
			else if ((int)(store->MultiBeamDepth[i].depthFlag
				& (SB_DEPTH_SUPPRESSED + SB_REDUCED_FAN)) != 0)
			    beamflag[i] = MB_FLAG_FLAG;
			else
			    beamflag[i] = MB_FLAG_NONE;
			bathacrosstrack[i] = (double) store->MultiBeamDepth[i].beamPositionStar;
			bathalongtrack[i] = (double) store->MultiBeamDepth[i].beamPositionAhead;
			amp[i] = (double) store->MultibeamBeamAmplitudes[i].beamAmplitude;
			}
		*namp = *nbath;

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
				    beamflag[0] = MB_FLAG_FLAG;
				else
				    beamflag[0] = MB_FLAG_NONE;
				}
			}

		/* get sidescan */
		*nss = store->SidescanData.actualNrOfSsDataPort
				+ store->SidescanData.actualNrOfSsDataStb;
		for (i=0;i<*nss;i++)
			{
			ss[i] = 0.0;
			ssacrosstrack[i] = 0.0;
			ssalongtrack[i] = 0.0;
			}
		if (store->SidescanData.actualNrOfSsDataPort > 1)
		    {
		    ssdepth = store->SoundingData.cMean
				* store->SidescanData.minSsTimePort / 2.0;
		    timeslice = (store->SidescanData.maxSsTimePort
				- store->SidescanData.minSsTimePort)
				/ store->SidescanData.actualNrOfSsDataPort;
		    }
		for (i=0;i<store->SidescanData.actualNrOfSsDataPort;i++)
			{
			j = store->SidescanData.actualNrOfSsDataPort - i - 1;
			tt = store->SidescanData.minSsTimePort + timeslice * i;
			ss[j] = store->SidescanData.ssData[i];
			range = store->SoundingData.cMean * tt / 2.0;
			ssacrosstrack[j] = -sqrt(range * range - ssdepth * ssdepth);
			ssalongtrack[j] = 0.0;
			}
		if (store->SidescanData.actualNrOfSsDataStb > 1)
		    {
		    ssdepth = store->SoundingData.cMean
				* store->SidescanData.minSsTimeStb / 2.0;
		    timeslice = (store->SidescanData.maxSsTimeStb
				- store->SidescanData.minSsTimeStb)
				/ store->SidescanData.actualNrOfSsDataStb;
		    }
		for (i=0;i<store->SidescanData.actualNrOfSsDataStb;i++)
			{
			j = store->SidescanData.actualNrOfSsDataPort + i;
			tt = store->SidescanData.minSsTimeStb + timeslice * i;
			ss[j] = store->SidescanData.ssData[i];
			range = store->SoundingData.cMean * tt / 2.0;
			ssacrosstrack[j] = sqrt(range * range - ssdepth * ssdepth);
			ssalongtrack[j] = 0.0;
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
/*		strncpy(comment,store->comment,
			MBSYS_SURF_COMMENT_LENGTH);*/

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
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
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

		/* read distance and depth values into storage arrays */
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
				store->MultibeamBeamAmplitudes[i].beamAmplitude = (unsigned short) amp[i];
				}
			}
		else if (store->GlobalData.typeOfSounder == 'V')
			{
			if (store->SingleBeamDepth.depthHFreq > 0.0)
				store->SingleBeamDepth.depthHFreq = bath[0];
			if (store->SingleBeamDepth.depthMFreq > 0.0)
				store->SingleBeamDepth.depthHFreq = bath[0];
			if (store->SingleBeamDepth.depthLFreq > 0.0)
				store->SingleBeamDepth.depthHFreq = bath[0];
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
		for (i=0;i<store->SidescanData.actualNrOfSsDataPort;i++)
			{
			store->SidescanData.ssData[i] = ss[store->SidescanData.actualNrOfSsDataPort - i - 1];
			}
		for (i=store->SidescanData.actualNrOfSsDataStb;i<nss;i++)
			{
			store->SidescanData.ssData[i] = ss[i];
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
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
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
	store = (struct mbsys_surf_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get travel times */
		if (store->GlobalData.typeOfSounder == 'B'
			|| store->GlobalData.typeOfSounder == 'F')
			{
			*nbeams = store->NrBeams;
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
				ttimes[i] = store->MultiBeamTraveltime[i].travelTimeOfRay;
				angles[i] = RTD * fabs(store->ActualAngleTable.beamAngle[i]);
				if (angles[i] < 0.0)
			    		angles_forward[i] = 180.0;
				else
			    		angles_forward[i] = 0.0;
				angles_null[i] = 0.0;
				heave[i] = 0.5 * (store->SoundingData.heaveWhileTransmitting
					+ store->MultiBeamReceiveParams[i].heaveWhileReceiving);
				alongtrack_offset[i] = 0.0;
				}
			}
		else if (store->GlobalData.typeOfSounder == 'V')
			{
			*nbeams = 1;
			ttimes[0] = store->SingleBeamDepth.travelTimeOfRay;
			angles_forward[i] = 0.0;
			angles_null[i] = 0.0;
			heave[i] = 0.5 * (store->SoundingData.heaveWhileTransmitting
					+ store->MultiBeamReceiveParams[i].heaveWhileReceiving);
			alongtrack_offset[i] = 0.0;
			}

		/* get draft */
		*draft = store->ActualTransducerTable.transducerDepth;

		/* get ssv */
		*ssv = store->SoundingData.cKeel;

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
int mbsys_surf_detects(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams, int *detects, int *error)
{
	char	*function_name = "mbsys_surf_detects";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_surf_struct *store;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		fprintf(stderr,"dbg2       detects:    %d\n",detects);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
int mbsys_surf_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, double *transducer_depth, double *altitude,
	int *error)
{
	char	*function_name = "mbsys_surf_extract_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_surf_struct *store;
	double	bath_best;
	double	xtrack_min;
	int	found;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
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
					+ store->SoundingData.heaveWhileTransmitting;
		found = MB_NO;
		bath_best = 0.0;
		xtrack_min = 99999999.9;
		for (i=0;i<store->NrBeams;i++)
		    {
		    if ((int)(store->MultiBeamDepth[i].depthFlag
		    		& (SB_DEPTH_SUPPRESSED + SB_REDUCED_FAN + SB_DELETED)) == 0
			&& fabs((double)store->MultiBeamDepth[i].beamPositionStar) < xtrack_min)
			{
			xtrack_min = fabs((double)store->MultiBeamDepth[i].beamPositionStar);
			bath_best = (double) store->MultiBeamDepth[i].depth;
			found = MB_YES;
			}
		    }
		if (found == MB_NO)
		    {
		    xtrack_min = 99999999.9;
		    for (i=0;i<store->NrBeams;i++)
			{
			if ((int)(store->MultiBeamDepth[i].depthFlag & SB_DELETED) == 0
			&& fabs((double)store->MultiBeamDepth[i].beamPositionStar) < xtrack_min)
			    {
			    xtrack_min = fabs((double)store->MultiBeamDepth[i].beamPositionStar);
			    bath_best = (double) store->MultiBeamDepth[i].depth;
			    found = MB_YES;
			    }
			}
		    }
		if (found == MB_YES)
		    *altitude = bath_best - *transducer_depth;
		else
		    *altitude = 0.0;

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
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
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

		/* get heading */
		*heading = RTD * store->SoundingData.headingWhileTransmitting;

		/* get speed  */
		*speed = 3.6 * store->CenterPosition[0].speed;

		/* get roll pitch and heave */
		*roll = RTD * store->SoundingData.rollWhileTransmitting;
		*pitch = RTD * store->SoundingData.pitchWhileTransmitting;
		*heave = store->SoundingData.heaveWhileTransmitting;

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
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
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
		store->SoundingData.pitchWhileTransmitting = (float) (DTR * pitch);
		store->SoundingData.heaveWhileTransmitting = (float) heave;
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
int mbsys_surf_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nsvp,
		double *depth, double *velocity,
		int *error)
{
	char	*function_name = "mbsys_surf_extract_svp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_surf_struct *store;
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		fprintf(stderr,"dbg2       copy_ptr:   %d\n",copy_ptr);
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

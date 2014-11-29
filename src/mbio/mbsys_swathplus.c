/*-------------------------------------------------------------------
 *    The MB-system:	mbsys_swathplus.c	3.00	1/27/2014
 *	$Id$
 *
 *    Copyright (c) 2014-2014 by
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
 * mbsys_swathplus.c contains the MBIO functions for handling data from
 * SEA SWATHplus interferometric formats:
 *      MBF_SWPLSSXI : MBIO ID 221 - SWATHplus intermediate format
 *      MBF_SWPLSSXP : MBIO ID 221 - SWATHplus processed format
 *
 * Author:	David Finlayson and D. W. Caress
 * Date:	Aug 29, 2013
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"
#include "mbsys_swathplus.h"

static int set_identity(int verbose, swpls_matrix *m, int *error);
static int concat_transform(int verbose, swpls_matrix *a, swpls_matrix *b,
	int *error);
static int get_sxp_heave(int verbose, swpls_sxpping *sxp_sxpping, double *heave,
	int *error);
static int set_sxp_height(int verbose, double heave, swpls_sxpping *sxp_ping,
	int *error);

static char rcs_id[] =
	"$Id$";

/*--------------------------------------------------------------------*/
int mbsys_swathplus_alloc(int verbose, void *mbio_ptr, void **store_ptr,
	int *error)
{
	char *function_name = "mbsys_swathplus_alloc";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	int i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	status =
		mb_mallocd(verbose, __FILE__, __LINE__,
		sizeof(struct mbsys_swathplus_struct), (void **)store_ptr,
		error);

	/* initialize allocated structure to zero */
	if (status == MB_SUCCESS)
		{
		memset(*store_ptr, 0, sizeof(struct mbsys_swathplus_struct));
		}

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *)*store_ptr;

	/* initialize everything */

	/* Type of data record kind */
	store->kind = MB_DATA_NONE;
	store->type = SWPLS_ID_NONE;


	/* initialize MB-System time stamp */
	store->time_d = 0;
	for (i = 0; i < 7; i++)
		{
		store->time_i[i] = 0;
		}

	/* initialize projection set */
	store->projection_set = MB_NO;
	store->projection.projection_alloc = 0;
	store->projection.projection_id = NULL;
	
	/* initialize comment struct */
	store->comment.message_alloc = 0;
	store->comment.message = NULL;
	
	/* initialize sxp structs */
	store->sxp_header_set = MB_NO;
	store->sxp_ping.points_alloc = 0;
	store->sxp_ping.points = NULL;

	/* initialize sxi structs */
	store->sxi_header_set = MB_NO;
	store->sxi_ping.samps_alloc = 0;
	store->sxi_ping.sampnum = NULL;
	store->sxi_ping.angle = NULL;
	store->sxi_ping.amplitude = NULL;
	store->sxi_ping.quality = NULL;
	

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}	/* mbsys_swathplus_alloc */
/*--------------------------------------------------------------------*/
int mbsys_swathplus_deall(int verbose, void *mbio_ptr, void **store_ptr,
	int *error)
{
	char *function_name = "mbsys_swathplus_deall";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_sxpping *sxp_ping;
	swpls_sxiping *sxi_ping;
	swpls_projection *projection;
	swpls_comment *comment;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
		}

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *)*store_ptr;
	sxp_ping = (swpls_sxpping *)&(store->sxp_ping);
	sxi_ping = (swpls_sxiping *)&(store->sxi_ping);
	projection = (swpls_projection *)&(store->projection);
	comment = (swpls_comment *)&(store->comment);

	/* deallocate any arrays or structures contained within the store data
	   structure */
	if (sxp_ping->points != NULL)
		{
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(sxp_ping->points),
			error);
		sxp_ping->points_alloc = 0;
		}
	if (sxi_ping->sampnum != NULL)
		{
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(sxi_ping->sampnum),
			error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(sxi_ping->angle),
			error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(sxi_ping->amplitude),
			error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(sxi_ping->quality),
			error);
		sxi_ping->samps_alloc = 0;
		}
	if (projection->projection_id != NULL)
		{
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(projection->projection_id),
			error);
		projection->projection_alloc = 0;
		}
	if (comment->message != NULL)
		{
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(comment->message),
			error);
		comment->message_alloc = 0;
		}

	/* deallocate memory for data structure */
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)store_ptr, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}	/* mbsys_swathplus_deall */
/*--------------------------------------------------------------------*/
int mbsys_swathplus_dimensions(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbath, int *namp, int *nss,
	int *error)
{
	char *function_name = "mbsys_swathplus_dimensions";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	swpls_sxpping *sxp_ping;
	swpls_sxiping *sxi_ping;
	int type;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	sxp_ping = (swpls_sxpping *)&(store->sxp_ping);
	sxi_ping = (swpls_sxiping *)&(store->sxi_ping);

	/* get data kind */
	*kind = store->kind;
	type = store->type;

	/* extract data from structure */
	if ((*kind == MB_DATA_DATA) &&
		((type == SWPLS_ID_PROCESSED_PING) ||
		(type == SWPLS_ID_PROCESSED_PING2)))
		{
		/* SXP Files */
		*nbath = sxp_ping->nosampsfile;
		*namp = sxp_ping->nosampsfile;
		*nss = 0;
		}
	else if ((*kind == MB_DATA_DATA) && (type == SWPLS_ID_PARSED_PING))
		{
		/* SXI Files */
		*nbath = sxi_ping->nosamps;
		*namp = sxi_ping->nosamps;
		*nss = 0;
		}
	else
		{
		/* everything else */
		*nbath = 0;
		*namp = 0;
		*nss = 0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		fprintf(stderr, "dbg2       nbath:      %d\n", *nbath);
		fprintf(stderr, "dbg2       namp:       %d\n", *namp);
		fprintf(stderr, "dbg2       nss:        %d\n", *nss);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}	/* mbsys_swathplus_dimensions */
/*--------------------------------------------------------------------*/
int mbsys_swathplus_pingnumber(int verbose, void *mbio_ptr, int *pingnumber,
	int *error)
{
	char *function_name = "mbsys_swathplus_pingnumber";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	swpls_sxpping *sxp_ping;
	swpls_sxiping *sxi_ping;
	int kind, type;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *)mb_io_ptr->store_data;
	sxp_ping = (swpls_sxpping *)&(store->sxp_ping);
	sxi_ping = (swpls_sxiping *)&(store->sxi_ping);

	/* get data kind */
	kind = store->kind;
	type = store->type;

	/* extract data from structure */
	if ((kind == MB_DATA_DATA) &&
		((type == SWPLS_ID_PROCESSED_PING) ||
		(type == SWPLS_ID_PROCESSED_PING2)))
		{
		*pingnumber = store->sxp_ping.pingnumber;
		}
	else if ((kind == MB_DATA_DATA) && (type == SWPLS_ID_PARSED_PING))
		{
		*pingnumber = store->sxi_ping.pingnumber;
		}
	else
		{
		*pingnumber = -1;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       pingnumber: %d\n", *pingnumber);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}	/* mbsys_swathplus_pingnumber */
/*--------------------------------------------------------------------*/
int mbsys_swathplus_sonartype(int verbose, void *mbio_ptr, void *store_ptr,
	int *sonartype, int *error)
{
	char *function_name = "mbsys_swathplus_sonartype";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *)store_ptr;

	/* get sonar type */
	*sonartype = MB_TOPOGRAPHY_TYPE_INTERFEROMETRIC;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       sonartype:  %d\n", *sonartype);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}	/* mbsys_swathplus_sonartype */
/*--------------------------------------------------------------------*/
int mbsys_swathplus_sidescantype(int verbose, void *mbio_ptr, void *store_ptr,
	int *ss_type, int *error)
{
	char *function_name = "mbsys_swathplus_sidescantype";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *)store_ptr;

	/* get sidescan type */
	*ss_type = MB_SIDESCAN_LOGARITHMIC;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ss_type:    %d\n", *ss_type);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}	/* mbsys_swathplus_sidescantype */
/*--------------------------------------------------------------------*/
int mbsys_swathplus_extract(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int time_i[7], double *time_d,
	double *navlon, double *navlat, double *speed,
	double *heading, int *nbath, int *namp, int *nss,
	char *beamflag, double *bath, double *amp,
	double *bathacrosstrack, double *bathalongtrack,
	double *ss, double *ssacrosstrack,
	double *ssalongtrack, char *comment, int *error)
{
	char *function_name = "mbsys_swathplus_extract";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	swpls_sxpping *sxp_ping;
	swpls_sxiping *sxi_ping;
	swpls_point *points;
	swpls_matrix wtov;
	int type;
	int i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	sxp_ping = (swpls_sxpping *)&(store->sxp_ping);
	sxi_ping = (swpls_sxiping *)&(store->sxi_ping);

	/* get data kind */
	*kind = store->kind;
	type = store->type;

	/* extract data from structure */
	if ((*kind == MB_DATA_DATA) &&
		((type == SWPLS_ID_PROCESSED_PING) ||
		(type == SWPLS_ID_PROCESSED_PING2)))
		{
		/* get time */
		for (i = 0; i < 7; i++)
			{
			time_i[i] = store->time_i[i];
			}
		*time_d = store->time_d;

		/* get navigation (probably projected coordinates) */
		*navlon = sxp_ping->txer_e;
		*navlat = sxp_ping->txer_n;

		/* get speed */
		*heading = sxp_ping->heading;
		*speed = 0.0;
		if (mb_io_ptr->nfix > 0)
			{
			mb_navint_prjinterp(verbose, mbio_ptr, store->time_d, *heading, *speed,
				navlon, navlat, speed, error);
			}

		/* reset heading */
		*heading = sxp_ping->heading;

		/* set beamwidths in mb_io structure based on sonar frequency (Hz) */
		if (sxp_ping->frequency < 200000.0)
			{
			mb_io_ptr->beamwidth_xtrack = SWPLS_TYPE_L_BEAM_WIDTH;
			mb_io_ptr->beamwidth_ltrack = SWPLS_TYPE_L_BEAM_WIDTH;
			}
		else if (sxp_ping->frequency < 400000.0)
			{
			mb_io_ptr->beamwidth_xtrack = SWPLS_TYPE_M_BEAM_WIDTH;
			mb_io_ptr->beamwidth_ltrack = SWPLS_TYPE_M_BEAM_WIDTH;
			}
		else
			{
			mb_io_ptr->beamwidth_xtrack = SWPLS_TYPE_H_BEAM_WIDTH;
			mb_io_ptr->beamwidth_ltrack = SWPLS_TYPE_H_BEAM_WIDTH;
			}

		/* read distance and depth values into storage arrays */
		*nbath = sxp_ping->nosampsfile;
		*namp = *nbath;

		swpls_init_transform(verbose, &wtov, error);
		swpls_concat_translate(verbose, &wtov, -(sxp_ping->txer_e), 0.0,
			-(sxp_ping->txer_n), error);
		swpls_concat_rotate_y(verbose, &wtov, -(sxp_ping->heading) * DTR,
			error);

		points = sxp_ping->points;
		for (i = 0; i < *nbath; i++)
			{
			swpls_vector ppos;

			ppos.x = points[i].x;
			ppos.y = -(points[i].z);
			ppos.z = points[i].y;

			swpls_transform(verbose, &wtov, &ppos, error);

			if (points[i].status != SWPLS_POINT_REJECTED)
				{
				beamflag[i] = MB_FLAG_NONE;
				}
			else
				{
				beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
				}

			bath[i] = -(ppos.y);
			bathacrosstrack[i] = ppos.x;
			bathalongtrack[i] = ppos.z;
			amp[i] = points[i].procamp;
			}

		/* extract sidescan */
		*nss = 0;

		/* print debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n",
				function_name);
			fprintf(stderr, "dbg4  Extracted values:\n");
			fprintf(stderr, "dbg4       kind:       %d\n", *kind);
			fprintf(stderr, "dbg4       error:      %d\n", *error);
			fprintf(stderr, "dbg4       time_i[0]:  %d\n", time_i[0]);
			fprintf(stderr, "dbg4       time_i[1]:  %d\n", time_i[1]);
			fprintf(stderr, "dbg4       time_i[2]:  %d\n", time_i[2]);
			fprintf(stderr, "dbg4       time_i[3]:  %d\n", time_i[3]);
			fprintf(stderr, "dbg4       time_i[4]:  %d\n", time_i[4]);
			fprintf(stderr, "dbg4       time_i[5]:  %d\n", time_i[5]);
			fprintf(stderr, "dbg4       time_i[6]:  %d\n", time_i[6]);
			fprintf(stderr, "dbg4       time_d:     %f\n", *time_d);
			fprintf(stderr, "dbg4       longitude:  %f\n", *navlon);
			fprintf(stderr, "dbg4       latitude:   %f\n", *navlat);
			fprintf(stderr, "dbg4       speed:      %f\n", *speed);
			fprintf(stderr, "dbg4       heading:    %f\n", *heading);
			fprintf(stderr, "dbg4       nbath:      %d\n", *nbath);
			for (i = 0; i < *nbath; i++)
				{
				fprintf(stderr,
					"dbg4       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i,
					beamflag[i], bath[i], bathacrosstrack[i],
					bathalongtrack[i]);
				}
			fprintf(stderr, "dbg4        namp:     %d\n", *namp);
			for (i = 0; i < *namp; i++)
				{
				fprintf(stderr,
					"dbg4        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i,
					amp[i], bathacrosstrack[i], bathalongtrack[i]);
				}
			fprintf(stderr, "dbg4        nss:      %d\n", *nss);
			for (i = 0; i < *nss; i++)
				{
				fprintf(stderr,
					"dbg4        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i,
					ss[i], ssacrosstrack[i], ssalongtrack[i]);
				}
			}

		/* done translating values */
		}
	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT)
		{
		/* get time */
		for (i = 0; i < 7; i++)
			{
			time_i[i] = store->time_i[i];
			}
		*time_d = store->time_d;

		/* copy comment */
		if (store->comment.nchars > 0)
			{
			strncpy(comment, store->comment.message, MB_COMMENT_MAXLINE);
			}
		else
			{
			comment[0] = '\0';
			}

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr, "\ndbg4  Comment extracted by MBIO function <%s>\n",
				function_name);
			fprintf(stderr, "dbg4  New ping values:\n");
			fprintf(stderr, "dbg4       kind:       %d\n", *kind);
			fprintf(stderr, "dbg4       error:      %d\n", *error);
			fprintf(stderr, "dbg4       time_i[0]:  %d\n", time_i[0]);
			fprintf(stderr, "dbg4       time_i[1]:  %d\n", time_i[1]);
			fprintf(stderr, "dbg4       time_i[2]:  %d\n", time_i[2]);
			fprintf(stderr, "dbg4       time_i[3]:  %d\n", time_i[3]);
			fprintf(stderr, "dbg4       time_i[4]:  %d\n", time_i[4]);
			fprintf(stderr, "dbg4       time_i[5]:  %d\n", time_i[5]);
			fprintf(stderr, "dbg4       time_i[6]:  %d\n", time_i[6]);
			fprintf(stderr, "dbg4       time_d:     %f\n", *time_d);
			fprintf(stderr, "dbg4       comment:    %s\n", comment);
			}
		}
	/* set time for other data records */
	else
		{
		/* get time */
		for (i = 0; i < 7; i++)
			{
			time_i[i] = store->time_i[i];
			}
		*time_d = store->time_d;

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n",
				function_name);
			fprintf(stderr, "dbg4  Extracted values:\n");
			fprintf(stderr, "dbg4       kind:       %d\n", *kind);
			fprintf(stderr, "dbg4       error:      %d\n", *error);
			fprintf(stderr, "dbg4       time_i[0]:  %d\n", time_i[0]);
			fprintf(stderr, "dbg4       time_i[1]:  %d\n", time_i[1]);
			fprintf(stderr, "dbg4       time_i[2]:  %d\n", time_i[2]);
			fprintf(stderr, "dbg4       time_i[3]:  %d\n", time_i[3]);
			fprintf(stderr, "dbg4       time_i[4]:  %d\n", time_i[4]);
			fprintf(stderr, "dbg4       time_i[5]:  %d\n", time_i[5]);
			fprintf(stderr, "dbg4       time_i[6]:  %d\n", time_i[6]);
			fprintf(stderr, "dbg4       time_d:     %f\n", *time_d);
			fprintf(stderr, "dbg4       comment:    %s\n", comment);
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		}
	if ((verbose >= 2) && (*error <= MB_ERROR_NO_ERROR) &&
		(*kind == MB_DATA_COMMENT))
		{
		fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
		}
	else if ((verbose >= 2) && (*error <= MB_ERROR_NO_ERROR) &&
		(*kind != MB_DATA_COMMENT))
		{
		fprintf(stderr, "dbg2       time_i[0]:     %d\n", time_i[0]);
		fprintf(stderr, "dbg2       time_i[1]:     %d\n", time_i[1]);
		fprintf(stderr, "dbg2       time_i[2]:     %d\n", time_i[2]);
		fprintf(stderr, "dbg2       time_i[3]:     %d\n", time_i[3]);
		fprintf(stderr, "dbg2       time_i[4]:     %d\n", time_i[4]);
		fprintf(stderr, "dbg2       time_i[5]:     %d\n", time_i[5]);
		fprintf(stderr, "dbg2       time_i[6]:     %d\n", time_i[6]);
		fprintf(stderr, "dbg2       time_d:        %f\n", *time_d);
		}
	if ((verbose >= 2) && ((*kind == MB_DATA_DATA) || (*kind == MB_DATA_NAV)))
		{
		fprintf(stderr, "dbg2       longitude:     %f\n", *navlon);
		fprintf(stderr, "dbg2       latitude:      %f\n", *navlat);
		fprintf(stderr, "dbg2       speed:         %f\n", *speed);
		fprintf(stderr, "dbg2       heading:       %f\n", *heading);
		}
	if ((verbose >= 2) && (*error <= MB_ERROR_NO_ERROR) &&
		(*kind == MB_DATA_DATA))
		{
		fprintf(stderr, "dbg2       nbath:      %d\n", *nbath);
		for (i = 0; i < *nbath; i++)
			{
			fprintf(stderr,
				"dbg2       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i,
				beamflag[i], bath[i], bathacrosstrack[i], bathalongtrack[i]);
			}
		fprintf(stderr, "dbg2        namp:     %d\n", *namp);
		for (i = 0; i < *namp; i++)
			{
			fprintf(stderr,
				"dbg2       beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n",
				i,
				amp[i], bathacrosstrack[i], bathalongtrack[i]);
			}
		fprintf(stderr, "dbg2        nss:      %d\n", *nss);
		for (i = 0; i < *nss; i++)
			{
			fprintf(stderr,
				"dbg2        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
				i,
				ss[i], ssacrosstrack[i], ssalongtrack[i]);
			}
		}
	if (verbose >= 2)
		{
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}	/* mbsys_swathplus_extract */
/*--------------------------------------------------------------------*/
int mbsys_swathplus_insert(int verbose, void *mbio_ptr, void *store_ptr,
	int kind, int time_i[7], double time_d,
	double navlon, double navlat, double speed,
	double heading, int nbath, int namp, int nss,
	char *beamflag, double *bath, double *amp,
	double *bathacrosstrack, double *bathalongtrack,
	double *ss, double *ssacrosstrack,
	double *ssalongtrack, char *comment, int *error)
{
	char *function_name = "mbsys_swathplus_insert";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	swpls_header *header;
	swpls_sxpping *sxp_ping;
	swpls_point *points;
	swpls_comment *ocomment;
	swpls_matrix vtow;
	char path[MB_PATH_MAXLINE];
	int i;
	int nchars;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       kind:       %d\n", kind);
		}
	if ((verbose >= 2) && (kind == MB_DATA_COMMENT))
		{
		fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
		}
	if ((verbose >= 2) && (kind != MB_DATA_COMMENT))
		{
		fprintf(stderr, "dbg2       time_i[0]:  %d\n", time_i[0]);
		fprintf(stderr, "dbg2       time_i[1]:  %d\n", time_i[1]);
		fprintf(stderr, "dbg2       time_i[2]:  %d\n", time_i[2]);
		fprintf(stderr, "dbg2       time_i[3]:  %d\n", time_i[3]);
		fprintf(stderr, "dbg2       time_i[4]:  %d\n", time_i[4]);
		fprintf(stderr, "dbg2       time_i[5]:  %d\n", time_i[5]);
		fprintf(stderr, "dbg2       time_i[6]:  %d\n", time_i[6]);
		fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
		}
	if ((verbose >= 2) && ((kind == MB_DATA_DATA) || (kind == MB_DATA_NAV)))
		{
		fprintf(stderr, "dbg2       navlon:     %f\n", navlon);
		fprintf(stderr, "dbg2       navlat:     %f\n", navlat);
		fprintf(stderr, "dbg2       speed:      %f\n", speed);
		fprintf(stderr, "dbg2       heading:    %f\n", heading);
		}
	if ((verbose >= 2) && (kind == MB_DATA_DATA))
		{
		fprintf(stderr, "dbg2       nbath:      %d\n", nbath);
		if (verbose >= 3)
			{
			for (i = 0; i < nbath; i++)
				{
				fprintf(stderr,
					"dbg3       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i,
					beamflag[i], bath[i], bathacrosstrack[i],
					bathalongtrack[i]);
				}
			}
		fprintf(stderr, "dbg2       namp:       %d\n", namp);
		if (verbose >= 3)
			{
			for (i = 0; i < namp; i++)
				{
				fprintf(stderr,
					"dbg3        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i,
					amp[i], bathacrosstrack[i], bathalongtrack[i]);
				}
			}
		fprintf(stderr, "dbg2        nss:       %d\n", nss);
		if (verbose >= 3)
			{
			for (i = 0; i < nss; i++)
				{
				fprintf(stderr,
					"dbg3        beam:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i,
					ss[i], ssacrosstrack[i], ssalongtrack[i]);
				}
			}
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	header = (swpls_header *)&(store->sxp_header);
	sxp_ping = (swpls_sxpping *)&(store->sxp_ping);
	ocomment = (swpls_comment *)&(store->comment);

	/* set data kind */
	store->kind = kind;

	/* insert data in structure */
	if ((store->kind == MB_DATA_DATA) && (mb_io_ptr->format == MBF_SWPLSSXP))
		{
		/* get time */
		for (i = 0; i < 7; i++)
			{
			store->time_i[i] = time_i[i];
			}
		store->time_d = time_d;
		sxp_ping->time_d = time_d;

		/* get the file name */
		strncpy(path, mb_io_ptr->file, MB_PATH_MAXLINE);
		mb_get_basename(verbose, &path[0], error);
		strncpy(&(sxp_ping->linename[0]), &path[0], SWPLS_MAX_LINENAME);
		sxp_ping->linename[SWPLS_MAX_LINENAME - 1] = '\0';

		/* get navigation */
		sxp_ping->easting = navlon;
		sxp_ping->txer_e = navlon;
		sxp_ping->northing = navlat;
		sxp_ping->txer_n = navlat;

		/* get heading */
		sxp_ping->heading = heading;

		/* get speed */

		/* read distance and depth values into storage arrays */
		sxp_ping->nosampsfile = nbath;
		if (sxp_ping->points_alloc < nbath)
			{
			status =
				mb_reallocd(verbose, __FILE__, __LINE__,
				nbath * sizeof(swpls_point),
				(void **)&(sxp_ping->points), error);
			if (status != MB_SUCCESS)
				{
				sxp_ping->points_alloc = 0;
				}
			else
				{
				sxp_ping->points_alloc = nbath;
				}
			}

		if (status == MB_SUCCESS)
			{
			swpls_init_transform(verbose, &vtow, error);
			swpls_concat_rotate_y(verbose, &vtow, sxp_ping->heading * DTR,
				error);
			swpls_concat_translate(verbose, &vtow, sxp_ping->txer_e, 0.0,
				sxp_ping->txer_n, error);

			points = store->sxp_ping.points;
			for (i = 0; i < nbath; i++)
				{
				swpls_vector ppos;

				ppos.x = bathacrosstrack[i];
				ppos.y = -(bath[i]);
				ppos.z = bathalongtrack[i];

				swpls_transform(verbose, &vtow, &ppos, error);

				points[i].sampnum = i;
				points[i].x = ppos.x;
				points[i].y = ppos.z;
				points[i].z = -(ppos.y);
				points[i].procamp = (int)amp[i];

				if (beamflag[i] == MB_FLAG_NONE)
					{
					points[i].status = SWPLS_POINT_ACCEPTED;
					}
				else
					{
					points[i].status = SWPLS_POINT_REJECTED;
					}
				}
			}
		else
			{
			store->kind = MB_DATA_NONE;
			}
		}
	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT)
		{
		/* get time */
		if (time_d > 0.0)
			{
			for (i = 0; i < 7; i++)
				{
				store->time_i[i] = time_i[i];
				}
			store->time_d = time_d;
			}
		else
			{
			store->time_d = (double)time(NULL);
			mb_get_date(verbose, store->time_d, store->time_i);
			}
		ocomment->time_d = (int)trunc(store->time_d);
		ocomment->microsec = 0;

		/* allocate more memory for comment if necessary */
		nchars = strnlen(comment, MB_COMMENT_MAXLINE) + 1;
		if (ocomment->message_alloc < nchars)
			{
			status = mb_reallocd(verbose, __FILE__, __LINE__, nchars,
				(void **)&(ocomment->message), error);
			if (status != MB_SUCCESS)
				{
				ocomment->message_alloc = 0;
				}
			else
				{
				ocomment->message_alloc = nchars;
				}
			}

		if (status == MB_SUCCESS)
			{
			ocomment->nchars = nchars;
			strcpy(ocomment->message, comment);
			}
		else
			{
			kind = MB_DATA_NONE;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return (status);
}	/* mbsys_swathplus_insert */
/*--------------------------------------------------------------------*/
int mbsys_swathplus_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams, double *ttimes,
	double *angles, double *angles_forward,
	double *angles_null, double *heave,
	double *alongtrack_offset, double *draft,
	double *ssv, int *error)
{
	char *function_name = "mbsys_swathplus_ttimes";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	swpls_sxpping *sxp_ping;
	swpls_point *points;
	int type;
	double dist;
	double alpha, beta, theta, phi;
	int i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       ttimes:     %p\n", (void *)ttimes);
		fprintf(stderr, "dbg2       angles_xtrk:%p\n", (void *)angles);
		fprintf(stderr, "dbg2       angles_ltrk:%p\n", (void *)angles_forward);
		fprintf(stderr, "dbg2       angles_null:%p\n", (void *)angles_null);
		fprintf(stderr, "dbg2       heave:      %p\n", (void *)heave);
		fprintf(stderr, "dbg2       ltrk_off:   %p\n",
			(void *)alongtrack_offset);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	sxp_ping = (swpls_sxpping *)&(store->sxp_ping);

	/* get data kind */
	*kind = store->kind;
	type = store->type;

	/* extract data from structure */
	if ((*kind == MB_DATA_DATA) &&
		((type == SWPLS_ID_PROCESSED_PING) ||
		(type == SWPLS_ID_PROCESSED_PING2)))
		{
		double sxpheave;
		swpls_matrix wtov;

		/* surface sound velocity */
		*ssv = sxp_ping->sos;

		/* get draft */
		*draft = 0.0;

		/* get travel times and angles into storage arrays */
		get_sxp_heave(verbose, sxp_ping, &sxpheave, error);

		/* transform samples from world to body axis */
		swpls_init_transform(verbose, &wtov, error);
		swpls_concat_translate(verbose, &wtov, -sxp_ping->txer_e,
			-(-(sxp_ping->height - sxp_ping->tide)), -sxp_ping->txer_n, error);
		swpls_concat_rotate_y(verbose, &wtov, -sxp_ping->heading, error);

		points = sxp_ping->points;
		for (i = 0; i < sxp_ping->nosampsfile; i++)
			{
			swpls_vector ppos;

			/* point position relative to reference point */
			ppos.x = points[i].x;
			ppos.y = -(points[i].z);
			ppos.z = points[i].y;

			swpls_transform(verbose, &wtov, &ppos, error);

			/* estimate ttime from slant range */
			dist = sqrt(ppos.x * ppos.x + ppos.y * ppos.y + ppos.z * ppos.z);
			ttimes[i] = 2 * dist / *ssv;

			/* estimate takeoff angles from geometry */
			alpha = atan2(ppos.z, -ppos.y);
			beta = atan2(-ppos.y, ppos.x);

			mb_rollpitch_to_takeoff(verbose, alpha * RTD, beta * RTD, &theta,
				&phi, error);

			angles[i] = theta;
			angles_forward[i] = phi;
			angles_null[i] = 0.0;
			heave[i] = -sxpheave;
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
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		}
	if ((verbose >= 2) && (*error == MB_ERROR_NO_ERROR))
		{
		fprintf(stderr, "dbg2       draft:      %f\n", *draft);
		fprintf(stderr, "dbg2       ssv:        %f\n", *ssv);
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (i = 0; i < *nbeams; i++)
			{
			fprintf(stderr,
				"dbg2       beam %d: tt:%f  angle_xtrk:%f  angle_ltrk:%f  angle_null:%f  depth_off:%f  ltrk_off:%f\n", i,
				ttimes[i], angles[i], angles_forward[i], angles_null[i], heave[i],
				alongtrack_offset[i]);
			}
		}
	if (verbose >= 2)
		{
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}	/* mbsys_swathplus_ttimes */
/*--------------------------------------------------------------------*/
int mbsys_swathplus_detects(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams, int *detects, int *error)
{
	char *function_name = "mbsys_swathplus_detects";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	swpls_sxpping *sxp_ping;
	int i;
	int type;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       detects:    %p\n", (void *)detects);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	sxp_ping = (swpls_sxpping *)&(store->sxp_ping);

	/* get data kind */
	*kind = store->kind;
	type = store->type;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get detect type for each sounding - options include:
		    MB_DETECT_UNKNOWN
		    MB_DETECT_AMPLITUDE
		    MB_DETECT_PHASE
		    MB_DETECT_UNKNOWN */
		*nbeams = sxp_ping->nosampsfile;
		for (i = 0; i < *nbeams; i++)
			{
			detects[i] = MB_DETECT_PHASE;
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
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		}
	if ((verbose >= 2) && (*error == MB_ERROR_NO_ERROR))
		{
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (i = 0; i < *nbeams; i++)
			{
			fprintf(stderr, "dbg2       beam %d: detects:%d\n", i, detects[i]);
			}
		}
	if (verbose >= 2)
		{
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}	/* mbsys_swathplus_detects */
/*--------------------------------------------------------------------*/
int mbsys_swathplus_gains(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, double *transmit_gain,
	double *pulse_length, double *receive_gain,
	int *error)
{
	char *function_name = "mbsys_swathplus_gains";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	swpls_sxpping *sxp_ping;
	int type;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	sxp_ping = (swpls_sxpping *)&(store->sxp_ping);

	/* get data kind */
	*kind = store->kind;
	type = store->type;

	/* extract data from structure */
	if ((*kind == MB_DATA_DATA) &&
		((type == SWPLS_ID_PROCESSED_PING) ||
		(type == SWPLS_ID_PROCESSED_PING2)))
		{
		/* get transmit_gain (dB) */
		*transmit_gain = (double)sxp_ping->txpower;

		/* get pulse_length (usec) */
		*pulse_length = (double)sxp_ping->trnstime / sxp_ping->frequency * 1e6;

		/* get receive_gain (dB) */
		*receive_gain = (double)sxp_ping->analoggain;

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
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		}
	if ((verbose >= 2) && (*error == MB_ERROR_NO_ERROR))
		{
		fprintf(stderr, "dbg2       transmit_gain: %f\n", *transmit_gain);
		fprintf(stderr, "dbg2       pulse_length:  %f\n", *pulse_length);
		fprintf(stderr, "dbg2       receive_gain:  %f\n", *receive_gain);
		}
	if (verbose >= 2)
		{
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}	/* mbsys_swathplus_gains */
/*--------------------------------------------------------------------*/
int mbsys_swathplus_extract_altitude(int verbose, void *mbio_ptr,
	void *store_ptr, int *kind,
	double *transducer_depth, double *altitude,
	int *error)
{
	char *function_name = "mbsys_swathplus_extract_altitude";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	swpls_sxpping *sxp_ping;
	int i;
	int n;
	double sum, ave;
	int type;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	sxp_ping = &(store->sxp_ping);

	/* get data kind */
	*kind = store->kind;
	type = store->type;

	/* extract data from structure */
	if ((*kind == MB_DATA_DATA) &&
		((type == SWPLS_ID_PROCESSED_PING) ||
		(type == SWPLS_ID_PROCESSED_PING2)))
		{
		/* get transducer depth below the sea surface */
		*transducer_depth = sxp_ping->txer_waterdepth;	/* static draft */

		/* calculate mean depth of first 25 near-nadir samples */
		i = n = 0;
		sum = 0.0;
		while (i < sxp_ping->nosampsfile && n < 25)
			{
			if (sxp_ping->points[i].status != SWPLS_POINT_REJECTED)
				{
				sum += sxp_ping->points[i].z;
				n++;
				}
			i++;
			}

		/* get the transducer altitude above the seafloor */
		if (n > 0)
			{
			ave = sum / n;
			*altitude = ave - (sxp_ping->height - sxp_ping->tide);
			}
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
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:              %d\n", *kind);
		fprintf(stderr, "dbg2       transducer_depth:  %f\n",
			*transducer_depth);
		fprintf(stderr, "dbg2       altitude:          %f\n", *altitude);
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:            %d\n", status);
		}

	/* return status */
	return (status);
}	/* mbsys_swathplus_extract_altitude */
/*--------------------------------------------------------------------*/
int mbsys_swathplus_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int time_i[7], double *time_d,
	double *navlon, double *navlat, double *speed,
	double *heading, double *draft, double *roll,
	double *pitch, double *heave, int *error)
{
	char *function_name = "mbsys_swathplus_extract_nav";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	swpls_sxpping *sxp_ping;
	int i;
	int type;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	sxp_ping = (swpls_sxpping *)&(store->sxp_ping);

	/* get data kind */
	*kind = store->kind;
	type = store->type;

	/* extract data from survey record */
	if ((*kind == MB_DATA_DATA) &&
		((type == SWPLS_ID_PROCESSED_PING) ||
		(type == SWPLS_ID_PROCESSED_PING2)))
		{
		double sxpheave;

		/* get time */
		for (i = 0; i < 7; i++)
			{
			time_i[i] = store->time_i[i];
			}
		*time_d = store->time_d;

		/* get navigation */
		*navlon = sxp_ping->txer_e;
		*navlat = sxp_ping->txer_n;

		/* get speed */
		*speed = 0.0;
		if (mb_io_ptr->nfix > 0)
			{
			mb_navint_prjinterp(verbose, mbio_ptr, store->time_d, *heading, *speed,
				navlon, navlat, speed, error);
			}

		/* get heading */
		*heading = sxp_ping->heading;

		/* get draft  */
		*draft = sxp_ping->txer_waterdepth;

		/* get attitude  */
		*roll = -(sxp_ping->roll);
		*pitch = sxp_ping->pitch;

		/* calculate heave */
		get_sxp_heave(verbose, sxp_ping, &sxpheave, error);
		*heave = -sxpheave;

		/* done translating values */
		}
	/* extract data from nav record */

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT)
		{
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;

		/* get time */
		for (i = 0; i < 7; i++)
			{
			time_i[i] = store->time_i[i];
			}
		*time_d = store->time_d;
		}
	/* deal with other record type */
	else
		{
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;

		/* get time */
		for (i = 0; i < 7; i++)
			{
			time_i[i] = store->time_i[i];
			}
		*time_d = store->time_d;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:          %d\n", *kind);
		fprintf(stderr, "dbg2       time_i[0]:     %d\n", time_i[0]);
		fprintf(stderr, "dbg2       time_i[1]:     %d\n", time_i[1]);
		fprintf(stderr, "dbg2       time_i[2]:     %d\n", time_i[2]);
		fprintf(stderr, "dbg2       time_i[3]:     %d\n", time_i[3]);
		fprintf(stderr, "dbg2       time_i[4]:     %d\n", time_i[4]);
		fprintf(stderr, "dbg2       time_i[5]:     %d\n", time_i[5]);
		fprintf(stderr, "dbg2       time_i[6]:     %d\n", time_i[6]);
		fprintf(stderr, "dbg2       time_d:        %f\n", *time_d);
		fprintf(stderr, "dbg2       longitude:     %f\n", *navlon);
		fprintf(stderr, "dbg2       latitude:      %f\n", *navlat);
		fprintf(stderr, "dbg2       speed:         %f\n", *speed);
		fprintf(stderr, "dbg2       heading:       %f\n", *heading);
		fprintf(stderr, "dbg2       draft:         %f\n", *draft);
		fprintf(stderr, "dbg2       roll:          %f\n", *roll);
		fprintf(stderr, "dbg2       pitch:         %f\n", *pitch);
		fprintf(stderr, "dbg2       heave:         %f\n", *heave);
		fprintf(stderr, "dbg2       error:         %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:        %d\n", status);
		}

	/* return status */
	return (status);
}	/* mbsys_swathplus_extract_nav */
/*--------------------------------------------------------------------*/
int mbsys_swathplus_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
	int time_i[7], double time_d, double navlon,
	double navlat, double speed, double heading,
	double draft, double roll, double pitch,
	double heave, int *error)
{
	char *function_name = "mbsys_swathplus_insert_nav";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	swpls_sxpping *sxp_ping;
	int i;
	int kind, type;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       time_i[0]:  %d\n", time_i[0]);
		fprintf(stderr, "dbg2       time_i[1]:  %d\n", time_i[1]);
		fprintf(stderr, "dbg2       time_i[2]:  %d\n", time_i[2]);
		fprintf(stderr, "dbg2       time_i[3]:  %d\n", time_i[3]);
		fprintf(stderr, "dbg2       time_i[4]:  %d\n", time_i[4]);
		fprintf(stderr, "dbg2       time_i[5]:  %d\n", time_i[5]);
		fprintf(stderr, "dbg2       time_i[6]:  %d\n", time_i[6]);
		fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
		fprintf(stderr, "dbg2       navlon:     %f\n", navlon);
		fprintf(stderr, "dbg2       navlat:     %f\n", navlat);
		fprintf(stderr, "dbg2       speed:      %f\n", speed);
		fprintf(stderr, "dbg2       heading:    %f\n", heading);
		fprintf(stderr, "dbg2       draft:      %f\n", draft);
		fprintf(stderr, "dbg2       roll:       %f\n", roll);
		fprintf(stderr, "dbg2       pitch:      %f\n", pitch);
		fprintf(stderr, "dbg2       heave:      %f\n", heave);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	sxp_ping = (swpls_sxpping *)&(store->sxp_ping);

	/* get data kind */
	kind = store->kind;
	type = store->type;

	/* insert data in ping structure */
	if ((kind == MB_DATA_DATA) &&
		((type == SWPLS_ID_PROCESSED_PING) ||
		(type == SWPLS_ID_PROCESSED_PING2)))
		{
		swpls_matrix vtow, m;
		swpls_vector old_txoffset, new_txoffset;
		swpls_point *points;
		double height;

		/* calculate old transducer offsets */
		old_txoffset.x = sxp_ping->txer_starboard;
		old_txoffset.y = -sxp_ping->txer_height;
		old_txoffset.z = sxp_ping->txer_forward;
		swpls_init_transform(verbose, &vtow, error);
		swpls_concat_rotate_z(verbose, &vtow, -sxp_ping->roll * DTR, error);
		swpls_concat_rotate_x(verbose, &vtow, -sxp_ping->pitch * DTR, error);
		swpls_concat_rotate_y(verbose, &vtow, sxp_ping->heading * DTR, error);
		swpls_transform(verbose, &vtow, &old_txoffset, error);

		/* calculate new transducer offsets */
		new_txoffset.x = sxp_ping->txer_starboard;
		new_txoffset.y = -sxp_ping->txer_height;
		new_txoffset.z = sxp_ping->txer_forward;
		swpls_init_transform(verbose, &vtow, error);
		swpls_concat_rotate_z(verbose, &vtow, -(-roll) * DTR, error);
		swpls_concat_rotate_x(verbose, &vtow, -pitch * DTR, error);
		swpls_concat_rotate_y(verbose, &vtow, heading * DTR, error);
		swpls_transform(verbose, &vtow, &new_txoffset, error);
		height = -heave - new_txoffset.y;

		/* calculate point transformation */

		/* from old world coordinates to vessel body coordinates*/
		swpls_init_transform(verbose, &m, error);
		swpls_concat_translate(verbose, &m, -(sxp_ping->txer_e),
			-(-(sxp_ping->height - sxp_ping->tide)), -sxp_ping->txer_n, error);
		swpls_concat_translate(verbose, &m, -old_txoffset.x, -old_txoffset.y,
			-old_txoffset.z, error);
		swpls_concat_rotate_y(verbose, &m, -(sxp_ping->heading) * DTR, error);
		swpls_concat_rotate_x(verbose, &m, -(-sxp_ping->pitch) * DTR, error);
		swpls_concat_rotate_z(verbose, &m, -(-sxp_ping->roll) * DTR, error);

		/* from vessel body coordinates to new world coordinates */
		swpls_concat_rotate_z(verbose, &m, +(-roll) * DTR, error);
		swpls_concat_rotate_x(verbose, &m, +(-pitch) * DTR, error);
		swpls_concat_rotate_y(verbose, &m, +heading * DTR, error);
		swpls_concat_translate(verbose, &m, new_txoffset.x, new_txoffset.y,
			new_txoffset.z, error);
		swpls_concat_translate(verbose, &m, navlon, (height - sxp_ping->tide), navlat, error);

		/* transform points from old to new coordinates */
		points = sxp_ping->points;
		for (i = 0; i < sxp_ping->nosampsfile; i++)
			{
			swpls_vector p;

			p.x = points[i].x;
			p.y = -points[i].z;
			p.z = points[i].y;
			swpls_transform(verbose, &m, &p, error);
			points[i].x = p.x;
			points[i].y = p.z;
			points[i].z = -p.y;
			}

		/* set time */
		for (i = 0; i < 7; i++)
			{
			store->time_i[i] = time_i[i];
			}
		store->time_d = time_d;

		/* set navigation */
		store->sxp_ping.txer_e = navlon;
		store->sxp_ping.txer_n = navlat;
		store->sxp_ping.easting = navlon;
		store->sxp_ping.northing = navlat;

		/* set speed  (no space in structure) */

		/* set draft  (no space in structure) */

		/* set attitude */
		store->sxp_ping.height = height;
		store->sxp_ping.pitch = pitch;
		store->sxp_ping.roll = -roll;

		/* print output debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
				function_name);
			fprintf(stderr, "dbg2  Return value:\n");
			fprintf(stderr, "dbg2       error:      %d\n", *error);
			fprintf(stderr, "dbg2  Return status:\n");
			fprintf(stderr, "dbg2       status:  %d\n", status);
			}
		}

	return (status);
}	/* mbsys_swathplus_insert_nav */
/*--------------------------------------------------------------------*/
int mbsys_swathplus_copy(int verbose, void *mbio_ptr, void *store_ptr,
	void *copy_ptr, int *error)
{
	char *function_name = "mbsys_swathplus_copy";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	struct mbsys_swathplus_struct *copy;
	size_t points_alloc_save;
	swpls_point *points_save;
	size_t samps_alloc_save;
	unsigned short *sampnum_save;
	short int *angle_save;
	unsigned short *amplitude_save;
	unsigned char *quality_save;
	size_t projection_alloc_save;
	char *projection_id_save;
	size_t message_alloc_save;
	char *message_save;
	size_t copy_len;
	int i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       copy_ptr:   %p\n", (void *)copy_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointers */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	copy = (struct mbsys_swathplus_struct *)copy_ptr;

	/* copy the data - for many formats memory must be allocated and
	    sub-structures copied separately */
	copy->kind = store->kind;
	copy->type = store->type;

	/* copy MB-System time stamp of most recently read record */
	copy->time_d = store->time_d;
	for (i = 0; i < 7; i++)
		{
		copy->time_i[i] = store->time_i[i];
		}

	/* copy HEADER records */
	copy->sxp_header_set = store->sxp_header_set;
	copy->sxp_header = store->sxp_header;
	copy->sxi_header_set = store->sxi_header_set;
	copy->sxi_header = store->sxi_header;

	/* copy SXP PING record */
	points_alloc_save = copy->sxp_ping.points_alloc;
	points_save = copy->sxp_ping.points;
	copy->sxp_ping = store->sxp_ping;
	copy->sxp_ping.points_alloc = points_alloc_save;
	copy->sxp_ping.points = points_save;
	copy_len = (size_t)store->sxp_ping.points_alloc * sizeof(swpls_point);
	if ((status == MB_SUCCESS) &&
		(copy->sxp_ping.points_alloc < store->sxp_ping.points_alloc))
		{
		status =
			mb_reallocd(verbose, __FILE__, __LINE__, copy_len,
			(void **)&(copy->sxp_ping.points), error);
		if (status == MB_SUCCESS)
			{
			copy->sxp_ping.points_alloc = store->sxp_ping.points_alloc;
			}
		else
			{
			copy->sxp_ping.points_alloc = 0;
			}
		}
	if (status == MB_SUCCESS)
		{
		memcpy(copy->sxp_ping.points, store->sxp_ping.points, copy_len);
		}

	/* copy SXI PING record */
	samps_alloc_save = copy->sxi_ping.samps_alloc;
	sampnum_save = copy->sxi_ping.sampnum;
	angle_save = copy->sxi_ping.angle;
	amplitude_save = copy->sxi_ping.amplitude;
	quality_save = copy->sxi_ping.quality;
	copy->sxi_ping = store->sxi_ping;
	copy->sxi_ping.samps_alloc = samps_alloc_save;
	copy->sxi_ping.sampnum = sampnum_save;
	copy->sxi_ping.angle = angle_save;
	copy->sxi_ping.amplitude = amplitude_save;
	copy->sxi_ping.quality = quality_save;
	if ((status == MB_SUCCESS) &&
		(copy->sxi_ping.samps_alloc < store->sxi_ping.samps_alloc))
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__,
			store->sxi_ping.samps_alloc * sizeof(unsigned short),
			(void **)&(copy->sxi_ping.sampnum), error);
		if (status == MB_SUCCESS)
			{
			status = mb_reallocd(verbose, __FILE__, __LINE__,
				store->sxi_ping.samps_alloc * sizeof(short int),
				(void **)&(copy->sxi_ping.angle), error);
			}
		if (status == MB_SUCCESS)
			{
			status = mb_reallocd(verbose, __FILE__, __LINE__,
				store->sxi_ping.samps_alloc *
				sizeof(unsigned short), (void **)&(copy->sxi_ping.amplitude),
				error);
			}
		if (status == MB_SUCCESS)
			{
			status = mb_reallocd(verbose, __FILE__, __LINE__,
				store->sxi_ping.samps_alloc *
				sizeof(unsigned char), (void **)&(copy->sxi_ping.quality),
				error);
			}
		if (status == MB_SUCCESS)
			{
			copy->sxi_ping.samps_alloc = store->sxi_ping.samps_alloc;
			}
		else
			{
			copy->sxi_ping.samps_alloc = 0;
			}
		}
	if (status == MB_SUCCESS)
		{
		memcpy(copy->sxi_ping.sampnum, store->sxi_ping.sampnum,
			store->sxi_ping.samps_alloc * sizeof(unsigned short));
		memcpy(copy->sxi_ping.angle, store->sxi_ping.angle,
			store->sxi_ping.samps_alloc * sizeof(short int));
		memcpy(copy->sxi_ping.amplitude, store->sxi_ping.amplitude,
			store->sxi_ping.samps_alloc * sizeof(unsigned short));
		memcpy(copy->sxi_ping.quality, store->sxi_ping.quality,
			store->sxi_ping.samps_alloc * sizeof(unsigned char));
		}

	/* copy ATTITUDE */
	copy->attitude = store->attitude;

	/* copy POSLL */
	copy->posll = store->posll;

	/* copy POSEN */
	copy->posen = store->posen;

	/* copy SOS */
	copy->ssv = store->ssv;

	/* copy TIDE */
	copy->tide = store->tide;

	/* copy ECHOSOUNDER */
	copy->echosounder = store->echosounder;

	/* copy PROJECTION */
	copy->projection_set = store->projection_set;
	projection_alloc_save = copy->projection.projection_alloc;
	projection_id_save = copy->projection.projection_id;
	copy->projection = store->projection;
	copy->projection.projection_alloc = projection_alloc_save;
	copy->projection.projection_id = projection_id_save;
	if (copy->projection.projection_alloc < store->projection.projection_alloc)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__,
			store->projection.projection_alloc,
			(void **)&(copy->projection.projection_id), error);
		if (status == MB_SUCCESS)
			{
			copy->projection.projection_alloc = store->projection.projection_alloc;
			}
		else
			{
			copy->projection.projection_alloc = 0;
			}
		}
	if (status == MB_SUCCESS)
		{
		strncpy(copy->projection.projection_id, store->projection.projection_id,
			store->projection.projection_alloc);
		}

	/* copy COMMENT */
	message_alloc_save = copy->comment.message_alloc;
	message_save = copy->comment.message;
	copy->comment = store->comment;
	copy->comment.message_alloc = message_alloc_save;
	copy->comment.message = message_save;
	if (copy->comment.message_alloc < store->comment.message_alloc)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__,
			store->comment.message_alloc,
			(void **)&(copy->comment.message), error);
		if (status == MB_SUCCESS)
			{
			copy->comment.message_alloc = store->comment.message_alloc;
			}
		else
			{
			copy->comment.message_alloc = 0;
			}
		}
	if (status == MB_SUCCESS)
		{
		strncpy(copy->comment.message, store->comment.message,
			store->comment.message_alloc);
		}

	/* copy POS OFFSET */
	copy->pos_offset = store->pos_offset;

	/* copy IMU OFFSET */
	copy->imu_offset = store->imu_offset;

	/* copy TXER OFFSET */
	copy->txer_offset = store->txer_offset;

	/* copy WL OFFSET */
	copy->wl_offset = store->wl_offset;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}		/* mbsys_swathplus_copy */
/*-------------------------------------------------------------------*/
int swpls_chk_header(int verbose, void *mbio_ptr, char *buffer, int *recordid,
	int *size, int *error)
{
	char *function_name = "swpls_chk_header";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:       %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:      %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       buffer:        %p\n", (void *)buffer);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get values to check */
	index = 0;
	mb_get_binary_int(MB_YES, &buffer[index], recordid); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], size); index += 4;

	/* check recordid */
	if ((*recordid != SWPLS_ID_SXP_HEADER_DATA) &&
		(*recordid != SWPLS_ID_PROCESSED_PING) &&
		(*recordid != SWPLS_ID_PROCESSED_PING2) &&
		(*recordid != SWPLS_ID_COMMENT) &&
		(*recordid != SWPLS_ID_PROJECTION) &&
		(*recordid != SWPLS_ID_SXI_HEADER_DATA) &&
		(*recordid != SWPLS_ID_PARSED_PING) &&
		(*recordid != SWPLS_ID_PARSED_ATTITUDE) &&
		(*recordid != SWPLS_ID_PARSED_POSITION_LL) &&
		(*recordid != SWPLS_ID_PARSED_POSITION_EN) &&
		(*recordid != SWPLS_ID_PARSED_SSV) &&
		(*recordid != SWPLS_ID_PARSED_ECHOSOUNDER) &&
		(*recordid != SWPLS_ID_PARSED_TIDE) &&
		(*recordid != SWPLS_ID_PARSED_TIDE) &&
		(*recordid != SWPLS_ID_PARSED_AGDS) &&
		(*recordid != SWPLS_ID_POS_OFFSET) &&
		(*recordid != SWPLS_ID_IMU_OFFSET) &&
		(*recordid != SWPLS_ID_TXER_OFFSET) &&
		(*recordid != SWPLS_ID_WL_OFFSET))
		{
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}
	else
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Output arguments:\n");
		fprintf(stderr, "dbg2       recordid:      %d\n", *recordid);
		fprintf(stderr, "dbg2       size:          %d\n", *size);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:        %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_chk_header */
/* --------------------------------------------------------------------*/
int swpls_rd_sxpheader(int verbose, char *buffer, void *store_ptr, int *error)
{
	char *function_name = "swpls_rd_sxpheader";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_header *header;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	header = &(store->sxp_header);

	/* extract the data */
	index = SWPLS_SIZE_BLOCKHEADER;
	mb_get_binary_int(MB_YES, &buffer[index], &(header->swver)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(header->fmtver)); index += 4;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		store->kind = MB_DATA_HEADER;
		store->type = SWPLS_ID_SXP_HEADER_DATA;
		store->sxp_header_set = MB_YES;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2      header:     %p\n", header);
		fprintf(stderr, "dbg2       error:     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	return (status);
}		/* swpls_rd_sxpheader */
/*--------------------------------------------------------------------*/
int swpls_rd_sxpping(int verbose, char *buffer, void *store_ptr, int pingtype,
	int *error)
{
	char *function_name = "swpls_rd_sxpping";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_sxpping *ping;
	int index;
	int int_val;
	short int short_val;
	int i;
	size_t read_len;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       pingtype:   %d\n", pingtype);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	ping = &(store->sxp_ping);

	/* extract the data */
	index = SWPLS_SIZE_BLOCKHEADER;
	strncpy(&buffer[index], ping->linename,
		SWPLS_MAX_LINENAME); index += SWPLS_MAX_LINENAME;
	ping->linename[SWPLS_MAX_LINENAME - 1] = '\0';
	mb_get_binary_int(MB_YES, &buffer[index], &int_val); index += 4;
	ping->pingnumber = (unsigned int)int_val;
	index += 4;		/* padding bytes */
	mb_get_binary_double(MB_YES, &buffer[index], &(ping->time_d)); index += 8;
	mb_get_binary_int(MB_YES, &buffer[index], &(ping->notxers)); index += 4;
	index += 4;		/* padding bytes */
	mb_get_binary_double(MB_YES, &buffer[index], &(ping->easting)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(ping->northing));
	index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(ping->roll)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(ping->pitch)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(ping->heading)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(ping->height)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(ping->tide)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(ping->sos)); index += 8;
	ping->txno = buffer[index++];
	ping->txstat = buffer[index++];
	ping->txpower = buffer[index++];
	index += 1;		/* padding byte */
	mb_get_binary_short(MB_YES, &buffer[index], &(ping->analoggain));
	index += 2;
	ping->nostaves = buffer[index++];
	for (i = 0; i < SWPLS_MAX_TX_INFO; i++)
		{
		ping->txinfo[i] = buffer[index++];
		}
	index += 1;		/* padding bytes */
	ping->freq = buffer[index++];
	index += 4;		/* padding bytes */
	mb_get_binary_double(MB_YES, &buffer[index], &(ping->frequency));
	index += 8;
	mb_get_binary_short(MB_YES, &buffer[index], &(ping->trnstime)); index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(ping->recvtime)); index += 2;
	ping->samprate = buffer[index++];
	index += 3;		/* padding bytes */
	mb_get_binary_int(MB_YES, &buffer[index], &(ping->nosampsorig));
	index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(ping->nosampsfile));
	index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(ping->nosampslots));
	index += 4;
	index += 4;		/* padding bytes */
	mb_get_binary_double(MB_YES, &buffer[index], &(ping->txer_e)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(ping->txer_n)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(ping->txer_height));
	index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(ping->txer_forward));
	index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(ping->txer_starboard));
	index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(ping->txer_azimuth));
	index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(ping->txer_elevation));
	index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(ping->txer_skew));
	index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(ping->txer_time));
	index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(ping->txer_waterdepth));
	index += 8;

	if (pingtype == SWPLS_ID_PROCESSED_PING)
		{
		ping->txer_pitch = 0.0;
		}
	else if (pingtype == SWPLS_ID_PROCESSED_PING2)
		{
		mb_get_binary_double(MB_YES, &buffer[index], &(ping->txer_pitch));
		index += 8;
		}

	/* check that we have enough storage for the points stored in the file
	 */
	if (ping->points_alloc < ping->nosampsfile)
		{
		read_len = (size_t)(ping->nosampsfile * sizeof(swpls_point));
		status =
			mb_reallocd(verbose, __FILE__, __LINE__, read_len,
			(void **)&(ping->points), error);
		if (status != MB_SUCCESS)
			{
			ping->points_alloc = 0;
			}
		else
			{
			ping->points_alloc = ping->nosampsfile;
			}
		}

	/* extract the point data from the buffer into the ping struct */
	if (status == MB_SUCCESS)
		{
		for (i = 0; i < ping->nosampsfile; i++)
			{
			mb_get_binary_int(MB_YES, &buffer[index],
				&(ping->points[i].sampnum)); index += 4;
			index += 4;		/* padding bytes */
			mb_get_binary_double(MB_YES, &buffer[index], &(ping->points[i].y));
			index += 8;
			mb_get_binary_double(MB_YES, &buffer[index], &(ping->points[i].x));
			index += 8;
			mb_get_binary_float(MB_YES, &buffer[index], &(ping->points[i].z));
			index += 4;
			mb_get_binary_short(MB_YES, &buffer[index], &short_val);
			index += 2;
			ping->points[i].amp = (unsigned short)short_val;
			mb_get_binary_short(MB_YES, &buffer[index], &short_val);
			index += 2;
			ping->points[i].procamp = (unsigned short)short_val;
			ping->points[i].status = buffer[index++];
			index += 7;		/* padding bytes */

			/* old-syle points don't have tpu parameter */
			if (pingtype == SWPLS_ID_PROCESSED_PING)
				{
				ping->points[i].tpu = 0.0;
				}
			else if (pingtype == SWPLS_ID_PROCESSED_PING2)
				{
				mb_get_binary_double(MB_YES, &buffer[index],
					&(ping->points[i].tpu)); index += 8;
				}
			}
		}

	if (status == MB_SUCCESS)
		{
		/* set the time and date */
		store->time_d = ping->time_d;
		mb_get_date(verbose, ping->time_d, &(store->time_i[0]));

		/* set the kind and type */
		store->kind = MB_DATA_DATA;
		store->type = pingtype;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	if (verbose >= 4)
		{
		fprintf(stderr, "\ndbg4 SXP Ping values read from file:\n");
		swpls_pr_sxpping(verbose, stderr, ping, error);
		}
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2        ping:      %p\n", ping);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	return (status);
}		/* swpls_rd_sxpping */
/*--------------------------------------------------------------------*/
int swpls_rd_projection(int verbose, char *buffer, void *store_ptr, int *error)
{
	char *function_name = "swpls_rd_projection";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_projection *projection;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;

	/* only read the projection if not previously set */
	if (store->projection_set == MB_NO)
		{
		projection = &(store->projection);

		/* extract the data */
		index = SWPLS_SIZE_BLOCKHEADER;
		mb_get_binary_int(MB_YES, &buffer[index], &(projection->time_d));
		index += 4;
		mb_get_binary_int(MB_YES, &buffer[index], &(projection->microsec));
		index += 4;
		mb_get_binary_int(MB_YES, &buffer[index], &(projection->nchars));
		index += 4;

		/* allocated memory to hold the message if necessary */
		if (projection->projection_alloc < projection->nchars)
			{
			status = mb_reallocd(verbose, __FILE__, __LINE__,
				(size_t)projection->nchars,
				(void **)&(projection->projection_id), error);
			if (status != MB_SUCCESS)
				{
				projection->projection_alloc = 0;
				}
			else
				{
				projection->projection_alloc = projection->nchars;
				}
			}

		if (status == MB_SUCCESS)
			{
			strncpy(&(projection->projection_id[0]), &buffer[index],
				(size_t)projection->nchars); index += projection->nchars;
			}

		if (status == MB_SUCCESS)
			{
			/* set the date and time */
			store->time_d = (double)projection->time_d +
				(double)projection->microsec * 1e-6;
			mb_get_date(verbose, store->time_d, &(store->time_i[0]));

			/* set the kind and type */
			store->kind = MB_DATA_PARAMETER;
			store->type = SWPLS_ID_PROJECTION;
			store->projection_set = MB_YES;
			}
		else
			{
			store->kind = MB_DATA_NONE;
			}
		}

	/* print debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:                 %d\n", 
			*error);
		fprintf(stderr, "dbg2       store->projection_set: %d\n", 
			store->projection_set);
		fprintf(stderr, "dbg2       store->projection_id:  %s\n",
			projection->projection_id);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	return (status);
}		/* swpls_rd_projection */
/*--------------------------------------------------------------------*/
int swpls_rd_comment(int verbose, char *buffer, void *store_ptr, int *error)
{
	char *function_name = "swpls_rd_comment";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_comment *comment;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	comment = &(store->comment);

	/* extract the data */
	index = SWPLS_SIZE_BLOCKHEADER;
	mb_get_binary_int(MB_YES, &buffer[index], &(comment->time_d)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(comment->microsec));
	index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(comment->nchars)); index += 4;

	/* allocated memory to hold the message if necessary */
	if (comment->message_alloc < comment->nchars)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__,
			(size_t)comment->nchars,
			(void **)&(comment->message), error);
		if (status != MB_SUCCESS)
			{
			comment->message_alloc = 0;
			}
		else
			{
			comment->message_alloc = comment->nchars;
			}
		}

	if (status == MB_SUCCESS)
		{
		strncpy(&(comment->message[0]), &buffer[index],
			(size_t)comment->nchars); index += comment->nchars;
		}

	if (status == MB_SUCCESS)
		{
		/* set the date and time */
		store->time_d = (double)comment->time_d + (double)comment->microsec *
			1e-6;
		mb_get_date(verbose, store->time_d, &(store->time_i[0]));

		/* set the kind and type */
		store->kind = MB_DATA_COMMENT;
		store->type = SWPLS_ID_COMMENT;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	return (status);
}		/* swpls_rd_comment */
/*--------------------------------------------------------------------*/
int swpls_rd_sxiheader(int verbose, char *buffer, void *store_ptr, int *error)
{
	char *function_name = "swpls_rd_sxiheader";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_header *header;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n:");
		fprintf(stderr, "dbg2     verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2     buffer:           %p\n", (void *)buffer);
		fprintf(stderr, "dbg2     store_ptr:        %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	header = &(store->sxi_header);

	/* extract the data */
	index = SWPLS_SIZE_BLOCKHEADER;
	mb_get_binary_int(MB_YES, &buffer[index], &(header->swver)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(header->fmtver)); index += 4;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		store->kind = MB_DATA_HEADER;
		store->type = SWPLS_ID_SXI_HEADER_DATA;
		store->sxi_header_set = MB_YES;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Output arguments:\n");
		fprintf(stderr, "dbg2      header:         %p\n", header);
		fprintf(stderr, "dbg2      error:          %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2      status:         %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_rd_sxiheader */
/*--------------------------------------------------------------------*/
int swpls_rd_sxiping(int verbose, char *buffer, void *store_ptr, int *error)
{
	char *function_name = "swpls_rd_sxiping";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_sxiping *ping;
	short short_val;
	int int_val;
	int index;
	size_t read_len;
	int i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n:");
		fprintf(stderr, "dbg2     verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2     buffer:           %p\n", (void *)buffer);
		fprintf(stderr, "dbg2     store_ptr:        %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	ping = &(store->sxi_ping);

	/* extract the data */
	index = SWPLS_SIZE_BLOCKHEADER;
	mb_get_binary_int(MB_YES, &buffer[index], &(ping->time_d)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(ping->microsec)); index += 4;
	ping->channel = buffer[index]; index++;
	mb_get_binary_int(MB_YES, &buffer[index], &int_val); index += 4;
	ping->pingnumber = (unsigned long)int_val;
	mb_get_binary_float(MB_YES, &buffer[index], &(ping->frequency));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(ping->samp_period));
	index += 4;
	mb_get_binary_short(MB_YES, &buffer[index], &short_val); index += 2;
	ping->nosamps = (unsigned short)short_val;
	mb_get_binary_float(MB_YES, &buffer[index], &(ping->sos)); index += 4;
	mb_get_binary_short(MB_YES, &buffer[index], &(ping->txpulse)); index += 2;
	ping->data_options = buffer[index]; index++;
	ping->ping_state = (unsigned char)buffer[index]; index++;
	mb_get_binary_short(MB_YES, &buffer[index], &short_val); index += 2;
	ping->max_count = (unsigned short)short_val;
	mb_get_binary_short(MB_YES, &buffer[index], &short_val); index += 2;
	ping->reserve1 = (unsigned short)short_val;

	/* check that we have enough storage for the samples stored in the file
	 */
	if (ping->samps_alloc < ping->nosamps)
		{
		read_len = (size_t)(ping->nosamps * sizeof(unsigned short));
		status =
			mb_reallocd(verbose, __FILE__, __LINE__, read_len,
			(void **)&(ping->sampnum), error);
		if (status == MB_SUCCESS)
			{
			read_len = (size_t)(ping->nosamps * sizeof(short int));
			status = mb_reallocd(verbose, __FILE__, __LINE__, read_len,
				(void **)&(ping->angle), error);
			}
		if (status == MB_SUCCESS)
			{
			read_len = (size_t)(ping->nosamps * sizeof(unsigned short));
			status = mb_reallocd(verbose, __FILE__, __LINE__, read_len,
				(void **)&(ping->amplitude), error);
			}
		if (status == MB_SUCCESS)
			{
			read_len = (size_t)(ping->nosamps * sizeof(unsigned char));
			status = mb_reallocd(verbose, __FILE__, __LINE__, read_len,
				(void **)&(ping->quality), error);
			}
		if (status != MB_SUCCESS)
			{
			ping->samps_alloc = 0;
			}
		else
			{
			ping->samps_alloc = ping->nosamps;
			}
		}

	/* extract the sample data from the buffer into the ping struct */
	if (status == MB_SUCCESS)
		{
		for (i = 0; i < ping->nosamps; i++)
			{
			mb_get_binary_short(MB_YES, &buffer[index], &short_val);
			index += 2;
			ping->sampnum[i] = (unsigned short)short_val;
			mb_get_binary_short(MB_YES, &buffer[index], &(ping->angle[i]));
			index += 2;
			mb_get_binary_short(MB_YES, &buffer[index], &short_val);
			index += 2;
			ping->amplitude[i] = (unsigned short)short_val;
			ping->quality[i] = (unsigned char)buffer[index]; index++;
			}
		}

	if (status == MB_SUCCESS)
		{
		/* set the date and time */
		store->time_d = (double)ping->time_d + (double)ping->microsec * 1e6;
		mb_get_date(verbose, store->time_d, &(store->time_i[0]));

		/* set the kind and type */
		store->kind = MB_DATA_DATA;
		store->type = SWPLS_ID_PARSED_PING;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2      error:          %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2      status:         %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_rd_sxiping */
/*--------------------------------------------------------------------*/
int swpls_rd_attitude(int verbose, char *buffer, void *store_ptr, int *error)
{
	char *function_name = "swpls_rd_attitude";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_attitude *attitude;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n:");
		fprintf(stderr, "dbg2     verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2     buffer:           %p\n", (void *)buffer);
		fprintf(stderr, "dbg2     store_ptr:        %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	attitude = &(store->attitude);

	/* extract the data */
	index = SWPLS_SIZE_BLOCKHEADER;
	mb_get_binary_int(MB_YES, &buffer[index], &(attitude->time_d)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(attitude->microsec));
	index += 4;
	attitude->channel = buffer[index]; index++;
	mb_get_binary_float(MB_YES, &buffer[index], &(attitude->roll)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(attitude->pitch));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(attitude->heading));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(attitude->height));
	index += 4;

	if (status == MB_SUCCESS)
		{
		/* set the date and time */
		store->time_d = (double)attitude->time_d + (double)attitude->microsec *
			1e6;
		mb_get_date(verbose, store->time_d, &(store->time_i[0]));

		/* set the kind and type */
		store->kind = MB_DATA_ATTITUDE;
		store->type = SWPLS_ID_PARSED_ATTITUDE;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2      error:          %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2      status:         %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_rd_attitude */
/*--------------------------------------------------------------------*/
int swpls_rd_posll(int verbose, char *buffer, void *store_ptr, int *error)
{
	char *function_name = "swpls_rd_posll";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_posll *posll;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n:");
		fprintf(stderr, "dbg2     verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2     buffer:           %p\n", (void *)buffer);
		fprintf(stderr, "dbg2     store_ptr:        %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	posll = &(store->posll);

	/* extract the data */
	index = SWPLS_SIZE_BLOCKHEADER;
	mb_get_binary_int(MB_YES, &buffer[index], &(posll->time_d)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(posll->microsec)); index += 4;
	posll->channel = buffer[index]; index++;
	mb_get_binary_double(MB_YES, &buffer[index], &(posll->latitude));
	index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(posll->longitude));
	index += 8;

	if (status == MB_SUCCESS)
		{
		/* set the date and time */
		store->time_d = (double)posll->time_d + (double)posll->microsec * 1e6;
		mb_get_date(verbose, store->time_d, &(store->time_i[0]));

		/* set the kind and type */
		store->kind = MB_DATA_NAV;
		store->type = SWPLS_ID_PARSED_POSITION_LL;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2      error:          %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2      status:         %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_rd_posll */
/*--------------------------------------------------------------------*/
int swpls_rd_posen(int verbose, char *buffer, void *store_ptr, int *error)
{
	char *function_name = "swpls_rd_posen";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_posen *posen;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n:");
		fprintf(stderr, "dbg2     verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2     buffer:           %p\n", (void *)buffer);
		fprintf(stderr, "dbg2     store_ptr:        %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	posen = &(store->posen);

	/* extract the data */
	index = SWPLS_SIZE_BLOCKHEADER;
	mb_get_binary_int(MB_YES, &buffer[index], &(posen->time_d)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(posen->microsec)); index += 4;
	posen->channel = buffer[index]; index++;
	mb_get_binary_double(MB_YES, &buffer[index], &(posen->easting));
	index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(posen->northing));
	index += 8;

	if (status == MB_SUCCESS)
		{
		/* set the date and time */
		store->time_d = (double)posen->time_d + (double)posen->microsec * 1e6;
		mb_get_date(verbose, store->time_d, &(store->time_i[0]));

		/* set the kind and type */
		store->kind = MB_DATA_NAV1;
		store->type = SWPLS_ID_PARSED_POSITION_EN;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2      error:          %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2      status:         %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_rd_posen */
/*--------------------------------------------------------------------*/
int swpls_rd_ssv(int verbose, char *buffer, void *store_ptr, int *error)
{
	char *function_name = "swpls_rd_ssv";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_ssv *ssv;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n:");
		fprintf(stderr, "dbg2     verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2     buffer:           %p\n", (void *)buffer);
		fprintf(stderr, "dbg2     store_ptr:        %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	ssv = &(store->ssv);

	/* extract the data */
	index = SWPLS_SIZE_BLOCKHEADER;
	mb_get_binary_int(MB_YES, &buffer[index], &(ssv->time_d)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(ssv->microsec)); index += 4;
	ssv->channel = buffer[index]; index++;
	mb_get_binary_float(MB_YES, &buffer[index], &(ssv->ssv)); index += 4;

	if (status == MB_SUCCESS)
		{
		/* set the date and time */
		store->time_d = (double)ssv->time_d + (double)ssv->microsec * 1e6;
		mb_get_date(verbose, store->time_d, &(store->time_i[0]));

		/* set kind and type */
		store->kind = MB_DATA_SSV;
		store->type = SWPLS_ID_PARSED_SSV;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2      error:          %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2      status:         %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_rd_ssv */
/*--------------------------------------------------------------------*/
int swpls_rd_tide(int verbose, char *buffer, void *store_ptr, int *error)
{
	char *function_name = "swpls_rd_tide";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_tide *tide;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n:");
		fprintf(stderr, "dbg2     verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2     buffer:           %p\n", (void *)buffer);
		fprintf(stderr, "dbg2     store_ptr:        %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	tide = &(store->tide);

	/* extract the data */
	index = SWPLS_SIZE_BLOCKHEADER;
	mb_get_binary_int(MB_YES, &buffer[index], &(tide->time_d)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(tide->microsec)); index += 4;
	tide->channel = buffer[index]; index++;
	mb_get_binary_float(MB_YES, &buffer[index], &(tide->tide)); index += 4;

	if (status == MB_SUCCESS)
		{
		/* set the date and time */
		store->time_d = (double)tide->time_d + (double)tide->microsec * 1e6;
		mb_get_date(verbose, store->time_d, &(store->time_i[0]));

		/* set the kind and type */
		store->kind = MB_DATA_TIDE;
		store->type = SWPLS_ID_PARSED_TIDE;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2      error:          %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2      status:         %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_rd_tide */
/*--------------------------------------------------------------------*/
int swpls_rd_echosounder(int verbose, char *buffer, void *store_ptr, int *error)
{
	char *function_name = "swpls_rd_echosounder";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_echosounder *echosounder;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n:");
		fprintf(stderr, "dbg2     verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2     buffer:           %p\n", (void *)buffer);
		fprintf(stderr, "dbg2     store_ptr:        %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	echosounder = &(store->echosounder);

	/* extract the data */
	index = SWPLS_SIZE_BLOCKHEADER;
	mb_get_binary_int(MB_YES, &buffer[index], &(echosounder->time_d));
	index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(echosounder->microsec));
	index += 4;
	echosounder->channel = buffer[index]; index++;
	mb_get_binary_float(MB_YES, &buffer[index], &(echosounder->altitude));
	index += 4;

	if (status == MB_SUCCESS)
		{
		/* set the date and time */
		store->time_d = (double)echosounder->time_d +
			(double)echosounder->microsec * 1e6;
		mb_get_date(verbose, store->time_d, &(store->time_i[0]));

		/* set the kind and type */
		store->kind = MB_DATA_ALTITUDE;
		store->type = SWPLS_ID_PARSED_ECHOSOUNDER;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2      error:          %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2      status:         %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_rd_echosounder */
/*--------------------------------------------------------------------*/
int swpls_rd_agds(int verbose, char *buffer, void *store_ptr, int *error)
{
	char *function_name = "swpls_rd_agds";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_agds *agds;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n:");
		fprintf(stderr, "dbg2     verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2     buffer:           %p\n", (void *)buffer);
		fprintf(stderr, "dbg2     store_ptr:        %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	agds = &(store->agds);

	/* extract the data */
	index = SWPLS_SIZE_BLOCKHEADER;
	mb_get_binary_int(MB_YES, &buffer[index], &(agds->time_d)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(agds->microsec)); index += 4;
	agds->channel = buffer[index]; index++;
	mb_get_binary_float(MB_YES, &buffer[index], &(agds->hardness)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(agds->roughness));
	index += 4;

	if (status == MB_SUCCESS)
		{
		/* set the date and time */
		store->time_d = (double)agds->time_d + (double)agds->microsec * 1e6;
		mb_get_date(verbose, store->time_d, &(store->time_i[0]));

		/* set the kind and type */
		store->kind = MB_DATA_NONE;
		store->type = SWPLS_ID_PARSED_AGDS;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2      error:          %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2      status:         %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_rd_agds */
/*--------------------------------------------------------------------*/
int swpls_rd_pos_offset(int verbose, char *buffer, void *store_ptr, int *error)
{
	char *function_name = "swpls_rd_pos_offset";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_pos_offset *pos_offset;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n:");
		fprintf(stderr, "dbg2     verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2     buffer:           %p\n", (void *)buffer);
		fprintf(stderr, "dbg2     store_ptr:        %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	pos_offset = &(store->pos_offset);

	/* extract the data */
	index = SWPLS_SIZE_BLOCKHEADER;
	mb_get_binary_int(MB_YES, &buffer[index], &(pos_offset->time_d));
	index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(pos_offset->microsec));
	index += 4;
	pos_offset->channel = buffer[index]; index++;
	mb_get_binary_float(MB_YES, &buffer[index], &(pos_offset->height));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(pos_offset->forward));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(pos_offset->starboard));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(pos_offset->time));
	index += 4;

	if (status == MB_SUCCESS)
		{
		/* set the date and time */
		store->time_d = (double)pos_offset->time_d +
			(double)pos_offset->microsec * 1e6;
		mb_get_date(verbose, store->time_d, &(store->time_i[0]));

		/* set the kind and type */
		store->kind = MB_DATA_PARAMETER;
		store->type = SWPLS_ID_POS_OFFSET;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2      error:          %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2      status:         %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_rd_pos_offset */
/*--------------------------------------------------------------------*/
int swpls_rd_imu_offset(int verbose, char *buffer, void *store_ptr, int *error)
{
	char *function_name = "swpls_rd_imu_offset";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_imu_offset *imu_offset;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n:");
		fprintf(stderr, "dbg2     verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2     buffer:           %p\n", (void *)buffer);
		fprintf(stderr, "dbg2     store_ptr:        %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	imu_offset = &(store->imu_offset);

	/* extract the data */
	index = SWPLS_SIZE_BLOCKHEADER;
	mb_get_binary_int(MB_YES, &buffer[index], &(imu_offset->time_d));
	index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(imu_offset->microsec));
	index += 4;
	imu_offset->channel = buffer[index]; index++;
	mb_get_binary_float(MB_YES, &buffer[index], &(imu_offset->height));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(imu_offset->forward));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(imu_offset->starboard));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(imu_offset->time));
	index += 4;

	if (status == MB_SUCCESS)
		{
		/* set the date and time */
		store->time_d = (double)imu_offset->time_d +
			(double)imu_offset->microsec * 1e6;
		mb_get_date(verbose, store->time_d, &(store->time_i[0]));

		/* set the kind and type */
		store->kind = MB_DATA_PARAMETER;
		store->type = SWPLS_ID_IMU_OFFSET;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2      error:          %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2      status:         %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_rd_imu_offset */
/*--------------------------------------------------------------------*/
int swpls_rd_txer_offset(int verbose, char *buffer, void *store_ptr, int *error)
{
	char *function_name = "swpls_rd_txer_offset";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_txer_offset *txer_offset;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n:");
		fprintf(stderr, "dbg2     verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2     buffer:           %p\n", (void *)buffer);
		fprintf(stderr, "dbg2     store_ptr:        %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	txer_offset = &(store->txer_offset);

	/* extract the data */
	index = SWPLS_SIZE_BLOCKHEADER;
	mb_get_binary_int(MB_YES, &buffer[index], &(txer_offset->time_d));
	index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(txer_offset->microsec));
	index += 4;
	txer_offset->channel = buffer[index]; index++;
	mb_get_binary_float(MB_YES, &buffer[index], &(txer_offset->height));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(txer_offset->forward));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(txer_offset->starboard));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(txer_offset->azimuth));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(txer_offset->elevation));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(txer_offset->pitch));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(txer_offset->skew));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(txer_offset->time));
	index += 4;

	if (status == MB_SUCCESS)
		{
		/* set the date and time */
		store->time_d = (double)txer_offset->time_d +
			(double)txer_offset->microsec * 1e6;
		mb_get_date(verbose, store->time_d, &(store->time_i[0]));

		/* set the kind and type */
		store->kind = MB_DATA_PARAMETER;
		store->type = SWPLS_ID_TXER_OFFSET;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2      error:          %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2      status:         %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_rd_txer_offset */
/*--------------------------------------------------------------------*/
int swpls_rd_wl_offset(int verbose, char *buffer, void *store_ptr, int *error)
{
	char *function_name = "swpls_rd_wl_offset";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_wl_offset *wl_offset;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n:");
		fprintf(stderr, "dbg2     verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2     buffer:           %p\n", (void *)buffer);
		fprintf(stderr, "dbg2     store_ptr:        %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	wl_offset = &(store->wl_offset);

	/* extract the data */
	index = SWPLS_SIZE_BLOCKHEADER;
	mb_get_binary_int(MB_YES, &buffer[index], &(wl_offset->time_d));
	index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(wl_offset->microsec));
	index += 4;
	wl_offset->channel = buffer[index]; index++;
	mb_get_binary_float(MB_YES, &buffer[index], &(wl_offset->height));
	index += 4;

	if (status == MB_SUCCESS)
		{
		/* set the date and time */
		store->time_d = (double)wl_offset->time_d +
			(double)wl_offset->microsec * 1e6;
		mb_get_date(verbose, store->time_d, &(store->time_i[0]));

		/* set the kind and type */
		store->kind = MB_DATA_PARAMETER;
		store->type = SWPLS_ID_WL_OFFSET;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2      error:          %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2      status:         %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_rd_wl_offset */
/*--------------------------------------------------------------------*/
int swpls_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char *function_name = "swpls_wr_data";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	FILE *mbfp;
	char **bufferptr;
	char *buffer;
	int *bufferalloc;
	int size;
	size_t write_len;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2 MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2      mbio_ptr:    %p\n", mbio_ptr);
		fprintf(stderr, "dbg2     store_ptr:    %p\n", mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	mbfp = mb_io_ptr->mbfp;

	/* get saved values */
	bufferptr = (char **)&mb_io_ptr->saveptr1;
	buffer = (char *)*bufferptr;
	bufferalloc = (int *)&mb_io_ptr->save6;

	/* write the current data record */

	/* write SWPLS_ID_SXP_HEADER_DATA record */
	if ((store->kind == MB_DATA_HEADER) &&
		(store->type == SWPLS_ID_SXP_HEADER_DATA))
		{
		status = swpls_wr_sxpheader(verbose, bufferalloc, bufferptr, store_ptr,
			&size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}
	/* write SWPLS_ID_PROJECTION record */
	else if ((store->kind == MB_DATA_PARAMETER) &&
		(store->type == SWPLS_ID_PROJECTION))
		{
		status = swpls_wr_projection(verbose, bufferalloc, bufferptr, store_ptr,
			&size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}
	/* write SWPLS_ID_PROCESSED_PING2 record */
	else if ((store->kind == MB_DATA_DATA) &&
		((store->type == SWPLS_ID_PROCESSED_PING) ||
		(store->type == SWPLS_ID_PROCESSED_PING2)))
		{
		status = swpls_wr_sxpping(verbose, bufferalloc, bufferptr, store_ptr,
			&size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}
	/* write SWPLS_ID_COMMENT record */
	else if (store->kind == MB_DATA_COMMENT)
		{
		status = swpls_wr_comment(verbose, bufferalloc, bufferptr, store_ptr,
			&size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}
	/* write SWPLS_ID_SXI_HEADER_DATA */
	else if ((store->kind == MB_DATA_HEADER) &&
		(store->type == SWPLS_ID_SXI_HEADER_DATA))
		{
		status = swpls_wr_sxiheader(verbose, bufferalloc, bufferptr, store_ptr,
			&size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}
	/* write SWPLS_ID_PARSED_PING */
	else if ((store->kind == MB_DATA_DATA) &&
		(store->type == SWPLS_ID_PARSED_PING))
		{
		status = swpls_wr_sxiping(verbose, bufferalloc, bufferptr, store_ptr,
			&size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}
	/* write SWPLS_ID_PARSED_ATTITUDE */
	else if (store->kind == MB_DATA_ATTITUDE)
		{
		status = swpls_wr_attitude(verbose, bufferalloc, bufferptr, store_ptr,
			&size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}
	/* write SWPLS_ID_PARSED_POSITION_LL */
	else if (store->kind == MB_DATA_NAV)
		{
		status = swpls_wr_posll(verbose, bufferalloc, bufferptr, store_ptr,
			&size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}
	/* write SWPLS_ID_PARSED_POSITION_EN */
	else if (store->kind == MB_DATA_NAV1)
		{
		status = swpls_wr_posen(verbose, bufferalloc, bufferptr, store_ptr,
			&size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}
	/* write SWPLS_ID_PARSED_SSV */
	else if (store->kind == MB_DATA_SSV)
		{
		status = swpls_wr_ssv(verbose, bufferalloc, bufferptr, store_ptr, &size,
			error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}
	/* write SWPLS_ID_PARSED_ECHOSOUNDER */
	else if (store->kind == MB_DATA_ALTITUDE)
		{
		status = swpls_wr_echosounder(verbose, bufferalloc, bufferptr,
			store_ptr, &size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}
	/* write SWPLS_ID_PARSED_TIDE */
	else if (store->kind == MB_DATA_TIDE)
		{
		status = swpls_wr_tide(verbose, bufferalloc, bufferptr, store_ptr,
			&size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}
	/* write SWPLS_ID_POS_OFFSET */
	else if ((store->kind == MB_DATA_PARAMETER) &&
		(store->type == SWPLS_ID_POS_OFFSET))
		{
		status = swpls_wr_pos_offset(verbose, bufferalloc, bufferptr, store_ptr,
			&size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}
	/* write SWPLS_ID_IMU_OFFSET */
	else if ((store->kind == MB_DATA_PARAMETER) &&
		(store->type == SWPLS_ID_IMU_OFFSET))
		{
		status = swpls_wr_imu_offset(verbose, bufferalloc, bufferptr, store_ptr,
			&size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}
	/* write SWPLS_ID_TXER_OFFSET */
	else if ((store->kind == MB_DATA_PARAMETER) &&
		(store->type == SWPLS_ID_TXER_OFFSET))
		{
		status = swpls_wr_txer_offset(verbose, bufferalloc, bufferptr,
			store_ptr, &size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}
	/* write SWPLS_ID_WL_OFFSET */
	else if ((store->kind == MB_DATA_PARAMETER) &&
		(store->type == SWPLS_ID_WL_OFFSET))
		{
		status = swpls_wr_wl_offset(verbose, bufferalloc, bufferptr, store_ptr,
			&size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:    %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_wr_data */
/*-------------------------------------------------------------------*/
int swpls_wr_sxpheader(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error)
{
	char *function_name = "swpls_wr_sxpheader";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_header *header;
	char *buffer;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:  %d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:    %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:    %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	header = &(store->sxp_header);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		swpls_pr_sxpheader(verbose, stderr, header, error);
		}

	/* figure out size of output record */
	*size = SWPLS_SIZE_BLOCKHEADER + SWPLS_SIZE_HEADER;

	/* allocate memory to write record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
			(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *)*bufferptr;

		/* insert the block header */
		index = 0;
		mb_put_binary_int(MB_YES, SWPLS_ID_SXP_HEADER_DATA, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, SWPLS_SIZE_HEADER, &buffer[index]);
		index += 4;

		/* insert the file header data */
		mb_put_binary_int(MB_YES, header->swver, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, header->fmtver, &buffer[index]); index += 4;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_wr_sxpheader */
/*---------------------------------------------------------------------*/
int swpls_wr_sxpping(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error)
{
	char *function_name = "swpls_wr_sxpping";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_sxpping *ping;
	char *buffer;
	int index;
	int i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2      verbose:     %d\n", verbose);
		fprintf(stderr, "dbg2      bufferalloc: %d\n", *bufferalloc);
		fprintf(stderr, "dbg2      bufferptr:   %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2      store_ptr:   %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	ping = &(store->sxp_ping);

	/* print debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr, "\ndbg4  Values to be written in MBIO function <%s>\n",
			function_name);
		swpls_pr_sxpping(verbose, stderr, ping, error);
		}

	/* figure out size of output record */
	*size = SWPLS_SIZE_BLOCKHEADER + SWPLS_SIZE_PROCESSED_PING2 +
		(ping->nosampsfile * SWPLS_SIZE_POINT2);

	/* allocate memory to write record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
			(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *)*bufferptr;

		/* insert the data */
		index = 0;
		mb_put_binary_int(MB_YES, SWPLS_ID_PROCESSED_PING2, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, (*size - SWPLS_SIZE_BLOCKHEADER),
			&buffer[index]); index += 4;
		strncpy(&buffer[index], &(ping->linename[0]), SWPLS_MAX_LINENAME);
		index += SWPLS_MAX_LINENAME;
		mb_put_binary_int(MB_YES, ping->pingnumber, &buffer[index]);
		index += 4;
		index += 4;		/* padding bytes */
		mb_put_binary_double(MB_YES, ping->time_d, &buffer[index]); index += 8;
		mb_put_binary_int(MB_YES, ping->notxers, &buffer[index]); index += 4;
		index += 4;		/* padding bytes */
		mb_put_binary_double(MB_YES, ping->easting, &buffer[index]);
		index += 8;
		mb_put_binary_double(MB_YES, ping->northing, &buffer[index]);
		index += 8;
		mb_put_binary_double(MB_YES, ping->roll, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, ping->pitch, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, ping->heading, &buffer[index]);
		index += 8;
		mb_put_binary_double(MB_YES, ping->height, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, ping->tide, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, ping->sos, &buffer[index]); index += 8;
		buffer[index] = ping->txno; index += 1;
		buffer[index] = ping->txstat; index += 1;
		buffer[index] = ping->txpower; index += 1;
		index += 1;		/* padding byte */
		mb_put_binary_short(MB_YES, ping->analoggain, &buffer[index]);
		index += 2;
		buffer[index] = ping->nostaves; index += 1;
		for (i = 0; i < SWPLS_MAX_TX_INFO; i++)
			{
			buffer[index] = ping->txinfo[i]; index += 1;
			}
		index += 1;		/* padding bytes */
		buffer[index] = ping->freq; index += 1;
		index += 4;		/* padding bytes */
		mb_put_binary_double(MB_YES, ping->frequency, &buffer[index]);
		index += 8;
		mb_put_binary_short(MB_YES, ping->trnstime, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ping->recvtime, &buffer[index]);
		index += 2;
		buffer[index] = ping->samprate; index += 1;
		index += 3;		/* padding bytes */
		mb_put_binary_int(MB_YES, ping->nosampsorig, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, ping->nosampsfile, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, ping->nosampslots, &buffer[index]);
		index += 4;
		index += 4;		/* padding bytes */
		mb_put_binary_double(MB_YES, ping->txer_e, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, ping->txer_n, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, ping->txer_height, &buffer[index]);
		index += 8;
		mb_put_binary_double(MB_YES, ping->txer_forward, &buffer[index]);
		index += 8;
		mb_put_binary_double(MB_YES, ping->txer_starboard, &buffer[index]);
		index += 8;
		mb_put_binary_double(MB_YES, ping->txer_azimuth, &buffer[index]);
		index += 8;
		mb_put_binary_double(MB_YES, ping->txer_elevation, &buffer[index]);
		index += 8;
		mb_put_binary_double(MB_YES, ping->txer_skew, &buffer[index]);
		index += 8;
		mb_put_binary_double(MB_YES, ping->txer_time, &buffer[index]);
		index += 8;
		mb_put_binary_double(MB_YES, ping->txer_waterdepth, &buffer[index]);
		index += 8;
		mb_put_binary_double(MB_YES, ping->txer_pitch, &buffer[index]);
		index += 8;

		/* insert the xyza point data */
		for (i = 0; i != ping->nosampsfile; ++i)
			{
			mb_put_binary_int(MB_YES, ping->points[i].sampnum, &buffer[index]);
			index += 4;
			index += 4;		/* padding bytes */
			mb_put_binary_double(MB_YES, ping->points[i].y, &buffer[index]);
			index += 8;
			mb_put_binary_double(MB_YES, ping->points[i].x, &buffer[index]);
			index += 8;
			mb_put_binary_float(MB_YES, ping->points[i].z, &buffer[index]);
			index += 4;
			mb_put_binary_short(MB_YES, ping->points[i].amp, &buffer[index]);
			index += 2;
			mb_put_binary_short(MB_YES, ping->points[i].procamp,
				&buffer[index]); index += 2;
			buffer[index] = ping->points[i].status; index += 1;
			index += 7;		/* padding bytes */
			mb_put_binary_double(MB_YES, ping->points[i].tpu, &buffer[index]);
			index += 8;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_wr_sxpping */
/*------------------------------------------------------------*/
int swpls_wr_projection(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error)
{
	char *function_name = "swpls_wr_projection";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_projection *projection;
	char *buffer;
	int index;
	size_t padding;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2      bufferalloc: %d\n", *bufferalloc);
		fprintf(stderr, "dbg2      bufferptr:   %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2      store_ptr:   %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	projection = &(store->projection);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		swpls_pr_projection(verbose, stderr, projection, error);
		}

	/* figure out the size of output record */
	*size = SWPLS_SIZE_BLOCKHEADER + SWPLS_SIZE_PROJECTION + projection->nchars;

	/* pad string to byte align on int32 boundary (not necessary, just
	   easier to read in hexfiend */
	padding = 4 - (*size % 4);
	*size = *size + padding;
	projection->nchars = projection->nchars + padding;

	/* allocate memory to write record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
			(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *)*bufferptr;

		/* insert the data */
		index = 0;
		mb_put_binary_int(MB_YES, SWPLS_ID_PROJECTION, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, *size - SWPLS_SIZE_BLOCKHEADER,
			&buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, projection->time_d, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, projection->microsec, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, projection->nchars, &buffer[index]); index += 4;
		strncpy(&buffer[index], projection->projection_id,
			projection->nchars); index += projection->nchars;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_wr_projection */
/*------------------------------------------------------------*/
int swpls_wr_comment(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error)
{
	char *function_name = "swpls_wr_comment";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_comment *comment;
	char *buffer;
	int index;
	size_t padding;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2      bufferalloc: %d\n", *bufferalloc);
		fprintf(stderr, "dbg2      bufferptr:   %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2      store_ptr:   %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	comment = &(store->comment);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		swpls_pr_comment(verbose, stderr, comment, error);
		}

	/* figure out the size of output record */
	*size = SWPLS_SIZE_BLOCKHEADER + SWPLS_SIZE_COMMENT + comment->nchars;

	/* pad string to byte align on int32 boundary (not necessary, just
	   easier to read in hexfiend */
	padding = 4 - (*size % 4);
	*size = *size + padding;
	comment->nchars = comment->nchars + padding;

	/* allocate memory to write record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
			(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *)*bufferptr;

		/* insert the data */
		index = 0;
		mb_put_binary_int(MB_YES, SWPLS_ID_COMMENT, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, *size - SWPLS_SIZE_BLOCKHEADER,
			&buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, comment->time_d, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, comment->microsec, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, comment->nchars, &buffer[index]); index += 4;
		strncpy(&buffer[index], comment->message,
			comment->nchars); index += comment->nchars;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_wr_comment */
/*-------------------------------------------------------------------*/
int swpls_wr_sxiheader(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error)
{
	char *function_name = "swpls_wr_sxiheader";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_header *header;
	char *buffer;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:  %d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:    %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:    %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	header = &(store->sxi_header);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		swpls_pr_sxiheader(verbose, stderr, header, error);
		}

	/* figure out size of output record */
	*size = SWPLS_SIZE_BLOCKHEADER + SWPLS_SIZE_HEADER;

	/* allocate memory to write record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
			(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *)*bufferptr;

		/* insert the block header */
		index = 0;
		mb_put_binary_int(MB_YES, SWPLS_ID_SXI_HEADER_DATA, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, SWPLS_SIZE_HEADER, &buffer[index]);
		index += 4;

		/* insert the file header data */
		mb_put_binary_int(MB_YES, header->swver, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, header->fmtver, &buffer[index]); index += 4;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_wr_sxiheader */
/*-------------------------------------------------------------------*/
int swpls_wr_sxiping(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error)
{
	char *function_name = "swpls_wr_sxiping";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_sxiping *ping;
	char *buffer;
	int index;
	int i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:  %d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:    %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:    %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	ping = &(store->sxi_ping);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		swpls_pr_sxiping(verbose, stderr, ping, error);
		}

	/* figure out size of output record */
	*size = SWPLS_SIZE_BLOCKHEADER + SWPLS_SIZE_PARSED_PING +
		(ping->nosamps * SWPLS_SIZE_PARSED_POINT);

	/* allocate memory to write record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
			(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *)*bufferptr;

		/* insert the block header */
		index = 0;
		mb_put_binary_int(MB_YES, SWPLS_ID_PARSED_PING, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, (*size - SWPLS_SIZE_BLOCKHEADER),
			&buffer[index]); index += 4;

		/* insert the data */
		mb_put_binary_int(MB_YES, ping->time_d, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, ping->microsec, &buffer[index]); index += 4;
		buffer[index] = ping->channel; index++;
		mb_put_binary_int(MB_YES, ping->pingnumber, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, ping->frequency, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, ping->samp_period, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, ping->nosamps, &buffer[index]); index += 2;
		mb_put_binary_float(MB_YES, ping->sos, &buffer[index]); index += 4;
		mb_put_binary_short(MB_YES, ping->txpulse, &buffer[index]); index += 2;
		buffer[index] = ping->data_options; index++;
		buffer[index] = ping->ping_state; index++;
		mb_put_binary_short(MB_YES, ping->max_count, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ping->reserve1, &buffer[index]);
		index += 2;
		for (i = 0; i < ping->nosamps; i++)
			{
			mb_put_binary_short(MB_YES, ping->sampnum[i], &buffer[index]);
			index += 2;
			mb_put_binary_short(MB_YES, ping->angle[i], &buffer[index]);
			index += 2;
			mb_put_binary_short(MB_YES, ping->amplitude[i], &buffer[index]);
			index += 2;
			buffer[index] = ping->quality[i]; index++;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_wr_sxiping */
/*-------------------------------------------------------------------*/
int swpls_wr_attitude(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error)
{
	char *function_name = "swpls_wr_attitude";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_attitude *attitude;
	char *buffer;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:  %d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:    %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:    %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	attitude = &(store->attitude);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		swpls_pr_attitude(verbose, stderr, attitude, error);
		}

	/* figure out size of output record */
	*size = SWPLS_SIZE_BLOCKHEADER + SWPLS_SIZE_ATTITUDE;

	/* allocate memory to write record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
			(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *)*bufferptr;

		/* insert the block header */
		index = 0;
		mb_put_binary_int(MB_YES, SWPLS_ID_PARSED_ATTITUDE, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, (*size - SWPLS_SIZE_BLOCKHEADER),
			&buffer[index]); index += 4;

		/* insert the data */
		mb_put_binary_int(MB_YES, attitude->time_d, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, attitude->microsec, &buffer[index]);
		index += 4;
		buffer[index] = attitude->channel; index++;
		mb_put_binary_float(MB_YES, attitude->roll, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, attitude->pitch, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, attitude->heading, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, attitude->height, &buffer[index]);
		index += 2;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_wr_attitude */
/*-------------------------------------------------------------------*/
int swpls_wr_posll(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error)
{
	char *function_name = "swpls_wr_posll";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_posll *posll;
	char *buffer;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:  %d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:    %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:    %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	posll = &(store->posll);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		swpls_pr_posll(verbose, stderr, posll, error);
		}

	/* figure out size of output record */
	*size = SWPLS_SIZE_BLOCKHEADER + SWPLS_SIZE_POSITION_LL;

	/* allocate memory to write record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
			(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *)*bufferptr;

		/* insert the block header */
		index = 0;
		mb_put_binary_int(MB_YES, SWPLS_ID_PARSED_POSITION_LL, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, (*size - SWPLS_SIZE_BLOCKHEADER),
			&buffer[index]); index += 4;

		/* insert the data */
		mb_put_binary_int(MB_YES, posll->time_d, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, posll->microsec, &buffer[index]); index += 4;
		buffer[index] = posll->channel; index++;
		mb_put_binary_double(MB_YES, posll->latitude, &buffer[index]);
		index += 8;
		mb_put_binary_double(MB_YES, posll->longitude, &buffer[index]);
		index += 8;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_wr_posll */
/*-------------------------------------------------------------------*/
int swpls_wr_posen(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error)
{
	char *function_name = "swpls_wr_posen";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_posen *posen;
	char *buffer;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:  %d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:    %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:    %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	posen = &(store->posen);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		swpls_pr_posen(verbose, stderr, posen, error);
		}

	/* figure out size of output record */
	*size = SWPLS_SIZE_BLOCKHEADER + SWPLS_SIZE_POSITION_EN;

	/* allocate memory to write record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
			(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *)*bufferptr;

		/* insert the block header */
		index = 0;
		mb_put_binary_int(MB_YES, SWPLS_ID_PARSED_POSITION_EN, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, (*size - SWPLS_SIZE_BLOCKHEADER),
			&buffer[index]); index += 4;

		/* insert the data */
		mb_put_binary_int(MB_YES, posen->time_d, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, posen->microsec, &buffer[index]); index += 4;
		buffer[index] = posen->channel; index++;
		mb_put_binary_double(MB_YES, posen->easting, &buffer[index]);
		index += 8;
		mb_put_binary_double(MB_YES, posen->northing, &buffer[index]);
		index += 8;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_wr_posen */
/*-------------------------------------------------------------------*/
int swpls_wr_ssv(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error)
{
	char *function_name = "swpls_wr_ssv";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_ssv *ssv;
	char *buffer;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:  %d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:    %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:    %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	ssv = &(store->ssv);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		swpls_pr_ssv(verbose, stderr, ssv, error);
		}

	/* figure out size of output record */
	*size = SWPLS_SIZE_BLOCKHEADER + SWPLS_SIZE_SSV;

	/* allocate memory to write record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
			(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *)*bufferptr;

		/* insert the block header */
		index = 0;
		mb_put_binary_int(MB_YES, SWPLS_ID_PARSED_SSV, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, (*size - SWPLS_SIZE_BLOCKHEADER),
			&buffer[index]); index += 4;

		/* insert the data */
		mb_put_binary_int(MB_YES, ssv->time_d, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, ssv->microsec, &buffer[index]); index += 4;
		buffer[index] = ssv->channel; index++;
		mb_put_binary_float(MB_YES, ssv->ssv, &buffer[index]); index += 4;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_wr_ssv */
/*-------------------------------------------------------------------*/
int swpls_wr_tide(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error)
{
	char *function_name = "swpls_wr_tide";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_tide *tide;
	char *buffer;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:  %d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:    %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:    %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	tide = &(store->tide);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		swpls_pr_tide(verbose, stderr, tide, error);
		}

	/* figure out size of output record */
	*size = SWPLS_SIZE_BLOCKHEADER + SWPLS_SIZE_TIDE;

	/* allocate memory to write record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
			(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *)*bufferptr;

		/* insert the block header */
		index = 0;
		mb_put_binary_int(MB_YES, SWPLS_ID_PARSED_TIDE, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, (*size - SWPLS_SIZE_BLOCKHEADER),
			&buffer[index]); index += 4;

		/* insert the data */
		mb_put_binary_int(MB_YES, tide->time_d, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, tide->microsec, &buffer[index]); index += 4;
		buffer[index] = tide->channel; index++;
		mb_put_binary_float(MB_YES, tide->tide, &buffer[index]); index += 4;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_wr_tide */
/*-------------------------------------------------------------------*/
int swpls_wr_echosounder(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error)
{
	char *function_name = "swpls_wr_echosounder";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_echosounder *echosounder;
	char *buffer;
	int index;

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	echosounder = &(store->echosounder);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		swpls_pr_echosounder(verbose, stderr, echosounder, error);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr, "dbg5        echosounder->time_d:      %d\n",
			echosounder->time_d);
		fprintf(stderr, "dbg5        echosounder->microsec:    %d\n",
			echosounder->microsec);
		fprintf(stderr, "dbg5        echosounder->channel:     %u\n",
			echosounder->channel);
		fprintf(stderr, "dbg5        echosounder->altitude:         %f\n",
			echosounder->altitude);
		}

	/* figure out size of output record */
	*size = SWPLS_SIZE_BLOCKHEADER + SWPLS_SIZE_ECHOSOUNDER;

	/* allocate memory to write record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
			(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *)*bufferptr;

		/* insert the block header */
		index = 0;
		mb_put_binary_int(MB_YES, SWPLS_ID_PARSED_ECHOSOUNDER, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, (*size - SWPLS_SIZE_BLOCKHEADER),
			&buffer[index]); index += 4;

		/* insert the data */
		mb_put_binary_int(MB_YES, echosounder->time_d, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, echosounder->microsec, &buffer[index]);
		index += 4;
		buffer[index] = echosounder->channel; index++;
		mb_put_binary_float(MB_YES, echosounder->altitude, &buffer[index]);
		index += 4;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_wr_echosounder */
/*-------------------------------------------------------------------*/
int swpls_wr_agds(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error)
{
	char *function_name = "swpls_wr_agds";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_agds *agds;
	char *buffer;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:  %d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:    %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:    %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	agds = &(store->agds);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		swpls_pr_agds(verbose, stderr, agds, error);
		}

	/* figure out size of output record */
	*size = SWPLS_SIZE_BLOCKHEADER + SWPLS_SIZE_AGDS;

	/* allocate memory to write record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
			(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *)*bufferptr;

		/* insert the block header */
		index = 0;
		mb_put_binary_int(MB_YES, SWPLS_ID_PARSED_AGDS, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, (*size - SWPLS_SIZE_BLOCKHEADER),
			&buffer[index]); index += 4;

		/* insert the data */
		mb_put_binary_int(MB_YES, agds->time_d, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, agds->microsec, &buffer[index]); index += 4;
		buffer[index] = agds->channel; index++;
		mb_put_binary_float(MB_YES, agds->hardness, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, agds->roughness, &buffer[index]);
		index += 4;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_wr_agds */
/*-------------------------------------------------------------------*/
int swpls_wr_pos_offset(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error)
{
	char *function_name = "swpls_wr_pos_offset";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_pos_offset *pos_offset;
	char *buffer;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:  %d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:    %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:    %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	pos_offset = &(store->pos_offset);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		swpls_pr_pos_offset(verbose, stderr, pos_offset, error);
		}

	/* figure out size of output record */
	*size = SWPLS_SIZE_BLOCKHEADER + SWPLS_SIZE_POS_OFFSET;

	/* allocate memory to write record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
			(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *)*bufferptr;

		/* insert the block header */
		index = 0;
		mb_put_binary_int(MB_YES, SWPLS_ID_POS_OFFSET, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, (*size - SWPLS_SIZE_BLOCKHEADER),
			&buffer[index]); index += 4;

		/* insert the data */
		mb_put_binary_int(MB_YES, pos_offset->time_d, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, pos_offset->microsec, &buffer[index]);
		index += 4;
		buffer[index] = pos_offset->channel; index++;
		mb_put_binary_float(MB_YES, pos_offset->height, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, pos_offset->forward, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, pos_offset->starboard, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, pos_offset->time, &buffer[index]);
		index += 4;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_wr_pos_offset */
/*-------------------------------------------------------------------*/
int swpls_wr_imu_offset(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error)
{
	char *function_name = "swpls_wr_imu_offset";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_imu_offset *imu_offset;
	char *buffer;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:  %d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:    %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:    %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	imu_offset = &(store->imu_offset);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		swpls_pr_imu_offset(verbose, stderr, imu_offset, error);
		}

	/* figure out size of output record */
	*size = SWPLS_SIZE_BLOCKHEADER + SWPLS_SIZE_IMU_OFFSET;

	/* allocate memory to write record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
			(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *)*bufferptr;

		/* insert the block header */
		index = 0;
		mb_put_binary_int(MB_YES, SWPLS_ID_IMU_OFFSET, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, (*size - SWPLS_SIZE_BLOCKHEADER),
			&buffer[index]); index += 4;

		/* insert the data */
		mb_put_binary_int(MB_YES, imu_offset->time_d, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, imu_offset->microsec, &buffer[index]);
		index += 4;
		buffer[index] = imu_offset->channel; index++;
		mb_put_binary_float(MB_YES, imu_offset->height, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, imu_offset->forward, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, imu_offset->starboard, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, imu_offset->time, &buffer[index]);
		index += 4;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_wr_imu_offset */
/*-------------------------------------------------------------------*/
int swpls_wr_txer_offset(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error)
{
	char *function_name = "swpls_wr_txer_offset";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_txer_offset *txer_offset;
	char *buffer;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:  %d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:    %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:    %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	txer_offset = &(store->txer_offset);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		swpls_pr_txer_offset(verbose, stderr, txer_offset, error);
		}

	/* figure out size of output record */
	*size = SWPLS_SIZE_BLOCKHEADER + SWPLS_SIZE_TXER_OFFSET;

	/* allocate memory to write record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
			(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *)*bufferptr;

		/* insert the block header */
		index = 0;
		mb_put_binary_int(MB_YES, SWPLS_ID_TXER_OFFSET, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, (*size - SWPLS_SIZE_BLOCKHEADER),
			&buffer[index]); index += 4;

		/* insert the data */
		mb_put_binary_int(MB_YES, txer_offset->time_d, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, txer_offset->microsec, &buffer[index]);
		index += 4;
		buffer[index] = txer_offset->channel; index++;
		mb_put_binary_float(MB_YES, txer_offset->height, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, txer_offset->forward, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, txer_offset->starboard, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, txer_offset->azimuth, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, txer_offset->elevation, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, txer_offset->pitch, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, txer_offset->skew, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, txer_offset->time, &buffer[index]);
		index += 4;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_wr_txer_offset */
/*-------------------------------------------------------------------*/
int swpls_wr_wl_offset(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error)
{
	char *function_name = "swpls_wr_wl_offset";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;
	swpls_wl_offset *wl_offset;
	char *buffer;
	int index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:  %d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:    %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:    %p\n", (void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *)store_ptr;
	wl_offset = &(store->wl_offset);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		swpls_pr_wl_offset(verbose, stderr, wl_offset, error);
		}

	/* figure out size of output record */
	*size = SWPLS_SIZE_BLOCKHEADER + SWPLS_SIZE_WL_OFFSET;

	/* allocate memory to write record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
			(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *)*bufferptr;

		/* insert the block header */
		index = 0;
		mb_put_binary_int(MB_YES, SWPLS_ID_WL_OFFSET, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, (*size - SWPLS_SIZE_BLOCKHEADER),
			&buffer[index]); index += 4;

		/* insert the data */
		mb_put_binary_int(MB_YES, wl_offset->time_d, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, wl_offset->microsec, &buffer[index]);
		index += 4;
		buffer[index] = wl_offset->channel; index++;
		mb_put_binary_float(MB_YES, wl_offset->height, &buffer[index]);
		index += 4;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_wr_wl_offset */
/*----------------------------------------------------------------------*/
int swpls_pr_sxpheader(int verbose, FILE *fout, swpls_header *header,
	int *error)
{
	char *function_name = "swpls_pr_sxpheader";
	int status = MB_SUCCESS;
	char starter[5];

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       header:       %p\n", (void *)header);
		}

	if (verbose > 0)
		{
		sprintf(starter, "dbg%d", MIN(verbose, 9));
		}
	else
		{
		strcpy(starter, "    ");
		}

	fprintf(fout, "\n%s  SWPLS_ID_SXP_HEADER_DATA [ID: 0x%X] %d bytes\n",
		starter, SWPLS_ID_SXP_HEADER_DATA, SWPLS_SIZE_HEADER);
	fprintf(fout, "%s        swver:  %d\n", starter, header->swver);
	fprintf(fout, "%s        fmtver: %d\n", starter, header->fmtver);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_pr_sxpheader */
/*----------------------------------------------------------------------*/
int swpls_pr_sxpping(int verbose, FILE *fout, swpls_sxpping *ping, int *error)
{
	char *function_name = "swpls_pr_sxpping";
	int status = MB_SUCCESS;
	swpls_point *points;
	char starter[5];
	int size;
	time_t tm;
	int i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       ping:         %p\n", (void *)ping);
		}

	if (verbose > 0)
		{
		sprintf(starter, "dbg%d", MIN(verbose, 9));
		}
	else
		{
		strcpy(starter, "    ");
		}

	size = SWPLS_SIZE_PROCESSED_PING2 + (ping->nosampsfile * SWPLS_SIZE_POINT2);
	tm = (time_t)trunc(ping->time_d);

	fprintf(fout, "\n%s  SWPLS_ID_PROCESSED_PING2 [ID: 0x%X] %d bytes\n",
		starter, SWPLS_ID_PROCESSED_PING2, size);
	fprintf(fout, "%s        linename:             %s\n", starter,
		ping->linename);
	fprintf(fout, "%s        pingnumber:           %d\n", starter,
		ping->pingnumber);
	fprintf(fout, "%s        time_d:               %lf :: %s", starter,
		ping->time_d, asctime(gmtime(&tm)));
	fprintf(fout, "%s        notxers:              %d\n", starter,
		ping->notxers);
	fprintf(fout, "%s        easting:              %lf\n", starter,
		ping->easting);
	fprintf(fout, "%s        northing:             %lf\n", starter,
		ping->northing);
	fprintf(fout, "%s        roll:                 %lf\n", starter, ping->roll);
	fprintf(fout, "%s        pitch:                %lf\n", starter,
		ping->pitch);
	fprintf(fout, "%s        heading:              %lf\n", starter,
		ping->heading);
	fprintf(fout, "%s        height:               %lf\n", starter,
		ping->height);
	fprintf(fout, "%s        tide:                 %lf\n", starter, ping->tide);
	fprintf(fout, "%s        sos:                  %lf\n", starter, ping->sos);
	fprintf(fout, "%s        txno:                 %u\n", starter, ping->txno);
	fprintf(fout, "%s        txstat:               %u\n", starter,
		ping->txstat);
	fprintf(fout, "%s        txpower:              %u\n", starter,
		ping->txpower);
	fprintf(fout, "%s        analoggain:           %u\n", starter,
		ping->analoggain);
	fprintf(fout, "%s        nostaves:             %u\n", starter,
		ping->nostaves);
	for (i = 0; i < SWPLS_MAX_TX_INFO; i++)
		{
		fprintf(fout, "%s        txinfo[%d]:            %u\n", starter, i,
			ping->txinfo[i]);
		}
	fprintf(fout, "%s        freq:                 %u\n", starter, ping->freq);
	fprintf(fout, "%s        frequency:            %lf\n", starter,
		ping->frequency);
	fprintf(fout, "%s        trnstime:             %d\n", starter,
		ping->trnstime);
	fprintf(fout, "%s        recvtime:             %d\n", starter,
		ping->recvtime);
	fprintf(fout, "%s        samprate:             %u\n", starter,
		ping->samprate);
	fprintf(fout, "%s        nosampsorig:          %d\n", starter,
		ping->nosampsorig);
	fprintf(fout, "%s        nosampsfile:          %d\n", starter,
		ping->nosampsfile);
	fprintf(fout, "%s        nosampslots:          %d\n", starter,
		ping->nosampslots);
	fprintf(fout, "%s        txer_e:               %lf\n", starter,
		ping->txer_e);
	fprintf(fout, "%s        txer_n:               %lf\n", starter,
		ping->txer_n);
	fprintf(fout, "%s        txer_height:          %lf\n", starter,
		ping->txer_height);
	fprintf(fout, "%s        txer_forward:         %lf\n", starter,
		ping->txer_forward);
	fprintf(fout, "%s        txer_starboard:       %lf\n", starter,
		ping->txer_starboard);
	fprintf(fout, "%s        txer_azimuth:         %lf\n", starter,
		ping->txer_azimuth);
	fprintf(fout, "%s        txer_elevation:       %lf\n", starter,
		ping->txer_elevation);
	fprintf(fout, "%s        txer_skew:            %lf\n", starter,
		ping->txer_skew);
	fprintf(fout, "%s        txer_time:            %lf\n", starter,
		ping->txer_time);
	fprintf(fout, "%s        txer_waterdepth:      %lf\n", starter,
		ping->txer_waterdepth);
	fprintf(fout, "%s        txer_pitch:           %lf\n", starter,
		ping->txer_pitch);

	points = ping->points;
	for (i = 0; i < ping->nosampsfile; i++)
		{
		fprintf(fout, "%s        %4d %11.2lf %10.2lf %7.2f %5u %5u %u %5.2lf\n",
			starter, points[i].sampnum, points[i].y, points[i].x,
			points[i].z, points[i].amp, points[i].procamp, points[i].status,
			points[i].tpu);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_pr_sxpping */
/*----------------------------------------------------------------------*/
int swpls_pr_projection(int verbose, FILE *fout, swpls_projection *projection,
	int *error)
{
	char *function_name = "swpls_pr_projection";
	int status = MB_SUCCESS;
	char starter[5];
	time_t tm;
	int size;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       projection:      %p\n", (void *)projection);
		}

	if (verbose > 0)
		{
		sprintf(starter, "dbg%d", MIN(verbose, 9));
		}
	else
		{
		strcpy(starter, "    ");
		}

	size = SWPLS_SIZE_PROJECTION + projection->nchars;
	tm = (time_t)trunc(projection->time_d);
	fprintf(fout, "\n%s  SWPLS_ID_PROJECTION [ID: 0x%X] %d bytes\n", starter,
		SWPLS_ID_PROJECTION, size);
	fprintf(fout, "%s        time_d:               %d :: %s", starter,
		projection->time_d, asctime(gmtime(&tm)));
	fprintf(fout, "%s        microsec:             %d\n", starter,
		projection->microsec);
	fprintf(fout, "%s        nchars:               %d\n", starter,
		projection->nchars);
	fprintf(fout, "%s        projection_id:        %s\n", starter,
		projection->projection_id);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_pr_projection */
/*----------------------------------------------------------------------*/
int swpls_pr_comment(int verbose, FILE *fout, swpls_comment *comment,
	int *error)
{
	char *function_name = "swpls_pr_comment";
	int status = MB_SUCCESS;
	char starter[5];
	time_t tm;
	int size;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       comment:      %p\n", (void *)comment);
		}

	if (verbose > 0)
		{
		sprintf(starter, "dbg%d", MIN(verbose, 9));
		}
	else
		{
		strcpy(starter, "    ");
		}

	size = SWPLS_SIZE_COMMENT + comment->nchars;
	tm = (time_t)trunc(comment->time_d);

	fprintf(fout, "\n%s  SWPLS_ID_COMMENT [ID: 0x%X] %d bytes\n", starter,
		SWPLS_ID_COMMENT, size);
	fprintf(fout, "%s        time_d:               %d :: %s", starter,
		comment->time_d, asctime(gmtime(&tm)));
	fprintf(fout, "%s        microsec:             %d\n", starter,
		comment->microsec);
	fprintf(fout, "%s        nchars:               %d\n", starter,
		comment->nchars);
	fprintf(fout, "%s        message:              %s\n", starter,
		comment->message);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_pr_comment */
/*----------------------------------------------------------------------*/
int swpls_pr_sxiheader(int verbose, FILE *fout, swpls_header *header,
	int *error)
{
	char *function_name = "swpls_pr_sxiheader";
	int status = MB_SUCCESS;
	char starter[5];

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       header:       %p\n", (void *)header);
		}

	if (verbose > 0)
		{
		sprintf(starter, "dbg%d", MIN(verbose, 9));
		}
	else
		{
		strcpy(starter, "    ");
		}

	fprintf(fout, "\n%s  SWPLS_ID_SXI_HEADER_DATA [ID: 0x%X] %d bytes\n",
		starter, SWPLS_ID_SXI_HEADER_DATA, SWPLS_SIZE_HEADER);
	fprintf(fout, "%s        swver:        %d\n", starter, header->swver);
	fprintf(fout, "%s        fmtver:       %d\n", starter, header->fmtver);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_pr_sxiheader */
/*----------------------------------------------------------------------*/
int swpls_pr_sxiping(int verbose, FILE *fout, swpls_sxiping *ping, int *error)
{
	char *function_name = "swpls_pr_sxiping";
	int status = MB_SUCCESS;
	char starter[5];
	int size;
	time_t tm;
	int i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       ping:       %p\n", (void *)ping);
		}

	if (verbose > 0)
		{
		sprintf(starter, "dbg%d", MIN(verbose, 9));
		}
	else
		{
		strcpy(starter, "    ");
		}

	/* figure out size of output record */
	size = SWPLS_SIZE_PARSED_PING + (ping->nosamps * SWPLS_SIZE_PARSED_POINT);
	tm = (time_t)trunc(ping->time_d);

	fprintf(fout, "\n%s  SWPLS_ID_SXI_PARSED_PING [ID: 0x%X] %d bytes\n",
		starter, SWPLS_ID_PARSED_PING, size);
	fprintf(fout, "%s        time_d:       %d :: %s", starter, ping->time_d, asctime(gmtime(&tm)));
	fprintf(fout, "%s        microsec:     %d\n", starter, ping->microsec);
	fprintf(fout, "%s        channel:      %u\n", starter, ping->channel);
	fprintf(fout, "%s        pingnumber:   %lu\n", starter, ping->pingnumber);
	fprintf(fout, "%s        frequency:    %f\n", starter, ping->frequency);
	fprintf(fout, "%s        samp_period:  %f\n", starter, ping->samp_period);
	fprintf(fout, "%s        nosamps:      %u\n", starter, ping->nosamps);
	fprintf(fout, "%s        sos:          %f\n", starter, ping->sos);
	fprintf(fout, "%s        txpulse:      %d\n", starter, ping->txpulse);
	fprintf(fout, "%s        data_options: %d\n", starter, ping->data_options);
	fprintf(fout, "%s        ping_state:   %u\n", starter, ping->ping_state);
	fprintf(fout, "%s        max_count:    %u\n", starter, ping->max_count);
	fprintf(fout, "%s        reserve1:     %u\n", starter, ping->reserve1);
	for (i = 0; i < ping->nosamps; i++)
		{
		fprintf(fout,
			"%s        sampnum: %4u angle: %6d amplitude: %5u quality: %u\n",
			starter,
			ping->sampnum[i], ping->angle[i], ping->amplitude[i],
			ping->quality[i]);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_pr_sxiping */
/*----------------------------------------------------------------------*/
int swpls_pr_attitude(int verbose, FILE *fout, swpls_attitude *attitude,
	int *error)
{
	char *function_name = "swpls_pr_attitude";
	int status = MB_SUCCESS;
	char starter[5];
	time_t tm;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       attitude:     %p\n", (void *)attitude);
		}

	if (verbose > 0)
		{
		sprintf(starter, "dbg%d", MIN(verbose, 9));
		}
	else
		{
		strcpy(starter, "    ");
		}
	tm = (time_t)trunc(attitude->time_d);

	fprintf(fout, "\n%s  SWPLS_ID_PARSED_ATTITUDE [ID: 0x%X] %d bytes\n",
		starter, SWPLS_ID_PARSED_ATTITUDE, SWPLS_SIZE_ATTITUDE);
	fprintf(fout, "%s        time_d:       %d :: %s", starter, attitude->time_d, asctime(gmtime(&tm)));
	fprintf(fout, "%s        microsec:     %d\n", starter, attitude->microsec);
	fprintf(fout, "%s        channel:      %u\n", starter, attitude->channel);
	fprintf(fout, "%s        roll:         %f\n", starter, attitude->roll);
	fprintf(fout, "%s        pitch:        %f\n", starter, attitude->pitch);
	fprintf(fout, "%s        heading:      %f\n", starter, attitude->heading);
	fprintf(fout, "%s        height:       %f\n", starter, attitude->height);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_pr_attitude */
/*----------------------------------------------------------------------*/
int swpls_pr_posll(int verbose, FILE *fout, swpls_posll *posll, int *error)
{
	char *function_name = "swpls_pr_posll";
	int status = MB_SUCCESS;
	char starter[5];
	time_t tm;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       posll:        %p\n", (void *)posll);
		}

	if (verbose > 0)
		{
		sprintf(starter, "dbg%d", MIN(verbose, 9));
		}
	else
		{
		strcpy(starter, "    ");
		}
	tm = (time_t)trunc(posll->time_d);

	fprintf(fout, "\n%s  SWPLS_ID_PARSED_POSITION_LL [ID: 0x%X] %d bytes\n",
		starter, SWPLS_ID_PARSED_POSITION_LL, SWPLS_SIZE_POSITION_LL);
	fprintf(fout, "%s        time_d:       %d :: %s", starter, posll->time_d, asctime(gmtime(&tm)));
	fprintf(fout, "%s        microsec:     %d\n", starter, posll->microsec);
	fprintf(fout, "%s        channel:      %u\n", starter, posll->channel);
	fprintf(fout, "%s        latitude:     %lf\n", starter, posll->latitude);
	fprintf(fout, "%s        longitude:    %lf\n", starter, posll->longitude);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_pr_posll */
/*----------------------------------------------------------------------*/
int swpls_pr_posen(int verbose, FILE *fout, swpls_posen *posen, int *error)
{
	char *function_name = "swpls_pr_posen";
	int status = MB_SUCCESS;
	char starter[5];
	time_t tm;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       posen:        %p\n", (void *)posen);
		}

	if (verbose > 0)
		{
		sprintf(starter, "dbg%d", MIN(verbose, 9));
		}
	else
		{
		strcpy(starter, "    ");
		}

	tm = (time_t)trunc(posen->time_d);

	fprintf(fout, "\n%s  SWPLS_ID_PARSED_POSITION_EN [ID: 0x%X] %d bytes\n",
		starter, SWPLS_ID_PARSED_POSITION_EN, SWPLS_SIZE_POSITION_EN);
	fprintf(fout, "%s        time_d:       %d :: %s", starter, posen->time_d, asctime(gmtime(&tm)));
	fprintf(fout, "%s        microsec:     %d\n", starter, posen->microsec);
	fprintf(fout, "%s        channel:      %u\n", starter, posen->channel);
	fprintf(fout, "%s        easting:      %lf\n", starter, posen->easting);
	fprintf(fout, "%s        northing:     %lf\n", starter, posen->northing);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_pr_posen */
/*----------------------------------------------------------------------*/
int swpls_pr_ssv(int verbose, FILE *fout, swpls_ssv *ssv, int *error)
{
	char *function_name = "swpls_pr_ssv";
	int status = MB_SUCCESS;
	char starter[5];
	time_t tm;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       ssv:          %p\n", (void *)ssv);
		}

	if (verbose > 0)
		{
		sprintf(starter, "dbg%d", MIN(verbose, 9));
		}
	else
		{
		strcpy(starter, "    ");
		}

	tm = (time_t)trunc(ssv->time_d);

	fprintf(fout, "\n%s  SWPLS_ID_PARSED_SSV [ID: 0x%X] %d bytes\n", starter,
		SWPLS_ID_PARSED_SSV, SWPLS_SIZE_SSV);
	fprintf(fout, "%s        time_d:       %d :: %s", starter, ssv->time_d, asctime(gmtime(&tm)));
	fprintf(fout, "%s        microsec:     %d\n", starter, ssv->microsec);
	fprintf(fout, "%s        channel:      %u\n", starter, ssv->channel);
	fprintf(fout, "%s        ssv:          %f\n", starter, ssv->ssv);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_pr_ssv */
/*----------------------------------------------------------------------*/
int swpls_pr_tide(int verbose, FILE *fout, swpls_tide *tide, int *error)
{
	char *function_name = "swpls_pr_tide";
	int status = MB_SUCCESS;
	char starter[5];
	time_t tm;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       tide:         %p\n", (void *)tide);
		}

	if (verbose > 0)
		{
		sprintf(starter, "dbg%d", MIN(verbose, 9));
		}
	else
		{
		strcpy(starter, "    ");
		}

	tm = (time_t)trunc(tide->time_d);

	fprintf(fout, "\n%s  SWPLS_ID_SXI_PARSED_TIDE [ID: 0x%X] %d bytes\n",
		starter, SWPLS_ID_PARSED_TIDE, SWPLS_SIZE_TIDE);
	fprintf(fout, "%s        time_d:       %d :: %s", starter, tide->time_d, asctime(gmtime(&tm)));
	fprintf(fout, "%s        microsec:     %d\n", starter, tide->microsec);
	fprintf(fout, "%s        channel:      %u\n", starter, tide->channel);
	fprintf(fout, "%s        tide:         %f\n", starter, tide->tide);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_pr_tide */
/*----------------------------------------------------------------------*/
int swpls_pr_echosounder(int verbose, FILE *fout,
	swpls_echosounder *echosounder, int *error)
{
	char *function_name = "swpls_pr_echosounder";
	int status = MB_SUCCESS;
	char starter[5];
	time_t tm;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       echosounder:  %p\n", (void *)echosounder);
		}

	if (verbose > 0)
		{
		sprintf(starter, "dbg%d", MIN(verbose, 9));
		}
	else
		{
		strcpy(starter, "    ");
		}

	tm = (time_t)trunc(echosounder->time_d);

	fprintf(fout, "\n%s  SWPLS_ID_PARSED_ECHOSOUNDER [ID: 0x%X] %d bytes\n",
		starter, SWPLS_ID_PARSED_ECHOSOUNDER, SWPLS_SIZE_ECHOSOUNDER);
	fprintf(fout, "%s        time_d:       %d :: %s", starter, echosounder->time_d, asctime(gmtime(&tm)));
	fprintf(fout, "%s        microsec:     %d\n", starter,
		echosounder->microsec);
	fprintf(fout, "%s        channel:      %u\n", starter,
		echosounder->channel);
	fprintf(fout, "%s        altitude:     %f\n", starter,
		echosounder->altitude);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_pr_echosounder */
/*----------------------------------------------------------------------*/
int swpls_pr_agds(int verbose, FILE *fout, swpls_agds *agds, int *error)
{
	char *function_name = "swpls_pr_agds";
	int status = MB_SUCCESS;
	char starter[5];
	time_t tm;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       agds:  %p\n", (void *)agds);
		}

	if (verbose > 0)
		{
		sprintf(starter, "dbg%d", MIN(verbose, 9));
		}
	else
		{
		strcpy(starter, "    ");
		}

	tm = (time_t)trunc(agds->time_d);

	fprintf(fout, "\n%s  SWPLS_ID_PARSED_AGDS [ID: 0x%X] %d bytes\n", starter,
		SWPLS_ID_PARSED_AGDS, SWPLS_SIZE_AGDS);
	fprintf(fout, "%s        time_d:       %d :: %s", starter, agds->time_d, asctime(gmtime(&tm)));
	fprintf(fout, "%s        microsec:     %d\n", starter, agds->microsec);
	fprintf(fout, "%s        channel:      %u\n", starter, agds->channel);
	fprintf(fout, "%s        hardness:     %f\n", starter, agds->hardness);
	fprintf(fout, "%s        roughness:    %f\n", starter, agds->roughness);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_pr_agds */
/*----------------------------------------------------------------------*/
int swpls_pr_pos_offset(int verbose, FILE *fout, swpls_pos_offset *pos_offset,
	int *error)
{
	char *function_name = "swpls_pr_pos_offset";
	int status = MB_SUCCESS;
	char starter[5];
	time_t tm;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       pos_offset:  %p\n", (void *)pos_offset);
		}

	if (verbose > 0)
		{
		sprintf(starter, "dbg%d", MIN(verbose, 9));
		}
	else
		{
		strcpy(starter, "    ");
		}

	tm = (time_t)trunc(pos_offset->time_d);

	fprintf(fout, "\n%s  SWPLS_ID_POS_OFFSET [ID: 0x%X] %d bytes\n", starter,
		SWPLS_ID_POS_OFFSET, SWPLS_SIZE_POS_OFFSET);
	fprintf(fout, "%s        time_d:       %d :: %s", starter, pos_offset->time_d, asctime(gmtime(&tm)));
	fprintf(fout, "%s        microsec:     %d\n", starter,
		pos_offset->microsec);
	fprintf(fout, "%s        channel:      %u\n", starter, pos_offset->channel);
	fprintf(fout, "%s        height:       %f\n", starter, pos_offset->height);
	fprintf(fout, "%s        forward:      %f\n", starter, pos_offset->forward);
	fprintf(fout, "%s        starboard:    %f\n", starter,
		pos_offset->starboard);
	fprintf(fout, "%s        time:         %f\n", starter, pos_offset->time);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_pr_pos_offset */
/*----------------------------------------------------------------------*/
int swpls_pr_imu_offset(int verbose, FILE *fout, swpls_imu_offset *imu_offset,
	int *error)
{
	char *function_name = "swpls_pr_imu_offset";
	int status = MB_SUCCESS;
	char starter[5];
	time_t tm;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       imu_offset:  %p\n", (void *)imu_offset);
		}

	if (verbose > 0)
		{
		sprintf(starter, "dbg%d", MIN(verbose, 9));
		}
	else
		{
		strcpy(starter, "    ");
		}

	tm = (time_t)trunc(imu_offset->time_d);

	fprintf(fout, "\n%s  SWPLS_ID_IMU_OFFSET [ID: 0x%X] %d bytes\n", starter,
		SWPLS_ID_IMU_OFFSET, SWPLS_SIZE_IMU_OFFSET);
	fprintf(fout, "%s        time_d:       %d :: %s", starter, imu_offset->time_d, asctime(gmtime(&tm)));
	fprintf(fout, "%s        microsec:     %d\n", starter,
		imu_offset->microsec);
	fprintf(fout, "%s        channel:      %u\n", starter, imu_offset->channel);
	fprintf(fout, "%s        height:       %f\n", starter, imu_offset->height);
	fprintf(fout, "%s        forward:      %f\n", starter, imu_offset->forward);
	fprintf(fout, "%s        starboard:    %f\n", starter,
		imu_offset->starboard);
	fprintf(fout, "%s        time:         %f\n", starter, imu_offset->time);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_pr_imu_offset */
/*----------------------------------------------------------------------*/
int swpls_pr_txer_offset(int verbose, FILE *fout,
	swpls_txer_offset *txer_offset, int *error)
{
	char *function_name = "swpls_pr_txer_offset";
	int status = MB_SUCCESS;
	char starter[5];
	time_t tm;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       txer_offset:  %p\n", (void *)txer_offset);
		}

	if (verbose > 0)
		{
		sprintf(starter, "dbg%d", MIN(verbose, 9));
		}
	else
		{
		strcpy(starter, "    ");
		}

	tm = (time_t)trunc(txer_offset->time_d);

	fprintf(fout, "\n%s  SWPLS_ID_TXER_OFFSET [ID: 0x%X] %d bytes\n", starter,
		SWPLS_ID_TXER_OFFSET, SWPLS_SIZE_TXER_OFFSET);
	fprintf(fout, "%s        time_d:       %d :: %s", starter, txer_offset->time_d, asctime(gmtime(&tm)));
	fprintf(fout, "%s        microsec:     %d\n", starter,
		txer_offset->microsec);
	fprintf(fout, "%s        channel:      %u\n", starter,
		txer_offset->channel);
	fprintf(fout, "%s        height:       %f\n", starter, txer_offset->height);
	fprintf(fout, "%s        forward:      %f\n", starter,
		txer_offset->forward);
	fprintf(fout, "%s        starboard:    %f\n", starter,
		txer_offset->starboard);
	fprintf(fout, "%s        azimuth:      %f\n", starter,
		txer_offset->azimuth);
	fprintf(fout, "%s        elevation:    %f\n", starter,
		txer_offset->elevation);
	fprintf(fout, "%s        pitch:        %f\n", starter, txer_offset->pitch);
	fprintf(fout, "%s        skew:         %f\n", starter, txer_offset->skew);
	fprintf(fout, "%s        time:         %f\n", starter, txer_offset->time);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return (status);
}		/* swpls_pr_txer_offset */
/*----------------------------------------------------------------------*/
int swpls_pr_wl_offset(int verbose, FILE *fout, swpls_wl_offset *wl_offset,
	int *error)
{
	char *function_name = "swpls_pr_wl_offset";
	int status = MB_SUCCESS;
	char starter[5];
	time_t tm;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       wl_offset:  %p\n", (void *)wl_offset);
		}

	if (verbose > 0)
		{
		sprintf(starter, "dbg%d", MIN(verbose, 9));
		}
	else
		{
		strcpy(starter, "    ");
		}

	tm = (time_t)trunc(wl_offset->time_d);

	fprintf(fout, "\n%s  SWPLS_ID_WL_OFFSET [ID: 0x%X] %d bytes\n", starter,
		SWPLS_ID_WL_OFFSET, SWPLS_SIZE_WL_OFFSET);
	fprintf(fout, "%s        time_d:       %d :: %s", starter, wl_offset->time_d, asctime(gmtime(&tm)));
	fprintf(fout, "%s        microsec:     %d\n", starter, wl_offset->microsec);
	fprintf(fout, "%s        channel:      %u\n", starter, wl_offset->channel);
	fprintf(fout, "%s        height:       %f\n", starter, wl_offset->height);

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}		/* swpls_pr_wl_offset */
/* Get heave value from SXP Processed ping
 *
 * sxp_ping - SWATHplus processed ping
 * heave - heave component of ping
 */
static int get_sxp_heave(int verbose, swpls_sxpping *sxp_ping, double *heave,
	int *error)
{
	char *function_name = "get_sxp_heave";
	int status = MB_SUCCESS;
	swpls_vector txoffset;
	swpls_matrix vtow;

	*error = MB_ERROR_NO_ERROR;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       sxp_ping:    %p\n", (void *)sxp_ping);
		}

	/* crp to txer lever arms (body coordinate system) */
	txoffset.x = sxp_ping->txer_starboard;
	txoffset.y = -sxp_ping->txer_height;
	txoffset.z = sxp_ping->txer_forward;

	/* transform transducer lever arms from body to inertial system */
	swpls_init_transform(verbose, &vtow, error);
	swpls_concat_rotate_z(verbose, &vtow, +(-sxp_ping->roll) * DTR, error);
	swpls_concat_rotate_x(verbose, &vtow, +(-sxp_ping->pitch) * DTR, error);
	swpls_concat_rotate_y(verbose, &vtow, +sxp_ping->heading * DTR, error);
	swpls_transform(verbose, &vtow, &txoffset, error);

	*heave = sxp_ping->height - (-txoffset.y);

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       height:     %f\n", sxp_ping->height);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
} /* get_sxp_heave */
/* Set SXP Processed ping height value from heave
 *
 * heave - heave calculated by MB System
 * sxp_ping - SWATHplus processed ping
 */
static int set_sxp_height(int verbose, double heave, swpls_sxpping *sxp_ping,
	int *error)
{
	char *function_name = "set_sxp_height";
	int status = MB_SUCCESS;
	swpls_vector txoffset;
	swpls_matrix vtow;

	*error = MB_ERROR_NO_ERROR;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       heave:        %f\n", heave);
		}

	/* crp to txer lever arms (body coordinate system) */
	txoffset.x = sxp_ping->txer_starboard;
	txoffset.y = -sxp_ping->txer_height;
	txoffset.z = sxp_ping->txer_forward;

	/* transform transducer lever arms from body to inertial system */
	swpls_init_transform(verbose, &vtow, error);
	swpls_concat_rotate_z(verbose, &vtow, +(-sxp_ping->roll) * DTR, error);
	swpls_concat_rotate_x(verbose, &vtow, +(-sxp_ping->pitch) * DTR, error);
	swpls_concat_rotate_y(verbose, &vtow, +sxp_ping->heading * DTR, error);
	swpls_transform(verbose, &vtow, &txoffset, error);

	sxp_ping->height = heave + (-txoffset.y);

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       sxp_ping->height: %f\n", sxp_ping->height);
		fprintf(stderr, "dbg2       error:            %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
} /* set_sxp_height */
/*********************************************************************

   Following 3D Math algorithms are based on the books:

   Dunn, F. and Parberry, I. (2002) 3D Math Primer for Graphics and
   Game Development. Wordware, Sudbury, MA. 428 pp.

   Mak, Ronald (2003) The Java Programmer's Guide to Numerical Computing.
   Prentice Hall PTR, Upper Saddle River, NJ

   Axis are different from MB System defaults as follows:

   x - positive to starboard
   y - positive up
   z - positive forward

   rotation about x axis is positive nose down (pitch)
   rotation about y axis is positive nose right (heading)
   rotation about z axis is positive starboard (up)

   All angles in radians, all distances use common units (meters)

   The entire library is stack allocated. No need to free memory.

   Usage:

   1. Start a transformation by calling swpls_init_transform on an swpls_matrix
   2. Appy translations and rotations to the transformation matrix in the order
     you want
   3. Create a point or set of points you want to transform as swpls_vectors
   4. Finally, use swpls_transform to do the coordinate transformation

*********************************************************************/

/* Initialize a 4x3 transformation matrix
 *
 * m - transformation matrix to (re)initialize
 */
int swpls_init_transform(int verbose, swpls_matrix *m, int *error)
{
	char *function_name = "swpls_init_transform";
	int status = MB_SUCCESS;

	*error = MB_ERROR_NO_ERROR;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       m:            %p\n", (void *)m);
		}

	status = set_identity(verbose, m, error);

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}		/* swpls_init_transform */
/* Apply a translation action to the transformation matrix
 *
 * dx - translate along x-axis (positive to right)
 * dy - translate along y-axis (positive up)
 * dz - translate along z-axis (positive forward)
 */
int swpls_concat_translate(int verbose, swpls_matrix *m, double dx, double dy,
	double dz, int *error)
{
	char *function_name = "swpls_concat_translation";
	int status = MB_SUCCESS;
	swpls_matrix translate;

	*error = MB_ERROR_NO_ERROR;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       m:            %p\n", (void *)m);
		fprintf(stderr, "dbg2       dx:           %f.3\n", dx);
		fprintf(stderr, "dbg2       dy:           %f.3\n", dy);
		fprintf(stderr, "dbg2       dz:           %f.3\n", dz);
		}

	status = set_identity(verbose, &translate, error);
	if (status == MB_SUCCESS)
		{
		translate.tx = dx;
		translate.ty = dy;
		translate.tz = dz;
		}

	if (status == MB_SUCCESS)
		{
		status = concat_transform(verbose, m, &translate, error);
		}

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}		/* swpls_concat_translation */
/* Apply a rotation about the x-axis to the transformation matrix
 *
 * pitch - angle of declination in radians (positive nose down)
 */
int swpls_concat_rotate_x(int verbose, swpls_matrix *m, double pitch,
	int *error)
{
	char *function_name = "swpls_concat_rotate_x";
	int status = MB_SUCCESS;
	double sinp, cosp;
	swpls_matrix rotate;

	*error = MB_ERROR_NO_ERROR;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       m:            %p\n", (void *)m);
		fprintf(stderr, "ddb2       pitch:        %f\n", pitch);
		}

	sinp = sin(pitch);
	cosp = cos(pitch);

	status = set_identity(verbose, &rotate, error);
	if (status == MB_SUCCESS)
		{
		rotate.m22 = cosp;
		rotate.m23 = sinp;
		rotate.m32 = -sinp;
		rotate.m33 = cosp;
		concat_transform(verbose, m, &rotate, error);
		}

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}		/* swpls_concat_rotate_x */
/* Apply a roation about the y-axis to the transformation matrix
 *
 * heading - angle of heading in radians (positive to right)
 */
int swpls_concat_rotate_y(int verbose, swpls_matrix *m, double heading,
	int *error)
{
	char *function_name = "swpls_concat_rotate_y";
	int status = MB_SUCCESS;
	double sint, cost;
	swpls_matrix rotate;

	*error = MB_ERROR_NO_ERROR;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       m:            %p\n", (void *)m);
		fprintf(stderr, "dbg2       heading:      %f\n", heading);
		}

	sint = sin(heading);
	cost = cos(heading);

	status = set_identity(verbose, &rotate, error);
	if (status == MB_SUCCESS)
		{
		rotate.m11 = cost;
		rotate.m13 = -sint;
		rotate.m31 = sint;
		rotate.m33 = cost;
		status = concat_transform(verbose, m, &rotate, error);
		}

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}		/* swpls_concat_rotate_y */
/* Apply rotation about the z-axis to the transformtion matrix
 *
 * bank - angle of bank in radians (positive starboard up)
 */
int swpls_concat_rotate_z(int verbose, swpls_matrix *m, double bank, int *error)
{
	char *function_name = "swpls_concat_rotate_z";
	int status = MB_SUCCESS;
	double sint, cost;
	swpls_matrix rotate;

	*error = MB_ERROR_NO_ERROR;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       m:            %p\n", (void *)m);
		fprintf(stderr, "dbg2       bank:         %f\n", bank);
		}

	sint = sin(bank);
	cost = cos(bank);

	status = set_identity(verbose, &rotate, error);
	if (status == MB_SUCCESS)
		{
		rotate.m11 = cost;
		rotate.m12 = sint;
		rotate.m21 = -sint;
		rotate.m22 = cost;
		status = concat_transform(verbose, m, &rotate, error);
		}

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}		/* swpls_concat_rotate_z */
/* Apply the transformations defined in m to vector p
 *
 * m - transformation matrix
 * p - vector to modify
 */
int swpls_transform(int verbose, const swpls_matrix *m, swpls_vector *p,
	int *error)
{
	char *function_name = "swpls_transform";
	int status = MB_SUCCESS;
	double x, y, z;

	*error = MB_ERROR_NO_ERROR;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       m:            %p\n", (void *)m);
		fprintf(stderr, "dbg2       p:            %p\n", (void *)p);
		fprintf(stderr, "dbg2       p->x:         %f\n", p->x);
		fprintf(stderr, "dbg2       p->y:         %f\n", p->y);
		fprintf(stderr, "dbg2       p->z:         %f\n", p->z);
		}

	x = p->x * m->m11 + p->y * m->m21 + p->z * m->m31 + m->tx;
	y = p->x * m->m12 + p->y * m->m22 + p->z * m->m32 + m->ty;
	z = p->x * m->m13 + p->y * m->m23 + p->z * m->m33 + m->tz;

	p->x = x;
	p->y = y;
	p->z = z;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2       p->x:       %f\n", p->x);
		fprintf(stderr, "dbg2       p->y:       %f\n", p->y);
		fprintf(stderr, "dbg2       p->z:       %f\n", p->z);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}		/* swpls_transform */
/* Setup a quaternion to perform an object->inertial rotation
 *
 * orientation - the orientation in Euler angle format
 * q - output quaternion
 */
int swpls_angles_to_quat(int verbose, const swpls_angles *orientation,
	swpls_quaternion *q, int *error)
{
	char *function_name = "swpls_angles_to_quat";
	int status = MB_SUCCESS;
	double sp, sb, sh;
	double cp, cb, ch;

	*error = MB_ERROR_NO_ERROR;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       orientation:  %p\n", (void *)orientation);
		fprintf(stderr, "dbg2       q:            %p\n", (void *)q);
		}

	sp = sin(0.5 * orientation->pitch);
	cp = cos(0.5 * orientation->pitch);
	sb = sin(0.5 * orientation->bank);
	cb = cos(0.5 * orientation->bank);
	sh = sin(0.5 * orientation->heading);
	ch = cos(0.5 * orientation->heading);

	q->w = ch * cp * cb + sh * sp * sb;
	q->x = ch * sp * cb + sh * cp * sb;
	q->y = -ch * sp * sb + sh * cp * cb;
	q->z = -sh * sp * cb + ch * cp * sb;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}		/* swpls_angles_to_quat */
/* Setup the Euler angles, given an object -> inertial rotation
 * quaternion
 *
 * q - object->inertial quaternion
 * orientation - output orienation in Euler angle format
 */
int swpls_quat_to_angles(int verbose, const swpls_quaternion *q,
	swpls_angles *orientation, int *error)
{
	char *function_name = "swpls_quat_to_angles";
	int status = MB_SUCCESS;
	double sp;

	*error = MB_ERROR_NO_ERROR;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       q:            %p\n", (void *)q);
		fprintf(stderr, "dbg2       orientation:  %p\n", (void *)orientation);
		}

	sp = -2.0 * (q->y * q->z - q->w * q->x);

	if (fabs(sp) > 0.9999)
		{
		orientation->pitch = kPiOver2 * sp;
		orientation->heading = atan2(-q->x * q->z + q->w * q->y,
			0.5 - q->y * q->y - q->z * q->z);
		orientation->bank = 0.0;
		}
	else
		{
		orientation->pitch = asin(sp);
		orientation->heading = atan2(q->x * q->z + q->w * q->y,
			0.5 - q->x * q->x - q->y * q->y);
		orientation->bank = atan2(q->x * q->y + q->w * q->z,
			0.5 - q->x * q->x - q->z * q->z);
		}

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}		/* swpls_quat_to_angles */
/* Spherical linear interpolation
 *
 * q0 - starting orientation as an object->inertial quaternion
 * q1 - ending orientation as an object->inertial quaternion
 * t - interpolation fraction ranging from 0 to 1 (corresponging to q0 and
 * q1, respectively)
 * q - iterpolated orientation quaternion coresponding to t
 */
int swpls_slerp(int verbose, const swpls_quaternion *q0,
	const swpls_quaternion *q1, double t, swpls_quaternion *q,
	int *error)
{
	char *function_name = "swpls_slerp";
	int status = MB_SUCCESS;
	double q1w, q1x, q1y, q1z;
	double k0, k1;
	double omega, cosOmega, sinOmega, oneOverSinOmega;

	*error = MB_ERROR_NO_ERROR;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       q0:           %p\n", (void *)q0);
		fprintf(stderr, "dbg2       q1:           %p\n", (void *)q1);
		fprintf(stderr, "dbg2       t:            %f\n", t);
		fprintf(stderr, "dbg2       q:            %p\n", (void *)q);
		}

	/* Check for out of range parameter and return edge points if so */
	if (t <= 0.0)
		{
		*q = *q0;
		}
	else if (t >= 1.0)
		{
		*q = *q1;
		}
	else
		{
		/* Compute "cosine of angle between quaternions" using dot product
		 */
		cosOmega = q0->w * q1->w + q0->x * q1->x + q0->y * q1->y + q0->z *
			q1->z;

		/* Chose q or -q to rotate using the acute angle */
		q1w = q1->w;
		q1x = q1->x;
		q1y = q1->y;
		q1z = q1->z;
		if (cosOmega < 0.0)
			{
			q1w = -q1w;
			q1x = -q1x;
			q1y = -q1y;
			q1z = -q1z;
			cosOmega = -cosOmega;
			}

		/* We should have two unit quaternions, so dot should be <= 1.0
		   assert(cosOmega < 1.1); */

		/* Compute interpolation fraction */
		if (cosOmega > 0.9999)
			{
			/* very close - just use linear interpolation */
			k0 = 1.0 - t;
			k1 = t;
			}
		else
			{
			sinOmega = sqrt(1.0 - cosOmega * cosOmega);
			omega = atan2(sinOmega, cosOmega);
			oneOverSinOmega = 1.0 / sinOmega;
			k0 = sin((1.0 - t) * omega) * oneOverSinOmega;
			k1 = sin(t * omega) * oneOverSinOmega;
			}

		/* Interpolate */
		q->x = k0 * q0->x + k1 * q1x;
		q->y = k0 * q0->y + k1 * q1y;
		q->z = k0 * q0->z + k1 * q1z;
		q->w = k0 * q0->w + k1 * q1w;
		}

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}		/* swpls_slerp */
/* wrap an angle in range -pi ... pi by adding the correct multiple of
 * 2pi
 *
 * theta angle to wrap (radians)
 */
static double wrap_pi(double theta)
{
	theta += kPi;
	theta -= floor(theta * k1Over2Pi) * k2Pi;
	theta -= kPi;

	return (theta);
}
/* set/reset an swpls_matrix back to the identity matrix
 *
 * m - transformation matrix to reset
 */
static int set_identity(int verbose, swpls_matrix *m, int *error)
{
	char *function_name = "set_identity";
	int status = MB_SUCCESS;

	*error = MB_ERROR_NO_ERROR;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       m:            %p\n", (void *)m);
		}

	/* rotation matrix */
	m->m11 = 1.0; m->m12 = 0.0; m->m13 = 0.0;
	m->m21 = 0.0; m->m22 = 1.0; m->m23 = 0.0;
	m->m31 = 0.0; m->m32 = 0.0; m->m33 = 1.0;

	/* translation vector */
	m->tx = 0.0; m->ty = 0.0; m->tz = 0.0;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}		/* set_identity */
/* concatinates a transformation matrix b onto an existing transformation
 * matrix a
 *
 * a - transformation matrix a
 * b - transformation matrix b
 */
static int concat_transform(int verbose, swpls_matrix *a, swpls_matrix *b,
	int *error)
{
	char *function_name = "concat_transform";
	int status = MB_SUCCESS;
	swpls_matrix r;

	*error = MB_ERROR_NO_ERROR;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called.\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       a:            %p\n", (void *)a);
		fprintf(stderr, "dbg2       b:            %p\n", (void *)b);
		}

	/* rotation matrix */
	r.m11 = a->m11 * b->m11 + a->m12 * b->m21 + a->m13 * b->m31;
	r.m12 = a->m11 * b->m12 + a->m12 * b->m22 + a->m13 * b->m32;
	r.m13 = a->m11 * b->m13 + a->m12 * b->m23 + a->m13 * b->m33;

	r.m21 = a->m21 * b->m11 + a->m22 * b->m21 + a->m23 * b->m31;
	r.m22 = a->m21 * b->m12 + a->m22 * b->m22 + a->m23 * b->m32;
	r.m23 = a->m21 * b->m13 + a->m22 * b->m23 + a->m23 * b->m33;

	r.m31 = a->m31 * b->m11 + a->m32 * b->m21 + a->m33 * b->m31;
	r.m32 = a->m31 * b->m12 + a->m32 * b->m22 + a->m33 * b->m32;
	r.m33 = a->m31 * b->m13 + a->m32 * b->m23 + a->m33 * b->m33;

	/* translation vector */
	r.tx = a->tx * b->m11 + a->ty * b->m21 + a->tz * b->m31 + b->tx;
	r.ty = a->tx * b->m12 + a->ty * b->m22 + a->tz * b->m32 + b->ty;
	r.tz = a->tx * b->m13 + a->ty * b->m23 + a->tz * b->m33 + b->tz;

	/* copy the results back into first matrix */
	a->m11 = r.m11; a->m12 = r.m12; a->m13 = r.m13;
	a->m21 = r.m21; a->m22 = r.m22; a->m23 = r.m23;
	a->m31 = r.m31; a->m32 = r.m32; a->m33 = r.m33;
	a->tx = r.tx; a->ty = r.ty; a->tz = r.tz;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}		/* concat_transform */


/*--------------------------------------------------------------------
 *    The MB-system:	mb_platform.c	11/1/00
 *    $Id$
 *
 *    Copyright (c) 2015-2015 by
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
 * This source file includes the functions used to read and write
 * platform definition files.
 * The structures used to store platform, sensor, and offset information
 * are defined in mb_io.h
 *
 * Author:	D. W. Caress
 * Date:	June 26, 2015
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"
#include "mb_segy.h"

static char svn_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mb_platform_init(int verbose, int type, char *name, char *organization,
			int source_swathbathymetry, int source_position,
			int source_depth, int source_heave, int source_attitude,
			void **platform_ptr, int *error)
{
	char	*function_name = "mb_platform_init";
	int	status = MB_SUCCESS;
	struct mb_platform_struct *platform;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                  %d\n", verbose);
		fprintf(stderr,"dbg2       type:		     %d\n", type);
		fprintf(stderr,"dbg2       name:		     %s\n", name);
		fprintf(stderr,"dbg2       organization:	     %s\n", organization);
		fprintf(stderr,"dbg2       source_swathbathymetry:   %d\n", source_swathbathymetry);
		fprintf(stderr,"dbg2       source_position:	     %d\n", source_position);
		fprintf(stderr,"dbg2       source_depth:	     %d\n", source_depth);
		fprintf(stderr,"dbg2       source_heave:	     %d\n", source_heave);
		fprintf(stderr,"dbg2       source_attitude:	     %d\n", source_attitude);
		fprintf(stderr,"dbg2       platform_ptr:             %p\n", platform_ptr);
		fprintf(stderr,"dbg2       *platform_ptr:            %p\n", *platform_ptr);
		}

	/* allocate memory for platform descriptor structure if needed */
	if (*platform_ptr == NULL)
		{
		status = mb_mallocd(verbose,__FILE__, __LINE__,sizeof(struct mb_platform_struct),
				(void **) platform_ptr, error);
		if (status == MB_SUCCESS)
			{
			memset(*platform_ptr, 0, sizeof(struct mb_platform_struct));
			}
		}
		
	/* get platform structure */
	platform = (struct mb_platform_struct *) *platform_ptr;
	
	/* set values */
	platform->type = type;
	if (name != NULL)
		strcpy(platform->name, name);
	else
		memset(platform->name, 0, sizeof(mb_longname));
	if (organization != NULL)
		strcpy(platform->organization, organization);
	else
		memset(platform->name, 0, sizeof(mb_longname));
	platform->source_swathbathymetry = source_swathbathymetry;
	platform->source_position = source_position;
	platform->source_depth = source_depth;
	platform->source_heave = source_heave;
	platform->source_attitude = source_attitude;
	platform->num_sensors = 0;
	
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       platform_ptr:		     %p\n", platform_ptr);
		fprintf(stderr,"dbg2       *platform_ptr:		     %p\n", *platform_ptr);
		fprintf(stderr,"dbg2       platform->type:		     %d\n", platform->type);
		fprintf(stderr,"dbg2       platform->name:		     %s\n", platform->name);
		fprintf(stderr,"dbg2       platform->organization:	     %s\n", platform->organization);
		fprintf(stderr,"dbg2       platform->source_swathbathymetry: %d\n", platform->source_swathbathymetry);
		fprintf(stderr,"dbg2       platform->source_position:	     %d\n", platform->source_position);
		fprintf(stderr,"dbg2       platform->source_depth:	     %d\n", platform->source_depth);
		fprintf(stderr,"dbg2       platform->source_heave:	     %d\n", platform->source_heave);
		fprintf(stderr,"dbg2       platform->source_attitude:	     %d\n", platform->source_attitude);
		fprintf(stderr,"dbg2       platform->num_sensors:	     %d\n", platform->num_sensors);
		fprintf(stderr,"dbg2       error:			     %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:			     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_platform_add_sensor(int verbose, void **platform_ptr,
					   int type, mb_longname model,
				   mb_longname manufacturer,
				   mb_longname serialnumber,
				   int capability, int special_capability,
				   int num_offsets, int num_time_latency,
				   int *error)
{
	char	*function_name = "mb_platform_add_sensor";
	int	status = MB_SUCCESS;
	struct mb_platform_struct *platform;
	struct mb_sensor_struct *sensor;
	struct mb_sensor_offset_struct *offset;
	int	isensor;
	int	ioffset;
	char	*message = NULL;
	size_t	size;
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:              %d\n", verbose);
		fprintf(stderr,"dbg2       platform_ptr:         %p\n", platform_ptr);
		fprintf(stderr,"dbg2       *platform_ptr:        %p\n", *platform_ptr);
		fprintf(stderr,"dbg2       type:                 %d\n", type);
		fprintf(stderr,"dbg2       model:                %s\n", model);
		fprintf(stderr,"dbg2       manufacturer:         %s\n", manufacturer);
		fprintf(stderr,"dbg2       serialnumber:         %s\n", serialnumber);
		fprintf(stderr,"dbg2       capability:           %d\n", capability);
		fprintf(stderr,"dbg2       special_capability:   %d\n", special_capability);
		fprintf(stderr,"dbg2       num_offsets:          %d\n", num_offsets);
		fprintf(stderr,"dbg2       num_time_latency:     %d\n", num_time_latency);
		}

	/* allocate memory for platform descriptor structure if needed */
	if (*platform_ptr == NULL)
		{
		status = mb_mallocd(verbose,__FILE__, __LINE__,sizeof(struct mb_platform_struct),
				(void **) platform_ptr, error);
		if (status == MB_SUCCESS)
			{
			memset(*platform_ptr, 0, sizeof(struct mb_platform_struct));
			}
		}
		
	/* get platform structure */
	platform = (struct mb_platform_struct *) *platform_ptr;
	
	/* allocate memory for sensor if needed */
	platform->num_sensors++;
	if (platform->num_sensors > platform->num_sensors_alloc)
		{
		size = platform->num_sensors * sizeof(struct mb_sensor_struct);
		status = mb_reallocd(verbose,__FILE__, __LINE__,
					size, (void **) &platform->sensors, error);
		if (status == MB_SUCCESS)
			{
			memset(&platform->sensors[platform->num_sensors_alloc],
				   0, (platform->num_sensors - platform->num_sensors_alloc) * sizeof(struct mb_sensor_struct));
			platform->num_sensors_alloc = platform->num_sensors;
			}
		else
			{
			mb_error(verbose,*error,&message);
			fprintf(stderr,"\nMBIO Error allocating sensor structures:\n%s\n",message);
			fprintf(stderr,"\nProgram terminated in function <%s>\n", function_name);
			exit(*error);
			}
		}
		
	/* allocate memory for offsets if needed */
	isensor = platform->num_sensors - 1;
	sensor = &platform->sensors[isensor];
	sensor->num_offsets = num_offsets;
	if (sensor->num_offsets > sensor->num_offsets_alloc)
		{
		size = sensor->num_offsets * sizeof(struct mb_sensor_offset_struct);
		status = mb_reallocd(verbose,__FILE__, __LINE__,
					size, (void **) &sensor->offsets, error);
		if (status == MB_SUCCESS)
			{
			memset(sensor->offsets, 0, size);
			sensor->num_offsets_alloc = sensor->num_offsets;
			}
		else
			{
			mb_error(verbose,*error,&message);
			fprintf(stderr,"\nMBIO Error allocating sensor offsets structures:\n%s\n",message);
			fprintf(stderr,"\nProgram terminated in function <%s>\n", function_name);
			exit(*error);
			}
		}
		
	/* allocate memory for time latency model if needed */
	if (num_time_latency > 0)
		{
		for (ioffset=0;ioffset<sensor->num_offsets;ioffset++)
			{
			offset = &sensor->offsets[ioffset];
			
			size = num_time_latency * sizeof(double);
			status = mb_reallocd(verbose,__FILE__, __LINE__,
						size, (void **) &offset->time_latency_time_d, error);
			if (status == MB_SUCCESS)
			status = mb_reallocd(verbose,__FILE__, __LINE__,
						size, (void **) &offset->time_latency_value, error);
			if (status == MB_SUCCESS)
				{
				memset(offset->time_latency_time_d, 0, size);
				memset(offset->time_latency_value, 0, size);
				offset->num_time_latency_alloc = num_time_latency;
				}
			else
				{
				mb_error(verbose,*error,&message);
				fprintf(stderr,"\nMBIO Error allocating sensor offsets structures:\n%s\n",message);
				fprintf(stderr,"\nProgram terminated in function <%s>\n", function_name);
				exit(*error);
				}
			}
		}
	
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       platform_ptr:		%p\n", platform_ptr);
		}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR)
		{
		fprintf(stderr,"dbg2       *platform_ptr:		     %p\n", *platform_ptr);
		fprintf(stderr,"dbg2       platform->type:		     %d\n", platform->type);
		fprintf(stderr,"dbg2       platform->name:		     %s\n", platform->name);
		fprintf(stderr,"dbg2       platform->organization:	     %s\n", platform->organization);
		fprintf(stderr,"dbg2       platform->source_swathbathymetry: %d\n", platform->source_swathbathymetry);
		fprintf(stderr,"dbg2       platform->source_position:	     %d\n", platform->source_position);
		fprintf(stderr,"dbg2       platform->source_depth:	     %d\n", platform->source_depth);
		fprintf(stderr,"dbg2       platform->source_heave:	     %d\n", platform->source_heave);
		fprintf(stderr,"dbg2       platform->source_attitude:	     %d\n", platform->source_attitude);
		fprintf(stderr,"dbg2       platform->num_sensors:	     %d\n", platform->num_sensors);
		for (i=0;i<platform->num_sensors;i++)
			{
			fprintf(stderr,"dbg2       platform->sensors[%2d].type:                 %d\n", i, platform->sensors[i].type);
			fprintf(stderr,"dbg2       platform->sensors[%2d].model:                %s\n", i, platform->sensors[i].model);
			fprintf(stderr,"dbg2       platform->sensors[%2d].manufacturer:         %s\n", i, platform->sensors[i].manufacturer);
			fprintf(stderr,"dbg2       platform->sensors[%2d].serialnumber:         %s\n", i, platform->sensors[i].serialnumber);
			fprintf(stderr,"dbg2       platform->sensors[%2d].capability:           %d\n", i, platform->sensors[i].capability);
			fprintf(stderr,"dbg2       platform->sensors[%2d].special_capability:   %d\n", i, platform->sensors[i].special_capability);
			fprintf(stderr,"dbg2       platform->sensors[%2d].num_offsets:          %d\n", i, platform->sensors[i].num_offsets);
			for (j=0;j<platform->sensors[i].num_offsets;j++)
				{
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].time_latency_mode:	%d\n", i, j, platform->sensors[i].offsets[j].time_latency_mode);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].time_latency_static:	%f\n", i, j, platform->sensors[i].offsets[j].time_latency_static);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].num_time_latency:		%d\n", i, j, platform->sensors[i].offsets[j].num_time_latency);
				for (k=0;k<platform->sensors[i].offsets[j].num_time_latency;k++)
					{
					fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].time_latency[%2d]:		%16.6f %8.6f\n", i, j, k,
						platform->sensors[i].offsets[j].time_latency_time_d[k],platform->sensors[i].offsets[j].time_latency_value[k]);
					}
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_mode:	%d\n", i, j, platform->sensors[i].offsets[j].position_offset_mode);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_x:	%f\n", i, j, platform->sensors[i].offsets[j].position_offset_x);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_y:	%f\n", i, j, platform->sensors[i].offsets[j].position_offset_y);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_z:	%f\n", i, j, platform->sensors[i].offsets[j].position_offset_z);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_mode:	%d\n", i, j, platform->sensors[i].offsets[j].attitude_offset_mode);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_azimuth:	%f\n", i, j, platform->sensors[i].offsets[j].attitude_offset_azimuth);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_roll:	%f\n", i, j, platform->sensors[i].offsets[j].attitude_offset_roll);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_pitch:	%f\n", i, j, platform->sensors[i].offsets[j].attitude_offset_pitch);
				}
			}
		}
	if (verbose >= 2)
		{
		fprintf(stderr,"dbg2       error:			%d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:			%d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_platform_add_sensor_offset(int verbose, void **platform_ptr,
					int isensor, int ioffset,
					int time_latency_mode,
					double time_latency_static,
					int num_time_latency,
					double *time_latency_time_d,
					double *time_latency_value,
					int position_offset_mode,
					double position_offset_x,
					double position_offset_y,
					double position_offset_z,   
					int attitude_offset_mode,
					double attitude_offset_azimuth,
					double attitude_offset_roll,
					double attitude_offset_pitch,
					int *error)
{
	char	*function_name = "mb_platform_add_sensor_offset";
	int	status = MB_SUCCESS;
	struct mb_platform_struct *platform;
	struct mb_sensor_struct *sensor;
	struct mb_sensor_offset_struct *offset;
	char	*message = NULL;
	size_t	size;
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                 %d\n", verbose);
		fprintf(stderr,"dbg2       platform_ptr:            %p\n", platform_ptr);
		fprintf(stderr,"dbg2       *platform_ptr:           %p\n", *platform_ptr);
		fprintf(stderr,"dbg2       isensor:                 %d\n", isensor);
		fprintf(stderr,"dbg2       ioffset:                 %d\n", ioffset);
		fprintf(stderr,"dbg2       time_latency_mode:	    %d\n", time_latency_mode);
		fprintf(stderr,"dbg2       time_latency_static:	    %f\n", time_latency_static);
		fprintf(stderr,"dbg2       num_time_latency:        %d\n", num_time_latency);
		for (k=0;k<num_time_latency;k++)
			{
			fprintf(stderr,"dbg2       time_latency[%2d]:       %16.6f %8.6f\n", 
				k, time_latency_time_d[k],time_latency_value[k]);
			}
		fprintf(stderr,"dbg2       position_offset_mode:    %d\n", position_offset_mode);
		fprintf(stderr,"dbg2       position_offset_x:       %f\n", position_offset_x);
		fprintf(stderr,"dbg2       position_offset_y:	    %f\n", position_offset_y);
		fprintf(stderr,"dbg2       position_offset_z:	    %f\n", position_offset_z);
		fprintf(stderr,"dbg2       attitude_offset_mode:    %d\n", attitude_offset_mode);
		fprintf(stderr,"dbg2       attitude_offset_azimuth: %f\n", attitude_offset_azimuth);
		fprintf(stderr,"dbg2       attitude_offset_roll:    %f\n", attitude_offset_roll);
		fprintf(stderr,"dbg2       attitude_offset_pitch:   %f\n", attitude_offset_pitch);
		}

	/* get platform and sensor structures */
	platform = (struct mb_platform_struct *) *platform_ptr;
	sensor = &platform->sensors[isensor];
		
	/* allocate memory for offsets if needed */
	if (ioffset > sensor->num_offsets - 1)
		sensor->num_offsets = ioffset + 1;
	if (sensor->num_offsets > sensor->num_offsets_alloc)
		{
		size = sensor->num_offsets * sizeof(struct mb_sensor_offset_struct);
		status = mb_reallocd(verbose,__FILE__, __LINE__,
					size, (void **) &sensor->offsets, error);
		if (status == MB_SUCCESS)
			{
			memset(sensor->offsets, 0, size);
			sensor->num_offsets_alloc = sensor->num_offsets;
			}
		else
			{
			mb_error(verbose,*error,&message);
			fprintf(stderr,"\nMBIO Error allocating sensor offsets structures:\n%s\n",message);
			fprintf(stderr,"\nProgram terminated in function <%s>\n", function_name);
			exit(*error);
			}
		}

	/* get offset structure */
	offset = &sensor->offsets[ioffset];
		
	/* allocate memory for time latency model if needed */
	if (num_time_latency > 0)
		{
		size = num_time_latency * sizeof(double);
		status = mb_reallocd(verbose,__FILE__, __LINE__,
					size, (void **) &offset->time_latency_time_d, error);
		if (status == MB_SUCCESS)
		status = mb_reallocd(verbose,__FILE__, __LINE__,
					size, (void **) &offset->time_latency_value, error);
		if (status == MB_SUCCESS)
			{
			memset(offset->time_latency_time_d, 0, size);
			memset(offset->time_latency_value, 0, size);
			offset->num_time_latency_alloc = num_time_latency;
			}
		else
			{
			mb_error(verbose,*error,&message);
			fprintf(stderr,"\nMBIO Error allocating sensor offsets structures:\n%s\n",message);
			fprintf(stderr,"\nProgram terminated in function <%s>\n", function_name);
			exit(*error);
			}
		}

	/* set offset values */
	offset->time_latency_mode = time_latency_mode;
	offset->time_latency_static = time_latency_static;
	offset->num_time_latency = num_time_latency;
	for (k=0;k<offset->num_time_latency;k++)
		{
		offset->time_latency_time_d[k] = time_latency_time_d[k];
		offset->time_latency_value[k] = time_latency_value[k];
		}
	offset->position_offset_mode = position_offset_mode;
	offset->position_offset_x = position_offset_x;
	offset->position_offset_y = position_offset_y;
	offset->position_offset_z = position_offset_z;
	offset->attitude_offset_mode = attitude_offset_mode;
	offset->attitude_offset_azimuth = attitude_offset_azimuth;
	offset->attitude_offset_roll = attitude_offset_roll;
	offset->attitude_offset_pitch = attitude_offset_pitch;
	
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       platform_ptr:		%p\n", platform_ptr);
		}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR)
		{
		fprintf(stderr,"dbg2       *platform_ptr:				%p\n", *platform_ptr);
		fprintf(stderr,"dbg2       platform->type:				%d\n", platform->type);
		fprintf(stderr,"dbg2       platform->name:				%s\n", platform->name);
		fprintf(stderr,"dbg2       platform->organization:		%s\n", platform->organization);
		fprintf(stderr,"dbg2       platform->source_swathbathymetry:	%d\n", platform->source_swathbathymetry);
		fprintf(stderr,"dbg2       platform->source_position:	%d\n", platform->source_position);
		fprintf(stderr,"dbg2       platform->source_depth:		%d\n", platform->source_depth);
		fprintf(stderr,"dbg2       platform->source_heave:		%d\n", platform->source_heave);
		fprintf(stderr,"dbg2       platform->source_attitude:	%d\n", platform->source_attitude);
		fprintf(stderr,"dbg2       platform->num_sensors:		%d\n", platform->num_sensors);
		for (i=0;i<platform->num_sensors;i++)
			{
			fprintf(stderr,"dbg2       platform->sensors[%2d].type:                 %d\n", i, platform->sensors[i].type);
			fprintf(stderr,"dbg2       platform->sensors[%2d].model:                %s\n", i, platform->sensors[i].model);
			fprintf(stderr,"dbg2       platform->sensors[%2d].manufacturer:         %s\n", i, platform->sensors[i].manufacturer);
			fprintf(stderr,"dbg2       platform->sensors[%2d].serialnumber:         %s\n", i, platform->sensors[i].serialnumber);
			fprintf(stderr,"dbg2       platform->sensors[%2d].capability:           %d\n", i, platform->sensors[i].capability);
			fprintf(stderr,"dbg2       platform->sensors[%2d].special_capability:   %d\n", i, platform->sensors[i].special_capability);
			fprintf(stderr,"dbg2       platform->sensors[%2d].num_offsets:          %d\n", i, platform->sensors[i].num_offsets);
			for (j=0;j<platform->sensors[i].num_offsets;j++)
				{
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].time_latency_mode:	%d\n", i, j, platform->sensors[i].offsets[j].time_latency_mode);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].time_latency_static:	%f\n", i, j, platform->sensors[i].offsets[j].time_latency_static);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].num_time_latency:		%d\n", i, j, platform->sensors[i].offsets[j].num_time_latency);
				for (k=0;k<platform->sensors[i].offsets[j].num_time_latency;k++)
					{
					fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].time_latency[%2d]:		%16.6f %8.6f\n", i, j, k,
						platform->sensors[i].offsets[j].time_latency_time_d[k],platform->sensors[i].offsets[j].time_latency_value[k]);
					}
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_mode:	%d\n", i, j, platform->sensors[i].offsets[j].position_offset_mode);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_x:	%f\n", i, j, platform->sensors[i].offsets[j].position_offset_x);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_y:	%f\n", i, j, platform->sensors[i].offsets[j].position_offset_y);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_z:	%f\n", i, j, platform->sensors[i].offsets[j].position_offset_z);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_mode:	%d\n", i, j, platform->sensors[i].offsets[j].attitude_offset_mode);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_azimuth:	%f\n", i, j, platform->sensors[i].offsets[j].attitude_offset_azimuth);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_roll:	%f\n", i, j, platform->sensors[i].offsets[j].attitude_offset_roll);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_pitch:	%f\n", i, j, platform->sensors[i].offsets[j].attitude_offset_pitch);
				}
			}
		}
	if (verbose >= 2)
		{
		fprintf(stderr,"dbg2       error:			%d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:			%d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
	
int mb_platform_deall(int verbose, void **platform_ptr, int *error)
{
	char	*function_name = "mb_platform_deall";
	int	status = MB_SUCCESS;
	struct mb_platform_struct *platform;
	struct mb_sensor_struct *sensor;
	struct mb_sensor_offset_struct *offset;
	int	isensor;
	int	ioffset;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n", verbose);
		fprintf(stderr,"dbg2       platform_ptr:      %p\n", platform_ptr);
		fprintf(stderr,"dbg2       *platform_ptr:     %p\n", *platform_ptr);
		}

	/* free memory for platform descriptor structure if needed */
	if (*platform_ptr != NULL)
		{
		/* get platform structure */
		platform = (struct mb_platform_struct *) *platform_ptr;
		
		/* loop over all sensors */
		for (isensor=0;isensor<platform->num_sensors_alloc;isensor++)
			{
			sensor = (struct mb_sensor_struct *) &platform->sensors[isensor];
			
			/* free all offsets */
			if (sensor->num_offsets_alloc > 0
				&& sensor->offsets != NULL)
				{
				/* free any time latency model */
				for (ioffset=0;ioffset<sensor->num_offsets;ioffset++)
					{
					offset = (struct mb_sensor_offset_struct *) &sensor->offsets[ioffset];
					if (offset->num_time_latency_alloc > 0)
						{
						status = mb_freed(verbose,__FILE__, __LINE__,
							(void **) &offset->time_latency_time_d, error);
						status = mb_freed(verbose,__FILE__, __LINE__,
							(void **) &offset->time_latency_value, error);
						offset->num_time_latency_alloc = 0;
						}
					}
				
				status = mb_freed(verbose,__FILE__, __LINE__,
						(void **) &sensor->offsets, error);
				sensor->num_offsets_alloc = 0;
				}
			}

		/* free all offsets */
		if (platform->num_sensors_alloc > 0
			&& platform->sensors != NULL)
			{
			status = mb_freed(verbose,__FILE__, __LINE__,
					(void **) &platform->sensors, error);
			}
			
		/* free platform structure */
		status = mb_freed(verbose,__FILE__, __LINE__,
				(void **) platform_ptr, error);
		}
	
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       platform_ptr:		%p\n", platform_ptr);
		fprintf(stderr,"dbg2       *platform_ptr:     %p\n", *platform_ptr);
		fprintf(stderr,"dbg2       error:			%d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:			%d\n",status);
		}

	/* return status */
	return(status);
}
	
/*--------------------------------------------------------------------*/

int mb_platform_read(int verbose, char *platform_file, void **platform_ptr, int *error)
{
	char	*function_name = "mb_platform_read";
	int	status = MB_SUCCESS;
	struct mb_platform_struct *platform;
	size_t	size;
	FILE	*fp;
	char	buffer[MB_PATH_MAXLINE], dummy[MB_PATH_MAXLINE], *result, *message;
	int	ivalue;
	double	dvalue, dvalue2, dvalue3;
	char	svalue[MB_PATH_MAXLINE];
	int 	len;
	int	isensor, ioffset;
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n", verbose);
		fprintf(stderr,"dbg2       platform_file:     %s\n", platform_file);
		fprintf(stderr,"dbg2       platform_ptr:      %p\n", platform_ptr);
		fprintf(stderr,"dbg2       *platform_ptr:     %p\n", *platform_ptr);
		}

	/* allocate memory for platform descriptor structure if needed */
	if (*platform_ptr == NULL)
		{
		status = mb_mallocd(verbose,__FILE__, __LINE__,sizeof(struct mb_platform_struct),
				(void **) platform_ptr, error);
		if (status == MB_SUCCESS)
			{
			memset(*platform_ptr, 0, sizeof(struct mb_platform_struct));
			}
		}
		
	/* get platform structure */
	platform = (struct mb_platform_struct *) *platform_ptr;
		
	/* open and read platform file */
	if ((fp = fopen(platform_file, "r")) != NULL)
		{
		while ((result = fgets(buffer,MB_PATH_MAXLINE,fp)) == buffer)
			{
			if (buffer[0] != '#')
				{
				len = strlen(buffer);
				if (len > 0)
					{
					if (buffer[len-1] == '\n')
						buffer[len-1] = '\0';
					if (buffer[len-2] == '\r')
						buffer[len-2] = '\0';
					}
		
				/* general parameters */
				if (strncmp(buffer, "PLATFORM_TYPE", 13) == 0)
					{
					sscanf(buffer, "%s %d", dummy, &platform->type);
					}
				else if (strncmp(buffer, "PLATFORM_NAME", 13) == 0)
					{
					sscanf(buffer, "%s %s", dummy, platform->name);
					}
				else if (strncmp(buffer, "PLATFORM_ORGANIZATION", 21) == 0)
					{
					sscanf(buffer, "%s %s", dummy, platform->organization);
					}
				else if (strncmp(buffer, "SOURCE_SWATHBATHYMETRY", 15) == 0)
					{
					sscanf(buffer, "%s %d", dummy, &platform->source_swathbathymetry);
					}
				else if (strncmp(buffer, "SOURCE_POSITION", 15) == 0)
					{
					sscanf(buffer, "%s %d", dummy, &platform->source_position);
					}
				else if (strncmp(buffer, "SOURCE_DEPTH", 12) == 0)
					{
					sscanf(buffer, "%s %d", dummy, &platform->source_depth);
					}
				else if (strncmp(buffer, "SOURCE_HEAVE", 12) == 0)
					{
					sscanf(buffer, "%s %d", dummy, &platform->source_heave);
					}
				else if (strncmp(buffer, "SOURCE_ATTITUDE", 14) == 0)
					{
					sscanf(buffer, "%s %d", dummy, &platform->source_attitude);
					}
				else if (strncmp(buffer, "PLATFORM_NUM_SENSORS", 20) == 0)
					{
					sscanf(buffer, "%s %d", dummy, &platform->num_sensors);
					if (platform->num_sensors > platform->num_sensors_alloc)
						{
						size = platform->num_sensors * sizeof(struct mb_sensor_struct);
						status = mb_mallocd(verbose,__FILE__, __LINE__,
									size, (void **) &platform->sensors, error);
						if (status == MB_SUCCESS)
							{
							memset(platform->sensors, 0, size);
							platform->num_sensors_alloc = platform->num_sensors;
							}
						else
							{
							mb_error(verbose,*error,&message);
							fprintf(stderr,"\nMBIO Error allocating sensor structures:\n%s\n",message);
							fprintf(stderr,"\nProgram terminated in function <%s>\n", function_name);
							exit(*error);
							}
						}
					}
	
				else if (strncmp(buffer, "SENSOR_TYPE", 11) == 0)
					{
					sscanf(buffer, "%s %d %d", dummy, &isensor, &ivalue);
					if (isensor >= 0 && isensor < platform->num_sensors)
						platform->sensors[isensor].type = ivalue;
					}
				else if (strncmp(buffer, "SENSOR_MODEL", 12) == 0)
					{
					sscanf(buffer, "%s %d %s", dummy, &isensor, svalue);
					if (isensor >= 0 && isensor < platform->num_sensors)
						strcpy(platform->sensors[isensor].model, svalue);
					}
				else if (strncmp(buffer, "SENSOR_MANUFACTURER", 19) == 0)
					{
					sscanf(buffer, "%s %d %s", dummy, &isensor, svalue);
					if (isensor >= 0 && isensor < platform->num_sensors)
						strcpy(platform->sensors[isensor].manufacturer, svalue);
					}
				else if (strncmp(buffer, "SENSOR_SERIALNUMBER", 19) == 0)
					{
					sscanf(buffer, "%s %d %s", dummy, &isensor, svalue);
					if (isensor >= 0 && isensor < platform->num_sensors)
						strcpy(platform->sensors[isensor].serialnumber, svalue);
					}
				else if (strncmp(buffer, "SENSOR_CAPABILITY", 17) == 0)
					{
					sscanf(buffer, "%s %d %d", dummy, &isensor, &ivalue);
					if (isensor >= 0 && isensor < platform->num_sensors)
						platform->sensors[isensor].capability = ivalue;
					}
				else if (strncmp(buffer, "SENSOR_SPECIAL_CAPABILITY", 25) == 0)
					{
					sscanf(buffer, "%s %d %d", dummy, &isensor, &ivalue);
					if (isensor >= 0 && isensor < platform->num_sensors)
						platform->sensors[isensor].special_capability = ivalue;
					}
				else if (strncmp(buffer, "SENSOR_NUM_OFFSETS", 17) == 0)
					{
					sscanf(buffer, "%s %d %d", dummy, &isensor, &ivalue);
					if (isensor >= 0 && isensor < platform->num_sensors)
						platform->sensors[isensor].num_offsets = ivalue;
					if (platform->sensors[isensor].num_offsets > platform->sensors[isensor].num_offsets_alloc)
						{
						size = platform->sensors[isensor].num_offsets * sizeof(struct mb_sensor_offset_struct);
						status = mb_mallocd(verbose,__FILE__, __LINE__,
									size, (void **) &platform->sensors[isensor].offsets, error);
						if (status == MB_SUCCESS)
							{
							memset(platform->sensors[isensor].offsets, 0, size);
							platform->sensors[isensor].num_offsets_alloc = platform->sensors[isensor].num_offsets;
							}
						else
							{
							mb_error(verbose,*error,&message);
							fprintf(stderr,"\nMBIO Error allocating sensor offsets structures:\n%s\n",message);
							fprintf(stderr,"\nProgram terminated in function <%s>\n", function_name);
							exit(*error);
							}
						}
					}
	
				else if (strncmp(buffer, "OFFSET_TIME_LATENCY_STATIC", 26) == 0)
					{
					sscanf(buffer, "%s %d %d %lf", dummy, &isensor, &ioffset, &dvalue);
					if (isensor >= 0 && isensor < platform->num_sensors
						&& ioffset >= 0 && ioffset < platform->sensors[isensor].num_offsets)
						{
						platform->sensors[isensor].offsets[ioffset].time_latency_static = dvalue;
						platform->sensors[isensor].offsets[ioffset].time_latency_mode = MB_SENSOR_TIME_LATENCY_STATIC;
						}
					}
				else if (strncmp(buffer, "OFFSET_TIME_LATENCY_MODEL", 26) == 0)
					{
					sscanf(buffer, "%s %d %d %d", dummy, &isensor, &ioffset, &ivalue);
					if (isensor >= 0 && isensor < platform->num_sensors
						&& ioffset >= 0 && ioffset < platform->sensors[isensor].num_offsets)
						{
						platform->sensors[isensor].offsets[ioffset].num_time_latency = ivalue;
						platform->sensors[isensor].offsets[ioffset].time_latency_mode = MB_SENSOR_TIME_LATENCY_MODEL;
						if (platform->sensors[isensor].offsets[ioffset].num_time_latency
							< platform->sensors[isensor].offsets[ioffset].num_time_latency_alloc)
							{
							size = platform->sensors[isensor].offsets[ioffset].num_time_latency * sizeof(double);
							status = mb_mallocd(verbose,__FILE__, __LINE__, size,
										(void **) &platform->sensors[isensor].offsets[ioffset].time_latency_time_d,
										error);
							status = mb_mallocd(verbose,__FILE__, __LINE__, size,
										(void **) &platform->sensors[isensor].offsets[ioffset].time_latency_value,
										error);
							if (status == MB_SUCCESS)
								{
								memset(platform->sensors[isensor].offsets[ioffset].time_latency_time_d, 0, size);
								memset(platform->sensors[isensor].offsets[ioffset].time_latency_value, 0, size);
								platform->sensors[isensor].offsets[ioffset].num_time_latency_alloc
									= platform->sensors[isensor].offsets[ioffset].num_time_latency;
								}
							else
								{
								mb_error(verbose,*error,&message);
								fprintf(stderr,"\nMBIO Error allocating sensor offsets structures:\n%s\n",message);
								fprintf(stderr,"\nProgram terminated in function <%s>\n", function_name);
								exit(*error);
								}
							
							}
									
						/* read the time latency model */
						for (i=0;i<platform->sensors[isensor].offsets[ioffset].num_time_latency;i++)
							{
							if ((result = fgets(buffer,MB_PATH_MAXLINE,fp)) == buffer)
								{
								sscanf(buffer, "%lf %lf",
									&platform->sensors[isensor].offsets[ioffset].time_latency_time_d[i],
									&platform->sensors[isensor].offsets[ioffset].time_latency_value[i]);
								}
							else
								{
								status = MB_FAILURE;
								*error = MB_ERROR_EOF;
								mb_error(verbose,*error,&message);
								fprintf(stderr,"\nMBIO Error parsing sensor offset time latency model:\n%s\n",message);
								fprintf(stderr,"\nProgram terminated in function <%s>\n", function_name);
								exit(*error);
								}
							}
						}
					}
	
				else if (strncmp(buffer, "OFFSET_POSITION", 15) == 0)
					{
					sscanf(buffer, "%s %d %d %lf %lf %lf", dummy, &isensor, &ioffset, &dvalue, &dvalue2, &dvalue3);
					platform->sensors[isensor].offsets[ioffset].position_offset_x = dvalue;
					platform->sensors[isensor].offsets[ioffset].position_offset_y = dvalue2;
					platform->sensors[isensor].offsets[ioffset].position_offset_z = dvalue3;
					platform->sensors[isensor].offsets[ioffset].position_offset_mode = MB_SENSOR_POSITION_OFFSET_STATIC;
					}
				else if (strncmp(buffer, "OFFSET_ATTITUDE", 15) == 0)
					{
					sscanf(buffer, "%s %d %d %lf %lf %lf", dummy, &isensor, &ioffset, &dvalue, &dvalue2, &dvalue3);
					platform->sensors[isensor].offsets[ioffset].attitude_offset_azimuth = dvalue;
					platform->sensors[isensor].offsets[ioffset].attitude_offset_roll = dvalue2;
					platform->sensors[isensor].offsets[ioffset].attitude_offset_pitch = dvalue3;
					platform->sensors[isensor].offsets[ioffset].attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_STATIC;
					}
				}
			}
			
		/* close the file */
		fclose(fp);
		}

	/* failure opening */
	else
		{
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		}
	
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       platform_ptr:		%p\n", platform_ptr);
		}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR)
		{
		fprintf(stderr,"dbg2       *platform_ptr:		     %p\n", *platform_ptr);
		fprintf(stderr,"dbg2       platform->type:		     %d\n", platform->type);
		fprintf(stderr,"dbg2       platform->name:		     %s\n", platform->name);
		fprintf(stderr,"dbg2       platform->organization:	     %s\n", platform->organization);
		fprintf(stderr,"dbg2       platform->source_swathbathymetry: %d\n", platform->source_swathbathymetry);
		fprintf(stderr,"dbg2       platform->source_position:	     %d\n", platform->source_position);
		fprintf(stderr,"dbg2       platform->source_depth:	     %d\n", platform->source_depth);
		fprintf(stderr,"dbg2       platform->source_heave:	     %d\n", platform->source_heave);
		fprintf(stderr,"dbg2       platform->source_attitude:	 %d\n", platform->source_attitude);
		fprintf(stderr,"dbg2       platform->num_sensors:	     %d\n", platform->num_sensors);
		for (i=0;i<platform->num_sensors;i++)
			{
			fprintf(stderr,"dbg2       platform->sensors[%2d].type:                 %d\n", i, platform->sensors[i].type);
			fprintf(stderr,"dbg2       platform->sensors[%2d].model:                %s\n", i, platform->sensors[i].model);
			fprintf(stderr,"dbg2       platform->sensors[%2d].manufacturer:         %s\n", i, platform->sensors[i].manufacturer);
			fprintf(stderr,"dbg2       platform->sensors[%2d].serialnumber:         %s\n", i, platform->sensors[i].serialnumber);
			fprintf(stderr,"dbg2       platform->sensors[%2d].capability:           %d\n", i, platform->sensors[i].capability);
			fprintf(stderr,"dbg2       platform->sensors[%2d].special_capability:   %d\n", i, platform->sensors[i].special_capability);
			fprintf(stderr,"dbg2       platform->sensors[%2d].num_offsets:          %d\n", i, platform->sensors[i].num_offsets);
			for (j=0;j<platform->sensors[i].num_offsets;j++)
				{
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].time_latency_mode:	%d\n", i, j, platform->sensors[i].offsets[j].time_latency_mode);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].time_latency_static:	%f\n", i, j, platform->sensors[i].offsets[j].time_latency_static);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].num_time_latency:		%d\n", i, j, platform->sensors[i].offsets[j].num_time_latency);
				for (k=0;k<platform->sensors[i].offsets[j].num_time_latency;k++)
					{
					fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].time_latency[%2d]:		%16.6f %8.6f\n", i, j, k,
						platform->sensors[i].offsets[j].time_latency_time_d[k],platform->sensors[i].offsets[j].time_latency_value[k]);
					}
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_mode:	%d\n", i, j, platform->sensors[i].offsets[j].position_offset_mode);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_x:	%f\n", i, j, platform->sensors[i].offsets[j].position_offset_x);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_y:	%f\n", i, j, platform->sensors[i].offsets[j].position_offset_y);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_z:	%f\n", i, j, platform->sensors[i].offsets[j].position_offset_z);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_mode:	%d\n", i, j, platform->sensors[i].offsets[j].attitude_offset_mode);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_azimuth:	%f\n", i, j, platform->sensors[i].offsets[j].attitude_offset_azimuth);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_roll:	%f\n", i, j, platform->sensors[i].offsets[j].attitude_offset_roll);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_pitch:	%f\n", i, j, platform->sensors[i].offsets[j].attitude_offset_pitch);
				}
			}
		}
	if (verbose >= 2)
		{
		fprintf(stderr,"dbg2       error:			%d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:			%d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_platform_write(int verbose, char *platform_file, void **platform_ptr, int *error)
{
	char	*function_name = "mb_platform_write";
	int	status = MB_SUCCESS;
	struct mb_platform_struct *platform;
	time_t	right_now;
	char	date[32], user[MB_PATH_MAXLINE], *user_ptr, host[MB_PATH_MAXLINE];
	FILE 	*fp;
	int	isensor, ioffset;
	int	i, j, k;
		
	/* get platform structure */
	platform = (struct mb_platform_struct *) *platform_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n", verbose);
		fprintf(stderr,"dbg2       platform_ptr:		%p\n", platform_ptr);
		fprintf(stderr,"dbg2       *platform_ptr:		%p\n", *platform_ptr);
		}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR)
		{
		fprintf(stderr,"dbg2       *platform_ptr:		     %p\n", *platform_ptr);
		fprintf(stderr,"dbg2       platform->type:		     %d\n", platform->type);
		fprintf(stderr,"dbg2       platform->name:		     %s\n", platform->name);
		fprintf(stderr,"dbg2       platform->organization:	     %s\n", platform->organization);
		fprintf(stderr,"dbg2       platform->source_swathbathymetry: %d\n", platform->source_swathbathymetry);
		fprintf(stderr,"dbg2       platform->source_position:	     %d\n", platform->source_position);
		fprintf(stderr,"dbg2       platform->source_depth:	     %d\n", platform->source_depth);
		fprintf(stderr,"dbg2       platform->source_heave:	     %d\n", platform->source_heave);
		fprintf(stderr,"dbg2       platform->source_attitude:	     %d\n", platform->source_attitude);
		fprintf(stderr,"dbg2       platform->num_sensors:	     %d\n", platform->num_sensors);
		for (i=0;i<platform->num_sensors;i++)
			{
			fprintf(stderr,"dbg2       platform->sensors[%2d].type:                 %d\n", i, platform->sensors[i].type);
			fprintf(stderr,"dbg2       platform->sensors[%2d].model:                %s\n", i, platform->sensors[i].model);
			fprintf(stderr,"dbg2       platform->sensors[%2d].manufacturer:         %s\n", i, platform->sensors[i].manufacturer);
			fprintf(stderr,"dbg2       platform->sensors[%2d].serialnumber:         %s\n", i, platform->sensors[i].serialnumber);
			fprintf(stderr,"dbg2       platform->sensors[%2d].capability:           %d\n", i, platform->sensors[i].capability);
			fprintf(stderr,"dbg2       platform->sensors[%2d].special_capability:   %d\n", i, platform->sensors[i].special_capability);
			fprintf(stderr,"dbg2       platform->sensors[%2d].num_offsets:          %d\n", i, platform->sensors[i].num_offsets);
			for (j=0;j<platform->sensors[i].num_offsets;j++)
				{
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].time_latency_mode:	%d\n", i, j, platform->sensors[i].offsets[j].time_latency_mode);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].time_latency_static:	%f\n", i, j, platform->sensors[i].offsets[j].time_latency_static);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].num_time_latency:		%d\n", i, j, platform->sensors[i].offsets[j].num_time_latency);
				for (k=0;k<platform->sensors[i].offsets[j].num_time_latency;k++)
					{
					fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].time_latency[%2d]:		%16.6f %8.6f\n", i, j, k,
						platform->sensors[i].offsets[j].time_latency_time_d[k],platform->sensors[i].offsets[j].time_latency_value[k]);
					}
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_mode:	%d\n", i, j, platform->sensors[i].offsets[j].position_offset_mode);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_x:	%f\n", i, j, platform->sensors[i].offsets[j].position_offset_x);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_y:	%f\n", i, j, platform->sensors[i].offsets[j].position_offset_y);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_z:	%f\n", i, j, platform->sensors[i].offsets[j].position_offset_z);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_mode:	%d\n", i, j, platform->sensors[i].offsets[j].attitude_offset_mode);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_azimuth:	%f\n", i, j, platform->sensors[i].offsets[j].attitude_offset_azimuth);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_roll:	%f\n", i, j, platform->sensors[i].offsets[j].attitude_offset_roll);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_pitch:	%f\n", i, j, platform->sensors[i].offsets[j].attitude_offset_pitch);
				}
			}
		}
		
	/* open and read platform file */
	if ((fp = fopen(platform_file, "w")) != NULL)
		{
		right_now = time((time_t *)0);
		strcpy(date,ctime(&right_now));
				date[strlen(date)-1] = '\0';
		if ((user_ptr = getenv("USER")) == NULL)
			user_ptr = getenv("LOGNAME");
		if (user_ptr != NULL)
			strcpy(user,user_ptr);
		else
			strcpy(user, "unknown");
		gethostname(host,MB_PATH_MAXLINE);
		fprintf(fp, "## MB-System Platform Definition File\n");
		fprintf(fp,"MB-SYSTEM_VERSION\t%s\n",MB_VERSION);
		fprintf(fp,"SOURCE_VERSION\t%s\n",svn_id);
		fprintf(fp,"FILE_VERSION\t1.00\n");
		fprintf(fp,"ORIGIN\tGenerated by user <%s> on cpu <%s> at <%s>\n", user,host,date);
		
		
		fprintf(stderr, "PLATFORM_TYPE\t%d\n", platform->type);
		fprintf(stderr, "PLATFORM_NAME\t%s\n", platform->name);
		fprintf(stderr, "PLATFORM_ORGANIZATION\t%s\n", platform->organization);
		fprintf(stderr, "SOURCE_SWATHBATHYMETRY\t%d\n", platform->source_swathbathymetry);
		fprintf(stderr, "SOURCE_POSITION\t%d\n", platform->source_position);
		fprintf(stderr, "SOURCE_DEPTH\t%d\n", platform->source_depth);
		fprintf(stderr, "SOURCE_HEAVE\t%d\n", platform->source_heave);
		fprintf(stderr, "SOURCE_ATTITUDE\t%d\n", platform->source_attitude);
		fprintf(stderr, "PLATFORM_NUM_SENSORS\t%d\n", platform->num_sensors);
		for (isensor=0;isensor<platform->num_sensors;isensor++)
			{
			fprintf(stderr, "SENSOR_TYPE\t%d\t%d\n", isensor, platform->sensors[i].type);
			fprintf(stderr, "SENSOR_MODEL\t%d\t%s\n", isensor, platform->sensors[i].model);
			fprintf(stderr, "SENSOR_MANUFACTURER\t%d\t%s\n", isensor, platform->sensors[i].manufacturer);
			fprintf(stderr, "SENSOR_SERIALNUMBER\t%d\t%s\n", isensor, platform->sensors[i].serialnumber);
			fprintf(stderr, "SENSOR_CAPABILITY\t%d\t%d\n", isensor, platform->sensors[i].capability);
			fprintf(stderr, "SENSOR_SPECIAL_CAPABILITY\t%d\t%d\n", isensor, platform->sensors[i].special_capability);
			fprintf(stderr, "SENSOR_NUM_OFFSETS\t%d\t%d\n", isensor, platform->sensors[i].num_offsets);
			for (ioffset=0;ioffset<platform->sensors[isensor].num_offsets;ioffset++)
				{
				if (platform->sensors[isensor].offsets[ioffset].time_latency_mode
					== MB_SENSOR_TIME_LATENCY_STATIC)
					{
					fprintf(stderr, "OFFSET_TIME_LATENCY_STATIC\t%d\t%d %f\n",
							isensor, ioffset, platform->sensors[isensor].offsets[ioffset].time_latency_static);
					}
				else if (platform->sensors[isensor].offsets[ioffset].time_latency_mode
					== MB_SENSOR_TIME_LATENCY_MODEL)
					{
					fprintf(stderr, "OFFSET_TIME_LATENCY_MODEL\t%d\t%d\t%d\n",
							isensor, ioffset, platform->sensors[isensor].offsets[ioffset].num_time_latency);
					for (i=0;i<platform->sensors[isensor].offsets[ioffset].num_time_latency;i++)
						{
						fprintf(stdout, "%f\t%f",
								platform->sensors[isensor].offsets[ioffset].time_latency_time_d[i],
								platform->sensors[isensor].offsets[ioffset].time_latency_value[i]);
						
						}
					}
				if (platform->sensors[isensor].offsets[ioffset].position_offset_mode == MB_SENSOR_POSITION_OFFSET_STATIC)
					{
					fprintf(stderr, "OFFSET_POSITION\t%d\t%d\t%lf\t%lf\t%lf\n", isensor, ioffset, 
						platform->sensors[isensor].offsets[ioffset].position_offset_x,
						platform->sensors[isensor].offsets[ioffset].position_offset_y,
						platform->sensors[isensor].offsets[ioffset].position_offset_z);
					}
				if (platform->sensors[isensor].offsets[ioffset].attitude_offset_mode == MB_SENSOR_ATTITUDE_OFFSET_STATIC)
					{
					fprintf(stderr, "OFFSET_ATTITUDE\t%d\t%d\t%lf\t%lf\t%lf\n", isensor, ioffset, 
						platform->sensors[isensor].offsets[ioffset].attitude_offset_azimuth,
						platform->sensors[isensor].offsets[ioffset].attitude_offset_roll,
						platform->sensors[isensor].offsets[ioffset].attitude_offset_pitch);
					}
				}
			}
			
		/* close the file */
		fclose(fp);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:			%d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:			%d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_platform_lever(int verbose, void **platform_ptr,
					  int targetsensor, int targetsensoroffset,
					  double heading, double roll, double pitch,
					  double *lever_x, double *lever_y, double *lever_z,
					  int *error)
{
	char	*function_name = "mb_platform_lever";
	int	status = MB_SUCCESS;
	struct mb_platform_struct *platform;
	struct mb_sensor_struct *sensor_target = NULL;
	struct mb_sensor_struct *sensor_swathbathymetry = NULL;
	struct mb_sensor_struct *sensor_attitude = NULL;
	struct mb_sensor_struct *sensor_position = NULL;
	struct mb_sensor_struct *sensor_depth = NULL;
	double	xx, yy, zz;
	double	proll, ppitch, pheading;
	double	croll, sroll;
	double	cpitch, spitch;
	double	cheading, sheading;
	int	i, j, k;
		
	/* reset error */
	*error = MB_ERROR_NO_ERROR;

	/* get platform structure */
	platform = (struct mb_platform_struct *) *platform_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:				%d\n", verbose);
		fprintf(stderr,"dbg2       platform_ptr:		%p\n", platform_ptr);
		fprintf(stderr,"dbg2       *platform_ptr:		%p\n", *platform_ptr);
		fprintf(stderr,"dbg2       targetsensor:		%d\n", targetsensor);
		fprintf(stderr,"dbg2       targetsensoroffset:	%d\n", targetsensoroffset);
		fprintf(stderr,"dbg2       heading:				%f\n", heading);
		fprintf(stderr,"dbg2       roll:				%f\n", roll);
		fprintf(stderr,"dbg2       pitch:				%f\n", pitch);
		}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR)
		{
		fprintf(stderr,"dbg2       *platform_ptr:		     %p\n", *platform_ptr);
		fprintf(stderr,"dbg2       platform->type:		     %d\n", platform->type);
		fprintf(stderr,"dbg2       platform->name:		     %s\n", platform->name);
		fprintf(stderr,"dbg2       platform->organization:	     %s\n", platform->organization);
		fprintf(stderr,"dbg2       platform->source_swathbathymetry: %d\n", platform->source_swathbathymetry);
		fprintf(stderr,"dbg2       platform->source_position:	     %d\n", platform->source_position);
		fprintf(stderr,"dbg2       platform->source_depth:	     %d\n", platform->source_depth);
		fprintf(stderr,"dbg2       platform->source_heave:	     %d\n", platform->source_heave);
		fprintf(stderr,"dbg2       platform->source_attitude:	     %d\n", platform->source_attitude);
		fprintf(stderr,"dbg2       platform->num_sensors:	     %d\n", platform->num_sensors);
		for (i=0;i<platform->num_sensors;i++)
			{
			fprintf(stderr,"dbg2       platform->sensors[%2d].type:                 %d\n", i, platform->sensors[i].type);
			fprintf(stderr,"dbg2       platform->sensors[%2d].model:                %s\n", i, platform->sensors[i].model);
			fprintf(stderr,"dbg2       platform->sensors[%2d].manufacturer:         %s\n", i, platform->sensors[i].manufacturer);
			fprintf(stderr,"dbg2       platform->sensors[%2d].serialnumber:         %s\n", i, platform->sensors[i].serialnumber);
			fprintf(stderr,"dbg2       platform->sensors[%2d].capability:           %d\n", i, platform->sensors[i].capability);
			fprintf(stderr,"dbg2       platform->sensors[%2d].special_capability:   %d\n", i, platform->sensors[i].special_capability);
			fprintf(stderr,"dbg2       platform->sensors[%2d].num_offsets:          %d\n", i, platform->sensors[i].num_offsets);
			for (j=0;j<platform->sensors[i].num_offsets;j++)
				{
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].time_latency_mode:	%d\n", i, j, platform->sensors[i].offsets[j].time_latency_mode);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].time_latency_static:	%f\n", i, j, platform->sensors[i].offsets[j].time_latency_static);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].num_time_latency:		%d\n", i, j, platform->sensors[i].offsets[j].num_time_latency);
				for (k=0;k<platform->sensors[i].offsets[j].num_time_latency;k++)
					{
					fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].time_latency[%2d]:		%16.6f %8.6f\n", i, j, k,
						platform->sensors[i].offsets[j].time_latency_time_d[k],platform->sensors[i].offsets[j].time_latency_value[k]);
					}
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_mode:	%d\n", i, j, platform->sensors[i].offsets[j].position_offset_mode);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_x:	%f\n", i, j, platform->sensors[i].offsets[j].position_offset_x);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_y:	%f\n", i, j, platform->sensors[i].offsets[j].position_offset_y);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_z:	%f\n", i, j, platform->sensors[i].offsets[j].position_offset_z);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_mode:	%d\n", i, j, platform->sensors[i].offsets[j].attitude_offset_mode);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_azimuth:	%f\n", i, j, platform->sensors[i].offsets[j].attitude_offset_azimuth);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_roll:	%f\n", i, j, platform->sensors[i].offsets[j].attitude_offset_roll);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_pitch:	%f\n", i, j, platform->sensors[i].offsets[j].attitude_offset_pitch);
				}
			}
		}
		
	/* check that all sensor id's are sensible for this platform */
	if (targetsensor < 0 || targetsensor >= platform->num_sensors
			|| platform->source_swathbathymetry < 0 || platform->source_swathbathymetry >= platform->num_sensors
			|| platform->source_attitude < 0 || platform->source_attitude >= platform->num_sensors
			|| platform->source_position < 0 || platform->source_position >= platform->num_sensors
			|| platform->source_depth < 0 || platform->source_depth >= platform->num_sensors)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_PARAMETER;
		}
		
	/* else proceed */
	else
		{
		/* get sensor structures */
		sensor_target   = &platform->sensors[targetsensor];
		sensor_swathbathymetry = &platform->sensors[platform->source_swathbathymetry];
		sensor_attitude = &platform->sensors[platform->source_attitude];
		sensor_position = &platform->sensors[platform->source_position];
		sensor_depth    = &platform->sensors[platform->source_depth];
		
		/* start with zero lever */
		*lever_x = 0.0;
		*lever_y = 0.0;
		*lever_z = 0.0;
		
		/* get platform attitude*/
		if (sensor_attitude->offsets[0].attitude_offset_mode == MB_SENSOR_ATTITUDE_OFFSET_STATIC)
			{			
			mb_platform_math_attitude_platform (verbose,
												roll, pitch, heading,
				                            	sensor_attitude->offsets[0].attitude_offset_roll,
				                            	sensor_attitude->offsets[0].attitude_offset_pitch,
				                            	sensor_attitude->offsets[0].attitude_offset_azimuth,
				                            	&proll, &ppitch, &pheading,
												error);
			}
		else
			{
			proll    = roll;
			ppitch   = pitch;
			pheading = heading;			
			}

		/* Convenient calculations for later coordinate operations */
		croll    = cos(DTR * proll);
		sroll    = sin(DTR * proll);
		cpitch   = cos(DTR * ppitch);
		spitch   = sin(DTR * ppitch);
		cheading = cos(DTR * pheading);
		sheading = sin(DTR * pheading);

		/* apply change in z due to offset between the depth sensor and the target sensor
		using roll add pitch values corrected for the attitude sensor offset */
		xx = 0.0;
		yy = 0.0;
		zz = 0.0;
		if (sensor_target->offsets[targetsensoroffset].position_offset_mode == MB_SENSOR_POSITION_OFFSET_STATIC)
			{
			xx += sensor_target->offsets[targetsensoroffset].position_offset_x;
			yy += sensor_target->offsets[targetsensoroffset].position_offset_y;
			zz += sensor_target->offsets[targetsensoroffset].position_offset_z;
			}
		if (sensor_depth->offsets[0].position_offset_mode == MB_SENSOR_POSITION_OFFSET_STATIC)
			{
			xx -= sensor_depth->offsets[0].position_offset_x;
			yy -= sensor_depth->offsets[0].position_offset_y;
			zz -= sensor_depth->offsets[0].position_offset_z;
			}

		*lever_z = cpitch * sroll * xx
				 - spitch         * yy
				 + cpitch * croll * zz;
				
		/* apply change in x and y due to offset between the position sensor and the target sensor
		using roll, pitch and heading corrected for the attitude sensor offset and the target sensor */
		xx = 0.0;
		yy = 0.0;
		zz = 0.0;
		if (sensor_target->offsets[targetsensoroffset].position_offset_mode == MB_SENSOR_POSITION_OFFSET_STATIC)
			{
			xx += sensor_target->offsets[targetsensoroffset].position_offset_x;
			yy += sensor_target->offsets[targetsensoroffset].position_offset_y;
			zz += sensor_target->offsets[targetsensoroffset].position_offset_z;
			}
		if (sensor_position->offsets[0].position_offset_mode == MB_SENSOR_POSITION_OFFSET_STATIC)
			{
			xx -= sensor_depth->offsets[0].position_offset_x;
			yy -= sensor_depth->offsets[0].position_offset_y;
			zz -= sensor_depth->offsets[0].position_offset_z;
			}

		*lever_x = (cheading*croll + sheading*spitch*sroll) * xx + 
					cpitch*sheading                         * yy +
				   (croll*sheading*spitch - cheading*sroll) * zz;


		*lever_y = (cheading*spitch*sroll - croll*sheading) * xx +
					cheading*cpitch                         * yy +
				   (sheading*sroll + cheading*croll*spitch) * zz;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       lever_x:		        %f\n", *lever_x);
		fprintf(stderr,"dbg2       lever_y:		        %f\n", *lever_y);
		fprintf(stderr,"dbg2       lever_z:		        %f\n", *lever_z);
		fprintf(stderr,"dbg2       error:			%d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:			%d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_platform_position(int verbose, void **platform_ptr,
						int targetsensor, int targetsensoroffset,
						double navlon, double navlat, double sensordepth,
						double heading, double roll, double pitch,
						double *targetlon, double *targetlat, double *targetz,
						int *error)
{
	char	*function_name = "mb_platform_position";
	int	status = MB_SUCCESS;
	struct mb_platform_struct *platform;
	double	mtodeglon, mtodeglat;
	double	lever_x, lever_y, lever_z;
	int	i, j, k;
		
	/* get platform structure */
	platform = (struct mb_platform_struct *) *platform_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n", verbose);
		fprintf(stderr,"dbg2       platform_ptr:		%p\n", platform_ptr);
		fprintf(stderr,"dbg2       *platform_ptr:		%p\n", *platform_ptr);
		fprintf(stderr,"dbg2       targetsensor:		%d\n", targetsensor);
		fprintf(stderr,"dbg2       targetsensoroffset:		%d\n", targetsensoroffset);
		fprintf(stderr,"dbg2       navlon:		        %f\n", navlon);
		fprintf(stderr,"dbg2       navlat:		        %f\n", navlat);
		fprintf(stderr,"dbg2       sensordepth:		        %f\n", sensordepth);
		fprintf(stderr,"dbg2       heading:		        %f\n", heading);
		fprintf(stderr,"dbg2       roll:		        %f\n", roll);
		fprintf(stderr,"dbg2       pitch:		        %f\n", pitch);
		}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR)
		{
		fprintf(stderr,"dbg2       *platform_ptr:		     %p\n", *platform_ptr);
		fprintf(stderr,"dbg2       platform->type:		     %d\n", platform->type);
		fprintf(stderr,"dbg2       platform->name:		     %s\n", platform->name);
		fprintf(stderr,"dbg2       platform->organization:	     %s\n", platform->organization);
		fprintf(stderr,"dbg2       platform->source_swathbathymetry: %d\n", platform->source_swathbathymetry);
		fprintf(stderr,"dbg2       platform->source_position:	     %d\n", platform->source_position);
		fprintf(stderr,"dbg2       platform->source_depth:	     %d\n", platform->source_depth);
		fprintf(stderr,"dbg2       platform->source_heave:	     %d\n", platform->source_heave);
		fprintf(stderr,"dbg2       platform->source_attitude:	     %d\n", platform->source_attitude);
		fprintf(stderr,"dbg2       platform->num_sensors:	     %d\n", platform->num_sensors);
		for (i=0;i<platform->num_sensors;i++)
			{
			fprintf(stderr,"dbg2       platform->sensors[%2d].type:                 %d\n", i, platform->sensors[i].type);
			fprintf(stderr,"dbg2       platform->sensors[%2d].model:                %s\n", i, platform->sensors[i].model);
			fprintf(stderr,"dbg2       platform->sensors[%2d].manufacturer:         %s\n", i, platform->sensors[i].manufacturer);
			fprintf(stderr,"dbg2       platform->sensors[%2d].serialnumber:         %s\n", i, platform->sensors[i].serialnumber);
			fprintf(stderr,"dbg2       platform->sensors[%2d].capability:           %d\n", i, platform->sensors[i].capability);
			fprintf(stderr,"dbg2       platform->sensors[%2d].special_capability:   %d\n", i, platform->sensors[i].special_capability);
			fprintf(stderr,"dbg2       platform->sensors[%2d].num_offsets:          %d\n", i, platform->sensors[i].num_offsets);
			for (j=0;j<platform->sensors[i].num_offsets;j++)
				{
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].time_latency_mode:	%d\n", i, j, platform->sensors[i].offsets[j].time_latency_mode);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].time_latency_static:	%f\n", i, j, platform->sensors[i].offsets[j].time_latency_static);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].num_time_latency:		%d\n", i, j, platform->sensors[i].offsets[j].num_time_latency);
				for (k=0;k<platform->sensors[i].offsets[j].num_time_latency;k++)
					{
					fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].time_latency[%2d]:		%16.6f %8.6f\n", i, j, k,
						platform->sensors[i].offsets[j].time_latency_time_d[k],platform->sensors[i].offsets[j].time_latency_value[k]);
					}
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_mode:	%d\n", i, j, platform->sensors[i].offsets[j].position_offset_mode);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_x:	%f\n", i, j, platform->sensors[i].offsets[j].position_offset_x);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_y:	%f\n", i, j, platform->sensors[i].offsets[j].position_offset_y);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_z:	%f\n", i, j, platform->sensors[i].offsets[j].position_offset_z);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_mode:	%d\n", i, j, platform->sensors[i].offsets[j].attitude_offset_mode);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_azimuth:	%f\n", i, j, platform->sensors[i].offsets[j].attitude_offset_azimuth);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_roll:	%f\n", i, j, platform->sensors[i].offsets[j].attitude_offset_roll);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_pitch:	%f\n", i, j, platform->sensors[i].offsets[j].attitude_offset_pitch);
				}
			}
		}

	// /* call mb_platform_orientation to get platform orientation */
	// status = mb_platform_orientation(verbose, platform_ptr,
	// 								 // targetsensor, targetsensoroffset,
	// 								 heading, roll, pitch,
	// 								 &pheading, &proll, &ppitch,
	// 								 error);


	/* call mb_platform lever to get relative lever offsets */
	status = mb_platform_lever(verbose, platform_ptr,
								targetsensor, targetsensoroffset,
								heading, roll, pitch,
								&lever_x, &lever_y, &lever_z,
								error);

	/* get local translation between lon lat degrees and meters */
	mb_coor_scale(verbose,navlat,&mtodeglon,&mtodeglat);
	
	/* calculate absolute position and depth for target sensor */
	*targetlon = navlon + lever_x * mtodeglon;
	*targetlat = navlat + lever_y * mtodeglat;
	*targetz = sensordepth + lever_z;
	

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       targetlon:		%f\n", *targetlon);
		fprintf(stderr,"dbg2       targetlat:		%f\n", *targetlat);
		fprintf(stderr,"dbg2       targetz:		    %f\n", *targetz);
		fprintf(stderr,"dbg2       error:			%d\n", *error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:			%d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_platform_orientation_offset (int verbose, void **platform_ptr,
							int targetsensor, int targetsensoroffset,
							double heading, double roll, double pitch,
							double *target_hdg_offset, 
							double *target_roll_offset, 
							double *target_pitch_offset,
							int *error)
{
	char	*function_name = "mb_platform_orientation_offset";
	int	status = MB_SUCCESS;
	struct mb_platform_struct *platform;
	struct mb_sensor_struct *sensor_target = NULL;
	struct mb_sensor_struct *sensor_attitude = NULL;
	int	i, j, k;
		
	/* get platform structure */
	platform = (struct mb_platform_struct *) *platform_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n", verbose);
		fprintf(stderr,"dbg2       platform_ptr:		%p\n", platform_ptr);
		fprintf(stderr,"dbg2       *platform_ptr:		%p\n", *platform_ptr);
		fprintf(stderr,"dbg2       targetsensor:		%d\n", targetsensor);
		fprintf(stderr,"dbg2       targetsensoroffset:	%d\n", targetsensoroffset);
		fprintf(stderr,"dbg2       heading:		        %f\n", heading);
		fprintf(stderr,"dbg2       roll:		        %f\n", roll);
		fprintf(stderr,"dbg2       pitch:		        %f\n", pitch);
		}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR)
		{
		fprintf(stderr,"dbg2       *platform_ptr:		     %p\n", *platform_ptr);
		fprintf(stderr,"dbg2       platform->type:		     %d\n", platform->type);
		fprintf(stderr,"dbg2       platform->name:		     %s\n", platform->name);
		fprintf(stderr,"dbg2       platform->organization:	     %s\n", platform->organization);
		fprintf(stderr,"dbg2       platform->source_swathbathymetry: %d\n", platform->source_swathbathymetry);
		fprintf(stderr,"dbg2       platform->source_position:	     %d\n", platform->source_position);
		fprintf(stderr,"dbg2       platform->source_depth:	     %d\n", platform->source_depth);
		fprintf(stderr,"dbg2       platform->source_heave:	     %d\n", platform->source_heave);
		fprintf(stderr,"dbg2       platform->source_attitude:	     %d\n", platform->source_attitude);
		fprintf(stderr,"dbg2       platform->num_sensors:	     %d\n", platform->num_sensors);
		for (i=0;i<platform->num_sensors;i++)
			{
			fprintf(stderr,"dbg2       platform->sensors[%2d].type:                 %d\n", i, platform->sensors[i].type);
			fprintf(stderr,"dbg2       platform->sensors[%2d].model:                %s\n", i, platform->sensors[i].model);
			fprintf(stderr,"dbg2       platform->sensors[%2d].manufacturer:         %s\n", i, platform->sensors[i].manufacturer);
			fprintf(stderr,"dbg2       platform->sensors[%2d].serialnumber:         %s\n", i, platform->sensors[i].serialnumber);
			fprintf(stderr,"dbg2       platform->sensors[%2d].capability:           %d\n", i, platform->sensors[i].capability);
			fprintf(stderr,"dbg2       platform->sensors[%2d].special_capability:   %d\n", i, platform->sensors[i].special_capability);
			fprintf(stderr,"dbg2       platform->sensors[%2d].num_offsets:          %d\n", i, platform->sensors[i].num_offsets);
			for (j=0;j<platform->sensors[i].num_offsets;j++)
				{
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].time_latency_mode:	%d\n", i, j, platform->sensors[i].offsets[j].time_latency_mode);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].time_latency_static:	%f\n", i, j, platform->sensors[i].offsets[j].time_latency_static);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].num_time_latency:		%d\n", i, j, platform->sensors[i].offsets[j].num_time_latency);
				for (k=0;k<platform->sensors[i].offsets[j].num_time_latency;k++)
					{
					fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].time_latency[%2d]:		%16.6f %8.6f\n", i, j, k,
						platform->sensors[i].offsets[j].time_latency_time_d[k],platform->sensors[i].offsets[j].time_latency_value[k]);
					}
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_mode:	%d\n", i, j, platform->sensors[i].offsets[j].position_offset_mode);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_x:	%f\n", i, j, platform->sensors[i].offsets[j].position_offset_x);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_y:	%f\n", i, j, platform->sensors[i].offsets[j].position_offset_y);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].position_offset_z:	%f\n", i, j, platform->sensors[i].offsets[j].position_offset_z);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_mode:	%d\n", i, j, platform->sensors[i].offsets[j].attitude_offset_mode);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_azimuth:	%f\n", i, j, platform->sensors[i].offsets[j].attitude_offset_azimuth);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_roll:	%f\n", i, j, platform->sensors[i].offsets[j].attitude_offset_roll);
				fprintf(stderr,"dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_pitch:	%f\n", i, j, platform->sensors[i].offsets[j].attitude_offset_pitch);
				}
			}
		}

	/* check that all sensor id's are sensible for this platform */
	if (targetsensor < 0 || targetsensor >= platform->num_sensors
			|| platform->source_swathbathymetry < 0 || platform->source_swathbathymetry >= platform->num_sensors
			|| platform->source_attitude < 0 || platform->source_attitude >= platform->num_sensors
			|| platform->source_position < 0 || platform->source_position >= platform->num_sensors
			|| platform->source_depth < 0 || platform->source_depth >= platform->num_sensors)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_PARAMETER;
		}
		
	/* else proceed */
	else
		{
		/* get sensor structures */
		sensor_target   = &platform->sensors[targetsensor];
		sensor_attitude = &platform->sensors[platform->source_attitude];
		
		/* start with zero attitude offset */
		*target_roll_offset = 0.0;
		*target_pitch_offset = 0.0;
		*target_hdg_offset = 0.0;
		
		/* calculate attitude offset for target sensor */
		mb_platform_math_attitude_offset (verbose,
										  sensor_target->offsets[targetsensoroffset].attitude_offset_roll,          
										  sensor_target->offsets[targetsensoroffset].attitude_offset_pitch, 
										  sensor_target->offsets[targetsensoroffset].attitude_offset_azimuth,
										  sensor_attitude->offsets[0].attitude_offset_roll,
										  sensor_attitude->offsets[0].attitude_offset_pitch,
										  sensor_attitude->offsets[0].attitude_offset_azimuth,
										  target_roll_offset, 
										  target_pitch_offset, 
										  target_hdg_offset,
										  error);

		}	

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       target_roll_offset:		%f\n", *target_roll_offset);
		fprintf(stderr,"dbg2       target_pitch_offset:		%f\n", *target_pitch_offset);
		fprintf(stderr,"dbg2       target_hdg_offset:		%f\n", *target_hdg_offset);
		fprintf(stderr,"dbg2       error:			%d\n", *error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:			%d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/

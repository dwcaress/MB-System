/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_reson7k.c	3.00	3/23/2004
 *	$Id: mbsys_reson7k.c,v 5.2 2004-06-18 05:22:32 caress Exp $
 *
 *    Copyright (c) 2004 by
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
 * mbsys_reson7k.c contains the MBIO functions for handling data from 
 * Reson SeaBat 7k series sonars.
 * The data formats associated with Reson SeaBat 7k multibeams 
 * include:
 *    MBSYS_RESON7K formats (code in mbsys_reson7k.c and mbsys_reson7k.h):
 *      MBF_RESON7KR : MBIO ID 191 - Raw vendor format 
 *      MBF_RESON7KP : MBIO ID 192 - Full processed data
 *      MBF_RESON7KP : MBIO ID 193 - Stripped processed data
 *
 * Author:	D. W. Caress
 * Date:	March 23, 2004
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.1  2004/05/21 23:44:49  caress
 * Progress supporting Reson 7k data, including support for extracing subbottom profiler data.
 *
 * Revision 5.0  2004/04/27 01:50:15  caress
 * Adding support for Reson 7k sonar data, including segy extensions.
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
#include "../../include/mbsys_reson7k.h"
#include "../../include/mb_segy.h"
	
/* turn on debug statements here */
/*#define MSYS_RESON7KR_DEBUG 1*/

static char res_id[]="$Id: mbsys_reson7k.c,v 5.2 2004-06-18 05:22:32 caress Exp $";

/*--------------------------------------------------------------------*/
int mbsys_reson7k_zero7kheader(int verbose, s7k_header	*header, 
			int *error)
{
	char	*function_name = "mbsys_reson7k_zero7kheader";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       header:     %d\n",header);
		}

	/* Reson 7k data record header information */
	header->Version = 0;
	header->Offset = 0;
	header->SyncPattern = 0;
	header->OffsetToOptionalData = 0;
	header->OptionalDataIdentifier = 0;
	header->s7kTime.Year = 0;
	header->s7kTime.Day = 0;
	header->s7kTime.Seconds = 0.0;
	header->s7kTime.Hours = 0;
	header->s7kTime.Minutes = 0;
	header->Reserved = 0;
	header->RecordType = 0;
	header->DeviceId = 0;
	header->SubsystemId = 0;
	header->DataSetNumber = 0;
	header->RecordNumber = 0;
	for (i=0;i<8;i++)
		{
		header->PreviousRecord[i] = 0;
		header->NextRecord[i] = 0;
		}
	header->Flags = 0;
	header->Reserved2 = 0;

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
int mbsys_reson7k_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
	char	*function_name = "mbsys_reson7k_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7k_header	*header;
	s7kr_reference	*reference;
	s7kr_sensoruncal	*sensoruncal;
	s7kr_sensorcal	*sensorcal;
	s7kr_position	*position;
	s7kr_attitude	*attitude;
	s7kr_tide	*tide;
	s7kr_altitude	*altitude;
	s7kr_motion	*motion;
	s7kr_depth	*depth;
	s7kr_svp	*svp;
	s7kr_ctd	*ctd;
	s7kr_geodesy	*geodesy;
	s7kr_survey	*survey;
	s7kr_fsdwss	*fsdwsslo;
	s7kr_fsdwss	*fsdwsshi;
	s7kr_fsdwsb	*fsdwsb;
	s7kr_bluefin	*bluefin;
	s7kr_volatilesettings	*volatilesettings;
	s7kr_configuration	*configuration;
	s7kr_beamgeometry	*beamgeometry;
	s7kr_calibration	*calibration;
	s7kr_bathymetry		*bathymetry;
	s7kr_backscatter	*backscatter;
	s7kr_systemevent	*systemevent;
	s7kr_fileheader		*fileheader;
	int	i, j;

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
	status = mb_malloc(verbose,sizeof(struct mbsys_reson7k_struct),
				store_ptr,error);

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *) *store_ptr;

	/* initialize everything */

	/* Type of data record */
	store->kind = MB_DATA_NONE;
	store->type = R7KRECID_None;

	/* MB-System time stamp */
	store->time_d = 0;
	for (i=0;i<7;i++)
		store->time_i[i] = 0;

	/* Reference point information (record 1000) */
	reference = &store->reference;
	mbsys_reson7k_zero7kheader(verbose, &reference->header, error);
	reference->offset_x = 0.0;
	reference->offset_y = 0.0;
	reference->offset_z = 0.0;
	reference->water_z = 0.0;

	/* Sensor uncalibrated offset position information (record 1001) */
	sensoruncal = &store->sensoruncal;
	mbsys_reson7k_zero7kheader(verbose, &sensoruncal->header, error);
	sensoruncal->offset_x = 0.0;
	sensoruncal->offset_y = 0.0;
	sensoruncal->offset_z = 0.0;
	sensoruncal->offset_roll = 0.0;
	sensoruncal->offset_pitch = 0.0;
	sensoruncal->offset_yaw = 0.0;

	/* Sensor calibrated offset position information (record 1002) */
	sensorcal = &store->sensorcal;
	mbsys_reson7k_zero7kheader(verbose, &sensorcal->header, error);
	sensorcal->offset_x = 0.0;
	sensorcal->offset_y = 0.0;
	sensorcal->offset_z = 0.0;
	sensorcal->offset_roll = 0.0;
	sensorcal->offset_pitch = 0.0;
	sensorcal->offset_yaw = 0.0;

	/* Position (record 1003) */
	position = &store->position;
	mbsys_reson7k_zero7kheader(verbose, &position->header, error);
	position->datum = 0;
	position->latitude = 0.0;
	position->longitude = 0.0;
	position->height = 0.0;

	/* Attitude (record 1004) */
	attitude = &store->attitude;
	mbsys_reson7k_zero7kheader(verbose, &attitude->header, error);
	attitude->bitfield = 0;
	attitude->reserved = 0;
	attitude->n = 0;
	attitude->frequency = 0;
	attitude->nalloc = 0;
	attitude->pitch = NULL;
	attitude->roll = NULL;
	attitude->heading = NULL;
	attitude->heave = NULL;

	/* Tide (record 1005) */
	tide = &store->tide;
	mbsys_reson7k_zero7kheader(verbose, &tide->header, error);
	tide->tide = 0.0;
	tide->source = 0;
	tide->reserved = 0;

	/* Altitude (record 1006) */
	altitude = &store->altitude;
	mbsys_reson7k_zero7kheader(verbose, &altitude->header, error);
	altitude->altitude = 0.0;
	
	/* Motion over ground (record 1007) */
	motion = &store->motion;
	mbsys_reson7k_zero7kheader(verbose, &motion->header, error);
	motion->bitfield = 0;
	motion->reserved = 0;
	motion->n = 0;
	motion->frequency = 0;
	motion->nalloc = 0;
	motion->x = NULL;
	motion->y = NULL;
	motion->z = NULL;
	motion->xa = NULL;
	motion->ya = NULL;
	motion->za = NULL;
	
	/* Depth (record 1008) */
	depth = &store->depth;
	mbsys_reson7k_zero7kheader(verbose, &depth->header, error);
	depth->descriptor = 0;
	depth->correction = 0;
	depth->reserved = 0;
	depth->depth = 0.0;
	
	/* Sound velocity profile (record 1009) */
	svp = &store->svp;
	mbsys_reson7k_zero7kheader(verbose, &svp->header, error);
	svp->position_flag = 0;
	svp->reserved1 = 0;
	svp->reserved2 = 0;
	svp->latitude = 0.0;
	svp->longitude = 0.0;
	svp->n = 0;
	svp->nalloc = 0;
	svp->depth = NULL;
	svp->sound_velocity = NULL;
	
	/* CTD (record 1010) */
	ctd = &store->ctd;
	mbsys_reson7k_zero7kheader(verbose, &ctd->header, error);
	ctd->velocity_source_flag = 0;
	ctd->velocity_algorithm = 0;
	ctd->conductivity_flag = 0;
	ctd->pressure_flag = 0;
	ctd->position_flag = 0;
	ctd->reserved1 = 0;
	ctd->reserved2 = 0;
	ctd->latitude = 0.0;
	ctd->longitude = 0.0;
	ctd->frequency = 0.0;
	ctd->n = 0;
	ctd->nalloc = 0;
	ctd->conductivity_salinity = NULL;
	ctd->temperature = NULL;
	ctd->pressure_depth = NULL;
	ctd->sound_velocity = NULL;
	
	/* Geodesy (record 1011) */
	geodesy = &store->geodesy;
	mbsys_reson7k_zero7kheader(verbose, &geodesy->header, error);
	for (i=0;i<32;i++)
		geodesy->spheroid[i] = '\0';
	geodesy->semimajoraxis = 0.0;
	geodesy->flattening = 0.0;
	for (i=0;i<16;i++)
		geodesy->reserved1[i] = '\0';
	for (i=0;i<32;i++)
		geodesy->datum[i] = '\0';
	geodesy->calculation_method = 0;
	geodesy->number_parameters = 0;
	geodesy->dx = 0.0;
	geodesy->dy = 0.0;
	geodesy->dz = 0.0;
	geodesy->rx = 0.0;
	geodesy->ry = 0.0;
	geodesy->rz = 0.0;
	geodesy->scale = 0.0;
	for (i=0;i<35;i++)
		geodesy->reserved2[i] = '\0';
	for (i=0;i<32;i++)
		geodesy->grid_name[i] = '\0';
	geodesy->distance_units = 0;
	geodesy->angular_units = 0;
	geodesy->latitude_origin = 0.0;
	geodesy->central_meriidan = 0.0;
	geodesy->false_easting = 0.0;
	geodesy->false_northing = 0.0;
	geodesy->central_scale_factor = 0.0;
	geodesy->custum_identifier = 0;
	for (i=0;i<50;i++)
		geodesy->reserved3[i] = '\0';

	/* MB-System 7k survey (record 2000) */
	survey = &store->survey;
	mbsys_reson7k_zero7kheader(verbose, &survey->header, error);
	survey->serial_number = 0;
	survey->ping_number = 0;
	survey->number_beams = 0;
	survey->number_pixels = 0;
	survey->number_sslow_pixels = 0;
	survey->number_sshi_pixels = 0;
	survey->longitude = 0.0;
	survey->latitude = 0.0;
	survey->sonar_depth = 0.0;
	survey->sonar_altitude = 0.0;
	survey->heading = 0.0;
	survey->speed = 0.0;
	survey->beamwidthx = 0.0;
	survey->beamwidthy = 0.0;
	for (i=0;i<MBSYS_RESON7K_MAX_BEAMS;i++)
		{
		survey->beam_depth[MBSYS_RESON7K_MAX_BEAMS] = 0.0;
		survey->beam_acrosstrack[MBSYS_RESON7K_MAX_BEAMS] = 0.0;
		survey->beam_alongtrack[MBSYS_RESON7K_MAX_BEAMS] = 0.0;
		survey->beam_flag[MBSYS_RESON7K_MAX_BEAMS] = 0;
		survey->beam_amplitude[MBSYS_RESON7K_MAX_BEAMS] = 0.0;
		}
	for (i=0;i<MBSYS_RESON7K_MAX_PIXELS;i++)
		{
		survey->ss[MBSYS_RESON7K_MAX_PIXELS] = 0.0;
		survey->ss_acrosstrack[MBSYS_RESON7K_MAX_PIXELS] = 0.0;
		survey->ss_alongtrack[MBSYS_RESON7K_MAX_PIXELS] = 0.0;
		survey->sslow[MBSYS_RESON7K_MAX_PIXELS] = 0.0;
		survey->sslow_acrosstrack[MBSYS_RESON7K_MAX_PIXELS] = 0.0;
		survey->sslow_alongtrack[MBSYS_RESON7K_MAX_PIXELS] = 0.0;
		survey->sshi[MBSYS_RESON7K_MAX_PIXELS] = 0.0;
		survey->sshi_acrosstrack[MBSYS_RESON7K_MAX_PIXELS] = 0.0;
		survey->sshi_alongtrack[MBSYS_RESON7K_MAX_PIXELS] = 0.0;
		}

	/* Edgetech FS-DW low frequency sidescan (record 3000) */
	fsdwsslo = &store->fsdwsslo;
	mbsys_reson7k_zero7kheader(verbose, &fsdwsslo->header, error);
	fsdwsslo->msec_timestamp = 0;
	fsdwsslo->ping_number = 0;
	fsdwsslo->number_channels = 0;
	fsdwsslo->total_bytes = 0;
	fsdwsslo->data_format = 0;
	for (i=0;i<2;i++)
		{
		fsdwsslo->channel[i].number = 0;
		fsdwsslo->channel[i].type = 0;
		fsdwsslo->channel[i].data_type = 0;
		fsdwsslo->channel[i].polarity = 0;
		fsdwsslo->channel[i].bytespersample = 0;
		fsdwsslo->channel[i].reserved1[0] = 0;
		fsdwsslo->channel[i].reserved1[1] = 0;
		fsdwsslo->channel[i].reserved1[2] = 0;
		fsdwsslo->channel[i].number_samples = 0;
		fsdwsslo->channel[i].start_time = 0;
		fsdwsslo->channel[i].sample_interval = 0;
		fsdwsslo->channel[i].range = 0.0;
		fsdwsslo->channel[i].voltage = 0.0;
		for (j=0;j<16;j++)
			fsdwsslo->channel[i].name[j] = '\0';
		for (j=0;j<20;j++)
			fsdwsslo->channel[i].reserved2[j] = 0;
		fsdwsslo->channel[i].data_alloc = 0;
		fsdwsslo->channel[i].data = NULL;
		}

	/* Edgetech FS-DW high frequency sidescan (record 3000) */
	fsdwsshi = &store->fsdwsshi;
	mbsys_reson7k_zero7kheader(verbose, &fsdwsshi->header, error);
	fsdwsshi->msec_timestamp = 0;
	fsdwsshi->ping_number = 0;
	fsdwsshi->number_channels = 0;
	fsdwsshi->total_bytes = 0;
	fsdwsshi->data_format = 0;
	for (i=0;i<2;i++)
		{
		fsdwsshi->channel[i].number = 0;
		fsdwsshi->channel[i].type = 0;
		fsdwsshi->channel[i].data_type = 0;
		fsdwsshi->channel[i].polarity = 0;
		fsdwsshi->channel[i].bytespersample = 0;
		fsdwsshi->channel[i].reserved1[0] = 0;
		fsdwsshi->channel[i].reserved1[1] = 0;
		fsdwsshi->channel[i].reserved1[2] = 0;
		fsdwsshi->channel[i].number_samples = 0;
		fsdwsshi->channel[i].start_time = 0;
		fsdwsshi->channel[i].sample_interval = 0;
		fsdwsshi->channel[i].range = 0.0;
		fsdwsshi->channel[i].voltage = 0.0;
		for (j=0;j<16;j++)
			fsdwsshi->channel[i].name[j] = '\0';
		for (j=0;j<20;j++)
			fsdwsshi->channel[i].reserved2[j] = 0;
		fsdwsshi->channel[i].data_alloc = 0;
		fsdwsshi->channel[i].data = NULL;
		}

	/* Edgetech FS-DW subbottom (record 3001) */
	fsdwsb = &store->fsdwsb;
	mbsys_reson7k_zero7kheader(verbose, &fsdwsb->header, error);
	fsdwsb->msec_timestamp = 0;
	fsdwsb->ping_number = 0;
	fsdwsb->number_channels = 0;
	fsdwsb->total_bytes = 0;
	fsdwsb->data_format = 0;
	fsdwsb->channel.number = 0;
	fsdwsb->channel.type = 0;
	fsdwsb->channel.data_type = 0;
	fsdwsb->channel.polarity = 0;
	fsdwsb->channel.bytespersample = 0;
	fsdwsb->channel.reserved1[0] = 0;
	fsdwsb->channel.reserved1[1] = 0;
	fsdwsb->channel.reserved1[2] = 0;
	fsdwsb->channel.number_samples = 0;
	fsdwsb->channel.start_time = 0;
	fsdwsb->channel.sample_interval = 0;
	fsdwsb->channel.range = 0.0;
	fsdwsb->channel.voltage = 0.0;
	for (j=0;j<16;j++)
		fsdwsb->channel.name[j] = '\0';
	for (j=0;j<20;j++)
		fsdwsb->channel.reserved2[j] = '\0';
	fsdwsb->channel.data_alloc = 0;
	fsdwsb->channel.data = NULL;

	/* Bluefin data frames (record 3100) */
	bluefin = &store->bluefin;
	mbsys_reson7k_zero7kheader(verbose, &bluefin->header, error);
	bluefin->msec_timestamp = 0;
	bluefin->number_frames = 0;
	bluefin->frame_size = 0;
	bluefin->data_format = 0;
	for (i=0;i<16;i++)
		bluefin->reserved[i] = 0;
	for (i=0;i<BLUEFIN_MAX_FRAMES;i++)
		{
		bluefin->nav[i].packet_size = 0;
		bluefin->nav[i].version = 0;
		bluefin->nav[i].offset = 0;
		bluefin->nav[i].data_type = 0;
		bluefin->nav[i].data_size = 0;
		bluefin->nav[i].s7kTime.Year = 0;
		bluefin->nav[i].s7kTime.Day = 0;
		bluefin->nav[i].s7kTime.Seconds = 0.0;
		bluefin->nav[i].s7kTime.Hours = 0;
		bluefin->nav[i].s7kTime.Minutes = 0;
		bluefin->nav[i].checksum = 0;
		bluefin->nav[i].reserved = 0;
		bluefin->nav[i].quality = 0;
		bluefin->nav[i].latitude = 0.0;
		bluefin->nav[i].longitude = 0.0;
		bluefin->nav[i].speed = 0.0;
		bluefin->nav[i].depth = 0.0;
		bluefin->nav[i].altitude = 0.0;
		bluefin->nav[i].roll = 0.0;
		bluefin->nav[i].pitch = 0.0;
		bluefin->nav[i].yaw = 0.0;
		bluefin->nav[i].northing_rate = 0.0;
		bluefin->nav[i].easting_rate = 0.0;
		bluefin->nav[i].depth_rate = 0.0;
		bluefin->nav[i].altitude_rate = 0.0;
		bluefin->nav[i].roll_rate = 0.0;
		bluefin->nav[i].pitch_rate = 0.0;
		bluefin->nav[i].yaw_rate = 0.0;
		bluefin->nav[i].position_time = 0.0;
		bluefin->nav[i].altitude_time = 0.0;
		bluefin->environmental[i].packet_size = 0;
		bluefin->environmental[i].version = 0;
		bluefin->environmental[i].offset = 0;
		bluefin->environmental[i].data_type = 0;
		bluefin->environmental[i].data_size = 0;
		bluefin->environmental[i].s7kTime.Year = 0;
		bluefin->environmental[i].s7kTime.Day = 0;
		bluefin->environmental[i].s7kTime.Seconds = 0.0;
		bluefin->environmental[i].s7kTime.Hours = 0;
		bluefin->environmental[i].s7kTime.Minutes = 0;
		bluefin->environmental[i].checksum = 0;
		bluefin->environmental[i].reserved1 = 0;
		bluefin->environmental[i].quality = 0;
		bluefin->environmental[i].sound_speed = 0.0;
		bluefin->environmental[i].conductivity = 0.0;
		bluefin->environmental[i].temperature = 0.0;
		bluefin->environmental[i].pressure = 0.0;
		bluefin->environmental[i].salinity = 0.0;
		bluefin->environmental[i].ctd_time = 0.0;
		bluefin->environmental[i].temperature_time = 0.0;
		for (j=0;j<56;j++)
			bluefin->environmental[i].reserved2[j] = 0;
		}

	/* Reson 7k volatile sonar settings (record 7000) */
	volatilesettings = &store->volatilesettings;
	mbsys_reson7k_zero7kheader(verbose, &volatilesettings->header, error);
	volatilesettings->serial_number = 0;
	volatilesettings->ping_number = 0;
	volatilesettings->frequency = 0.0;
	volatilesettings->sample_rate = 0.0;
	volatilesettings->receiver_bandwidth = 0.0;
	volatilesettings->pulse_width = 0.0;
	volatilesettings->pulse_type = 0;
	volatilesettings->pulse_reserved = 0;
	volatilesettings->ping_period = 0.0;
	volatilesettings->range_selection = 0.0;
	volatilesettings->power_selection = 0.0;
	volatilesettings->gain_selection = 0.0;
	volatilesettings->steering_x = 0.0;
	volatilesettings->steering_y = 0.0;
	volatilesettings->beamwidth_x = 0.0;
	volatilesettings->beamwidth_y = 0.0;
	volatilesettings->focal_point = 0.0;
	volatilesettings->control_flags = 0;
	volatilesettings->projector_selection = 0;
	volatilesettings->transmit_flags = 0;
	volatilesettings->hydrophone_selection = 0;
	volatilesettings->receive_flags = 0;
	volatilesettings->range_minimum = 0.0;
	volatilesettings->range_maximum = 0.0;
	volatilesettings->depth_minimum = 0.0;
	volatilesettings->depth_maximum = 0.0;
	volatilesettings->absorption = 0.0;
	volatilesettings->sound_velocity = 0.0;
	volatilesettings->spreading = 0.0;

	/* Reson 7k volatile sonar settings (record 7000) */
	configuration = &store->configuration;
	mbsys_reson7k_zero7kheader(verbose, &configuration->header, error);
	configuration->serial_number = 0;
	configuration->number_devices = 0;
	for (i=0;i<MBSYS_RESON7K_MAX_DEVICE;i++)
		{
		configuration->device[i].magic_number = 0;
		for (j=0;j<16;j++)
			configuration->device[i].description[j] = '\0';
		configuration->device[i].serial_number = 0;
		configuration->device[i].info_length = 0;
		configuration->device[i].info_alloc = 0;
		configuration->device[i].info = NULL;
		}

	/* Reson 7k beam geometry (record 7004) */
	beamgeometry = &store->beamgeometry;
	mbsys_reson7k_zero7kheader(verbose, &beamgeometry->header, error);
	beamgeometry->serial_number = 0;
	beamgeometry->number_beams = 0;
	for (i=0;i<MBSYS_RESON7K_MAX_BEAMS;i++)
		{
		beamgeometry->angle_x[i] = 0.0;
		beamgeometry->angle_y[i] = 0.0;
		beamgeometry->beamwidth_x[i] = 0.0;
		beamgeometry->beamwidth_y[i] = 0.0;
		}

	/* Reson 7k calibration data (record 7005) */
	calibration = &store->calibration;
	mbsys_reson7k_zero7kheader(verbose, &calibration->header, error);
	calibration->serial_number = 0;
	calibration->number_channels = 0;
	for (i=0;i<MBSYS_RESON7K_MAX_RECEIVERS;i++)
		{
		calibration->gain[i] = 0.0;
		calibration->phase[i] = 0.0;
		}

	/* Reson 7k bathymetry (record 7006) */
	bathymetry = &store->bathymetry;
	mbsys_reson7k_zero7kheader(verbose, &bathymetry->header, error);
	bathymetry->serial_number = 0;
	bathymetry->ping_number = 0;
	bathymetry->number_beams = 0;
	for (i=0;i<MBSYS_RESON7K_MAX_BEAMS;i++)
		{
		bathymetry->range[i] = 0.0;
		bathymetry->quality[i] = 0;
		bathymetry->intensity[i] = 0.0;
		}

	/* Reson 7k backscatter imagery data (record 7007) */
	backscatter = &store->backscatter;
	mbsys_reson7k_zero7kheader(verbose, &backscatter->header, error);
	backscatter->serial_number = 0;
	backscatter->ping_number = 0;
	backscatter->beam_position = 0.0;
	backscatter->control_flags = 0;
	backscatter->number_samples = 0;
	backscatter->port_beamwidth_x = 0.0;
	backscatter->port_beamwidth_y = 0.0;
	backscatter->stbd_beamwidth_x = 0.0;
	backscatter->stbd_beamwidth_y = 0.0;
	backscatter->port_steering_x = 0.0;
	backscatter->port_steering_y = 0.0;
	backscatter->stbd_steering_x = 0.0;
	backscatter->stbd_steering_y = 0.0;
	backscatter->number_beams = 0;
	backscatter->current_beam = 0;
	backscatter->sample_size = 0;
	backscatter->data_type = 0;
	backscatter->nalloc = 0;
	backscatter->port_data = NULL;
	backscatter->stbd_data = NULL;

	/* Reson 7k system event (record 7051) */
	systemevent = &store->systemevent;
	mbsys_reson7k_zero7kheader(verbose, &systemevent->header, error);
	systemevent->serial_number = 0;
	systemevent->event_id = 0;
	systemevent->message_length = 0;
	systemevent->message_alloc = 0;
	systemevent->message = NULL;

	/* Reson 7k file header (record 7200) */
	fileheader = &store->fileheader;
	mbsys_reson7k_zero7kheader(verbose, &fileheader->header, error);
	for (i=0;i<16;i++)
		fileheader->file_identifier[i] = '\0';
	fileheader->version = 0;
	fileheader->reserved = 0;
	for (i=0;i<16;i++)
		fileheader->session_identifier[i] = '\0';
	fileheader->record_data_size = 0;
	fileheader->number_subsystems = 0;
	for (i=0;i<64;i++)
		fileheader->recording_name[i] = '\0';
	for (i=0;i<16;i++)
		fileheader->recording_version[i] = '\0';
	for (i=0;i<64;i++)
		fileheader->user_defined_name[i] = '\0';
	for (i=0;i<128;i++)
		fileheader->notes[i] = '\0';
	for (j=0;j<MBSYS_RESON7K_MAX_DEVICE;j++)
		{
		fileheader->subsystem[j].device_identifier = 0;
		fileheader->subsystem[j].subsystem_identifier = 0;
		fileheader->subsystem[j].system_enumerator = 0;
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
int mbsys_reson7k_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
	char	*function_name = "mbsys_reson7k_deall";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7k_header	*header;
	s7kr_reference	*reference;
	s7kr_sensoruncal	*sensoruncal;
	s7kr_sensorcal	*sensorcal;
	s7kr_position	*position;
	s7kr_attitude	*attitude;
	s7kr_tide	*tide;
	s7kr_altitude	*altitude;
	s7kr_motion	*motion;
	s7kr_depth	*depth;
	s7kr_svp	*svp;
	s7kr_ctd	*ctd;
	s7kr_geodesy	*geodesy;
	s7kr_survey		*survey;
	s7kr_fsdwss	*fsdwsslo;
	s7kr_fsdwss	*fsdwsshi;
	s7kr_fsdwsb	*fsdwsb;
	s7kr_volatilesettings	*volatilesettings;
	s7kr_configuration	*configuration;
	s7kr_beamgeometry	*beamgeometry;
	s7kr_calibration	*calibration;
	s7kr_bathymetry		*bathymetry;
	s7kr_backscatter	*backscatter;
	s7kr_systemevent	*systemevent;
	s7kr_fileheader		*fileheader;
	int	i, j;

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

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *) *store_ptr;

	/* Attitude (record 1004) */
	attitude = &store->attitude;
	attitude->n = 0;
	attitude->nalloc = 0;
	if (attitude->pitch != NULL)
		status = mb_free(verbose,&(attitude->pitch),error);
	if (attitude->roll != NULL)
		status = mb_free(verbose,&(attitude->roll),error);
	if (attitude->heading != NULL)
		status = mb_free(verbose,&(attitude->heading),error);
	if (attitude->heave != NULL)
		status = mb_free(verbose,&(attitude->heave),error);
	
	/* Motion over ground (record 1007) */
	motion = &store->motion;
	motion->n = 0;
	motion->nalloc = 0;
	if (motion->x != NULL)
		status = mb_free(verbose,&(motion->x),error);
	if (motion->y != NULL)
		status = mb_free(verbose,&(motion->y),error);
	if (motion->z != NULL)
		status = mb_free(verbose,&(motion->z),error);
	if (motion->xa != NULL)
		status = mb_free(verbose,&(motion->xa),error);
	if (motion->ya != NULL)
		status = mb_free(verbose,&(motion->ya),error);
	if (motion->za != NULL)
		status = mb_free(verbose,&(motion->za),error);
	
	/* Sound velocity profile (record 1009) */
	svp = &store->svp;
	svp->n = 0;
	svp->nalloc = 0;
	if (svp->depth != NULL)
		status = mb_free(verbose,&(svp->depth),error);
	if (svp->sound_velocity != NULL)
		status = mb_free(verbose,&(svp->sound_velocity),error);
	
	/* CTD (record 1010) */
	ctd = &store->ctd;
	ctd->n = 0;
	ctd->nalloc = 0;
	if (ctd->conductivity_salinity != NULL)
		status = mb_free(verbose,&(ctd->conductivity_salinity),error);
	if (ctd->temperature != NULL)
		status = mb_free(verbose,&(ctd->temperature),error);
	if (ctd->pressure_depth != NULL)
		status = mb_free(verbose,&(ctd->pressure_depth),error);
	if (ctd->sound_velocity != NULL)
		status = mb_free(verbose,&(ctd->sound_velocity),error);

	/* Edgetech FS-DW low frequency sidescan (record 3000) */
	fsdwsslo = &store->fsdwsslo;
	for (i=0;i<2;i++)
		{
		fsdwsslo->channel[i].data_alloc = 0;
		if (fsdwsslo->channel[i].data != NULL)
			status = mb_free(verbose,&(fsdwsslo->channel[i].data),error);
		}

	/* Edgetech FS-DW high frequency sidescan (record 3000) */
	fsdwsshi = &store->fsdwsshi;
	for (i=0;i<2;i++)
		{
		fsdwsshi->channel[i].data_alloc = 0;
		if (fsdwsshi->channel[i].data != NULL)
			status = mb_free(verbose,&(fsdwsshi->channel[i].data),error);
		}

	/* Edgetech FS-DW subbottom (record 3001) */
	fsdwsb = &store->fsdwsb;
	fsdwsb->channel.data_alloc = 0;
	if (fsdwsb->channel.data != NULL)
		status = mb_free(verbose,&(fsdwsb->channel.data),error);

	/* Reson 7k volatile sonar settings (record 7000) */
	configuration = &store->configuration;
	for (i=0;i<MBSYS_RESON7K_MAX_DEVICE;i++)
		{
		configuration->device[i].info_length = 0;
		configuration->device[i].info_alloc = 0;
		if (configuration->device[i].info != NULL)
			status = mb_free(verbose,&(configuration->device[i].info),error);
		}

	/* Reson 7k backscatter imagery data (record 7007) */
	backscatter = &store->backscatter;
	backscatter->number_samples = 0;
	backscatter->nalloc = 0;
	if (backscatter->port_data != NULL)
			status = mb_free(verbose,&(backscatter->port_data),error);
	if (backscatter->stbd_data != NULL)
			status = mb_free(verbose,&(backscatter->stbd_data),error);

	/* Reson 7k system event (record 7051) */
	systemevent = &store->systemevent;
	systemevent->message_length = 0;
	systemevent->message_alloc = 0;
	if (systemevent->message != NULL)
		status = mb_free(verbose,&(systemevent->message),error);

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
/* 7K Macros */
int mbsys_reson7k_checkheader(s7k_header header)
    {
        return ( ( header.Version          >   0                                  ) &&
                 ( header.SyncPattern      ==  0x0000ffff                         ) &&
                 ( header.Size             >   MBSYS_RESON7K_RECORDHEADER_SIZE    ) &&
                 ( header.s7kTime.Day     >= 1    ) &&
                 ( header.s7kTime.Day     <= 366  ) &&
                 ( header.s7kTime.Seconds  >= 0.0f ) &&
                 ( header.s7kTime.Seconds  < 60.0f ) &&
                 ( header.s7kTime.Hours   <= 23   ) &&
                 ( header.s7kTime.Minutes <= 59   )  );
    }


/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_header(int verbose, 
			s7k_header *header,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_header";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       header:     %d\n",header);
		}


	/* print Reson 7k data record header information */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     Version:                 %d\n",first,header->Version);
	fprintf(stderr,"%s     Offset:                  %d\n",first,header->Offset);
	fprintf(stderr,"%s     SyncPattern:             %d\n",first,header->SyncPattern);
	fprintf(stderr,"%s     OffsetToOptionalData:    %d\n",first,header->OffsetToOptionalData);
	fprintf(stderr,"%s     OptionalDataIdentifier:  %d\n",first,header->OptionalDataIdentifier);
	fprintf(stderr,"%s     s7kTime.Year:            %d\n",first,header->s7kTime.Year);
	fprintf(stderr,"%s     s7kTime.Day:             %d\n",first,header->s7kTime.Day);
	fprintf(stderr,"%s     s7kTime.Seconds:         %f\n",first,header->s7kTime.Seconds);
	fprintf(stderr,"%s     s7kTime.Hours:           %d\n",first,header->s7kTime.Hours);
	fprintf(stderr,"%s     7kTime->Minutes:         %d\n",first,header->s7kTime.Minutes);
	fprintf(stderr,"%s     Reserved:                %d\n",first,header->Reserved);
	fprintf(stderr,"%s     RecordType:              %d\n",first,header->RecordType);
	fprintf(stderr,"%s     DeviceId:                %d\n",first,header->DeviceId);
	fprintf(stderr,"%s     SubsystemId:             %d\n",first,header->SubsystemId);
	fprintf(stderr,"%s     DataSetNumber:           %d\n",first,header->DataSetNumber);
	fprintf(stderr,"%s     RecordNumber:            %d\n",first,header->RecordNumber);
	for (i=0;i<8;i++)
		{
		fprintf(stderr,"%s     PreviousRecord[%d]:       %d\n",first,i,header->PreviousRecord[i]);
		fprintf(stderr,"%s     NextRecord[%d]:           %d\n",first,i,header->NextRecord[i]);
		}
	fprintf(stderr,"%s     Flags:                   %d\n",first,header->Flags);
	fprintf(stderr,"%s     Reserved2:               %d\n",first,header->Reserved2);

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
int mbsys_reson7k_print_reference(int verbose, 
			s7kr_reference *reference,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_reference";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       reference:  %d\n",reference);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &reference->header, error);

	/* print Reference point information (record 1000) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     offset_x:                %f\n",first,reference->offset_x);
	fprintf(stderr,"%s     offset_y:                %f\n",first,reference->offset_y);
	fprintf(stderr,"%s     offset_z:                %f\n",first,reference->offset_z);
	fprintf(stderr,"%s     water_z:                 %f\n",first,reference->water_z);

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
int mbsys_reson7k_print_sensoruncal(int verbose, 
			s7kr_sensoruncal *sensoruncal,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_sensoruncal";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:       %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:      %d\n",verbose);
		fprintf(stderr,"dbg2       sensoruncal:  %d\n",sensoruncal);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &sensoruncal->header, error);

	/* print Sensor uncalibrated offset position information (record 1001) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     offset_x:                %f\n",first,sensoruncal->offset_x);
	fprintf(stderr,"%s     offset_y:                %f\n",first,sensoruncal->offset_y);
	fprintf(stderr,"%s     offset_z:                %f\n",first,sensoruncal->offset_z);
	fprintf(stderr,"%s     offset_roll:             %f\n",first,sensoruncal->offset_roll);
	fprintf(stderr,"%s     offset_pitch:            %f\n",first,sensoruncal->offset_pitch);
	fprintf(stderr,"%s     offset_yaw:              %f\n",first,sensoruncal->offset_yaw);

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
int mbsys_reson7k_print_sensorcal(int verbose, 
			s7kr_sensorcal *sensorcal,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_sensorcal";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:       %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:      %d\n",verbose);
		fprintf(stderr,"dbg2       sensorcal:  %d\n",sensorcal);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &sensorcal->header, error);

	/* print Sensor Calibrated offset position information (record 1001) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     offset_x:                %f\n",first,sensorcal->offset_x);
	fprintf(stderr,"%s     offset_y:                %f\n",first,sensorcal->offset_y);
	fprintf(stderr,"%s     offset_z:                %f\n",first,sensorcal->offset_z);
	fprintf(stderr,"%s     offset_roll:             %f\n",first,sensorcal->offset_roll);
	fprintf(stderr,"%s     offset_pitch:            %f\n",first,sensorcal->offset_pitch);
	fprintf(stderr,"%s     offset_yaw:              %f\n",first,sensorcal->offset_yaw);

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
int mbsys_reson7k_print_position(int verbose, 
			s7kr_position *position,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_position";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:       %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:      %d\n",verbose);
		fprintf(stderr,"dbg2       position:     %d\n",position);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &position->header, error);

	/* print Sensor Calibrated offset position information (record 1001) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     datum:                   %d\n",first,position->datum);
	fprintf(stderr,"%s     latitude:                %f\n",first,position->latitude);
	fprintf(stderr,"%s     longitude:               %f\n",first,position->longitude);
	fprintf(stderr,"%s     height:                  %f\n",first,position->height);

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
int mbsys_reson7k_print_attitude(int verbose, 
			s7kr_attitude *attitude,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_attitude";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:       %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:      %d\n",verbose);
		fprintf(stderr,"dbg2       attitude:     %d\n",attitude);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &attitude->header, error);

	/* print Attitude (record 1004) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     bitfield:                   %d\n",first,attitude->bitfield);
	fprintf(stderr,"%s     reserved:                   %d\n",first,attitude->reserved);
	fprintf(stderr,"%s     n:                          %d\n",first,attitude->n);
	fprintf(stderr,"%s     frequency:                  %f\n",first,attitude->frequency);
	fprintf(stderr,"%s     nalloc:                     %d\n",first,attitude->nalloc);
	for (i=0;i<attitude->n;i++)
		fprintf(stderr,"%s     i:%d pitch:%f roll:%f heading:%f heave:%f\n",
					first,i,attitude->pitch[i],attitude->roll[i],
					attitude->heading[i],attitude->heave[i]);

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
int mbsys_reson7k_print_tide(int verbose, 
			s7kr_tide *tide,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_tide";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:       %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:      %d\n",verbose);
		fprintf(stderr,"dbg2       tide:         %d\n",tide);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &tide->header, error);

	/* print Tide (record 1005) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     tide:                       %f\n",first,tide->tide);
	fprintf(stderr,"%s     source:                     %d\n",first,tide->source);
	fprintf(stderr,"%s     reserved:                   %d\n",first,tide->reserved);

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
int mbsys_reson7k_print_altitude(int verbose, 
			s7kr_altitude *altitude,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_altitude";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:       %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:      %d\n",verbose);
		fprintf(stderr,"dbg2       altitude:     %d\n",altitude);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &altitude->header, error);

	/* print Altitude (record 1006) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     altitude:                   %f\n",first,altitude->altitude);

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
int mbsys_reson7k_print_motion(int verbose, 
			s7kr_motion *motion,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_motion";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:       %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:      %d\n",verbose);
		fprintf(stderr,"dbg2       motion:       %d\n",motion);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &motion->header, error);

	/* print Motion over ground (record 1007) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     bitfield:                   %d\n",first,motion->bitfield);
	fprintf(stderr,"%s     reserved:                   %d\n",first,motion->reserved);
	fprintf(stderr,"%s     n:                          %d\n",first,motion->n);
	fprintf(stderr,"%s     frequency:                  %f\n",first,motion->frequency);
	fprintf(stderr,"%s     nalloc:                     %d\n",first,motion->nalloc);
	for (i=0;i<motion->n;i++)
		fprintf(stderr,"%s     i:%d x:%f y:%f z:%f xa:%f ya:%f za:%f\n",
					first,i,motion->x[i],motion->y[i],motion->z[i],
					motion->xa[i],motion->ya[i],motion->za[i]);

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
int mbsys_reson7k_print_depth(int verbose, 
			s7kr_depth *depth,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_depth";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:       %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:      %d\n",verbose);
		fprintf(stderr,"dbg2       depth:        %d\n",depth);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &depth->header, error);

	/* print Depth (record 1008) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     descriptor:                  %d\n",first,depth->descriptor);
	fprintf(stderr,"%s     correction:                  %d\n",first,depth->correction);
	fprintf(stderr,"%s     reserved:                    %d\n",first,depth->reserved);
	fprintf(stderr,"%s     depth:                       %f\n",first,depth->depth);

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
int mbsys_reson7k_print_svp(int verbose, 
			s7kr_svp *svp,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_svp";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:       %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:      %d\n",verbose);
		fprintf(stderr,"dbg2       svp:          %d\n",svp);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &svp->header, error);

	/* print Sound velocity profile (record 1009) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     position_flag:              %d\n",first,svp->position_flag);
	fprintf(stderr,"%s     reserved1:                  %d\n",first,svp->reserved1);
	fprintf(stderr,"%s     reserved2:                  %d\n",first,svp->reserved2);
	fprintf(stderr,"%s     latitude:                   %f\n",first,svp->latitude);
	fprintf(stderr,"%s     longitude:                  %f\n",first,svp->longitude);
	fprintf(stderr,"%s     n:                          %d\n",first,svp->n);
	fprintf(stderr,"%s     nalloc:                     %d\n",first,svp->nalloc);
	for (i=0;i<svp->n;i++)
		fprintf(stderr,"%s     i:%d depth:%f sound_velocity:%f\n",
					first,i,svp->depth[i],svp->sound_velocity[i]);

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
int mbsys_reson7k_print_ctd(int verbose, 
			s7kr_ctd *ctd,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_ctd";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:       %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:      %d\n",verbose);
		fprintf(stderr,"dbg2       ctd:          %d\n",ctd);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &ctd->header, error);

	/* print CTD (record 1010) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     velocity_source_flag:       %d\n",first,ctd->velocity_source_flag);
	fprintf(stderr,"%s     velocity_algorithm:         %d\n",first,ctd->velocity_algorithm);
	fprintf(stderr,"%s     conductivity_flag:          %d\n",first,ctd->conductivity_flag);
	fprintf(stderr,"%s     pressure_flag:              %d\n",first,ctd->pressure_flag);
	fprintf(stderr,"%s     position_flag:              %d\n",first,ctd->position_flag);
	fprintf(stderr,"%s     reserved1:                  %d\n",first,ctd->reserved1);
	fprintf(stderr,"%s     reserved2:                  %d\n",first,ctd->reserved2);
	fprintf(stderr,"%s     latitude:                   %f\n",first,ctd->latitude);
	fprintf(stderr,"%s     longitude:                  %f\n",first,ctd->longitude);
	fprintf(stderr,"%s     frequency:                  %f\n",first,ctd->frequency);
	fprintf(stderr,"%s     n:                          %d\n",first,ctd->n);
	fprintf(stderr,"%s     nalloc:                     %d\n",first,ctd->nalloc);
	for (i=0;i<ctd->n;i++)
		fprintf(stderr,"%s     i:%d conductivity_salinity:%f temperature:%f pressure_depth:%f sound_velocity:%f\n",
					first,i,ctd->conductivity_salinity[i],
					ctd->temperature[i],ctd->pressure_depth[i],
					ctd->sound_velocity[i]);

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
int mbsys_reson7k_print_geodesy(int verbose, 
			s7kr_geodesy *geodesy,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_geodesy";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:       %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:      %d\n",verbose);
		fprintf(stderr,"dbg2       geodesy:      %d\n",geodesy);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &geodesy->header, error);

	/* print Geodesy (record 1011) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     spheroid:                   %s\n",first,geodesy->spheroid);
	fprintf(stderr,"%s     semimajoraxis:              %f\n",first,geodesy->semimajoraxis);
	fprintf(stderr,"%s     flattening:                 %f\n",first,geodesy->flattening);
	fprintf(stderr,"%s     reserved1:                  %s\n",first,geodesy->reserved1);
	fprintf(stderr,"%s     datum:                      %s\n",first,geodesy->datum);
	fprintf(stderr,"%s     calculation_method:         %d\n",first,geodesy->calculation_method);
	fprintf(stderr,"%s     number_parameters:          %d\n",first,geodesy->number_parameters);
	fprintf(stderr,"%s     dx:                         %f\n",first,geodesy->dx);
	fprintf(stderr,"%s     dy:                         %f\n",first,geodesy->dy);
	fprintf(stderr,"%s     dz:                         %f\n",first,geodesy->dz);
	fprintf(stderr,"%s     rx:                         %f\n",first,geodesy->rx);
	fprintf(stderr,"%s     ry:                         %f\n",first,geodesy->ry);
	fprintf(stderr,"%s     rz:                         %f\n",first,geodesy->rz);
	fprintf(stderr,"%s     scale:                      %f\n",first,geodesy->scale);
	fprintf(stderr,"%s     reserved2:                  %s\n",first,geodesy->reserved2);
	fprintf(stderr,"%s     grid_name:                  %s\n",first,geodesy->grid_name);
	fprintf(stderr,"%s     distance_units:             %d\n",first,geodesy->distance_units);
	fprintf(stderr,"%s     angular_units:              %d\n",first,geodesy->angular_units);
	fprintf(stderr,"%s     latitude_origin:            %f\n",first,geodesy->latitude_origin);
	fprintf(stderr,"%s     central_meriidan:           %f\n",first,geodesy->central_meriidan);
	fprintf(stderr,"%s     false_easting:              %f\n",first,geodesy->false_easting);
	fprintf(stderr,"%s     false_northing:             %f\n",first,geodesy->false_northing);
	fprintf(stderr,"%s     central_scale_factor:       %f\n",first,geodesy->central_scale_factor);
	fprintf(stderr,"%s     custum_identifier:          %d\n",first,geodesy->custum_identifier);
	fprintf(stderr,"%s     reserved3:                  %s\n",first,geodesy->reserved3);

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
int mbsys_reson7k_print_survey(int verbose, 
			s7kr_survey *survey,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_survey";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:       %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:      %d\n",verbose);
		fprintf(stderr,"dbg2       survey:      %d\n",survey);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &survey->header, error);

	/* print MB-System 7k survey (record 2000) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     serial_number:              %d\n",first,survey->serial_number);
	fprintf(stderr,"%s     ping_number:                %d\n",first,survey->ping_number);
	fprintf(stderr,"%s     number_beams:               %d\n",first,survey->number_beams);
	fprintf(stderr,"%s     number_pixels:              %d\n",first,survey->number_pixels);
	fprintf(stderr,"%s     number_sslow_pixels:        %d\n",first,survey->number_sslow_pixels);
	fprintf(stderr,"%s     number_sshi_pixels:         %d\n",first,survey->number_sshi_pixels);
	fprintf(stderr,"%s     longitude:                  %f\n",first,survey->longitude);
	fprintf(stderr,"%s     latitude:                   %f\n",first,survey->latitude);
	fprintf(stderr,"%s     sonar_depth:                %f\n",first,survey->sonar_depth);
	fprintf(stderr,"%s     sonar_altitude:             %f\n",first,survey->sonar_altitude);
	fprintf(stderr,"%s     heading:                    %f\n",first,survey->heading);
	fprintf(stderr,"%s     speed:                      %f\n",first,survey->speed);
	fprintf(stderr,"%s     beamwidthx:                 %f\n",first,survey->beamwidthx);
	fprintf(stderr,"%s     beamwidthy:                 %f\n",first,survey->beamwidthy);
	for (i=0;i<survey->number_beams;i++)
		fprintf(stderr,"%s     beam[i]: depth:%f xtrack:%f ltrack:%f amp:%f flag:%d\n",
				survey->beam_depth[i],survey->beam_acrosstrack[i],survey->beam_alongtrack[i],
				survey->beam_amplitude[i],survey->beam_flag[i]);
	for (i=0;i<survey->number_pixels;i++)
		fprintf(stderr,"%s     beam[i]: sidescan:%f xtrack:%f ltrack:%f amp:%f flag:%d\n",
				survey->ss[i],survey->ss_acrosstrack[i],survey->ss_alongtrack[i]);
	for (i=0;i<survey->number_sslow_pixels;i++)
		fprintf(stderr,"%s     beam[i]: low sidescan:%f xtrack:%f ltrack:%f\n",
				survey->sslow[i],survey->sslow_acrosstrack[i],survey->sslow_alongtrack[i]);
	for (i=0;i<survey->number_sshi_pixels;i++)
		fprintf(stderr,"%s     beam[i]: high sidescan:%f xtrack:%f ltrack:%f\n",
				survey->sshi[i],survey->sshi_acrosstrack[i],survey->sshi_alongtrack[i]);

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
int mbsys_reson7k_print_fsdwchannel(int verbose, int data_format,
			s7k_fsdwchannel *fsdwchannel,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_fsdwchannel";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;
	short	*shortptr;
	unsigned short	*ushortptr;
	int	*intptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:       %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:      %d\n",verbose);
		fprintf(stderr,"dbg2       data_format:  %d\n",data_format);
		fprintf(stderr,"dbg2       fsdwchannel:  %d\n",fsdwchannel);
		}

	/* print Edgetech sidescan or subbottom channel header data */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     number:                     %d\n",first,fsdwchannel->number);
	fprintf(stderr,"%s     type:                       %d\n",first,fsdwchannel->type);
	fprintf(stderr,"%s     data_type:                  %d\n",first,fsdwchannel->data_type);
	fprintf(stderr,"%s     polarity:                   %d\n",first,fsdwchannel->polarity);
	fprintf(stderr,"%s     bytespersample:             %d\n",first,fsdwchannel->bytespersample);
	fprintf(stderr,"%s     reserved1[0]                %d\n",first,fsdwchannel->reserved1[0]);
	fprintf(stderr,"%s     reserved1[1]                %d\n",first,fsdwchannel->reserved1[1]);
	fprintf(stderr,"%s     reserved1[2]                %d\n",first,fsdwchannel->reserved1[2]);
	fprintf(stderr,"%s     number_samples:             %d\n",first,fsdwchannel->number_samples);
	fprintf(stderr,"%s     start_time:                 %d\n",first,fsdwchannel->start_time);
	fprintf(stderr,"%s     sample_interval:            %d\n",first,fsdwchannel->sample_interval);
	fprintf(stderr,"%s     range:                      %f\n",first,fsdwchannel->range);
	fprintf(stderr,"%s     voltage:                    %f\n",first,fsdwchannel->voltage);
	fprintf(stderr,"%s     name:                       %s\n",first,fsdwchannel->name);
	fprintf(stderr,"%s     reserved2:                  %s\n",first,fsdwchannel->reserved2);
	fprintf(stderr,"%s     data_alloc:                 %d\n",first,fsdwchannel->data_alloc);
	shortptr = (short *) fsdwchannel->data;
	ushortptr = (unsigned short *) fsdwchannel->data;
	for (i=0;i<fsdwchannel->number_samples;i++)
		{
		if (data_format == EDGETECH_TRACEFORMAT_ENVELOPE)
			fprintf(stderr,"%s     data[%d]:                   %d\n", first,i,ushortptr[i]);
		else if (data_format == EDGETECH_TRACEFORMAT_ANALYTIC)
			fprintf(stderr,"%s     data[%d]:                   %d %d\n", first,i,shortptr[2*i],shortptr[2*i+1]);
		else if (data_format == EDGETECH_TRACEFORMAT_RAW)
			fprintf(stderr,"%s     data[%d]:                   %d\n", first,i,ushortptr[i]);
		else if (data_format == EDGETECH_TRACEFORMAT_REALANALYTIC)
			fprintf(stderr,"%s     data[%d]:                   %d\n", first,i,ushortptr[i]);
		else if (data_format == EDGETECH_TRACEFORMAT_PIXEL)
			fprintf(stderr,"%s     data[%d]:                   %d\n", first,i,ushortptr[i]);
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
int mbsys_reson7k_print_fsdwssheader(int verbose, 
			s7k_fsdwssheader *fsdwssheader,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_fsdwssheader";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;
	short	*shortptr;
	int	*intptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:       %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:      %d\n",verbose);
		fprintf(stderr,"dbg2       fsdwssheader:  %d\n",fsdwssheader);
		}

	/* print Edgetech sidescan or subbottom channel header data */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     subsystem;                    %d\n",first,fsdwssheader->subsystem);
	fprintf(stderr,"%s     channelNum;                   %d\n",first,fsdwssheader->channelNum);
	fprintf(stderr,"%s     pingNum;                      %d\n",first,fsdwssheader->pingNum);
	fprintf(stderr,"%s     packetNum;                    %d\n",first,fsdwssheader->packetNum);
	fprintf(stderr,"%s     trigSource;                   %d\n",first,fsdwssheader->trigSource);
	fprintf(stderr,"%s     samples;                      %d\n",first,fsdwssheader->samples);
	fprintf(stderr,"%s     sampleInterval;               %d\n",first,fsdwssheader->sampleInterval);
	fprintf(stderr,"%s     startDepth;                   %d\n",first,fsdwssheader->startDepth);
	fprintf(stderr,"%s     weightingFactor;              %d\n",first,fsdwssheader->weightingFactor);
	fprintf(stderr,"%s     ADCGain;                      %d\n",first,fsdwssheader->ADCGain);
	fprintf(stderr,"%s     ADCMax;                       %d\n",first,fsdwssheader->ADCMax);
	fprintf(stderr,"%s     rangeSetting;                 %d\n",first,fsdwssheader->rangeSetting);
	fprintf(stderr,"%s     pulseID;                      %d\n",first,fsdwssheader->pulseID);
	fprintf(stderr,"%s     markNumber;                   %d\n",first,fsdwssheader->markNumber);
	fprintf(stderr,"%s     dataFormat;                   %d\n",first,fsdwssheader->dataFormat);
	fprintf(stderr,"%s     reserved;                     %d\n",first,fsdwssheader->reserved);
	fprintf(stderr,"%s     millisecondsToday;            %d\n",first,fsdwssheader->millisecondsToday);
	fprintf(stderr,"%s     year;                         %d\n",first,fsdwssheader->year);
	fprintf(stderr,"%s     day;                          %d\n",first,fsdwssheader->day);
	fprintf(stderr,"%s     hour;                         %d\n",first,fsdwssheader->hour);
	fprintf(stderr,"%s     minute;                       %d\n",first,fsdwssheader->minute);
	fprintf(stderr,"%s     second;                       %d\n",first,fsdwssheader->second);
	fprintf(stderr,"%s     heading;                      %d\n",first,fsdwssheader->heading);
	fprintf(stderr,"%s     pitch;                        %d\n",first,fsdwssheader->pitch);
	fprintf(stderr,"%s     roll;                         %d\n",first,fsdwssheader->roll);
	fprintf(stderr,"%s     heave;                        %d\n",first,fsdwssheader->heave);
	fprintf(stderr,"%s     yaw;                          %d\n",first,fsdwssheader->yaw);
	fprintf(stderr,"%s     depth;                        %d\n",first,fsdwssheader->depth);
	fprintf(stderr,"%s     temperature;                  %d\n",first,fsdwssheader->temperature);
	for (i=0;i<10;i++)
		fprintf(stderr,"%s     reserved2[%d];                 %d\n",first,fsdwssheader->reserved2[i]);

		
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
int mbsys_reson7k_print_fsdwsegyheader(int verbose, 
			s7k_fsdwsegyheader *fsdwsegyheader,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_fsdwsegyheader";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;
	short	*shortptr;
	int	*intptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:       %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:      %d\n",verbose);
		fprintf(stderr,"dbg2       fsdwsegyheader:  %d\n",fsdwsegyheader);
		}

	/* print Edgetech sidescan or subbottom channel header data */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     sequenceNumber;              %d\n",first,fsdwsegyheader->sequenceNumber);
	fprintf(stderr,"%s     startDepth;                  %d\n",first,fsdwsegyheader->startDepth);
	fprintf(stderr,"%s     pingNum;                     %d\n",first,fsdwsegyheader->pingNum);
	fprintf(stderr,"%s     channelNum;                  %d\n",first,fsdwsegyheader->channelNum);
	for (i=0;i<6;i++)
		fprintf(stderr,"%s     unused1[%d];                  %d\n",first,i,fsdwsegyheader->unused1[i]);
	fprintf(stderr,"%s     traceIDCode;                 %d\n",first,fsdwsegyheader->traceIDCode);
	for (i=0;i<2;i++)
		fprintf(stderr,"%s     unused2[%d];                  %d\n",first,i,fsdwsegyheader->unused2[i]);
	fprintf(stderr,"%s     dataFormat;                  %d\n",first,fsdwsegyheader->dataFormat);
	fprintf(stderr,"%s     NMEAantennaeR;               %d\n",first,fsdwsegyheader->NMEAantennaeR);
	fprintf(stderr,"%s     NMEAantennaeO;               %d\n",first,fsdwsegyheader->NMEAantennaeO);
	for (i=0;i<32;i++)
		fprintf(stderr,"%s     RS232[%d];                   %d\n",first,i,fsdwsegyheader->RS232[i]);
	fprintf(stderr,"%s     sourceCoordX;                %d\n",first,fsdwsegyheader->sourceCoordX);
	fprintf(stderr,"%s     sourceCoordY;                %d\n",first,fsdwsegyheader->sourceCoordY);
	fprintf(stderr,"%s     groupCoordX;                 %d\n",first,fsdwsegyheader->groupCoordX);
	fprintf(stderr,"%s     groupCoordY;                 %d\n",first,fsdwsegyheader->groupCoordY);
	fprintf(stderr,"%s     coordUnits;                  %d\n",first,fsdwsegyheader->coordUnits);
	fprintf(stderr,"%s     annotation;                  %s\n",first,fsdwsegyheader->annotation);
	fprintf(stderr,"%s     samples;                     %d\n",first,fsdwsegyheader->samples);
	fprintf(stderr,"%s     sampleInterval;              %d\n",first,fsdwsegyheader->sampleInterval);
	fprintf(stderr,"%s     ADCGain;                     %d\n",first,fsdwsegyheader->ADCGain);
	fprintf(stderr,"%s     pulsePower;                  %d\n",first,fsdwsegyheader->pulsePower);
	fprintf(stderr,"%s     correlated;                  %d\n",first,fsdwsegyheader->correlated);
	fprintf(stderr,"%s     startFreq;                   %d\n",first,fsdwsegyheader->startFreq);
	fprintf(stderr,"%s     endFreq;                     %d\n",first,fsdwsegyheader->endFreq);
	fprintf(stderr,"%s     sweepLength;                 %d\n",first,fsdwsegyheader->sweepLength);
	for (i=0;i<4;i++)
		fprintf(stderr,"%s     unused7[%d];                  %d\n",first,i,fsdwsegyheader->unused7[i]);
	fprintf(stderr,"%s     aliasFreq;                   %d\n",first,fsdwsegyheader->aliasFreq);
	fprintf(stderr,"%s     pulseID;                     %d\n",first,fsdwsegyheader->pulseID);
	for (i=0;i<6;i++)
		fprintf(stderr,"%s     unused8[%d];                  %d\n",first,i,fsdwsegyheader->unused8[i]);
	fprintf(stderr,"%s     year;                        %d\n",first,fsdwsegyheader->year);
	fprintf(stderr,"%s     day;                         %d\n",first,fsdwsegyheader->day);
	fprintf(stderr,"%s     hour;                        %d\n",first,fsdwsegyheader->hour);
	fprintf(stderr,"%s     minute;                      %d\n",first,fsdwsegyheader->minute);
	fprintf(stderr,"%s     second;                      %d\n",first,fsdwsegyheader->second);
	fprintf(stderr,"%s     timeBasis;                   %d\n",first,fsdwsegyheader->timeBasis);
	fprintf(stderr,"%s     weightingFactor;             %d\n",first,fsdwsegyheader->weightingFactor);
	fprintf(stderr,"%s     unused9;                     %d\n",first,fsdwsegyheader->unused9);
	fprintf(stderr,"%s     heading;                     %d\n",first,fsdwsegyheader->heading);
	fprintf(stderr,"%s     pitch;                       %d\n",first,fsdwsegyheader->pitch);
	fprintf(stderr,"%s     roll;                        %d\n",first,fsdwsegyheader->roll);
	fprintf(stderr,"%s     temperature;                 %d\n",first,fsdwsegyheader->temperature);
	fprintf(stderr,"%s     heaveCompensation;           %d\n",first,fsdwsegyheader->heaveCompensation);
	fprintf(stderr,"%s     trigSource;                  %d\n",first,fsdwsegyheader->trigSource);
	fprintf(stderr,"%s     markNumber;                  %d\n",first,fsdwsegyheader->markNumber);
	fprintf(stderr,"%s     NMEAHour;                    %d\n",first,fsdwsegyheader->NMEAHour);
	fprintf(stderr,"%s     NMEAMinutes;                 %d\n",first,fsdwsegyheader->NMEAMinutes);
	fprintf(stderr,"%s     NMEASeconds;                 %d\n",first,fsdwsegyheader->NMEASeconds);
	fprintf(stderr,"%s     NMEACourse;                  %d\n",first,fsdwsegyheader->NMEACourse);
	fprintf(stderr,"%s     NMEASpeed;                   %d\n",first,fsdwsegyheader->NMEASpeed);
	fprintf(stderr,"%s     NMEADay;                     %d\n",first,fsdwsegyheader->NMEADay);
	fprintf(stderr,"%s     NMEAYear;                    %d\n",first,fsdwsegyheader->NMEAYear);
	fprintf(stderr,"%s     millisecondsToday;           %d\n",first,fsdwsegyheader->millisecondsToday);
	fprintf(stderr,"%s     ADCMax;                      %d\n",first,fsdwsegyheader->ADCMax);
	fprintf(stderr,"%s     calConst;                    %d\n",first,fsdwsegyheader->calConst);
	fprintf(stderr,"%s     vehicleID;                   %d\n",first,fsdwsegyheader->vehicleID);
	fprintf(stderr,"%s     softwareVersion;             %s\n",first,fsdwsegyheader->softwareVersion);
	fprintf(stderr,"%s     sphericalCorrection;         %d\n",first,fsdwsegyheader->sphericalCorrection);
	fprintf(stderr,"%s     packetNum;                   %d\n",first,fsdwsegyheader->packetNum);
	fprintf(stderr,"%s     ADCDecimation;               %d\n",first,fsdwsegyheader->ADCDecimation);
	fprintf(stderr,"%s     decimation;                  %d\n",first,fsdwsegyheader->decimation);
	for (i=0;i<7;i++)
		fprintf(stderr,"%s     unuseda[%d];                  %d\n",first,fsdwsegyheader->unuseda[i]);
		
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
int mbsys_reson7k_print_fsdwss(int verbose,
			s7kr_fsdwss *fsdwss,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_fsdwss";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:       %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:      %d\n",verbose);
		fprintf(stderr,"dbg2       fsdwss:       %d\n",fsdwss);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &fsdwss->header, error);

	/* print Edgetech FS-DW sidescan (record 3000) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     msec_timestamp:             %d\n",first,fsdwss->msec_timestamp);
	fprintf(stderr,"%s     ping_number:                %d\n",first,fsdwss->ping_number);
	fprintf(stderr,"%s     number_channels:            %d\n",first,fsdwss->number_channels);
	fprintf(stderr,"%s     total_bytes:                %d\n",first,fsdwss->total_bytes);
	fprintf(stderr,"%s     data_format:                %d\n",first,fsdwss->data_format);
	for (i=0;i<fsdwss->number_channels;i++)
		{
		mbsys_reson7k_print_fsdwchannel(verbose, fsdwss->data_format, &fsdwss->channel[i], error);
		mbsys_reson7k_print_fsdwssheader(verbose, &fsdwss->ssheader[i], error);
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
int mbsys_reson7k_print_fsdwsb(int verbose, 
			s7kr_fsdwsb *fsdwsb,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_fsdwsb";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:       %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:      %d\n",verbose);
		fprintf(stderr,"dbg2       fsdwsb:       %d\n",fsdwsb);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &fsdwsb->header, error);

	/* print Edgetech FS-DW subbottom (record 3001) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     msec_timestamp:             %d\n",first,fsdwsb->msec_timestamp);
	fprintf(stderr,"%s     ping_number:                %d\n",first,fsdwsb->ping_number);
	fprintf(stderr,"%s     number_channels:            %d\n",first,fsdwsb->number_channels);
	fprintf(stderr,"%s     total_bytes:                %d\n",first,fsdwsb->total_bytes);
	fprintf(stderr,"%s     data_format:                %d\n",first,fsdwsb->data_format);
	mbsys_reson7k_print_fsdwchannel(verbose, fsdwsb->data_format, &fsdwsb->channel, error);
	mbsys_reson7k_print_fsdwsegyheader(verbose, &fsdwsb->segyheader, error);
		
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
int mbsys_reson7k_print_bluefin(int verbose, 
			s7kr_bluefin *bluefin,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_bluefin";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:       %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:      %d\n",verbose);
		fprintf(stderr,"dbg2       bluefin:       %d\n",bluefin);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &bluefin->header, error);

	/* print Bluefin data frames (record 3100) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     msec_timestamp:             %d\n",first,bluefin->msec_timestamp);
	fprintf(stderr,"%s     number_frames:              %d\n",first,bluefin->number_frames);
	fprintf(stderr,"%s     frame_size:                 %d\n",first,bluefin->frame_size);
	fprintf(stderr,"%s     data_format:                %d\n",first,bluefin->data_format);
	for(i=0;i<16;i++)
		fprintf(stderr,"%s     reserved[%d]:                %d\n",first,i,bluefin->reserved[i]);
	if (bluefin->data_format == R7KRECID_BluefinNav)
		{
		for (i=0;i<MIN(bluefin->number_frames,BLUEFIN_MAX_FRAMES);i++)
			{
			fprintf(stderr,"%s     nav[%d].packet_size:        %d\n",first,i,bluefin->nav[i].packet_size);
			fprintf(stderr,"%s     nav[%d].version:            %d\n",first,i,bluefin->nav[i].version);
			fprintf(stderr,"%s     nav[%d].offset:             %d\n",first,i,bluefin->nav[i].offset);
			fprintf(stderr,"%s     nav[%d].data_type:          %d\n",first,i,bluefin->nav[i].data_type);
			fprintf(stderr,"%s     nav[%d].data_size:          %d\n",first,i,bluefin->nav[i].data_size);
			fprintf(stderr,"%s     nav[%d].s7kTime.Year:       %d\n",first,i,bluefin->nav[i].s7kTime.Year);
			fprintf(stderr,"%s     nav[%d].s7kTime.Day:        %d\n",first,i,bluefin->nav[i].s7kTime.Day);
			fprintf(stderr,"%s     nav[%d].s7kTime.Seconds:    %f\n",first,i,bluefin->nav[i].s7kTime.Seconds);
			fprintf(stderr,"%s     nav[%d].s7kTime.Hours:      %d\n",first,i,bluefin->nav[i].s7kTime.Hours);
			fprintf(stderr,"%s     nav[%d].7kTime->Minutes:    %d\n",first,i,bluefin->nav[i].s7kTime.Minutes);
			fprintf(stderr,"%s     nav[%d].checksum:           %d\n",first,i,bluefin->nav[i].checksum);
			fprintf(stderr,"%s     nav[%d].reserved:           %d\n",first,i,bluefin->nav[i].reserved);
			fprintf(stderr,"%s     nav[%d].quality:            %d\n",first,i,bluefin->nav[i].quality);
			fprintf(stderr,"%s     nav[%d].latitude:           %f\n",first,i,bluefin->nav[i].latitude);
			fprintf(stderr,"%s     nav[%d].longitude:          %f\n",first,i,bluefin->nav[i].longitude);
			fprintf(stderr,"%s     nav[%d].speed:              %f\n",first,i,bluefin->nav[i].speed);
			fprintf(stderr,"%s     nav[%d].depth:              %f\n",first,i,bluefin->nav[i].depth);
			fprintf(stderr,"%s     nav[%d].roll:               %f\n",first,i,bluefin->nav[i].roll);
			fprintf(stderr,"%s     nav[%d].pitch:              %f\n",first,i,bluefin->nav[i].pitch);
			fprintf(stderr,"%s     nav[%d].yaw:                %f\n",first,i,bluefin->nav[i].yaw);
			fprintf(stderr,"%s     nav[%d].northing_rate:      %f\n",first,i,bluefin->nav[i].northing_rate);
			fprintf(stderr,"%s     nav[%d].easting_rate:       %f\n",first,i,bluefin->nav[i].easting_rate);
			fprintf(stderr,"%s     nav[%d].depth_rate:         %f\n",first,i,bluefin->nav[i].depth_rate);
			fprintf(stderr,"%s     nav[%d].altitude_rate:      %f\n",first,i,bluefin->nav[i].altitude_rate);
			fprintf(stderr,"%s     nav[%d].roll_rate:          %f\n",first,i,bluefin->nav[i].roll_rate);
			fprintf(stderr,"%s     nav[%d].pitch_rate:         %f\n",first,i,bluefin->nav[i].pitch_rate);
			fprintf(stderr,"%s     nav[%d].yaw_rate:           %f\n",first,i,bluefin->nav[i].yaw_rate);
			fprintf(stderr,"%s     nav[%d].position_time:      %f\n",first,i,bluefin->nav[i].position_time);
			fprintf(stderr,"%s     nav[%d].altitude_time:      %f\n",first,i,bluefin->nav[i].altitude_time);
			}
		}
	else if (bluefin->data_format == R7KRECID_BluefinEnvironmental)
		{
		for (i=0;i<MIN(bluefin->number_frames,BLUEFIN_MAX_FRAMES);i++)
			{
			fprintf(stderr,"%s     env[%d].packet_size:        %d\n",first,i,bluefin->environmental[i].packet_size);
			fprintf(stderr,"%s     env[%d].version:            %d\n",first,i,bluefin->environmental[i].version);
			fprintf(stderr,"%s     env[%d].offset:             %d\n",first,i,bluefin->environmental[i].offset);
			fprintf(stderr,"%s     env[%d].data_type:          %d\n",first,i,bluefin->environmental[i].data_type);
			fprintf(stderr,"%s     env[%d].data_size:          %d\n",first,i,bluefin->environmental[i].data_size);
			fprintf(stderr,"%s     env[%d].s7kTime.Year:       %d\n",first,i,bluefin->environmental[i].s7kTime.Year);
			fprintf(stderr,"%s     env[%d].s7kTime.Day:        %d\n",first,i,bluefin->environmental[i].s7kTime.Day);
			fprintf(stderr,"%s     env[%d].s7kTime.Seconds:    %f\n",first,i,bluefin->environmental[i].s7kTime.Seconds);
			fprintf(stderr,"%s     env[%d].s7kTime.Hours:      %d\n",first,i,bluefin->environmental[i].s7kTime.Hours);
			fprintf(stderr,"%s     env[%d].7kTime->Minutes:    %d\n",first,i,bluefin->environmental[i].s7kTime.Minutes);
			fprintf(stderr,"%s     env[%d].checksum:           %d\n",first,i,bluefin->environmental[i].checksum);
			fprintf(stderr,"%s     env[%d].reserved1:          %d\n",first,i,bluefin->environmental[i].reserved1);
			fprintf(stderr,"%s     env[%d].quality:            %d\n",first,i,bluefin->environmental[i].quality);
			fprintf(stderr,"%s     env[%d].sound_speed:        %f\n",first,i,bluefin->environmental[i].sound_speed);
			fprintf(stderr,"%s     env[%d].conductivity:       %f\n",first,i,bluefin->environmental[i].conductivity);
			fprintf(stderr,"%s     env[%d].temperature:        %f\n",first,i,bluefin->environmental[i].temperature);
			fprintf(stderr,"%s     env[%d].pressure:           %f\n",first,i,bluefin->environmental[i].pressure);
			fprintf(stderr,"%s     env[%d].salinity:           %f\n",first,i,bluefin->environmental[i].salinity);
			fprintf(stderr,"%s     env[%d].ctd_time:           %f\n",first,i,bluefin->environmental[i].ctd_time);
			fprintf(stderr,"%s     env[%d].temperature_time:   %f\n",first,i,bluefin->environmental[i].temperature_time);
			for (j=0;j<56;j++)
				fprintf(stderr,"%s     env[%d].reserved2[%2d]:      %d\n",first,i,j,bluefin->environmental[i].reserved2[j]);
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
int mbsys_reson7k_print_volatilesettings(int verbose, 
			s7kr_volatilesettings *volatilesettings,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_volatilesettings";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:            %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       volatilesettings:  %d\n",volatilesettings);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &volatilesettings->header, error);

	/* print Reson 7k volatile sonar settings (record 7000) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     serial_number:              %d\n",first,volatilesettings->serial_number);
	fprintf(stderr,"%s     ping_number:                %d\n",first,volatilesettings->ping_number);
	fprintf(stderr,"%s     frequency:                  %f\n",first,volatilesettings->frequency);
	fprintf(stderr,"%s     sample_rate:                %f\n",first,volatilesettings->sample_rate);
	fprintf(stderr,"%s     receiver_bandwidth:         %f\n",first,volatilesettings->receiver_bandwidth);
	fprintf(stderr,"%s     pulse_width:                %f\n",first,volatilesettings->pulse_width);
	fprintf(stderr,"%s     pulse_type:                 %d\n",first,volatilesettings->pulse_type);
	fprintf(stderr,"%s     pulse_reserved:             %d\n",first,volatilesettings->pulse_reserved);
	fprintf(stderr,"%s     ping_period:                %f\n",first,volatilesettings->ping_period);
	fprintf(stderr,"%s     range_selection:            %f\n",first,volatilesettings->range_selection);
	fprintf(stderr,"%s     power_selection:            %f\n",first,volatilesettings->power_selection);
	fprintf(stderr,"%s     gain_selection:             %f\n",first,volatilesettings->gain_selection);
	fprintf(stderr,"%s     steering_x:                 %f\n",first,volatilesettings->steering_x);
	fprintf(stderr,"%s     steering_y:                 %f\n",first,volatilesettings->steering_y);
	fprintf(stderr,"%s     beamwidth_x:                %f\n",first,volatilesettings->beamwidth_x);
	fprintf(stderr,"%s     beamwidth_y:                %f\n",first,volatilesettings->beamwidth_y);
	fprintf(stderr,"%s     focal_point:                %f\n",first,volatilesettings->focal_point);
	fprintf(stderr,"%s     control_flags:              %d\n",first,volatilesettings->control_flags);
	fprintf(stderr,"%s     projector_selection:        %d\n",first,volatilesettings->projector_selection);
	fprintf(stderr,"%s     transmit_flags:             %d\n",first,volatilesettings->transmit_flags);
	fprintf(stderr,"%s     hydrophone_selection:       %d\n",first,volatilesettings->hydrophone_selection);
	fprintf(stderr,"%s     receive_flags:              %d\n",first,volatilesettings->receive_flags);
	fprintf(stderr,"%s     range_minimum:              %f\n",first,volatilesettings->range_minimum);
	fprintf(stderr,"%s     range_maximum:              %f\n",first,volatilesettings->range_maximum);
	fprintf(stderr,"%s     depth_minimum:              %f\n",first,volatilesettings->depth_minimum);
	fprintf(stderr,"%s     depth_maximum:              %f\n",first,volatilesettings->depth_maximum);
	fprintf(stderr,"%s     absorption:                 %f\n",first,volatilesettings->absorption);
	fprintf(stderr,"%s     sound_velocity:             %f\n",first,volatilesettings->sound_velocity);
	fprintf(stderr,"%s     spreading:                  %f\n",first,volatilesettings->spreading);
		
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
int mbsys_reson7k_print_device(int verbose, 
			s7k_device *device,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_device";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:            %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       device:  %d\n",device);
		}

	/* print Reson 7k device configuration structure */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     magic_number:               %d\n",first,device->magic_number);
	fprintf(stderr,"%s     description:                %s\n",first,device->description);
	fprintf(stderr,"%s     serial_number:              %d\n",first,device->serial_number);
	fprintf(stderr,"%s     info_length:                %d\n",first,device->info_length);
	fprintf(stderr,"%s     info_alloc:                 %d\n",first,device->info_alloc);
	fprintf(stderr,"%s     info:                       %s\n",first,device->info);
		
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
int mbsys_reson7k_print_configuration(int verbose, 
			s7kr_configuration *configuration,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_configuration";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:            %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       configuration:     %d\n",configuration);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &configuration->header, error);

	/* print Reson 7k configuration (record 7001) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     serial_number:              %d\n",first,configuration->serial_number);
	fprintf(stderr,"%s     number_devices:             %d\n",first,configuration->number_devices);
	for (i=0;i<configuration->number_devices;i++)
		mbsys_reson7k_print_device(verbose, &configuration->device[i], error);
		
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
int mbsys_reson7k_print_beamgeometry(int verbose, 
			s7kr_beamgeometry *beamgeometry,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_beamgeometry";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:            %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       beamgeometry:      %d\n",beamgeometry);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &beamgeometry->header, error);

	/* print Reson 7k beam geometry (record 7004) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     serial_number:              %d\n",first,beamgeometry->serial_number);
	fprintf(stderr,"%s     number_beams:               %d\n",first,beamgeometry->number_beams);
	for (i=0;i<beamgeometry->number_beams;i++)
	fprintf(stderr,"%s     beam[%d]:  angle_x:%f angle_y:%f beamwidth_x:%f beamwidth_y:%f\n",
			first,i,beamgeometry->angle_x[i],beamgeometry->angle_y[i],
			beamgeometry->beamwidth_x[i],beamgeometry->beamwidth_y[i]);
		
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
int mbsys_reson7k_print_calibration(int verbose, 
			s7kr_calibration *calibration,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_calibration";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:            %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       calibration:       %d\n",calibration);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &calibration->header, error);

	/* print Reson 7k calibration data (record 7005) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     serial_number:              %d\n",first,calibration->serial_number);
	fprintf(stderr,"%s     number_channels:            %d\n",first,calibration->number_channels);
	for (i=0;i<calibration->number_channels;i++)
		fprintf(stderr,"%s     channel[%d]:  gain:%f phase:%f\n",
				first,i,calibration->gain[i],calibration->phase[i]);
		
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
int mbsys_reson7k_print_bathymetry(int verbose, 
			s7kr_bathymetry *bathymetry,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_bathymetry";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:            %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       bathymetry:        %d\n",bathymetry);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &bathymetry->header, error);

	/* print Reson 7k bathymetry (record 7006) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     serial_number:              %d\n",first,bathymetry->serial_number);
	fprintf(stderr,"%s     ping_number:                %d\n",first,bathymetry->ping_number);
	fprintf(stderr,"%s     number_beams:               %d\n",first,bathymetry->number_beams);
	for (i=0;i<bathymetry->number_beams;i++)
		fprintf(stderr,"%s     beam[%d]:  range:%f quality:%d intensity:%f\n",
				first,i,bathymetry->range[i],bathymetry->quality[i],bathymetry->intensity[i]);
		
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
int mbsys_reson7k_print_backscatter(int verbose, 
			s7kr_backscatter *backscatter,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_backscatter";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;
	mb_s_char	*charptr;
	short	*shortptr;
	int	*intptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:            %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       backscatter:       %d\n",backscatter);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &backscatter->header, error);

	/* print Reson 7k backscatter imagery data (record 7007) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     serial_number:              %d\n",first,backscatter->serial_number);
	fprintf(stderr,"%s     ping_number:                %d\n",first,backscatter->ping_number);
	fprintf(stderr,"%s     beam_position:              %f\n",first,backscatter->beam_position);
	fprintf(stderr,"%s     control_flags:              %d\n",first,backscatter->control_flags);
	fprintf(stderr,"%s     number_samples:             %d\n",first,backscatter->number_samples);
	fprintf(stderr,"%s     port_beamwidth_x:           %f\n",first,backscatter->port_beamwidth_x);
	fprintf(stderr,"%s     port_beamwidth_y:           %f\n",first,backscatter->port_beamwidth_y);
	fprintf(stderr,"%s     stbd_beamwidth_x:           %f\n",first,backscatter->stbd_beamwidth_x);
	fprintf(stderr,"%s     stbd_beamwidth_y:           %f\n",first,backscatter->stbd_beamwidth_y);
	fprintf(stderr,"%s     port_steering_x:            %f\n",first,backscatter->port_steering_x);
	fprintf(stderr,"%s     port_steering_y:            %f\n",first,backscatter->port_steering_y);
	fprintf(stderr,"%s     stbd_steering_x:            %f\n",first,backscatter->stbd_steering_x);
	fprintf(stderr,"%s     stbd_steering_y:            %f\n",first,backscatter->stbd_steering_y);
	fprintf(stderr,"%s     number_beams:               %d\n",first,backscatter->number_beams);
	fprintf(stderr,"%s     current_beam:               %d\n",first,backscatter->current_beam);
	fprintf(stderr,"%s     sample_size:                %d\n",first,backscatter->sample_size);
	fprintf(stderr,"%s     data_type:                  %d\n",first,backscatter->data_type);
	fprintf(stderr,"%s     nalloc:                     %d\n",first,backscatter->nalloc);
	if (backscatter->sample_size == 1)
		{
		charptr = (mb_s_char *) backscatter->port_data;
		for (i=0;i<backscatter->number_samples;i++)
			fprintf(stderr,"%s     port backscatter[%d]:  %d\n",
				first,i,charptr[i]);
		charptr = (mb_s_char *) backscatter->stbd_data;
		for (i=0;i<backscatter->number_samples;i++)
			fprintf(stderr,"%s     stbd backscatter[%d]:  %d\n",
				first,i,charptr[i]);
		}
	else if (backscatter->sample_size == 2)
		{
		shortptr = (short *) backscatter->port_data;
		for (i=0;i<backscatter->number_samples;i++)
			fprintf(stderr,"%s     port backscatter[%d]:  %d\n",
				first,i,shortptr[i]);
		shortptr = (short *) backscatter->stbd_data;
		for (i=0;i<backscatter->number_samples;i++)
			fprintf(stderr,"%s     stbd backscatter[%d]:  %d\n",
				first,i,shortptr[i]);
		}
	else if (backscatter->sample_size == 4)
		{
		intptr = (int *) backscatter->port_data;
		for (i=0;i<backscatter->number_samples;i++)
			fprintf(stderr,"%s     port backscatter[%d]:  %d\n",
				first,i,intptr[i]);
		intptr = (int *) backscatter->stbd_data;
		for (i=0;i<backscatter->number_samples;i++)
			fprintf(stderr,"%s     stbd backscatter[%d]:  %d\n",
				first,i,intptr[i]);
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
int mbsys_reson7k_print_systemevent(int verbose, 
			s7kr_systemevent *systemevent,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_systemevent";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:            %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       systemevent:       %d\n",systemevent);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &systemevent->header, error);

	/* print Reson 7k system event (record 7051) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     serial_number:              %d\n",first,systemevent->serial_number);
	fprintf(stderr,"%s     event_id:                   %d\n",first,systemevent->event_id);
	fprintf(stderr,"%s     message_length:             %d\n",first,systemevent->message_length);
	fprintf(stderr,"%s     message_alloc:              %d\n",first,systemevent->message_alloc);
	fprintf(stderr,"%s     message:                    %s\n",first,systemevent->message);
		
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
int mbsys_reson7k_print_subsystem(int verbose, 
			s7kr_subsystem *subsystem,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_subsystem";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:            %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       subsystem:        %d\n",subsystem);
		}

	/* print Reson 7k subsystem structure */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     device_identifier:          %d\n",first,subsystem->device_identifier);
	fprintf(stderr,"%s     subsystem_identifier:       %d\n",first,subsystem->subsystem_identifier);
	fprintf(stderr,"%s     system_enumerator:          %d\n",first,subsystem->system_enumerator);
		
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
int mbsys_reson7k_print_fileheader(int verbose, 
			s7kr_fileheader *fileheader,
			int *error)
{
	char	*function_name = "mbsys_reson7k_print_fileheader";
	int	status = MB_SUCCESS;
	char	*debug_str = "dbg2  ";
	char	*nodebug_str = "  ";
	char	*first;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:            %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       fileheader:        %d\n",fileheader);
		}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &fileheader->header, error);

	/* print Reson 7k system event (record 7051) */
	if (verbose >= 2)
		first = debug_str;
	else
		{
		first = nodebug_str;
		fprintf(stderr,"\n%sMBIO function <%s> called\n",
			first,function_name);
		}
	fprintf(stderr,"%sInput arguments:\n", first);
	fprintf(stderr,"%s     file_identifier:            0x",first);
	for (i=0;i<16;i++)
		fprintf(stderr,"%hhx",fileheader->file_identifier[i]);
	fprintf(stderr,"\n");
	fprintf(stderr,"%s     version:                    %d\n",first,fileheader->version);
	fprintf(stderr,"%s     reserved:                   %d\n",first,fileheader->reserved);
	fprintf(stderr,"%s     session_identifier:         %s\n",first,fileheader->session_identifier);
	fprintf(stderr,"%s     record_data_size:           %d\n",first,fileheader->record_data_size);
	fprintf(stderr,"%s     number_subsystems:          %d\n",first,fileheader->number_subsystems);
	fprintf(stderr,"%s     recording_name:             %s\n",first,fileheader->recording_name);
	fprintf(stderr,"%s     recording_version:          %s\n",first,fileheader->recording_version);
	fprintf(stderr,"%s     user_defined_name:          %s\n",first,fileheader->user_defined_name);
	fprintf(stderr,"%s     notes:                      %s\n",first,fileheader->notes);
	for (i=0;i<fileheader->number_subsystems;i++)
		mbsys_reson7k_print_subsystem(verbose, &fileheader->subsystem[i], error);
		
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
int mbsys_reson7k_extract(int verbose, void *mbio_ptr, void *store_ptr, 
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_reson7k_extract";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_survey *survey;
	s7kr_position *position;
	s7kr_systemevent *systemevent;
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
	store = (struct mbsys_reson7k_struct *) store_ptr;
	survey = (s7kr_survey *) &store->survey;
	position = (s7kr_position *) &store->position;
	systemevent = (s7kr_systemevent *) &store->systemevent;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		for (i=0;i<7;i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get navigation */
		*navlon = survey->longitude;
		*navlat = survey->latitude;

		/* get heading */
		*heading = survey->heading;

		/* get speed  */
		*speed = 0.036 * survey->speed;
			
		/* set beamwidths in mb_io structure */
		mb_io_ptr->beamwidth_xtrack = survey->beamwidthx;
		mb_io_ptr->beamwidth_ltrack = survey->beamwidthy;

		/* read distance and depth values into storage arrays */
		*nbath = survey->number_beams;
		*namp = *nbath;
		for (i=0;i<*nbath;i++)
			{
			bath[i] = survey->beam_depth[i];
			beamflag[i] = survey->beam_flag[i];
			bathacrosstrack[i] = survey->beam_acrosstrack[i];
			bathalongtrack[i] = survey->beam_alongtrack[i];
			amp[i] = survey->beam_amplitude[i];
			}
		*nss = survey->number_sslow_pixels;
		for (i=0;i<*nss;i++)
			{
			ss[i] = survey->sslow[i];
			ssacrosstrack[i] = survey->sslow_acrosstrack[i];
			ssalongtrack[i] = survey->sslow_alongtrack[i];
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

	/* extract data from structure */
	else if (*kind == MB_DATA_NAV)
		{
		/* get time */
		for (i=0;i<7;i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get navigation */
		*navlon = RTD * position->longitude;
		*navlat = RTD * position->latitude;

		/* get heading */
		*heading = survey->heading;

		/* get speed  */
		*speed = 0.036 * survey->speed;

		/* set beam and pixel numbers */
		*nbath = 0;
		*namp = 0;
		*nss = 0;

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
			}

		/* done translating values */

		}

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		strncpy(comment, systemevent->message, MB_COMMENT_MAXLINE);

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
int mbsys_reson7k_insert(int verbose, void *mbio_ptr, void *store_ptr, 
		int kind, int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_reson7k_insert";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_survey *survey;
	s7kr_position *position;
	s7kr_systemevent *systemevent;
	int	msglen;
	int	i, j;

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
	store = (struct mbsys_reson7k_struct *) store_ptr;
	survey = (s7kr_survey *) &store->survey;
	position = (s7kr_position *) &store->position;
	systemevent = (s7kr_systemevent *) &store->systemevent;

	/* set data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get time */
		for (i=0;i<7;i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation */
		survey->longitude = navlon;
		survey->latitude = navlat;

		/* get heading */
		survey->heading = heading;

		/* get speed  */
		survey->speed = speed / 0.036;

		/* read distance and depth values into storage arrays */
		survey->number_beams = nbath;
		for (i=0;i<survey->number_beams;i++)
			{
			survey->beam_depth[i] = bath[i];
			survey->beam_flag[i] = beamflag[i];
			survey->beam_acrosstrack[i] = bathacrosstrack[i];
			survey->beam_alongtrack[i] = bathalongtrack[i];
			survey->beam_amplitude[i] = amp[i];
			}
		survey->number_sslow_pixels = nss;
		for (i=0;i<survey->number_sslow_pixels;i++)
			{
			survey->sslow[i] = ss[i];
			survey->sslow_acrosstrack[i] = ssacrosstrack[i];
			survey->sslow_alongtrack[i] = ssalongtrack[i];
			}
		}

	/* insert data in nav structure */
	else if (store->kind == MB_DATA_NAV)
		{
		/* get time */
		for (i=0;i<7;i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation */
		position->longitude = DTR * navlon;
		position->latitude = DTR * navlat;

		/* get heading */
		survey->heading = heading;

		/* get speed  */
		survey->speed = speed / 0.036;
		}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT)
		{
		/* make sure memory is allocated for comment */
		msglen = MIN(strlen(comment) +  1, MB_COMMENT_MAXLINE);
		if (msglen % 2 > 0)
			msglen++;
		if (systemevent->message_alloc < msglen)
			{
			status = mb_realloc(verbose, msglen,
						&(systemevent->message), error);
			if (status != MB_SUCCESS)
				{
				systemevent->message_alloc = 0;
				systemevent->message = NULL;
				}
			else
				{
				systemevent->message_alloc = msglen;
				}
			}
		
		/* copy comment */
		if (status == MB_SUCCESS)
			{
fprintf(stderr,"INSERTING COMMENT: %s\n",comment);
			store->type = R7KRECID_7kSystemEvent;
			systemevent->serial_number = 0;
			systemevent->event_id = 1;
			systemevent->message_length = msglen;
			strncpy(systemevent->message, comment, msglen);
			systemevent->header.Size = MBSYS_RESON7K_RECORDHEADER_SIZE 
							+ R7KHDRSIZE_7kSystemEvent 
							+ msglen 
							+ MBSYS_RESON7K_RECORDTAIL_SIZE;
			systemevent->header.OffsetToOptionalData = 0;
			systemevent->header.OptionalDataIdentifier = 0;
			systemevent->header.Reserved = 0;
			systemevent->header.RecordType = R7KRECID_7kSystemEvent;
			systemevent->header.DeviceId = 0;
			systemevent->header.SubsystemId = 0;
			systemevent->header.DataSetNumber = 0;
			systemevent->header.RecordNumber = 0;
			for (i=0;i<8;i++)
				{
				systemevent->header.PreviousRecord[i] = -1;
				systemevent->header.NextRecord[i] = -1;
				}
			systemevent->header.Flags = 0;
			systemevent->header.Reserved2 = 0;
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
int mbsys_reson7k_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams,
	double *ttimes, double *angles, 
	double *angles_forward, double *angles_null,
	double *heave, double *alongtrack_offset, 
	double *draft, double *ssv, int *error)
{
	char	*function_name = "mbsys_reson7k_ttimes";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_survey *survey;
	s7kr_bathymetry *bathymetry;
	s7kr_depth *depth;
	s7kr_beamgeometry *beamgeometry;
	s7kr_attitude *attitude;
	s7kr_ctd *ctd;
	s7kr_reference *reference;
	double	heave_use;
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
	store = (struct mbsys_reson7k_struct *) store_ptr;
	bathymetry = (s7kr_bathymetry *) &store->bathymetry;
	depth = (s7kr_depth *) &store->depth;
	attitude = (s7kr_attitude *) &store->attitude;
	ctd = (s7kr_ctd *) &store->ctd;
	beamgeometry = (s7kr_beamgeometry *) &store->beamgeometry;
	reference = (s7kr_reference *) &store->reference;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get depth offset (heave + sonar depth) */
		if (ctd != NULL && ctd->n > 0)
			*ssv = ctd->sound_velocity[0];
		else
			*ssv = 1500.0;
		if (depth->descriptor == 0)
			{
			*draft = depth->depth;
			heave_use = 0.0;
			}
		else
			{
			*draft = reference->water_z;
			if (attitude->n > 0)
				heave_use = attitude->heave[0];
			}

		/* get travel times, angles */
		*nbeams = bathymetry->number_beams;
		for (i=0;i<bathymetry->number_beams;i++)
			{
			ttimes[i] = bathymetry->range[i];
			angles[i] = RTD * beamgeometry->angle_x[i];
			angles_forward[i] = RTD * beamgeometry->angle_y[i];
			angles_null[i] = angles[i];
			heave[i] = heave_use;
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
int mbsys_reson7k_detects(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams, int *detects, int *error)
{
	char	*function_name = "mbsys_reson7k_detects";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_survey *survey;
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
	store = (struct mbsys_reson7k_struct *) store_ptr;
	survey = (s7kr_survey *) &store->survey;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* read distance and depth values into storage arrays */
		*nbeams = survey->number_beams;
		for (i=0;i<*nbeams;i++)
			{
			detects[i] = MB_DETECT_UNKNOWN;
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
int mbsys_reson7k_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, double *transducer_depth, double *altitudev, 
	int *error)
{
	char	*function_name = "mbsys_reson7k_extract_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_survey *survey;
	s7kr_depth *depth;
	s7kr_altitude *altitude;
	s7kr_attitude *attitude;
	s7kr_reference *reference;
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
	store = (struct mbsys_reson7k_struct *) store_ptr;
	survey = (s7kr_survey *) &store->survey;
	depth = (s7kr_depth *) &store->depth;
	attitude = (s7kr_attitude *) &store->attitude;
	altitude = (s7kr_altitude *) &store->altitude;
	reference = (s7kr_reference *) &store->reference;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get transducer depth and altitude */
		if (depth->descriptor == 0)
			{
			*transducer_depth = depth->depth;
			}
		else
			{
			*transducer_depth = reference->water_z;
			if (attitude->n > 0)
				*transducer_depth += attitude->heave[0];
			}
		*altitudev += altitude->altitude;

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
		fprintf(stderr,"dbg2       altitude:          %f\n",*altitudev);
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft, 
		double *roll, double *pitch, double *heave, 
		int *error)
{
	char	*function_name = "mbsys_reson7k_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_survey *survey;
	s7kr_position *position;
	s7kr_depth *depth;
	s7kr_attitude *attitude;
	s7kr_reference *reference;
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
	store = (struct mbsys_reson7k_struct *) store_ptr;
	survey = (s7kr_survey *) &store->survey;
	position = (s7kr_position *) &store->position;
	depth = (s7kr_depth *) &store->depth;
	attitude = (s7kr_attitude *) &store->attitude;
	survey = (s7kr_survey *) &store->survey;
	reference = (s7kr_reference *) &store->reference;

	/* get data kind */
	*kind = store->kind;

	/* extract data from ping structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		for (i=0;i<7;i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get navigation */
		*navlon = survey->longitude;
		*navlat = survey->latitude;

		/* get heading */
		*heading = survey->heading;

		/* get speed  */
		*speed = 0.036 * survey->speed;

		/* get draft  */
		if (depth->descriptor == 0)
			{
			*draft = depth->depth;
			}
		else
			{
			*draft = reference->water_z;
			}

		/* get roll pitch and heave */
		if (attitude->n > 0)
			{
			*heave = attitude->heave[0];
			*pitch = attitude->pitch[0];
			*roll = attitude->roll[0];
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

	/* extract data from nav structure */
	else if (*kind == MB_DATA_NAV)
		{
		/* get position data structure */
		position = (s7kr_position *) &store->position;

		/* get time */
		for (i=0;i<7;i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get navigation */
		*navlon = RTD * position->longitude;
		*navlat = RTD * position->latitude;

		/* get heading */
		*heading = survey->heading;

		/* get speed  */
		*speed = 0.036 * survey->speed;

		/* get draft  */
		if (depth->descriptor == 0)
			{
			*draft = depth->depth;
			}
		else
			{
			*draft = reference->water_z;
			}

		/* get roll pitch and heave */
		if (attitude->n > 0)
			{
			*heave = attitude->heave[0];
			*pitch = attitude->pitch[0];
			*roll = attitude->roll[0];
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
int mbsys_reson7k_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading, double draft, 
		double roll, double pitch, double heave,
		int *error)
{
	char	*function_name = "mbsys_reson7k_insert_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_survey *survey;
	s7kr_position *position;
	s7kr_depth *depth;
	s7kr_attitude *attitude;
	s7kr_reference *reference;
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
	store = (struct mbsys_reson7k_struct *) store_ptr;
	survey = (s7kr_survey *) &store->survey;
	position = (s7kr_position *) &store->position;
	depth = (s7kr_depth *) &store->depth;
	attitude = (s7kr_attitude *) &store->attitude;
	survey = (s7kr_survey *) &store->survey;
	reference = (s7kr_reference *) &store->reference;

	/* insert data in ping structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get time */
		for (i=0;i<7;i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation */
		survey->longitude = navlon;
		survey->latitude = navlat;

		/* get heading */
		survey->heading = heading;

		/* get speed  */
		survey->speed = speed / 0.036;

		/* get draft  */
		if (depth->descriptor == 0)
			{
			depth->depth = draft;
			}
		else
			{
			reference->water_z = draft;
			}

		/* get roll pitch and heave */
		if (attitude->n > 0)
			{
			attitude->heave[0] = heave;
			attitude->pitch[0] = pitch;
			attitude->roll[0] = roll;
			}
		}

	/* insert data in nav structure */
	else if (store->kind == MB_DATA_NAV)
		{
		/* get time */
		for (i=0;i<7;i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation */
		position->longitude = DTR * navlon;
		position->latitude = DTR * navlat;

		/* get heading */
		survey->heading = heading;

		/* get speed  */
		survey->speed = speed / 0.036;

		/* get draft  */
		if (depth->descriptor == 0)
			{
			depth->depth = draft;
			}
		else
			{
			reference->water_z = draft;
			}

		/* get roll pitch and heave */
		if (attitude->n > 0)
			{
			attitude->heave[0] = heave;
			attitude->pitch[0] = pitch;
			attitude->roll[0] = roll;
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
int mbsys_reson7k_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nsvp,
		double *depth, double *velocity,
		int *error)
{
	char	*function_name = "mbsys_reson7k_extract_svp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_svp *svp;
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
	store = (struct mbsys_reson7k_struct *) store_ptr;
	svp = (s7kr_svp *) &store->svp;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_VELOCITY_PROFILE)
		{
		/* get number of depth-velocity pairs */
		*nsvp = svp->n;
		
		/* get profile */
		for (i=0;i<*nsvp;i++)
			{
			depth[i] = svp->depth[i];
			velocity[i] =svp->sound_velocity[i];
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
int mbsys_reson7k_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
		int nsvp,
		double *depth, double *velocity,
		int *error)
{
	char	*function_name = "mbsys_reson7k_insert_svp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_svp *svp;
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
		fprintf(stderr,"dbg2       nsvp:       %d\n",nsvp);
		for (i=0;i<nsvp;i++)
		    fprintf(stderr,"dbg2       depth[%d]: %f   velocity[%d]: %f\n",i, depth[i], i, velocity[i]);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	svp = (s7kr_svp *) &store->svp;

	/* insert data in structure */
	if (store->kind == MB_DATA_VELOCITY_PROFILE)
		{
		/* allocate memory if necessary */
		if (svp->nalloc < nsvp)
			{
			status = mb_realloc(verbose, nsvp * sizeof(float),
						(char **) &(svp->depth), error);
			status = mb_realloc(verbose, nsvp * sizeof(float),
						(char **) &(svp->sound_velocity), error);
			if (status == MB_SUCCESS)
				{
				svp->nalloc = nsvp;
				}
			else
				{
				svp->n = 0;
				svp->nalloc = 0;
				}
			}
		
		/* get profile */
		if (status == MB_SUCCESS)
			{
			svp->n = nsvp;
			for (i=0;i<svp->n;i++)
				{
				svp->depth[i] = depth[i];
				svp->sound_velocity[i] = velocity[i];
				}
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
int mbsys_reson7k_extract_segytraceheader(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, void *segytraceheader_ptr, int *error)
{
	char	*function_name = "mbsys_reson7k_extract_segytraceheader";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	struct mb_segytraceheader_struct *mb_segytraceheader_ptr;
	s7k_header *header;
	s7kr_survey *survey;
	s7kr_fsdwsb *fsdwsb;
	s7k_fsdwchannel *fsdwchannel;
	s7k_fsdwsegyheader *fsdwsegyheader;
	s7kr_ctd *ctd;
	int	sonardepth;
	int	waterdepth;
	int	watersoundspeed;
	float	fwatertime;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:         %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:        %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:         %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:      %d\n",store_ptr);
		fprintf(stderr,"dbg2       kind:           %d\n",kind);
		fprintf(stderr,"dbg2       segytraceheader_ptr: %d\n",segytraceheader_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_SUBBOTTOM_SUBBOTTOM)
		{
		/* get relevant structures */
		mb_segytraceheader_ptr = (struct mb_segytraceheader_struct *) segytraceheader_ptr;
		survey = &(store->survey);
		ctd = &(store->ctd);
		fsdwsb = &(store->fsdwsb);
		header = &(fsdwsb->header);
		fsdwchannel = &(fsdwsb->channel);
		fsdwsegyheader = &(fsdwsb->segyheader);
		
		/* get needed values */
		sonardepth = (int) (100 * survey->sonar_depth);
		waterdepth = (int) (100 * survey->beam_depth[survey->number_beams / 2]);
		if (ctd->n > 0)
			watersoundspeed = (int) (ctd->sound_velocity[ctd->n-1]);
		else
			watersoundspeed = 1500;
		fwatertime = 2.0 * 0.01 * ((double) waterdepth) / ((double) watersoundspeed);
		mb_get_jtime(verbose, store->time_i, time_j);
			
		/* extract the data */
		mb_segytraceheader_ptr->seq_num 	= fsdwsb->ping_number;
		mb_segytraceheader_ptr->seq_reel 	= fsdwsb->ping_number;
		mb_segytraceheader_ptr->shot_num 	= fsdwsb->ping_number;
		mb_segytraceheader_ptr->shot_tr	= 1;
		mb_segytraceheader_ptr->espn		= 0;
		mb_segytraceheader_ptr->rp_num	= fsdwsb->ping_number;
		mb_segytraceheader_ptr->rp_tr	= 1;
		mb_segytraceheader_ptr->trc_id	= 1;
		mb_segytraceheader_ptr->num_vstk	= 0;
		mb_segytraceheader_ptr->cdp_fold	= 0;
		mb_segytraceheader_ptr->use		= fsdwsb->data_format;
		mb_segytraceheader_ptr->range	= 0;
		mb_segytraceheader_ptr->grp_elev	= -sonardepth;
		mb_segytraceheader_ptr->src_elev	= -sonardepth;
		mb_segytraceheader_ptr->src_depth	= sonardepth;
		mb_segytraceheader_ptr->grp_datum	= 0;
		mb_segytraceheader_ptr->src_datum	= 0;
		mb_segytraceheader_ptr->src_wbd	= waterdepth;
		mb_segytraceheader_ptr->grp_wbd	= waterdepth;
		mb_segytraceheader_ptr->elev_scalar	= -100; 	/* 0.01 m precision for depths */
		mb_segytraceheader_ptr->coord_scalar	= -100;		/* 0.01 arc second precision for position
									= 0.3 m precision at equator */
		mb_segytraceheader_ptr->src_long	= (int)(survey->longitude * 360000.0);
		mb_segytraceheader_ptr->src_lat	= (int)(survey->latitude * 360000.0);
		mb_segytraceheader_ptr->grp_long	= (int)(survey->longitude * 360000.0);
		mb_segytraceheader_ptr->grp_lat	= (int)(survey->latitude * 360000.0);
		mb_segytraceheader_ptr->coord_units	= 2;
		mb_segytraceheader_ptr->wvel		= watersoundspeed;
		mb_segytraceheader_ptr->sbvel	= 0;
		mb_segytraceheader_ptr->src_up_vel	= 0;
		mb_segytraceheader_ptr->grp_up_vel	= 0;
		mb_segytraceheader_ptr->src_static	= 0;
		mb_segytraceheader_ptr->grp_static	= 0;
		mb_segytraceheader_ptr->tot_static	= 0;
		mb_segytraceheader_ptr->laga		= 0;
		mb_segytraceheader_ptr->delay_mils	= 0;
		mb_segytraceheader_ptr->smute_mils	= 0;
		mb_segytraceheader_ptr->emute_mils	= 0;
		mb_segytraceheader_ptr->nsamps	= fsdwchannel->number_samples;
		mb_segytraceheader_ptr->si_micros	= fsdwchannel->sample_interval;
		for (i=0;i<19;i++)
			mb_segytraceheader_ptr->other_1[i]	= 0;
		mb_segytraceheader_ptr->year		= store->time_i[0];
		mb_segytraceheader_ptr->day_of_yr	= time_j[1];
		mb_segytraceheader_ptr->hour		= store->time_i[3];
		mb_segytraceheader_ptr->min		= store->time_i[4];
		mb_segytraceheader_ptr->sec		= store->time_i[5];
		mb_segytraceheader_ptr->mils		= store->time_i[6] / 1000;
		mb_segytraceheader_ptr->tr_weight	= fsdwsegyheader->weightingFactor;
		for (i=0;i<5;i++)
			mb_segytraceheader_ptr->other_2[i]	= 0;
		mb_segytraceheader_ptr->delay	= 0.0;
		mb_segytraceheader_ptr->smute_sec	= 0.0;
		mb_segytraceheader_ptr->emute_sec	= 0.0;
		mb_segytraceheader_ptr->si_secs	= 0.000001 * ((float)fsdwchannel->sample_interval);
		mb_segytraceheader_ptr->wbt_secs	= fwatertime;
		mb_segytraceheader_ptr->end_of_rp	= 0;
		mb_segytraceheader_ptr->dummy1	= 0.0;
		mb_segytraceheader_ptr->dummy2	= 0.0;
		mb_segytraceheader_ptr->dummy3	= 0.0;
		mb_segytraceheader_ptr->dummy4	= 0.0;
		mb_segytraceheader_ptr->dummy5	= 0.0;
		mb_segytraceheader_ptr->dummy6	= 0.0;
		mb_segytraceheader_ptr->dummy7	= 0.0;
		mb_segytraceheader_ptr->dummy8	= 0.0;
		mb_segytraceheader_ptr->dummy9	= 0.0;
		
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
int mbsys_reson7k_extract_segy(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, void *segyheader_ptr, float *segydata, int *error)
{
	char	*function_name = "mbsys_reson7k_extract_segy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	struct mb_segytraceheader_struct *mb_segytraceheader_ptr;
	s7k_header *header;
	s7kr_fsdwsb *fsdwsb;
	s7k_fsdwchannel *fsdwchannel;
	s7k_fsdwsegyheader *fsdwsegyheader;
	short	*shortptr;
	unsigned short	*ushortptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:         %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:        %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:         %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:      %d\n",store_ptr);
		fprintf(stderr,"dbg2       kind:           %d\n",kind);
		fprintf(stderr,"dbg2       segyheader_ptr: %d\n",segyheader_ptr);
		fprintf(stderr,"dbg2       segydata:       %d\n",segydata);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_SUBBOTTOM_SUBBOTTOM)
		{
		/* extract segy header */
		status = mbsys_reson7k_extract_segytraceheader(verbose, mbio_ptr, store_ptr,
								kind, segyheader_ptr, error);

		/* get relevant structures */
		mb_segytraceheader_ptr = (struct mb_segytraceheader_struct *) segyheader_ptr;
		fsdwsb = &(store->fsdwsb);
		header = &(fsdwsb->header);
		fsdwchannel = &(fsdwsb->channel);
		fsdwsegyheader = &(fsdwsb->segyheader);
		shortptr = (short *) fsdwchannel->data;
		ushortptr = (unsigned short *) fsdwchannel->data;
			
		/* extract the data */
		if (fsdwsb->data_format == EDGETECH_TRACEFORMAT_ENVELOPE)
			{
			for (i=0;i<fsdwchannel->number_samples;i++)
				{
				segydata[i] = (float) ushortptr[i];
				}
			}
		else if (fsdwsb->data_format == EDGETECH_TRACEFORMAT_ANALYTIC)
			{
			for (i=0;i<fsdwchannel->number_samples;i++)
				{
				segydata[i] = (float) sqrt((float) (shortptr[2*i] * shortptr[2*i] 
								+ shortptr[2*i+1] * shortptr[2*i+1]));
				}
			}
		else if (fsdwsb->data_format == EDGETECH_TRACEFORMAT_RAW)
			{
			for (i=0;i<fsdwchannel->number_samples;i++)
				{
				segydata[i] = (float) ushortptr[i];
				}
			}
		else if (fsdwsb->data_format == EDGETECH_TRACEFORMAT_REALANALYTIC)
			{
			for (i=0;i<fsdwchannel->number_samples;i++)
				{
				segydata[i] = (float) ushortptr[i];
				}
			}
		else if (fsdwsb->data_format == EDGETECH_TRACEFORMAT_PIXEL)
			{
			for (i=0;i<fsdwchannel->number_samples;i++)
				{
				segydata[i] = (float) ushortptr[i];
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
int mbsys_reson7k_insert_segy(int verbose, void *mbio_ptr, void *store_ptr,
		int kind, void *segyheader_ptr, float *segydata, int *error)
{
	char	*function_name = "mbsys_reson7k_insert_segy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	struct mb_segytraceheader_struct *mb_segytraceheader_ptr;
	s7k_header *header;
	s7kr_survey *survey;
	s7kr_fsdwsb *fsdwsb;
	s7k_fsdwchannel *fsdwchannel;
	s7k_fsdwsegyheader *fsdwsegyheader;
	s7kr_ctd *ctd;
	int	sonardepth;
	int	waterdepth;
	int	watersoundspeed;
	float	fwatertime;
	int	time_j[5];
	float	factor;
	float	datamax;
	int	data_size;
	mb_s_char	*mb_s_charptr;
	short	*shortptr;
	int	*intptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:         %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:        %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:         %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:      %d\n",store_ptr);
		fprintf(stderr,"dbg2       kind:           %d\n",kind);
		fprintf(stderr,"dbg2       segyheader_ptr: %d\n",segyheader_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *) store_ptr;

	/* get data kind */
	store->kind = kind;

	/* insert data to structure */
	if (store->kind == MB_DATA_SUBBOTTOM_SUBBOTTOM)
		{
		/* get relevant structures */
		mb_segytraceheader_ptr = (struct mb_segytraceheader_struct *) segyheader_ptr;
		survey = &(store->survey);
		ctd = &(store->ctd);
		fsdwsb = &(store->fsdwsb);
		header = &(fsdwsb->header);
		fsdwchannel = &(fsdwsb->channel);
		fsdwsegyheader = &(fsdwsb->segyheader);
		
		/* get needed values */
		sonardepth = (int) (100 * survey->sonar_depth);
		waterdepth = (int) (100 * survey->beam_depth[survey->number_beams / 2]);
		if (ctd->n > 0)
			watersoundspeed = (int) (ctd->sound_velocity[ctd->n-1]);
		else
			watersoundspeed = 1500;
		fwatertime = 2.0 * 0.01 * ((double) waterdepth) / ((double) watersoundspeed);
		mb_get_jtime(verbose, store->time_i, time_j);
			
		/* extract the data */
		if (mb_segytraceheader_ptr->shot_num != 0)
			fsdwsb->ping_number = mb_segytraceheader_ptr->shot_num;
		else if (mb_segytraceheader_ptr->seq_reel != 0)
			fsdwsb->ping_number = mb_segytraceheader_ptr->seq_reel;
		else if (mb_segytraceheader_ptr->seq_num != 0)
			fsdwsb->ping_number = mb_segytraceheader_ptr->seq_num;
		else if (mb_segytraceheader_ptr->rp_num != 0)
			fsdwsb->ping_number = mb_segytraceheader_ptr->rp_num;
		else
			fsdwsb->ping_number = 0;
		fsdwsb->data_format = mb_segytraceheader_ptr->use;
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
		survey->sonar_depth  = factor * (float)sonardepth;
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
		if (mb_segytraceheader_ptr->src_long != 0)
			survey->longitude  = factor * ((float)mb_segytraceheader_ptr->src_long);
		else
			survey->longitude  = factor * ((float)mb_segytraceheader_ptr->grp_long);
		if (mb_segytraceheader_ptr->src_lat != 0)
			survey->latitude  = factor * ((float)mb_segytraceheader_ptr->src_lat);
		else
			survey->latitude  = factor * ((float)mb_segytraceheader_ptr->grp_lat);
		fsdwchannel->number_samples = mb_segytraceheader_ptr->nsamps;
		fsdwchannel->sample_interval = mb_segytraceheader_ptr->si_micros;
		time_j[0] = mb_segytraceheader_ptr->year;
		time_j[1] = mb_segytraceheader_ptr->day_of_yr;
		time_j[2] = 60 * mb_segytraceheader_ptr->hour + mb_segytraceheader_ptr->min;
		time_j[3] = mb_segytraceheader_ptr->sec;
		time_j[4] = 1000 * mb_segytraceheader_ptr->mils;
		mb_get_itime(verbose,time_j,store->time_i);
		mb_get_time(verbose,store->time_i,&(store->time_d));
		header->s7kTime.Year = time_j[0];
		header->s7kTime.Day = time_j[1];
		header->s7kTime.Seconds = 0.000001 * store->time_i[6] + store->time_i[5];
		header->s7kTime.Hours = store->time_i[3];
		header->s7kTime.Minutes = store->time_i[4];
		
		/* get max data value */
		datamax = 0.0;
		for (i=0;i<mb_segytraceheader_ptr->nsamps;i++)
			{
			if (fabs(segydata[i]) > datamax)
				datamax = fabs(segydata[i]);
			}
		if (datamax <= 128.0)
			fsdwchannel->bytespersample = 1;
		else if (datamax <= 32768.0)
			fsdwchannel->bytespersample = 2;
		else
			fsdwchannel->bytespersample = 4;
		
		/* make sure enough memory is allocated for channel data */
		data_size = fsdwchannel->bytespersample * fsdwchannel->number_samples;
		if (fsdwchannel->data_alloc < data_size)
			{
			status = mb_realloc(verbose, data_size, &(fsdwchannel->data), error);
			if (status == MB_SUCCESS)
				{
				fsdwchannel->data_alloc = data_size;
				}
			else
				{
				fsdwchannel->data_alloc = 0;
				fsdwchannel->number_samples = 0;
				}
			}

		/* copy over the data */
		if (fsdwchannel->data_alloc >= data_size)
			{
			if (fsdwchannel->bytespersample == 1)
				{
				mb_s_charptr = (mb_s_char *) fsdwchannel->data;
				for (i=0;i<fsdwchannel->number_samples;i++)
					{
					mb_s_charptr[i] = segydata[i];
					}
				}
			else if (fsdwchannel->bytespersample == 2)
				{
				shortptr = (short *) fsdwchannel->data;
				for (i=0;i<fsdwchannel->number_samples;i++)
					{
					shortptr[i] = segydata[i];
					}
				}
			else if (fsdwchannel->bytespersample == 4)
				{
				intptr = (int *) fsdwchannel->data;
				for (i=0;i<fsdwchannel->number_samples;i++)
					{
					intptr[i] = segydata[i];
					}
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
int mbsys_reson7k_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error)
{
	char	*function_name = "mbsys_reson7k_copy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	struct mbsys_reson7k_struct *copy;
	s7k_header *header;
	s7kr_reference *reference;
	s7kr_sensoruncal *sensoruncal;
	s7kr_sensorcal *sensorcal;
	s7kr_position *position;
	s7kr_attitude *attitude;
	s7kr_tide *tide;
	s7kr_altitude *altitude;
	s7kr_motion *motion;
	s7kr_depth *depth;
	s7kr_svp *svp;
	s7kr_ctd *ctd;
	s7kr_geodesy *geodesy;
	s7kr_survey *survey;
	s7kr_fsdwss *fsdwsslo;
	s7kr_fsdwss *fsdwsshi;
	s7kr_fsdwsb *fsdwsb;
	s7kr_volatilesettings *volatilesettings;
	s7kr_configuration *configuration;
	s7kr_beamgeometry *beamgeometry;
	s7kr_calibration *calibration;
	s7kr_bathymetry *bathymetry;
	s7kr_backscatter *backscatter;
	s7kr_systemevent *systemevent;
	s7kr_fileheader *fileheader;
	int	i, j;

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
	store = (struct mbsys_reson7k_struct *) store_ptr;
	copy = (struct mbsys_reson7k_struct *) copy_ptr;
	
	/* copy over structures, allocating memory where necessary */
	
	/* Type of data record */
	copy->kind = store->kind;			/* MB-System record ID */
	copy->type = store->type;			/* Reson record ID */
	
	/* MB-System time stamp */
	copy->time_d = store->time_d;
	for (i=0;i<7;i++)	
		copy->time_i[i] = store->time_i[i];
	
	/* Reference point information (record 1000) */
	/*  Note: these offsets should be zero for submersible vehicles */
	copy->reference = store->reference;
	
	/* Sensor uncalibrated offset position information (record 1001) */
	copy->sensoruncal = store->sensoruncal;
	
	/* Sensor calibrated offset position information (record 1002) */
	copy->sensorcal = store->sensorcal;
	
	/* Position (record 1003) */
	copy->position = store->position;
	
	/* Attitude (record 1004) */
	attitude = &copy->attitude;
	copy->attitude = store->attitude;
	copy->attitude.nalloc = attitude->nalloc;
	copy->attitude.pitch = attitude->pitch;
	copy->attitude.roll = attitude->roll;
	copy->attitude.heading = attitude->heading;
	copy->attitude.heave = attitude->heave;
	if (status == MB_SUCCESS
		&& copy->attitude.nalloc < copy->attitude.n * sizeof(float))
		{
		copy->attitude.nalloc = copy->attitude.n * sizeof(float);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, copy->attitude.nalloc,
					(char **) &(copy->attitude.pitch), error);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, copy->attitude.nalloc,
					(char **) &(copy->attitude.roll), error);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, copy->attitude.nalloc,
					(char **) &(copy->attitude.heading), error);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, copy->attitude.nalloc,
					(char **) &(copy->attitude.heave), error);
		if (status != MB_SUCCESS)
			{
			copy->attitude.n = 0;
			copy->attitude.nalloc = 0;
			}
		}
	if (status == MB_SUCCESS)
		{
		for (i=0;i<copy->attitude.n;i++)
			{
			copy->attitude.pitch[i] = store->attitude.pitch[i];
			copy->attitude.roll[i] = store->attitude.roll[i];
			copy->attitude.heading[i] = store->attitude.heading[i];
			copy->attitude.heave[i] = store->attitude.heave[i];
			}
		}
	
	/* Tide (record 1005) */
	copy->tide = store->tide;
	
	/* Altitude (record 1006) */
	copy->altitude = store->altitude;
	
	/* Motion over ground (record 1007) */
	motion = &copy->motion;
	copy->motion = store->motion;
	copy->motion.nalloc = motion->nalloc;
	copy->motion.x = motion->x;
	copy->motion.y = motion->y;
	copy->motion.z = motion->z;
	copy->motion.xa = motion->xa;
	copy->motion.ya = motion->ya;
	copy->motion.za = motion->za;
	if (status == MB_SUCCESS
		&& copy->motion.nalloc < copy->motion.n * sizeof(float))
		{
		copy->motion.nalloc = copy->motion.n * sizeof(float);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, copy->motion.nalloc,
					(char **) &(copy->motion.x), error);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, copy->motion.nalloc,
					(char **) &(copy->motion.y), error);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, copy->motion.nalloc,
					(char **) &(copy->motion.z), error);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, copy->motion.nalloc,
					(char **) &(copy->motion.xa), error);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, copy->motion.nalloc,
					(char **) &(copy->motion.ya), error);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, copy->motion.nalloc,
					(char **) &(copy->motion.za), error);
		if (status != MB_SUCCESS)
			{
			copy->motion.n = 0;
			copy->motion.nalloc = 0;
			}
		}
	if (status == MB_SUCCESS)
		{
		for (i=0;i<copy->motion.n;i++)
			{
			copy->motion.x[i] = store->motion.x[i];
			copy->motion.y[i] = store->motion.y[i];
			copy->motion.z[i] = store->motion.z[i];
			copy->motion.xa[i] = store->motion.xa[i];
			copy->motion.ya[i] = store->motion.ya[i];
			copy->motion.za[i] = store->motion.za[i];
			}
		}
	
	/* Depth (record 1008) */
	copy->depth = store->depth;
	
	/* Sound velocity profile (record 1009) */
	svp = &copy->svp;
	copy->svp = store->svp;
	copy->svp.nalloc = svp->nalloc;
	copy->svp.depth = svp->depth;
	copy->svp.sound_velocity = svp->sound_velocity;
	if (status == MB_SUCCESS
		&& copy->svp.nalloc < copy->svp.n * sizeof(float))
		{
		copy->svp.nalloc = copy->svp.n * sizeof(float);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, copy->svp.nalloc,
					(char **) &(copy->svp.depth), error);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, copy->svp.nalloc,
					(char **) &(copy->svp.sound_velocity), error);
		if (status != MB_SUCCESS)
			{
			copy->svp.n = 0;
			copy->svp.nalloc = 0;
			}
		}
	if (status == MB_SUCCESS)
		{
		for (i=0;i<copy->svp.n;i++)
			{
			copy->svp.depth[i] = store->svp.depth[i];
			copy->svp.sound_velocity[i] = store->svp.sound_velocity[i];
			}
		}
	
	/* CTD (record 1010) */
	ctd = &copy->ctd;
	copy->ctd = store->ctd;
	copy->ctd.nalloc = ctd->nalloc;
	copy->ctd.conductivity_salinity = ctd->conductivity_salinity;
	copy->ctd.temperature = ctd->temperature;
	copy->ctd.pressure_depth = ctd->pressure_depth;
	copy->ctd.sound_velocity = ctd->sound_velocity;
	if (status == MB_SUCCESS
		&& copy->ctd.nalloc < copy->ctd.n * sizeof(float))
		{
		copy->ctd.nalloc = copy->ctd.n * sizeof(float);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, copy->ctd.nalloc,
					(char **) &(copy->ctd.conductivity_salinity), error);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, copy->ctd.nalloc,
					(char **) &(copy->ctd.temperature), error);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, copy->ctd.nalloc,
					(char **) &(copy->ctd.pressure_depth), error);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, copy->ctd.nalloc,
					(char **) &(copy->ctd.sound_velocity), error);
		if (status != MB_SUCCESS)
			{
			copy->ctd.n = 0;
			copy->ctd.nalloc = 0;
			}
		}
	if (status == MB_SUCCESS)
		{
		for (i=0;i<copy->ctd.n;i++)
			{
			copy->ctd.conductivity_salinity[i] = store->ctd.conductivity_salinity[i];
			copy->ctd.temperature[i] = store->ctd.temperature[i];
			copy->ctd.pressure_depth[i] = store->ctd.pressure_depth[i];
			copy->ctd.sound_velocity[i] = store->ctd.sound_velocity[i];
			}
		}
	
	/* Geodesy (record 1011) */
	copy->geodesy = store->geodesy;
	
	/* MB-System 7k survey (record 2000) */
	copy->survey = store->survey;
	
	/* Edgetech FS-DW low frequency sidescan (record 3000) */
	fsdwsslo = &copy->fsdwsslo;
	copy->fsdwsslo = store->fsdwsslo;
	for (j=0;j<2;j++)
		{
		copy->fsdwsslo.channel[j].data_alloc = fsdwsslo->channel[j].data_alloc;
		copy->fsdwsslo.channel[j].data = fsdwsslo->channel[j].data;
		if (status == MB_SUCCESS
			&& copy->fsdwsslo.channel[j].data_alloc 
				< copy->fsdwsslo.channel[j].number_samples 
					* copy->fsdwsslo.channel[j].bytespersample)
			{
			copy->fsdwsslo.channel[j].data_alloc 
				= copy->fsdwsslo.channel[j].number_samples 
					* copy->fsdwsslo.channel[j].bytespersample;
			if (status == MB_SUCCESS)
			status = mb_realloc(verbose, store->fsdwsslo.channel[j].data_alloc,
						(char **) &(copy->fsdwsslo.channel[j].data), error);
			if (status != MB_SUCCESS)
				{
				copy->fsdwsslo.channel[j].data_alloc = 0;
				copy->fsdwsslo.channel[j].number_samples = 0;
				}
			}
		if (status == MB_SUCCESS)
			{
			for (i=0;i<copy->fsdwsslo.channel[j].data_alloc;i++)
				{
				copy->fsdwsslo.channel[j].data[i] 
					= store->fsdwsslo.channel[j].data[i];
				}
			}
		}
	
	/* Edgetech FS-DW high frequency sidescan (record 3000) */
	fsdwsshi = &copy->fsdwsshi;
	copy->fsdwsshi = store->fsdwsshi;
	for (j=0;j<2;j++)
		{
		copy->fsdwsshi.channel[j].data_alloc = fsdwsshi->channel[j].data_alloc;
		copy->fsdwsshi.channel[j].data = fsdwsshi->channel[j].data;
		if (status == MB_SUCCESS
			&& copy->fsdwsshi.channel[j].data_alloc 
				< copy->fsdwsshi.channel[j].number_samples 
					* copy->fsdwsshi.channel[j].bytespersample)
			{
			copy->fsdwsshi.channel[j].data_alloc 
				= copy->fsdwsshi.channel[j].number_samples 
					* copy->fsdwsshi.channel[j].bytespersample;
			if (status == MB_SUCCESS)
			status = mb_realloc(verbose, store->fsdwsshi.channel[j].data_alloc,
						(char **) &(copy->fsdwsshi.channel[j].data), error);
			if (status != MB_SUCCESS)
				{
				copy->fsdwsshi.channel[j].data_alloc = 0;
				copy->fsdwsshi.channel[j].number_samples = 0;
				}
			}
		if (status == MB_SUCCESS)
			{
			for (i=0;i<copy->fsdwsshi.channel[j].data_alloc;i++)
				{
				copy->fsdwsshi.channel[j].data[i] 
					= store->fsdwsshi.channel[j].data[i];
				}
			}
		}

	/* Edgetech FS-DW subbottom (record 3001) */
	fsdwsb = &copy->fsdwsb;
	copy->fsdwsb = store->fsdwsb;
	copy->fsdwsb.channel.data_alloc = fsdwsb->channel.data_alloc;
	copy->fsdwsb.channel.data = fsdwsb->channel.data;
	if (status == MB_SUCCESS
		&& copy->fsdwsb.channel.data_alloc 
			< copy->fsdwsb.channel.number_samples 
				* copy->fsdwsb.channel.bytespersample)
		{
		copy->fsdwsb.channel.data_alloc 
			= copy->fsdwsb.channel.number_samples 
				* copy->fsdwsb.channel.bytespersample;
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, store->fsdwsb.channel.data_alloc,
					(char **) &(copy->fsdwsb.channel.data), error);
		if (status != MB_SUCCESS)
			{
			copy->fsdwsb.channel.data_alloc = 0;
			copy->fsdwsb.channel.number_samples = 0;
			}
		}
	if (status == MB_SUCCESS)
		{
		for (i=0;i<copy->fsdwsb.channel.data_alloc;i++)
			{
			copy->fsdwsb.channel.data[i] 
				= store->fsdwsb.channel.data[i];
			}
		}

	/* Bluefin Environmental Data Frame (can be included in record 3100) */
	copy->bluefin = store->bluefin;

	/* Reson 7k volatile sonar settings (record 7000) */
	copy->volatilesettings = store->volatilesettings;

	/* Reson 7k configuration (record 7001) */
	configuration = &copy->configuration;
	copy->configuration = store->configuration;
	for (j=0;j<MBSYS_RESON7K_MAX_DEVICE;j++)
		{
		copy->configuration.device[j].info_alloc = configuration->device[j].info_alloc;
		copy->configuration.device[j].info = configuration->device[j].info;
		if (status == MB_SUCCESS
			&& copy->configuration.device[j].info_alloc 
				< copy->configuration.device[j].info_length)
			{
			copy->configuration.device[j].info_alloc = copy->configuration.device[j].info_length;
			if (status == MB_SUCCESS)
			status = mb_realloc(verbose, copy->configuration.device[j].info_alloc,
						(char **) &(copy->configuration.device[j].info), error);
			if (status != MB_SUCCESS)
				{
				copy->configuration.device[j].info_alloc = 0;
				copy->configuration.device[j].info_length = 0;
				}
			}
		if (status == MB_SUCCESS)
			{
			for (i=0;i<copy->configuration.device[j].info_length;i++)
				{
				copy->configuration.device[j].info[i] 
					= store->configuration.device[j].info[i];
				}
			}
		}

	/* Reson 7k beam geometry (record 7004) */
	copy->beamgeometry = store->beamgeometry;

	/* Reson 7k calibration (record 7005) */
	copy->calibration = store->calibration;

	/* Reson 7k bathymetry (record 7006) */
	copy->bathymetry = store->bathymetry;

	/* Reson 7k backscatter imagery data (record 7007) */
	backscatter = &copy->backscatter;
	copy->backscatter = store->backscatter;
	copy->backscatter.nalloc = backscatter->nalloc;
	copy->backscatter.port_data = backscatter->port_data;
	copy->backscatter.stbd_data = backscatter->stbd_data;
	if (status == MB_SUCCESS
		&& copy->backscatter.nalloc 
			< copy->backscatter.number_samples 
				* copy->backscatter.sample_size)
		{
		copy->backscatter.nalloc = copy->backscatter.number_samples * copy->backscatter.sample_size;
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, copy->backscatter.nalloc,
					(char **) &(copy->backscatter.port_data), error);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, copy->backscatter.nalloc,
					(char **) &(copy->backscatter.stbd_data), error);
		if (status != MB_SUCCESS)
			{
			copy->backscatter.nalloc = 0;
			copy->backscatter.number_samples = 0;
			}
		}
	if (status == MB_SUCCESS)
		{
		for (i=0;i<copy->backscatter.number_samples;i++)
			{
			copy->backscatter.port_data[i] = store->backscatter.port_data[i];
			copy->backscatter.stbd_data[i] = store->backscatter.stbd_data[i];
			}
		}

	/* Reson 7k system event (record 7051) */
	systemevent = &copy->systemevent;
	copy->systemevent = store->systemevent;
	copy->systemevent.message_alloc = systemevent->message_alloc;
	copy->systemevent.message = systemevent->message;
	if (status == MB_SUCCESS
		&& copy->systemevent.message_alloc 
			< copy->systemevent.message_length)
		{
		copy->systemevent.message_alloc = copy->systemevent.message_length;
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, copy->systemevent.message_alloc,
					(char **) &(copy->systemevent.message), error);
		if (status != MB_SUCCESS)
			{
			copy->systemevent.message_alloc = 0;
			copy->systemevent.message_length = 0;
			}
		}
	if (status == MB_SUCCESS)
		{
		for (i=0;i<copy->systemevent.message_length;i++)
			{
			copy->systemevent.message[i] = store->systemevent.message[i];
			}
		}

	/* Reson 7k file header (record 7200) */
	copy->fileheader = store->fileheader;
	
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

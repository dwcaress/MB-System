/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_reson7k.c	3.00	3/23/2004
 *	$Id$
 *
 *    Copyright (c) 2004-2019 by
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
 * Author:	D. W. Caress & C. S. Ferreira
 * Date:	March 23, 2004 & June 6, 2017
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"
#include "mb_process.h"
#include "mbsys_reson7k3.h"
#include "mb_segy.h"

/* turn on debug statements here */
/* #define MSYS_RESON7KR_DEBUG 1 */

static char svn_id[] = "$Id$";

/*--------------------------------------------------------------------*/
int mbsys_reson7k_zero7kheader(int verbose, s7k_header *header, int *error) {
	char *function_name = "mbsys_reson7k_zero7kheader";
	int status = MB_SUCCESS;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       header:     %p\n", (void *)header);
	}

	/* Reson 7k data record header information */
	header->Version = 0;
	header->Offset = 0;
	header->SyncPattern = 0;
	header->Size = 0;
	header->OptionalDataOffset = 0;
	header->OptionalDataIdentifier = 0;
	header->s7kTime.Year = 0;
	header->s7kTime.Day = 0;
	header->s7kTime.Seconds = 0.0;
	header->s7kTime.Hours = 0;
	header->s7kTime.Minutes = 0;
	header->RecordVersion = 0;
	header->RecordType = 0;
	header->DeviceId = 0;
	header->Reserved = 0;
	header->SystemEnumerator = 0;
	header->Reserved2 = 0;
	header->Flags = 0;
	header->Reserved3 = 0;
	header->Reserved4 = 0;
	header->FragmentedTotal = 0;
	header->FragmentNumber = 0;

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	char *function_name = "mbsys_reson7k_alloc";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_reference *reference;
	s7kr_sensoruncal *sensoruncal;
	s7kr_sensorcal *sensorcal;
	s7kr_position *position;
	s7kr_customattitude *customattitude;
	s7kr_tide *tide;
	s7kr_altitude *altitude;
	s7kr_motion *motion;
	s7kr_depth *depth;
	s7kr_svp *svp;
	s7kr_ctd *ctd;
	s7kr_geodesy *geodesy;
	s7kr_rollpitchheave *rollpitchheave;
	s7kr_heading *heading;
	s7kr_surveyline *surveyline;
	s7kr_navigation *navigation;
	s7kr_attitude *attitude;
	s7kr_pantilt *pantilt;
	s7kr_sonarinstallationids *sonarinstallationids;
	s7kr_sonarpipeenvironment *sonarpipeenvironment;
	s7kr_contactoutput *contactoutput;
	s7kr_sonarsettings *sonarsettings;
	s7kr_configuration *configuration;
	s7kr_matchfilter *matchfilter;
	s7kr_firmwarehardwareconfiguration *firmwarehardwareconfiguration;
	s7kr_beamgeometry *beamgeometry;
	s7kr_bathymetry *bathymetry;
	s7kr_sidescan *sidescan;
	s7kr_watercolumn *s7kr_watercolumn;
	s7kr_verticaldepth *verticaldepth;
	s7kr_tvg *tvg;
	s7kr_image *image;
	s7kr_pingmotion *pingmotion;
	s7kr_adaptivegate *adaptivegate;
	s7kr_detectionsetup *detectionsetup;
	s7kr_beamformed *beamformed;
	s7kr_vernierprocessingdataraw *vernierprocessingdataraw;
	s7kr_bite *bite;
	s7kr_v37kcentersourceversion *v37kcentersourceversion;
	s7kr_v38kwetendversion *v38kwetendversion;
	s7kr_rawdetection *rawdetection;
	s7kr_snippet *snippet;
	s7kr_vernierprocessingdatafiltered *vernierprocessingdatafiltered;
	s7kr_installation *installation;
	s7kr_bitesummary *bitesummary;
	s7kr_compressedbeamformedmagnitude *compressedbeamformedmagnitude;
	s7kr_compressedwatercolumn *compressedwatercolumn;
	s7kr_segmentedrawdetection *segmentedrawdetection;
	s7kr_calibratedbeam *calibratedbeam;
	s7kr_systemeventmessage *systemeventmessage;
	s7kr_rdrrecordingstatus *rdrrecordingstatus;
	s7kr_subscriptions *subscriptions;
	s7kr_rdrstoragerecording *rdrstoragerecording;
	s7kr_calibrationstatus *calibrationstatus;
	s7kr_calibratedsidescan *calibratedsidescan;
	s7kr_snippetbackscatteringstrength *snippetbackscatteringstrength;
	s7kr_mb2status *mb2status;
	s7kr_fileheader *fileheader;
	s7kr_filecatalogrecord *filecatalogrecord;
	s7kr_timemessage *timemessage;
	s7kr_remotecontrol *remotecontrol;
	s7kr_remotecontrolacknowledge *remotecontrolacknowledge;
	s7kr_remotecontrolnotacknowledge *remotecontrolnotacknowledge;
	s7kr_remotecontrolsettings *remotecontrolsettings;
	s7kr_commonsystemsettings *commonsystemsettings;
	s7kr_svfiltering *svfiltering;
	s7kr_systemlockstatus *systemlockstatus;
	s7kr_soundvelocity *soundvelocity;
	s7kr_absorptionloss *absorptionloss;
	s7kr_spreadingloss *spreadingloss;
	int i, j, k;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_reson7k_struct), (void **)store_ptr, error);

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *)*store_ptr;

	/* initialize everything */

	/* Type of data record */
	store->kind = MB_DATA_NONE;
	store->type = R7KRECID_None;

	/* ping record id's */
	store->current_ping_number = -1;
	store->read_sonarsettings = MB_NO;
	store->read_matchfilter = MB_NO;
	store->read_beamgeometry = MB_NO;
	store->read_bathymetry = MB_NO;
	store->read_sidescan = MB_NO;
	store->read_verticaldepth = MB_NO;
	store->read_tvg = MB_NO;
	store->read_image = MB_NO;
	store->read_pingmotion = MB_NO;
	store->read_detectionsetup = MB_NO;
	store->read_beamformed = MB_NO;
	store->read_rawdetection = MB_NO;
	store->read_snippet = MB_NO;
	store->read_calibratedsidescan = MB_NO;
	store->read_snippetbackscatteringstrength = MB_NO;

	/* MB-System time stamp */
	store->time_d = 0;
	for (i = 0; i < 7; i++)
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
	position->latency = 0.0;
	position->latitude_northing = 0.0;
	position->longitude_easting = 0.0;
	position->height = 0.0;
	position->type = 0;
	position->utm_zone = 0;
	position->quality = 0;
	position->method = 0;
	position->nsat = 0;

	/* Custom attitude (record 1004) */
	customattitude = &store->customattitude;
	mbsys_reson7k_zero7kheader(verbose, &customattitude->header, error);
	customattitude->fieldmask = 0;
	customattitude->reserved = 0;
	customattitude->n = 0;
	customattitude->frequency = 0.0;
	customattitude->nalloc = 0;
	customattitude->pitch = NULL;
	customattitude->roll = NULL;
	customattitude->heading = NULL;
	customattitude->heave = NULL;
	customattitude->pitchrate = NULL;
	customattitude->rollrate = NULL;
	customattitude->headingrate = NULL;
	customattitude->heaverate = NULL;

	/* Tide (record 1005) */
	tide = &store->tide;
	mbsys_reson7k_zero7kheader(verbose, &tide->header, error);
	tide->tide = 0.0;
	tide->source = 0;
	tide->flags = 0;
	tide->gauge = 0;
	tide->datum = 0;
	tide->latency = 0.0;
	tide->latitude_northing = 0.0;
	tide->longitude_easting = 0.0;
	tide->height = 0.0;
	tide->type = 0;
	tide->utm_zone = 0;

	/* Altitude (record 1006) */
	altitude = &store->altitude;
	mbsys_reson7k_zero7kheader(verbose, &altitude->header, error);
	altitude->altitude = 0.0;

	/* Motion over ground (record 1007) */
	motion = &store->motion;
	mbsys_reson7k_zero7kheader(verbose, &motion->header, error);
	motion->flags = 0;
	motion->reserved = 0;
	motion->n = 0;
	motion->frequency = 0.0;
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
	ctd->frequency = 0.0;
	ctd->velocity_source_flag = 0;
	ctd->velocity_algorithm = 0;
	ctd->conductivity_flag = 0;
	ctd->pressure_flag = 0;
	ctd->position_flag = 0;
	ctd->validity = 0;
	ctd->reserved = 0;
	ctd->latitude = 0.0;
	ctd->longitude = 0.0;
	ctd->sample_rate = 0.0;
	ctd->n = 0;
	ctd->nalloc = 0;
	ctd->conductivity_salinity = NULL;
	ctd->temperature = NULL;
	ctd->pressure_depth = NULL;
	ctd->sound_velocity = NULL;
	ctd->absorption = NULL;

	/* Geodesy (record 1011) */
	geodesy = &store->geodesy;
	mbsys_reson7k_zero7kheader(verbose, &geodesy->header, error);
	for (i = 0; i < 32; i++)
		geodesy->spheroid[i] = '\0';
	geodesy->semimajoraxis = 0.0;
	geodesy->flattening = 0.0;
	for (i = 0; i < 16; i++)
		geodesy->reserved1[i] = '\0';
	for (i = 0; i < 32; i++)
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
	for (i = 0; i < 35; i++)
		geodesy->reserved2[i] = '\0';
	for (i = 0; i < 32; i++)
		geodesy->grid_name[i] = '\0';
	geodesy->distance_units = 0;
	geodesy->angular_units = 0;
	geodesy->latitude_origin = 0.0;
	geodesy->central_meridan = 0.0;
	geodesy->false_easting = 0.0;
	geodesy->false_northing = 0.0;
	geodesy->central_scale_factor = 0.0;
	geodesy->custom_identifier = 0;
	for (i = 0; i < 50; i++)
		geodesy->reserved3[i] = '\0';

	/* Roll pitch heave (record 1012) */
	rollpitchheave = &store->rollpitchheave;
	mbsys_reson7k_zero7kheader(verbose, &rollpitchheave->header, error);
	rollpitchheave->roll = 0.0;
	rollpitchheave->pitch = 0.0;
	rollpitchheave->heave = 0.0;

	/* Heading (record 1013) */
	heading = &store->heading;
	mbsys_reson7k_zero7kheader(verbose, &heading->header, error);
	heading->heading = 0.0;

	/* Survey Line (record 1014) */
	surveyline = &store->surveyline;
	mbsys_reson7k_zero7kheader(verbose, &surveyline->header, error);
	surveyline->n = 0;
	surveyline->type = 0;
	surveyline->turnradius = 0.0;
	for (i = 0; i < 64; i++)
		surveyline->name[i] = '\0';
	surveyline->nalloc = 0;
	surveyline->latitude_northing = NULL;
	surveyline->longitude_easting = NULL;

	/* Navigation (record 1015) */
	navigation = &store->navigation;
	mbsys_reson7k_zero7kheader(verbose, &navigation->header, error);
	navigation->vertical_reference = 0;
	navigation->latitude = 0.0;
	navigation->longitude = 0.0;
	navigation->position_accuracy = 0.0;
	navigation->height = 0.0;
	navigation->height_accuracy = 0.0;
	navigation->speed = 0.0;
	navigation->course = 0.0;
	navigation->heading = 0.0;

	/* Attitude (record 1016) */
	attitude = &store->attitude;
	mbsys_reson7k_zero7kheader(verbose, &attitude->header, error);
	attitude->n = 0;
	attitude->nalloc = 0;
	attitude->delta_time = NULL;
	attitude->roll = NULL;
	attitude->pitch = NULL;
	attitude->heave = NULL;
	attitude->heading = NULL;

	/* Pan Tilt (record 1017) */
	pantilt = &store->pantilt;
	mbsys_reson7k_zero7kheader(verbose, &pantilt->header, error);
	pantilt->pan = 0.0;
	pantilt->tilt = 0.0;

	/* Sonar Installation Identifiers (record 1020) */
	sonarinstallationids = &store->sonarinstallationids;
	mbsys_reson7k_zero7kheader(verbose, &sonarinstallationids->header, error);
	sonarinstallationids->system_id = 0;
	sonarinstallationids->tx_id = 0;
	sonarinstallationids->rx_id = 0;
	sonarinstallationids->std_id = 0;
	sonarinstallationids->conf_pars = 0;
	sonarinstallationids->tx_length = 0.0;
	sonarinstallationids->tx_width = 0.0;
	sonarinstallationids->tx_height = 0.0;
	sonarinstallationids->tx_radius = 0.0;
	sonarinstallationids->offset_srp2tx_x = 0.0;
	sonarinstallationids->offset_srp2tx_y = 0.0;
	sonarinstallationids->offset_srp2tx_z = 0.0;
	sonarinstallationids->offset_tx_roll = 0.0;
	sonarinstallationids->offset_tx_pitch = 0.0;
	sonarinstallationids->offset_tx_yaw = 0.0;
	sonarinstallationids->rx_length = 0.0;
	sonarinstallationids->rx_width = 0.0;
	sonarinstallationids->rx_height = 0.0;
	sonarinstallationids->rx_radius = 0.0;
	sonarinstallationids->offset_srp2rx_x = 0.0;
	sonarinstallationids->offset_srp2rx_y = 0.0;
	sonarinstallationids->offset_srp2rx_z = 0.0;
	sonarinstallationids->offset_rx_roll = 0.0;
	sonarinstallationids->offset_rx_pitch = 0.0;
	sonarinstallationids->offset_rx_yaw = 0.0;
	sonarinstallationids->frequency = 0.0;
	sonarinstallationids->offset_vrp2srp_x = 0.0;
	sonarinstallationids->offset_vrp2srp_y = 0.0;
	sonarinstallationids->offset_vrp2srp_z = 0.0;
	sonarinstallationids->cable_length = 0;
	for (i = 0; i < 44; i++)
		sonarinstallationids->reserved[i] = '\0';

	/* Sonar Pipe Environment (record 2004) */
	sonarpipeenvironment = &store->sonarpipeenvironment;
	mbsys_reson7k_zero7kheader(verbose, &sonarpipeenvironment->header, error);
	sonarpipeenvironment->pipe_number = 0;
	for (i = 0; i < 10; i++)
		sonarpipeenvironment->s7k_time[i] = '\0';
	sonarpipeenvironment->ping_number = 0;
	sonarpipeenvironment->multiping_number = 0;
	sonarpipeenvironment->pipe_diameter = 0.0;
	sonarpipeenvironment->sound_velocity = 0.0;
	sonarpipeenvironment->sample_rate = 0.0;
	sonarpipeenvironment->finished = 0;
	sonarpipeenvironment->points_number = 0;
	sonarpipeenvironment->n = 0;
	for (i = 0; i < 10; i++)
		sonarpipeenvironment->reserved[i] = '\0';
	sonarpipeenvironment->nalloc = 0;
	sonarpipeenvironment->x = NULL;
	sonarpipeenvironment->y = NULL;
	sonarpipeenvironment->z = NULL;
	sonarpipeenvironment->angle = NULL;
	sonarpipeenvironment->sample_number = NULL;

	/* Contact Output (record 3001) */
	contactoutput = &store->contactoutput;
	mbsys_reson7k_zero7kheader(verbose, &contactoutput->header, error);
	contactoutput->target_id = 0;
	contactoutput->ping_number = 0;
	for (i = 0; i < 10; i++)
		contactoutput->s7k_time[i] = '\0';
	for (j = 0; j < 128; j++)
		contactoutput->operator_name[i] = '\0';
	contactoutput->contact_state = 0;
	contactoutput->range = 0.0;
	contactoutput->bearing = 0.0;
	contactoutput->info_flags = 0;
	contactoutput->latitude = 0.0;
	contactoutput->longitude = 0.0;
	contactoutput->azimuth = 0.0;
	contactoutput->contact_length = 0.0;
	contactoutput->contact_width = 0.0;
	for (k = 0; k < 128; k++)
		contactoutput->classification[i] = '\0';
	for (l = 0; l < 128; l++)
		contactoutput->description[i] = '\0';

	/* Reson 7k volatile sonar settings (record 7000) */
	sonarsettings = &store->sonarsettings;
	mbsys_reson7k_zero7kheader(verbose, &sonarsettings->header, error);
	sonarsettings->serial_number = 0;
	sonarsettings->ping_number = 0;
	sonarsettings->multi_ping = 0;
	sonarsettings->frequency = 0.0;
	sonarsettings->sample_rate = 0.0;
	sonarsettings->receiver_bandwidth = 0.0;
	sonarsettings->tx_pulse_width = 0.0;
	sonarsettings->tx_pulse_type = 0;
	sonarsettings->tx_pulse_envelope = 0;
	sonarsettings->tx_pulse_envelope_par = 0.0;
	sonarsettings->tx_pulse_mode = 0;
	sonarsettings->max_ping_rate = 0.0;
	sonarsettings->ping_period = 0.0;
	sonarsettings->range_selection = 0.0;
	sonarsettings->power_selection = 0.0;
	sonarsettings->gain_selection = 0.0;
	sonarsettings->control_flags = 0;
	sonarsettings->projector_magic_no = 0;
	sonarsettings->steering_vertical = 0.0;
	sonarsettings->steering_horizontal = 0.0;
	sonarsettings->beamwidth_vertical = 0.0;
	sonarsettings->beamwidth_horizontal = 0.0;
	sonarsettings->focal_point = 0.0;
	sonarsettings->projector_weighting = 0;
	sonarsettings->projector_weighting_par = 0.0;
	sonarsettings->transmit_flags = 0;
	sonarsettings->hydrophone_magic_no = 0;
	sonarsettings->rx_weighting = 0;
	sonarsettings->rx_weighting_par = 0.0;
	sonarsettings->rx_flags = 0;
	sonarsettings->rx_width = 0.0;
	sonarsettings->range_minimum = 0.0;
	sonarsettings->range_maximum = 0.0;
	sonarsettings->depth_minimum = 0.0;
	sonarsettings->depth_maximum = 0.0;
	sonarsettings->absorption = 0.0;
	sonarsettings->sound_velocity = 0.0;
	sonarsettings->spreading = 0.0;
	sonarsettings->reserved = 0;

	/* Reson 7k configuration (record 7001) */
	configuration = &store->configuration;
	mbsys_reson7k_zero7kheader(verbose, &configuration->header, error);
	configuration->serial_number = 0;
	configuration->number_devices = 0;
	for (i = 0; i < MBSYS_RESON7K_MAX_DEVICE; i++) {
		configuration->device[i].magic_number = 0;
		for (j = 0; j < 60; j++)
			configuration->device[i].description[j] = '\0';
			configuration->device[i].alphadata_card = 0;
			configuration->device[i].serial_number = 0;
			configuration->device[i].info_length = 0;
			configuration->device[i].info_alloc = 0;
			configuration->device[i].info = NULL;
	}

	/* Reson 7k match filter (record 7002) */
	matchfilter = &store->matchfilter;
	mbsys_reson7k_zero7kheader(verbose, &matchfilter->header, error);
	matchfilter->serial_number = 0;
	matchfilter->ping_number = 0;
	matchfilter->operation = 0;
	matchfilter->start_frequency = 0.0;
	matchfilter->end_frequency = 0.0;
	matchfilter->window_type = 0;
	matchfilter->shading = 0.0;
	matchfilter->pulse_width = 0.0;
	for (i = 0; i < 13; i++)
		matchfilter->reserved[i] = '\0';

	/* Reson 7k firmware and hardware configuration (record 7003) */
	firmwarehardwareconfiguration = &store->firmwarehardwareconfiguration;
	mbsys_reson7k_zero7kheader(verbose, &firmwarehardwareconfiguration->header, error);
	firmwarehardwareconfiguration->device_count = 0;
	firmwarehardwareconfiguration->info_length = 0;
	firmwarehardwareconfiguration->info_alloc = 0;
	firmwarehardwareconfiguration->info = NULL;

	/* Reson 7k beam geometry (record 7004) */
	beamgeometry = &store->beamgeometry;
	mbsys_reson7k_zero7kheader(verbose, &beamgeometry->header, error);
	beamgeometry->serial_number = 0;
	beamgeometry->number_beams = 0;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
		beamgeometry->angle_alongtrack[i] = 0.0;
		beamgeometry->angle_acrosstrack[i] = 0.0;
		beamgeometry->beamwidth_alongtrack[i] = 0.0;
		beamgeometry->beamwidth_acrosstrack[i] = 0.0;
		tx_delay[i] = 0.0;
	}

	/* Reson 7k bathymetry (record 7006) */
	bathymetry = &store->bathymetry;
	mbsys_reson7k_zero7kheader(verbose, &bathymetry->header, error);
	bathymetry->serial_number = 0;
	bathymetry->ping_number = 0;
	bathymetry->multi_ping = 0;
	bathymetry->number_beams = 0;
	bathymetry->layer_comp_flag = 0;
	bathymetry->sound_vel_flag = 0;
	bathymetry->sound_velocity = 0.0;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
		bathymetry->range[i] = 0.0;
		bathymetry->quality[i] = 0;
		bathymetry->intensity[i] = 0.0;
		bathymetry->min_depth_gate[i] = 0.0;
		bathymetry->max_depth_gate[i] = 0.0;
	}
	bathymetry->optionaldata = MB_NO;
	bathymetry->frequency = 0.0;
	bathymetry->latitude = 0.0;
	bathymetry->longitude = 0.0;
	bathymetry->heading = 0.0;
	bathymetry->height_source = 0;
	bathymetry->tide = 0.0;
	bathymetry->roll = 0.0;
	bathymetry->pitch = 0.0;
	bathymetry->heave = 0.0;
	bathymetry->vehicle_depth = 0.0;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
		bathymetry->depth[i] = 0.0;
		bathymetry->alongtrack[i] = 0.0;
		bathymetry->acrosstrack[i] = 0.0;
		bathymetry->pointing_angle[i] = 0.0;
		bathymetry->azimuth_angle[i] = 0.0;
	}

	/* Reson 7k sidescan imagery data (record 7007) */
	sidescan = &store->sidescan;
	mbsys_reson7k_zero7kheader(verbose, &sidescan->header, error);
	sidescan->serial_number = 0;
	sidescan->ping_number = 0;
	sidescan->multi_ping = 0;
	sidescan->beam_position = 0.0;
	sidescan->control_flags = 0;
	sidescan->number_samples = 0;
	sidescan->nadir_depth = 0;
	for (i = 0; i < 7; i++)
		sidescan->reserved[i] = '\0';
	sidescan->number_beams = 0;
	sidescan->current_beam = 0;
	sidescan->sample_size = 0;
	sidescan->data_type = 0;
	sidescan->nalloc = 0;
	sidescan->port_data = NULL;
	sidescan->stbd_data = NULL;
	sidescan->optionaldata = MB_NO;
	sidescan->frequency = 0.0;
	sidescan->latitude = 0.0;
	sidescan->longitude = 0.0;
	sidescan->heading = 0.0;
	sidescan->altitude = 0.0;
	sidescan->depth = 0.0;

	/* Reson 7k Generic Water Column data (record 7008) */
	watercolumn = &store->watercolumn;
	mbsys_reson7k_zero7kheader(verbose, &watercolumn->header, error);
	watercolumn->serial_number = 0;
	watercolumn->ping_number = 0;
	watercolumn->multi_ping = 0;
	watercolumn->number_beams = 0;
	watercolumn->reserved = 0;
	watercolumn->samples = 0;
	watercolumn->subset_flag = 0;
	watercolumn->column_flag = 0;
	watercolumn->reserved2 = 0;
	watercolumn->sample_type = 0;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
		wcd = &v2watercolumn->wcd[i];
		wcd->n = 0;
		wcd->nalloc = 0;
		wcd->descriptor = NULL;
		wcd->first_sample = NULL;
		wcd->last_sample = NULL;
	}
	watercolumn->optionaldata = MB_NO;
	watercolumn->frequency = 0.0;
	watercolumn->latitude = 0.0;
	watercolumn->longitude = 0.0;
	watercolumn->heading = 0.0;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
		watercolumn->beam_alongtrack[i] = 0.0;
		watercolumn->beam_acrosstrack[i] = 0.0;
		watercolumn->center_sample[i] = 0;
	}

	/* Reson 7k vertical depth (record 7009) */
	verticaldepth = &store->verticaldepth;
	mbsys_reson7k_zero7kheader(verbose, &verticaldepth->header, error);
	verticaldepth->frequency = 0.0;
	verticaldepth->ping_number = 0;
	verticaldepth->multi_ping = 0;
	verticaldepth->latitude = 0.0;
	verticaldepth->longitude = 0.0;
	verticaldepth->heading = 0.0;
	verticaldepth->alongtrack = 0.0;
	verticaldepth->acrosstrack = 0.0;
	verticaldepth->vertical_depth = 0;

	/* Reson 7k tvg data (record 7010) */
	tvg = &store->tvg;
	mbsys_reson7k_zero7kheader(verbose, &tvg->header, error);
	tvg->serial_number = 0;
	tvg->ping_number = 0;
	tvg->multi_ping = 0;
	tvg->n = 0;
	for (i = 0; i < 8; i++)
		tvg->reserved[i] = 0;
	tvg->nalloc = 0;
	tvg->tvg = NULL;

	/* Reson 7k image data (record 7011) */
	image = &store->image;
	mbsys_reson7k_zero7kheader(verbose, &image->header, error);
	image->ping_number = 0;
	image->multi_ping = 0;
	image->width = 0;
	image->height = 0;
	image->color_depth = 0;
	image->reserved = 0;
	image->compression = 0;
	image->samples = 0;
	image->flag = 0;
	image->rx_delay = 0;
	for (i = 0; i < 6; i++)
		image->reserved2[i] = 0;
	image->nalloc = 0;
	image->image = NULL;

	/* Ping motion (record 7012) */
	pingmotion = &store->pingmotion;
	mbsys_reson7k_zero7kheader(verbose, &pingmotion->header, error);
	pingmotion->serial_number = 0;
	pingmotion->ping_number = 0;
	pingmotion->multi_ping = 0;
	pingmotion->n = 0;
	pingmotion->flags = 0;
	pingmotion->error_flags = 0;
	pingmotion->frequency = 0.0;
	pingmotion->nalloc = 0;
	pingmotion->pitch = 0.0;
	pingmotion->roll = NULL;
	pingmotion->heading = NULL;
	pingmotion->heave = NULL;
	
	/* Reson 7k Adaptive Gate (record 7014) */
	adaptivegate = &store->adaptivegate;
	mbsys_reson7k_zero7kheader(verbose, &adaptivegate->header, error);
	adaptivegate->record_size = 0;
	adaptivegate->serial_number = 0;
	adaptivegate->ping_number = 0;
	adaptivegate->multi_ping = 0;
	adaptivegate->n = 0;
	adaptivegate->gate_size = 0;
	adaptivegate->nalloc = 0;
	adaptivegate->angle = NULL;
	adaptivegate->min_limit = NULL;
	adaptivegate->max_limit = NULL;

	/* Detection setup (record 7017) */
	detectionsetup = &store->detectionsetup;
	mbsys_reson7k_zero7kheader(verbose, &detectionsetup->header, error);
	detectionsetup->serial_number = 0;
	detectionsetup->ping_number = 0;
	detectionsetup->multi_ping = 0;
	detectionsetup->number_beams = 0;
	detectionsetup->data_block_size = 0;
	detectionsetup->detection_algorithm = 0;
	detectionsetup->detection_flags = 0;
	detectionsetup->minimum_depth = 0.0;
	detectionsetup->maximum_depth = 0.0;
	detectionsetup->minimum_range = 0.0;
	detectionsetup->maximum_range = 0.0;
	detectionsetup->minimum_nadir_search = 0.0;
	detectionsetup->maximum_nadir_search = 0.0;
	detectionsetup->automatic_filter_window = 0.0;
	detectionsetup->applied_roll = 0.0;
	detectionsetup->depth_gate_tilt = 0.0;
	detectionsetup->nadir_depth = 0.0;
	for (i = 0; i < 13; i++)
		detectionsetup->reserved[i] = 0;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
		detectionsetup->beam_descriptor[i] = 0;
		detectionsetup->detection_point[i] = 0.0;
		detectionsetup->flags[i] = 0;
		detectionsetup->auto_limits_min_sample[i] = 0.0;
		detectionsetup->auto_limits_max_sample[i] = 0.0;
		detectionsetup->user_limits_min_sample[i] = 0.0;
		detectionsetup->user_limits_max_sample[i] = 0.0;
		detectionsetup->quality[i] = 0;
		detectionsetup->uncertainty[i] = 0.0;
	}

	/* Reson 7k Beamformed Data (record 7018) */
	beamformed = &store->beamformed;
	mbsys_reson7k_zero7kheader(verbose, &beamformed->header, error);
	beamformed->serial_number = 0;
	beamformed->ping_number = 0;
	beamformed->multi_ping = 0;
	beamformed->beams_number = 0;
	beamformed->samples_number = 0;
	for (i = 0; i < 8; i++)
		beamformed->reserved[i] = 0;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
		amplitudephase = &beamformed->amplitudephase[i];
		amplitudephase->beams_number = 0;
		amplitudephase->samples_number = 0;
		amplitudephase->nalloc = 0;
		amplitudephase->amplitude = NULL;
		amplitudephase->phase = NULL;
	}
	
	/* Reson 7k Vernier Processing Data Raw (record 7019) */
	vernierprocessingdataraw = &store->vernierprocessingdataraw;
	mbsys_reson7k_zero7kheader(verbose, &vernierprocessingdataraw->header, error);
	vernierprocessingdataraw->serial_number = 0;
	vernierprocessingdataraw->ping_number = 0;
	vernierprocessingdataraw->multi_ping = 0;
	vernierprocessingdataraw->reference_array = 0;
	vernierprocessingdataraw->pair1_array2 = 0;
	vernierprocessingdataraw->pair2_array2 = 0;
	vernierprocessingdataraw->decimator = 0;
	vernierprocessingdataraw->beam_number = 0;
	vernierprocessingdataraw->n = 0;
	vernierprocessingdataraw->decimated_samples = 0;
	vernierprocessingdataraw->first_sample = 0;
	for (i = 0; i < 2; i++)
		vernierprocessingdataraw->reserved[i] = 0;
	vernierprocessingdataraw->smoothing_type = 0;
	vernierprocessingdataraw->smoothing_length = 0;
	for (i = 0; i < 2; i++)
		vernierprocessingdataraw->reserved2[i] = 0;
	vernierprocessingdataraw->magnitude = 0.0;
	vernierprocessingdataraw->min_qf = 0.0;
	vernierprocessingdataraw->max_qf = 0.0;
	vernierprocessingdataraw->min_angle = 0.0;
	vernierprocessingdataraw->max_angle = 0.0;
	vernierprocessingdataraw->elevation_coverage = 0.0;
	for (i = 0; i < 4; i++)
		vernierprocessingdataraw->reserved3[i] = 0;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
		anglemagnitude = &vernierprocessingdataraw->anglemagnitude[i];
		anglemagnitude->beam_number = 0;
		anglemagnitude->n = 0;
		anglemagnitude->nalloc = 0;
		anglemagnitude->angle = NULL;
		anglemagnitude->magnitude = NULL;
		anglemagnitude->coherence = NULL;
		anglemagnitude->cross_power = NULL;
		anglemagnitude->quality_factor = NULL;
		anglemagnitude->reserved = NULL;
	}

	/* Reson 7k BITE (record 7021) */
	bite = &store->bite;
	mbsys_reson7k_zero7kheader(verbose, &bite->header, error);
	bite->n = 0;
	bite->nalloc = 0;
	for (i = 0; i < n; i++) {
		for (j = 0; j < 64; j++) {
			bite->source_name[j] = '\0';
		}
		bite->source_address[i] = 0;
		bite->reserved[i] = 0.0;
		bite->reserved2[i] = 0;
		for (j = 0; j < 10; j++) {
			bite->s7ktime.downlink_time[j] = '\0';
			bite->s7ktime.uplink_time[j] = '\0';
			bite->s7ktime.bite_time[j] = '\0';
		}
		bite->status[i] = 0;
		bite->number_bite[i] = 0;
		for (j = 0; j < 4; j++) {
			bite->bite_status[i] = 0;
		}
		for (j = 0; j < 256; j++) {
			bitereport->bitefield[j].field = 0;
			for (l = 0; l < 64; l++) {
				bitereport->name[k] = '\0';
			}
			bitereport->device_type[j] = 0;
			bitereport->minimum[j] = 0.0;
			bitereport->maximum[j] = 0.0;
			bitereport->value[j] = 0.0;
		}
	}

	/* Reson 7k center version (record 7022) */
	v37kcenterversion = &store->v37kcenterversion;
	mbsys_reson7k_zero7kheader(verbose, &v37kcenterversion->header, error);
	for (i = 0; i < 32; i++)
		v37kcenterversion->version[i] = '\0';

	/* Reson 7k 8k wet end version (record 7023) */
	v38kwetendversion = &store->v38kwetendversion;
	mbsys_reson7k_zero7kheader(verbose, &v38kwetendversion->header, error);
	for (i = 0; i < 32; i++)
		v38kwetendversion->version[i] = '\0';

	/* Reson 7k version 2 raw detection (record 7027) */
	rawdetection = &store->rawdetection;
	mbsys_reson7k_zero7kheader(verbose, &rawdetection->header, error);
	rawdetection->serial_number = 0;
	rawdetection->ping_number = 0;
	rawdetection->multi_ping = 0;
	rawdetection->number_beams = 0;
	rawdetection->data_field_size = 0;
	rawdetection->detection_algorithm = 0;
	rawdetection->flags = 0;
	rawdetection->sampling_rate = 0.0;
	rawdetection->tx_angle = 0.0;
	rawdetection->applied_roll = 0.0;
	for (i = 0; i < 15; i++)
		rawdetection->reserved[i] = 0;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
		rawdetectiondata = &rawdetection->rawdetectiondata[i];
		rawdetectiondata->beam_descriptor[i] = 0;
		rawdetectiondata->detection_point[i] = 0.0;
		rawdetectiondata->rx_angle[i] = 0.0;
		rawdetectiondata->flags[i] = 0;
		rawdetectiondata->quality[i] = 0;
		rawdetectiondata->uncertainty[i] = 0.0;
		rawdetectiondata->signal_strength[i] = 0.0;
		rawdetectiondata->min_limit[i] = 0.0;
		rawdetectiondata->max_limit[i] = 0.0;
	}
	rawdetection->optionaldata = MB_NO;
	rawdetection->frequency = 0.0;
	rawdetection->latitude = 0.0;
	rawdetection->longitude = 0.0;
	rawdetection->heading = 0.0;
	rawdetection->height_source = 0;
	rawdetection->tide = 0.0;
	rawdetection->roll = 0.0;
	rawdetection->pitch = 0.0;
	rawdetection->heave = 0.0;
	rawdetection->vehicle_depth = 0.0;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
		rawdetection->depth[i] = 0.0;
		rawdetection->alongtrack[i] = 0.0;
		rawdetection->acrosstrack[i] = 0.0;
		rawdetection->pointing_angle[i] = 0.0;
		rawdetection->azimuth_angle[i] = 0.0;
	}

	/* Reson 7k version 2 snippet (record 7028) */
	snippet = &store->snippet;
	mbsys_reson7k_zero7kheader(verbose, &snippet->header, error);
	snippet->serial_number = 0;
	snippet->ping_number = 0;
	snippet->multi_ping = 0;
	snippet->n = 0;
	snippet->error_flag = 0;
	snippet->control_flags = 0;
	snippet->flags = 0;
	for (i = 0; i < 6; i++)
		snippet->reserved[i] = 0;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
		snippettimeseries = &snippet->snippettimeseries[i];
		snippettimeseries->beam_number = 0;
		snippettimeseries->begin_sample = 0;
		snippettimeseries->detect_sample = 0;
		snippettimeseries->end_sample = 0;
		snippettimeseries->nalloc = 0;
		snippettimeseries->amplitude = NULL;
	}
	snippet->optionaldata = MB_NO;
	snippet->frequency = 0.0;
	snippet->latitude = 0.0;
	snippet->longitude = 0.0;
	snippet->heading = 0.0;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
		snippet->beam_alongtrack[i] = 0.0;
		snippet->beam_acrosstrack[i] = 0.0;
		snippet->center_sample[i] = 0.0;
	}
	
	/* Reson 7k vernier Processing Data Filtered (Record 7029) */
	vernierprocessingdatafiltered = &store->vernierprocessingdatafiltered;
	mbsys_reson7k_zero7kheader(verbose, &vernierprocessingdatafiltered->header, error);
	vernierprocessingdatafiltered->serial_number = 0;
	vernierprocessingdatafiltered->ping_number = 0;
	vernierprocessingdatafiltered->multi_ping = 0;
	vernierprocessingdatafiltered->number_soundings = 0;
	vernierprocessingdatafiltered->min_angle = 0.0;
	vernierprocessingdatafiltered->max_angle = 0.0;
	vernierprocessingdatafiltered->repeat_size = 0;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
		vernierprocessingdatasoundings = &vernierprocessingdatafiltered->vernierprocessingdatasoundings[i];
		vernierprocessingdatasoundings->beam_angle = 0.0;
		vernierprocessingdatasoundings->sample = 0;
		vernierprocessingdatasoundings->elevation = 0.0;
		vernierprocessingdatasoundings->reserved = 0.0;
	}

	/* Reson 7k sonar installation parameters (record 7030) */
	installation = &store->installation;
	mbsys_reson7k_zero7kheader(verbose, &installation->header, error);
	installation->frequency = 0.0;
	installation->firmware_version_len = 0;
	for (i = 0; i < 128; i++)
		installation->firmware_version[i] = 0;
	installation->software_version_len = 0;
	for (i = 0; i < 128; i++)
		installation->software_version[i] = 0;
	installation->s7k_version_len = 0;
	for (i = 0; i < 128; i++)
		installation->s7k_version[i] = 0;
	installation->protocal_version_len = 0;
	for (i = 0; i < 128; i++)
		installation->protocal_version[i] = 0;
	installation->transmit_x = 0.0;
	installation->transmit_y = 0.0;
	installation->transmit_z = 0.0;
	installation->transmit_roll = 0.0;
	installation->transmit_pitch = 0.0;
	installation->transmit_heading = 0.0;
	installation->receive_x = 0.0;
	installation->receive_y = 0.0;
	installation->receive_z = 0.0;
	installation->receive_roll = 0.0;
	installation->receive_pitch = 0.0;
	installation->receive_heading = 0.0;
	installation->motion_x = 0.0;
	installation->motion_y = 0.0;
	installation->motion_z = 0.0;
	installation->motion_roll = 0.0;
	installation->motion_pitch = 0.0;
	installation->motion_heading = 0.0;
	installation->motion_time_delay = 0;
	installation->position_x = 0.0;
	installation->position_y = 0.0;
	installation->position_z = 0.0;
	installation->position_time_delay = 0;
	installation->waterline_z = 0.0;

	/* Reson 7k BITE summary (Record 7031) */
	bitesummary = &store->bitesummary;
	mbsys_reson7k_zero7kheader(verbose, &bitesummary->header, error);
	bitesummary->total_items = 0;
	for (i = 0; i < 4; i++)
		bitesummary->warnings[i] = 0;
	for (i = 0; i < 4; i++)
		bitesummary->errors[i] = 0;
	for (i = 0; i < 4; i++)
		bitesummary->fatals[i] = 0;
	for (i = 0; i < 2; i++)
		bitesummary->reserved[i] = 0;

	/* Reson 7k Compressed Beamformed Magnitude Data (Record 7041) */
	compressedbeamformedmagnitude = &store->compressedbeamformedmagnitude;
	mbsys_reson7k_zero7kheader(verbose, &compressedbeamformedmagnitude->header, error);
	compressedbeamformedmagnitude->serial_number = 0;
	compressedbeamformedmagnitude->ping_number = 0;
	compressedbeamformedmagnitude->multi_ping = 0;
	compressedbeamformedmagnitude->number_beams = 0;
	compressedbeamformedmagnitude->flags = 0;
	compressedbeamformedmagnitude->sample_rate = 0.0;
	compressedbeamformedmagnitude->reserved = 0;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
		beamformedmagnitude = &compressedbeamformedmagnitude->beamformedmagnitude[i];
		beamformedmagnitude->beam = 0;
		beamformedmagnitude->samples = 0;
		beamformedmagnitude->nalloc = 0;
		beamformedmagnitude->data = NULL;
	}
	
	/* Reson 7k Compressed Water Column Data (Record 7042) */
	compressedwatercolumn = &store->compressedwatercolumn;
	mbsys_reson7k_zero7kheader(verbose, &compressedwatercolumn->header, error);
	compressedwatercolumn->serial_number = 0;
	compressedwatercolumn->ping_number = 0;
	compressedwatercolumn->multi_ping = 0;
	compressedwatercolumn->number_beams = 0;
	compressedwatercolumn->samples = 0;
	compressedwatercolumn->compressed_samples = 0;
	compressedwatercolumn->flags = 0;
	compressedwatercolumn->first_sample = 0;
	compressedwatercolumn->sample_rate = 0.0;
	compressedwatercolumn->compression_factor = 0.0;
	compressedwatercolumn->reserved = 0;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
		compressedwatercolumndata = &compressedwatercolumn->compressedwatercolumndata[i];
		compressedwatercolumndata->beam_number = 0;
		compressedwatercolumndata->segment_number = 0;
		compressedwatercolumndata->samples = 0;
		compressedwatercolumndata->nalloc = 0;
		compressedwatercolumndata->sample = NULL;
	}
	
	/* Reson 7k Segmented Raw Detection Data (Record 7047) */
	segmentedrawdetection = &store->segmentedrawdetection;
	mbsys_reson7k_zero7kheader(verbose, &segmentedrawdetection->header, error);
	segmentedrawdetection->record_header_size = 0;
	segmentedrawdetection->n_segments = 0;
	segmentedrawdetection->segment_field_size = 0;
	segmentedrawdetection->r_rx = 0;
	segmentedrawdetection->rx_field_size = 0;
	segmentedrawdetection->serial_number = 0;
	segmentedrawdetection->ping_number = 0;
	segmentedrawdetection->multi_ping = 0;
	segmentedrawdetection->sound_velocity = 0.0;
	segmentedrawdetection->rx_delay = 0.0;
	for (i = 0; i < n_segments; i++) {
		segmentedrawdetectiondata = &segmentedrawdetection->segmentedrawdetectiondata[i];
		segmentedrawdetectiondata->segment_number = 0;
		segmentedrawdetectiondata->tx_angle_along = 0.0;
		segmentedrawdetectiondata->tx_angle_across = 0.0;
		segmentedrawdetectiondata->tx_delay = 0.0;
		segmentedrawdetectiondata->frequency = 0.0;
		segmentedrawdetectiondata->pulse_type = 0;
		segmentedrawdetectiondata->pulse_bandwidth = 0.0;
		segmentedrawdetectiondata->tx_pulse_width = 0.0;
		segmentedrawdetectiondata->tx_pulse_width_across = 0.0;
		segmentedrawdetectiondata->tx_pulse_width_along = 0.0;
		segmentedrawdetectiondata->tx_pulse_envelope = 0;
		segmentedrawdetectiondata->tx_pulse_envelope_parameter = 0.0;
		segmentedrawdetectiondata->tx_relative_src_level = 0.0;
		segmentedrawdetectiondata->rx_beam_width = 0.0;
		segmentedrawdetectiondata->detection_algorithm = 0;
		segmentedrawdetectiondata->flags = 0;
		segmentedrawdetectiondata->sampling_rate = 0.0;
		segmentedrawdetectiondata->tvg = 0;
		segmentedrawdetectiondata->rx_bandwidth = 0.0;
	}
	for (i = 0; i < r_rx; i++) {
		segmentedrawdetectiondata = &segmentedrawdetection->segmentedrawdetectiondata[i];
		segmentedrawdetectiondata->beam_number = 0;
		segmentedrawdetectiondata->used_segment = 0;
		segmentedrawdetectiondata->detection_point = 0.0;
		segmentedrawdetectiondata->rx_angle_across = 0.0;
		segmentedrawdetectiondata->flags2 = 0;
		segmentedrawdetectiondata->quality = 0;
		segmentedrawdetectiondata->uncertainty = 0.0;
		segmentedrawdetectiondata->signal_strength = 0.0;
		segmentedrawdetectiondata->sn_ratio = 0.0;
	}
	
	/* Reson 7k Calibrated Beam Data (Record 7048) */
	calibratedbeam = &store->calibratedbeam;
	mbsys_reson7k_zero7kheader(verbose, &calibratedbeam->header, error);
	calibratedbeam->serial_number = 0;
	calibratedbeam->ping_number = 0;
	calibratedbeam->multi_ping = 0;
	calibratedbeam->first_beam = 0;
	calibratedbeam->total_beams = 0;
	calibratedbeam->total_samples = 0;
	calibratedbeam->foward_looking_sonar = 0;
	calibratedbeam->error_flag = 0;
	for (i = 0; i < 8; i++)
		calibratedbeam->reserved = 0;
	calibratedbeam->sample = NULL;
	
	/* Reson 7k Reserved (Record 7049) */

	/* Reson 7k System Events (record 7050) */
	systemevents = &store->systemevents;
	mbsys_reson7k_zero7kheader(verbose, &systemevents->header, error);
	systemevents->serial_number = 0;
	systemevents->number_events = 0;
	for (i = 0; i < number_events; i++) {
		systemeventsdata = &systemevents->systemeventsdata[i];
		systemeventsdata->event_type[i] = 0;
		systemeventsdata->event_id[i] = 0;
		systemeventsdata->device_id[i] = 0;
		systemeventsdata->system_enum[i] = 0;
		systemeventsdata->event_message_length[i] = 0;
		systemeventsdata->7ktime[i] = '\0';
		systemeventsdata->event_message[i] = NULL;
	}

	/* Reson 7k System Event (record 7051) */
	systemeventmessage = &store->systemeventmessage;
	mbsys_reson7k_zero7kheader(verbose, &systemeventmessage->header, error);
	systemeventmessage->serial_number = 0;
	systemeventmessage->event_id = 0;
	systemeventmessage->message_length = 0;
	systemeventmessage->event_identifier = 0;
	systemeventmessage->message_alloc = 0;
	systemeventmessage->message = NULL;
	
	/* Reson 7k RDR Recording Status (part of Record 7052) */
	rdrrecordingstatus = &store->rdrrecordingstatus;
	mbsys_reson7k_zero7kheader(verbose, &rdrrecordingstatus->header, error);
	rdrrecordingstatus->position = 0;
	rdrrecordingstatus->disk_free = 0;
	rdrrecordingstatus->mode = 0;
	rdrrecordingstatus->filerecords = 0;
	rdrrecordingstatus->filesize = 0;
	for (i = 0; i < 10; i++)
		rdrrecordingstatus->first_7ktime[i] = 0;
	for (i = 0; i < 10; i++)
		rdrrecordingstatus->last_7ktime[i] = 0;
	rdrrecordingstatus->totaltime = 0;
	for (i = 0; i < 256; i++)
		rdrrecordingstatus->directory_name = '\0';
	for (i = 0; i < 256; i++)
		rdrrecordingstatus->filename[i] = '\0';
	rdrrecordingstatus->error = 0;
	rdrrecordingstatus->flags = 0;
	rdrrecordingstatus->logger_address = 0;
	rdrrecordingstatus->file_number = 0;
	rdrrecordingstatus->ping_number = 0;
	rdrrecordingstatus->reserved = 0;
	for (i = 0; i < 4; i++)
		rdrrecordingstatus->reserved2[i] = 0;
	rdrrecordingstatus->???
	
	/* Reson 7k Subscriptions (part of Record 7053) */
	subscriptions = &store->subscriptions;
	mbsys_reson7k_zero7kheader(verbose, &subscriptions->header, error);
	subscriptions->n_subscriptions = 0;
	subscriptions->address = 0;
	subscriptions->port = 0;
	subscriptions->type = 0;
	subscriptions->records_number = 0;
	for (i = 0; i < 64; i++)
		subscriptions->record_list[i] = 0;
	for (i = 0; i < 128; i++)
		subscriptions->reserved[i] = 0;
	
	/* Reson 7k RDR Storage Recording (Record 7054) */
	rdrstoragerecording = &store->rdrstoragerecording;
	mbsys_reson7k_zero7kheader(verbose, &rdrstoragerecording->header, error);
	rdrstoragerecording->diskfree_percentage = 0;
	rdrstoragerecording->number_records = 0;
	rdrstoragerecording->size = 0;
	for (i = 0; i < 4; i++)
		rdrstoragerecording->reserved[i] = 0;
	rdrstoragerecording->mode = 0;
	for (i = 0; i < 256; i++)
		rdrstoragerecording->file_name[i] = '\0';
	rdrstoragerecording->RDR_error = 0;
	rdrstoragerecording->data_rate = 0;
	rdrstoragerecording->minutes_left = 0;
	
	/* Reson 7k Calibration Status (Record 7055) */
	calibrationstatus = &store->calibrationstatus;
	mbsys_reson7k_zero7kheader(verbose, &calibrationstatus->header, error);
	calibrationstatus->serial_number = 0;
	calibrationstatus->calibration_status = 0;
	calibrationstatus->percent_complete = 0;
	for (i = 0; i < 10; i++)
		calibrationstatus->calibration_time[i] = 0;
	for (i = 0; i < 800; i++)
		calibrationstatus->status_message[i] = '\0';
	calibrationstatus->sub_status = 0;
	calibrationstatus->optional_data = MB_NO;
	calibrationstatus->system_calibration = 0;
	calibrationstatus->done_calibration = 0;
	calibrationstatus->current_calibration = 0;
	calibrationstatus->startup_calibration = 0;
	for (i = 0; i < 8; i++)
		calibrationstatus->status[i] = 0;
	for (i = 0; i < 2; i++)
		calibrationstatus->reserved[i] = 0;
	
	/* Reson 7k Calibrated Sidescan Data (record 7057) */
	calibratedsidescan = &store->calibratedsidescan;
	mbsys_reson7k_zero7kheader(verbose, &calibratedsidescan->header, error);
	calibratedsidescan->serial_number = 0;
	calibratedsidescan->ping_number = 0;
	calibratedsidescan->multi_ping = 0;
	calibratedsidescan->beam_position = 0.0;
	calibratedsidescan->reserved = 0;
	calibratedsidescan->samples = 0;
	calibratedsidescan->reserved2 = 0.0;
	calibratedsidescan->beams = 0;
	calibratedsidescan->current_beam = 0;
	calibratedsidescan->bytes_persample = 0;
	calibratedsidescan->data_types = 0;
	calibratedsidescan->error_flag = 0;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
		calibratedsidescanseries = &calibratedsidescan->calibratedsidescanseries[i];
		calibratedsidescanseries->nalloc = 0;
		calibratedsidescanseries->portbeams = NULL;
		calibratedsidescanseries->starboardbeams = NULL;
		calibratedsidescanseries->portbeams_number = NULL;
		calibratedsidescanseries->starboardbeams_number = NULL;
	}
	calibratedsidescan->optionaldata = MB_NO;
	calibratedsidescan->frequency = 0.0;
	calibratedsidescan->latitude = 0.0;
	calibratedsidescan->longitude = 0.0;
	calibratedsidescan->heading = 0.0;
	calibratedsidescan->depth = 0.0;
	
	/* Reson 7k Snippet Backscattering Strength (record 7058) */
	snippetbackscatteringstrength = &store->snippetbackscatteringstrength;
	mbsys_reson7k_zero7kheader(verbose, &snippetbackscatteringstrength->header, error);
	snippetbackscatteringstrength->serial_number = 0;
	snippetbackscatteringstrength->ping_number = 0;
	snippetbackscatteringstrength->multi_ping = 0;
	snippetbackscatteringstrength->number_beams = 0;
	snippetbackscatteringstrength->error_flag = 0;
	snippetbackscatteringstrength->control_flags = 0;
	snippetbackscatteringstrength->absorption = 0;
	for (i = 0; i < 6; i++)
		snippetbackscatteringstrength->reserved[i] = 0;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
		snippetbackscatteringstrengthdata = &snippetbackscatteringstrength->snippetbackscatteringstrengthdata[i];
		snippetbackscatteringstrengthdata->beam_number = 0;
		snippetbackscatteringstrengthdata->begin_sample = 0;
		snippetbackscatteringstrengthdata->bottom_sample = 0;
		snippetbackscatteringstrengthdata->end_sample = 0;
		snippetbackscatteringstrengthdata->nalloc = 0;
		snippetbackscatteringstrengthdata->bs = NULL;
		snippetbackscatteringstrengthdata->amplitude = NULL;
	}
	snippetbackscatteringstrength->optionaldata = MB_NO;
	snippetbackscatteringstrength->frequency = 0.0;
	snippetbackscatteringstrength->latitude = 0.0;
	snippetbackscatteringstrength->longitude = 0.0;
	snippetbackscatteringstrength->heading = 0
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++)
		snippetbackscatteringstrength->beam_alongtrack[i] = 0.0;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++)
		snippetbackscatteringstrength->beam_acrosstrack[i] = 0.0;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++)
		snippetbackscatteringstrength->center_sample[i] = 0;
	
	/* Reson 7k MB2 Specific Status (Record 7059) */
	mb2status = &store->mb2status;
	mbsys_reson7k_zero7kheader(verbose, &mb2status->header, error);
	for (i = 0; i < 256; i++)
		mb2status->directory[i] = '\0';
	for (i = 0; i < 256; i++)
		mb2status->header_name[i] = '\0';
	for (i = 0; i < 256; i++)
		mb2status->trailer_name[i] = '\0';
	mb2status->prepend_header = 0;
	mb2status->append_trailer = 0;
	mb2status->storage = 0;
	for (i = 0; i < 256; i++)
		mb2status->playback_path[i] = '\0';
	for (i = 0; i < 256; i++)
		mb2status->playback_file[i] = '\0';
	mb2status->playback_loopmode = 0;
	mb2status->playback = 0;
	for (i = 0; i < 256; i++)
		mb2status->rrio_address1[i] = '\0';
	for (i = 0; i < 256; i++)
		mb2status->rrio_address2[i] = '\0';
	for (i = 0; i < 256; i++)
		mb2status->rrio_address3[i] = '\0';
	mb2status->build_hpr = 0;
	mb2status->attached_hpr = 0;
	mb2status->stacking = 0;
	mb2status->stacking_value = 0;
	mb2status->zda_baudrate = 0;
	mb2status->zda_parity = 0;
	mb2status->zda_databits = 0;
	mb2status->zda_stopbits = 0;
	mb2status->gga_baudrate = 0;
	mb2status->gga_parity = 0;
	mb2status->gga_databits = 0;
	mb2status->gga_stopbits = 0;
	mb2status->svp_baudrate = 0;
	mb2status->svp_parity = 0;
	mb2status->svp_databits = 0;
	mb2status->svp_stopbits = 0;
	mb2status->hpr_baudrate = 0;
	mb2status->hpr_parity = 0;
	mb2status->hpr_databits = 0;
	mb2status->hpr_stopbits = 0;
	mb2status->hdt_baudrate = 0;
	mb2status->hdt_parity = 0;
	mb2status->hdt_databits = 0;
	mb2status->hdt_stopbits = 0;
	mb2status->rrio = 0;
	mb2status->playback_timestamps = 0;
	mb2status->reserved = 0;
	mb2status->reserved2 = 0;
	
	/* Reson 7k File Header (Record 7200) */
	fileheader = &store->fileheader;
	mbsys_reson7k_zero7kheader(verbose, &fileheader->header, error);
	for (i = 0; i < 2; i++)
		fileheader->file_identifier[i] = 0;
	fileheader->version = 0;
	fileheader->reserved = 0;
	for (i = 0; i < 2; i++)
		fileheader->session_identifier[i] = 0;
	fileheader->record_data_size = 0;
	fileheader->number_subsystems = 0;
	for (i = 0; i < 64; i++)
		fileheader->recording_name[i] = '\0';
	for (i = 0; i < 16; i++)
		fileheader->recording_version[i] = '\0';
	for (i = 0; i < 64; i++)
		fileheader->user_defined_name[i] = '\0';
	for (i = 0; i < 128; i++)
		fileheader->notes[i] = '\0';
	for (j = 0; j < MBSYS_RESON7K_MAX_DEVICE; j++) {
		fileheader->subsystem[j].device_identifier = 0;
		fileheader->subsystem[j].system_enumerator = 0;
	}
	
	/* Reson 7k File Catalog Record (Record 7300) */
	filecatalogrecord = &store->filecatalogrecord;
	mbsys_reson7k_zero7kheader(verbose, &filecatalogrecord->header, error);
	filecatalogrecord->size = 0;
	filecatalogrecord->version = 0;
	filecatalogrecord->n = 0;
	filecatalogrecord->nalloc = 0;
	filecatalogrecord->reserved = 0;
	for (i = 0; i < n; i++) {
		filecatalogrecord->filecatalogrecorddata[i].size = 0;
		filecatalogrecord->filecatalogrecorddata[i].offset = 0;
		filecatalogrecord->filecatalogrecorddata[i].record_type = 0;
		filecatalogrecord->filecatalogrecorddata[i].device_id = 0;
		filecatalogrecord->filecatalogrecorddata[i].system_enumerator = 0;
		for (j = 0; j < 10; j++)
			filecatalogrecord->filecatalogrecorddata[i].s7ktime[j] = 0;
		filecatalogrecord->filecatalogrecorddata[j].record_count = 0;
		for (j = 0; j < 8; j++)
			filecatalogrecord->filecatalogrecorddata[i].reserved[j] = 0;
	}

	/* Reson 7k Time Message (Record 7400) */
	timemessage = &store->timemessage;
	mbsys_reson7k_zero7kheader(verbose, &timemessage->header, error);
	timemessage->second_offset = 0;
	timemessage->pulse_flag = 0;
	timemessage->port_id = 0;
	timemessage->reserved = 0;
	timemessage->reserved2 = 0;
	timemessage->optionaldata = MB_NO;
	timemessage->utctime = 0.0;
	timemessage->external_time = 0.0;
	timemessage->t0 = 0.0;
	timemessage->t1 = 0.0;
	timemessage->pulse_length = 0.0;
	timemessage->difference = 0.0;
	timemessage->io_status = 0;
	
	/* Reson 7k Remote Control (Record 7500) */
	remotecontrol = &store->remotecontrol;
	mbsys_reson7k_zero7kheader(verbose, &remotecontrol->header, error);
	remotecontrol->remote_id = 0;
	remotecontrol->ticket = 0;
	for (i = 0; i < 2; i++)
		remotecontrol->tracking_n[i] = 0;
	
	/* Reson 7k Remote Control Acknowledge (Record 7501) */
	remotecontrolacknowledge = &store->remotecontrolacknowledge;
	mbsys_reson7k_zero7kheader(verbose, &remotecontrolacknowledge->header, error);
	remotecontrolacknowledge->ticket = 0;
	for (i = 0; i < 2; i++)
		remotecontrolacknowledge->tracking_n[i] = 0;

	/* Reson 7k Remote Control Not Acknowledge (Record 7502) */
	remotecontrolnotacknowledge = &store->remotecontrolnotacknowledge;
	mbsys_reson7k_zero7kheader(verbose, &remotecontrolnotacknowledge->header, error);
	remotecontrolnotacknowledge->ticket = 0;
	for (i = 0; i < 2; i++)
		remotecontrolnotacknowledge->tracking_n[i] = 0;
	remotecontrolnotacknowledge->error_code = 0;
	
	/* Reson 7k remote control sonar settings (record 7503) */
	remotecontrolsettings = &store->remotecontrolsettings;
	mbsys_reson7k_zero7kheader(verbose, &remotecontrolsettings->header, error);
	remotecontrolsettings->serial_number = 0;
	remotecontrolsettings->ping_number = 0;
	remotecontrolsettings->frequency = 0.0;
	remotecontrolsettings->sample_rate = 0.0;
	remotecontrolsettings->receiver_bandwidth = 0.0;
	remotecontrolsettings->pulse_width = 0.0;
	remotecontrolsettings->pulse_type = 0;
	remotecontrolsettings->pulse_envelope = 0;
	remotecontrolsettings->pulse_envelope_par = 0.0;
	remotecontrolsettings->pulse_mode = 0;
	remotecontrolsettings->pulse_reserved = 0;
	remotecontrolsettings->max_ping_rate = 0.0;
	remotecontrolsettings->ping_period = 0.0;
	remotecontrolsettings->range_selection = 0.0;
	remotecontrolsettings->power_selection = 0.0;
	remotecontrolsettings->gain_selection = 0.0;
	remotecontrolsettings->control_flags = 0;
	remotecontrolsettings->projector_id = 0;
	remotecontrolsettings->steering_vertical = 0.0;
	remotecontrolsettings->steering_horizontal = 0.0;
	remotecontrolsettings->beamwidth_vertical = 0.0;
	remotecontrolsettings->beamwidth_horizontal = 0.0;
	remotecontrolsettings->focal_point = 0.0;
	remotecontrolsettings->projector_weighting = 0;
	remotecontrolsettings->projector_weighting_par = 0.0;
	remotecontrolsettings->hydrophone_id = 0;
	remotecontrolsettings->receive_weighting = 0;
	remotecontrolsettings->receive_weighting_par = 0.0;
	remotecontrolsettings->receive_flags = 0;
	remotecontrolsettings->range_minimum = 0.0;
	remotecontrolsettings->range_maximum = 0.0;
	remotecontrolsettings->depth_minimum = 0.0;
	remotecontrolsettings->depth_maximum = 0.0;
	remotecontrolsettings->absorption = 0.0;
	remotecontrolsettings->sound_velocity = 0.0;
	remotecontrolsettings->spreading = 0.0;
	remotecontrolsettings->operation_mode = 0;
	remotecontrolsettings->autofilter_window = 0;
	remotecontrolsettings->tx_offset_x = 0.0;
	remotecontrolsettings->tx_offset_y = 0.0;
	remotecontrolsettings->tx_offset_z = 0.0;
	remotecontrolsettings->head_tilt_x = 0.0;
	remotecontrolsettings->head_tilt_y = 0.0;
	remotecontrolsettings->head_tilt_z = 0.0;
	remotecontrolsettings->ping_state = 0;
	remotecontrolsettings->beam_angle_mode = 0;
	remotecontrolsettings->s7kcenter_mode = 0;
	remotecontrolsettings->gate_depth_min = 0.0;
	remotecontrolsettings->gate_depth_max = 0.0;
	remotecontrolsettings->trigger_width = 0.0;
	remotecontrolsettings->trigger_offset = 0.0;
	remotecontrolsettings->projector_selection = 0;
	for (i = 0; i < 2; i++)
		remotecontrolsettings->reserved2[i] = 0;
	remotecontrolsettings->alternate_gain = 0.0;
	remotecontrolsettings->vernier_filter = 0.0;
	remotecontrolsettings->reserved3 = 0;
	remotecontrolsettings->custom_beams = 0;
	remotecontrolsettings->coverage_angle = 0;
	remotecontrolsettings->coverage_mode = 0;
	remotecontrolsettings->quality_filter = 0;
	remotecontrolsettings->received_steering = 0.0;
	remotecontrolsettings->flexmode_coverage = 0.0;
	remotecontrolsettings->flexmode_steering = 0.0;
	remotecontrolsettings->constant_spacing = 0.0;
	remotecontrolsettings->beam_mode = 0;
	remotecontrolsettings->depth_gate_tilt = 0.0;
	remotecontrolsettings->applied_frequency = 0.0;
	remotecontrolsettings->element_number = 0;
	
	/* Reson 7k Common System Settings (Record 7504) */
	commonsystemsettings = &store->commonsystemsettings;
	mbsys_reson7k_zero7kheader(verbose, &commonsystemsettings->header, error);
	commonsystemsettings->serial_number = 0;
	commonsystemsettings->ping_number = 0;
	commonsystemsettings->sound_velocity = 0.0;
	commonsystemsettings->absorption = 0.0;
	commonsystemsettings->spreading_loss = 0.0;
	commonsystemsettings->sequencer_control = 0;
	commonsystemsettings->mru_format = 0;
	commonsystemsettings->mru_baudrate = 0;
	commonsystemsettings->mru_parity = 0;
	commonsystemsettings->mru_databits = 0;
	commonsystemsettings->mru_stopbits = 0;
	commonsystemsettings->orientation = 0;
	commonsystemsettings->record_version = 0;
	commonsystemsettings->motion_latency = 0.0;
	commonsystemsettings->svp_filter = 0;
	commonsystemsettings->sv_override = 0;
	commonsystemsettings->activeenum = 0;
	commonsystemsettings->active_id = 0;
	commonsystemsettings->system_mode = 0;
	commonsystemsettings->master_slave = 0;
	commonsystemsettings->tracker_flags = 0;
	commonsystemsettings->tracker_swathwidth = 0.0;
	commonsystemsettings->multidetect_enable = 0;
	commonsystemsettings->multidetect_obsize = 0;
	commonsystemsettings->multidetect_sensitivity = 0;
	commonsystemsettings->multidetect_detections = 0;
	for (i = 0; i < 2; i++)
		commonsystemsettings->multidetect_reserved[i] = 0;
	for (i = 0; i < 4; i++)
		commonsystemsettings->slave_ip[i] = 0;
	commonsystemsettings->snippet_controlflags = 0;
	commonsystemsettings->snippet_minwindow = 0;
	commonsystemsettings->snippet_maxwindow = 0;
	commonsystemsettings->fullrange_dualhead = 0;
	commonsystemsettings->delay_multiplier = 0.0;
	commonsystemsettings->powersaving_mode = 0;
	commonsystemsettings->flags = 0;
	commonsystemsettings->range_blank = 0;
	commonsystemsettings->startup_normalization = 0;
	commonsystemsettings->restore_pingrate = 0;
	commonsystemsettings->restore_power = 0;
	commonsystemsettings->sv_interlock = 0;
	commonsystemsettings->ignorepps_errors = 0;
	for (i = 0; i < 15; i++)
		commonsystemsettings->reserved1[i] = 0;
	commonsystemsettings->compressed_wcflags = 0;
	commonsystemsettings->deckmode = 0;
	commonsystemsettings->reserved2 = 0;
	commonsystemsettings->powermode_max = 0;
	commonsystemsettings->water_temperature = 0.0;
	commonsystemsettings->sensor_override = 0;
	commonsystemsettings->sensor_dataflags = 0;
	commonsystemsettings->sensor_active = 0;
	commonsystemsettings->reserved3 = 0;
	commonsystemsettings->tracker_maxcoverage = 0.0;
	commonsystemsettings->dutycycle_mode = 0;
	commonsystemsettings->reserved4 = 0;
	for (i = 0; i < 99; i++)
		commonsystemsettings->reserved5[i] = 0;
	
	/* Reson 7k SV Filtering (record 7510) */
	svfiltering = &store->svfiltering;
	mbsys_reson7k_zero7kheader(verbose, &svfiltering->header, error);
	svfiltering->sensor_sv = 0.0;
	svfiltering->filtered_sv = 0.0;
	svfiltering->filter = 0;
	
	/* Reson 7k System Lock Status (record 7511) */
	systemlockstatus = &store->systemlockstatus;
	mbsys_reson7k_zero7kheader(verbose, &systemlockstatus->header, error);
	systemlockstatus->systemlock = 0;
	systemlockstatus->client_ip = 0;
	for (i = 0; i < 8; i++)
		systemlockstatus->reserved[i] = 0;
	
	/* Reson 7k Sound Velocity (record 7610) */
	soundvelocity; = &store->soundvelocity;;
	mbsys_reson7k_zero7kheader(verbose, &soundvelocity;->header, error);
	soundvelocity->soundvelocity = 0.0;
	soundvelocity->optionaldata = 0;
	soundvelocity->temperature = 0.0;
	soundvelocity->pressure = 0.0;
	
	/* Reson 7k Absorption Loss (record 7611) */
	absorptionloss; = &store->absorptionloss;;
	mbsys_reson7k_zero7kheader(verbose, &absorptionloss;->header, error);
	absorptionloss->absorptionloss = 0.0;
	
	/* Reson 7k Spreading Loss (record 7612) */
	spreadingloss = &store->spreadingloss;
	mbsys_reson7k_zero7kheader(verbose, &spreadingloss->header, error);
	spreadingloss->spreadingloss = 0.0;
	
	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	char *function_name = "mbsys_reson7k_deall";
	int status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7kr_customattitude *customattitude;
	s7kr_motion *motion;
	s7kr_svp *svp;
	s7kr_ctd *ctd;
	s7kr_surveyline *surveyline;
	s7kr_attitude *attitude;
	// s7kr_sonarpipeenvironment *sonarpipeenvironment;
	s7kr_configuration *configuration;
	s7kr_firmwarehardwareconfiguration *firmwarehardwareconfiguration;
	s7kr_sidescan *sidescan;
	s7kr_watercolumn *watercolumn;
	s7kr_tvg *tvg;
	s7kr_image *image;
	s7kr_pingmotion *pingmotion;
	// s7kr_adaptivegate *adaptivegate;
	s7kr_beamformed *beamformed;
	// s7kr_vernierprocessingdataraw *vernierprocessingdataraw;
	s7kr_bite *bite;
	s7kr_rawdetection *rawdetection;
	s7kr_snippet *snippet;
	// s7kr_compressedbeamformedmagnitude *compressedbeamformedmagnitude;
	s7kr_compressedwatercolumn *compressedwatercolumn;
	s7kr_segmentedrawdetection segmentedrawdetection;
	// s7kr_calibratedbeam *calibratedbeam;
	// s7kr_systemevents *systemevents;
	s7kr_systemeventmessage *systemeventmessage;
	// s7kr_rdrrecordingstatusdata *rdrrecordingstatusdata;
	s7kr_calibratedsidescan *calibratedsidescan;
	s7kr_snippetbackscatteringstrength *snippetbackscatteringstrength;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
	}

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *)*store_ptr;

	/* Custom attitude (record 1004) */
	customattitude = &store->customattitude;
	customattitude->n = 0;
	customattitude->nalloc = 0;
	if (customattitude->pitch != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(customattitude->pitch), error);
	if (customattitude->roll != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(customattitude->roll), error);
	if (customattitude->heading != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(customattitude->heading), error);
	if (customattitude->heave != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(customattitude->heave), error);
	if (customattitude->pitchrate != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(customattitude->pitchrate), error);
	if (customattitude->rollrate != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(customattitude->rollrate), error);
	if (customattitude->headingrate != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(customattitude->headingrate), error);
	if (customattitude->heaverate != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(customattitude->heaverate), error);

	/* Motion over ground (record 1007) */
	motion = &store->motion;
	motion->n = 0;
	motion->nalloc = 0;
	if (motion->x != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(motion->x), error);
	if (motion->y != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(motion->y), error);
	if (motion->z != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(motion->z), error);
	if (motion->xa != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(motion->xa), error);
	if (motion->ya != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(motion->ya), error);
	if (motion->za != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(motion->za), error);

	/* Sound velocity profile (record 1009) */
	svp = &store->svp;
	svp->n = 0;
	svp->nalloc = 0;
	if (svp->depth != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(svp->depth), error);
	if (svp->sound_velocity != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(svp->sound_velocity), error);

	/* CTD (record 1010) */
	ctd = &store->ctd;
	ctd->n = 0;
	ctd->nalloc = 0;
	if (ctd->conductivity_salinity != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(ctd->conductivity_salinity), error);
	if (ctd->temperature != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(ctd->temperature), error);
	if (ctd->pressure_depth != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(ctd->pressure_depth), error);
	if (ctd->sound_velocity != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(ctd->sound_velocity), error);
	if (ctd->absorption != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(ctd->absorption), error);

	/* Survey Line (record 1014) */
	surveyline = &store->surveyline;
	surveyline->n = 0;
	surveyline->nalloc = 0;
	if (surveyline->latitude_northing != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(surveyline->latitude_northing), error);
	if (surveyline->longitude_easting != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(surveyline->longitude_easting), error);

	/* Attitude (record 1016) */
	attitude = &store->attitude;
	attitude->n = 0;
	attitude->nalloc = 0;
	if (attitude->delta_time != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(attitude->delta_time), error);
	if (attitude->roll != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(attitude->roll), error);
	if (attitude->pitch != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(attitude->pitch), error);
	if (attitude->heave != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(attitude->heave), error);
	if (attitude->heading != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(attitude->heading), error);

	/* Sonar Pipe Environment (record 2004) */
	sonarpipeenvironment = &store->sonarpipeenvironment;
	sonarpipeenvironment->n = 0;
	sonarpipeenvironment->nalloc = 0;
	if (sonarpipeenvironment->x != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(sonarpipeenvironment->x), error);
	if (sonarpipeenvironment->y != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(sonarpipeenvironment->y), error);
	if (sonarpipeenvironment->z != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(sonarpipeenvironment->z), error);
	if (sonarpipeenvironment->angle != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(sonarpipeenvironment->angle), error);
	if (sonarpipeenvironment->sample_number != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(sonarpipeenvironment->sample_number), error);

	/* Reson 7k configuration (record 7001) */
	configuration = &store->configuration;
	for (i = 0; i < MBSYS_RESON7K_MAX_DEVICE; i++) {
		configuration->device[i].info_length = 0;
		configuration->device[i].info_alloc = 0;
		if (configuration->device[i].info != NULL)
			status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(configuration->device[i].info), error);
	}

	/* Reson 7k firmware and hardware configuration (record 7003) */
	firmwarehardwareconfiguration = &store->firmwarehardwareconfiguration;
	if (firmwarehardwareconfiguration->info != NULL && firmwarehardwareconfiguration->info_alloc > 0)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(firmwarehardwareconfiguration->info), error);
	firmwarehardwareconfiguration->info_length = 0;
	firmwarehardwareconfiguration->info_alloc = 0;

	/* Reson 7k Side Scan Data (record 7007) */
	sidescan = &store->sidescan;
	sidescan->number_samples = 0;
	sidescan->nalloc = 0;
	if (sidescan->port_data != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(sidescan->port_data), error);
	if (sidescan->stbd_data != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(sidescan->stbd_data), error);

	/* Reson 7k Generic Water Column data (record 7008) */
	

	/* Reson 7k tvg data (record 7010) */
	tvg = &store->tvg;
	tvg->serial_number = 0;
	tvg->ping_number = 0;
	tvg->multi_ping = 0;
	tvg->n = 0;
	for (i = 0; i < 8; i++)
		tvg->reserved[i] = 0;
	tvg->nalloc = 0;
	if (tvg->tvg != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(tvg->tvg), error);

	/* Reson 7k image data (record 7011) */
	image = &store->image;
	image->width = 0;
	image->height = 0;
	image->nalloc = 0;
	if (image->image != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(image->image), error);

	/* Reson 7k ping motion (record 7012) */
	pingmotion = &store->pingmotion;
	pingmotion->n = 0;
	pingmotion->nalloc = 0;
	if (pingmotion->roll != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(pingmotion->roll), error);
	if (pingmotion->heading != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(pingmotion->heading), error);
	if (pingmotion->heave != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(pingmotion->heave), error);
		
	/* Reson 7k Adaptive Gate (record 7014) */
	

	/* Reson 7k beamformed magnitude and phase data (record 7018) */
	beamformed = &store->beamformed;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
		amplitudephase = &(beamformed->amplitudephase[i]);
		amplitudephase->samples_number = 0;
		amplitudephase->nalloc = 0;
		if (amplitudephase->amplitude != NULL)
			status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(amplitudephase->amplitude), error);
		if (amplitudephase->phase != NULL)
			status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(amplitudephase->phase), error);
	}
	
	/* Reson 7k Vernier Processing Data Raw (record 7019) */
	

	/* Reson 7k BITE (record 7021) */
	bite = &store->bite;
	bite->number_reports = 0;
	bite->nalloc = 0;
	if (bite->reports != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(bite->reports), error);
	
	/* Reson 7k snippet data (record 7028) */
	snippet = &store->snippet;
	snippet->number_beams = 0;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
		snippetdataseries = &(snippet->snippetdataseries[i]);
		snippetdataseries->beam_number = 0;
		snippetdataseries->begin_sample = 0;
		snippetdataseries->detect_sample = 0;
		snippetdataseries->end_sample = 0;
		snippetdataseries->nalloc = 0;
		if (snippetdataseries->amplitude != NULL)
			status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(snippetdataseries->amplitude), error);
	}
	
	/* Reson 7k Compressed Beamformed Magnitude Data (Record 7041) */
	
	
	/* Reson 7k Compressed Water Column Data (Record 7042) */
	
	
	/* Reson 7k Segmented Raw Detection Data (Record 7047) */
	
	
	/* Reson 7k Calibrated Beam Data (Record 7048) */
	
	
	/* Reson 7k System Events (Record 7050) */
	
	
	/* Reson 7k System Event Message (record 7051) */
	systemeventmessage = &store->systemeventmessage;
	systemeventmessage->message_length = 0;
	systemeventmessage->event_identifier = 0;
	systemeventmessage->message_alloc = 0;
	if (systemeventmessage->message != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(systemeventmessage->message), error);

	/* Reson 7k RDR Recording Status (Record 7052) */
	

	/* Reson 7k Calibrated Sidescan Data (record 7057) */
	

	/* Reson 7k Snippet Backscattering Strength (Record 7058) */
	snippetbackscatteringstrength = &store->snippetbackscatteringstrength;
	snippetbackscatteringstrength->number_beams = 0;
	for (i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
		snippetbackscatteringstrengthdata = &(snippetbackscatteringstrength->snippetbackscatteringstrengthdata[i]);
		snippetbackscatteringstrengthdata->beam_number = 0;
		snippetbackscatteringstrengthdata->begin_sample = 0;
		snippetbackscatteringstrengthdata->bottom_sample = 0;
		snippetbackscatteringstrengthdata->end_sample = 0;
		snippetbackscatteringstrengthdata->nalloc = 0;
		if (snippetbackscatteringstrengthdata->bs != NULL)
			status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(snippetbackscatteringstrengthdata->bs), error);
		if (snippetbackscatteringstrengthdata->footprints != NULL)
			status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(snippetbackscatteringstrengthdata->footprints), error);
	}

	/* deallocate memory for data structure */
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)store_ptr, error);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
/* 7K Macros */
int mbsys_reson7k_checkheader(s7k_header header) {
	return ((header.Version > 0) && (header.SyncPattern == 0x0000ffff) && (header.Size > MBSYS_RESON7K_RECORDHEADER_SIZE) &&
	        (header.s7kTime.Day >= 1) && (header.s7kTime.Day <= 366) && (header.s7kTime.Seconds >= 0.0f) &&
	        (header.s7kTime.Seconds < 60.0f) && (header.s7kTime.Hours <= 23) && (header.s7kTime.Minutes <= 59));
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_header(int verbose, s7k_header *header, int *error) {
	char *function_name = "mbsys_reson7k_print_header";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       header:     %p\n", (void *)header);
	}

	/* print Reson 7k data record header information */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     Version:                 %d\n", first, header->Version);
	fprintf(stderr, "%s     Offset:                  %d\n", first, header->Offset);
	fprintf(stderr, "%s     SyncPattern:             %d\n", first, header->SyncPattern);
	fprintf(stderr, "%s     Size:                    %d\n", first, header->Size);
	fprintf(stderr, "%s     OptionalDataOffset:      %d\n", first, header->OptionalDataOffset);
	fprintf(stderr, "%s     OptionalDataIdentifier:  %d\n", first, header->OptionalDataIdentifier);
	fprintf(stderr, "%s     s7kTime.Year:            %d\n", first, header->s7kTime.Year);
	fprintf(stderr, "%s     s7kTime.Day:             %d\n", first, header->s7kTime.Day);
	fprintf(stderr, "%s     s7kTime.Seconds:         %f\n", first, header->s7kTime.Seconds);
	fprintf(stderr, "%s     s7kTime.Hours:           %d\n", first, header->s7kTime.Hours);
	fprintf(stderr, "%s     s7kTime.Minutes:         %d\n", first, header->s7kTime.Minutes);
	fprintf(stderr, "%s     RecordVersion:                %d\n", first, header->RecordVersion);
	fprintf(stderr, "%s     RecordType:              %d\n", first, header->RecordType);
	fprintf(stderr, "%s     DeviceId:                %d\n", first, header->DeviceId);
	fprintf(stderr, "%s     Reserved:               %d\n", first, header->Reserved);
	fprintf(stderr, "%s     SystemEnumerator:        %d\n", first, header->SystemEnumerator);
	fprintf(stderr, "%s     Reserved2:           %d\n", first, header->Reserved2);
	fprintf(stderr, "%s     Flags:                   %d\n", first, header->Flags);
	fprintf(stderr, "%s     Reserved3:               %d\n", first, header->Reserved3);
	fprintf(stderr, "%s     Reserved4:               %d\n", first, header->Reserved4);
	fprintf(stderr, "%s     FragmentedTotal:         %d\n", first, header->FragmentedTotal);
	fprintf(stderr, "%s     FragmentNumber:          %d\n", first, header->FragmentNumber);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_reference(int verbose, s7kr_reference *reference, int *error) {
	char *function_name = "mbsys_reson7k_print_reference";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       reference:  %p\n", (void *)reference);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &reference->header, error);

	/* print Reference point information (record 1000) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     offset_x:                %f\n", first, reference->offset_x);
	fprintf(stderr, "%s     offset_y:                %f\n", first, reference->offset_y);
	fprintf(stderr, "%s     offset_z:                %f\n", first, reference->offset_z);
	fprintf(stderr, "%s     water_z:                 %f\n", first, reference->water_z);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_sensoruncal(int verbose, s7kr_sensoruncal *sensoruncal, int *error) {
	char *function_name = "mbsys_reson7k_print_sensoruncal";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       sensoruncal:  %p\n", (void *)sensoruncal);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &sensoruncal->header, error);

	/* print Sensor uncalibrated offset position information (record 1001) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     offset_x:                %f\n", first, sensoruncal->offset_x);
	fprintf(stderr, "%s     offset_y:                %f\n", first, sensoruncal->offset_y);
	fprintf(stderr, "%s     offset_z:                %f\n", first, sensoruncal->offset_z);
	fprintf(stderr, "%s     offset_roll:             %f\n", first, sensoruncal->offset_roll);
	fprintf(stderr, "%s     offset_pitch:            %f\n", first, sensoruncal->offset_pitch);
	fprintf(stderr, "%s     offset_yaw:              %f\n", first, sensoruncal->offset_yaw);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_sensorcal(int verbose, s7kr_sensorcal *sensorcal, int *error) {
	char *function_name = "mbsys_reson7k_print_sensorcal";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       sensorcal:    %p\n", (void *)sensorcal);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &sensorcal->header, error);

	/* print Sensor Calibrated offset position information (record 1002) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     offset_x:                %f\n", first, sensorcal->offset_x);
	fprintf(stderr, "%s     offset_y:                %f\n", first, sensorcal->offset_y);
	fprintf(stderr, "%s     offset_z:                %f\n", first, sensorcal->offset_z);
	fprintf(stderr, "%s     offset_roll:             %f\n", first, sensorcal->offset_roll);
	fprintf(stderr, "%s     offset_pitch:            %f\n", first, sensorcal->offset_pitch);
	fprintf(stderr, "%s     offset_yaw:              %f\n", first, sensorcal->offset_yaw);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_position(int verbose, s7kr_position *position, int *error) {
	char *function_name = "mbsys_reson7k_print_position";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       position:     %p\n", (void *)position);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &position->header, error);

	/* print Position (record 1003) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     datum:                   %d\n", first, position->datum);
	fprintf(stderr, "%s     latency:                 %f\n", first, position->latency);
	fprintf(stderr, "%s     latitude:                %f\n", first, position->latitude_northing);
	fprintf(stderr, "%s     longitude:               %f\n", first, position->longitude_easting);
	fprintf(stderr, "%s     height:                  %f\n", first, position->height);
	fprintf(stderr, "%s     type:                    %d\n", first, position->type);
	fprintf(stderr, "%s     utm_zone:                %d\n", first, position->utm_zone);
	fprintf(stderr, "%s     quality:                 %d\n", first, position->quality);
	fprintf(stderr, "%s     method:                  %d\n", first, position->method);
	fprintf(stderr, "%s     nsat:                  %d\n", first, position->nsat);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_customattitude(int verbose, s7kr_customattitude *customattitude, int *error) {
	char *function_name = "mbsys_reson7k_print_customattitude";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:       %d\n", verbose);
		fprintf(stderr, "dbg2       customattitude:%p\n", (void *)customattitude);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &customattitude->header, error);

	/* print Custom attitude (record 1004) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     bitfield:                   %d\n", first, customattitude->fieldmask);
	fprintf(stderr, "%s     reserved:                   %d\n", first, customattitude->reserved);
	fprintf(stderr, "%s     n:                          %d\n", first, customattitude->n);
	fprintf(stderr, "%s     frequency:                  %f\n", first, customattitude->frequency);
	fprintf(stderr, "%s     nalloc:                     %d\n", first, customattitude->nalloc);
	for (i = 0; i < customattitude->n; i++)
		fprintf(stderr, "%s     i:%d pitch:%f roll:%f heading:%f heave:%f\n", first, i, customattitude->pitch[i],
		        customattitude->roll[i], customattitude->heading[i], customattitude->heave[i]);
	for (i = 0; i < customattitude->n; i++)
		fprintf(stderr, "%s     i:%d pitchrate:%f rollrate:%f headingrate:%f heaverate:%f\n", first, i,
		        customattitude->pitchrate[i], customattitude->rollrate[i], customattitude->headingrate[i],
		        customattitude->heaverate[i]);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_tide(int verbose, s7kr_tide *tide, int *error) {
	char *function_name = "mbsys_reson7k_print_tide";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       tide:         %p\n", (void *)tide);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &tide->header, error);

	/* print Tide (record 1005) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     tide:                       %f\n", first, tide->tide);
	fprintf(stderr, "%s     source:                     %d\n", first, tide->source);
	fprintf(stderr, "%s     flags:                      %d\n", first, tide->flags);
	fprintf(stderr, "%s     gauge:                      %d\n", first, tide->gauge);
	fprintf(stderr, "%s     datum:                      %d\n", first, tide->datum);
	fprintf(stderr, "%s     latency:                    %f\n", first, tide->latency);
	fprintf(stderr, "%s     latitude:                   %f\n", first, tide->latitude_northing);
	fprintf(stderr, "%s     longitude:                  %f\n", first, tide->longitude_easting);
	fprintf(stderr, "%s     height:                     %f\n", first, tide->height);
	fprintf(stderr, "%s     type:                       %d\n", first, tide->type);
	fprintf(stderr, "%s     utm_zone:                   %d\n", first, tide->utm_zone);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_altitude(int verbose, s7kr_altitude *altitude, int *error) {
	char *function_name = "mbsys_reson7k_print_altitude";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       altitude:     %p\n", (void *)altitude);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &altitude->header, error);

	/* print Altitude (record 1006) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     altitude:                   %f\n", first, altitude->altitude);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_motion(int verbose, s7kr_motion *motion, int *error) {
	char *function_name = "mbsys_reson7k_print_motion";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       motion:       %p\n", (void *)motion);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &motion->header, error);

	/* print Motion over ground (record 1007) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     bitfield:                   %d\n", first, motion->flags);
	fprintf(stderr, "%s     reserved:                   %d\n", first, motion->reserved);
	fprintf(stderr, "%s     n:                          %d\n", first, motion->n);
	fprintf(stderr, "%s     frequency:                  %f\n", first, motion->frequency);
	fprintf(stderr, "%s     nalloc:                     %d\n", first, motion->nalloc);
	for (i = 0; i < motion->n; i++)
		fprintf(stderr, "%s     i:%d x:%f y:%f z:%f xa:%f ya:%f za:%f\n", first, i, motion->x[i], motion->y[i], motion->z[i],
		        motion->xa[i], motion->ya[i], motion->za[i]);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_depth(int verbose, s7kr_depth *depth, int *error) {
	char *function_name = "mbsys_reson7k_print_depth";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       depth:        %p\n", (void *)depth);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &depth->header, error);

	/* print Depth (record 1008) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     descriptor:                  %d\n", first, depth->descriptor);
	fprintf(stderr, "%s     correction:                  %d\n", first, depth->correction);
	fprintf(stderr, "%s     reserved:                    %d\n", first, depth->reserved);
	fprintf(stderr, "%s     depth:                       %f\n", first, depth->depth);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_svp(int verbose, s7kr_svp *svp, int *error) {
	char *function_name = "mbsys_reson7k_print_svp";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       svp:          %p\n", (void *)svp);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &svp->header, error);

	/* print Sound velocity profile (record 1009) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     position_flag:              %d\n", first, svp->position_flag);
	fprintf(stderr, "%s     reserved1:                  %d\n", first, svp->reserved1);
	fprintf(stderr, "%s     reserved2:                  %d\n", first, svp->reserved2);
	fprintf(stderr, "%s     latitude:                   %f\n", first, svp->latitude);
	fprintf(stderr, "%s     longitude:                  %f\n", first, svp->longitude);
	fprintf(stderr, "%s     n:                          %d\n", first, svp->n);
	fprintf(stderr, "%s     nalloc:                     %d\n", first, svp->nalloc);
	for (i = 0; i < svp->n; i++)
		fprintf(stderr, "%s     i:%d depth:%f sound_velocity:%f\n", first, i, svp->depth[i], svp->sound_velocity[i]);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_ctd(int verbose, s7kr_ctd *ctd, int *error) {
	char *function_name = "mbsys_reson7k_print_ctd";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       ctd:          %p\n", (void *)ctd);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &ctd->header, error);

	/* print CTD (record 1010) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     frequency:                  %f\n", first, ctd->frequency);
	fprintf(stderr, "%s     velocity_source_flag:       %d\n", first, ctd->velocity_source_flag);
	fprintf(stderr, "%s     velocity_algorithm:         %d\n", first, ctd->velocity_algorithm);
	fprintf(stderr, "%s     conductivity_flag:          %d\n", first, ctd->conductivity_flag);
	fprintf(stderr, "%s     pressure_flag:              %d\n", first, ctd->pressure_flag);
	fprintf(stderr, "%s     position_flag:              %d\n", first, ctd->position_flag);
	fprintf(stderr, "%s     validity:                   %d\n", first, ctd->validity);
	fprintf(stderr, "%s     reserved:                   %d\n", first, ctd->reserved);
	fprintf(stderr, "%s     latitude:                   %f\n", first, ctd->latitude);
	fprintf(stderr, "%s     longitude:                  %f\n", first, ctd->longitude);
	fprintf(stderr, "%s     sample_rate:                %f\n", first, ctd->sample_rate);
	fprintf(stderr, "%s     n:                          %d\n", first, ctd->n);
	fprintf(stderr, "%s     nalloc:                     %d\n", first, ctd->nalloc);
	for (i = 0; i < ctd->n; i++)
		fprintf(stderr, "%s     i:%d conductivity_salinity:%f temperature:%f pressure_depth:%f sound_velocity:%f absorption:%f\n",
		        first, i, ctd->conductivity_salinity[i], ctd->temperature[i], ctd->pressure_depth[i], ctd->sound_velocity[i],
		        ctd->absorption[i]);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_geodesy(int verbose, s7kr_geodesy *geodesy, int *error) {
	char *function_name = "mbsys_reson7k_print_geodesy";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       geodesy:      %p\n", (void *)geodesy);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &geodesy->header, error);

	/* print Geodesy (record 1011) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     spheroid:                   %s\n", first, geodesy->spheroid);
	fprintf(stderr, "%s     semimajoraxis:              %f\n", first, geodesy->semimajoraxis);
	fprintf(stderr, "%s     flattening:                 %f\n", first, geodesy->flattening);
	fprintf(stderr, "%s     reserved1:                  %s\n", first, geodesy->reserved1);
	fprintf(stderr, "%s     datum:                      %s\n", first, geodesy->datum);
	fprintf(stderr, "%s     calculation_method:         %d\n", first, geodesy->calculation_method);
	fprintf(stderr, "%s     number_parameters:          %d\n", first, geodesy->number_parameters);
	fprintf(stderr, "%s     dx:                         %f\n", first, geodesy->dx);
	fprintf(stderr, "%s     dy:                         %f\n", first, geodesy->dy);
	fprintf(stderr, "%s     dz:                         %f\n", first, geodesy->dz);
	fprintf(stderr, "%s     rx:                         %f\n", first, geodesy->rx);
	fprintf(stderr, "%s     ry:                         %f\n", first, geodesy->ry);
	fprintf(stderr, "%s     rz:                         %f\n", first, geodesy->rz);
	fprintf(stderr, "%s     scale:                      %f\n", first, geodesy->scale);
	fprintf(stderr, "%s     reserved2:                  %s\n", first, geodesy->reserved2);
	fprintf(stderr, "%s     grid_name:                  %s\n", first, geodesy->grid_name);
	fprintf(stderr, "%s     distance_units:             %d\n", first, geodesy->distance_units);
	fprintf(stderr, "%s     angular_units:              %d\n", first, geodesy->angular_units);
	fprintf(stderr, "%s     latitude_origin:            %f\n", first, geodesy->latitude_origin);
	fprintf(stderr, "%s     central_meriidan:           %f\n", first, geodesy->central_meridian);
	fprintf(stderr, "%s     false_easting:              %f\n", first, geodesy->false_easting);
	fprintf(stderr, "%s     false_northing:             %f\n", first, geodesy->false_northing);
	fprintf(stderr, "%s     central_scale_factor:       %f\n", first, geodesy->central_scale_factor);
	fprintf(stderr, "%s     custum_identifier:          %d\n", first, geodesy->custum_identifier);
	fprintf(stderr, "%s     reserved3:                  %s\n", first, geodesy->reserved3);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_rollpitchheave(int verbose, s7kr_rollpitchheave *rollpitchheave, int *error) {
	char *function_name = "mbsys_reson7k_print_rollpitchheave";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
		fprintf(stderr, "dbg2       rollpitchheave: %p\n", (void *)rollpitchheave);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &rollpitchheave->header, error);

	/* print Roll pitch heave (record 1012) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     roll:                       %f\n", first, rollpitchheave->roll);
	fprintf(stderr, "%s     pitch:                      %f\n", first, rollpitchheave->pitch);
	fprintf(stderr, "%s     heave:                      %f\n", first, rollpitchheave->heave);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_heading(int verbose, s7kr_heading *heading, int *error) {
	char *function_name = "mbsys_reson7k_print_heading";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       heading:      %p\n", (void *)heading);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &heading->header, error);

	/* print Heading (record 1013) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     heading:                    %f\n", first, heading->heading);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_surveyline(int verbose, s7kr_surveyline *surveyline, int *error) {
	char *function_name = "mbsys_reson7k_print_surveyline";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       surveyline:   %p\n", (void *)surveyline);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &surveyline->header, error);

	/* print Survey Line (record 1014) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     n:                          %d\n", first, surveyline->n);
	fprintf(stderr, "%s     type:                       %d\n", first, surveyline->type);
	fprintf(stderr, "%s     turnradius:                 %f\n", first, surveyline->turnradius);
	fprintf(stderr, "%s     name:                       %s\n", first, surveyline->name);
	fprintf(stderr, "%s     nalloc:                     %d\n", first, surveyline->nalloc);
	for (i = 0; i < surveyline->n; i++)
		fprintf(stderr, "%s     i:%d latitude_northing:%f longitude_easting:%f\n", first, i, surveyline->latitude_northing[i], surveyline->longitude_easting[i]);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_navigation(int verbose, s7kr_navigation *navigation, int *error) {
	char *function_name = "mbsys_reson7k_print_navigation";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       navigation:   %p\n", (void *)navigation);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &navigation->header, error);

	/* print Navigation (record 1015) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     vertical_reference:         %d\n", first, navigation->vertical_reference);
	fprintf(stderr, "%s     latitude:                   %f\n", first, navigation->latitude);
	fprintf(stderr, "%s     longitude:                  %f\n", first, navigation->longitude);
	fprintf(stderr, "%s     position_accuracy:          %f\n", first, navigation->position_accuracy);
	fprintf(stderr, "%s     height:                     %f\n", first, navigation->height);
	fprintf(stderr, "%s     height_accuracy:            %f\n", first, navigation->height_accuracy);
	fprintf(stderr, "%s     speed:                      %f\n", first, navigation->speed);
	fprintf(stderr, "%s     course:                     %f\n", first, navigation->course);
	fprintf(stderr, "%s     heading:                    %f\n", first, navigation->heading);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_attitude(int verbose, s7kr_attitude *attitude, int *error) {
	char *function_name = "mbsys_reson7k_print_attitude";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       attitude:     %p\n", (void *)attitude);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &attitude->header, error);

	/* print Attitude (record 1016) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     n:                          %d\n", first, attitude->n);
	fprintf(stderr, "%s     nalloc:                     %d\n", first, attitude->nalloc);
	for (i = 0; i < attitude->n; i++)
		fprintf(stderr, "%s     i:%d delta_time:%d pitch:%f roll:%f heading:%f heave:%f\n", first, i, attitude->delta_time[i],
		        attitude->pitch[i], attitude->roll[i], attitude->heave[i], attitude->heading[i]);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_pantilt(int verbose, s7kr_pantilt *pantilt, int *error) {
	char *function_name = "mbsys_reson7k_print_pantilt";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       navigation:   %p\n", (void *)pantilt);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &pantilt->header, error);

	/* print Pan Tilt (record 1017) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     pan:         %d\n", first, pantilt->pan);
	fprintf(stderr, "%s     tilt:                   %f\n", first, pantilt->tilt);
	
	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
/* Sonar Installation Identifiers (record 1020) */

/*--------------------------------------------------------------------*/
/* Sonar Pipe Environment (record 2004) */

/*--------------------------------------------------------------------*/
/* Contact Output (record 3001) */

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_sonarsettings(int verbose, s7kr_sonarsettings *sonarsettings, int *error) {
	char *function_name = "mbsys_reson7k_print_sonarsettings";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       sonarsettings:  %p\n", (void *)sonarsettings);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &sonarsettings->header, error);

	/* print Reson 7k Ssonar settings (record 7000) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     serial_number:              %llu\n", first, sonarsettings->serial_number);
	fprintf(stderr, "%s     ping_number:                %u\n", first, sonarsettings->ping_number);
	fprintf(stderr, "%s     multi_ping:                 %u\n", first, sonarsettings->multi_ping);
	fprintf(stderr, "%s     frequency:                  %f\n", first, sonarsettings->frequency);
	fprintf(stderr, "%s     sample_rate:                %f\n", first, sonarsettings->sample_rate);
	fprintf(stderr, "%s     receiver_bandwidth:         %f\n", first, sonarsettings->receiver_bandwidth);
	fprintf(stderr, "%s     tx_pulse_width:                %f\n", first, sonarsettings->tx_pulse_width);
	fprintf(stderr, "%s     tx_pulse_type:                 %d\n", first, sonarsettings->tx_pulse_type);
	fprintf(stderr, "%s     tx_pulse_envelope:             %d\n", first, sonarsettings->tx_pulse_envelope);
	fprintf(stderr, "%s     tx_pulse_envelope_par:         %f\n", first, sonarsettings->tx_pulse_envelope_par);
	fprintf(stderr, "%s     tx_pulse_mode:             %d\n", first, sonarsettings->tx_pulse_mode);
	fprintf(stderr, "%s     max_ping_rate:              %f\n", first, sonarsettings->max_ping_rate);
	fprintf(stderr, "%s     ping_period:                %f\n", first, sonarsettings->ping_period);
	fprintf(stderr, "%s     range_selection:            %f\n", first, sonarsettings->range_selection);
	fprintf(stderr, "%s     power_selection:            %f\n", first, sonarsettings->power_selection);
	fprintf(stderr, "%s     gain_selection:             %f\n", first, sonarsettings->gain_selection);
	fprintf(stderr, "%s     control_flags:              %d\n", first, sonarsettings->control_flags);
	fprintf(stderr, "%s     projector_magic_no:         %d\n", first, sonarsettings->projector_magic_no);
	fprintf(stderr, "%s     steering_vertical:          %f\n", first, sonarsettings->steering_vertical);
	fprintf(stderr, "%s     steering_horizontal:        %f\n", first, sonarsettings->steering_horizontal);
	fprintf(stderr, "%s     beamwidth_vertical:         %f\n", first, sonarsettings->beamwidth_vertical);
	fprintf(stderr, "%s     beamwidth_horizontal:       %f\n", first, sonarsettings->beamwidth_horizontal);
	fprintf(stderr, "%s     focal_point:                %f\n", first, sonarsettings->focal_point);
	fprintf(stderr, "%s     projector_weighting:        %d\n", first, sonarsettings->projector_weighting);
	fprintf(stderr, "%s     projector_weighting_par:    %f\n", first, sonarsettings->projector_weighting_par);
	fprintf(stderr, "%s     transmit_flags:             %d\n", first, sonarsettings->transmit_flags);
	fprintf(stderr, "%s     hydrophone_magic_no:        %d\n", first, sonarsettings->hydrophone_magic_no);
	fprintf(stderr, "%s     rx_weighting:          %d\n", first, sonarsettings->rx_weighting);
	fprintf(stderr, "%s     rx_weighting_par:      %f\n", first, sonarsettings->rx_weighting_par);
	fprintf(stderr, "%s     rx_flags:              %d\n", first, sonarsettings->rx_flags);
	fprintf(stderr, "%s     rx_width:              %f\n", first, sonarsettings->rx_width);
	fprintf(stderr, "%s     range_minimum:              %f\n", first, sonarsettings->range_minimum);
	fprintf(stderr, "%s     range_maximum:              %f\n", first, sonarsettings->range_maximum);
	fprintf(stderr, "%s     depth_minimum:              %f\n", first, sonarsettings->depth_minimum);
	fprintf(stderr, "%s     depth_maximum:              %f\n", first, sonarsettings->depth_maximum);
	fprintf(stderr, "%s     absorption:                 %f\n", first, sonarsettings->absorption);
	fprintf(stderr, "%s     sound_velocity:             %f\n", first, sonarsettings->sound_velocity);
	fprintf(stderr, "%s     spreading:                  %f\n", first, sonarsettings->spreading);
	fprintf(stderr, "%s     reserved:                   %d\n", first, sonarsettings->reserved);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_device(int verbose, s7kr_device *device, int *error) {
	char *function_name = "mbsys_reson7k_print_device";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       device:            %p\n", (void *)device);
	}

	/* print Reson 7k device configuration structure (part of record 7001) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     magic_number:               %d\n", first, device->magic_number);
	fprintf(stderr, "%s     description:                %s\n", first, device->description);
	fprintf(stderr, "%s     serial_number:              %llu\n", first, device->serial_number);
	fprintf(stderr, "%s     info_length:                %d\n", first, device->info_length);
	fprintf(stderr, "%s     info_alloc:                 %d\n", first, device->info_alloc);
	fprintf(stderr, "%s     info:                       %s\n", first, device->info);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_configuration(int verbose, s7kr_configuration *configuration, int *error) {
	char *function_name = "mbsys_reson7k_print_configuration";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       configuration:     %p\n", (void *)configuration);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &configuration->header, error);

	/* print Reson 7k configuration (record 7001) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     serial_number:              %llu\n", first, configuration->serial_number);
	fprintf(stderr, "%s     number_devices:             %d\n", first, configuration->number_devices);
	for (i = 0; i < configuration->number_devices; i++)
		mbsys_reson7k_print_device(verbose, &configuration->device[i], error);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_matchfilter(int verbose, s7kr_matchfilter *matchfilter, int *error) {
	char *function_name = "mbsys_reson7k_print_matchfilter";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       matchfilter:       %p\n", (void *)matchfilter);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &matchfilter->header, error);

	/* print Reson 7k match filter (record 7002) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     serial_number:              %llu\n", first, matchfilter->serial_number);
	fprintf(stderr, "%s     ping_number:                %u\n", first, matchfilter->ping_number);
	fprintf(stderr, "%s     operation:                  %d\n", first, matchfilter->operation);
	fprintf(stderr, "%s     start_frequency:            %f\n", first, matchfilter->start_frequency);
	fprintf(stderr, "%s     end_frequency:              %f\n", first, matchfilter->end_frequency);
	fprintf(stderr, "%s     window_type:              %f\n", first, matchfilter->window_type);
	fprintf(stderr, "%s     shading:              %f\n", first, matchfilter->shading);
	fprintf(stderr, "%s     pulse_width:              %f\n", first, matchfilter->pulse_width);
	fprintf(stderr, "%s     reserved:              %f\n", first, matchfilter->reserved);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_firmwarehardwareconfiguration(int verbose,
                                                        s7kr_firmwarehardwareconfiguration *firmwarehardwareconfiguration,
                                                        int *error) {
	char *function_name = "mbsys_reson7k_print_firmwarehardwareconfiguration";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       firmwarehardwareconfiguration:       %p\n", (void *)firmwarehardwareconfiguration);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &firmwarehardwareconfiguration->header, error);

	/* print Reson firmware and hardware configuration (record 7003) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     device_count:               %d\n", first, firmwarehardwareconfiguration->device_count);
	fprintf(stderr, "%s     info_length:                %d\n", first, firmwarehardwareconfiguration->info_length);
	fprintf(stderr, "%s     info:                       \n", first);
	fprintf(stderr, "%s\n%s\n", firmwarehardwareconfiguration->info, first);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_beamgeometry(int verbose, s7kr_beamgeometry *beamgeometry, int *error) {
	char *function_name = "mbsys_reson7k_print_beamgeometry";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       beamgeometry:      %p\n", (void *)beamgeometry);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &beamgeometry->header, error);

	/* print Reson 7k beam geometry (record 7004) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     serial_number:              %llu\n", first, beamgeometry->serial_number);
	fprintf(stderr, "%s     number_beams:               %u\n", first, beamgeometry->number_beams);
	for (i = 0; i < beamgeometry->number_beams; i++)
		fprintf(stderr,
		        "%s     beam[%d]:  angle_alongtrack:%f angle_acrosstrack:%f beamwidth_alongtrack:%f beamwidth_acrosstrack:%f\n",
		        first, i, beamgeometry->angle_alongtrack[i], beamgeometry->angle_acrosstrack[i],
		        beamgeometry->beamwidth_alongtrack[i], beamgeometry->beamwidth_acrosstrack[i]);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_bathymetry(int verbose, s7kr_bathymetry *bathymetry, int *error) {
	char *function_name = "mbsys_reson7k_print_bathymetry";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       bathymetry:        %p\n", (void *)bathymetry);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &bathymetry->header, error);

	/* print Reson 7k bathymetry (record 7006) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     serial_number:              %llu\n", first, bathymetry->serial_number);
	fprintf(stderr, "%s     ping_number:                %u\n", first, bathymetry->ping_number);
	fprintf(stderr, "%s     multi_ping:                 %u\n", first, bathymetry->multi_ping);
	fprintf(stderr, "%s     number_beams:               %u\n", first, bathymetry->number_beams);
	fprintf(stderr, "%s     layer_comp_flag:            %d\n", first, bathymetry->layer_comp_flag);
	fprintf(stderr, "%s     sound_vel_flag:             %d\n", first, bathymetry->sound_vel_flag);
	fprintf(stderr, "%s     sound_velocity:             %f\n", first, bathymetry->sound_velocity);
	for (i = 0; i < bathymetry->number_beams; i++)
		fprintf(stderr, "%s     beam[%d]:  range:%f quality:%d intensity:%f min_depth_gate:%f min_depth_gate:%f\n", first, i,
		        bathymetry->range[i], bathymetry->quality[i], bathymetry->intensity[i], bathymetry->min_depth_gate[i],
		        bathymetry->max_depth_gate[i]);
	fprintf(stderr, "%s     optionaldata:               %d\n", first, bathymetry->optionaldata);
	fprintf(stderr, "%s     frequency:                  %f\n", first, bathymetry->frequency);
	fprintf(stderr, "%s     latitude:                   %f\n", first, bathymetry->latitude);
	fprintf(stderr, "%s     longitude:                  %f\n", first, bathymetry->longitude);
	fprintf(stderr, "%s     heading:                    %f\n", first, bathymetry->heading);
	fprintf(stderr, "%s     height_source:              %d\n", first, bathymetry->height_source);
	fprintf(stderr, "%s     tide:                       %f\n", first, bathymetry->tide);
	fprintf(stderr, "%s     roll:                       %f\n", first, bathymetry->roll);
	fprintf(stderr, "%s     pitch:                      %f\n", first, bathymetry->pitch);
	fprintf(stderr, "%s     heave:                      %f\n", first, bathymetry->heave);
	fprintf(stderr, "%s     vehicle_height:             %f\n", first, bathymetry->vehicle_height);
	for (i = 0; i < bathymetry->number_beams; i++)
		fprintf(stderr, "%s     beam[%d]:  depth:%f ltrack:%f xtrack:%f angles: %f %f\n", first, i, bathymetry->depth[i],
		        bathymetry->alongtrack[i], bathymetry->acrosstrack[i], bathymetry->pointing_angle[i],
		        bathymetry->azimuth_angle[i]);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_sidescan(int verbose, s7kr_sidescan *sidescan, int *error) {
	char *function_name = "mbsys_reson7k_print_sidescan";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	mb_s_char *charptr;
	short *shortptr;
	int *intptr;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       sidescan:       %p\n", (void *)sidescan);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &sidescan->header, error);

	/* print Reson 7k sidescan data (record 7007) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     serial_number:              %llu\n", first, sidescan->serial_number);
	fprintf(stderr, "%s     ping_number:                %u\n", first, sidescan->ping_number);
	fprintf(stderr, "%s     multi_ping:                 %u\n", first, sidescan->multi_ping);
	fprintf(stderr, "%s     beam_position:              %f\n", first, sidescan->beam_position);
	fprintf(stderr, "%s     control_flags:              %d\n", first, sidescan->control_flags);
	fprintf(stderr, "%s     number_samples:             %d\n", first, sidescan->number_samples);
	fprintf(stderr, "%s     nadir_depth:             %d\n", first, sidescan->nadir_depth);
	fprintf(stderr, "%s     reserved:             %d\n", first, sidescan->reserved);
	fprintf(stderr, "%s     number_beams:               %u\n", first, sidescan->number_beams);
	fprintf(stderr, "%s     current_beam:               %d\n", first, sidescan->current_beam);
	fprintf(stderr, "%s     sample_size:                %d\n", first, sidescan->sample_size);
	fprintf(stderr, "%s     data_type:                  %d\n", first, sidescan->data_type);
	fprintf(stderr, "%s     nalloc:                     %d\n", first, sidescan->nalloc);
	if (sidescan->sample_size == 1) {
		charptr = (mb_s_char *)sidescan->port_data;
		for (i = 0; i < sidescan->number_samples; i++)
			fprintf(stderr, "%s     port sidescan[%d]:  %d\n", first, i, charptr[i]);
		charptr = (mb_s_char *)sidescan->stbd_data;
		for (i = 0; i < sidescan->number_samples; i++)
			fprintf(stderr, "%s     stbd sidescan[%d]:  %d\n", first, i, charptr[i]);
	}
	else if (sidescan->sample_size == 2) {
		shortptr = (short *)sidescan->port_data;
		for (i = 0; i < sidescan->number_samples; i++)
			fprintf(stderr, "%s     port sidescan[%d]:  %d\n", first, i, shortptr[i]);
		shortptr = (short *)sidescan->stbd_data;
		for (i = 0; i < sidescan->number_samples; i++)
			fprintf(stderr, "%s     stbd sidescan[%d]:  %d\n", first, i, shortptr[i]);
	}
	else if (sidescan->sample_size == 4) {
		intptr = (int *)sidescan->port_data;
		for (i = 0; i < sidescan->number_samples; i++)
			fprintf(stderr, "%s     port sidescan[%d]:  %d\n", first, i, intptr[i]);
		intptr = (int *)sidescan->stbd_data;
		for (i = 0; i < sidescan->number_samples; i++)
			fprintf(stderr, "%s     stbd sidescan[%d]:  %d\n", first, i, intptr[i]);
	}
	fprintf(stderr, "%s     optionaldata:               %d\n", first, sidescan->optionaldata);
	fprintf(stderr, "%s     frequency:                  %f\n", first, sidescan->frequency);
	fprintf(stderr, "%s     latitude:                   %f\n", first, sidescan->latitude);
	fprintf(stderr, "%s     longitude:                  %f\n", first, sidescan->longitude);
	fprintf(stderr, "%s     heading:                    %f\n", first, sidescan->heading);
	fprintf(stderr, "%s     altitude:                   %f\n", first, sidescan->altitude);
	fprintf(stderr, "%s     depth:                   %f\n", first, sidescan->depth);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/* Reson 7k Generic Water Column data (record 7008) */

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_verticaldepth(int verbose, s7kr_verticaldepth *verticaldepth, int *error) {
	char *function_name = "mbsys_reson7k_print_verticaldepth";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       verticaldepth:     %p\n", (void *)verticaldepth);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &verticaldepth->header, error);

	/* print Reson 7k vertical depth data (record 7009) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     frequency:                  %f\n", first, verticaldepth->frequency);
	fprintf(stderr, "%s     ping_number:                %u\n", first, verticaldepth->ping_number);
	fprintf(stderr, "%s     multi_ping:                 %u\n", first, verticaldepth->multi_ping);
	fprintf(stderr, "%s     latitude:                   %f\n", first, verticaldepth->latitude);
	fprintf(stderr, "%s     longitude:                  %f\n", first, verticaldepth->longitude);
	fprintf(stderr, "%s     heading:                    %f\n", first, verticaldepth->heading);
	fprintf(stderr, "%s     alongtrack:                 %f\n", first, verticaldepth->alongtrack);
	fprintf(stderr, "%s     acrosstrack:                %f\n", first, verticaldepth->acrosstrack);
	fprintf(stderr, "%s     vertical_depth:             %f\n", first, verticaldepth->vertical_depth);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_tvg(int verbose, s7kr_tvg *tvg, int *error) {
	char *function_name = "mbsys_reson7k_print_tvg";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	float *tvg_float;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       tvg:               %p\n", (void *)tvg);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &tvg->header, error);

	/* print Reson 7k tvg data (record 7010) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     serial_number:              %llu\n", first, tvg->serial_number);
	fprintf(stderr, "%s     ping_number:                %u\n", first, tvg->ping_number);
	fprintf(stderr, "%s     multi_ping:                 %u\n", first, tvg->multi_ping);
	fprintf(stderr, "%s     n:                          %d\n", first, tvg->n);
	for (i = 0; i < 8; i++)
		fprintf(stderr, "%s     reserved[%d]:                %d\n", first, i, tvg->reserved[i]);
	for (i = 0; i < tvg->n; i++) {
		tvg_float = (float *)tvg->tvg;
		fprintf(stderr, "%s     tvg[%d]:  %f\n", first, i, tvg_float[i]);
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_image(int verbose, s7kr_image *image, int *error) {
	char *function_name = "mbsys_reson7k_print_image";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	mb_s_char *charptr;
	short *shortptr;
	int *intptr;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       image:             %p\n", (void *)image);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &image->header, error);

	/* print Reson 7k image imagery data (record 7011) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     ping_number:                %u\n", first, image->ping_number);
	fprintf(stderr, "%s     multi_ping:                 %u\n", first, image->multi_ping);
	fprintf(stderr, "%s     width:                      %d\n", first, image->width);
	fprintf(stderr, "%s     height:                     %d\n", first, image->height);
	fprintf(stderr, "%s     color_depth:                %d\n", first, image->color_depth);
	fprintf(stderr, "%s     reserved:                %d\n", first, image->reserved);
	fprintf(stderr, "%s     compression:                %d\n", first, image->compression);
	fprintf(stderr, "%s     samples:                %d\n", first, image->samples);
	fprintf(stderr, "%s     flag:                %d\n", first, image->flag);
	fprintf(stderr, "%s     rx_delay:                %d\n", first, image->rx_delay);
	fprintf(stderr, "%s     reserved:                %d\n", first, image->reserved2);
	fprintf(stderr, "%s     nalloc:                     %d\n", first, image->nalloc);
	if (image->color_depth == 1) {
		charptr = (mb_s_char *)image->image;
		for (i = 0; i < image->width * image->height; i++)
			fprintf(stderr, "%s     image[%d]:  %hhu\n", first, i, charptr[i]);
	}
	else if (image->color_depth == 2) {
		shortptr = (short *)image->image;
		for (i = 0; i < image->width * image->height; i++)
			fprintf(stderr, "%s     image[%d]:  %hu\n", first, i, shortptr[i]);
	}
	else if (image->color_depth == 4) {
		intptr = (int *)image->image;
		for (i = 0; i < image->width * image->height; i++)
			fprintf(stderr, "%s     image[%d]:  %u\n", first, i, intptr[i]);
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_pingmotion(int verbose, s7kr_pingmotion *pingmotion, int *error) {
	char *function_name = "mbsys_reson7k_print_pingmotion";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       pingmotion:      %p\n", (void *)pingmotion);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &pingmotion->header, error);

	/* print Reson 7k ping motion (record 7012) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     serial_number:              %llu\n", first, pingmotion->serial_number);
	fprintf(stderr, "%s     ping_number:                %u\n", first, pingmotion->ping_number);
	fprintf(stderr, "%s     multi_ping:                 %u\n", first, pingmotion->multi_ping);
	fprintf(stderr, "%s     n:                          %d\n", first, pingmotion->n);
	fprintf(stderr, "%s     flags:                      %d\n", first, pingmotion->flags);
	fprintf(stderr, "%s     error_flags:                %d\n", first, pingmotion->error_flags);
	fprintf(stderr, "%s     frequency:                  %f\n", first, pingmotion->frequency);
	fprintf(stderr, "%s     pitch:                      %f\n", first, pingmotion->pitch);
	fprintf(stderr, "%s     nalloc:                     %d\n", first, pingmotion->nalloc);
	fprintf(stderr, "%s     beam	roll    heading    heave\n", first);
	fprintf(stderr, "%s     ----	----    -------    -----\n", first);
	for (i = 0; i < pingmotion->n; i++) {
		fprintf(stderr, "%s     %3d  %10g  %10g  %10g\n", first, i, pingmotion->roll[i], pingmotion->heading[i],
		        pingmotion->heave[i]);
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/* Reson 7k Adaptive Gate (record 7014) */

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_detectionsetup(int verbose, s7kr_detectionsetup *detectionsetup, int *error) {
	char *function_name = "mbsys_reson7k_print_detectionsetup";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       detectionsetup:  %p\n", (void *)detectionsetup);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &detectionsetup->header, error);

	/* print Reson 7k detection setup (record 7017) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     serial_number:              %llu\n", first, detectionsetup->serial_number);
	fprintf(stderr, "%s     ping_number:                %u\n", first, detectionsetup->ping_number);
	fprintf(stderr, "%s     multi_ping:                 %u\n", first, detectionsetup->multi_ping);
	fprintf(stderr, "%s     number_beams:               %u\n", first, detectionsetup->number_beams);
	fprintf(stderr, "%s     data_block_size:            %d\n", first, detectionsetup->data_block_size);
	fprintf(stderr, "%s     detection_algorithm:        %d\n", first, detectionsetup->detection_algorithm);
	fprintf(stderr, "%s     detection_flags:            %d\n", first, detectionsetup->detection_flags);
	fprintf(stderr, "%s     minimum_depth:              %f\n", first, detectionsetup->minimum_depth);
	fprintf(stderr, "%s     maximum_depth:              %f\n", first, detectionsetup->maximum_depth);
	fprintf(stderr, "%s     minimum_range:              %f\n", first, detectionsetup->minimum_range);
	fprintf(stderr, "%s     maximum_range:              %f\n", first, detectionsetup->maximum_range);
	fprintf(stderr, "%s     minimum_nadir_search:       %f\n", first, detectionsetup->minimum_nadir_search);
	fprintf(stderr, "%s     maximum_nadir_search:       %f\n", first, detectionsetup->maximum_nadir_search);
	fprintf(stderr, "%s     automatic_filter_window:    %u\n", first, detectionsetup->automatic_filter_window);
	fprintf(stderr, "%s     applied_roll:               %f\n", first, detectionsetup->applied_roll);
	fprintf(stderr, "%s     depth_gate_tilt:            %f\n", first, detectionsetup->depth_gate_tilt);
	for (i = 0; i < 14; i++) {
		fprintf(stderr, "%s     reserved[%2d]:               %f\n", first, i, detectionsetup->reserved[i]);
	}
	fprintf(stderr, "%s     beam	descriptor pick flag amin amax umin umax quality uncertainty\n", first);
	fprintf(stderr, "%s     ---------------------------------------------------------\n", first);
	for (i = 0; i < detectionsetup->number_beams; i++) {
		fprintf(stderr, "%s     %3d %u %10.3f %u %u %u %u %u %u %f\n", first, i, detectionsetup->beam_descriptor[i],
		        detectionsetup->detection_point[i], detectionsetup->flags[i], detectionsetup->auto_limits_min_sample[i],
		        detectionsetup->auto_limits_max_sample[i], detectionsetup->user_limits_min_sample[i],
		        detectionsetup->user_limits_max_sample[i], detectionsetup->quality[i], detectionsetup->uncertainty[i]);
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_beamformed(int verbose, s7kr_beamformed *beamformed, int *error) {
	char *function_name = "mbsys_reson7k_print_beamformed";
	int status = MB_SUCCESS;
	s7kr_amplitudephase *amplitudephase;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int i, j;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       beamformed:      %p\n", (void *)beamformed);
	}

	/* Reson 7k beamformed magnitude and phase data (record 7018) */
	mbsys_reson7k_print_header(verbose, &beamformed->header, error);

	/* print Reson 7k Beamformed Data (record 7018)  */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     serial_number:              %llu\n", first, beamformed->serial_number);
	fprintf(stderr, "%s     ping_number:                %u\n", first, beamformed->ping_number);
	fprintf(stderr, "%s     multi_ping:                 %u\n", first, beamformed->multi_ping);
	fprintf(stderr, "%s     beams_number:               %u\n", first, beamformed->beams_number);
	fprintf(stderr, "%s     samples_number:             %d\n", first, beamformed->samples_number);
	fprintf(stderr, "%s     samples_number:             %d\n", first, beamformed->samples_number);
	fprintf(stderr, "%s     reserved:                   ", first);
	for (i = 0; i < 8; i++)
		fprintf(stderr, "%u ", beamformed->reserved[i]);
	fprintf(stderr, "\n");
	for (i = 0; i < beamformed->beams_number; i++) {
		amplitudephase = &(beamformed->amplitudephase[i]);
		fprintf(stderr, "%s     beam_number:                %d\n", first, amplitudephase->beam_number);
		fprintf(stderr, "%s     samples_number:             %d\n", first, amplitudephase->samples_number);
		for (j = 0; j < amplitudephase->samples_number; j++) {
			fprintf(stderr, "%s     beam[%d] sample[%d] amplitude:%u phase:%d\n", first, i, j, amplitudephase->amplitude[j],
			        amplitudephase->phase[j]);
		}
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/* Reson 7k Vernier Processing Data Raw (record 7019) */


/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_bite(int verbose, s7kr_bite *bite, int *error) {
	char *function_name = "mbsys_reson7k_print_bite";
	int status = MB_SUCCESS;
	s7kr_bitereport *bitereport;
	s7kr_bitefield *bitefield;
	s7k_time *s7ktime;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int i, j;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       bite:      %p\n", (void *)bite);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &bite->header, error);

	/* Reson 7k BITE (record 7021) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     number_reports:             %u\n", first, bite->number_reports);
	for (i = 0; i < bite->number_reports; i++) {
		bitereport = &(bite->reports[i]);
		fprintf(stderr, "%s     source_name:                %s\n", first, bitereport->source_name);
		fprintf(stderr, "%s     source_address:             %u\n", first, bitereport->source_address);
		fprintf(stderr, "%s     frequency:                  %f\n", first, bitereport->reserved);
		fprintf(stderr, "%s     enumerator:                 %u\n", first, bitereport->reserved2);
		s7ktime = &(bitereport->downlink_time);
		fprintf(stderr, "%s     downlink_time:              %4.4d/%3.3d %2.2d:%2.2d:%9.6f\n", first, s7ktime->Year, s7ktime->Day,
		        s7ktime->Hours, s7ktime->Minutes, s7ktime->Seconds);
		s7ktime = &(bitereport->uplink_time);
		fprintf(stderr, "%s     uplink_time:                %4.4d/%3.3d %2.2d:%2.2d:%9.6f\n", first, s7ktime->Year, s7ktime->Day,
		        s7ktime->Hours, s7ktime->Minutes, s7ktime->Seconds);
		s7ktime = &(bitereport->bite_time);
		fprintf(stderr, "%s     bite_time:                  %4.4d/%3.3d %2.2d:%2.2d:%9.6f\n", first, s7ktime->Year, s7ktime->Day,
		        s7ktime->Hours, s7ktime->Minutes, s7ktime->Seconds);
		fprintf(stderr, "%s     status:                     %u\n", first, bitereport->status);
		fprintf(stderr, "%s     number_bite:                %u\n", first, bitereport->number_bite);
		fprintf(stderr, "%s     bite_status:                ", first);
		for (j = 0; j < 4; j++)
			fprintf(stderr, "%u ", bitereport->bite_status[j]);
		fprintf(stderr, "\n");
		for (j = 0; j < bitereport->number_bite; j++) {
			bitefield = &(bitereport->bitefield[j]);
			fprintf(stderr, "%s     reserved[%2d]:               %u\n", first, j, bitefield->reserved);
			fprintf(stderr, "%s     name[%2d]:                   %s\n", first, j, bitefield->name);
			fprintf(stderr, "%s     device_type[%2d]:            %d\n", first, j, bitefield->device_type);
			fprintf(stderr, "%s     minimum[%2d]:                %f\n", first, j, bitefield->minimum);
			fprintf(stderr, "%s     maximum[%2d]:                %f\n", first, j, bitefield->maximum);
			fprintf(stderr, "%s     value[%2d]:                  %f\n", first, j, bitefield->value);
		}
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_v37ksonarsourceversion(int verbose, s7kr_v37ksonarsourceversion *v37ksonarsourceversion, int *error) {
	char *function_name = "mbsys_reson7k_print_v37ksonarsourceversion";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       v37ksonarsourceversion: %p\n", (void *)v37ksonarsourceversion);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &v37ksonarsourceversion->header, error);

	/* Reson 7k center version (record 7022) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     version:                    %s\n", first, v37ksonarsourceversion->version);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_v38kwetendversion(int verbose, s7kr_v38kwetendversion *v38kwetendversion, int *error) {
	char *function_name = "mbsys_reson7k_print_v38kwetendversion";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       v38kwetendversion:      %p\n", (void *)v38kwetendversion);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &v38kwetendversion->header, error);

	/* Reson 7k 8k wet end version (record 7023) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     version:                    %s\n", first, v38kwetendversion->version);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_rawdetection(int verbose, s7kr_rawdetection *rawdetection, int *error) {
	char *function_name = "mbsys_reson7k_print_rawdetection";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       rawdetection:      %p\n", (void *)rawdetection);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &rawdetection->header, error);

	/* print Reson 7k raw detection (record 7027) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     serial_number:              %llu\n", first, rawdetection->serial_number);
	fprintf(stderr, "%s     ping_number:                %u\n", first, rawdetection->ping_number);
	fprintf(stderr, "%s     multi_ping:                 %u\n", first, rawdetection->multi_ping);
	fprintf(stderr, "%s     number_beams:               %u\n", first, rawdetection->number_beams);
	fprintf(stderr, "%s     data_field_size:            %d\n", first, rawdetection->data_field_size);
	fprintf(stderr, "%s     detection_algorithm:        %d\n", first, rawdetection->detection_algorithm);
	fprintf(stderr, "%s     detection_flags:            %d\n", first, rawdetection->flags);
	fprintf(stderr, "%s     sampling_rate:              %f\n", first, rawdetection->sampling_rate);
	fprintf(stderr, "%s     tx_angle:                   %f\n", first, rawdetection->tx_angle);
	fprintf(stderr, "%s     applied_roll:                   %f\n", first, rawdetection->applied_roll);
	fprintf(stderr, "%s     reserved:                   ", first);
	for (i = 0; i < 15; i++)
		fprintf(stderr, "%u ", rawdetection->reserved[i]);
	fprintf(stderr, "\n%s     beam	beam_descriptor detection_point rx_angle flags quality uncertainty\n", first);
	fprintf(stderr, "%s     ----------------------------------------------------------------------\n", first);
	for (i = 0; i < rawdetection->number_beams; i++) {
		fprintf(stderr, "%s     %3d %u %f %f %u %u %f\n", first, i, rawdetectiondata->beam_descriptor[i],
		        rawdetectiondata->detection_point[i], rawdetectiondata->rx_angle[i], rawdetectiondata->flags[i],
		        rawdetectiondata->quality[i], rawdetectiondata->uncertainty[i], , rawdetectiondata->signal_strength[i], 
		        rawdetectiondata->min_limit[i], rawdetectiondata->max_limit[i]);
	}
	fprintf(stderr, "%s     frequency:                   %f\n", first, rawdetection->frequency);
	fprintf(stderr, "%s     latitude:                   %f\n", first, rawdetection->latitude);
	fprintf(stderr, "%s     longitude:                   %f\n", first, rawdetection->longitude);
	fprintf(stderr, "%s     heading:                   %f\n", first, rawdetection->heading);
	fprintf(stderr, "%s     height_source:                   %f\n", first, rawdetection->height_source);
	fprintf(stderr, "%s     tide:                   %f\n", first, rawdetection->tide);
	fprintf(stderr, "%s     roll:                   %f\n", first, rawdetection->roll);
	fprintf(stderr, "%s     pitch:                   %f\n", first, rawdetection->pitch);
	fprintf(stderr, "%s     heave:                   %f\n", first, rawdetection->heave);
	fprintf(stderr, "%s     vehicle_depth:                   %f\n", first, rawdetection->vehicle_depth);
	for (i = 0; i < rawdetection->number_beams; i++) {
		fprintf(stderr, "%s     depth:                   %f\n", first, rawdetection->depth[i]);
		fprintf(stderr, "%s     alongtrack:                   %f\n", first, rawdetection->alongtrack[i]);
		fprintf(stderr, "%s     acrosstrack:                   %f\n", first, rawdetection->acrosstrack[i]);
		fprintf(stderr, "%s     pointing_angle:                   %f\n", first, rawdetection->pointing_angle[i]);
		fprintf(stderr, "%s     azimuth_angle:                   %f\n", first, rawdetection->azimuth_angle[i]);
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_snippet(int verbose, s7kr_snippet *snippet, int *error) {
	char *function_name = "mbsys_reson7k_print_snippet";
	int status = MB_SUCCESS;
	s7kr_snippetdataseries *snippetdataseries;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int i, j;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       snippet:      %p\n", (void *)snippet);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &snippet->header, error);

	/* print Reson 7k snippet (record 7028) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     serial_number:              %llu\n", first, snippet->serial_number);
	fprintf(stderr, "%s     ping_number:                %u\n", first, snippet->ping_number);
	fprintf(stderr, "%s     multi_ping:                 %u\n", first, snippet->multi_ping);
	fprintf(stderr, "%s     number_beams:               %u\n", first, snippet->number_beams);
	fprintf(stderr, "%s     error_flag:                 %u\n", first, snippet->error_flag);
	fprintf(stderr, "%s     control_flags:              %u\n", first, snippet->control_flags);
	fprintf(stderr, "%s     flags:              %u\n", first, snippet->flags);
	for (i = 0; i < 6; i++)
		fprintf(stderr, "%u ", snippet->reserved[i]);
	for (i = 0; i < snippet->number_beams; i++) {
		snippetdataseries = &(snippet->snippetdataseries[i]);
		fprintf(stderr, "%s     beam: %u begin:%u detect:%u end:%u\n", first, snippetdataseries->beam_number,
		        snippetdataseries->begin_sample, snippetdataseries->detect_sample, snippetdataseries->end_sample);
		for (j = 0; j < snippetdataseries->end_sample - snippetdataseries->begin_sample + 1; j++)
			fprintf(stderr, "%s     amplitude[%d]:%d\n", first, snippetdataseries->begin_sample + j,
			        snippetdataseries->amplitude[j]);
	}
	fprintf(stderr, "%s     frequency:              %f\n", first, snippet->frequency);
	fprintf(stderr, "%s     latitude:              %f\n", first, snippet-latitude);
	fprintf(stderr, "%s     longitude:              %f\n", first, snippet->longitude);
	fprintf(stderr, "%s     heading:              %f\n", first, snippet->heading);
	for (i = 0; i < snippet->number_beams; i++) {
		fprintf(stderr, "%s     beam_alongtrack:              %f\n", first, snippet->beam_alongtrack);
		fprintf(stderr, "%s     beam_acrosstrack:              %f\n", first, snippet->beam_acrosstrack);
		fprintf(stderr, "%s     center_sample:              %f\n", first, snippet->center_sample);
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/* Reson 7k vernier Processing Data Filtered (Record 7029) */


/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_installation(int verbose, s7kr_installation *installation, int *error) {
	char *function_name = "mbsys_reson7k_print_installation";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       installation:      %p\n", (void *)installation);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &installation->header, error);

	/* print Reson 7k sonar installation parameters (record 7030) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     frequency:                  %f\n", first, installation->frequency);
	fprintf(stderr, "%s     firmware_version_len:       %d\n", first, installation->firmware_version_len);
	fprintf(stderr, "%s     firmware_version:           %s\n", first, installation->firmware_version);
	fprintf(stderr, "%s     software_version_len:       %d\n", first, installation->software_version_len);
	fprintf(stderr, "%s     software_version:           %s\n", first, installation->software_version);
	fprintf(stderr, "%s     s7k_version_len:            %d\n", first, installation->s7k_version_len);
	fprintf(stderr, "%s     s7k_version:                %s\n", first, installation->s7k_version);
	fprintf(stderr, "%s     protocal_version_len:       %d\n", first, installation->protocal_version_len);
	fprintf(stderr, "%s     protocal_version:           %s\n", first, installation->protocal_version);
	fprintf(stderr, "%s     transmit_x:                 %f\n", first, installation->transmit_x);
	fprintf(stderr, "%s     transmit_y:                 %f\n", first, installation->transmit_y);
	fprintf(stderr, "%s     transmit_z:                 %f\n", first, installation->transmit_z);
	fprintf(stderr, "%s     transmit_roll:              %f\n", first, installation->transmit_roll);
	fprintf(stderr, "%s     transmit_pitch:             %f\n", first, installation->transmit_pitch);
	fprintf(stderr, "%s     transmit_heading:           %f\n", first, installation->transmit_heading);
	fprintf(stderr, "%s     receive_x:                  %f\n", first, installation->receive_x);
	fprintf(stderr, "%s     receive_y:                  %f\n", first, installation->receive_y);
	fprintf(stderr, "%s     receive_z:                  %f\n", first, installation->receive_z);
	fprintf(stderr, "%s     receive_roll:               %f\n", first, installation->receive_roll);
	fprintf(stderr, "%s     receive_pitch:              %f\n", first, installation->receive_pitch);
	fprintf(stderr, "%s     receive_heading:            %f\n", first, installation->receive_heading);
	fprintf(stderr, "%s     motion_x:                   %f\n", first, installation->motion_x);
	fprintf(stderr, "%s     motion_y:                   %f\n", first, installation->motion_y);
	fprintf(stderr, "%s     motion_z:                   %f\n", first, installation->motion_z);
	fprintf(stderr, "%s     motion_roll:                %f\n", first, installation->motion_roll);
	fprintf(stderr, "%s     motion_pitch:               %f\n", first, installation->motion_pitch);
	fprintf(stderr, "%s     motion_heading:             %f\n", first, installation->motion_heading);
	fprintf(stderr, "%s     motion_time_delay:          %d\n", first, installation->motion_time_delay);
	fprintf(stderr, "%s     position_x:                 %f\n", first, installation->position_x);
	fprintf(stderr, "%s     position_y:                 %f\n", first, installation->position_y);
	fprintf(stderr, "%s     position_z:                 %f\n", first, installation->position_z);
	fprintf(stderr, "%s     position_time_delay:        %d\n", first, installation->position_time_delay);
	fprintf(stderr, "%s     waterline_z:                %f\n", first, installation->waterline_z);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/* Reson 7k BITE summary (Record 7031) */

/* Reson 7k Compressed Beamformed Magnitude Data (Record 7041) */

/* Reson 7k Compressed Water Column Data (Record 7042) */

/* Reson 7k Segmented Raw Detection Data (Record 7047) */

/* Reson 7k Calibrated Beam Data (Record 7048) */

/* Reson 7k Reserved (Record 7049) */

/* Reson 7k System Events (Record 7050) */

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_systemeventmessage(int verbose, s7kr_systemeventmessage *systemeventmessage, int *error) {
	char *function_name = "mbsys_reson7k_print_systemeventmessage";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       systemeventmessage:%p\n", (void *)systemeventmessage);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &systemeventmessage->header, error);

	/* print Reson 7k system event (record 7051) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     serial_number:              %llu\n", first, systemeventmessage->serial_number);
	fprintf(stderr, "%s     event_id:                   %d\n", first, systemeventmessage->event_id);
	fprintf(stderr, "%s     message_length:             %d\n", first, systemeventmessage->message_length);
	fprintf(stderr, "%s     event_identifier:           %d\n", first, systemeventmessage->event_identifier);
	fprintf(stderr, "%s     message_alloc:              %d\n", first, systemeventmessage->message_alloc);
	fprintf(stderr, "%s     message:                    %s\n", first, systemeventmessage->message);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/* Reson 7k RDR Recording Status (Record 7052) */

/* Reson 7k Subscriptions (Record 7053) */

/* Reson 7k RDR Storage Recording (Record 7054) */

/* Reson 7k Calibration Status (Record 7055) */

/* Reson 7k Calibrated Sidescan Data (record 7057) */

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_snippetbackscatteringstrength(int verbose, s7kr_snippetbackscatteringstrength *snippetbackscatteringstrength, int *error) {
	char *function_name = "mbsys_reson7k_print_snippetbackscatteringstrength";
	int status = MB_SUCCESS;
	s7kr_snippetbackscatteringstrengthtimedata *snippetbackscatteringstrengthtimedata;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int i, j;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       snippetbackscatteringstrength:      %p\n", (void *)snippetbackscatteringstrength);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &snippetbackscatteringstrength->header, error);

	/* print Reson 7k Snippet Backscattering Strength (Record 7058) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     serial_number:              %llu\n", first, snippetbackscatteringstrength->serial_number);
	fprintf(stderr, "%s     ping_number:                %u\n", first, snippetbackscatteringstrength->ping_number);
	fprintf(stderr, "%s     multi_ping:                 %u\n", first, snippetbackscatteringstrength->multi_ping);
	fprintf(stderr, "%s     number_beams:               %u\n", first, snippetbackscatteringstrength->number_beams);
	fprintf(stderr, "%s     error_flag:                 %u\n", first, snippetbackscatteringstrength->error_flag);
	fprintf(stderr, "%s     control_flags:              %u\n", first, snippetbackscatteringstrength->control_flags);
	fprintf(stderr, "%s     absorption:              %f\n", first, snippetbackscatteringstrength->absorption);
	for (i = 0; i < 6; i++)
		fprintf(stderr, "%s     reserved[%d]:                %u\n", first, i, snippetbackscatteringstrength->reserved[i]);
	for (i = 0; i < snippetbackscatteringstrength->number_beams; i++) {
		snippetbackscatteringstrengthdata = &(snippetbackscatteringstrength->snippetbackscatteringstrengthdata[i]);
		fprintf(stderr, "%s     beam: %u begin:%u bottom:%u end:%u\n", first, snippetbackscatteringstrengthdata->beam_number,
		        snippetbackscatteringstrengthdata->begin_sample, snippetbackscatteringstrengthdata->bottom_sample,
		        snippetbackscatteringstrengthdata->end_sample);
		for (j = 0; j < snippetbackscatteringstrengthdata->end_sample - snippetbackscatteringstrengthdata->begin_sample + 1; j++)
			fprintf(stderr, "%s     bs[%d]:%f\n", first, snippetbackscatteringstrengthdata->begin_sample + j,
			        snippetbackscatteringstrengthdata->bs[j]);
		for (j = 0; j < snippetbackscatteringstrengthdata->end_sample - snippetbackscatteringstrengthdata->begin_sample + 1; j++)
			fprintf(stderr, "%s     fooprints[%d]:%f\n", first, snippetbackscatteringstrengthdata->begin_sample + j,
			        snippetbackscatteringstrengthdata->footprints[j]);
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/* Reson 7k MB2 Specific Status (Record 7059) */

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_subsystem(int verbose, s7kr_subsystem *subsystem, int *error) {
	char *function_name = "mbsys_reson7k_print_subsystem";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       subsystem:         %p\n", (void *)subsystem);
	}

	/* print Reson 7k subsystem structure (part of Record 7200) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     device_identifier:          %d\n", first, subsystem->device_identifier);
	fprintf(stderr, "%s     system_enumerator:          %d\n", first, subsystem->system_enumerator);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_fileheader(int verbose, s7kr_fileheader *fileheader, int *error) {
	char *function_name = "mbsys_reson7k_print_fileheader";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       fileheader:        %p\n", (void *)fileheader);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &fileheader->header, error);

	/* print Reson 7k file header (record 7200) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     file_identifier:            0x", first);
	for (i = 0; i < 16; i++)
		fprintf(stderr, "%hhx", fileheader->file_identifier[i]);
	fprintf(stderr, "\n");
	fprintf(stderr, "%s     version:                    %d\n", first, fileheader->version);
	fprintf(stderr, "%s     reserved:                   %d\n", first, fileheader->reserved);
	fprintf(stderr, "%s     session_identifier:         %s\n", first, fileheader->session_identifier);
	fprintf(stderr, "%s     record_data_size:           %d\n", first, fileheader->record_data_size);
	fprintf(stderr, "%s     number_subsystems:          %d\n", first, fileheader->number_devices);
	fprintf(stderr, "%s     recording_name:             %s\n", first, fileheader->recording_name);
	fprintf(stderr, "%s     recording_version:          %s\n", first, fileheader->recording_version);
	fprintf(stderr, "%s     user_defined_name:          %s\n", first, fileheader->user_defined_name);
	fprintf(stderr, "%s     notes:                      %s\n", first, fileheader->notes);
	for (i = 0; i < fileheader->number_devices; i++)
		mbsys_reson7k_print_subsystem(verbose, &fileheader->subsystem[i], error);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/* Reson 7k File Catalog Record (Record 7300) */

/* Reson 7k Time Message (Record 7400) */

/* Reson 7k Remote Control (Record 7500) */

/* Reson 7k Remote Control Acknowledge (Record 7501) */

/* Reson 7k Remote Control Not Acknowledge (Record 7502) */

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_remotecontrolsettings(int verbose, s7kr_remotecontrolsettings *remotecontrolsettings, int *error) {
	char *function_name = "mbsys_reson7k_print_remotecontrolsettings";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       remotecontrolsettings:  %p\n", (void *)remotecontrolsettings);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &remotecontrolsettings->header, error);

	/* print Reson 7k remote control sonar settings (record 7503) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     serial_number:              %llu\n", first, remotecontrolsettings->serial_number);
	fprintf(stderr, "%s     ping_number:                %u\n", first, remotecontrolsettings->ping_number);
	fprintf(stderr, "%s     frequency:                  %f\n", first, remotecontrolsettings->frequency);
	fprintf(stderr, "%s     sample_rate:                %f\n", first, remotecontrolsettings->sample_rate);
	fprintf(stderr, "%s     receiver_bandwidth:         %f\n", first, remotecontrolsettings->receiver_bandwidth);
	fprintf(stderr, "%s     pulse_width:                %f\n", first, remotecontrolsettings->pulse_width);
	fprintf(stderr, "%s     pulse_type:                 %d\n", first, remotecontrolsettings->pulse_type);
	fprintf(stderr, "%s     pulse_envelope:             %d\n", first, remotecontrolsettings->pulse_envelope);
	fprintf(stderr, "%s     pulse_envelope_par:         %f\n", first, remotecontrolsettings->pulse_envelope_par);
	fprintf(stderr, "%s     pulse_mode:             %d\n", first, remotecontrolsettings->pulse_mode);
	fprintf(stderr, "%s     pulse_reserved:             %d\n", first, remotecontrolsettings->pulse_reserved);
	fprintf(stderr, "%s     max_ping_rate:              %f\n", first, remotecontrolsettings->max_ping_rate);
	fprintf(stderr, "%s     ping_period:                %f\n", first, remotecontrolsettings->ping_period);
	fprintf(stderr, "%s     range_selection:            %f\n", first, remotecontrolsettings->range_selection);
	fprintf(stderr, "%s     power_selection:            %f\n", first, remotecontrolsettings->power_selection);
	fprintf(stderr, "%s     gain_selection:             %f\n", first, remotecontrolsettings->gain_selection);
	fprintf(stderr, "%s     control_flags:              %d\n", first, remotecontrolsettings->control_flags);
	fprintf(stderr, "%s     projector_id:         %d\n", first, remotecontrolsettings->projector_id);
	fprintf(stderr, "%s     steering_vertical:          %f\n", first, remotecontrolsettings->steering_vertical);
	fprintf(stderr, "%s     steering_horizontal:        %f\n", first, remotecontrolsettings->steering_horizontal);
	fprintf(stderr, "%s     beamwidth_vertical:         %f\n", first, remotecontrolsettings->beamwidth_vertical);
	fprintf(stderr, "%s     beamwidth_horizontal:       %f\n", first, remotecontrolsettings->beamwidth_horizontal);
	fprintf(stderr, "%s     focal_point:                %f\n", first, remotecontrolsettings->focal_point);
	fprintf(stderr, "%s     projector_weighting:        %d\n", first, remotecontrolsettings->projector_weighting);
	fprintf(stderr, "%s     projector_weighting_par:    %f\n", first, remotecontrolsettings->projector_weighting_par);
	fprintf(stderr, "%s     transmit_flags:             %d\n", first, remotecontrolsettings->transmit_flags);
	fprintf(stderr, "%s     hydrophone_id:        %d\n", first, remotecontrolsettings->hydrophone_id);
	fprintf(stderr, "%s     receive_weighting:          %d\n", first, remotecontrolsettings->receive_weighting);
	fprintf(stderr, "%s     receive_weighting_par:      %f\n", first, remotecontrolsettings->receive_weighting_par);
	fprintf(stderr, "%s     receive_flags:              %d\n", first, remotecontrolsettings->receive_flags);
	fprintf(stderr, "%s     range_minimum:              %f\n", first, remotecontrolsettings->range_minimum);
	fprintf(stderr, "%s     range_maximum:              %f\n", first, remotecontrolsettings->range_maximum);
	fprintf(stderr, "%s     depth_minimum:              %f\n", first, remotecontrolsettings->depth_minimum);
	fprintf(stderr, "%s     depth_maximum:              %f\n", first, remotecontrolsettings->depth_maximum);
	fprintf(stderr, "%s     absorption:                 %f\n", first, remotecontrolsettings->absorption);
	fprintf(stderr, "%s     sound_velocity:             %f\n", first, remotecontrolsettings->sound_velocity);
	fprintf(stderr, "%s     spreading:                  %f\n", first, remotecontrolsettings->spreading);
	fprintf(stderr, "%s     operation_mode:                   %u\n", first, remotecontrolsettings->);
	fprintf(stderr, "%s     autofilter_window:                   %u\n", first, remotecontrolsettings->autofilter_window);
	fprintf(stderr, "%s     tx_offset_x:                %f\n", first, remotecontrolsettings->tx_offset_x);
	fprintf(stderr, "%s     tx_offset_y:                %f\n", first, remotecontrolsettings->tx_offset_y);
	fprintf(stderr, "%s     tx_offset_z:                %f\n", first, remotecontrolsettings->tx_offset_z);
	fprintf(stderr, "%s     head_tilt_x:                %f\n", first, remotecontrolsettings->head_tilt_x);
	fprintf(stderr, "%s     head_tilt_y:                %f\n", first, remotecontrolsettings->head_tilt_y);
	fprintf(stderr, "%s     head_tilt_z:                %f\n", first, remotecontrolsettings->head_tilt_z);
	fprintf(stderr, "%s     ping_state:                %d\n", first, remotecontrolsettings->ping_state);
	fprintf(stderr, "%s     beam_angle_mode:            %d\n", first, remotecontrolsettings->beam_angle_mode);
	fprintf(stderr, "%s     r7kcenter_mode:             %d\n", first, remotecontrolsettings->r7kcenter_mode);
	fprintf(stderr, "%s     gate_depth_min:             %f\n", first, remotecontrolsettings->gate_depth_min);
	fprintf(stderr, "%s     gate_depth_max:             %f\n", first, remotecontrolsettings->gate_depth_max);
	fprintf(stderr, "%s     trigger_width:                %f\n", first, remotecontrolsettings->trigger_width);
	fprintf(stderr, "%s     trigger_offset:                %f\n", first, remotecontrolsettings->trigger_offset);
	fprintf(stderr, "%s     projector_selection:                %d\n", first, remotecontrolsettings->projector_selection);
	for (i = 0; i < 2; i++)
		fprintf(stderr, "%s     reserved2[i]:               %d\n", first, remotecontrolsettings->reserved2[i]);
	fprintf(stderr, "%s     alternate_gain:                %f\n", first, remotecontrolsettings->alternate_gain);
	fprintf(stderr, "%s     vernier_filter:                %u\n", first, remotecontrolsettings->vernier_filter);
	fprintf(stderr, "%s     reserved3:                %u\n", first, remotecontrolsettings->reserved3);
	fprintf(stderr, "%s     custom_beams:                %d\n", first, remotecontrolsettings->custom_beams);
	fprintf(stderr, "%s     coverage_angle:                %f\n", first, remotecontrolsettings->coverage_angle);
	fprintf(stderr, "%s     coverage_mode:                %u\n", first, remotecontrolsettings->coverage_mode);
	fprintf(stderr, "%s     quality_filter:                %u\n", first, remotecontrolsettings->quality_filter);
	fprintf(stderr, "%s     received_steering:                %f\n", first, remotecontrolsettings->received_steering);
	fprintf(stderr, "%s     flexmode_coverage:                %f\n", first, remotecontrolsettings->flexmode_coverage);
	fprintf(stderr, "%s     flexmode_steering:                %f\n", first, remotecontrolsettings->flexmode_steering);
	fprintf(stderr, "%s     constant_spacing:                %f\n", first, remotecontrolsettings->constant_spacing);
	fprintf(stderr, "%s    	beam_mode:                %d\n", first, remotecontrolsettings->beam_mode);
	fprintf(stderr, "%s     depth_gate_tilt:                %f\n", first, remotecontrolsettings->depth_gate_tilt);
	fprintf(stderr, "%s     applied_frequency:                %f\n", first, remotecontrolsettings->applied_frequency);
	fprintf(stderr, "%s     element_number:                %d\n", first, remotecontrolsettings->element_number);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/* Reson 7k Common System Settings (Record 7504) */

/* Reson 7k SV Filtering (record 7510) */

/* Reson 7k System Lock Status (record 7511) */

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_soundvelocity(int verbose, s7kr_soundvelocity *soundvelocity, int *error) {
	char *function_name = "mbsys_reson7k_print_soundvelocity";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       soundvelocity:     %p\n", (void *)soundvelocity);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &soundvelocity->header, error);

	/* print Reson 7k Sound Velocity (record 7610) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     soundvelocity:              %f\n", first, soundvelocity->soundvelocity);
	
	/* Optional data */

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_absorptionloss(int verbose, s7kr_absorptionloss *absorptionloss, int *error) {
	char *function_name = "mbsys_reson7k_print_absorptionloss";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       absorptionloss:    %p\n", (void *)absorptionloss);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &absorptionloss->header, error);

	/* print Reson 7k Absorption Loss (record 7611) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     absorptionloss:             %f\n", first, absorptionloss->absorptionloss);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_spreadingloss(int verbose, s7kr_spreadingloss *spreadingloss, int *error) {
	char *function_name = "mbsys_reson7k_print_spreadingloss";
	int status = MB_SUCCESS;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       spreadingloss:     %p\n", (void *)spreadingloss);
	}

	/* print Reson 7k data record header information */
	mbsys_reson7k_print_header(verbose, &spreadingloss->header, error);

	/* print Reson 7k Spreading Loss (record 7612) */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%sStructure Contents:\n", first);
	fprintf(stderr, "%s     spreadingloss:              %f\n", first, spreadingloss->spreadingloss);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss,
                             int *error) {
	char *function_name = "mbsys_reson7k_dimensions";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_bathymetry *bathymetry;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get beam and pixel numbers */
		bathymetry = (s7kr_bathymetry *)&store->bathymetry;
		*nbath = bathymetry->number_beams;
		*namp = *nbath;
		*nss = 0;
	}
	else {
		/* get beam and pixel numbers */
		*nbath = 0;
		*namp = 0;
		*nss = 0;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		fprintf(stderr, "dbg2       nbath:      %d\n", *nbath);
		fprintf(stderr, "dbg2        namp:      %d\n", *namp);
		fprintf(stderr, "dbg2        nss:       %d\n", *nss);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_pingnumber(int verbose, void *mbio_ptr, int *pingnumber, int *error) {
	char *function_name = "mbsys_reson7k_pingnumber";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_bathymetry *bathymetry;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *)mb_io_ptr->store_data;

	/* extract data from structure */
	bathymetry = (s7kr_bathymetry *)&store->bathymetry;
	*pingnumber = bathymetry->ping_number;

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       pingnumber: %d\n", *pingnumber);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_sonartype(int verbose, void *mbio_ptr, void *store_ptr, int *sonartype, int *error) {
	char *function_name = "mbsys_reson7k_sonartype";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *)store_ptr;

	/* get sonar type */
	*sonartype = MB_TOPOGRAPHY_TYPE_MULTIBEAM;

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       sonartype:  %d\n", *sonartype);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_sidescantype(int verbose, void *mbio_ptr, void *store_ptr, int *ss_type, int *error) {
	char *function_name = "mbsys_reson7k_sidescantype";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *)store_ptr;

	/* get sidescan type */
	*ss_type = MB_SIDESCAN_LINEAR;

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ss_type:    %d\n", *ss_type);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_preprocess(int verbose,     /* in: verbosity level set on command line 0..N */
                             void *mbio_ptr,  /* in: see mb_io.h:/^struct mb_io_struct/ */
                             void *store_ptr, /* in: see mbsys_reson7k.h:/^struct mbsys_reson7k_struct/ */
                             void *platform_ptr, void *preprocess_pars_ptr, int *error) {
	char *function_name = "mbsys_reson7k_preprocess";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	struct mb_platform_struct *platform;
	struct mb_preprocess_struct *pars;

	/* data structure pointers */
	s7k_header *header;
	// s7kr_reference			*reference;
	// s7kr_sensoruncal		*sensoruncal;
	// s7kr_sensorcal			*sensorcal;
	// s7kr_position 			*position;
	// s7kr_customattitude		*customattitude;
	// s7kr_tide				*tide;
	// s7kr_altitude			*altituderec;
	// s7kr_motion				*motion;
	// s7kr_depth				*depth;
	// s7kr_svp				*svp;
	// s7kr_ctd				*ctd;
	// s7kr_geodesy			*geodesy;
	// s7kr_rollpitchheave 	*rollpitchheave;
	// s7kr_heading			*headingrec;
	// s7kr_surveyline			*surveyline;
	// s7kr_navigation			*navigation;
	// s7kr_attitude			*attitude;
	// s7kr_fsdwss 			*fsdwsslo;
	// s7kr_fsdwss 			*fsdwsshi;
	// s7kr_fsdwsb 			*fsdwsb;
	// s7k_fsdwchannel 		*fsdwchannel;
	// s7k_fsdwssheader 		*fsdwssheader;
	// s7k_fsdwsegyheader 		*fsdwsegyheader;
	s7kr_bluefin *bluefin;
	s7kr_volatilesettings *volatilesettings;
	s7kr_matchfilter *matchfilter;
	s7kr_beamgeometry *beamgeometry;
	s7kr_bathymetry *bathymetry;
	s7kr_backscatter *backscatter;
	s7kr_beam *beam;
	// s7kr_v2pingmotion		*v2pingmotion;
	s7kr_v2detectionsetup *v2detectionsetup;
	// s7kr_v2beamformed		*v2beamformed;
	s7kr_verticaldepth *verticaldepth;
	s7kr_v2detection *v2detection;
	s7kr_v2rawdetection *v2rawdetection;
	// s7kr_v2snippet			*v2snippet;
	// s7kr_calibratedsnippet 	*calibratedsnippet;
	// s7kr_processedsidescan	*processedsidescan;
	s7kr_image *image;
	// s7kr_fileheader			*fileheader;
	// s7kr_installation		*installation;
	s7kr_remotecontrolsettings *remotecontrolsettings;

	/* control parameters */
	int ss_source = R7KRECID_None;

	/* kluge parameters */
	int kluge_beampatternsnell = MB_NO;
	double kluge_beampatternsnellfactor = 1.0;
	int kluge_soundspeedsnell = MB_NO;
	double kluge_soundspeedsnellfactor = 1.0;
	int kluge_zeroattitudecorrection = MB_NO;
	int kluge_zeroalongtrackangles = MB_NO;

	/* variables for beam angle calculation */
	mb_3D_orientation tx_align;
	mb_3D_orientation tx_orientation;
	double tx_steer;
	mb_3D_orientation rx_align;
	mb_3D_orientation rx_orientation;
	double rx_steer;
	double reference_heading;
	double beamAzimuth;
	double beamDepression;

	s7k_time s7kTime;
	int time_i[7];
	int time_j[5];
	double time_d = 0.0;
	double navlon = 0.0;
	double navlat = 0.0;
	double speed = 0.0;
	double altitude = 0.0;
	double sensordepth = 0.0;
	double heading = 0.0;
	double beamheading, beamheadingr;
	double roll = 0.0;
	double rollr, beamroll, beamrollr;
	double pitch = 0.0;
	double pitchr, beampitch, beampitchr;
	double heave = 0.0;
	double beamheave;
	double soundspeed;
	double soundspeednew;
	double soundspeedsnellfactor = 1.0;
	double theta, phi;
	double rr, xx, zz;
	double mtodeglon, mtodeglat, headingx, headingy;
	double dx, dy, dt;
	int jnav = 0;
	int jsensordepth = 0;
	int jheading = 0;
	int jaltitude = 0;
	int jattitude = 0;
	int jsoundspeed = 0;
	int j1, j2;
	int interp_status = MB_SUCCESS;
	int interp_error = MB_ERROR_NO_ERROR;
	double *pixel_size;
	double *swath_width;

	int i, j;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:                   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:                  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       platform_ptr:               %p\n", (void *)platform_ptr);
		fprintf(stderr, "dbg2       preprocess_pars_ptr:        %p\n", (void *)preprocess_pars_ptr);
	}

	/* always successful */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);
	assert(preprocess_pars_ptr != NULL);

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointers */
	store = (struct mbsys_reson7k_struct *)store_ptr;
	platform = (struct mb_platform_struct *)platform_ptr;
	pars = (struct mb_preprocess_struct *)preprocess_pars_ptr;

	/* get saved values */
	pixel_size = (double *)&mb_io_ptr->saved1;
	swath_width = (double *)&mb_io_ptr->saved2;

	/* get kluges */
	for (i = 0; i < pars->n_kluge; i++) {
		if (pars->kluge_id[i] == MB_PR_KLUGE_BEAMTWEAK) {
			kluge_beampatternsnell = MB_YES;
			kluge_beampatternsnellfactor = *((double *)&pars->kluge_pars[i * MB_PR_KLUGE_PAR_SIZE]);
		}
		else if (pars->kluge_id[i] == MB_PR_KLUGE_SOUNDSPEEDTWEAK) {
			kluge_soundspeedsnell = MB_YES;
			kluge_soundspeedsnellfactor = *((double *)&pars->kluge_pars[i * MB_PR_KLUGE_PAR_SIZE]);
		}
		else if (pars->kluge_id[i] == MB_PR_KLUGE_ZEROATTITUDECORRECTION) {
			kluge_zeroattitudecorrection = MB_YES;
		}
		else if (pars->kluge_id[i] == MB_PR_KLUGE_ZEROALONGTRACKANGLES) {
			kluge_zeroalongtrackangles = MB_YES;
		}
	}

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       target_sensor:                 %d\n", pars->target_sensor);
		fprintf(stderr, "dbg2       timestamp_changed:             %d\n", pars->timestamp_changed);
		fprintf(stderr, "dbg2       time_d:                        %f\n", pars->time_d);
		fprintf(stderr, "dbg2       n_nav:                         %d\n", pars->n_nav);
		fprintf(stderr, "dbg2       nav_time_d:                    %p\n", pars->nav_time_d);
		fprintf(stderr, "dbg2       nav_lon:                       %p\n", pars->nav_lon);
		fprintf(stderr, "dbg2       nav_lat:                       %p\n", pars->nav_lat);
		fprintf(stderr, "dbg2       nav_speed:                     %p\n", pars->nav_speed);
		fprintf(stderr, "dbg2       n_sensordepth:                 %d\n", pars->n_sensordepth);
		fprintf(stderr, "dbg2       sensordepth_time_d:            %p\n", pars->sensordepth_time_d);
		fprintf(stderr, "dbg2       sensordepth_sensordepth:       %p\n", pars->sensordepth_sensordepth);
		fprintf(stderr, "dbg2       n_heading:                     %d\n", pars->n_heading);
		fprintf(stderr, "dbg2       heading_time_d:                %p\n", pars->heading_time_d);
		fprintf(stderr, "dbg2       heading_heading:               %p\n", pars->heading_heading);
		fprintf(stderr, "dbg2       n_altitude:                    %d\n", pars->n_altitude);
		fprintf(stderr, "dbg2       altitude_time_d:               %p\n", pars->altitude_time_d);
		fprintf(stderr, "dbg2       altitude_altitude:             %p\n", pars->altitude_altitude);
		fprintf(stderr, "dbg2       n_attitude:                    %d\n", pars->n_attitude);
		fprintf(stderr, "dbg2       attitude_time_d:               %p\n", pars->attitude_time_d);
		fprintf(stderr, "dbg2       attitude_roll:                 %p\n", pars->attitude_roll);
		fprintf(stderr, "dbg2       attitude_pitch:                %p\n", pars->attitude_pitch);
		fprintf(stderr, "dbg2       attitude_heave:                %p\n", pars->attitude_heave);
		fprintf(stderr, "dbg2       no_change_survey:              %d\n", pars->no_change_survey);
		fprintf(stderr, "dbg2       multibeam_sidescan_source:     %d\n", pars->multibeam_sidescan_source);
		fprintf(stderr, "dbg2       modify_soundspeed:             %d\n", pars->modify_soundspeed);
		fprintf(stderr, "dbg2       recalculate_bathymetry:        %d\n", pars->recalculate_bathymetry);
		fprintf(stderr, "dbg2       sounding_amplitude_filter:     %d\n", pars->sounding_amplitude_filter);
		fprintf(stderr, "dbg2       sounding_amplitude_threshold:  %f\n", pars->sounding_amplitude_threshold);
		fprintf(stderr, "dbg2       ignore_water_column:           %d\n", pars->ignore_water_column);
		fprintf(stderr, "dbg2       n_kluge:                       %d\n", pars->n_kluge);
		for (i = 0; i < pars->n_kluge; i++) {
			fprintf(stderr, "dbg2       kluge_id[%d]:                    %d\n", i, pars->kluge_id[i]);
			if (pars->kluge_id[i] == MB_PR_KLUGE_BEAMTWEAK) {
				fprintf(stderr, "dbg2       kluge_beampatternsnell:        %d\n", kluge_beampatternsnell);
				fprintf(stderr, "dbg2       kluge_beampatternsnellfactor:  %f\n", kluge_beampatternsnellfactor);
			}
			else if (pars->kluge_id[i] == MB_PR_KLUGE_SOUNDSPEEDTWEAK) {
				fprintf(stderr, "dbg2       kluge_soundspeedsnell:         %d\n", kluge_soundspeedsnell);
				fprintf(stderr, "dbg2       kluge_soundspeedsnellfactor:   %f\n", kluge_soundspeedsnellfactor);
			}
			else if (pars->kluge_id[i] == MB_PR_KLUGE_ZEROATTITUDECORRECTION) {
				fprintf(stderr, "dbg2       kluge_zeroattitudecorrection:  %d\n", kluge_zeroattitudecorrection);
			}
			else if (pars->kluge_id[i] == MB_PR_KLUGE_ZEROALONGTRACKANGLES) {
				fprintf(stderr, "dbg2       kluge_zeroalongtrackangles:    %d\n", kluge_zeroalongtrackangles);
			}
		}
	}

	/* deal with a survey record */
	if (store->kind == MB_DATA_DATA) {
		bathymetry = &(store->bathymetry);
		v2detection = &(store->v2detection);
		v2detectionsetup = &(store->v2detectionsetup);
		v2rawdetection = &(store->v2rawdetection);
		bluefin = &(store->bluefin);
		volatilesettings = &(store->volatilesettings);
		matchfilter = &(store->matchfilter);
		beamgeometry = &(store->beamgeometry);
		remotecontrolsettings = &(store->remotecontrolsettings);
		backscatter = &(store->backscatter);
		beam = &(store->beam);
		verticaldepth = &(store->verticaldepth);
		image = &(store->image);
		bathymetry = &(store->bathymetry);

		/* print out record headers */
		if (store->read_volatilesettings == MB_YES) {
			header = &(volatilesettings->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int)header->s7kTime.Seconds;
			time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 1)
				fprintf(stderr,
				        "R7KRECID_7kVolatileSonarSettings:  7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
				        time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], header->RecordNumber);
		}
		if (store->read_matchfilter == MB_YES) {
			header = &(matchfilter->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int)header->s7kTime.Seconds;
			time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 1)
				fprintf(stderr,
				        "R7KRECID_7kMatchFilter:            7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
				        time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], header->RecordNumber);
		}
		if (store->read_beamgeometry == MB_YES) {
			header = &(beamgeometry->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int)header->s7kTime.Seconds;
			time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 1)
				fprintf(stderr,
				        "R7KRECID_7kBeamGeometry:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d "
				        "beams:%d\n",
				        time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], header->RecordNumber,
				        beamgeometry->number_beams);
		}
		if (store->read_remotecontrolsettings == MB_YES) {
			header = &(remotecontrolsettings->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int)header->s7kTime.Seconds;
			time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 1)
				fprintf(stderr,
				        "R7KRECID_7kremotecontrolsettings:  7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
				        time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], header->RecordNumber);
		}
		if (store->read_backscatter == MB_YES) {
			header = &(backscatter->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int)header->s7kTime.Seconds;
			time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 1)
				fprintf(stderr,
				        "R7KRECID_7kBackscatterImageData:   7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d "
				        "ping:%d samples:%d\n",
				        time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], header->RecordNumber,
				        backscatter->ping_number, backscatter->number_samples);
		}
		if (store->read_beam == MB_YES) {
			header = &(beam->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int)header->s7kTime.Seconds;
			time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 1)
				fprintf(stderr,
				        "R7KRECID_7kBeamData: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d "
				        "beams:%d samples:%d\n",
				        time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], header->RecordNumber,
				        beam->ping_number, beam->number_beams, beam->number_samples);
		}
		if (store->read_verticaldepth == MB_YES) {
			header = &(verticaldepth->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int)header->s7kTime.Seconds;
			time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 1)
				fprintf(stderr,
				        "R7KRECID_7kVerticalDepth: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d\n",
				        time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], header->RecordNumber,
				        verticaldepth->ping_number);
		}
		if (store->read_image == MB_YES) {
			header = &(image->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int)header->s7kTime.Seconds;
			time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 1)
				fprintf(stderr,
				        "R7KRECID_7kImageData:              7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d "
				        "ping:%d width:%d height:%d\n",
				        time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], header->RecordNumber,
				        image->ping_number, image->width, image->height);
		}
		if (store->read_bathymetry != MB_YES) {
			status = MB_FAILURE;
			*error = MB_ERROR_IGNORE;
		}
		else {
			header = &(bathymetry->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int)header->s7kTime.Seconds;
			time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 1)
				fprintf(stderr,
				        "R7KRECID_7kBathymetricData:        7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d "
				        "ping:%d beams:%d\n",
				        time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], header->RecordNumber,
				        bathymetry->ping_number, bathymetry->number_beams);

			/*--------------------------------------------------------------*/
			/* apply any required fixes to survey data beam flags */
			/*--------------------------------------------------------------*/
			/* fix version 4 quality flags */
			if (bathymetry->header.Version < 5) {
				for (i = 0; i < bathymetry->number_beams; i++) {
					if ((bathymetry->quality[i]) < 16) {
						if (bathymetry->range[i] > 0.007) {
							bathymetry->quality[i] = 23;
						}
						else if (bathymetry->range[i] > 0.0) {
							bathymetry->quality[i] = 20;
						}
						else {
							bathymetry->quality[i] = 0;
						}
					}
				}
			}

			/* fix early version 5 quality flags */
			else if (bathymetry->header.Version == 5 && header->s7kTime.Year < 2006) {
				for (i = 0; i < bathymetry->number_beams; i++) {
					/* phase picks */
					if ((bathymetry->quality[i]) == 8) {
						/*fprintf(stderr,"beam %d: PHASE quality: %d",i,bathymetry->quality[i]);*/
						bathymetry->quality[i] = 32 + 15;
						/*fprintf(stderr," %d\n",bathymetry->quality[i]);*/
					}
					else if ((bathymetry->quality[i]) == 4) {
						/*fprintf(stderr,"beam %d: AMPLI quality: %d",i,bathymetry->quality[i]);*/
						bathymetry->quality[i] = 16 + 15;
						/*fprintf(stderr," %d\n",bathymetry->quality[i]);*/
					}
				}
			}

			/* fix early MBARI version 5 quality flags */
			else if (bathymetry->header.Version == 5 && store->nrec_bluefinnav > 0 && header->s7kTime.Year < 2008) {
				for (i = 0; i < bathymetry->number_beams; i++) {
					/* phase picks */
					if ((bathymetry->quality[i]) == 4) {
						/*fprintf(stderr,"beam %d: PHASE quality: %d",i,bathymetry->quality[i]);*/
						bathymetry->quality[i] = 32 + 15;
						/*fprintf(stderr," %d\n",bathymetry->quality[i]);*/
					}
					else if ((bathymetry->quality[i]) == 2) {
						/*fprintf(stderr,"beam %d: AMPLI quality: %d",i,bathymetry->quality[i]);*/
						bathymetry->quality[i] = 16 + 15;
						/*fprintf(stderr," %d\n",bathymetry->quality[i]);*/
					}
				}
			}

			/* fix upgraded MBARI version 5 quality flags */
			else if (bathymetry->header.Version >= 5 && store->nrec_bluefinnav > 0 && header->s7kTime.Year <= 2010) {
				for (i = 0; i < bathymetry->number_beams; i++) {
					/* fprintf(stderr,"S Flag[%d]: %d\n",i,bathymetry->quality[i]); */
					bathymetry->quality[i] = bathymetry->quality[i] & 15;

					/* phase or amplitude picks */
					if (bathymetry->quality[i] & 8) {
						/* fprintf(stderr,"beam %d: PHASE quality: %d",i,bathymetry->quality[i]); */
						bathymetry->quality[i] += 32;
						/* fprintf(stderr," %d\n",bathymetry->quality[i]); */
					}
					else if (bathymetry->quality[i] & 4) {
						/* fprintf(stderr,"beam %d: AMPLI quality: %d",i,bathymetry->quality[i]); */
						bathymetry->quality[i] += 16;
						/* fprintf(stderr," %d\n",bathymetry->quality[i]); */
					}

					/* flagged by sonar */
					if ((bathymetry->quality[i] & 3) == 0 && bathymetry->quality[i] > 0) {
						bathymetry->quality[i] += 64;
					}
					/* fprintf(stderr,"E Flag[%d]: %d\n\n",i,bathymetry->quality[i]); */
				}
			}

			/* fix upgraded version 5 quality flags */
			else if (bathymetry->header.Version >= 5) {
				for (i = 0; i < bathymetry->number_beams; i++) {
					// fprintf(stderr,"S Flag[%d]: %d\n",i,bathymetry->quality[i]);
					bathymetry->quality[i] = bathymetry->quality[i] & 15;

					/* phase or amplitude picks */
					if (bathymetry->quality[i] & 8) {
						// fprintf(stderr,"beam %d: PHASE quality: %d",i,bathymetry->quality[i]);
						bathymetry->quality[i] += 32;
						// fprintf(stderr," %d\n",bathymetry->quality[i]);
					}
					else if (bathymetry->quality[i] & 4) {
						// fprintf(stderr,"beam %d: AMPLI quality: %d",i,bathymetry->quality[i]);
						bathymetry->quality[i] += 16;
						// fprintf(stderr," %d\n",bathymetry->quality[i]);
					}

					/* flagged by sonar */
					if ((bathymetry->quality[i] & 3) == 3
						&& pars->sounding_amplitude_filter == MB_YES
						&& (double)bathymetry->intensity[i] < pars->sounding_amplitude_threshold) {
						bathymetry->quality[i] += 64;
					}
					else if ((bathymetry->quality[i] & 3) == 3) {
					}
					else if ((bathymetry->quality[i] & 3) == 0 && bathymetry->quality[i] > 0) {
						bathymetry->quality[i] += 64;
					}
					else if (bathymetry->quality[i] > 0) {
						bathymetry->quality[i] += 64;
					}
					// fprintf(stderr,"E Flag[%d]: %d\n\n",i,bathymetry->quality[i]);
				}
			}

			/* if requested ignore water column data
			 * (will not be included in any output file) */
			if (pars->ignore_water_column == MB_YES
				&& store->read_v2beamformed == MB_YES)
				store->read_v2beamformed = MB_NO;

			/*--------------------------------------------------------------*/
			/* change timestamp if indicated */
			/*--------------------------------------------------------------*/
			if (pars->timestamp_changed == MB_YES) {
				time_d = pars->time_d;
				mb_get_date(verbose, time_d, time_i);
				mb_get_jtime(verbose, time_i, time_j);
				s7kTime.Year = time_i[0];
				s7kTime.Day = time_j[1];
				s7kTime.Hours = time_i[3];
				s7kTime.Minutes = time_i[4];
				s7kTime.Seconds = time_i[5] + 0.000001 * time_i[6];
				fprintf(stderr,
				        "Timestamp changed in function %s: "
				        "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d "
				        "| ping_number:%d\n",
				        function_name, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
				        bathymetry->ping_number);

				/* apply the timestamp to all of the relevant data records */
				if (store->read_volatilesettings == MB_YES)
					store->volatilesettings.header.s7kTime = s7kTime;
				if (store->read_matchfilter == MB_YES)
					store->matchfilter.header.s7kTime = s7kTime;
				if (store->read_beamgeometry == MB_YES)
					store->beamgeometry.header.s7kTime = s7kTime;
				if (store->read_remotecontrolsettings == MB_YES)
					store->remotecontrolsettings.header.s7kTime = s7kTime;
				if (store->read_bathymetry == MB_YES)
					store->bathymetry.header.s7kTime = s7kTime;
				if (store->read_backscatter == MB_YES)
					store->backscatter.header.s7kTime = s7kTime;
				if (store->read_beam == MB_YES)
					store->beam.header.s7kTime = s7kTime;
				if (store->read_verticaldepth == MB_YES)
					store->verticaldepth.header.s7kTime = s7kTime;
				if (store->read_image == MB_YES)
					store->image.header.s7kTime = s7kTime;
				if (store->read_v2pingmotion == MB_YES)
					store->v2pingmotion.header.s7kTime = s7kTime;
				if (store->read_v2detectionsetup == MB_YES)
					store->v2detectionsetup.header.s7kTime = s7kTime;
				if (store->read_v2beamformed == MB_YES)
					store->v2beamformed.header.s7kTime = s7kTime;
				if (store->read_v2detection == MB_YES)
					store->v2detection.header.s7kTime = s7kTime;
				if (store->read_v2rawdetection == MB_YES)
					store->v2rawdetection.header.s7kTime = s7kTime;
				if (store->read_v2snippet == MB_YES)
					store->v2snippet.header.s7kTime = s7kTime;
				if (store->read_calibratedsnippet == MB_YES)
					store->calibratedsnippet.header.s7kTime = s7kTime;
				if (store->read_processedsidescan == MB_YES)
					store->processedsidescan.header.s7kTime = s7kTime;
			}

			/*--------------------------------------------------------------*/
			/* interpolate ancillary values  */
			/*--------------------------------------------------------------*/
			interp_status = mb_linear_interp_longitude(verbose, pars->nav_time_d - 1, pars->nav_lon - 1, pars->n_nav, time_d,
			                                           &navlon, &jnav, &interp_error);
			interp_status = mb_linear_interp_latitude(verbose, pars->nav_time_d - 1, pars->nav_lat - 1, pars->n_nav, time_d,
			                                          &navlat, &jnav, &interp_error);
			interp_status = mb_linear_interp(verbose, pars->nav_time_d - 1, pars->nav_speed - 1, pars->n_nav, time_d, &speed,
			                                 &jnav, &interp_error);

			/* interpolate sensordepth */
			interp_status = mb_linear_interp(verbose, pars->sensordepth_time_d - 1, pars->sensordepth_sensordepth - 1,
			                                 pars->n_sensordepth, time_d, &sensordepth, &jsensordepth, &interp_error);

			/* interpolate heading */
			interp_status = mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
			                                         pars->n_heading, time_d, &heading, &jheading, &interp_error);

			/* interpolate altitude */
			interp_status = mb_linear_interp(verbose, pars->altitude_time_d - 1, pars->altitude_altitude - 1, pars->n_altitude,
			                                 time_d, &altitude, &jaltitude, &interp_error);

			/* interpolate attitude */
			interp_status = mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude,
			                                 time_d, &roll, &jattitude, &interp_error);
			interp_status = mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude,
			                                 time_d, &pitch, &jattitude, &interp_error);
			interp_status = mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_heave - 1, pars->n_attitude,
			                                 time_d, &heave, &jattitude, &interp_error);

			/* interpolate soundspeed */
			interp_status = mb_linear_interp(verbose, pars->soundspeed_time_d - 1, pars->soundspeed_soundspeed - 1, pars->n_soundspeed,
			                                 time_d, &soundspeednew, &jsoundspeed, &interp_error);

			/* do lever arm correction */
			if (platform != NULL) {
				/* calculate sonar position position */
				status = mb_platform_position(verbose, (void *)platform, pars->target_sensor, 0, navlon, navlat, sensordepth,
				                              heading, roll, pitch, &navlon, &navlat, &sensordepth, error);

				/* calculate sonar attitude */
				status = mb_platform_orientation_target(verbose, (void *)platform, pars->target_sensor, 0, heading, roll, pitch,
				                                        &heading, &roll, &pitch, error);
			}

			/* get local translation between lon lat degrees and meters */
			mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
			headingx = sin(DTR * heading);
			headingy = cos(DTR * heading);

			/* if a valid speed is not available calculate it */
			if (interp_status == MB_SUCCESS && speed <= 0.0) {
				if (jnav > 1) {
					j1 = jnav - 2;
					j2 = jnav - 1;
				}
				else {
					j1 = jnav - 1;
					j2 = jnav;
				}
				dx = (pars->nav_lon[j2] - pars->nav_lon[j1]) / mtodeglon;
				dy = (pars->nav_lat[j2] - pars->nav_lat[j1]) / mtodeglat;
				dt = (pars->nav_time_d[j2] - pars->nav_time_d[j1]);
				if (dt > 0.0)
					speed = sqrt(dx * dx + dy * dy) / dt;
			}

			/* if the optional data are not all available, this ping
			    is not useful, and is discarded by setting
			    *error to MB_ERROR_MISSING_NAVATTITUDE */
			if (interp_status == MB_FAILURE) {
				status = MB_FAILURE;
				*error = MB_ERROR_MISSING_NAVATTITUDE;
			}

			/*--------------------------------------------------------------*/
			/* recalculate bathymetry  */
			/*--------------------------------------------------------------*/
			if (bathymetry->optionaldata == MB_NO || pars->recalculate_bathymetry == MB_YES) {

				/* print debug statements */
				if (verbose >= 2) {
					fprintf(stderr, "\ndbg2 Recalculating bathymetry in %s: 7k ping records read:\n", function_name);
					fprintf(stderr, "dbg2      current_ping_number:           %d\n", store->current_ping_number);
					fprintf(stderr, "dbg2      read_volatilesettings:         %d\n", store->read_volatilesettings);
					fprintf(stderr, "dbg2      read_matchfilter:              %d\n", store->read_matchfilter);
					fprintf(stderr, "dbg2      read_beamgeometry:             %d\n", store->read_beamgeometry);
					fprintf(stderr, "dbg2      read_remotecontrolsettings:    %d\n", store->read_remotecontrolsettings);
					fprintf(stderr, "dbg2      read_bathymetry:               %d\n", store->read_bathymetry);
					fprintf(stderr, "dbg2      read_backscatter:              %d\n", store->read_backscatter);
					fprintf(stderr, "dbg2      read_beam:                     %d\n", store->read_beam);
					fprintf(stderr, "dbg2      read_verticaldepth:            %d\n", store->read_verticaldepth);
					fprintf(stderr, "dbg2      read_tvg:                      %d\n", store->read_tvg);
					fprintf(stderr, "dbg2      read_image:                    %d\n", store->read_image);
					fprintf(stderr, "dbg2      read_v2pingmotion:             %d\n", store->read_v2pingmotion);
					fprintf(stderr, "dbg2      read_v2detectionsetup:         %d\n", store->read_v2detectionsetup);
					fprintf(stderr, "dbg2      read_v2beamformed:             %d\n", store->read_v2beamformed);
					fprintf(stderr, "dbg2      read_v2detection:              %d\n", store->read_v2detection);
					fprintf(stderr, "dbg2      read_v2rawdetection:           %d\n", store->read_v2rawdetection);
					fprintf(stderr, "dbg2      read_v2snippet:                %d\n", store->read_v2snippet);
					fprintf(stderr, "dbg2      read_calibratedsnippet:        %d\n", store->read_calibratedsnippet);
					fprintf(stderr, "dbg2      read_processedsidescan:        %d\n", store->read_processedsidescan);
				}

				/* initialize all of the beams */
				for (i = 0; i < bathymetry->number_beams; i++) {
					if (store->read_v2rawdetection == MB_YES ||
					    (store->read_v2detection == MB_YES && store->read_v2detectionsetup == MB_YES))
						bathymetry->quality[i] = 0;
					bathymetry->depth[i] = 0.0;
					bathymetry->acrosstrack[i] = 0.0;
					bathymetry->alongtrack[i] = 0.0;
					bathymetry->pointing_angle[i] = 0.0;
					bathymetry->azimuth_angle[i] = 0.0;
				}
				// fprintf(stderr,"sonardepth:%f heave:%f\n",sonardepth,heave);

				/* set ping values */
				bathymetry->longitude = DTR * navlon;
				bathymetry->latitude = DTR * navlat;
				bathymetry->heading = DTR * heading;
				bathymetry->height_source = 1;
				bathymetry->tide = 0.0;
				bathymetry->roll = DTR * roll;
				bathymetry->pitch = DTR * pitch;
				bathymetry->heave = heave;
				if ((volatilesettings->receive_flags & 0x2) != 0) {
					bathymetry->vehicle_height = -sensordepth - heave;
				}
				else {
					bathymetry->vehicle_height = -sensordepth;
				}

				/* get ready to calculate bathymetry */
				if (volatilesettings->sound_velocity > 0.0)
					soundspeed = volatilesettings->sound_velocity;
				else if (bluefin->environmental[0].sound_speed > 0.0)
					soundspeed = bluefin->environmental[0].sound_speed;
				else
					soundspeed = 1500.0;
				rollr = DTR * roll;
				pitchr = DTR * pitch;

				/* zero atttitude correction if requested */
				if (kluge_zeroattitudecorrection == MB_YES) {
					rollr = 0.0;
					pitchr = 0.0;
				}

				/* zero alongtrack angles if requested */
				if (kluge_zeroalongtrackangles == MB_YES) {
					for (i = 0; i < bathymetry->number_beams; i++) {
						beamgeometry->angle_alongtrack[i] = 0.0;
					}
				}

				/* if requested apply kluge scaling of rx beam angles */
				if (kluge_beampatternsnell == MB_YES) {
					/*
					 * v2rawdetection record
					 */
					if (store->read_v2rawdetection == MB_YES) {
						for (i = 0; i < v2rawdetection->number_beams; i++) {
							v2rawdetection->rx_angle[i]
								= asin(MAX(-1.0, MIN(1.0, kluge_beampatternsnellfactor
													 * sin(v2rawdetection->rx_angle[i]))));
						}
					}

					/*
					 * v2detection record with or without v2detectionsetup
					 */
					if (store->read_v2detection == MB_YES) {
						for (i = 0; i < v2detection->number_beams; i++) {
							v2detection->angle_x[i]
								= asin(MAX(-1.0, MIN(1.0, kluge_beampatternsnellfactor
													 * sin(v2detection->angle_x[i]))));
						}
					}

					/*
					 * beamgeometry record
					 */
					if (store->read_beamgeometry == MB_YES) {
						for (i = 0; i < bathymetry->number_beams; i++) {
							beamgeometry->angle_acrosstrack[i] =
							    asin(MAX(-1.0, MIN(1.0, kluge_beampatternsnellfactor
												   * sin(beamgeometry->angle_acrosstrack[i]))));
						}
					}
				}

				/* Change the sound speed used to calculate bathymetry */
				if (pars->modify_soundspeed) {
					soundspeedsnellfactor = soundspeednew / soundspeed;
//fprintf(stderr,"MODIFY SOUND SPEED:  old: %.3f  new: *%.3f    ratio: %.6f\n", soundspeed, soundspeednew, soundspeedsnellfactor);
					soundspeed = soundspeednew;
					bathymetry->sound_velocity = soundspeed;
				}
				
				/* if requested apply kluge scaling of sound speed - which means
				    changing beam angles by Snell's law and changing the sound
				    speed used to calculate bathymetry */
				if (kluge_soundspeedsnell == MB_YES) {
					/*
					 * sound speed
					 */
					soundspeedsnellfactor *= kluge_soundspeedsnellfactor;
					soundspeed *= kluge_soundspeedsnellfactor;
				}

				if (pars->modify_soundspeed || kluge_soundspeedsnell == MB_YES) {
					/* change the sound speed recorded for the current ping and
					 * then use it to alter the beam angles and recalculated the
					 * bathymetry
					 */
					volatilesettings->sound_velocity = soundspeed;

					/*
					 * v2rawdetection record
					 */
					if (store->read_v2rawdetection == MB_YES) {
						for (i = 0; i < v2rawdetection->number_beams; i++) {
							v2rawdetection->rx_angle[i] =
							    asin(MAX(-1.0, MIN(1.0, soundspeedsnellfactor
												   * sin(v2rawdetection->rx_angle[i]))));
						}
					}

					/*
					 * v2detection record with or without v2detectionsetup
					 */
					if (store->read_v2detection == MB_YES) {
						for (i = 0; i < v2detection->number_beams; i++) {
							v2detection->angle_x[i]
								= asin(MAX(-1.0, MIN(1.0, soundspeedsnellfactor
													 * sin(v2detection->angle_x[i]))));
						}
					}

					/*
					 * beamgeometry record
					 */
					if (store->read_beamgeometry == MB_YES) {
						for (i = 0; i < bathymetry->number_beams; i++) {
							beamgeometry->angle_acrosstrack[i] =
							    asin(MAX(-1.0, MIN(1.0, soundspeedsnellfactor
												   * sin(beamgeometry->angle_acrosstrack[i]))));
						}
					}
				}

				/* get transducer angular offsets */
				if (platform != NULL) {
					status = mb_platform_orientation_offset(verbose, (void *)platform, pars->target_sensor, 0,
					                                        &(tx_align.heading), &(tx_align.roll), &(tx_align.pitch), error);

					status = mb_platform_orientation_offset(verbose, (void *)platform, pars->target_sensor, 1,
					                                        &(rx_align.heading), &(rx_align.roll), &(rx_align.pitch), error);
				}

				/* loop over detections as available - the 7k format has used several
				   different records over the years, so there are several different
				   cases that must be handled */

				/* case of v2rawdetection record */
				if (store->read_v2rawdetection == MB_YES) {
					for (j = 0; j < v2rawdetection->number_beams; j++) {
						/* beam id */
						i = v2rawdetection->beam_descriptor[j];

						/* get range and quality */
						bathymetry->range[i] = v2rawdetection->detection_point[j] / v2rawdetection->sampling_rate;
						bathymetry->quality[i] = v2rawdetection->quality[j];

						/* get roll at bottom return time for this beam */
						interp_status =
						    mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude,
						                     time_d + bathymetry->range[i], &beamroll, &jattitude, error);
						beamrollr = DTR * beamroll;

						/* get pitch at bottom return time for this beam */
						interp_status =
						    mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude,
						                     time_d + bathymetry->range[i], &beampitch, &jattitude, error);
						beampitchr = DTR * beampitch;

						/* get heading at bottom return time for this beam */
						interp_status = mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
						                                         pars->n_heading, time_d + bathymetry->range[i], &beamheading,
						                                         &jheading, error);
						beamheadingr = DTR * beamheading;

						/* calculate beam angles for raytracing using Jon Beaudoin's code based on:
						    Beaudoin, J., Hughes Clarke, J., and Bartlett, J. Application of
						    Surface Sound Speed Measurements in Post-Processing for Multi-Sector
						    Multibeam Echosounders : International Hydrographic Review, v.5, no.3,
						    p.26-31.
						    (http://www.omg.unb.ca/omg/papers/beaudoin_IHR_nov2004.pdf).
						   note complexity if transducer arrays are reverse mounted, as determined
						   by a mount heading angle of about 180 degrees rather than about 0 degrees.
						   If a receive array or a transmit array are reverse mounted then:
						    1) subtract 180 from the heading mount angle of the array
						    2) flip the sign of the pitch and roll mount offsets of the array
						    3) flip the sign of the beam steering angle from that array
						        (reverse TX means flip sign of TX steer, reverse RX
						        means flip sign of RX steer) */
						tx_steer = RTD * v2rawdetection->tx_angle;
						tx_orientation.roll = roll;
						tx_orientation.pitch = pitch;
						tx_orientation.heading = heading;
						rx_steer = -RTD * v2rawdetection->rx_angle[j];
						rx_orientation.roll = beamroll;
						rx_orientation.pitch = beampitch;
						rx_orientation.heading = beamheading;
						reference_heading = heading;

						status = mb_beaudoin(verbose, tx_align, tx_orientation, tx_steer, rx_align, rx_orientation, rx_steer,
						                     reference_heading, &beamAzimuth, &beamDepression, error);
						theta = 90.0 - beamDepression;
						phi = 90.0 - beamAzimuth;
						if (phi < 0.0)
							phi += 360.0;

						/* calculate bathymetry */
						rr = 0.5 * soundspeed * bathymetry->range[i];
						xx = rr * sin(DTR * theta);
						zz = rr * cos(DTR * theta);
						bathymetry->acrosstrack[i] = xx * cos(DTR * phi);
						bathymetry->alongtrack[i] = xx * sin(DTR * phi);
						bathymetry->depth[i] = zz + sensordepth - heave;
						bathymetry->pointing_angle[i] = DTR * theta;
						bathymetry->azimuth_angle[i] = DTR * phi;
						// fprintf(stderr,"beam:%d time_d:%f heading:%f %f roll:%f %f pitch:%f %f theta:%f phi:%f bath:%f %f
						// %f\n",  i,time_d + bathymetry->range[i],heading,beamheading,roll,beamroll,pitch,beampitch,theta,phi,
						// bathymetry->depth[i],bathymetry->acrosstrack[i],bathymetry->alongtrack[i]);
					}
				}

				/* case of v2detection record with v2detectionsetup */
				else if (store->read_v2detection == MB_YES && store->read_v2detectionsetup == MB_YES) {
					for (j = 0; j < v2detection->number_beams; j++) {
						i = v2detectionsetup->beam_descriptor[j];

						bathymetry->range[i] = v2detection->range[j];
						bathymetry->quality[i] = v2detectionsetup->quality[j];

						/* compensate for pitch if not already compensated */
						if ((volatilesettings->transmit_flags & 0xF) != 0) {
							beampitch = 0.0;
						}
						else {
							beampitch = pitch;
						}
						beampitchr = DTR * beampitch;

						/* compensate for roll if not already compensated */
						if ((volatilesettings->receive_flags & 0x1) != 0) {
							beamroll = 0.0;
						}
						else {
							/* get roll at bottom return time for this beam */
							interp_status =
							    mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude,
							                     time_d + bathymetry->range[i], &beamroll, &jattitude, error);
						}
						beamrollr = DTR * beamroll;

						/* get heading at bottom return time for this beam */
						interp_status = mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
						                                         pars->n_heading, time_d + bathymetry->range[i], &beamheading,
						                                         &jheading, error);
						beamheadingr = DTR * beamheading;

						/* calculate beam angles for raytracing using Jon Beaudoin's code based on:
						    Beaudoin, J., Hughes Clarke, J., and Bartlett, J. Application of
						    Surface Sound Speed Measurements in Post-Processing for Multi-Sector
						    Multibeam Echosounders : International Hydrographic Review, v.5, no.3,
						    p.26-31.
						    (http://www.omg.unb.ca/omg/papers/beaudoin_IHR_nov2004.pdf).
						   note complexity if transducer arrays are reverse mounted, as determined
						   by a mount heading angle of about 180 degrees rather than about 0 degrees.
						   If a receive array or a transmit array are reverse mounted then:
						    1) subtract 180 from the heading mount angle of the array
						    2) flip the sign of the pitch and roll mount offsets of the array
						    3) flip the sign of the beam steering angle from that array
						        (reverse TX means flip sign of TX steer, reverse RX
						        means flip sign of RX steer) */
						tx_steer = RTD * v2detection->angle_y[j];
						tx_orientation.roll = roll;
						tx_orientation.pitch = pitch;
						tx_orientation.heading = heading;
						rx_steer = -RTD * v2detection->angle_x[j];
						rx_orientation.roll = beamroll;
						rx_orientation.pitch = beampitch;
						rx_orientation.heading = beamheading;
						reference_heading = heading;

						status = mb_beaudoin(verbose, tx_align, tx_orientation, tx_steer, rx_align, rx_orientation, rx_steer,
						                     reference_heading, &beamAzimuth, &beamDepression, error);
						theta = 90.0 - beamDepression;
						phi = 90.0 - beamAzimuth;
						if (phi < 0.0)
							phi += 360.0;

						/* calculate bathymetry */
						rr = 0.5 * soundspeed * bathymetry->range[i];
						xx = rr * sin(DTR * theta);
						zz = rr * cos(DTR * theta);
						bathymetry->acrosstrack[i] = xx * cos(DTR * phi);
						bathymetry->alongtrack[i] = xx * sin(DTR * phi);
						bathymetry->depth[i] = zz + sensordepth - heave;
						bathymetry->pointing_angle[i] = DTR * theta;
						bathymetry->azimuth_angle[i] = DTR * phi;
					}
				}

				/* case of v2detection record */
				else if (store->read_v2detection == MB_YES) {
					/* now loop over the detects */
					for (i = 0; i < v2detection->number_beams; i++) {
						bathymetry->range[i] = v2detection->range[i];
						/* bathymetry->quality[i] set in bathymetry record */

						/* compensate for pitch if not already compensated */
						if ((volatilesettings->transmit_flags & 0xF) != 0) {
							beampitch = 0.0;
						}
						else {
							beampitch = pitch;
						}
						beampitchr = DTR * beampitch;

						/* compensate for roll if not already compensated */
						if ((volatilesettings->receive_flags & 0x1) != 0) {
							beamroll = 0.0;
						}
						else {
							/* get roll at bottom return time for this beam */
							interp_status =
							    mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude,
							                     time_d + bathymetry->range[i], &beamroll, &jattitude, error);
						}
						beamrollr = DTR * beamroll;

						/* get heading at bottom return time for this beam */
						interp_status = mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
						                                         pars->n_heading, time_d + bathymetry->range[i], &beamheading,
						                                         &jheading, error);
						beamheadingr = DTR * beamheading;

						/* calculate beam angles for raytracing using Jon Beaudoin's code based on:
						    Beaudoin, J., Hughes Clarke, J., and Bartlett, J. Application of
						    Surface Sound Speed Measurements in Post-Processing for Multi-Sector
						    Multibeam Echosounders : International Hydrographic Review, v.5, no.3,
						    p.26-31.
						    (http://www.omg.unb.ca/omg/papers/beaudoin_IHR_nov2004.pdf).
						   note complexity if transducer arrays are reverse mounted, as determined
						   by a mount heading angle of about 180 degrees rather than about 0 degrees.
						   If a receive array or a transmit array are reverse mounted then:
						    1) subtract 180 from the heading mount angle of the array
						    2) flip the sign of the pitch and roll mount offsets of the array
						    3) flip the sign of the beam steering angle from that array
						        (reverse TX means flip sign of TX steer, reverse RX
						        means flip sign of RX steer) */
						tx_steer = RTD * v2detection->angle_y[i];
						tx_orientation.roll = roll;
						tx_orientation.pitch = pitch;
						tx_orientation.heading = heading;
						rx_steer = -RTD * v2detection->angle_x[i];
						rx_orientation.roll = beamroll;
						rx_orientation.pitch = beampitch;
						rx_orientation.heading = beamheading;
						reference_heading = heading;

						status = mb_beaudoin(verbose, tx_align, tx_orientation, tx_steer, rx_align, rx_orientation, rx_steer,
						                     reference_heading, &beamAzimuth, &beamDepression, error);
						theta = 90.0 - beamDepression;
						phi = 90.0 - beamAzimuth;
						if (phi < 0.0)
							phi += 360.0;

						/* calculate bathymetry */
						rr = 0.5 * soundspeed * bathymetry->range[i];
						xx = rr * sin(DTR * theta);
						zz = rr * cos(DTR * theta);
						bathymetry->acrosstrack[i] = xx * cos(DTR * phi);
						bathymetry->alongtrack[i] = xx * sin(DTR * phi);
						bathymetry->depth[i] = zz + sensordepth - heave;
						bathymetry->pointing_angle[i] = DTR * theta;
						bathymetry->azimuth_angle[i] = DTR * phi;
					}
				}

				/* else default case of beamgeometry record */
				else {
					/* loop over all beams */
					for (i = 0; i < bathymetry->number_beams; i++) {
						/* bathymetry->range[i] set */
						/* bathymetry->quality[i] set */
						if ((bathymetry->quality[i] & 15) > 0) {
							/* compensate for pitch if not already compensated */
							if ((volatilesettings->transmit_flags & 0xF) != 0) {
								beampitch = 0.0;
							}
							else {
								beampitch = pitch;
							}
							beampitchr = DTR * beampitch;

							/* compensate for roll if not already compensated */
							if ((volatilesettings->receive_flags & 0x1) != 0) {
								beamroll = 0.0;
							}
							else {
								/* get roll at bottom return time for this beam */
								interp_status = mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1,
								                                 pars->n_attitude, time_d + bathymetry->range[i], &beamroll,
								                                 &jattitude, error);
							}
							beamrollr = DTR * beamroll;

							/* compensate for heave if not already compensated */
							if ((volatilesettings->receive_flags & 0x2) != 0) {
								beamheave = 0.0;
							}
							else {
								interp_status = mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_heave - 1,
								                                 pars->n_attitude, time_d + bathymetry->range[i], &beamheave,
								                                 &jattitude, error);
							}

							/* get heading at bottom return time for this beam */
							interp_status = mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
							                                         pars->n_heading, time_d + bathymetry->range[i], &beamheading,
							                                         &jheading, error);
							beamheadingr = DTR * beamheading;

							/* calculate beam angles for raytracing using Jon Beaudoin's code based on:
							    Beaudoin, J., Hughes Clarke, J., and Bartlett, J. Application of
							    Surface Sound Speed Measurements in Post-Processing for Multi-Sector
							    Multibeam Echosounders : International Hydrographic Review, v.5, no.3,
							    p.26-31.
							    (http://www.omg.unb.ca/omg/papers/beaudoin_IHR_nov2004.pdf).
							   note complexity if transducer arrays are reverse mounted, as determined
							   by a mount heading angle of about 180 degrees rather than about 0 degrees.
							   If a receive array or a transmit array are reverse mounted then:
							    1) subtract 180 from the heading mount angle of the array
							    2) flip the sign of the pitch and roll mount offsets of the array
							    3) flip the sign of the beam steering angle from that array
							        (reverse TX means flip sign of TX steer, reverse RX
							        means flip sign of RX steer) */
							tx_steer = RTD * beamgeometry->angle_alongtrack[i];
							tx_orientation.roll = roll;
							tx_orientation.pitch = pitch;
							tx_orientation.heading = heading;
							rx_steer = -RTD * beamgeometry->angle_acrosstrack[i];
							rx_orientation.roll = beamroll;
							rx_orientation.pitch = beampitch;
							rx_orientation.heading = beamheading;
							reference_heading = heading;

							status = mb_beaudoin(verbose, tx_align, tx_orientation, tx_steer, rx_align, rx_orientation, rx_steer,
							                     reference_heading, &beamAzimuth, &beamDepression, error);
							theta = 90.0 - beamDepression;
							phi = 90.0 - beamAzimuth;
							if (phi < 0.0)
								phi += 360.0;

							/* calculate bathymetry */
							rr = 0.5 * soundspeed * bathymetry->range[i];
							xx = rr * sin(DTR * theta);
							zz = rr * cos(DTR * theta);
							bathymetry->acrosstrack[i] = xx * cos(DTR * phi);
							bathymetry->alongtrack[i] = xx * sin(DTR * phi);
							bathymetry->depth[i] = zz + sensordepth - heave;
							bathymetry->pointing_angle[i] = DTR * theta;
							bathymetry->azimuth_angle[i] = DTR * phi;
						}
					}
				}

				/* set flag */
				bathymetry->optionaldata = MB_YES;
				bathymetry->header.OptionalDataOffset =
				    MBSYS_RESON7K_RECORDHEADER_SIZE + R7KHDRSIZE_7kBathymetricData + bathymetry->number_beams * 9;

				if (pars->multibeam_sidescan_source == MB_PR_SSSOURCE_SNIPPET)
					ss_source = R7KRECID_7kV2SnippetData;
				else if (pars->multibeam_sidescan_source == MB_PR_SSSOURCE_CALIBRATEDSNIPPET)
					ss_source = R7KRECID_7kCalibratedSnippetData;
				else if (pars->multibeam_sidescan_source == MB_PR_SSSOURCE_WIDEBEAMBACKSCATTER)
					ss_source = R7KRECID_7kBackscatterImageData;

				/* regenerate sidescan */
				status = mbsys_reson7k_makess(verbose, mbio_ptr, store_ptr, ss_source, MB_NO, pixel_size, MB_NO, swath_width,
				                              MB_YES, error);
			}
			/*--------------------------------------------------------------*/
		}
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:         %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:        %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_extract_platform(int verbose, void *mbio_ptr, void *store_ptr, int *kind, void **platform_ptr, int *error) {
	char *function_name = "mbsys_reson7k_extract_platform";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mb_platform_struct *platform;
	struct mbsys_reson7k_struct *store;
	s7kr_installation *installation;
	int sensor_multibeam, sensor_position, sensor_attitude;
	int ntimelag = 0;
	int isensor;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:         %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:      %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       platform_ptr:   %p\n", (void *)platform_ptr);
		fprintf(stderr, "dbg2       *platform_ptr:  %p\n", (void *)*platform_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	store = (struct mbsys_reson7k_struct *)store_ptr;
	installation = (s7kr_installation *)&store->installation;

	/* if needed allocate a new platform structure */
	if (*platform_ptr == NULL) {
		status = mb_platform_init(verbose, (void **)platform_ptr, error);
	}

	/* extract sensor offsets from installation record */
	if (*platform_ptr != NULL) {
		/* get pointer to platform structure */
		platform = (struct mb_platform_struct *)(*platform_ptr);

		/* look for multibeam sensor, add it if necessary */
		sensor_multibeam = -1;
		for (isensor = 0; isensor < platform->num_sensors && sensor_multibeam < 0; isensor++) {
			if (platform->sensors[isensor].type == MB_SENSOR_TYPE_SONAR_MULTIBEAM &&
			    platform->sensors[isensor].num_offsets == 2) {
				sensor_multibeam = isensor;
			}
		}
		if (sensor_multibeam < 0) {
			/* set sensor 0 (multibeam) */
			status = mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_SONAR_MULTIBEAM, NULL, "Reson", NULL,
			                                MB_SENSOR_CAPABILITY1_NONE, MB_SENSOR_CAPABILITY2_TOPOGRAPHY_MULTIBEAM, 2, 0, error);
			if (status == MB_SUCCESS) {
				sensor_multibeam = platform->num_sensors - 1;
			}
		}
		if (sensor_multibeam >= 0 && platform->sensors[sensor_multibeam].num_offsets == 2) {
			if (status == MB_SUCCESS) {
				platform->source_bathymetry = sensor_multibeam;
				platform->source_backscatter = sensor_multibeam;
			}
			if (status == MB_SUCCESS)
				status = mb_platform_set_sensor_offset(
				    verbose, (void *)platform, 0, 0, MB_SENSOR_POSITION_OFFSET_STATIC, (double)installation->transmit_x,
				    (double)installation->transmit_y, (double)installation->transmit_z, MB_SENSOR_ATTITUDE_OFFSET_STATIC,
				    (double)installation->transmit_heading, (double)installation->transmit_roll,
				    (double)installation->transmit_pitch, error);
			if (status == MB_SUCCESS)
				status = mb_platform_set_sensor_offset(verbose, (void *)platform, 0, 1, MB_SENSOR_POSITION_OFFSET_STATIC,
				                                       (double)installation->receive_x, (double)installation->receive_y,
				                                       (double)installation->receive_z, MB_SENSOR_ATTITUDE_OFFSET_STATIC,
				                                       (double)installation->receive_heading, (double)installation->receive_roll,
				                                       (double)installation->receive_pitch, error);
		}

		/* look for position sensor, add it if necessary */
		sensor_position = -1;
		if (platform->source_position1 >= 0)
			sensor_position = platform->source_position1;
		for (isensor = 0; isensor < platform->num_sensors && sensor_position < 0; isensor++) {
			if (platform->sensors[isensor].type == MB_SENSOR_TYPE_POSITION && platform->sensors[isensor].num_offsets == 1) {
				sensor_position = isensor;
			}
		}
		if (sensor_position < 0) {
			/* set sensor 1 (position) */
			status = mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_POSITION, NULL, NULL, NULL, 0, 0, 1,
			                                ntimelag, error);
			if (status == MB_SUCCESS) {
				sensor_position = platform->num_sensors - 1;
			}
		}
		if (sensor_position >= 0 && platform->sensors[sensor_position].num_offsets == 1) {
			if (status == MB_SUCCESS) {
				platform->source_position1 = sensor_position;
				platform->source_depth1 = sensor_position;
				platform->source_position = sensor_position;
				platform->source_depth = sensor_position;
			}

			if (status == MB_SUCCESS)
				status = mb_platform_set_sensor_offset(verbose, (void *)platform, 1, 0, MB_SENSOR_POSITION_OFFSET_STATIC,
				                                       (double)installation->position_x, (double)installation->position_y,
				                                       (double)installation->position_z, MB_SENSOR_ATTITUDE_OFFSET_NONE,
				                                       (double)0.0, (double)0.0, (double)0.0, error);
			if (status == MB_SUCCESS && installation->position_time_delay != 0) {
				status =
				    mb_platform_set_sensor_timelatency(verbose, (void *)platform, 1, MB_SENSOR_TIME_LATENCY_STATIC,
				                                       (double)(0.001 * installation->position_time_delay), 0, NULL, NULL, error);
			}
		}

		/* look for attitude sensor, add it if necessary */
		sensor_attitude = -1;
		if (platform->source_rollpitch1 >= 0)
			sensor_attitude = platform->source_rollpitch1;
		for (isensor = 0; isensor < platform->num_sensors && sensor_attitude < 0; isensor++) {
			if ((platform->sensors[isensor].type == MB_SENSOR_TYPE_VRU || platform->sensors[isensor].type == MB_SENSOR_TYPE_IMU ||
			     platform->sensors[isensor].type == MB_SENSOR_TYPE_INS) &&
			    platform->sensors[isensor].num_offsets == 1) {
				sensor_attitude = isensor;
			}
		}
		if (sensor_attitude < 0) {
			/* set sensor 2 (attitude) */
			status =
			    mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_VRU, NULL, NULL, NULL, 0, 0, 1, ntimelag, error);
			if (status == MB_SUCCESS) {
				sensor_attitude = platform->num_sensors - 1;
			}
		}
		if (sensor_attitude >= 0 && platform->sensors[sensor_attitude].num_offsets == 1) {
			if (status == MB_SUCCESS) {
				platform->source_rollpitch1 = sensor_attitude;
				platform->source_heading1 = sensor_attitude;
				platform->source_rollpitch = sensor_attitude;
				platform->source_heading = sensor_attitude;
			}

			if (status == MB_SUCCESS)
				status = mb_platform_set_sensor_offset(verbose, (void *)platform, 2, 0, MB_SENSOR_POSITION_OFFSET_STATIC,
				                                       (double)installation->motion_x, (double)installation->motion_y,
				                                       (double)installation->motion_z, MB_SENSOR_ATTITUDE_OFFSET_STATIC,
				                                       (double)installation->motion_heading, (double)installation->motion_roll,
				                                       (double)installation->motion_pitch, error);
			if (status == MB_SUCCESS && installation->motion_time_delay != 0) {
				status =
				    mb_platform_set_sensor_timelatency(verbose, (void *)platform, 1, MB_SENSOR_TIME_LATENCY_STATIC,
				                                       (double)(0.001 * installation->motion_time_delay), 0, NULL, NULL, error);
			}
		}

		/* print platform */
		if (verbose >= 2) {
			status = mb_platform_print(verbose, (void *)platform, error);
		}
	}
	else {
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		fprintf(stderr, "\nUnable to initialize platform offset structure\n");
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:           %d\n", *kind);
		fprintf(stderr, "dbg2       platform_ptr:   %p\n", (void *)platform_ptr);
		fprintf(stderr, "dbg2       *platform_ptr:  %p\n", (void *)*platform_ptr);
		fprintf(stderr, "dbg2       error:          %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:         %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                          double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag,
                          double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                          double *ssacrosstrack, double *ssalongtrack, char *comment, int *error) {
	char *function_name = "mbsys_reson7k_extract";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_bluefin *bluefin;
	s7kr_processedsidescan *processedsidescan;
	s7kr_volatilesettings *volatilesettings;
	s7kr_beamgeometry *beamgeometry;
	s7kr_bathymetry *bathymetry;
	s7kr_backscatter *backscatter;
	s7kr_beam *beam;
	s7kr_position *position;
	s7kr_systemeventmessage *systemeventmessage;
	s7kr_fsdwsb *fsdwsb;
	s7kr_fsdwss *fsdwsslo;
	s7kr_fsdwss *fsdwsshi;
	s7k_fsdwsegyheader *fsdwsegyheader;
	s7k_fsdwssheader *fsdwssheader;
	double *pixel_size;
	double *swath_width;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *)store_ptr;
	bluefin = (s7kr_bluefin *)&store->bluefin;
	processedsidescan = (s7kr_processedsidescan *)&store->processedsidescan;
	volatilesettings = (s7kr_volatilesettings *)&(store->volatilesettings);
	beamgeometry = (s7kr_beamgeometry *)&(store->beamgeometry);
	bathymetry = (s7kr_bathymetry *)&store->bathymetry;
	backscatter = (s7kr_backscatter *)&store->backscatter;
	beam = (s7kr_beam *)&store->beam;
	position = (s7kr_position *)&store->position;
	systemeventmessage = (s7kr_systemeventmessage *)&store->systemeventmessage;
	fsdwsb = &(store->fsdwsb);
	fsdwsslo = &(store->fsdwsslo);
	fsdwsshi = &(store->fsdwsshi);

	/* get saved values */
	pixel_size = (double *)&mb_io_ptr->saved1;
	swath_width = (double *)&mb_io_ptr->saved2;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get interpolated nav heading and speed  */
		*speed = 0.0;
		if (mb_io_ptr->nfix > 0)
			mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

		/* get heading */
		if (bathymetry->optionaldata == MB_YES)
			*heading = RTD * bathymetry->heading;

		/* get navigation */
		if (bathymetry->optionaldata == MB_YES && bathymetry->longitude != 0.0 && bathymetry->latitude != 0.0) {
			*navlon = RTD * bathymetry->longitude;
			*navlat = RTD * bathymetry->latitude;
			/* fprintf(stderr,"mbsys_reson7k_extract: radians lon lat: %.10f %.10f  degrees lon lat: %.10f %.10f\n",
			bathymetry->longitude,bathymetry->latitude,*navlon,*navlat); */
		}

		/* set beamwidths in mb_io structure */
		if (store->read_volatilesettings == MB_YES) {
			mb_io_ptr->beamwidth_xtrack = RTD * volatilesettings->receive_width;
			mb_io_ptr->beamwidth_ltrack = RTD * volatilesettings->beamwidth_vertical;
		}
		else if (store->read_beamgeometry == MB_YES) {
			mb_io_ptr->beamwidth_xtrack = RTD * beamgeometry->beamwidth_acrosstrack[beamgeometry->number_beams / 2];
			mb_io_ptr->beamwidth_ltrack = RTD * beamgeometry->beamwidth_alongtrack[beamgeometry->number_beams / 2];
		}
		mb_io_ptr->beamwidth_xtrack = MIN(mb_io_ptr->beamwidth_xtrack, 2.0);
		mb_io_ptr->beamwidth_ltrack = MIN(mb_io_ptr->beamwidth_ltrack, 2.0);

		/* read distance and depth values into storage arrays */
		*nbath = bathymetry->number_beams;
		*namp = *nbath;
		for (i = 0; i < *nbath; i++) {
			bath[i] = bathymetry->depth[i];

			/* beamflagging scheme:
			    Reson quality flags use bits 0-3
			        bit 0: brightness test
			        bit 1: colinearity test
			        bit 2: amplitude pick
			        bit 3: phase pick
			    Early MB scheme (through 2007) - use bits 0-5
			        null: 0
			        flagged: 2
			        good: 15
			        amplitude: +16
			        phase: +32
			    Current MB scheme (>= 2008) - use bits 4-7
			        - bits 0-3 left in original values
			        - beam valid if bit 4 or 5 are set
			        - beam flagged if bit 6 or 7 set
			        bit 4: on = amplitude
			        bit 5: on = phase
			        bit 6: on = auto flag
			        bit 7: on = manual flag */
			if (bathymetry->quality[i] == 0) {
				beamflag[i] = MB_FLAG_NULL;
			}
			else if (bathymetry->quality[i] & 64) {
				beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
			}
			else if (bathymetry->quality[i] & 128) {
				beamflag[i] = MB_FLAG_FLAG + MB_FLAG_MANUAL;
			}
			else if (bathymetry->quality[i] & 240) {
				beamflag[i] = MB_FLAG_NONE;
			}
			else if ((bathymetry->quality[i] & 3) == 3) {
				beamflag[i] = MB_FLAG_NONE;
			}
			else if ((bathymetry->quality[i] & 15) == 0) {
				beamflag[i] = MB_FLAG_NULL;
			}
			else if ((bathymetry->quality[i] & 3) == 0) {
				beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
			}
			else {
				beamflag[i] = MB_FLAG_FLAG + MB_FLAG_MANUAL;
			}
#ifdef MSYS_RESON7KR_DEBUG
			fprintf(stderr, "EXTRACT: beam:%d quality:%d q&240:%d ", i, bathymetry->quality[i], bathymetry->quality[i] & 240);
			if (bathymetry->quality[i] & 1)
				fprintf(stderr, "1");
			else
				fprintf(stderr, "0");
			if (bathymetry->quality[i] & 2)
				fprintf(stderr, "1");
			else
				fprintf(stderr, "0");
			if (bathymetry->quality[i] & 4)
				fprintf(stderr, "1");
			else
				fprintf(stderr, "0");
			if (bathymetry->quality[i] & 8)
				fprintf(stderr, "1");
			else
				fprintf(stderr, "0");
			if (bathymetry->quality[i] & 16)
				fprintf(stderr, "1");
			else
				fprintf(stderr, "0");
			if (bathymetry->quality[i] & 32)
				fprintf(stderr, "1");
			else
				fprintf(stderr, "0");
			if (bathymetry->quality[i] & 64)
				fprintf(stderr, "1");
			else
				fprintf(stderr, "0");
			if (bathymetry->quality[i] & 128)
				fprintf(stderr, "1");
			else
				fprintf(stderr, "0");
			fprintf(stderr, " flag:%d\n", beamflag[i]);
#endif
			bathacrosstrack[i] = bathymetry->acrosstrack[i];
			bathalongtrack[i] = bathymetry->alongtrack[i];
			if (bathymetry->intensity[i] > 0.0)
				amp[i] = 20.0 * log10((double)bathymetry->intensity[i]);
			else
				amp[i] = 0.0;
		}

		/* extract sidescan */
		*nss = 0;
		if (store->read_processedsidescan == MB_YES) {
			*nss = processedsidescan->number_pixels;
			for (i = 0; i < processedsidescan->number_pixels; i++) {
				ss[i] = processedsidescan->sidescan[i];
				ssacrosstrack[i] = processedsidescan->pixelwidth * (i - (int)processedsidescan->number_pixels / 2);
				ssalongtrack[i] = processedsidescan->alongtrack[i];
			}
			for (i = processedsidescan->number_pixels; i < MBSYS_RESON7K_MAX_PIXELS; i++) {
				ss[i] = MB_SIDESCAN_NULL;
				ssacrosstrack[i] = 0.0;
				ssalongtrack[i] = 0.0;
			}
		}
		else {
			for (i = 0; i < MBSYS_RESON7K_MAX_PIXELS; i++) {
				ss[i] = MB_SIDESCAN_NULL;
				ssacrosstrack[i] = 0.0;
				ssalongtrack[i] = 0.0;
			}
		}

		/* print debug statements */
		if (verbose >= 5) {
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", function_name);
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
				fprintf(stderr, "dbg4       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
				        bathacrosstrack[i], bathalongtrack[i]);
			fprintf(stderr, "dbg4        namp:     %d\n", *namp);
			for (i = 0; i < *namp; i++)
				fprintf(stderr, "dbg4        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
				        bathalongtrack[i]);
			fprintf(stderr, "dbg4        nss:      %d\n", *nss);
			for (i = 0; i < *nss; i++)
				fprintf(stderr, "dbg4        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
				        ssalongtrack[i]);
		}

		/* done translating values */
	}

	/* extract data from structure */
	else if (*kind == MB_DATA_NAV1) {
		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get heading */
		if (mb_io_ptr->nheading > 0)
			mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);

		/* get speed */
		*speed = 0.0;
		if (mb_io_ptr->nfix > 0)
			mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

		/* get navigation */
		*navlon = RTD * position->longitude;
		*navlat = RTD * position->latitude;

		/* set beam and pixel numbers */
		*nbath = 0;
		*namp = 0;
		*nss = 0;

		/* print debug statements */
		if (verbose >= 5) {
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", function_name);
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
		}

		/* done translating values */
	}

	/* extract data from structure */
	else if (*kind == MB_DATA_NAV2) {
		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get heading */
		*heading = RTD * bluefin->nav[0].yaw;

		/* get speed */
		*speed = 0.0;
		if (mb_io_ptr->nfix > 0)
			mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

		/* get navigation */
		*navlon = RTD * bluefin->nav[0].longitude;
		*navlat = RTD * bluefin->nav[0].latitude;

		/* set beam and pixel numbers */
		*nbath = 0;
		*namp = 0;
		*nss = 0;

		/* print debug statements */
		if (verbose >= 5) {
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", function_name);
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
		}

		/* done translating values */
	}

	/* extract data from structure */
	else if (*kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) {
		/* get edgetech segy header */
		fsdwsegyheader = &(fsdwsb->segyheader);

		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get heading */
		if (fsdwsegyheader->heading != 0)
			*heading = 0.01 * fsdwsegyheader->heading;
		else if (mb_io_ptr->nheading > 0)
			mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);

		/* get speed and position */
		*speed = 0.0;
		if (mb_io_ptr->nfix > 0)
			mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

		/* get position */
		if (fsdwsegyheader->sourceCoordX != 0 || fsdwsegyheader->sourceCoordY != 0) {
			*navlon = ((double)fsdwsegyheader->sourceCoordX) / 360000.0;
			*navlat = ((double)fsdwsegyheader->sourceCoordY) / 360000.0;
		}

		/* set beam and pixel numbers */
		*nbath = 0;
		*namp = 0;
		*nss = 0;

		/* print debug statements */
		if (verbose >= 5) {
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", function_name);
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
		}

		/* done translating values */
	}

	/* extract data from sidescan structure */
	else if (*kind == MB_DATA_SIDESCAN2 || *kind == MB_DATA_SIDESCAN3) {
		/* get edgetech sidescan header */
		if (*kind == MB_DATA_SIDESCAN2)
			fsdwssheader = &(fsdwsslo->ssheader[0]);
		else // if (*kind == MB_DATA_SIDESCAN3)
			fsdwssheader = &(fsdwsshi->ssheader[0]);

		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get heading */
		if (fsdwssheader->heading != 0)
			*heading = 0.01 * fsdwssheader->heading;
		else if (mb_io_ptr->nheading > 0)
			mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);

		/* get speed and position */
		*speed = 0.0;
		mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

		/* get position */
		if (fsdwssheader->longitude != 0 || fsdwssheader->latitude != 0) {
			*navlon = ((double)fsdwssheader->longitude) / 360000.0;
			*navlat = ((double)fsdwssheader->latitude) / 360000.0;
		}

		/* set beam and pixel numbers */
		*nbath = 0;
		*namp = 0;
		*nss = 0;

		/* print debug statements */
		if (verbose >= 5) {
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", function_name);
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
		}

		/* done translating values */
	}

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT) {
		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* copy comment */
		if (systemeventmessage->message_length > 0)
			strncpy(comment, systemeventmessage->message, MB_COMMENT_MAXLINE);
		else
			comment[0] = '\0';

		/* print debug statements */
		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  Comment extracted by MBIO function <%s>\n", function_name);
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
	else {
		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* print debug statements */
		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", function_name);
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
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind == MB_DATA_COMMENT) {
		fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
	}
	else if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind != MB_DATA_COMMENT) {
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
	}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind == MB_DATA_DATA) {
		fprintf(stderr, "dbg2       nbath:      %d\n", *nbath);
		for (i = 0; i < *nbath; i++)
			fprintf(stderr, "dbg2       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
			        bathacrosstrack[i], bathalongtrack[i]);
		fprintf(stderr, "dbg2        namp:     %d\n", *namp);
		for (i = 0; i < *namp; i++)
			fprintf(stderr, "dbg2       beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
			        bathalongtrack[i]);
		fprintf(stderr, "dbg2        nss:      %d\n", *nss);
		for (i = 0; i < *nss; i++)
			fprintf(stderr, "dbg2        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
			        ssalongtrack[i]);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                         double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                         double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                         double *ssalongtrack, char *comment, int *error) {
	char *function_name = "mbsys_reson7k_insert";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_bluefin *bluefin;
	s7kr_processedsidescan *processedsidescan;
	s7kr_volatilesettings *volatilesettings;
	s7kr_bathymetry *bathymetry;
	s7kr_backscatter *backscatter;
	s7kr_beam *beam;
	s7kr_position *position;
	s7kr_systemeventmessage *systemeventmessage;
	s7kr_fsdwsb *fsdwsb;
	s7kr_fsdwss *fsdwsslo;
	s7kr_fsdwss *fsdwsshi;
	s7k_fsdwsegyheader *fsdwsegyheader;
	s7k_fsdwssheader *fsdwssheader;
	int msglen;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       kind:       %d\n", kind);
	}
	if (verbose >= 2 && (kind == MB_DATA_DATA || kind == MB_DATA_NAV1 || kind == MB_DATA_NAV2)) {
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
	}
	if (verbose >= 2 && kind == MB_DATA_DATA) {
		fprintf(stderr, "dbg2       nbath:      %d\n", nbath);
		if (verbose >= 3)
			for (i = 0; i < nbath; i++)
				fprintf(stderr, "dbg3       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
				        bathacrosstrack[i], bathalongtrack[i]);
		fprintf(stderr, "dbg2       namp:       %d\n", namp);
		if (verbose >= 3)
			for (i = 0; i < namp; i++)
				fprintf(stderr, "dbg3        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
				        bathalongtrack[i]);
		fprintf(stderr, "dbg2        nss:       %d\n", nss);
		if (verbose >= 3)
			for (i = 0; i < nss; i++)
				fprintf(stderr, "dbg3        beam:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
				        ssalongtrack[i]);
	}
	if (verbose >= 2 && kind == MB_DATA_COMMENT) {
		fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *)store_ptr;
	bluefin = (s7kr_bluefin *)&store->bluefin;
	volatilesettings = (s7kr_volatilesettings *)&store->volatilesettings;
	bathymetry = (s7kr_bathymetry *)&store->bathymetry;
	backscatter = (s7kr_backscatter *)&store->backscatter;
	beam = (s7kr_beam *)&store->beam;
	processedsidescan = (s7kr_processedsidescan *)&store->processedsidescan;
	position = (s7kr_position *)&store->position;
	systemeventmessage = (s7kr_systemeventmessage *)&store->systemeventmessage;
	fsdwsb = &(store->fsdwsb);
	fsdwsslo = &(store->fsdwsslo);
	fsdwsshi = &(store->fsdwsshi);

	/* set data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA) {
		/* get time */
		for (i = 0; i < 7; i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation */
		bathymetry->longitude = DTR * navlon;
		bathymetry->latitude = DTR * navlat;

		/* get heading */
		bathymetry->heading = DTR * heading;

		/* get speed  */

		/* read distance and depth values into storage arrays */
		bathymetry->number_beams = nbath;
		for (i = 0; i < bathymetry->number_beams; i++) {
			bathymetry->depth[i] = bath[i];
			if (beamflag[i] == MB_FLAG_NULL)
				bathymetry->quality[i] = 0;
			else if (mb_beam_check_flag_manual(beamflag[i]))
				bathymetry->quality[i] = (bathymetry->quality[i] & 63) + 128;
			else if (mb_beam_check_flag(beamflag[i]))
				bathymetry->quality[i] = (bathymetry->quality[i] & 63) + 64;
			else {
				bathymetry->quality[i] = (bathymetry->quality[i] & 63);
				if (!(bathymetry->quality[i] & 12))
					bathymetry->quality[i] = bathymetry->quality[i] | 16;
				else if (bathymetry->quality[i] & 4)
					bathymetry->quality[i] = bathymetry->quality[i] | 16;
				else if (bathymetry->quality[i] & 8)
					bathymetry->quality[i] = bathymetry->quality[i] | 32;
			}
			bathymetry->acrosstrack[i] = bathacrosstrack[i];
			bathymetry->alongtrack[i] = bathalongtrack[i];
			bathymetry->intensity[i] = (float)(pow(10.0, (amp[i] / 20.0)));
#ifdef MSYS_RESON7KR_DEBUG
			fprintf(stderr, "INSERT: beam:%d quality:%d q&240:%d ", i, bathymetry->quality[i], bathymetry->quality[i] & 240);
			if (bathymetry->quality[i] & 1)
				fprintf(stderr, "1");
			else
				fprintf(stderr, "0");
			if (bathymetry->quality[i] & 2)
				fprintf(stderr, "1");
			else
				fprintf(stderr, "0");
			if (bathymetry->quality[i] & 4)
				fprintf(stderr, "1");
			else
				fprintf(stderr, "0");
			if (bathymetry->quality[i] & 8)
				fprintf(stderr, "1");
			else
				fprintf(stderr, "0");
			if (bathymetry->quality[i] & 16)
				fprintf(stderr, "1");
			else
				fprintf(stderr, "0");
			if (bathymetry->quality[i] & 32)
				fprintf(stderr, "1");
			else
				fprintf(stderr, "0");
			if (bathymetry->quality[i] & 64)
				fprintf(stderr, "1");
			else
				fprintf(stderr, "0");
			if (bathymetry->quality[i] & 128)
				fprintf(stderr, "1");
			else
				fprintf(stderr, "0");
			fprintf(stderr, " flag:%d\n", beamflag[i]);
#endif
		}

		/* insert the sidescan */
		processedsidescan->number_pixels = nss;
		for (i = 0; i < processedsidescan->number_pixels; i++) {
			processedsidescan->sidescan[i] = ss[i];
			processedsidescan->alongtrack[i] = processedsidescan->alongtrack[i];
		}
		for (i = processedsidescan->number_pixels; i < MBSYS_RESON7K_MAX_PIXELS; i++) {
			processedsidescan->sidescan[i] = 0.0;
			processedsidescan->alongtrack[i] = 0.0;
		}
	}

	/* insert data in nav structure */
	else if (store->kind == MB_DATA_NAV1) {
		/* get time */
		for (i = 0; i < 7; i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation */
		position->longitude = DTR * navlon;
		position->latitude = DTR * navlat;

		/* get heading */

		/* get speed  */
	}

	/* insert data in nav structure */
	else if (store->kind == MB_DATA_NAV2) {
		/* get time */
		for (i = 0; i < 7; i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation */
		bluefin->nav[0].longitude = DTR * navlon;
		bluefin->nav[0].latitude = DTR * navlat;

		/* get heading */
		bluefin->nav[0].yaw = DTR * heading;

		/* get speed  */
	}

	/* insert data in subbottom structure */
	else if (store->kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) {
		/* get edgetech segy header */
		fsdwsegyheader = &(fsdwsb->segyheader);

		/* get time */
		for (i = 0; i < 7; i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation */
		fsdwsegyheader->sourceCoordX = (int)(navlon * 360000.0);
		fsdwsegyheader->sourceCoordY = (int)(navlat * 360000.0);

		/* get heading */
		fsdwsegyheader->heading = (int)(100 * heading);

		/* get speed  */
	}

	/* insert data in sidescan structure */
	else if (store->kind == MB_DATA_SIDESCAN2 || store->kind == MB_DATA_SIDESCAN3) {
		/* get edgetech sidescan header */
		if (store->kind == MB_DATA_SIDESCAN2)
			fsdwssheader = &(fsdwsslo->ssheader[0]);
		else // if (store->kind == MB_DATA_SIDESCAN3)
			fsdwssheader = &(fsdwsshi->ssheader[0]);

		/* get time */
		for (i = 0; i < 7; i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation */
		fsdwssheader->longitude = (int)(navlon * 360000.0);
		fsdwssheader->latitude = (int)(navlat * 360000.0);

		/* get heading */
		fsdwssheader->heading = (int)(100 * heading);

		/* get speed  */
	}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT) {
		/* make sure memory is allocated for comment */
		msglen = MIN(strlen(comment) + 1, MB_COMMENT_MAXLINE);
		if (msglen % 2 > 0)
			msglen++;
		if (systemeventmessage->message_alloc < msglen) {
			status = mb_reallocd(verbose, __FILE__, __LINE__, msglen, (void **)&(systemeventmessage->message), error);
			if (status != MB_SUCCESS) {
				systemeventmessage->message_alloc = 0;
				systemeventmessage->message = NULL;
			}
			else {
				systemeventmessage->message_alloc = msglen;
			}
		}

		/* copy comment */
		if (status == MB_SUCCESS) {
			/*fprintf(stderr,"INSERTING COMMENT: %s\n",comment);
			fprintf(stderr,"INSERTING COMMENT: msglen:%d message_alloc:%d status:%d error:%d\n",
			msglen,systemeventmessage->message_alloc,status,*error);*/
			store->type = R7KRECID_7kSystemEventMessage;
			systemeventmessage->serial_number = 0;
			systemeventmessage->event_id = 1;
			systemeventmessage->message_length = msglen;
			systemeventmessage->event_identifier = 0;
			strncpy(systemeventmessage->message, comment, msglen);
			systemeventmessage->header.Size =
			    MBSYS_RESON7K_RECORDHEADER_SIZE + R7KHDRSIZE_7kSystemEventMessage + msglen + MBSYS_RESON7K_RECORDTAIL_SIZE;
			systemeventmessage->header.OptionalDataOffset = 0;
			systemeventmessage->header.OptionalDataIdentifier = 0;
			systemeventmessage->header.Reserved = 0;
			systemeventmessage->header.RecordType = R7KRECID_7kSystemEventMessage;
			systemeventmessage->header.DeviceId = 0;
			systemeventmessage->header.SystemEnumerator = 0;
			systemeventmessage->header.DataSetNumber = 0;
			systemeventmessage->header.RecordNumber = 0;
			for (i = 0; i < 8; i++) {
				systemeventmessage->header.PreviousRecord[i] = -1;
				systemeventmessage->header.NextRecord[i] = -1;
			}
			systemeventmessage->header.Flags = 0;
			systemeventmessage->header.Reserved2 = 0;
		}
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
                         double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft,
                         double *ssv, int *error) {
	char *function_name = "mbsys_reson7k_ttimes";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_bathymetry *bathymetry;
	s7kr_depth *depth;
	s7kr_beamgeometry *beamgeometry;
	s7kr_attitude *attitude;
	s7kr_ctd *ctd;
	s7kr_reference *reference;
	double heave_use, roll, pitch;
	double alpha, beta, theta, phi;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       ttimes:     %p\n", (void *)ttimes);
		fprintf(stderr, "dbg2       angles_xtrk:%p\n", (void *)angles);
		fprintf(stderr, "dbg2       angles_ltrk:%p\n", (void *)angles_forward);
		fprintf(stderr, "dbg2       angles_null:%p\n", (void *)angles_null);
		fprintf(stderr, "dbg2       heave:      %p\n", (void *)heave);
		fprintf(stderr, "dbg2       ltrk_off:   %p\n", (void *)alongtrack_offset);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *)store_ptr;
	bathymetry = (s7kr_bathymetry *)&store->bathymetry;
	depth = (s7kr_depth *)&store->depth;
	attitude = (s7kr_attitude *)&store->attitude;
	ctd = (s7kr_ctd *)&store->ctd;
	beamgeometry = (s7kr_beamgeometry *)&store->beamgeometry;
	reference = (s7kr_reference *)&store->reference;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get depth offset (heave + sonar depth) */
		if (bathymetry->sound_velocity > 0.0)
			*ssv = bathymetry->sound_velocity;
		else if (ctd != NULL && ctd->n > 0)
			*ssv = ctd->sound_velocity[0];
		else
			*ssv = 1500.0;

		/* get attitude data */
		if (bathymetry->optionaldata == MB_YES) {
			heave_use = bathymetry->heave;
		}
		else if (mb_io_ptr->nattitude > 0) {
			mb_attint_interp(verbose, mbio_ptr, store->time_d, &heave_use, &roll, &pitch, error);
		}

		/* get draft */
		if (bathymetry->optionaldata == MB_YES) {
			*draft = -bathymetry->vehicle_height + reference->water_z;
			heave_use = 0.0;
		}
		else if (mb_io_ptr->nsonardepth > 0) {
			mb_depint_interp(verbose, mbio_ptr, store->time_d, draft, error);
			heave_use = 0.0;
		}
		else {
			*draft = reference->water_z;
		}

		/* get travel times, angles */
		*nbeams = bathymetry->number_beams;
		for (i = 0; i < bathymetry->number_beams; i++) {
			ttimes[i] = bathymetry->range[i];
			if (bathymetry->optionaldata == MB_YES) {
				angles[i] = RTD * bathymetry->pointing_angle[i];
				angles_forward[i] = RTD * bathymetry->azimuth_angle[i];
			}
			else {
				alpha = RTD * beamgeometry->angle_alongtrack[i] + bathymetry->pitch;
				beta = 90.0 - RTD * beamgeometry->angle_acrosstrack[i] + bathymetry->roll;
				mb_rollpitch_to_takeoff(verbose, alpha, beta, &theta, &phi, error);
				angles[i] = theta;
				angles_forward[i] = phi;
			}
			if (bathymetry->header.DeviceId == 7100)
				angles_null[i] = angles[i];
			else
				angles_null[i] = 0.0;
			heave[i] = heave_use;
			alongtrack_offset[i] = 0.0;
		}

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* done translating values */
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* deal with other record type */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
		fprintf(stderr, "dbg2       draft:      %f\n", *draft);
		fprintf(stderr, "dbg2       ssv:        %f\n", *ssv);
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (i = 0; i < *nbeams; i++)
			fprintf(stderr, "dbg2       beam %d: tt:%f  angle_xtrk:%f  angle_ltrk:%f  angle_null:%f  depth_off:%f  ltrk_off:%f\n",
			        i, ttimes[i], angles[i], angles_forward[i], angles_null[i], heave[i], alongtrack_offset[i]);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error) {
	char *function_name = "mbsys_reson7k_detects";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_bathymetry *bathymetry;
	mb_u_char detect;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       detects:    %p\n", (void *)detects);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *)store_ptr;
	bathymetry = (s7kr_bathymetry *)&store->bathymetry;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* read distance and depth values into storage arrays */
		*nbeams = bathymetry->number_beams;
		for (i = 0; i < *nbeams; i++) {
			detect = (bathymetry->quality[i] & 48) >> 4;
			if (detect == 0)
				detects[i] = MB_DETECT_UNKNOWN;
			else if (detect == 1)
				detects[i] = MB_DETECT_AMPLITUDE;
			else if (detect == 2)
				detects[i] = MB_DETECT_PHASE;
			else
				detects[i] = MB_DETECT_UNKNOWN;
		}

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* done translating values */
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* deal with other record type */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (i = 0; i < *nbeams; i++)
			fprintf(stderr, "dbg2       beam %d: detects:%d\n", i, detects[i]);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_gains(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transmit_gain, double *pulse_length,
                        double *receive_gain, int *error) {
	char *function_name = "mbsys_reson7k_gains";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_volatilesettings *volatilesettings;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get survey data structure */
		volatilesettings = &(store->volatilesettings);
		header = &(volatilesettings->header);

		/* get transmit_gain (dB) */
		*transmit_gain = (double)volatilesettings->power_selection;

		/* get pulse_length (usec) */
		*pulse_length = (double)volatilesettings->pulse_width;

		/* get receive_gain (dB) */
		*receive_gain = (double)volatilesettings->gain_selection;

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* done translating values */
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* deal with other record type */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
		fprintf(stderr, "dbg2       transmit_gain: %f\n", *transmit_gain);
		fprintf(stderr, "dbg2       pulse_length:  %f\n", *pulse_length);
		fprintf(stderr, "dbg2       receive_gain:  %f\n", *receive_gain);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                   double *altitudev, int *error) {
	char *function_name = "mbsys_reson7k_extract_altitude";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_bathymetry *bathymetry;
	s7kr_depth *depth;
	s7kr_altitude *altitude;
	s7kr_attitude *attitude;
	s7kr_reference *reference;
	double heave, roll, pitch;
	double xtrackmin;
	int altitude_found;
	char flag;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *)store_ptr;
	bathymetry = (s7kr_bathymetry *)&store->bathymetry;
	depth = (s7kr_depth *)&store->depth;
	attitude = (s7kr_attitude *)&store->attitude;
	altitude = (s7kr_altitude *)&store->altitude;
	reference = (s7kr_reference *)&store->reference;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get transducer depth and altitude */
		if (bathymetry->optionaldata == MB_YES) {
			*transducer_depth = -bathymetry->vehicle_height + reference->water_z;
		}
		else if (mb_io_ptr->nsonardepth > 0) {
			mb_depint_interp(verbose, mbio_ptr, store->time_d, transducer_depth, error);
		}
		else if (mb_io_ptr->nattitude > 0) {
			*transducer_depth = reference->water_z;
			mb_attint_interp(verbose, mbio_ptr, store->time_d, &heave, &roll, &pitch, error);
			*transducer_depth += heave;
		}
		else {
			*transducer_depth = reference->water_z;
		}

		/* get altitude */
		altitude_found = MB_NO;
		if (mb_io_ptr->naltitude > 0) {
			mb_altint_interp(verbose, mbio_ptr, store->time_d, altitudev, error);
			altitude_found = MB_YES;
		}
		if (altitude_found == MB_NO && bathymetry->optionaldata == MB_YES) {
			/* get depth closest to nadir */
			xtrackmin = 999999.9;
			for (i = 0; i < bathymetry->number_beams; i++) {
				if (bathymetry->quality[i] == 0) {
					flag = MB_FLAG_NULL;
				}
				else if (bathymetry->quality[i] & 64) {
					flag = MB_FLAG_FLAG + MB_FLAG_FILTER;
				}
				else if (bathymetry->quality[i] & 128) {
					flag = MB_FLAG_FLAG + MB_FLAG_MANUAL;
				}
				else if (bathymetry->quality[i] & 240) {
					flag = MB_FLAG_NONE;
				}
				else if ((bathymetry->quality[i] & 3) == 3) {
					flag = MB_FLAG_NONE;
				}
				else if ((bathymetry->quality[i] & 15) == 0) {
					flag = MB_FLAG_NULL;
				}
				else if ((bathymetry->quality[i] & 3) == 0) {
					flag = MB_FLAG_FLAG + MB_FLAG_FILTER;
				}
				else {
					flag = MB_FLAG_FLAG + MB_FLAG_MANUAL;
				}

				if ((flag == MB_FLAG_NONE) && fabs((double)bathymetry->acrosstrack[i]) < xtrackmin) {
					*altitudev = bathymetry->depth[i] - *transducer_depth;
					altitude_found = MB_YES;
					xtrackmin = fabs((double)bathymetry->acrosstrack[i]);
				}
			}
		}
		if (altitude_found == MB_NO && altitude->altitude > 0.0) {
			*altitudev = altitude->altitude;
		}
		else if (altitude_found == MB_NO) {
			*altitudev = 0.0;
		}

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* done translating values */
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* deal with other record type */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:              %d\n", *kind);
		fprintf(stderr, "dbg2       transducer_depth:  %f\n", *transducer_depth);
		fprintf(stderr, "dbg2       altitude:          %f\n", *altitudev);
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:            %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                              double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                              double *pitch, double *heave, int *error) {
	char *function_name = "mbsys_reson7k_extract_nav";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_bathymetry *bathymetry;
	s7kr_bluefin *bluefin;
	s7kr_position *position;
	s7kr_depth *depth;
	s7kr_attitude *attitude;
	s7kr_reference *reference;
	s7kr_navigation *navigation;
	s7kr_fsdwsb *fsdwsb;
	s7kr_fsdwss *fsdwsslo;
	s7kr_fsdwss *fsdwsshi;
	s7k_fsdwsegyheader *fsdwsegyheader;
	s7k_fsdwssheader *fsdwssheader;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *)store_ptr;
	bathymetry = (s7kr_bathymetry *)&store->bathymetry;
	bluefin = (s7kr_bluefin *)&store->bluefin;
	position = (s7kr_position *)&store->position;
	depth = (s7kr_depth *)&store->depth;
	attitude = (s7kr_attitude *)&store->attitude;
	reference = (s7kr_reference *)&store->reference;
	navigation = &(store->navigation);
	fsdwsb = &(store->fsdwsb);
	fsdwsslo = &(store->fsdwsslo);
	fsdwsshi = &(store->fsdwsshi);

	/* get data kind */
	*kind = store->kind;

	/* extract data from ping structure */
	if (*kind == MB_DATA_DATA) {
		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get interpolated nav heading and speed  */
		*speed = 0.0;
		if (mb_io_ptr->nheading > 0)
			mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);
		if (mb_io_ptr->nfix > 0)
			mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

		/* get heading */
		if (bathymetry->optionaldata == MB_YES)
			*heading = RTD * bathymetry->heading;

		/* get navigation */
		if (bathymetry->optionaldata == MB_YES) {
			*navlon = RTD * bathymetry->longitude;
			*navlat = RTD * bathymetry->latitude;
		}

		/* get draft  */
		if (bathymetry->optionaldata == MB_YES) {
			*draft = -bathymetry->vehicle_height + reference->water_z;
		}
		else if (mb_io_ptr->nsonardepth > 0) {
			mb_depint_interp(verbose, mbio_ptr, store->time_d, draft, error);
		}
		else {
			*draft = reference->water_z;
		}

		/* get attitude  */
		if (bathymetry->optionaldata == MB_YES) {
			*roll = RTD * bathymetry->roll;
			*pitch = RTD * bathymetry->pitch;
			*heave = bathymetry->heave;
		}
		else {
			if (mb_io_ptr->nattitude > 0) {
				mb_attint_interp(verbose, mbio_ptr, store->time_d, heave, roll, pitch, error);
			}
		}

		/* done translating values */
	}

	/* extract data from nav structure */
	else if (*kind == MB_DATA_NAV1) {
		/* get position data structure */
		position = (s7kr_position *)&store->position;

		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get navigation and heading */
		*speed = 0.0;
		if (mb_io_ptr->nheading > 0)
			mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);
		if (mb_io_ptr->nfix > 0)
			mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);
		*navlon = RTD * position->longitude;
		*navlat = RTD * position->latitude;

		/* get roll pitch and heave */
		if (mb_io_ptr->nattitude > 0) {
			mb_attint_interp(verbose, mbio_ptr, *time_d, heave, roll, pitch, error);
		}

		/* get draft  */
		if (mb_io_ptr->nsonardepth > 0) {
			if (mb_io_ptr->nsonardepth > 0)
				mb_depint_interp(verbose, mbio_ptr, store->time_d, draft, error);
			*heave = 0.0;
		}
		else if (bathymetry->optionaldata == MB_YES) {
			*draft = -bathymetry->vehicle_height + reference->water_z;
			*heave = 0.0;
		}
		else {
			*draft = reference->water_z;
		}

		/* done translating values */
	}

	/* extract data from structure */
	else if (*kind == MB_DATA_NAV2) {
		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get heading */
		*heading = RTD * bluefin->nav[0].yaw;

		/* get speed */
		*speed = 0.0;
		if (mb_io_ptr->nfix > 0)
			mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

		/* get navigation */
		*navlon = RTD * bluefin->nav[0].longitude;
		*navlat = RTD * bluefin->nav[0].latitude;

		/* get roll pitch and heave */
		*roll = RTD * bluefin->nav[0].roll;
		*pitch = RTD * bluefin->nav[0].pitch;
		*heave = 0.0;

		/* get draft  */
		*draft = bluefin->nav[0].depth;

		/* done translating values */
	}

	/* extract data from structure */
	else if (*kind == MB_DATA_NAV3) {
		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		time_d[0] = store->time_d;

		/* get navigation */
		navlon[0] = RTD * navigation->longitude;
		navlat[0] = RTD * navigation->latitude;

		/* get speed  */
		speed[0] = 0.0;

		/* get heading */
		if (mb_io_ptr->nheading > 0)
			mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);
		else if (bathymetry->optionaldata == MB_YES)
			heading[0] = RTD * bathymetry->heading;

		/* get draft  */
		if (mb_io_ptr->nsonardepth > 0) {
			mb_depint_interp(verbose, mbio_ptr, store->time_d, &(draft[0]), error);
		}
		else if (bathymetry->optionaldata == MB_YES) {
			draft[0] = -bathymetry->vehicle_height + reference->water_z;
		}
		else {
			draft[0] = reference->water_z;
		}

		/* get attitude  */
		if (mb_io_ptr->nattitude > 0) {
			mb_attint_interp(verbose, mbio_ptr, store->time_d, &(heave[0]), &(roll[0]), &(pitch[0]), error);
		}
		else if (bathymetry->optionaldata == MB_YES) {
			roll[0] = RTD * bathymetry->roll;
			pitch[0] = RTD * bathymetry->pitch;
			heave[0] = bathymetry->heave;
		}
		else {
			roll[0] = 0.0;
			pitch[0] = 0.0;
			heave[0] = 0.0;
		}

		/* done translating values */
	}

	/* extract data from structure */
	else if (*kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) {
		/* get edgetech segy header */
		fsdwsegyheader = &(fsdwsb->segyheader);

		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get heading */
		if (fsdwsegyheader->heading != 0)
			*heading = 0.01 * fsdwsegyheader->heading;
		else
			mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);

		/* get speed and position */
		*speed = 0.0;
		if (mb_io_ptr->nfix > 0)
			mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

		/* get position */
		if (fsdwsegyheader->sourceCoordX != 0 || fsdwsegyheader->sourceCoordY != 0) {
			*navlon = ((double)fsdwsegyheader->sourceCoordX) / 360000.0;
			*navlat = ((double)fsdwsegyheader->sourceCoordY) / 360000.0;
		}

		/* get roll pitch and heave */
		*roll = 0.01 * fsdwsegyheader->roll;
		*pitch = 0.01 * fsdwsegyheader->pitch;
		*heave = 0.0;

		if (mb_io_ptr->nattitude > 0) {
			mb_attint_interp(verbose, mbio_ptr, store->time_d, heave, roll, pitch, error);
		}

		/* get draft  */
		*draft = reference->water_z;

		/* done translating values */
	}

	/* extract data from sidescan structure */
	else if (*kind == MB_DATA_SIDESCAN2 || *kind == MB_DATA_SIDESCAN3) {
		/* get edgetech sidescan header */
		if (*kind == MB_DATA_SIDESCAN2)
			fsdwssheader = &(fsdwsslo->ssheader[0]);
		else // if (*kind == MB_DATA_SIDESCAN3)
			fsdwssheader = &(fsdwsshi->ssheader[0]);

		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get heading */
		if (fsdwssheader->heading != 0)
			*heading = 0.01 * fsdwssheader->heading;
		else if (mb_io_ptr->nheading > 0)
			mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);

		/* get speed and position */
		*speed = 0.0;
		if (mb_io_ptr->nfix > 0)
			mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

		/* get position */
		if (fsdwssheader->longitude != 0 || fsdwssheader->latitude != 0) {
			*navlon = ((double)fsdwssheader->longitude) / 360000.0;
			*navlat = ((double)fsdwssheader->latitude) / 360000.0;
		}

		/* print debug statements */
		if (verbose >= 5) {
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", function_name);
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
		}

		/* done translating values */
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;

		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;
	}

	/* deal with other record type */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;

		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
	/*if (status == MB_SUCCESS)
	fprintf(stderr,"%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %f %f %f %f %f %f %f %f\n",
	time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],
	*navlon,*navlat,*speed,*heading,*draft,*roll,*pitch,*heave);*/

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr, int nmax, int *kind, int *n, int *time_i,
                               double *time_d, double *navlon, double *navlat, double *speed, double *heading, double *draft,
                               double *roll, double *pitch, double *heave, int *error) {
	char *function_name = "mbsys_reson7k_extract_nnav";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_bathymetry *bathymetry;
	s7kr_bluefin *bluefin;
	s7kr_position *position;
	s7kr_depth *depth;
	s7kr_attitude *attitude;
	s7kr_reference *reference;
	s7kr_navigation *navigation;
	s7kr_rollpitchheave *rollpitchheave;
	int i, inav;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       nmax:       %d\n", nmax);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *)store_ptr;
	bathymetry = (s7kr_bathymetry *)&store->bathymetry;
	bluefin = (s7kr_bluefin *)&store->bluefin;
	position = (s7kr_position *)&store->position;
	depth = (s7kr_depth *)&store->depth;
	attitude = (s7kr_attitude *)&store->attitude;
	reference = (s7kr_reference *)&store->reference;
	navigation = &(store->navigation);
	rollpitchheave = &(store->rollpitchheave);

	/* get data kind */
	*kind = store->kind;

	/* extract data from ping structure */
	if (*kind == MB_DATA_DATA) {
		/* just one navigation value */
		*n = 1;

		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		time_d[0] = store->time_d;

		/* get interpolated nav heading and speed  */
		speed[0] = 0.0;
		mb_hedint_interp(verbose, mbio_ptr, store->time_d, &(heading[0]), error);
		mb_navint_interp(verbose, mbio_ptr, store->time_d, heading[0], speed[0], &(navlon[0]), &(navlat[0]), &(speed[0]), error);

		/* get heading */
		if (bathymetry->optionaldata == MB_YES)
			heading[0] = RTD * bathymetry->heading;

		/* get navigation */
		if (bathymetry->optionaldata == MB_YES) {
			navlon[0] = RTD * bathymetry->longitude;
			navlat[0] = RTD * bathymetry->latitude;
		}

		/* get draft  */
		if (bathymetry->optionaldata == MB_YES) {
			draft[0] = -bathymetry->vehicle_height + reference->water_z;
		}
		else if (mb_io_ptr->nsonardepth > 0) {
			mb_depint_interp(verbose, mbio_ptr, store->time_d, &(draft[0]), error);
		}
		else {
			draft[0] = reference->water_z;
		}

		/* get attitude  */
		if (bathymetry->optionaldata == MB_YES) {
			roll[0] = RTD * bathymetry->roll;
			pitch[0] = RTD * bathymetry->pitch;
			heave[0] = bathymetry->heave;
		}
		else {
			mb_attint_interp(verbose, mbio_ptr, store->time_d, &(heave[0]), &(roll[0]), &(pitch[0]), error);
		}

		/* done translating values */
	}

	/* extract data from nav structure */
	else if (*kind == MB_DATA_NAV1) {
		/* just one navigation value */
		*n = 1;

		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		time_d[0] = store->time_d;

		/* get navigation and heading */
		speed[0] = 0.0;
		mb_hedint_interp(verbose, mbio_ptr, store->time_d, &(heading[0]), error);
		mb_navint_interp(verbose, mbio_ptr, store->time_d, heading[0], speed[0], &(navlon[0]), &(navlat[0]), &(speed[0]), error);
		navlon[0] = RTD * position->longitude;
		navlat[0] = RTD * position->latitude;

		/* get roll pitch and heave */
		mb_attint_interp(verbose, mbio_ptr, *time_d, &(heave[0]), &(roll[0]), &(pitch[0]), error);

		/* get draft  */
		if (mb_io_ptr->nsonardepth > 0) {
			mb_depint_interp(verbose, mbio_ptr, store->time_d, &draft[0], error);
			heave[0] = 0.0;
		}
		else if (bathymetry->optionaldata == MB_YES) {
			draft[0] = -bathymetry->vehicle_height + reference->water_z;
			heave[0] = 0.0;
		}
		else {
			draft[0] = reference->water_z;
		}

		/* done translating values */
	}

	/* extract data from structure */
	else if (*kind == MB_DATA_NAV2) {
		/* get number of available navigation values */
		if (bluefin->data_format == 0 && bluefin->number_frames > 0)
			*n = bluefin->number_frames;
		else
			*n = 0;

		/* loop over navigation values */
		for (inav = 0; inav < *n; inav++) {
			/* get time */
			time_d[inav] = bluefin->nav[inav].position_time;
			mb_get_date(verbose, time_d[inav], &(time_i[7 * inav]));

			/* get heading */
			heading[inav] = RTD * bluefin->nav[inav].yaw;

			/* get speed */
			speed[inav] = 0.0;
			mb_navint_interp(verbose, mbio_ptr, time_d[inav], heading[inav], speed[inav], &(navlon[inav]), &(navlat[inav]),
			                 &(speed[inav]), error);

			/* get navigation */
			navlon[inav] = RTD * bluefin->nav[inav].longitude;
			navlat[inav] = RTD * bluefin->nav[inav].latitude;

			/* get roll pitch and heave */
			roll[inav] = RTD * bluefin->nav[inav].roll;
			pitch[inav] = RTD * bluefin->nav[inav].pitch;
			heave[inav] = 0.0;

			/* get draft  */
			draft[inav] = bluefin->nav[inav].depth;
		}

		/* done translating values */
	}

	/* extract data from structure */
	else if (*kind == MB_DATA_NAV3) {
		/* get number of available navigation values */
		*n = 1;

		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		time_d[0] = store->time_d;

		/* get navigation */
		navlon[0] = RTD * navigation->longitude;
		navlat[0] = RTD * navigation->latitude;

		/* get speed  */
		speed[0] = 0.0;

		/* get heading */
		if (mb_io_ptr->nheading > 0)
			mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);
		else if (bathymetry->optionaldata == MB_YES)
			heading[0] = RTD * bathymetry->heading;

		/* get draft  */
		if (mb_io_ptr->nsonardepth > 0) {
			mb_depint_interp(verbose, mbio_ptr, store->time_d, &(draft[0]), error);
		}
		else if (bathymetry->optionaldata == MB_YES) {
			draft[0] = -bathymetry->vehicle_height + reference->water_z;
		}
		else {
			draft[0] = reference->water_z;
		}

		/* get attitude  */
		if (mb_io_ptr->nattitude > 0) {
			mb_attint_interp(verbose, mbio_ptr, store->time_d, &(heave[0]), &(roll[0]), &(pitch[0]), error);
		}
		else if (bathymetry->optionaldata == MB_YES) {
			roll[0] = RTD * bathymetry->roll;
			pitch[0] = RTD * bathymetry->pitch;
			heave[0] = bathymetry->heave;
		}
		else {
			roll[0] = 0.0;
			pitch[0] = 0.0;
			heave[0] = 0.0;
		}

		/* done translating values */
	}

	/* extract data from attitude structure */
	else if (*kind == MB_DATA_ATTITUDE) {
		/* just one navigation value */
		*n = 1;

		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		time_d[0] = store->time_d;

		/* get interpolated nav heading and speed  */
		speed[0] = 0.0;
		heading[0] = 0.0;
		if (mb_io_ptr->nheading > 0)
			mb_hedint_interp(verbose, mbio_ptr, store->time_d, &(heading[0]), error);
		else if (bathymetry->optionaldata == MB_YES)
			heading[0] = RTD * bathymetry->heading;
		if (mb_io_ptr->nfix > 0)
			mb_navint_interp(verbose, mbio_ptr, store->time_d, heading[0], speed[0], &(navlon[0]), &(navlat[0]), &(speed[0]),
			                 error);
		else if (bathymetry->optionaldata == MB_YES) {
			navlon[0] = RTD * bathymetry->longitude;
			navlat[0] = RTD * bathymetry->latitude;
		}

		/* get draft  */
		if (mb_io_ptr->nsonardepth > 0) {
			mb_depint_interp(verbose, mbio_ptr, store->time_d, &(draft[0]), error);
		}
		else if (bathymetry->optionaldata == MB_YES) {
			draft[0] = -bathymetry->vehicle_height + reference->water_z;
		}
		else {
			draft[0] = reference->water_z;
		}

		/* get attitude  */
		roll[0] = RTD * rollpitchheave->roll;
		pitch[0] = RTD * rollpitchheave->pitch;
		heave[0] = rollpitchheave->heave;

		/* done translating values */
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;

		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;
	}

	/* deal with other record type */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;

		/* get number of available navigation values */
		*n = 1;

		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		fprintf(stderr, "dbg2       n:          %d\n", *n);
		for (inav = 0; inav < *n; inav++) {
			for (i = 0; i < 7; i++)
				fprintf(stderr, "dbg2       %d time_i[%d]:     %d\n", inav, i, time_i[inav * 7 + i]);
			fprintf(stderr, "dbg2       %d time_d:        %f\n", inav, time_d[inav]);
			fprintf(stderr, "dbg2       %d longitude:     %f\n", inav, navlon[inav]);
			fprintf(stderr, "dbg2       %d latitude:      %f\n", inav, navlat[inav]);
			fprintf(stderr, "dbg2       %d speed:         %f\n", inav, speed[inav]);
			fprintf(stderr, "dbg2       %d heading:       %f\n", inav, heading[inav]);
			fprintf(stderr, "dbg2       %d draft:         %f\n", inav, draft[inav]);
			fprintf(stderr, "dbg2       %d roll:          %f\n", inav, roll[inav]);
			fprintf(stderr, "dbg2       %d pitch:         %f\n", inav, pitch[inav]);
			fprintf(stderr, "dbg2       %d heave:         %f\n", inav, heave[inav]);
		}
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
                             double navlat, double speed, double heading, double draft, double roll, double pitch, double heave,
                             int *error) {
	char *function_name = "mbsys_reson7k_insert_nav";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_bathymetry *bathymetry;
	s7kr_position *position;
	s7kr_depth *depth;
	s7kr_attitude *attitude;
	s7kr_reference *reference;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
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
	store = (struct mbsys_reson7k_struct *)store_ptr;
	bathymetry = (s7kr_bathymetry *)&store->bathymetry;
	position = (s7kr_position *)&store->position;
	depth = (s7kr_depth *)&store->depth;
	attitude = (s7kr_attitude *)&store->attitude;
	reference = (s7kr_reference *)&store->reference;

	/* insert data in ping structure */
	if (store->kind == MB_DATA_DATA) {
		/* get time */
		for (i = 0; i < 7; i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation */
		bathymetry->longitude = DTR * navlon;
		bathymetry->latitude = DTR * navlat;

		/* get heading */
		bathymetry->heading = DTR * heading;

		/* get speed  */

		/* get draft  */
		bathymetry->vehicle_height = reference->water_z - draft;

		/* get roll pitch and heave */
		bathymetry->heave = heave;
		bathymetry->pitch = DTR * pitch;
		bathymetry->roll = DTR * roll;
	}

	/* insert data in nav structure */
	else if (store->kind == MB_DATA_NAV1) {
		/* get time */
		for (i = 0; i < 7; i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation */
		position->longitude = DTR * navlon;
		position->latitude = DTR * navlat;

		/* get heading */

		/* get speed  */

		/* get draft  */

		/* get roll pitch and heave */
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_extract_svp(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsvp, double *depth, double *velocity,
                              int *error) {
	char *function_name = "mbsys_reson7k_extract_svp";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_svp *svp;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *)store_ptr;
	svp = (s7kr_svp *)&store->svp;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_VELOCITY_PROFILE) {
		/* get number of depth-velocity pairs */
		*nsvp = svp->n;

		/* get profile */
		for (i = 0; i < *nsvp; i++) {
			depth[i] = svp->depth[i];
			velocity[i] = svp->sound_velocity[i];
		}

		/* done translating values */
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* deal with other record type */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:              %d\n", *kind);
		fprintf(stderr, "dbg2       nsvp:              %d\n", *nsvp);
		for (i = 0; i < *nsvp; i++)
			fprintf(stderr, "dbg2       depth[%d]: %f   velocity[%d]: %f\n", i, depth[i], i, velocity[i]);
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:            %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_insert_svp(int verbose, void *mbio_ptr, void *store_ptr, int nsvp, double *depth, double *velocity,
                             int *error) {
	char *function_name = "mbsys_reson7k_insert_svp";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_svp *svp;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       nsvp:       %d\n", nsvp);
		for (i = 0; i < nsvp; i++)
			fprintf(stderr, "dbg2       depth[%d]: %f   velocity[%d]: %f\n", i, depth[i], i, velocity[i]);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *)store_ptr;
	svp = (s7kr_svp *)&store->svp;

	/* insert data in structure */
	if (store->kind == MB_DATA_VELOCITY_PROFILE) {
		/* allocate memory if necessary */
		if (svp->nalloc < nsvp) {
			status = mb_reallocd(verbose, __FILE__, __LINE__, nsvp * sizeof(float), (void **)&(svp->depth), error);
			status = mb_reallocd(verbose, __FILE__, __LINE__, nsvp * sizeof(float), (void **)&(svp->sound_velocity), error);
			if (status == MB_SUCCESS) {
				svp->nalloc = nsvp;
			}
			else {
				svp->n = 0;
				svp->nalloc = 0;
			}
		}

		/* get profile */
		if (status == MB_SUCCESS) {
			svp->n = nsvp;
			for (i = 0; i < svp->n; i++) {
				svp->depth[i] = depth[i];
				svp->sound_velocity[i] = velocity[i];
			}
		}
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_ctd(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nctd, double *time_d, double *conductivity,
                      double *temperature, double *depth, double *salinity, double *soundspeed, int *error) {
	char *function_name = "mbsys_reson7k_ctd";
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_bluefin *bluefin;
	s7k_bluefin_environmental *environmental;
	s7kr_ctd *ctd;
	int status = MB_SUCCESS;
	int time_j[5];
	int time_i[7];
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract ctd data from bluefin environmental SSV record */
	if (*kind == MB_DATA_SSV) {
		bluefin = &(store->bluefin);
		header = &(bluefin->header);

		*nctd = 0;
		for (i = 0; i < bluefin->number_frames; i++) {
			environmental = &(bluefin->environmental[i]);
			if (environmental->ctd_time > 0.0 && *nctd < MB_CTD_MAX) {
				/* get time_d if needed */
				if (environmental->ctd_time < 10000.0) {
					time_j[0] = environmental->s7kTime.Year;
					time_j[1] = environmental->s7kTime.Day;
					time_j[2] = 60 * environmental->s7kTime.Hours + environmental->s7kTime.Minutes;
					time_j[3] = (int)environmental->s7kTime.Seconds;
					time_j[4] = (int)(1000000 * (environmental->s7kTime.Seconds - time_j[3]));
					mb_get_itime(verbose, time_j, time_i);
					mb_get_time(verbose, time_i, &environmental->ctd_time);
				}

				/* get values */
				time_d[*nctd] = environmental->ctd_time;
				conductivity[*nctd] = environmental->conductivity;
				temperature[*nctd] = environmental->temperature;
				depth[*nctd] = environmental->pressure;
				salinity[*nctd] = environmental->salinity;
				soundspeed[*nctd] = environmental->sound_speed;
				(*nctd)++;
			}
		}
	}

	/* extract ctd data from CTD record */
	else if (*kind == MB_DATA_CTD) {
		ctd = &(store->ctd);
		header = &(ctd->header);

		/* get time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int)header->s7kTime.Seconds;
		time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, time_i);
		mb_get_time(verbose, time_i, &time_d[0]);

		*nctd = MIN(ctd->n, MB_CTD_MAX);
		for (i = 0; i < *nctd; i++) {
			time_d[i] = time_d[0];
			if (ctd->sample_rate > 0.0)
				time_d[i] += i * (1.0 / ctd->sample_rate);
			if (ctd->conductivity_flag == 0)
				conductivity[i] = ctd->conductivity_salinity[i];
			else
				salinity[i] = ctd->conductivity_salinity[i];
			temperature[i] = ctd->temperature[i];
			depth[i] = ctd->pressure_depth[i];
			soundspeed[i] = ctd->sound_velocity[i];
		}
	}

	/* else failure */
	else {
		*nctd = 0;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
		fprintf(stderr, "dbg2       nctd:          %d\n", *nctd);
		for (i = 0; i < *nctd; i++) {
			fprintf(stderr, "dbg2       time_d:        %f\n", time_d[i]);
			fprintf(stderr, "dbg2       conductivity:  %f\n", conductivity[i]);
			fprintf(stderr, "dbg2       temperature:   %f\n", temperature[i]);
			fprintf(stderr, "dbg2       depth:         %f\n", depth[i]);
			fprintf(stderr, "dbg2       salinity:      %f\n", salinity[i]);
			fprintf(stderr, "dbg2       soundspeed:    %f\n", soundspeed[i]);
		}
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_ancilliarysensor(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsamples, double *time_d,
                                   double *sensor1, double *sensor2, double *sensor3, double *sensor4, double *sensor5,
                                   double *sensor6, double *sensor7, double *sensor8, int *error) {
	char *function_name = "mbsys_reson7k_ancilliarysensor";
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_bluefin *bluefin;
	s7k_bluefin_environmental *environmental;
	int status = MB_SUCCESS;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract ctd data from bluefin environmental SSV record */
	if (*kind == MB_DATA_SSV) {
		bluefin = &(store->bluefin);
		header = &(bluefin->header);

		*nsamples = 0;
		for (i = 0; i < bluefin->number_frames; i++) {
			environmental = &(bluefin->environmental[i]);
			time_d[*nsamples] = environmental->sensor_time_sec + 0.000000001 * environmental->sensor_time_nsec;
			sensor1[*nsamples] = -5.0 + ((double)environmental->sensor1) / 6553.6;
			sensor2[*nsamples] = -5.0 + ((double)environmental->sensor2) / 6553.6;
			sensor3[*nsamples] = -5.0 + ((double)environmental->sensor3) / 6553.6;
			sensor4[*nsamples] = -5.0 + ((double)environmental->sensor4) / 6553.6;
			sensor5[*nsamples] = -5.0 + ((double)environmental->sensor5) / 6553.6;
			sensor6[*nsamples] = -5.0 + ((double)environmental->sensor6) / 6553.6;
			sensor7[*nsamples] = -5.0 + ((double)environmental->sensor7) / 6553.6;
			sensor8[*nsamples] = -5.0 + ((double)environmental->sensor8) / 6553.6;
			(*nsamples)++;
		}
	}

	/* else failure */
	else {
		*nsamples = 0;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
		fprintf(stderr, "dbg2       nsamples:   %d\n", *nsamples);
		for (i = 0; i < *nsamples; i++) {
			fprintf(stderr, "dbg2       time_d:        %f\n", time_d[i]);
			fprintf(stderr, "dbg2       sensor1:       %f\n", sensor1[i]);
			fprintf(stderr, "dbg2       sensor2:       %f\n", sensor2[i]);
			fprintf(stderr, "dbg2       sensor3:       %f\n", sensor3[i]);
			fprintf(stderr, "dbg2       sensor4:       %f\n", sensor4[i]);
			fprintf(stderr, "dbg2       sensor5:       %f\n", sensor5[i]);
			fprintf(stderr, "dbg2       sensor6:       %f\n", sensor6[i]);
			fprintf(stderr, "dbg2       sensor7:       %f\n", sensor7[i]);
			fprintf(stderr, "dbg2       sensor8:       %f\n", sensor8[i]);
		}
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error) {
	char *function_name = "mbsys_reson7k_copy";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	struct mbsys_reson7k_struct *copy;
	s7kr_attitude *attitude;
	s7kr_motion *motion;
	s7kr_svp *svp;
	s7kr_ctd *ctd;
	s7kr_fsdwss *fsdwsslo;
	s7kr_fsdwss *fsdwsshi;
	s7kr_fsdwsb *fsdwsb;
	s7kr_configuration *configuration;
	s7kr_backscatter *backscatter;
	s7kr_beam *beam;
	s7kr_tvg *tvg;
	s7kr_image *image;
	s7kr_systemeventmessage *systemeventmessage;
	int nalloc;
	char *charptr, *copycharptr;
	int i, j;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       copy_ptr:   %p\n", (void *)copy_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointers */
	store = (struct mbsys_reson7k_struct *)store_ptr;
	copy = (struct mbsys_reson7k_struct *)copy_ptr;

	/* copy over structures, allocating memory where necessary */

	/* Type of data record */
	copy->kind = store->kind; /* MB-System record ID */
	copy->type = store->type; /* Reson record ID */

	/* MB-System time stamp */
	copy->time_d = store->time_d;
	for (i = 0; i < 7; i++)
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
	if (status == MB_SUCCESS && copy->attitude.nalloc < copy->attitude.n * sizeof(float)) {
		copy->attitude.nalloc = copy->attitude.n * sizeof(float);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, copy->attitude.nalloc, (void **)&(copy->attitude.pitch), error);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, copy->attitude.nalloc, (void **)&(copy->attitude.roll), error);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, copy->attitude.nalloc, (void **)&(copy->attitude.heading), error);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, copy->attitude.nalloc, (void **)&(copy->attitude.heave), error);
		if (status != MB_SUCCESS) {
			copy->attitude.n = 0;
			copy->attitude.nalloc = 0;
		}
	}
	if (status == MB_SUCCESS) {
		for (i = 0; i < copy->attitude.n; i++) {
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
	if (status == MB_SUCCESS && copy->motion.nalloc < copy->motion.n * sizeof(float)) {
		copy->motion.nalloc = copy->motion.n * sizeof(float);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, copy->motion.nalloc, (void **)&(copy->motion.x), error);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, copy->motion.nalloc, (void **)&(copy->motion.y), error);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, copy->motion.nalloc, (void **)&(copy->motion.z), error);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, copy->motion.nalloc, (void **)&(copy->motion.xa), error);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, copy->motion.nalloc, (void **)&(copy->motion.ya), error);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, copy->motion.nalloc, (void **)&(copy->motion.za), error);
		if (status != MB_SUCCESS) {
			copy->motion.n = 0;
			copy->motion.nalloc = 0;
		}
	}
	if (status == MB_SUCCESS) {
		for (i = 0; i < copy->motion.n; i++) {
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
	if (status == MB_SUCCESS && copy->svp.nalloc < copy->svp.n * sizeof(float)) {
		copy->svp.nalloc = copy->svp.n * sizeof(float);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, copy->svp.nalloc, (void **)&(copy->svp.depth), error);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, copy->svp.nalloc, (void **)&(copy->svp.sound_velocity), error);
		if (status != MB_SUCCESS) {
			copy->svp.n = 0;
			copy->svp.nalloc = 0;
		}
	}
	if (status == MB_SUCCESS) {
		for (i = 0; i < copy->svp.n; i++) {
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
	copy->ctd.absorption = ctd->absorption;
	if (status == MB_SUCCESS && copy->ctd.nalloc < copy->ctd.n * sizeof(float)) {
		copy->ctd.nalloc = copy->ctd.n * sizeof(float);
		if (status == MB_SUCCESS)
			status =
			    mb_reallocd(verbose, __FILE__, __LINE__, copy->ctd.nalloc, (void **)&(copy->ctd.conductivity_salinity), error);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, copy->ctd.nalloc, (void **)&(copy->ctd.temperature), error);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, copy->ctd.nalloc, (void **)&(copy->ctd.pressure_depth), error);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, copy->ctd.nalloc, (void **)&(copy->ctd.sound_velocity), error);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, copy->ctd.nalloc, (void **)&(copy->ctd.absorption), error);
		if (status != MB_SUCCESS) {
			copy->ctd.n = 0;
			copy->ctd.nalloc = 0;
		}
	}
	if (status == MB_SUCCESS) {
		for (i = 0; i < copy->ctd.n; i++) {
			copy->ctd.conductivity_salinity[i] = store->ctd.conductivity_salinity[i];
			copy->ctd.temperature[i] = store->ctd.temperature[i];
			copy->ctd.pressure_depth[i] = store->ctd.pressure_depth[i];
			copy->ctd.sound_velocity[i] = store->ctd.sound_velocity[i];
			copy->ctd.absorption[i] = store->ctd.absorption[i];
		}
	}

	/* Geodesy (record 1011) */
	copy->geodesy = store->geodesy;

	/* Edgetech FS-DW low frequency sidescan (record 3000) */
	fsdwsslo = &copy->fsdwsslo;
	copy->fsdwsslo = store->fsdwsslo;
	for (j = 0; j < 2; j++) {
		copy->fsdwsslo.channel[j].data_alloc = fsdwsslo->channel[j].data_alloc;
		copy->fsdwsslo.channel[j].data = fsdwsslo->channel[j].data;
		if (status == MB_SUCCESS && copy->fsdwsslo.channel[j].data_alloc <
		                                copy->fsdwsslo.channel[j].number_samples * copy->fsdwsslo.channel[j].bytespersample) {
			copy->fsdwsslo.channel[j].data_alloc =
			    copy->fsdwsslo.channel[j].number_samples * copy->fsdwsslo.channel[j].bytespersample;
			if (status == MB_SUCCESS)
				status = mb_reallocd(verbose, __FILE__, __LINE__, store->fsdwsslo.channel[j].data_alloc,
				                     (void **)&(copy->fsdwsslo.channel[j].data), error);
			if (status != MB_SUCCESS) {
				copy->fsdwsslo.channel[j].data_alloc = 0;
				copy->fsdwsslo.channel[j].number_samples = 0;
			}
		}
		if (status == MB_SUCCESS) {
			for (i = 0; i < copy->fsdwsslo.channel[j].data_alloc; i++) {
				copy->fsdwsslo.channel[j].data[i] = store->fsdwsslo.channel[j].data[i];
			}
		}
	}

	/* Edgetech FS-DW high frequency sidescan (record 3000) */
	fsdwsshi = &copy->fsdwsshi;
	copy->fsdwsshi = store->fsdwsshi;
	for (j = 0; j < 2; j++) {
		copy->fsdwsshi.channel[j].data_alloc = fsdwsshi->channel[j].data_alloc;
		copy->fsdwsshi.channel[j].data = fsdwsshi->channel[j].data;
		if (status == MB_SUCCESS && copy->fsdwsshi.channel[j].data_alloc <
		                                copy->fsdwsshi.channel[j].number_samples * copy->fsdwsshi.channel[j].bytespersample) {
			copy->fsdwsshi.channel[j].data_alloc =
			    copy->fsdwsshi.channel[j].number_samples * copy->fsdwsshi.channel[j].bytespersample;
			if (status == MB_SUCCESS)
				status = mb_reallocd(verbose, __FILE__, __LINE__, store->fsdwsshi.channel[j].data_alloc,
				                     (void **)&(copy->fsdwsshi.channel[j].data), error);
			if (status != MB_SUCCESS) {
				copy->fsdwsshi.channel[j].data_alloc = 0;
				copy->fsdwsshi.channel[j].number_samples = 0;
			}
		}
		if (status == MB_SUCCESS) {
			for (i = 0; i < copy->fsdwsshi.channel[j].data_alloc; i++) {
				copy->fsdwsshi.channel[j].data[i] = store->fsdwsshi.channel[j].data[i];
			}
		}
	}

	/* Edgetech FS-DW subbottom (record 3001) */
	fsdwsb = &copy->fsdwsb;
	copy->fsdwsb = store->fsdwsb;
	copy->fsdwsb.channel.data_alloc = fsdwsb->channel.data_alloc;
	copy->fsdwsb.channel.data = fsdwsb->channel.data;
	if (status == MB_SUCCESS &&
	    copy->fsdwsb.channel.data_alloc < copy->fsdwsb.channel.number_samples * copy->fsdwsb.channel.bytespersample) {
		copy->fsdwsb.channel.data_alloc = copy->fsdwsb.channel.number_samples * copy->fsdwsb.channel.bytespersample;
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, store->fsdwsb.channel.data_alloc,
			                     (void **)&(copy->fsdwsb.channel.data), error);
		if (status != MB_SUCCESS) {
			copy->fsdwsb.channel.data_alloc = 0;
			copy->fsdwsb.channel.number_samples = 0;
		}
	}
	if (status == MB_SUCCESS) {
		for (i = 0; i < copy->fsdwsb.channel.data_alloc; i++) {
			copy->fsdwsb.channel.data[i] = store->fsdwsb.channel.data[i];
		}
	}

	/* Bluefin Environmental Data Frame (can be included in record 3100) */
	copy->bluefin = store->bluefin;

	/* Reson 7k volatile sonar settings (record 7000) */
	copy->volatilesettings = store->volatilesettings;

	/* Reson 7k configuration (record 7001) */
	configuration = &copy->configuration;
	copy->configuration = store->configuration;
	for (j = 0; j < MBSYS_RESON7K_MAX_DEVICE; j++) {
		copy->configuration.device[j].info_alloc = configuration->device[j].info_alloc;
		copy->configuration.device[j].info = configuration->device[j].info;
		if (status == MB_SUCCESS && copy->configuration.device[j].info_alloc < copy->configuration.device[j].info_length) {
			copy->configuration.device[j].info_alloc = copy->configuration.device[j].info_length;
			if (status == MB_SUCCESS)
				status = mb_reallocd(verbose, __FILE__, __LINE__, copy->configuration.device[j].info_alloc,
				                     (void **)&(copy->configuration.device[j].info), error);
			if (status != MB_SUCCESS) {
				copy->configuration.device[j].info_alloc = 0;
				copy->configuration.device[j].info_length = 0;
			}
		}
		if (status == MB_SUCCESS) {
			for (i = 0; i < copy->configuration.device[j].info_length; i++) {
				copy->configuration.device[j].info[i] = store->configuration.device[j].info[i];
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
	if (status == MB_SUCCESS && copy->backscatter.nalloc < copy->backscatter.number_samples * copy->backscatter.sample_size) {
		copy->backscatter.nalloc = copy->backscatter.number_samples * copy->backscatter.sample_size;
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, copy->backscatter.nalloc, (void **)&(copy->backscatter.port_data),
			                     error);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, copy->backscatter.nalloc, (void **)&(copy->backscatter.stbd_data),
			                     error);
		if (status != MB_SUCCESS) {
			copy->backscatter.nalloc = 0;
			copy->backscatter.number_samples = 0;
		}
	}
	if (status == MB_SUCCESS) {
		for (i = 0; i < copy->backscatter.number_samples; i++) {
			copy->backscatter.port_data[i] = store->backscatter.port_data[i];
			copy->backscatter.stbd_data[i] = store->backscatter.stbd_data[i];
		}
	}

	/* Reson 7k beam data (record 7008) */
	beam = &copy->beam;
	copy->beam = store->beam;
	for (i = 0; i < MBSYS_RESON7K_MAX_RECEIVERS; i++) {
		copy->beam.snippets[i].nalloc_amp = beam->snippets[i].nalloc_amp;
		copy->beam.snippets[i].nalloc_phase = beam->snippets[i].nalloc_phase;
		copy->beam.snippets[i].amplitude = beam->snippets[i].amplitude;
		copy->beam.snippets[i].phase = beam->snippets[i].phase;
		if (status == MB_SUCCESS && (copy->beam.snippets[i].nalloc_amp < store->beam.snippets[i].nalloc_amp ||
		                             copy->beam.snippets[i].nalloc_phase < store->beam.snippets[i].nalloc_phase)) {
			copy->beam.snippets[i].nalloc_amp = store->beam.snippets[i].nalloc_amp;
			if (status == MB_SUCCESS)
				status = mb_reallocd(verbose, __FILE__, __LINE__, copy->beam.snippets[i].nalloc_amp,
				                     (void **)&(copy->beam.snippets[i].amplitude), error);
			copy->beam.snippets[i].nalloc_phase = store->beam.snippets[i].nalloc_phase;
			if (status == MB_SUCCESS)
				status = mb_reallocd(verbose, __FILE__, __LINE__, copy->beam.snippets[i].nalloc_phase,
				                     (void **)&(copy->beam.snippets[i].phase), error);
			if (status != MB_SUCCESS) {
				copy->beam.snippets[i].nalloc_amp = 0;
				copy->beam.snippets[i].nalloc_phase = 0;
				copy->beam.snippets[i].end_sample = 0;
				copy->beam.snippets[i].begin_sample = 0;
			}
		}
		if (status == MB_SUCCESS) {
			copycharptr = (char *)(copy->beam.snippets[i].amplitude);
			charptr = (char *)(store->beam.snippets[i].amplitude);
			for (j = 0; j < copy->beam.snippets[i].nalloc_amp; j++)
				copycharptr[j] = charptr[j];
			copycharptr = (char *)(copy->beam.snippets[i].phase);
			charptr = (char *)(store->beam.snippets[i].phase);
			for (j = 0; j < copy->beam.snippets[i].nalloc_phase; j++)
				copycharptr[j] = charptr[j];
		}
	}

	/* Reson 7k vertical depth (record 7009) */
	copy->verticaldepth = store->verticaldepth;

	/* Reson 7k tvg data (record 7010) */
	tvg = &copy->tvg;
	copy->tvg = store->tvg;
	copy->tvg.nalloc = tvg->nalloc;
	copy->tvg.tvg = tvg->tvg;
	nalloc = tvg->n * sizeof(float);
	if (status == MB_SUCCESS && copy->tvg.nalloc < nalloc) {
		copy->tvg.nalloc = nalloc;
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, copy->tvg.nalloc, (void **)&(copy->tvg.tvg), error);
		if (status != MB_SUCCESS) {
			copy->tvg.nalloc = 0;
			copy->tvg.n = 0;
		}
	}
	if (status == MB_SUCCESS) {
		copycharptr = (char *)(copy->tvg.tvg);
		charptr = (char *)(store->tvg.tvg);
		for (j = 0; j < nalloc; j++)
			copycharptr[j] = charptr[j];
	}

	/* Reson 7k image data (record 7011) */
	image = &copy->image;
	copy->image = store->image;
	copy->image.nalloc = image->nalloc;
	copy->image.image = image->image;
	nalloc = image->width * image->height * image->color_depth;
	if (status == MB_SUCCESS && copy->image.nalloc < nalloc) {
		copy->image.nalloc = nalloc;
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, copy->image.nalloc, (void **)&(copy->image.image), error);
		if (status != MB_SUCCESS) {
			copy->image.nalloc = 0;
			copy->image.width = 0;
			copy->image.height = 0;
			copy->image.color_depth = 0;
		}
	}
	if (status == MB_SUCCESS) {
		copycharptr = (char *)(copy->image.image);
		charptr = (char *)(store->image.image);
		for (j = 0; j < nalloc; j++)
			copycharptr[j] = charptr[j];
	}

	/* Reson 7k system event (record 7051) */
	systemeventmessage = &copy->systemeventmessage;
	copy->systemeventmessage = store->systemeventmessage;
	copy->systemeventmessage.message_alloc = systemeventmessage->message_alloc;
	copy->systemeventmessage.message = systemeventmessage->message;
	if (status == MB_SUCCESS && copy->systemeventmessage.message_alloc < copy->systemeventmessage.message_length) {
		copy->systemeventmessage.message_alloc = copy->systemeventmessage.message_length;
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, copy->systemeventmessage.message_alloc,
			                     (void **)&(copy->systemeventmessage.message), error);
		if (status != MB_SUCCESS) {
			copy->systemeventmessage.event_id = 0;
			copy->systemeventmessage.message_alloc = 0;
			copy->systemeventmessage.message_length = 0;
			copy->systemeventmessage.event_identifier = 0;
		}
	}
	if (status == MB_SUCCESS) {
		for (i = 0; i < copy->systemeventmessage.message_length; i++) {
			copy->systemeventmessage.message[i] = store->systemeventmessage.message[i];
		}
	}

	/* Reson 7k file header (record 7200) */
	copy->fileheader = store->fileheader;

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_makess(int verbose, void *mbio_ptr, void *store_ptr, int source, int pixel_size_set, double *pixel_size,
                         int swath_width_set, double *swath_width, int pixel_int, int *error) {
	char *function_name = "mbsys_reson7k_makess";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_reference *reference;
	s7kr_volatilesettings *volatilesettings;
	s7kr_beamgeometry *beamgeometry;
	s7kr_bathymetry *bathymetry;
	s7kr_backscatter *backscatter;
	s7kr_snippet *snippet;
	s7kr_beam *beam;
	s7kr_v2snippettimeseries *snippettimeseries;
	s7kr_v2snippet *v2snippet;
	s7kr_calibratedsnippettimeseries *calibratedsnippettimeseries;
	s7kr_calibratedsnippet *calibratedsnippet;
	s7kr_processedsidescan *processedsidescan;
	s7kr_bluefin *bluefin;
	s7kr_soundvelocity *soundvelocity;
	int nss;
	int ss_cnt[MBSYS_RESON7K_MAX_PIXELS];
	double ss[MBSYS_RESON7K_MAX_PIXELS];
	double ssacrosstrack[MBSYS_RESON7K_MAX_PIXELS];
	double ssalongtrack[MBSYS_RESON7K_MAX_PIXELS];
	int nbathsort;
	double bathsort[MBSYS_RESON7K_MAX_BEAMS];
	char beamflag[MBSYS_RESON7K_MAX_BEAMS];
	double pixel_size_calc;
	double ss_spacing, ss_spacing_use;
	double soundspeed;
	int iminxtrack;
	double minxtrack;
	double maxxtrack;
	int nrangetable;
	double rangetable[MBSYS_RESON7K_MAX_BEAMS];
	double acrosstracktable[MBSYS_RESON7K_MAX_BEAMS], acrosstracktablemin;
	double alongtracktable[MBSYS_RESON7K_MAX_BEAMS];
	int irangenadir, irange;
	int found;
	int pixel_int_use;
	int nsample, nsample_use, sample_start, sample_detect, sample_end;
	double angle, altitude, xtrack, xtrackss, ltrackss, factor;
	double range, beam_foot, beamwidth, sint;
	mb_u_char *data_uchar;
	unsigned short *data_ushort;
	unsigned int *data_uint;
	int first, last, k1, k2;
	int ibeam;
	int i, j, k, kk;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", svn_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:        %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:       %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       source:          %d\n", source);
		fprintf(stderr, "dbg2       pixel_size_set:  %d\n", pixel_size_set);
		fprintf(stderr, "dbg2       pixel_size:      %f\n", *pixel_size);
		fprintf(stderr, "dbg2       swath_width_set: %d\n", swath_width_set);
		fprintf(stderr, "dbg2       swath_width:     %f\n", *swath_width);
		fprintf(stderr, "dbg2       pixel_int:       %d\n", pixel_int);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_reson7k_struct *)store_ptr;
	reference = (s7kr_reference *)&store->reference;
	volatilesettings = (s7kr_volatilesettings *)&store->volatilesettings;
	beamgeometry = (s7kr_beamgeometry *)&store->beamgeometry;
	bathymetry = (s7kr_bathymetry *)&store->bathymetry;
	backscatter = (s7kr_backscatter *)&store->backscatter;
	v2snippet = (s7kr_v2snippet *)&store->v2snippet;
	calibratedsnippet = (s7kr_calibratedsnippet *)&store->calibratedsnippet;
	beam = (s7kr_beam *)&store->beam;
	processedsidescan = (s7kr_processedsidescan *)&store->processedsidescan;
	bluefin = (s7kr_bluefin *)&store->bluefin;
	soundvelocity = (s7kr_soundvelocity *)&store->soundvelocity;

	/* if necessary pick a source for the backscatter */
	if (store->kind == MB_DATA_DATA && source == R7KRECID_None) {
		if (store->read_calibratedsnippet == MB_YES)
			source = R7KRECID_7kCalibratedSnippetData;
		else if (store->read_v2snippet == MB_YES)
			source = R7KRECID_7kV2SnippetData;
		else if (store->read_beam == MB_YES)
			source = R7KRECID_7kBeamData;
		else if (store->read_backscatter == MB_YES)
			source = R7KRECID_7kBackscatterImageData;
	}

	/* calculate sidescan from the desired source data if it is available */
	if (store->kind == MB_DATA_DATA && ((source == R7KRECID_7kV2SnippetData && store->read_v2snippet == MB_YES) ||
	                                    (source == R7KRECID_7kCalibratedSnippetData && store->read_calibratedsnippet == MB_YES) ||
	                                    (source == R7KRECID_7kBeamData && store->read_beam == MB_YES) ||
	                                    (source == R7KRECID_7kBackscatterImageData && store->read_backscatter == MB_YES))) {
		/* get beamflags - only use snippets from good beams */
		for (i = 0; i < bathymetry->number_beams; i++) {
			/* beamflagging scheme:
			    Reson quality flags use bits 0-3
			        bit 0: brightness test
			        bit 1: colinearity test
			        bit 2: amplitude pick
			        bit 3: phase pick
			    Early MB scheme (through 2007) - use bits 0-5
			        null: 0
			        flagged: 2
			        good: 15
			        amplitude: +16
			        phase: +32
			    Current MB scheme (>= 2008) - use bits 4-7
			        - bits 0-3 left in original values
			        - beam valid if bit 4 or 5 are set
			        - beam flagged if bit 6 or 7 set
			        bit 4: on = amplitude
			        bit 5: on = phase
			        bit 6: on = auto flag
			        bit 7: on = manual flag */
			if (bathymetry->quality[i] == 0) {
				beamflag[i] = MB_FLAG_NULL;
			}
			else if (bathymetry->quality[i] & 64) {
				beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
			}
			else if (bathymetry->quality[i] & 128) {
				beamflag[i] = MB_FLAG_FLAG + MB_FLAG_MANUAL;
			}
			else if (bathymetry->quality[i] & 240) {
				beamflag[i] = MB_FLAG_NONE;
			}
			else if ((bathymetry->quality[i] & 3) == 3) {
				beamflag[i] = MB_FLAG_NONE;
			}
			else if ((bathymetry->quality[i] & 15) == 0) {
				beamflag[i] = MB_FLAG_NULL;
			}
			else if ((bathymetry->quality[i] & 3) == 0) {
				beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
			}
			else {
				beamflag[i] = MB_FLAG_FLAG + MB_FLAG_MANUAL;
			}
		}

		/* get beam angle size */
		beamwidth = 2.0 * RTD * volatilesettings->receive_width;

		/* get soundspeed */
		if (volatilesettings->sound_velocity > 0.0)
			soundspeed = volatilesettings->sound_velocity;
		else if (soundvelocity->soundvelocity > 0.0)
			soundspeed = soundvelocity->soundvelocity;
		else if (bluefin->environmental[0].sound_speed > 0.0)
			soundspeed = bluefin->environmental[0].sound_speed;
		else
			soundspeed = 1500.0;

		/* get raw pixel size */
		ss_spacing = 0.5 * soundspeed / volatilesettings->sample_rate;

		/* get median depth relative to the sonar and check for min max xtrack */
		nbathsort = 0;
		minxtrack = 0.0;
		maxxtrack = 0.0;
		iminxtrack = bathymetry->number_beams / 2;
		found = MB_NO;
		for (i = 0; i < bathymetry->number_beams; i++) {
			if (mb_beam_ok(beamflag[i])) {
				bathsort[nbathsort] = bathymetry->depth[i] + bathymetry->vehicle_height;
				nbathsort++;

				if (found == MB_NO || fabs(bathymetry->acrosstrack[i]) < minxtrack) {
					minxtrack = fabs(bathymetry->acrosstrack[i]);
					iminxtrack = i;
					found = MB_YES;
				}

				maxxtrack = MAX(fabs(bathymetry->acrosstrack[i]), maxxtrack);
			}
		}

		/* set number of pixels */
		nss = MIN(4 * bathymetry->number_beams, MBSYS_RESON7K_MAX_PIXELS);

		/* get sidescan pixel size */
		if (swath_width_set == MB_NO && bathymetry->number_beams > 0) {
			(*swath_width) = MAX(fabs(RTD * beamgeometry->angle_acrosstrack[0]),
			                     fabs(RTD * beamgeometry->angle_acrosstrack[bathymetry->number_beams - 1]));
		}
		if (pixel_size_set == MB_NO && nbathsort > 0) {
			/* calculate pixel size implied using swath width and nadir altitude */
			qsort((void *)bathsort, nbathsort, sizeof(double), (void *)mb_double_compare);
			pixel_size_calc = 2.1 * tan(DTR * (*swath_width)) * bathsort[nbathsort / 2] / nss;

			/* use pixel size based on actual swath width if that is larger than the first value */
			pixel_size_calc = MAX(pixel_size_calc, 2.1 * maxxtrack / nss);

			/* make sure the pixel size is at least equivalent to a 0.1 degree nadir beamwidth */
			pixel_size_calc = MAX(pixel_size_calc, bathsort[nbathsort / 2] * sin(DTR * 0.1));

			/* if the pixel size appears to be changing in size, moderate the change */
			if ((*pixel_size) <= 0.0)
				(*pixel_size) = pixel_size_calc;
			else if (0.95 * (*pixel_size) > pixel_size_calc)
				(*pixel_size) = 0.95 * (*pixel_size);
			else if (1.05 * (*pixel_size) < pixel_size_calc)
				(*pixel_size) = 1.05 * (*pixel_size);
			else
				(*pixel_size) = pixel_size_calc;
		}

		/* get pixel interpolation */
		pixel_int_use = pixel_int + 1;

		/* zero the sidescan */
		for (i = 0; i < MBSYS_RESON7K_MAX_PIXELS; i++) {
			ss[i] = 0.0;
			ssacrosstrack[i] = 0.0;
			ssalongtrack[i] = 0.0;
			ss_cnt[i] = 0;
		}
		for (i = 0; i < nss; i++) {
			ssacrosstrack[i] = (*pixel_size) * (double)(i - (nss / 2));
			;
		}

		/* loop over raw backscatter or sidescan from the desired source,
		 * 	putting each raw sample into the binning arrays */

		/* use calibrated snippet data
		   error_flag = 0 is calibrated snippet data
		   error_flag = 1 is uncalibrated snippet data
		   error_flag > 1 indicates a problem */
		if (source == R7KRECID_7kCalibratedSnippetData && calibratedsnippet->error_flag < 3) {
			for (i = 0; i < calibratedsnippet->number_beams; i++) {
				calibratedsnippettimeseries =
				    (s7kr_calibratedsnippettimeseries *)&(calibratedsnippet->calibratedsnippettimeseries[i]);
				ibeam = calibratedsnippettimeseries->beam_number;

				/* only use snippets from non-null and unflagged beams */
				/* note: errors have been observed in data produced by a Reson
				    simulator in which the detect_sample was
				    was outside the range of begin_sample to end_sample
				    - the current code effectively ignores this case because
				    sample_end < sample_start, so no samples are processed. */
				if (mb_beam_ok(beamflag[ibeam])) {
					nsample = calibratedsnippettimeseries->end_sample - calibratedsnippettimeseries->begin_sample + 1;
					altitude = bathymetry->depth[ibeam] + bathymetry->vehicle_height;
					xtrack = bathymetry->acrosstrack[ibeam];
					range = 0.5 * soundspeed * bathymetry->range[ibeam];
					angle = RTD * beamgeometry->angle_acrosstrack[ibeam];
					beam_foot = range * sin(DTR * beamwidth) / cos(DTR * angle);
					sint = fabs(sin(DTR * angle));
					nsample_use = beam_foot / ss_spacing;
					if (sint < nsample_use * ss_spacing / beam_foot)
						ss_spacing_use = beam_foot / nsample_use;
					else
						ss_spacing_use = ss_spacing / sint;
					/* fprintf(stderr, "spacing: %f %f n:%d sint:%f angle:%f range:%f foot:%f factor:%f\n",
					ss_spacing, ss_spacing_use,
					nsample_use, sint, angle, range, beam_foot,
					nsample_use * ss_spacing / beam_foot); */
					sample_start = MAX(((int)calibratedsnippettimeseries->detect_sample - (nsample_use / 2)),
					                   (int)calibratedsnippettimeseries->begin_sample);
					sample_end = MIN(((int)calibratedsnippettimeseries->detect_sample + (nsample_use / 2)),
					                 (int)calibratedsnippettimeseries->end_sample);
					/* fprintf(stderr,"beam:%d snippet samples: b:%d d:%d e:%d   start:%d end:%d\n",
					ibeam,calibratedsnippettimeseries->begin_sample,calibratedsnippettimeseries->detect_sample,calibratedsnippettimeseries->end_sample,sample_start,sample_end);
				  */
					for (k = sample_start; k <= sample_end; k++) {
						if (xtrack < 0.0)
							xtrackss = xtrack - ss_spacing_use * (k - (int)calibratedsnippettimeseries->detect_sample);
						else
							xtrackss = xtrack + ss_spacing_use * (k - (int)calibratedsnippettimeseries->detect_sample);
						kk = nss / 2 + (int)(xtrackss / (*pixel_size));
						kk = MIN(MAX(0, kk), nss - 1);
						ss[kk] +=
						    (double)calibratedsnippettimeseries->amplitude[k - (int)calibratedsnippettimeseries->begin_sample];
						ssalongtrack[kk] += bathymetry->alongtrack[i];
						ss_cnt[kk]++;
						/* fprintf(stderr,"k:%d detect:%d xtrack:%f xtrackss:%f kk:%d ss:%f ss_cnt:%d\n",
						k,snippettimeseries->detect_sample,xtrack,xtrackss,kk,ss[kk],ss_cnt[kk]); */
					}
				}
			}
		}

		/* use v2 snippet data */
		else if (source == R7KRECID_7kV2SnippetData && v2snippet->error_flag == MB_NO) {
			for (i = 0; i < v2snippet->number_beams; i++) {
				snippettimeseries = (s7kr_v2snippettimeseries *)&(v2snippet->snippettimeseries[i]);
				ibeam = snippettimeseries->beam_number;

				/* only use snippets from non-null and unflagged beams */
				/* note: errors have been observed in data produced by a Reson
				    simulator in which the detect_sample was
				    was outside the range of begin_sample to end_sample
				    - the current code effectively ignores this case because
				    sample_end < sample_start, so no samples are processed. */
				if (mb_beam_ok(beamflag[ibeam])) {
					nsample = snippettimeseries->end_sample - snippettimeseries->begin_sample + 1;
					altitude = bathymetry->depth[ibeam] + bathymetry->vehicle_height;
					xtrack = bathymetry->acrosstrack[ibeam];
					range = 0.5 * soundspeed * bathymetry->range[ibeam];
					angle = RTD * beamgeometry->angle_acrosstrack[ibeam];
					beam_foot = range * sin(DTR * beamwidth) / cos(DTR * angle);
					sint = fabs(sin(DTR * angle));
					nsample_use = beam_foot / ss_spacing;
					if (sint < nsample_use * ss_spacing / beam_foot)
						ss_spacing_use = beam_foot / nsample_use;
					else
						ss_spacing_use = ss_spacing / sint;
					/* fprintf(stderr, "spacing: %f %f n:%d sint:%f angle:%f range:%f foot:%f factor:%f\n",
					ss_spacing, ss_spacing_use,
					nsample_use, sint, angle, range, beam_foot,
					nsample_use * ss_spacing / beam_foot); */
					sample_start =
					    MAX(((int)snippettimeseries->detect_sample - (nsample_use / 2)), (int)snippettimeseries->begin_sample);
					sample_end =
					    MIN(((int)snippettimeseries->detect_sample + (nsample_use / 2)), (int)snippettimeseries->end_sample);
					/* fprintf(stderr,"beam:%d snippet samples: b:%d d:%d e:%d   start:%d end:%d\n",
					ibeam,snippettimeseries->begin_sample,snippettimeseries->detect_sample,snippettimeseries->end_sample,sample_start,sample_end);
				  */
					for (k = sample_start; k <= sample_end; k++) {
						if (xtrack < 0.0)
							xtrackss = xtrack - ss_spacing_use * (k - (int)snippettimeseries->detect_sample);
						else
							xtrackss = xtrack + ss_spacing_use * (k - (int)snippettimeseries->detect_sample);
						kk = nss / 2 + (int)(xtrackss / (*pixel_size));
						kk = MIN(MAX(0, kk), nss - 1);
						ss[kk] += (double)snippettimeseries->amplitude[k - (int)snippettimeseries->begin_sample];
						ssalongtrack[kk] += bathymetry->alongtrack[i];
						ss_cnt[kk]++;
						/* fprintf(stderr,"k:%d detect:%d xtrack:%f xtrackss:%f kk:%d ss:%f ss_cnt:%d\n",
						k,snippettimeseries->detect_sample,xtrack,xtrackss,kk,ss[kk],ss_cnt[kk]); */
					}
				}
			}
		}

		/* use old snippet data */
		else if (source == R7KRECID_7kBeamData) {
			for (i = 0; i < beam->number_beams; i++) {
				snippet = (s7kr_snippet *)&(beam->snippets[i]);
				ibeam = snippet->beam_number;

				/* only use snippets from non-null and unflagged beams */
				if (mb_beam_ok(beamflag[ibeam])) {
					nsample = snippet->end_sample - snippet->begin_sample + 1;
					altitude = bathymetry->depth[ibeam] + bathymetry->vehicle_height;
					xtrack = bathymetry->acrosstrack[ibeam];
					range = 0.5 * soundspeed * bathymetry->range[ibeam];
					angle = RTD * beamgeometry->angle_acrosstrack[ibeam];
					beam_foot = range * sin(DTR * beamwidth) / cos(DTR * angle);
					sint = fabs(sin(DTR * angle));
					nsample_use = beam_foot / ss_spacing;
					if (sint < nsample_use * ss_spacing / beam_foot)
						ss_spacing_use = beam_foot / nsample_use;
					else
						ss_spacing_use = ss_spacing / sint;
					/* fprintf(stderr, "spacing: %f %f xtrack:%f altitude:%f n:%d sint:%f angle:%f range:%f foot:%f factor:%f\n",
					ss_spacing, ss_spacing_use,
					xtrack,altitude,
					nsample_use, sint, angle, range, beam_foot,
					nsample_use * ss_spacing / beam_foot); */
					sample_detect = volatilesettings->sample_rate * bathymetry->range[ibeam];
					sample_start = MAX(sample_detect - (nsample_use / 2), snippet->begin_sample);
					sample_end = MIN(sample_detect + (nsample_use / 2), snippet->end_sample);
					if ((beam->sample_type & 15) == 3)
						data_uint = (unsigned int *)snippet->amplitude;
					else if ((beam->sample_type & 15) == 2)
						data_ushort = (unsigned short *)snippet->amplitude;
					else
						data_uchar = (mb_u_char *)snippet->amplitude;
					for (k = sample_start; k <= sample_end; k++) {
						if (xtrack < 0.0)
							xtrackss = xtrack - ss_spacing_use * (k - sample_detect);
						else
							xtrackss = xtrack + ss_spacing_use * (k - sample_detect);
						kk = nss / 2 + (int)(xtrackss / (*pixel_size));
						kk = MIN(MAX(0, kk), nss - 1);
						if ((beam->sample_type & 15) == 3)
							ss[kk] += (double)data_uint[k - snippet->begin_sample];
						else if ((beam->sample_type & 15) == 2)
							ss[kk] += (double)data_ushort[k - snippet->begin_sample];
						else
							ss[kk] += (double)data_uchar[k - snippet->begin_sample];
						ssalongtrack[kk] += bathymetry->alongtrack[ibeam];
						ss_cnt[kk]++;
						/* fprintf(stderr,"ibeam:%d k:%d kk:%d ss_cnt:%d ss:%f xtrackss:%f %f ssalongtrack:%f \n",
						ibeam,k,kk,ss_cnt[kk],ss[kk], xtrackss, (k-nss/2)*(*pixel_size), ssalongtrack[kk]); */
					}
				}
			}
		}

		/* use backscatter record - basically a pseudosidescan */
		else if (source == R7KRECID_7kBackscatterImageData) {
			/* get acrosstrack distance versus range table from bathymetry */
			nrangetable = 0;
			irangenadir = 0;
			for (i = 0; i < bathymetry->number_beams; i++) {
				if (mb_beam_ok(beamflag[i])) {
					rangetable[nrangetable] = bathymetry->range[i];
					acrosstracktable[nrangetable] = bathymetry->acrosstrack[i];
					alongtracktable[nrangetable] = bathymetry->alongtrack[i];
					if (nrangetable == 0 || fabs(acrosstracktable[nrangetable]) < acrosstracktablemin) {
						irangenadir = nrangetable;
						acrosstracktablemin = fabs(acrosstracktable[nrangetable]);
					}
					nrangetable++;
				}
			}

			/* lay out port side */
			data_uchar = (mb_u_char *)backscatter->port_data;
			data_ushort = (unsigned short *)backscatter->port_data;
			data_uint = (unsigned int *)backscatter->port_data;
			sample_start = rangetable[irangenadir] * volatilesettings->sample_rate;
			sample_end = MIN(rangetable[0] * volatilesettings->sample_rate, backscatter->number_samples - 1);
			irange = irangenadir;
			for (i = sample_start; i < sample_end; i++) {
				range = ((double)i) / ((double)volatilesettings->sample_rate);
				found = MB_NO;
				for (j = irange; j > 0 && found == MB_NO; j--) {
					if (range >= rangetable[j] && range < rangetable[j - 1]) {
						irange = j;
						found = MB_YES;
					}
				}
				factor = (range - rangetable[irange]) / (rangetable[irange - 1] - rangetable[irange]);
				xtrackss = acrosstracktable[irange] + factor * (acrosstracktable[irange - 1] - acrosstracktable[irange]);
				ltrackss = alongtracktable[irange] + factor * (alongtracktable[irange - 1] - alongtracktable[irange]);
				kk = nss / 2 + (int)(xtrackss / (*pixel_size));
				if (kk >= 0 && kk < nss) {
					if (backscatter->sample_size == 1)
						ss[kk] += (double)data_uchar[i];
					else if (backscatter->sample_size == 2)
						ss[kk] += (double)data_ushort[i];
					else
						ss[kk] += (double)data_uint[i];
					ssalongtrack[kk] += ltrackss;
					ss_cnt[kk]++;
				}
			}

			/* lay out starboard side */
			data_uchar = (mb_u_char *)backscatter->stbd_data;
			data_ushort = (unsigned short *)backscatter->stbd_data;
			data_uint = (unsigned int *)backscatter->stbd_data;
			sample_start = rangetable[irangenadir] * volatilesettings->sample_rate;
			sample_end = MIN(rangetable[nrangetable - 1] * volatilesettings->sample_rate, backscatter->number_samples - 1);
			irange = irangenadir;
			for (i = sample_start; i < sample_end; i++) {
				range = ((double)i) / ((double)volatilesettings->sample_rate);
				found = MB_NO;
				for (j = irange; j < nrangetable - 1 && found == MB_NO; j++) {
					if (range >= rangetable[j] && range < rangetable[j + 1]) {
						irange = j;
						found = MB_YES;
					}
				}
				factor = (range - rangetable[irange]) / (rangetable[irange + 1] - rangetable[irange]);
				xtrackss = acrosstracktable[irange] + factor * (acrosstracktable[irange + 1] - acrosstracktable[irange]);
				ltrackss = alongtracktable[irange] + factor * (alongtracktable[irange + 1] - alongtracktable[irange]);
				kk = nss / 2 + (int)(xtrackss / (*pixel_size));
				if (kk >= 0 && kk < nss) {
					if (backscatter->sample_size == 1)
						ss[kk] += (double)data_uchar[i];
					else if (backscatter->sample_size == 2)
						ss[kk] += (double)data_ushort[i];
					else
						ss[kk] += (double)data_uint[i];
					ssalongtrack[kk] += ltrackss;
					ss_cnt[kk]++;
				}
			}
		}

		/* average the sidescan */
		first = nss;
		last = -1;
		for (k = 0; k < nss; k++) {
			if (ss_cnt[k] > 0) {
				ss[k] /= ss_cnt[k];
				ssalongtrack[k] /= ss_cnt[k];
				first = MIN(first, k);
				last = k;
			}
			else
				ss[k] = MB_SIDESCAN_NULL;
		}

		/* interpolate the sidescan */
		k1 = first;
		k2 = first;
		for (k = first + 1; k < last; k++) {
			if (ss_cnt[k] <= 0) {
				if (k2 <= k) {
					k2 = k + 1;
					while (k2 < last && ss_cnt[k2] <= 0)
						k2++;
				}
				if (k2 - k1 <= pixel_int_use) {
					ss[k] = ss[k1] + (ss[k2] - ss[k1]) * ((double)(k - k1)) / ((double)(k2 - k1));
					ssacrosstrack[k] = (k - nss / 2) * (*pixel_size);
					ssalongtrack[k] =
					    ssalongtrack[k1] + (ssalongtrack[k2] - ssalongtrack[k1]) * ((double)(k - k1)) / ((double)(k2 - k1));
				}
			}
			else {
				k1 = k;
			}
		}

		/* embed the sidescan into the processed sidescan record */
		store->read_processedsidescan = MB_YES;
		processedsidescan->header = bathymetry->header;
		processedsidescan->header.Offset = R7KRECID_ProcessedSidescan;
		processedsidescan->header.Size =
		    MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE + R7KHDRSIZE_ProcessedSidescan + nss * 8;
		processedsidescan->header.OptionalDataOffset = 0;
		processedsidescan->header.OptionalDataIdentifier = 0;
		processedsidescan->header.RecordType = R7KRECID_ProcessedSidescan;
		processedsidescan->serial_number = bathymetry->serial_number;
		processedsidescan->ping_number = bathymetry->ping_number;
		processedsidescan->multi_ping = bathymetry->multi_ping;
		processedsidescan->recordversion = 1;
		processedsidescan->ss_source = source;
		processedsidescan->number_pixels = nss;
		processedsidescan->ss_type = MB_SIDESCAN_LINEAR;
		processedsidescan->pixelwidth = *pixel_size;
		processedsidescan->sonardepth = -bathymetry->vehicle_height + reference->water_z;
		processedsidescan->altitude = bathymetry->depth[iminxtrack] - processedsidescan->sonardepth;
		for (i = 0; i < MBSYS_RESON7K_MAX_PIXELS; i++) {
			processedsidescan->sidescan[i] = ss[i];
			processedsidescan->alongtrack[i] = ssalongtrack[i];
		}

		/* print debug statements */
		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  Sidescan regenerated in <%s>\n", function_name);
			fprintf(stderr, "dbg2       pixels_ss:  %d\n", nss);
			for (i = 0; i < nss; i++)
				fprintf(stderr, "dbg2       pixel:%4d  cnt:%3d  ss:%10f  xtrack:%10f  ltrack:%10f\n", i, ss_cnt[i], ss[i],
				        ssacrosstrack[i], ssalongtrack[i]);
		}
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       pixel_size:      %f\n", *pixel_size);
		fprintf(stderr, "dbg2       swath_width:     %f\n", *swath_width);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/

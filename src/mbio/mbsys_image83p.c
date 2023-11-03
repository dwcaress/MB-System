/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_image83p.c	5/5/2008
 *
 *    Copyright (c) 2008-2023 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbsys_image83p.c contains the functions for handling the data structure
 * used by MBIO functions to store data from the 480 Beam Imagenex DeltaT
 * multibeam sonar systems.
 * The data formats which are commonly used to store Imagenex DeltaT
 * data in files include
 *      MBF_IMAGE83P : MBIO ID 191
 *      MBF_IMAGEMBA : MBIO ID 192
 *
 * Author:	Vivek Reddy, Santa Clara University
 *       	D.W. Caress
 * Date:	May 5, 2008
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_image83p.h"

/*--------------------------------------------------------------------*/
int mbsys_image83p_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	(void)mbio_ptr;  // Unused arg

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	const int status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_image83p_struct), store_ptr, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_image83p_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	(void)mbio_ptr;  // Unused arg

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
	}

	/* deallocate memory for data structure */
	const int status = mb_freed(verbose, __FILE__, __LINE__, (void **)store_ptr, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_image83p_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss,
                              int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_image83p_struct *store = (struct mbsys_image83p_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get beam and pixel numbers */
		*nbath = mb_io_ptr->beams_bath_max;
		*namp = mb_io_ptr->beams_amp_max;
		*nss = 0;
	}
	else {
		/* get beam and pixel numbers */
		*nbath = 0;
		*namp = 0;
		*nss = 0;
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
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
int mbsys_image83p_pingnumber(int verbose, void *mbio_ptr, unsigned int *pingnumber, int *error) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
  }

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
	struct mbsys_image83p_struct *store = (struct mbsys_image83p_struct *)mb_io_ptr->store_data;

  /* extract data from structure */
  *pingnumber = store->ping_number;

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       pingnumber: %u\n", *pingnumber);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_image83p_sonartype(int verbose, void *mbio_ptr, void *store_ptr, int *sonartype, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get sonar type */
  *sonartype = MB_TOPOGRAPHY_TYPE_MULTIBEAM;

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       sonartype:  %d\n", *sonartype);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_image83p_preprocess(int verbose,     /* in: verbosity level set on command line 0..N */
                             void *mbio_ptr,  /* in: see mb_io.h:/^struct mb_io_struct/ */
                             void *store_ptr, /* in: see mbsys_image83p.h:/^struct mbsys_image83p_struct/ */
                             void *platform_ptr, void *preprocess_pars_ptr, int *error) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:                   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:                  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       platform_ptr:               %p\n", (void *)platform_ptr);
    fprintf(stderr, "dbg2       preprocess_pars_ptr:        %p\n", (void *)preprocess_pars_ptr);
  }

  *error = MB_ERROR_NO_ERROR;

  /* check for non-null data */
  assert(mbio_ptr != NULL);
  assert(preprocess_pars_ptr != NULL);

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get preprocessing parameters */
  struct mb_preprocess_struct *pars = (struct mb_preprocess_struct *)preprocess_pars_ptr;

  /* get data structure pointers */
  struct mbsys_image83p_struct *store = (struct mbsys_image83p_struct *)store_ptr;
  struct mb_platform_struct *platform = (struct mb_platform_struct *)platform_ptr;

  /* kluge parameters */
  double kluge_beampatternsnellfactor = 1.0;
  double kluge_soundspeedsnellfactor = 1.0;
  bool kluge_beampatternsnell = false;
  bool kluge_soundspeedsnell = false;
  bool kluge_zeroAttitudecorrection = false;
  bool kluge_zeroalongtrackangles = false;
  bool kluge_sensordepthfromheave = false;

  /* get kluges */
  for (int i = 0; i < pars->n_kluge; i++) {
    if (pars->kluge_id[i] == MB_PR_KLUGE_BEAMTWEAK) {
      kluge_beampatternsnell = true;
      kluge_beampatternsnellfactor = *((double *)&pars->kluge_pars[i * MB_PR_KLUGE_PAR_SIZE]);
    }
    else if (pars->kluge_id[i] == MB_PR_KLUGE_SOUNDSPEEDTWEAK) {
      kluge_soundspeedsnell = true;
      kluge_soundspeedsnellfactor = *((double *)&pars->kluge_pars[i * MB_PR_KLUGE_PAR_SIZE]);
    }
    else if (pars->kluge_id[i] == MB_PR_KLUGE_ZEROATTITUDECORRECTION) {
      kluge_zeroAttitudecorrection = true;
    }
    else if (pars->kluge_id[i] == MB_PR_KLUGE_ZEROALONGTRACKANGLES) {
      kluge_zeroalongtrackangles = true;
    }
    else if (pars->kluge_id[i] == MB_PR_KLUGE_SENSORDEPTHFROMHEAVE) {
      kluge_sensordepthfromheave = true;
    }
  }

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
    for (int i = 0; i < pars->n_kluge; i++) {
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
        fprintf(stderr, "dbg2       kluge_zeroAttitudecorrection:  %d\n", kluge_zeroAttitudecorrection);
      }
      else if (pars->kluge_id[i] == MB_PR_KLUGE_ZEROALONGTRACKANGLES) {
        fprintf(stderr, "dbg2       kluge_zeroalongtrackangles:    %d\n", kluge_zeroalongtrackangles);
      }
      else if (pars->kluge_id[i] == MB_PR_KLUGE_SENSORDEPTHFROMHEAVE) {
        fprintf(stderr, "dbg2       kluge_sensordepthfromheave:     %d\n", kluge_sensordepthfromheave);
      }
    }
  }

  int status = MB_SUCCESS;

  /* if called with store_ptr == NULL then called after mb_read_init() but before
      any data are read - for some formats this allows kluge options to set special
      reading conditions/behaviors */
  if (store_ptr == NULL) {

  }

  /* deal with a survey record */
  else if (store->kind == MB_DATA_DATA) {

    /*--------------------------------------------------------------*/
    /* change timestamp if indicated */
    /*--------------------------------------------------------------*/
    if (pars->timestamp_changed) {
      store->time_d = pars->time_d;
      mb_get_date(verbose, store->time_d, store->time_i);
      if (verbose > 1)
        fprintf(stderr, "Timestamp changed in function %s: "
              "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d | ping_number:%d\n",
              __func__, store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3],
              store->time_i[4], store->time_i[5], store->time_i[6], store->ping_number);
    }

    /*--------------------------------------------------------------*/
    /* interpolate ancillary values  */
    /*--------------------------------------------------------------*/

    double time_d = store->time_d;
    int time_i[7];
    for (int i=0;i<7;i++)
      time_i[i] = store->time_i[i];
    double navlon = store->nav_long;
    double navlat = store->nav_lat;
    double speed = 18.52 * store->nav_speed;
    double altitude = 0.0;
    double sensordepth = store->sonar_depth;
    double sensordepth_org = sensordepth;
    bool sensordepth_change = false;
    double heading = (double) store->heading_external;
    double roll = (double) store->roll_external;
    double pitch = (double) store->pitch_external;
    double heave = (double) store->heave_external;
    double heave_org = heave;
    bool heave_change = false;
    double mtodeglon, mtodeglat;
    int jnav = 0;
    int jsensordepth = 0;
    int jheading = 0;
    int jaltitude = 0;
    int jattitude = 0;
    int jsoundspeed = 0;
    int interp_error = MB_ERROR_NO_ERROR;

    /* zero attitude correction if requested */
    if (kluge_zeroAttitudecorrection) {
      roll = 0.0;
      pitch = 0.0;
    }

    /* case in which sensordepth has been encoded as the external heave - move the
        value from the heave_external parameter to the sonar_depth parameter and
        set the heave_external to zero. */
    if (kluge_sensordepthfromheave) {
      sensordepth = store->heave_external;
      store->sonar_depth = sensordepth;
      heave = 0.0;
      store->heave_external = heave;
    }

    if (pars->n_nav > 0) {

      mb_linear_interp_longitude(verbose, pars->nav_time_d - 1, pars->nav_lon - 1, pars->n_nav, time_d,
                                               &navlon, &jnav, &interp_error);
      mb_linear_interp_latitude(verbose, pars->nav_time_d - 1, pars->nav_lat - 1, pars->n_nav, time_d,
                                              &navlat, &jnav, &interp_error);
      if (pars->nav_speed != NULL) {
        mb_linear_interp(verbose, pars->nav_time_d - 1, pars->nav_speed - 1, pars->n_nav, time_d, &speed,
                                       &jnav, &interp_error);
      }

      /* if a valid speed is not available calculate it */
      if (speed <= 0.0 && jnav > 0) {
        int j1;
        int j2;
        if (jnav > 1) {
          j1 = jnav - 2;
          j2 = jnav - 1;
        }
        else /* if (jnav == 1) */ {
          j1 = jnav - 1;
          j2 = jnav;
        }
        mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
        double dx = (pars->nav_lon[j2] - pars->nav_lon[j1]) / mtodeglon;
        double dy = (pars->nav_lat[j2] - pars->nav_lat[j1]) / mtodeglat;
        double dt = (pars->nav_time_d[j2] - pars->nav_time_d[j1]);
        if (dt > 0.0)
          speed = sqrt(dx * dx + dy * dy) / dt;
      }
    }

    /* interpolate sensordepth */
    if (pars->n_sensordepth > 0) {
      mb_linear_interp(verbose, pars->sensordepth_time_d - 1, pars->sensordepth_sensordepth - 1,
                                     pars->n_sensordepth, time_d, &sensordepth, &jsensordepth, &interp_error);
    }

    /* interpolate heading */
    if (pars->n_heading > 0) {
      mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
                                             pars->n_heading, time_d, &heading, &jheading, &interp_error);
    }

    /* interpolate altitude */
    if (pars->n_altitude > 0) {
      mb_linear_interp(verbose, pars->altitude_time_d - 1, pars->altitude_altitude - 1, pars->n_altitude,
                                        time_d, &altitude, &jaltitude, &interp_error);
    }

    /* interpolate Attitude */
    if (pars->n_attitude > 0) {
      mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude,
                                       time_d, &roll, &jattitude, &interp_error);
      mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude,
                                       time_d, &pitch, &jattitude, &interp_error);
      mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_heave - 1, pars->n_attitude,
                                       time_d, &heave, &jattitude, &interp_error);
    }

    /* do lever arm correction */
    if (platform != NULL) {
      /* calculate sonar Position */
      status = mb_platform_position(verbose, (void *)platform, pars->target_sensor, 0, navlon, navlat, sensordepth,
                                    heading, roll, pitch, &navlon, &navlat, &sensordepth, error);

      /* calculate sonar Attitude */
      status = mb_platform_orientation_target(verbose, (void *)platform, pars->target_sensor, 0, heading, roll, pitch,
                                              &heading, &roll, &pitch, error);
    }

    store->nav_long = navlon;
    store->nav_lat = navlat;
    store->nav_speed = speed / 18.52;
    store->sonar_depth = sensordepth;
    if (sensordepth != sensordepth_org)
      sensordepth_change = true;
    if (heave != heave_org)
      heave_change = true;
    store->heading_external = (float) heading;
    store->roll_external = (float) roll;
    store->pitch_external = (float) pitch;

    /* get local translation between lon lat degrees and meters */
    mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);

    /* modify sound speed if needed */
    double soundspeed = 1500.0;
    if (store->sound_velocity > 13000 && store->sound_velocity < 17000)
      soundspeed = 0.1 * store->sound_velocity;
    double soundspeednew = soundspeed;
    double soundspeedsnellfactor = 1.0;

    /* interpolate soundspeed */
    if (pars->n_soundspeed > 0 && (pars->modify_soundspeed || kluge_soundspeedsnell)) {
      mb_linear_interp(verbose, pars->soundspeed_time_d - 1, pars->soundspeed_soundspeed - 1, pars->n_soundspeed,
                                     time_d, &soundspeednew, &jsoundspeed, &interp_error);
    }

    /* Change the sound speed used to calculate Bathymetry */
    if (pars->modify_soundspeed) {
      soundspeedsnellfactor = soundspeednew / soundspeed;
    }

    /* if requested apply kluge scaling of sound speed - which means
        changing beam angles by Snell's law and changing the sound
        speed used to calculate Bathymetry */
    if (kluge_beampatternsnell) {
      soundspeedsnellfactor *= kluge_beampatternsnellfactor;
    }
    if (kluge_soundspeedsnell) {
      soundspeedsnellfactor *= kluge_soundspeedsnellfactor;
    }

    /* change the sound speed recorded for the current ping and
       then use it to alter the beam angles and recalculated the Bathymetry */
    if (pars->modify_soundspeed || kluge_beampatternsnell || kluge_soundspeedsnell) {
      soundspeed *= soundspeedsnellfactor;
      store->sound_velocity = (int)(10.0 * soundspeed);
    }

    /*--------------------------------------------------------------*/
    /* recalculate Bathymetry  */
    /*--------------------------------------------------------------*/
    if (!pars->recalculate_bathymetry && (sensordepth_change || heave_change)) {
      store->num_proc_beams = store->num_beams;
      for (int i = 0; i < store->num_proc_beams; i++) {
        if (store->range[i] > 0) {
          store->bath[i] += (sensordepth - sensordepth_org) - (heave - heave_org);
        }
      }
    }
    else if (pars->recalculate_bathymetry) {

      if (verbose >= 2) {
        fprintf(stderr, "\ndbg2 Recalculating Bathymetry in %s:\n", __func__);
      }

      /* variables for beam angle calculation */
      mb_3D_orientation tx_align = {0.0, 0.0, 0.0};
      mb_3D_orientation tx_orientation = {0.0, 0.0, 0.0};
      double tx_steer = 0.0;
      mb_3D_orientation rx_align = {0.0, 0.0, 0.0};
      mb_3D_orientation rx_orientation = {0.0, 0.0, 0.0};
      double rx_steer = 0.0;

      /* get transducer angular offsets */
      int tx_sign = 1;
      int rx_sign = 1;
      if (platform != NULL) {
        status = mb_platform_orientation_offset(verbose, (void *)platform, pars->target_sensor, 0,
                                                &(tx_align.heading), &(tx_align.roll), &(tx_align.pitch), error);

        // handle reverse mounting of transmit array */
        if (tx_align.heading > 100.0 || tx_align.heading < -100.0) {
          tx_align.heading -= 180.0;
          if (tx_align.heading < 0.0)
            tx_align.heading += 360.0;
          tx_sign = -1;
        }

        status = mb_platform_orientation_offset(verbose, (void *)platform, pars->target_sensor, 1,
                                                &(rx_align.heading), &(rx_align.roll), &(rx_align.pitch), error);
        if (rx_align.heading > 100.0 || rx_align.heading < -100.0) {
          rx_align.heading -= 180.0;
          if (rx_align.heading < 0.0)
            rx_align.heading += 360.0;
          rx_sign = -1;
        }
      }

      store->num_proc_beams = store->num_beams;
      for (int i = 0; i < store->num_proc_beams; i++) {
        if (store->range[i] > 0) {

          /* get heading roll and pitch at bottom return time for this beam */
          double beamheading = heading;
          double beamroll = roll;
          double beampitch = pitch;
          double ttime = 0.001 * store->range_resolution * store->range[i] / 1500.0;
          if (pars->n_attitude > 0) {
            mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude,
                                             time_d + ttime, &beamroll, &jattitude, &interp_error);
            mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude,
                                             time_d + ttime, &beampitch, &jattitude, &interp_error);
            mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude,
                               time_d + ttime, &beamroll, &jattitude, error);
          }
          if (pars->n_heading > 0) {
            mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
                                                   pars->n_heading, time_d + ttime, &beamheading, &jheading, &interp_error);
          }

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
          tx_steer = tx_sign * 0.0;
          tx_orientation.roll = roll;
          tx_orientation.pitch = pitch + ((double)store->profile_tilt_angle - 180.0);
          tx_orientation.heading = heading;
          rx_steer = rx_sign * (180.0 - 0.01 * (store->start_angle + i * store->angle_increment));
          rx_orientation.roll = beamroll;
          rx_orientation.pitch = beampitch + ((double)store->profile_tilt_angle - 180.0);
          rx_orientation.heading = beamheading;
          double reference_heading = heading;
          double beamAzimuth;
          double beamDepression;
          status = mb_beaudoin(verbose, tx_align, tx_orientation, tx_steer, rx_align, rx_orientation, rx_steer,
                               reference_heading, &beamAzimuth, &beamDepression, error);
          const double theta = 90.0 - beamDepression;
          double phi = 90.0 - beamAzimuth;
          if (phi < 0.0)
            phi += 360.0;

          /* calculate Bathymetry */
          double rr = (soundspeed / 1500.0) * 0.001 * store->range_resolution * store->range[i];
          const double xx = rr * sin(DTR * theta);
          const double zz = rr * cos(DTR * theta);
          store->beamrange[i] = rr;
          store->angles[i] = theta;
          store->angles_forward[i] = phi;
          store->beamflag[i] = MB_FLAG_NONE;
          store->bath[i] = zz + store->sonar_depth - store->heave_external;
          store->bathacrosstrack[i] = xx * cos(DTR * phi);
          store->bathalongtrack[i] = xx * sin(DTR * phi);
          store->amp[i] = (float) store->intensity[i];
        }
        else {
          store->beamrange[i] = 0.0;
          store->angles[i] = 0.0;
          store->angles_forward[i] = 0.0;
          store->beamflag[i] = MB_FLAG_NULL;
          store->bath[i] = 0.0;
          store->bathacrosstrack[i] = 0.0;
          store->bathalongtrack[i] = 0.0;
          store->amp[i] = 0.0;
        }
      }
    }
  }

  /*--------------------------------------------------------------*/

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:         %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:        %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_image83p_extract_platform(int verbose, void *mbio_ptr, void *store_ptr,
      int *kind, void **platform_ptr, int *error) {
  int sensor_multibeam, sensor_position, sensor_Attitude;
  int ntimelag = 0;
  int isensor;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:         %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:      %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       platform_ptr:   %p\n", (void *)platform_ptr);
    fprintf(stderr, "dbg2       *platform_ptr:  %p\n", (void *)*platform_ptr);
  }

  struct mbsys_image83p_struct *store = (struct mbsys_image83p_struct *)store_ptr;

  int status = MB_SUCCESS;

  /* if needed allocate a new platform structure */
  if (*platform_ptr == NULL) {
    status = mb_platform_init(verbose, (void **)platform_ptr, error);
  }

  /* extract sensor offsets from InstallationParameters record */
  if (*platform_ptr != NULL) {
    /* get pointer to platform structure */
    struct mb_platform_struct *platform = (struct mb_platform_struct *)(*platform_ptr);

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
            verbose, (void *)platform, 0, 0, MB_SENSOR_POSITION_OFFSET_STATIC,
            (double)store->sonar_x_offset, (double)store->sonar_y_offset,
            (double)store->sonar_z_offset, MB_SENSOR_ATTITUDE_OFFSET_STATIC,
            (double)0.0, (double)0.0, (double)0.0, error);
      if (status == MB_SUCCESS)
        status = mb_platform_set_sensor_offset(
            verbose, (void *)platform, 0, 1, MB_SENSOR_POSITION_OFFSET_STATIC,
           (double)store->sonar_x_offset, (double)store->sonar_y_offset,
           (double)store->sonar_z_offset, MB_SENSOR_ATTITUDE_OFFSET_STATIC,
           (double)0.0, (double)0.0, (double)0.0, error);
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
                                               (double)0.0, (double)0.0, (double)0.0, MB_SENSOR_ATTITUDE_OFFSET_NONE,
                                               (double)0.0, (double)0.0, (double)0.0, error);
      if (status == MB_SUCCESS && store->ping_latency != 0) {
        status =
            mb_platform_set_sensor_timelatency(verbose, (void *)platform, 1, MB_SENSOR_TIME_LATENCY_STATIC,
                                               (double)(0.0001 * store->ping_latency), 0, NULL, NULL, error);
      }
    }

    /* look for Attitude sensor, add it if necessary */
    sensor_Attitude = -1;
    if (platform->source_rollpitch1 >= 0)
      sensor_Attitude = platform->source_rollpitch1;
    for (isensor = 0; isensor < platform->num_sensors && sensor_Attitude < 0; isensor++) {
      if ((platform->sensors[isensor].type == MB_SENSOR_TYPE_VRU || platform->sensors[isensor].type == MB_SENSOR_TYPE_IMU ||
           platform->sensors[isensor].type == MB_SENSOR_TYPE_INS) &&
          platform->sensors[isensor].num_offsets == 1) {
        sensor_Attitude = isensor;
      }
    }
    if (sensor_Attitude < 0) {
      /* set sensor 2 (Attitude) */
      status =
          mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_VRU, NULL, NULL, NULL, 0, 0, 1, ntimelag, error);
      if (status == MB_SUCCESS) {
        sensor_Attitude = platform->num_sensors - 1;
      }
    }
    if (sensor_Attitude >= 0 && platform->sensors[sensor_Attitude].num_offsets == 1) {
      if (status == MB_SUCCESS) {
        platform->source_rollpitch1 = sensor_Attitude;
        platform->source_heading1 = sensor_Attitude;
        platform->source_rollpitch = sensor_Attitude;
        platform->source_heading = sensor_Attitude;
      }

      if (status == MB_SUCCESS)
        status = mb_platform_set_sensor_offset(verbose, (void *)platform, 2, 0, MB_SENSOR_POSITION_OFFSET_STATIC,
                                               (double)0.0, (double)0.0, (double)0.0, MB_SENSOR_ATTITUDE_OFFSET_STATIC,
                                               (double)0.0, (double)0.0, (double)0.0, error);
      if (status == MB_SUCCESS && store->ping_latency != 0) {
        status =
            mb_platform_set_sensor_timelatency(verbose, (void *)platform, 1, MB_SENSOR_TIME_LATENCY_STATIC,
                                               (double)(0.001 * store->ping_latency), 0, NULL, NULL, error);
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

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
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
int mbsys_image83p_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                           double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag,
                           double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                           double *ssacrosstrack, double *ssalongtrack, char *comment, int *error) {
	(void)amp;  // Unused arg
	(void)ss;  // Unused arg
	(void)ssacrosstrack;  // Unused arg
	(void)ssalongtrack;  // Unused arg

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_image83p_struct *store = (struct mbsys_image83p_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get time */
		for (int i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get navigation */
		*navlon = store->nav_long;
		*navlat = store->nav_lat;

		/* get heading */
		*heading = (double) store->heading_external;

		/* get speed (convert knots to km/hr) */
		*speed = 1.852 * store->nav_speed * 0.1;

		/* set beamwidths in mb_io structure */
		mb_io_ptr->beamwidth_ltrack = 0.75;
		mb_io_ptr->beamwidth_xtrack = 0.75;

		/* read distance and depth values into storage arrays */
		*nbath = store->num_proc_beams;
		*namp = store->num_proc_beams;
		*nss = 0;
		for (int i = 0; i < *nbath; i++) {
			beamflag[i] = store->beamflag[i];
			bath[i] = store->bath[i];
			amp[i] = store->amp[i];
			bathacrosstrack[i] = store->bathacrosstrack[i];
			bathalongtrack[i] = store->bathalongtrack[i];
		}

		if (verbose >= 5) {
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", __func__);
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
			for (int i = 0; i < *nbath; i++)
				fprintf(stderr, "dbg4       beam:%d  flag:%3d  bath:%f  amp:%f  acrosstrack:%f  alongtrack:%f\n",
                i, beamflag[i], bath[i], amp[i], bathacrosstrack[i], bathalongtrack[i]);
		}

		/* done translating values */
	}

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT) {
		/* copy comment */
    memset((void *)comment, 0, MB_COMMENT_MAXLINE);
		strncpy(comment, store->comment, MIN(MB_COMMENT_MAXLINE, MBSYS_IMAGE83P_COMMENTLEN) - 1);

		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  New ping read by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  New ping values:\n");
			fprintf(stderr, "dbg4       error:      %d\n", *error);
			fprintf(stderr, "dbg4       comment:    %s\n", comment);
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
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
		for (int i = 0; i < *nbath; i++)
			fprintf(stderr, "dbg2       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
			        bathacrosstrack[i], bathalongtrack[i]);
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_image83p_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                          double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                          double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                          double *ssalongtrack, char *comment, int *error) {
	(void)mbio_ptr;  // Unused arg

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       kind:       %d\n", kind);
	}
	if (verbose >= 2 && (kind == MB_DATA_DATA || kind == MB_DATA_NAV)) {
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
			for (int i = 0; i < nbath; i++)
				fprintf(stderr, "dbg3       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
				        bathacrosstrack[i], bathalongtrack[i]);
		fprintf(stderr, "dbg2       namp:       %d\n", namp);
		if (verbose >= 3)
			for (int i = 0; i < namp; i++)
				fprintf(stderr, "dbg3        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
				        bathalongtrack[i]);
		fprintf(stderr, "dbg2        nss:       %d\n", nss);
		if (verbose >= 3)
			for (int i = 0; i < nss; i++)
				fprintf(stderr, "dbg3        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
				        ssalongtrack[i]);
	}
	if (verbose >= 2 && kind == MB_DATA_COMMENT) {
		fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_image83p_struct *store = (struct mbsys_image83p_struct *)store_ptr;

	/* set data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA) {
		/* get time */
		for (int i = 0; i < 7; i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation */

		store->nav_long = navlon;
		store->nav_lat = navlat;

		/* get heading */
		store->heading_external = (float) heading;

		/* get speed (convert km/hr to knots) */
		store->nav_speed = (int)(0.539996 * speed * 10);

		/* put depth values
		    into data structure */
		store->num_proc_beams = nbath;
		for (int i = 0; i < nbath; i++) {
			store->beamflag[i] = beamflag[i];
			store->bath[i] = bath[i];
			store->bathacrosstrack[i] = bathacrosstrack[i];
			store->bathalongtrack[i] = bathalongtrack[i];
		}
	}

	/* insert data in structure */
	else if (store->kind == MB_DATA_COMMENT) {
    memset((void *)store->comment, 0, MBSYS_IMAGE83P_COMMENTLEN);
		strncpy(store->comment, comment, MIN(MBSYS_IMAGE83P_COMMENTLEN, MB_COMMENT_MAXLINE) - 1);
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_image83p_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
                          double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft,
                          double *ssv, int *error) {
	(void)mbio_ptr;  // Unused arg

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       ttimes:     %p\n", (void *)ttimes);
		fprintf(stderr, "dbg2       angles_xtrk:%p\n", (void *)angles);
		fprintf(stderr, "dbg2       angles_ltrk:%p\n", (void *)angles_forward);
		fprintf(stderr, "dbg2       angles_null:%p\n", (void *)angles_null);
		fprintf(stderr, "dbg2       heave:      %p\n", (void *)heave);
		fprintf(stderr, "dbg2       ltrk_off:   %p\n", (void *)alongtrack_offset);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_image83p_struct *store = (struct mbsys_image83p_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get nbeams */
		*nbeams = store->num_proc_beams;

		*draft = store->sonar_depth - store->heave_external;
		if (store->sound_velocity > 13000 && store->sound_velocity < 17000)
			*ssv = 0.1 * store->sound_velocity;
		else
			*ssv = 1500.0;

		/* get travel times, angles */
		for (int i = 0; i < store->num_proc_beams; i++) {
			ttimes[i] = store->beamrange[i];
			angles[i] = store->angles[i];
			angles_forward[i] = store->angles_forward[i];
			angles_null[i] = 0.0;
			alongtrack_offset[i] = 0.0;
			heave[i] = store->heave_external;
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

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
		fprintf(stderr, "dbg2       draft:      %f\n", *draft);
		fprintf(stderr, "dbg2       ssv:        %f\n", *ssv);
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (int i = 0; i < *nbeams; i++)
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
int mbsys_image83p_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       detects:    %p\n", (void *)detects);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_image83p_struct *store = (struct mbsys_image83p_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get nbeams */
		*nbeams = store->num_proc_beams;

		/* get detects */
		for (int i = 0; i < *nbeams; i++) {
			detects[i] = MB_DETECT_PHASE;
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

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (int i = 0; i < *nbeams; i++)
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
int mbsys_image83p_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                    double *altitudev, int *error) {
	(void)mbio_ptr;  // Unused arg

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_image83p_struct *store = (struct mbsys_image83p_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;
	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get transducer depth */
		*transducer_depth = store->sonar_depth - store->heave_external;

		/* get altitude from depth closest to nadir */
		double xtrackmin = 999999.9;
		*altitudev = 0.0;
		for (int i = 0; i < store->num_proc_beams; i++) {
			if (mb_beam_ok(store->beamflag[i]) && fabs(store->bathacrosstrack[i]) < xtrackmin) {
				*altitudev = store->bath[i] - *transducer_depth;
				xtrackmin = fabs(store->bathacrosstrack[i]);
			}
		}

		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* done translating values */
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* deal with other record type */
	else {
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
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
int mbsys_image83p_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                               double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                               double *pitch, double *heave, int *error) {
	(void)mbio_ptr;  // Unused arg

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_image83p_struct *store = (struct mbsys_image83p_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get time */
		for (int i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get navigation */
		*navlon = store->nav_long;
		*navlat = store->nav_lat;

		/* get heading */
		*heading = (double) store->heading_external;

		/* get draft */
		*draft = (double) store->sonar_depth;

		/* get speed (convert knots to km/hr) */
		*speed = 1.852 * store->nav_speed * 0.1;

		/* get roll pitch  */
    *roll = (double) store->roll_external;
    *pitch = (double) store->pitch_external;
		*heave = (double) store->heave_external;

		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", __func__);
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
			fprintf(stderr, "dbg4       draft:      %f\n", *draft);
			fprintf(stderr, "dbg4       roll:       %f\n", *roll);
			fprintf(stderr, "dbg4       pitch:      %f\n", *pitch);
			fprintf(stderr, "dbg4       heave:      %f\n", *heave);
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

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind == MB_DATA_DATA) {
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
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_image83p_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
                              double navlat, double speed, double heading, double draft, double roll, double pitch, double heave,
                              int *error) {
	(void)mbio_ptr;  // Unused arg

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)store_ptr);
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
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_image83p_struct *store = (struct mbsys_image83p_struct *)store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA) {
		for (int i = 0; i < 7; i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation */

		store->nav_long = navlon;
		store->nav_lat = navlat;

		/* get heading */
		store->heading_external = (float) heading;

		/* get draft */
    bool sonar_depth_change = false;
    float dsonar_depth = 0.0;
    float sonar_depth_org = store->sonar_depth;
		store->sonar_depth = draft;
    if (store->sonar_depth != sonar_depth_org) {
      sonar_depth_change = true;
      dsonar_depth = store->sonar_depth - sonar_depth_org;
    }

		/* get speed (convert km/hr to knots) */
		store->nav_speed = (int)(0.539996 * speed * 10);

		/* get roll pitch and heave */
    store->roll_external = (float) roll;
    store->pitch_external = (float) pitch;
    float heave_external_org = store->heave_external;
		store->heave_external = heave;
    if (store->heave_external != heave_external_org) {
      sonar_depth_change = true;
      dsonar_depth += -(store->heave_external - heave_external_org);
    }

    /* apply any change to sonar_depth or heave to the bathymetry */
    if (sonar_depth_change) {
      for (int i = 0; i < store->num_proc_beams; i++) {
        if (store->range[i] > 0) {
          store->bath[i] += dsonar_depth;
        }
      }
    }
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_image83p_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error) {
	(void)mbio_ptr;  // Unused arg

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       copy_ptr:   %p\n", (void *)copy_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointers */
	struct mbsys_image83p_struct *store = (struct mbsys_image83p_struct *)store_ptr;
	struct mbsys_image83p_struct *copy = (struct mbsys_image83p_struct *)copy_ptr;

	/* copy the data */
	*copy = *store;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/

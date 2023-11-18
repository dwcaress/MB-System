/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_simrad3.c	3.00	2/22/2008
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
 * mbsys_simrad3.c contains the MBIO functions for handling data from
 * new (post-2006) Simrad multibeam sonars (e.g. EM710, EM3002, EM302, EM122).
 * The data formats associated with Simrad multibeams
 * (both old and new) include:
 *    MBSYS_SIMRAD formats (code in mbsys_simrad.c and mbsys_simrad.h):
 *      MBF_EMOLDRAW : MBIO ID 51 - Vendor EM1000, EM12S, EM12D, EM121
 *                   : MBIO ID 52 - aliased to 51
 *      MBF_EM12IFRM : MBIO ID 53 - IFREMER EM12S and EM12D
 *      MBF_EM12DARW : MBIO ID 54 - NERC EM12S
 *                   : MBIO ID 55 - aliased to 51
 *    MBSYS_SIMRAD3 formats (code in mbsys_simrad2.c and mbsys_simrad2.h):
 *      MBF_EM300RAW : MBIO ID 56 - Vendor EM3000, EM300, EM120
 *      MBF_EM300MBA : MBIO ID 57 - MBARI EM3000, EM300, EM120 for processing
 *    MBSYS_SIMRAD3 formats (code in mbsys_simrad3.c and mbsys_simrad3.h):
 *      MBF_EM710RAW : MBIO ID 58 - Vendor EM710
 *      MBF_EM710MBA : MBIO ID 59 - MBARI EM710 for processing
 *
 * Author:	D. W. Caress
 * Date:	February 22, 2008
 *
 *
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mbsys_simrad3.h"

/*--------------------------------------------------------------------*/
int mbsys_simrad3_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	const int status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_simrad3_struct), (void **)store_ptr, error);

	/* initialize everything to zero */
	if (status == MB_SUCCESS)
		memset(*store_ptr, 0, sizeof(struct mbsys_simrad3_struct));

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
int mbsys_simrad3_survey_alloc(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	// struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	// TODO(schwehr): Is this supposed to alloc something?

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
int mbsys_simrad3_extraparameters_alloc(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	int status = MB_SUCCESS;

	/* allocate memory for data structure if needed */
	if (store->extraparameters == NULL) {
		status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_simrad3_extraparameters_struct),
		                    (void **)&(store->extraparameters), error);
		if (status == MB_SUCCESS)
			memset(store->extraparameters, 0, sizeof(struct mbsys_simrad3_extraparameters_struct));
	}

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
int mbsys_simrad3_wc_alloc(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	int status = MB_SUCCESS;

	/* allocate memory for data structure if needed */
	if (store->wc == NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_simrad3_watercolumn_struct), (void **)&(store->wc),
		                    error);

	if (status == MB_SUCCESS) {

		/* get data structure pointer */
		struct mbsys_simrad3_watercolumn_struct *wc = (struct mbsys_simrad3_watercolumn_struct *)store->wc;

		/* initialize everything */
		wc->wtc_date = 0;       /* date = year*10000 + month*100 + day
		                Feb 26, 1995 = 19950226 */
		wc->wtc_msec = 0;       /* time since midnight in msec
		                08:12:51.234 = 29570234 */
		wc->wtc_count = 0;      /* sequential counter or input identifier */
		wc->wtc_serial = 0;     /* system 1 or system 2 serial number */
		wc->wtc_ndatagrams = 0; /* number of datagrams used to represent
		                the water column for this ping */
		wc->wtc_datagram = 0;   /* number this datagram */
		wc->wtc_ntx = 0;        /* number of transmit sectors */
		wc->wtc_nrx = 0;        /* number of receive beams */
		wc->wtc_nbeam = 0;      /* number of beams in this datagram */
		wc->wtc_ssv = 0;        /* sound speed at transducer (0.1 m/sec) */
		wc->wtc_sfreq = 0;      /* sampling frequency (0.01 Hz) */
		wc->wtc_heave = 0;      /* tx time heave at transducer (0.01 m) */
		wc->wtc_spare1 = 0;     /* spare */
		wc->wtc_spare2 = 0;     /* spare */
		wc->wtc_spare3 = 0;     /* spare */
		for (int i = 0; i < MBSYS_SIMRAD3_MAXTX; i++) {
			wc->wtc_txtiltangle[i] = 0; /* tilt angle (0.01 deg) */
			wc->wtc_txcenter[i] = 0;    /* center frequency (Hz) */
			wc->wtc_txsector[i] = 0;    /* transmit sector number (0-19) */
		}
		for (int i = 0; i < MBSYS_SIMRAD3_MAXBEAMS; i++) {
			wc->beam[i].wtc_rxpointangle = 0; /* Beam pointing angles in 0.01 degree,
			                      positive to port. These values are roll stabilized. */
			wc->beam[i].wtc_start_sample = 0; /* start sample number */
			wc->beam[i].wtc_beam_samples = 0; /* number of water column samples derived from
			                      each beam */
			wc->beam[i].wtc_sector = 0; /* transmit sector identifier */
			wc->beam[i].wtc_beam = 0;   /* beam 128 is first beam on
			                    second head of EM3000D */
			for (int j = 0; j < MBSYS_SIMRAD3_MAXRAWPIXELS; j++)
				wc->beam[i].wtc_amp[j] = 0; /* water column amplitude (dB) */
		}
	}

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
int mbsys_simrad3_attitude_alloc(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	int status = MB_SUCCESS;

	/* allocate memory for data structure if needed */
	if (store->attitude == NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_simrad3_attitude_struct),
		                    (void **)&(store->attitude), error);

	if (status == MB_SUCCESS) {

		/* get data structure pointer */
		struct mbsys_simrad3_attitude_struct *attitude = (struct mbsys_simrad3_attitude_struct *)store->attitude;

		/* initialize everything */
		attitude->att_date = 0;
		/* date = year*10000 + month*100 + day
		    Feb 26, 1995 = 19950226 */
		attitude->att_msec = 0;
		/* time since midnight in msec
		    08:12:51.234 = 29570234 */
		attitude->att_count = 0;
		/* sequential counter or input identifier */
		attitude->att_serial = 0;
		/* system 1 or system 2 serial number */
		attitude->att_ndata = 0;
		/* number of attitude data */
		for (int i = 0; i < MBSYS_SIMRAD3_MAXATTITUDE; i++) {
			attitude->att_time[i] = 0;
			/* time since record start (msec) */
			attitude->att_sensor_status[i] = 0;
			/* see note 12 above */
			attitude->att_roll[i] = 0;
			/* roll (0.01 degree) */
			attitude->att_pitch[i] = 0;
			/* pitch (0.01 degree) */
			attitude->att_heave[i] = 0;
			/* heave (cm) */
			attitude->att_heading[i] = 0;
			/* heading (0.01 degree) */
		}
		attitude->att_sensordescriptor = 0;
		/* heading status (0=inactive) */
	}

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
int mbsys_simrad3_netattitude_alloc(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	int status = MB_SUCCESS;

	/* allocate memory for data structure if needed */
	if (store->netattitude == NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_simrad3_netattitude_struct),
		                    (void **)&(store->netattitude), error);

	if (status == MB_SUCCESS) {
		/* get data structure pointer */
		struct mbsys_simrad3_netattitude_struct *netattitude = (struct mbsys_simrad3_netattitude_struct *)store->netattitude;

		/* initialize everything */
		netattitude->nat_date = 0;
		/* date = year*10000 + month*100 + day
		    Feb 26, 1995 = 19950226 */
		netattitude->nat_msec = 0;
		/* time since midnight in msec
		    08:12:51.234 = 29570234 */
		netattitude->nat_count = 0;
		/* sequential counter or input identifier */
		netattitude->nat_serial = 0;
		/* system 1 or system 2 serial number */
		netattitude->nat_ndata = 0;
		/* number of attitude data */
		netattitude->nat_sensordescriptor = 0; /* sensor system descriptor */
		for (int i = 0; i < MBSYS_SIMRAD3_MAXATTITUDE; i++) {
			netattitude->nat_time[i] = 0;
			/* time since record start (msec) */
			netattitude->nat_roll[i] = 0;
			/* roll (0.01 degree) */
			netattitude->nat_pitch[i] = 0;
			/* pitch (0.01 degree) */
			netattitude->nat_heave[i] = 0;
			/* heave (cm) */
			netattitude->nat_heading[i] = 0;
			/* heading (0.01 degree) */
			netattitude->nat_nbyte_raw[i] = 0; /* number of bytes in input datagram (Nd) */
			for (int j = 0; j < MBSYS_SIMRAD3_BUFFER_SIZE; j++)
				netattitude->nat_raw[i * MBSYS_SIMRAD3_BUFFER_SIZE + j] =
				    0; /* network attitude input datagram as received by datalogger */
		}
	}

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
int mbsys_simrad3_heading_alloc(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	int status = MB_SUCCESS;

	/* allocate memory for data structure if needed */
	if (store->heading == NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_simrad3_heading_struct), (void **)&(store->heading),
		                    error);

	if (status == MB_SUCCESS) {
		/* get data structure pointer */
		struct mbsys_simrad3_heading_struct *heading = (struct mbsys_simrad3_heading_struct *)store->heading;

		/* initialize everything */
		heading->hed_date = 0;
		/* date = year*10000 + month*100 + day
		    Feb 26, 1995 = 19950226 */
		heading->hed_msec = 0;
		/* time since midnight in msec
		    08:12:51.234 = 29570234 */
		heading->hed_count = 0;
		/* sequential counter or input identifier */
		heading->hed_serial = 0;
		/* system 1 or system 2 serial number */
		heading->hed_ndata = 0;
		/* number of heading data */
		for (int i = 0; i < MBSYS_SIMRAD3_MAXHEADING; i++) {
			heading->hed_time[i] = 0;
			/* time since record start (msec) */
			heading->hed_heading[i] = 0;
			/* heading (0.01 degree) */
		}
		heading->hed_heading_status = 0;
		/* heading status (0=inactive) */
	}

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
int mbsys_simrad3_ssv_alloc(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	int status = MB_SUCCESS;

	/* allocate memory for data structure if needed */
	if (store->ssv == NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_simrad3_ssv_struct), (void **)&(store->ssv), error);

	if (status == MB_SUCCESS) {
		/* get data structure pointer */
		struct mbsys_simrad3_ssv_struct *ssv = (struct mbsys_simrad3_ssv_struct *)store->ssv;

		/* initialize everything */
		ssv->ssv_date = 0;
		/* date = year*10000 + month*100 + day
		    Feb 26, 1995 = 19950226 */
		ssv->ssv_msec = 0;
		/* time since midnight in msec
		    08:12:51.234 = 29570234 */
		ssv->ssv_count = 0;
		/* sequential counter or input identifier */
		ssv->ssv_serial = 0;
		/* system 1 or system 2 serial number */
		ssv->ssv_ndata = 0;
		/* number of ssv data */
		for (int i = 0; i < MBSYS_SIMRAD3_MAXTILT; i++) {
			ssv->ssv_time[i] = 0;
			/* time since record start (msec) */
			ssv->ssv_ssv[i] = 0;
			/* ssv (0.1 m/s) */
		}
	}

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
int mbsys_simrad3_tilt_alloc(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	int status = MB_SUCCESS;

	/* allocate memory for data structure if needed */
	if (store->tilt == NULL)
		status =
		    mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_simrad3_tilt_struct), (void **)&(store->tilt), error);

	if (status == MB_SUCCESS) {
		/* get data structure pointer */
		struct mbsys_simrad3_tilt_struct *tilt = (struct mbsys_simrad3_tilt_struct *)store->tilt;

		/* initialize everything */
		tilt->tlt_date = 0;
		/* date = year*10000 + month*100 + day
		    Feb 26, 1995 = 19950226 */
		tilt->tlt_msec = 0;
		/* time since midnight in msec
		    08:12:51.234 = 29570234 */
		tilt->tlt_count = 0;
		/* sequential counter or input identifier */
		tilt->tlt_serial = 0;
		/* system 1 or system 2 serial number */
		tilt->tlt_ndata = 0;
		/* number of tilt data */
		for (int i = 0; i < MBSYS_SIMRAD3_MAXTILT; i++) {
			tilt->tlt_time[i] = 0;
			/* time since record start (msec) */
			tilt->tlt_tilt[i] = 0;
			/* tilt + forward (0.01 deg) */
		}
	}

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
int mbsys_simrad3_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
	}

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)*store_ptr;

	int status = MB_SUCCESS;

	/* deallocate memory for extraparameters data structure */
	if (store->extraparameters != NULL) {
		struct mbsys_simrad3_extraparameters_struct *extraparameters =
		    (struct mbsys_simrad3_extraparameters_struct *)store->extraparameters;
		if (extraparameters->xtr_data != NULL)
			status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(extraparameters->xtr_data), error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->extraparameters), error);
	}

	/* deallocate memory for water column data structure */
	if (store->wc != NULL)
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->wc), error);

	/* deallocate memory for attitude data structure */
	if (store->attitude != NULL)
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->attitude), error);

	/* deallocate memory for network attitude data structure */
	if (store->netattitude != NULL)
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->netattitude), error);

	/* deallocate memory for heading data structure */
	if (store->heading != NULL)
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->heading), error);

	/* deallocate memory for ssv data structure */
	if (store->ssv != NULL)
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->ssv), error);

	/* deallocate memory for tilt data structure */
	if (store->tilt != NULL)
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->tilt), error);

	/* deallocate memory for data structure */
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)store_ptr, error);

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
int mbsys_simrad3_zero_ss(int verbose, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to data descriptor */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	for (int ping_num = 0; ping_num < MBSYS_SIMRAD3_NUM_PING_STRUCTURES; ping_num++) {
		struct mbsys_simrad3_ping_struct *ping =
			(struct mbsys_simrad3_ping_struct *)&(store->pings[ping_num]);

		ping->png_ss_read = 0;          /* flag indicating actual reading of sidescan record */
		ping->png_ss_date = 0;          /* date = year*10000 + month*100 + day
		                        Feb 26, 1995 = 19950226 */
		ping->png_ss_msec = 0;          /* time since midnight in msec
		                        08:12:51.234 = 29570234 */
		ping->png_ss_count = 0;         /* sequential counter or input identifier */
		ping->png_ss_serial = 0;        /* system 1 or system 2 serial number */
		ping->png_ss_sample_rate = 0.0; /* sampling rate (Hz) */
		ping->png_r_zero = 0;           /* range to normal incidence used in TVG
		                        (R0 predicted) in samples */
		ping->png_bsn = 0;              /* normal incidence backscatter (BSN) (0.1 dB) */
		ping->png_bso = 0;              /* oblique incidence backscatter (BSO) (0.1 dB) */
		ping->png_tx = 0;               /* Tx beamwidth (0.1 deg) */
		ping->png_tvg_crossover = 0;
		/* TVG law crossover angle (0.1 deg) */
		ping->png_nbeams_ss = 0; /* number of beams with sidescan */
		ping->png_npixels = 0;   /* number of pixels of sidescan */
		for (int beam_num = 0; beam_num < MBSYS_SIMRAD3_MAXBEAMS; beam_num++) {
			ping->png_sort_direction[beam_num] = 0;
			/* sorting direction - The first sample in a beam
			    has lowest range if 1, highest if -- 1. Note
			    that the ranges in the seabed image datagram
			    are all two-- way from time of transmit to
			    time of receive. */
			ping->png_beam_samples[beam_num] = 0;
			/* number of sidescan samples derived from
			    each beam */
			ping->png_start_sample[beam_num] = 0;
			/* start sample number */
			ping->png_ssdetection[beam_num] = 0; /* Detection info:
			                       This datagram may contain data for beams with and without a
			                       valid detection. Eight bits (0-7) gives details about the detection:
			                        A) If the most significant bit (bit7) is zero, this beam has a valid
			                            detection. Bit 0-3 is used to specify how the range for this beam
			                            is calculated
			                            0: Amplitude detect
			                            1: Phase detect
			                            2-15: Future use
			                        B) If the most significant bit is 1, this beam has an invalid
			                            detection. Bit 4-6 is used to specify how the range (and x,y,z
			                            parameters) for this beam is calculated
			                            0: Normal detection
			                            1: Interpolated or extrapolated from neighbour detections
			                            2: Estimated
			                            3: Rejected candidate
			                            4: No detection data is available for this beam (all parameters
			                                are set to zero)
			                            5-7: Future use
			                        The invalid range has been used to fill in amplitude samples in
			                        the seabed image datagram.
			                            bit 7: 0 = good detection
			                            bit 7: 1 = bad detection
			                            bit 3: 0 = amplitude detect
			                            bit 3: 1 = phase detect
			                            bits 4-6: 0 = normal detection
			                            bits 4-6: 1 = interpolated from neighbor detections
			                            bits 4-6: 2 = estimated
			                            bits 4-6: 3 = rejected
			                            bits 4-6: 4 = no detection available
			                            other bits : future use */
			ping->png_center_sample[beam_num] = 0;
			/* center sample number */
		}
		for (int i = 0; i < MBSYS_SIMRAD3_MAXRAWPIXELS; i++) {
			ping->png_ssraw[i] = 0; /* the raw sidescan ordered port to starboard */
		}
		ping->png_pixel_size = 0.0; /* processed sidescan pixel size (m) */
		ping->png_pixels_ss = 0;    /* number of processed sidescan pixels stored */
		for (int i = 0; i < MBSYS_SIMRAD3_MAXPIXELS; i++) {
			ping->png_ss[i] = 0;           /* the processed sidescan ordered port to starboard */
			ping->png_ssalongtrack[i] = 0; /* the processed sidescan alongtrack distances (0.01 m) */
		}
	}

	/* assume success */
	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad3_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss,
                             int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get beam and pixel numbers */
		struct mbsys_simrad3_ping_struct *ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);
		*nbath = ping->png_nbeams;
		*namp = *nbath;
		*nss = MBSYS_SIMRAD3_MAXPIXELS;
	}
	else {
		/* get beam and pixel numbers */
		*nbath = 0;
		*namp = 0;
		*nss = 0;
	}

	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		fprintf(stderr, "dbg2       nbath:      %d\n", *nbath);
		fprintf(stderr, "dbg2       namp:      %d\n", *namp);
		fprintf(stderr, "dbg2       nss:       %d\n", *nss);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad3_pingnumber(int verbose, void *mbio_ptr, unsigned int *pingnumber, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)mb_io_ptr->store_data;

	/* extract data from structure */
	struct mbsys_simrad3_ping_struct *ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);
	*pingnumber = ping->png_count;

	int status = MB_SUCCESS;

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
int mbsys_simrad3_sonartype(int verbose, void *mbio_ptr, void *store_ptr, int *sonartype, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	// struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	/* get sonar type */
	*sonartype = MB_TOPOGRAPHY_TYPE_MULTIBEAM;

	int status = MB_SUCCESS;

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
int mbsys_simrad3_sidescantype(int verbose, void *mbio_ptr, void *store_ptr, int *ss_type, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	// struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	/* get sidescan type */
	*ss_type = MB_SIDESCAN_LOGARITHMIC;

	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ss_type:    %d\n", *ss_type);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad3_preprocess(int verbose,     /* in: verbosity level set on command line 0..N */
                             void *mbio_ptr,  /* in: see mb_io.h:/^struct mb_io_struct/ */
                             void *store_ptr, /* in: see mbsys_reson7k.h:/^struct mbsys_reson7k_struct/ */
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
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get preprocessing parameters */
  struct mb_preprocess_struct *pars = (struct mb_preprocess_struct *)preprocess_pars_ptr;

	/* get data structure pointers */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;
	// struct mb_platform_struct *platforms = (struct mb_platform_struct *)platform_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "dbg2       target_sensor:              %d\n", pars->target_sensor);
		fprintf(stderr, "dbg2       timestamp_changed:          %d\n", pars->timestamp_changed);
		fprintf(stderr, "dbg2       time_d:                     %f\n", pars->time_d);
		fprintf(stderr, "dbg2       n_nav:                      %d\n", pars->n_nav);
		fprintf(stderr, "dbg2       nav_time_d:                 %p\n", pars->nav_time_d);
		fprintf(stderr, "dbg2       nav_lon:                    %p\n", pars->nav_lon);
		fprintf(stderr, "dbg2       nav_lat:                    %p\n", pars->nav_lat);
		fprintf(stderr, "dbg2       nav_speed:                  %p\n", pars->nav_speed);
		fprintf(stderr, "dbg2       n_sensordepth:              %d\n", pars->n_sensordepth);
		fprintf(stderr, "dbg2       sensordepth_time_d:         %p\n", pars->sensordepth_time_d);
		fprintf(stderr, "dbg2       sensordepth_sensordepth:    %p\n", pars->sensordepth_sensordepth);
		fprintf(stderr, "dbg2       n_heading:                  %d\n", pars->n_heading);
		fprintf(stderr, "dbg2       heading_time_d:             %p\n", pars->heading_time_d);
		fprintf(stderr, "dbg2       heading_heading:            %p\n", pars->heading_heading);
		fprintf(stderr, "dbg2       n_altitude:                 %d\n", pars->n_altitude);
		fprintf(stderr, "dbg2       altitude_time_d:            %p\n", pars->altitude_time_d);
		fprintf(stderr, "dbg2       altitude_altitude:          %p\n", pars->altitude_altitude);
		fprintf(stderr, "dbg2       n_attitude:                 %d\n", pars->n_attitude);
		fprintf(stderr, "dbg2       attitude_time_d:            %p\n", pars->attitude_time_d);
		fprintf(stderr, "dbg2       attitude_roll:              %p\n", pars->attitude_roll);
		fprintf(stderr, "dbg2       attitude_pitch:             %p\n", pars->attitude_pitch);
		fprintf(stderr, "dbg2       attitude_heave:             %p\n", pars->attitude_heave);
		fprintf(stderr, "dbg2       n_kluge:                    %d\n", pars->n_kluge);
		for (int i = 0; i < pars->n_kluge; i++)
			fprintf(stderr, "dbg2       kluge_id[%d]:                    %d\n", i, pars->kluge_id[i]);
	}

	int status = MB_SUCCESS;

  /* if called with store_ptr == NULL then called after mb_read_init() but before
      any data are read - for some formats this allows kluge options to set special
      reading conditions/behaviors */
  if (store_ptr == NULL) {
		for (int i = 0; i < pars->n_kluge; i++)
			if (pars->kluge_id[i] == MB_PR_KLUGE_IGNORESNIPPETS) {
        bool *ignore_snippets = (bool *)&mb_io_ptr->save4;
        *ignore_snippets = true;
      }
  }

	/* deal with a survey record */
	else if (store->kind == MB_DATA_DATA) {

  	/* get data structure pointers */
    struct mbsys_simrad3_ping_struct *ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);

		/* depth sensor offsets - used in place of heave for underwater platforms */
		int depthsensor_mode = MBSYS_SIMRAD3_ZMODE_UNKNOWN;

		/*--------------------------------------------------------------*/
		/* get depth sensor mode from the start record
		    NI => Use heave
		    IN => Depth sensor */
		/*--------------------------------------------------------------*/
		if (store->par_dsh[0] == 'I')
			depthsensor_mode = MBSYS_SIMRAD3_ZMODE_USE_SENSORDEPTH_ONLY;
		else if (store->par_dsh[0] == 'N')
			depthsensor_mode = MBSYS_SIMRAD3_ZMODE_USE_SENSORDEPTH_AND_HEAVE;
		else
			depthsensor_mode = MBSYS_SIMRAD3_ZMODE_USE_HEAVE_ONLY;

		/*--------------------------------------------------------------*/
		/* change timestamp if indicated */
		/*--------------------------------------------------------------*/
		double time_d;
		int time_i[7];
		if (pars->timestamp_changed) {
			time_d = pars->time_d;
			mb_get_date(verbose, time_d, time_i);

			/* set time */
			ping->png_date = 10000 * time_i[0] + 100 * time_i[1] + time_i[2];
			ping->png_msec = 3600000 * time_i[3] + 60000 * time_i[4] + 1000 * time_i[5] + 0.001 * time_i[6];
			store->date = ping->png_date;
			store->msec = ping->png_msec;
			fprintf(stderr,
			        "Timestamp changed in function %s: "
			        "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d "
			        "| ping_number:%d\n",
			        __func__, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], ping->png_count);
		}

		/*--------------------------------------------------------------*/
		/* interpolate ancillary values from arrays provided by mbpreprocess  */
		/*--------------------------------------------------------------*/

		/* get time */
		time_i[0] = ping->png_date / 10000;
		time_i[1] = (ping->png_date % 10000) / 100;
		time_i[2] = ping->png_date % 100;
		time_i[3] = ping->png_msec / 3600000;
		time_i[4] = (ping->png_msec % 3600000) / 60000;
		time_i[5] = (ping->png_msec % 60000) / 1000;
		time_i[6] = (ping->png_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &time_d);

		double navlon;
		double navlat;
		int jnav;
    	int interp_error = MB_ERROR_NO_ERROR;
		mb_linear_interp_longitude(verbose, pars->nav_time_d - 1, pars->nav_lon - 1, pars->n_nav, time_d, &navlon,
		                                           &jnav, &interp_error);
		if (navlon < -180.0)
			navlon += 360.0;
		else if (navlon > 180.0)
			navlon -= 360.0;
		mb_linear_interp_latitude(verbose, pars->nav_time_d - 1, pars->nav_lat - 1, pars->n_nav, time_d, &navlat,
		                                          &jnav, &interp_error);
		double speed;
		mb_linear_interp(verbose, pars->nav_time_d - 1, pars->nav_speed - 1, pars->n_nav, time_d, &speed, &jnav, &interp_error);

		/* interpolate sensordepth */
		double sensordepth;
		int jsensordepth;
		mb_linear_interp(verbose, pars->sensordepth_time_d - 1, pars->sensordepth_sensordepth - 1,
		                                 pars->n_sensordepth, time_d, &sensordepth, &jsensordepth, &interp_error);

		/* interpolate heading */
		double heading;
		int jheading;
		mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1, pars->n_heading,
		                                         time_d, &heading, &jheading, &interp_error);
		if (heading < 0.0)
			heading += 360.0;
		else if (heading >= 360.0)
			heading -= 360.0;

		/* interpolate altitude */
		double altitude;
		int jaltitude;
		mb_linear_interp(verbose, pars->altitude_time_d - 1, pars->altitude_altitude - 1, pars->n_altitude,
		                                 time_d, &altitude, &jaltitude, &interp_error);

		/* interpolate attitude */
		double roll;
		int jattitude;
		mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude, time_d,
		                                 &roll, &jattitude, &interp_error);
		double pitch;
		mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude, time_d,
		                                 &pitch, &jattitude, &interp_error);
		double heave;
		mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_heave - 1, pars->n_attitude, time_d,
		                                 &heave, &jattitude, &interp_error);

		/* insert navigation */
		ping->png_longitude = 10000000 * navlon;
		ping->png_latitude = 20000000 * navlat;

		/* insert heading */
		if (heading < 0.0)
			heading += 360.0;
		else if (heading > 360.0)
			heading -= 360.0;
		ping->png_heading = (int)rint(heading * 100);

		/* insert roll pitch and heave */
		ping->png_roll = (int)rint(roll / 0.01);
		ping->png_pitch = (int)rint(pitch / 0.01);
		ping->png_heave = (int)rint(heave / 0.01);

		/*--------------------------------------------------------------*/
		/* get transducer offsets */
		/*--------------------------------------------------------------*/
		/* transmit and receive array offsets */
		// double tx_x, tx_y, tx_z;
		double tx_h, tx_r, tx_p;
		// double rx_x, rx_y, rx_z;
		double rx_h, rx_r, rx_p;

		if (store->par_stc == 0) {
			// tx_x = store->par_s1x;
			// tx_y = store->par_s1y;
			// tx_z = store->par_s1z;
			tx_h = store->par_s1h;
			tx_r = store->par_s1r;
			tx_p = store->par_s1p;
			// rx_x = store->par_s2x;
			// rx_y = store->par_s2y;
			// rx_z = store->par_s2z;
			rx_h = store->par_s2h;
			rx_r = store->par_s2r;
			rx_p = store->par_s2p;
		}
		else if (store->par_stc == 1) {
			// tx_x = store->par_s1x;
			// tx_y = store->par_s1y;
			// tx_z = store->par_s1z;
			tx_h = store->par_s1h;
			tx_r = store->par_s1r;
			tx_p = store->par_s1p;
			// rx_x = store->par_s1x;
			// rx_y = store->par_s1y;
			// rx_z = store->par_s1z;
			rx_h = store->par_s1h;
			rx_r = store->par_s1r;
			rx_p = store->par_s1p;
		}
		else if (store->par_stc == 2 && ping->png_serial == store->par_serial_1) {
			// tx_x = store->par_s1x;
			// tx_y = store->par_s1y;
			// tx_z = store->par_s1z;
			tx_h = store->par_s1h;
			tx_r = store->par_s1r;
			tx_p = store->par_s1p;
			// rx_x = store->par_s1x;
			// rx_y = store->par_s1y;
			// rx_z = store->par_s1z;
			rx_h = store->par_s1h;
			rx_r = store->par_s1r;
			rx_p = store->par_s1p;
		}
		else if (store->par_stc == 2 && ping->png_serial == store->par_serial_2) {
			// tx_x = store->par_s2x;
			// tx_y = store->par_s2y;
			// tx_z = store->par_s2z;
			tx_h = store->par_s2h;
			tx_r = store->par_s2r;
			tx_p = store->par_s2p;
			// rx_x = store->par_s2x;
			// rx_y = store->par_s2y;
			// rx_z = store->par_s2z;
			rx_h = store->par_s2h;
			rx_r = store->par_s2r;
			rx_p = store->par_s2p;
		}
		else if (store->par_stc == 3 && ping->png_serial == store->par_serial_1) {
			// tx_x = store->par_s1x;
			// tx_y = store->par_s1y;
			// tx_z = store->par_s1z;
			tx_h = store->par_s1h;
			tx_r = store->par_s1r;
			tx_p = store->par_s1p;
			// rx_x = store->par_s2x;
			// rx_y = store->par_s2y;
			// rx_z = store->par_s2z;
			rx_h = store->par_s2h;
			rx_r = store->par_s2r;
			rx_p = store->par_s2p;
		}
		else if (store->par_stc == 3 && ping->png_serial == store->par_serial_2) {
			// tx_x = store->par_s1x;
			// tx_y = store->par_s1y;
			// tx_z = store->par_s1z;
			tx_h = store->par_s1h;
			tx_r = store->par_s1r;
			tx_p = store->par_s1p;
			// rx_x = store->par_s3x;
			// rx_y = store->par_s3y;
			// rx_z = store->par_s3z;
			rx_h = store->par_s3h;
			rx_r = store->par_s3r;
			rx_p = store->par_s3p;
		}
		else if (store->par_stc == 4 && ping->png_serial == store->par_serial_1) {
			// tx_x = store->par_s0x;
			// tx_y = store->par_s0y;
			// tx_z = store->par_s0z;
			tx_h = store->par_s0h;
			tx_r = store->par_s0r;
			tx_p = store->par_s0p;
			// rx_x = store->par_s2x;
			// rx_y = store->par_s2y;
			// rx_z = store->par_s2z;
			rx_h = store->par_s2h;
			rx_r = store->par_s2r;
			rx_p = store->par_s2p;
		}
		else if (store->par_stc == 4 && ping->png_serial == store->par_serial_2) {
			// tx_x = store->par_s1x;
			// tx_y = store->par_s1y;
			// tx_z = store->par_s1z;
			tx_h = store->par_s1h;
			tx_r = store->par_s1r;
			tx_p = store->par_s1p;
			// rx_x = store->par_s3x;
			// rx_y = store->par_s3y;
			// rx_z = store->par_s3z;
			rx_h = store->par_s3h;
			rx_r = store->par_s3r;
			rx_p = store->par_s3p;
		}

		/* insert sensordepth if requested */
		// if (depthsensor_mode == MBSYS_SIMRAD3_ZMODE_USE_SENSORDEPTH_ONLY) {
			// ping->png_xducer_depth = sensordepth;
		// } else {
			// ping->png_xducer_depth = 0.5 * (tx_z + rx_z) - store->par_wlz + heave;
		// }

		double transmit_heading, transmit_heave, transmit_roll, transmit_pitch;
		double receive_heading, receive_heave, receive_roll, receive_pitch;

		/* variables for beam angle calculation */
		mb_3D_orientation tx_align;
		mb_3D_orientation tx_orientation;
		double tx_steer;
		mb_3D_orientation rx_align;
		mb_3D_orientation rx_orientation;
		double rx_steer;
		double beamAzimuth;
		double beamDepression;
		double *pixel_size, *swath_width;

		/*--------------------------------------------------------------*/
		/* calculate corrected ranges, angles, and bathymetry for each beam */
		/*--------------------------------------------------------------*/
		for (int i = 0; i < ping->png_nbeams; i++) {
			/* calculate time of transmit and receive */
			const double transmit_time_d = time_d + (double)ping->png_raw_txoffset[ping->png_raw_rxsector[i]];
			const double receive_time_d = transmit_time_d + ping->png_raw_rxrange[i];

			/* merge heading from best available source */
			mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
			                                         pars->n_heading, transmit_time_d, &transmit_heading, &jheading, error);
			mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
			                                         pars->n_heading, receive_time_d, &receive_heading, &jheading, error);
			if (transmit_heading < 0.0)
				transmit_heading += 360.0;
			else if (transmit_heading >= 360.0)
				transmit_heading -= 360.0;
			if (receive_heading < 0.0)
				receive_heading += 360.0;
			else if (receive_heading >= 360.0)
				receive_heading -= 360.0;

			/* get attitude from best available source */
			mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude,
			                                 transmit_time_d, &transmit_roll, &jattitude, error);
			mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude,
				                                 transmit_time_d, &transmit_pitch, &jattitude, error);
			mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_heave - 1, pars->n_attitude,
				                                 transmit_time_d, &transmit_heave, &jattitude, error);
			mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude,
			                                 receive_time_d, &receive_roll, &jattitude, error);
			mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude,
				                                 receive_time_d, &receive_pitch, &jattitude, error);
			mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_heave - 1, pars->n_attitude,
				                                 receive_time_d, &receive_heave, &jattitude, error);

			/* use sensordepth instead of heave for submerged platforms */
			if (depthsensor_mode == MBSYS_SIMRAD3_ZMODE_USE_SENSORDEPTH_ONLY) {
				mb_linear_interp(verbose, pars->sensordepth_time_d - 1, pars->sensordepth_sensordepth - 1,
				                                 pars->n_sensordepth, transmit_time_d, &transmit_heave, &jsensordepth, error);
				mb_linear_interp(verbose, pars->sensordepth_time_d - 1, pars->sensordepth_sensordepth - 1,
				                                 pars->n_sensordepth, receive_time_d, &receive_heave, &jsensordepth, error);
				heave = transmit_heave;
			}

			/* get ssv and range */
			if (ping->png_ssv <= 0)
				ping->png_ssv = 150;
			ping->png_range[i] = ping->png_raw_rxrange[i];

			/* ping->png_bheave[i] is the difference between the heave at the ping timestamp time that is factored
			 * into the ping->png_xducer_depth value and the average heave at the sector transmit time and the beam receive time
			 */
			ping->png_bheave[i] = 0.5 * (receive_heave + transmit_heave) - heave;

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
			if (tx_h <= 90.0 || tx_h >= 270.0) {
				tx_align.roll = tx_r;
				tx_align.pitch = tx_p;
				tx_align.heading = tx_h;
				tx_steer = (0.01 * (double)ping->png_raw_txtiltangle[ping->png_raw_rxsector[i]]);
			}
			else {
				tx_align.roll = -tx_r;
				tx_align.pitch = -tx_p;
				tx_align.heading = tx_h - 180.0;
				tx_steer = -(0.01 * (double)ping->png_raw_txtiltangle[ping->png_raw_rxsector[i]]);
			}
			tx_orientation.roll = transmit_roll;
			tx_orientation.pitch = transmit_pitch;
			tx_orientation.heading = transmit_heading;
			if (rx_h <= 90.0 || rx_h >= 270.0) {
				rx_align.roll = rx_r;
				rx_align.pitch = rx_p;
				rx_align.heading = rx_h;
				rx_steer = (0.01 * (double)ping->png_raw_rxpointangle[i]);
			}
			else {
				rx_align.roll = -rx_r;
				rx_align.pitch = -rx_p;
				rx_align.heading = rx_h - 180.0;
				rx_steer = -(0.01 * (double)ping->png_raw_rxpointangle[i]);
			}
			rx_orientation.roll = receive_roll;
			rx_orientation.pitch = receive_pitch;
			rx_orientation.heading = receive_heading;
			const double reference_heading = heading;

			status = mb_beaudoin(verbose, tx_align, tx_orientation, tx_steer, rx_align, rx_orientation, rx_steer,
			                     reference_heading, &beamAzimuth, &beamDepression, error);
			ping->png_depression[i] = 90.0 - beamDepression;
			ping->png_azimuth[i] = 90.0 + beamAzimuth;
			if (ping->png_azimuth[i] < 0.0)
				ping->png_azimuth[i] += 360.0;

			/* calculate beamflag */
			const mb_u_char detection_mask = (mb_u_char)ping->png_raw_rxdetection[i];
			if (store->sonar == MBSYS_SIMRAD3_M3 && (ping->png_detection[i] & 128) == 128) {
				ping->png_beamflag[i] = MB_FLAG_NULL;
				ping->png_raw_rxdetection[i] = ping->png_raw_rxdetection[i] | 128;
			}
			else if ((detection_mask & 128) == 128) {
				if ((detection_mask & 15) == 0) {
					ping->png_beamflag[i] = MB_FLAG_FLAG + MB_FLAG_SONAR;
				}
				else if ((detection_mask & 15) == 1) {
					ping->png_beamflag[i] = MB_FLAG_FLAG + MB_FLAG_INTERPOLATE;
				}
				else if ((detection_mask & 15) == 2) {
					ping->png_beamflag[i] = MB_FLAG_FLAG + MB_FLAG_INTERPOLATE;
				}
				else if ((detection_mask & 15) == 3) {
					ping->png_beamflag[i] = MB_FLAG_FLAG + MB_FLAG_SONAR;
					;
				}
				else if ((detection_mask & 15) == 4) {
					ping->png_beamflag[i] = MB_FLAG_NULL;
				}
			}
			else if (ping->png_clean[i] != 0) {
				ping->png_beamflag[i] = MB_FLAG_FLAG + MB_FLAG_SONAR;
			}
			else {
				ping->png_beamflag[i] = MB_FLAG_NONE;
			}

			/* check for NaN value */
			if (isnan(ping->png_depth[i])
          || isnan(ping->png_acrosstrack[i])
          || isnan(ping->png_alongtrack[i])) {
				ping->png_beamflag[i] = MB_FLAG_NULL;
			}
		}

		/* generate processed sidescan */
		pixel_size = (double *)&mb_io_ptr->saved1;
		swath_width = (double *)&mb_io_ptr->saved2;
		ping->png_pixel_size = 0;
		ping->png_pixels_ss = 0;
    if (ping->png_ss_read)
		  status = mbsys_simrad3_makess(verbose, mbio_ptr, store_ptr, false, pixel_size, false, swath_width, 1, error);
    else
      status = mbsys_simrad3_zero_ss(verbose, store_ptr, error);
	}

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
int mbsys_simrad3_extract_platform(int verbose, void *mbio_ptr, void *store_ptr, int *kind, void **platform_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:         %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:      %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       platform_ptr:   %p\n", (void *)platform_ptr);
		fprintf(stderr, "dbg2       *platform_ptr:  %p\n", (void *)*platform_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	int status = MB_SUCCESS;

	/* if needed allocate a new platform structure */
	if (*platform_ptr == NULL) {
		status = mb_platform_init(verbose, (void **)platform_ptr, error);
	}

	/* extract sensor offsets from installation record */
	if (*platform_ptr != NULL) {
		/* get pointer to platform structure */
		struct mb_platform_struct *platform = (struct mb_platform_struct *)(*platform_ptr);

		/* look for multibeam sensor, add it if necessary */
		int sensor_multibeam = -1;
		for (int isensor = 0; isensor < platform->num_sensors && sensor_multibeam < 0; isensor++) {
			if (platform->sensors[isensor].type == MB_SENSOR_TYPE_SONAR_MULTIBEAM &&
			    platform->sensors[isensor].num_offsets == 2) {
				sensor_multibeam = isensor;
			}
		}
		int capability1;
		int capability2;
		// int multibeam_type;
		if (sensor_multibeam < 0) {
			/* set sensor 0 (multibeam) */
			int multibeam_offsets;
			if (store->par_stc > 1) {
				// multibeam_type = MB_SENSOR_TYPE_SONAR_MULTIBEAM_TWOHEAD;
				multibeam_offsets = 4;
			}
			else {
				// multibeam_type = MB_SENSOR_TYPE_SONAR_MULTIBEAM;
				multibeam_offsets = 2;
			}

			mb_path multibeam_model;
			switch (store->sonar) {
			case MBSYS_SIMRAD3_M3:
				strcpy(multibeam_model, "M3");
				break;
			case MBSYS_SIMRAD3_EM2045:
				strcpy(multibeam_model, "EM2045");
				break;
			case MBSYS_SIMRAD3_EM2040:
				strcpy(multibeam_model, "EM2040");
				break;
			case MBSYS_SIMRAD3_EM710:
				strcpy(multibeam_model, "EM710");
				break;
			case MBSYS_SIMRAD3_EM712:
				strcpy(multibeam_model, "EM712");
				break;
			case MBSYS_SIMRAD3_EM850:
				strcpy(multibeam_model, "EM850");
				break;
			case MBSYS_SIMRAD3_EM302:
				strcpy(multibeam_model, "EM302");
				break;
			case MBSYS_SIMRAD3_EM122:
				strcpy(multibeam_model, "EM122");
				break;
			case MBSYS_SIMRAD3_EM120:
				strcpy(multibeam_model, "EM120");
				break;
			case MBSYS_SIMRAD3_EM300:
				strcpy(multibeam_model, "EM300");
				break;
			case MBSYS_SIMRAD3_EM1002:
				strcpy(multibeam_model, "EM1002");
				break;
			case MBSYS_SIMRAD3_EM2000:
				strcpy(multibeam_model, "EM2000");
				break;
			case MBSYS_SIMRAD3_EM3000:
				strcpy(multibeam_model, "EM3000");
				break;
			case MBSYS_SIMRAD3_EM3000D_1:
				strcpy(multibeam_model, "EM3000D_1");
				break;
			case MBSYS_SIMRAD3_EM3000D_2:
				strcpy(multibeam_model, "EM3000D_2");
				break;
			case MBSYS_SIMRAD3_EM3000D_3:
				strcpy(multibeam_model, "EM3000D_3");
				break;
			case MBSYS_SIMRAD3_EM3000D_4:
				strcpy(multibeam_model, "EM3000D_4");
				break;
			case MBSYS_SIMRAD3_EM3000D_5:
				strcpy(multibeam_model, "EM3000D_5");
				break;
			case MBSYS_SIMRAD3_EM3000D_6:
				strcpy(multibeam_model, "EM3000D_6");
				break;
			case MBSYS_SIMRAD3_EM3000D_7:
				strcpy(multibeam_model, "EM3000D_7");
				break;
			case MBSYS_SIMRAD3_EM3000D_8:
				strcpy(multibeam_model, "EM3000D_8");
				break;
			case MBSYS_SIMRAD3_EM3002:
				strcpy(multibeam_model, "EM3002");
				break;
			case MBSYS_SIMRAD3_EM12S:
				strcpy(multibeam_model, "EM12S");
				break;
			case MBSYS_SIMRAD3_EM12D:
				strcpy(multibeam_model, "EM12D");
				break;
			case MBSYS_SIMRAD3_EM121:
				strcpy(multibeam_model, "EM121");
				break;
			case MBSYS_SIMRAD3_EM100:
				strcpy(multibeam_model, "EM100");
				break;
			case MBSYS_SIMRAD3_EM1000:
				strcpy(multibeam_model, "EM1000");
				break;
			default:
				sprintf(multibeam_model, "Unknown sonar model %d", store->sonar);
			}
			mb_path multibeam_serial;
			sprintf(multibeam_serial, "%d", store->par_serial_1);
			capability1 = MB_SENSOR_CAPABILITY1_NONE;
			capability2 = MB_SENSOR_CAPABILITY2_TOPOGRAPHY_MULTIBEAM + MB_SENSOR_CAPABILITY2_BACKSCATTER_MULTIBEAM;
			const int num_offsets = multibeam_offsets;
			const int num_time_latency = 0;
			status =
			    mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_SONAR_MULTIBEAM, multibeam_model, "Kongsberg",
			                           multibeam_serial, capability1, capability2, num_offsets, num_time_latency, error);
			if (status == MB_SUCCESS) {
				sensor_multibeam = platform->num_sensors - 1;
			}
		}
		if (sensor_multibeam >= 0) {
			if (status == MB_SUCCESS) {
				platform->source_bathymetry = sensor_multibeam;
				platform->source_backscatter = sensor_multibeam;
			}
			/* store->par_stc:
			 * System transducer configuration
			 *      0 = Single TX + single RX
			 *              EM122, EM302, EM710, EM2040-Single
			 *      1 = Single head
			 *              EM3002S, EM2040C-Single, EM2040P
			 *      2 = Dual Head
			 *              EM3002-Dual, EM2040C-Dual
			 *      3 = Single TX + Dual RX
			 *              EM2040-Dual-RX
			 *      4 = Dual TX + Dual RX
			 *              EM2040-Dual-TX
			 *  If present, the STC parameter can be used in
			 *  decoding of the transducer installation parameters:
			 *      STC  S0X/Y/Z/R/P/H  S1X/Y/Z/R/P/H  S2X/Y/Z/R/P/H  S3X/Y/Z/R/P/H
			 *      ---  -------------  -------------  -------------  -------------
			 *       0        ----            TX             RX           ----
			 *       1        ----           Head           ----          ----
			 *       2        ----          Head 1         Head 2         ----
			 *       3        ----            TX            RX 1          RX 2
			 *       4        TX 1           TX 2           RX 1          RX 2
			 */
			if (store->par_stc == 0) {
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 0, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s1y,
					    (double)store->par_s1x, (double)-store->par_s1z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s1h,
					    (double)store->par_s1r, (double)store->par_s1p, error);
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 1, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s2y,
					    (double)store->par_s2x, (double)-store->par_s2z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s2h,
					    (double)store->par_s2r, (double)store->par_s2p, error);
			}
			else if (store->par_stc == 1) {
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 0, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s1y,
					    (double)store->par_s1x, (double)-store->par_s1z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s1h,
					    (double)store->par_s1r, (double)store->par_s1p, error);
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 1, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s1y,
					    (double)store->par_s1x, (double)-store->par_s1z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s1h,
					    (double)store->par_s1r, (double)store->par_s1p, error);
			}
			else if (store->par_stc == 2) {
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 0, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s1y,
					    (double)store->par_s1x, (double)-store->par_s1z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s1h,
					    (double)store->par_s1r, (double)store->par_s1p, error);
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 1, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s1y,
					    (double)store->par_s1x, (double)-store->par_s1z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s1h,
					    (double)store->par_s1r, (double)store->par_s1p, error);
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 2, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s2y,
					    (double)store->par_s2x, (double)-store->par_s2z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s2h,
					    (double)store->par_s2r, (double)store->par_s2p, error);
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 3, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s2y,
					    (double)store->par_s2x, (double)-store->par_s2z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s2h,
					    (double)store->par_s2r, (double)store->par_s2p, error);
			}
			else if (store->par_stc == 3) {
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 0, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s1y,
					    (double)store->par_s1x, (double)-store->par_s1z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s1h,
					    (double)store->par_s1r, (double)store->par_s1p, error);
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 1, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s2y,
					    (double)store->par_s2x, (double)-store->par_s2z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s2h,
					    (double)store->par_s2r, (double)store->par_s2p, error);
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 2, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s1y,
					    (double)store->par_s1x, (double)-store->par_s1z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s1h,
					    (double)store->par_s1r, (double)store->par_s1p, error);
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 3, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s3y,
					    (double)store->par_s3x, (double)-store->par_s3z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s3h,
					    (double)store->par_s3r, (double)store->par_s3p, error);
			}
			else if (store->par_stc == 4) {
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 0, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s0y,
					    (double)store->par_s0x, (double)-store->par_s0z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s0h,
					    (double)store->par_s0r, (double)store->par_s0p, error);
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 1, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s2y,
					    (double)store->par_s2x, (double)-store->par_s2z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s2h,
					    (double)store->par_s2r, (double)store->par_s2p, error);
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 2, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s1y,
					    (double)store->par_s1x, (double)-store->par_s1z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s1h,
					    (double)store->par_s1r, (double)store->par_s1p, error);
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 3, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s3y,
					    (double)store->par_s3x, (double)-store->par_s3z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s3h,
					    (double)store->par_s3r, (double)store->par_s3p, error);
			}
		}

		/* set position sensor 1, add it if necessary
		   - note that sometimes the start datagrams may have the active position
		     sensor set == 0 (which means position system 1 is active) while
		     having the position system 1 quality flag set to off  - in this case
		     force the position system 1 quality flag to on so that the sensor
		     structure is created */
		if (store->par_aps == 0 && store->par_p1q == 0)
			store->par_p1q = 1;
		if (platform->source_position1 < 0 && store->par_p1q) {
			/* set sensor 1 (position) */
			// TODO(schwehr): Bug?
			// capability1 = MB_SENSOR_CAPABILITY1_POSITION + MB_SENSOR_CAPABILITY1_HEADING;
			// capability2 = MB_SENSOR_CAPABILITY2_NONE;
			capability1 = 0;
			capability2 = 0;
			const int num_offsets = 1;
			const int num_time_latency = 0;
			status = mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_POSITION, NULL, NULL, NULL, capability1,
			                                capability2, num_offsets, num_time_latency, error);
			if (status == MB_SUCCESS) {
				platform->source_position1 = platform->num_sensors - 1;
			}
		}

		/* set offsets for position sensor 1 */
		if (store->par_p1q && platform->source_position1 >= 0 && platform->sensors[platform->source_position1].num_offsets == 1) {
			/* set offsets based on whether position data are already motion compensated */
			int position_offset_mode;
			double position_offset_x;
			double position_offset_y;
			double position_offset_z;
			int attitude_offset_mode;
			double attitude_offset_heading;
			double attitude_offset_roll;
			double attitude_offset_pitch;
			if (store->par_p1m) {
				position_offset_mode = MB_SENSOR_POSITION_OFFSET_STATIC;
				position_offset_x = 0.0;
				position_offset_y = 0.0;
				position_offset_z = 0.0;
				attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_NONE;
				attitude_offset_heading = 0.0;
				attitude_offset_roll = 0.0;
				attitude_offset_pitch = 0.0;
			}
			else {
				position_offset_mode = MB_SENSOR_POSITION_OFFSET_STATIC;
				position_offset_x = store->par_p1y;
				position_offset_y = store->par_p1x;
				position_offset_z = -store->par_p1z;
				attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_NONE;
				attitude_offset_heading = 0.0;
				attitude_offset_roll = 0.0;
				attitude_offset_pitch = 0.0;
			}

			/* if position sensor 1 is also the heading sensor, add heading offset */
			if ((store->par_ahs == 1 || store->par_gcg != 0.0) && !store->par_p1m) {
				attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_STATIC;
				attitude_offset_heading = store->par_gcg;
			}

			/* now set the offsets for position sensor 1 */
			status = mb_platform_set_sensor_offset(verbose, (void *)platform, platform->source_position1, 0, position_offset_mode,
			                                       position_offset_x, position_offset_y, position_offset_z, attitude_offset_mode,
			                                       attitude_offset_heading, attitude_offset_roll, attitude_offset_pitch, error);

			/* set time latency for position sensor 1 */
			if (status == MB_SUCCESS && store->par_p1d != 0.0) {
				status = mb_platform_set_sensor_timelatency(verbose, (void *)platform, platform->source_position1,
				                                            MB_SENSOR_TIME_LATENCY_STATIC, (double)store->par_p1d, 0, NULL, NULL,
				                                            error);
			}
		}

		/* set position sensor 2, add it if necessary
		   - note that sometimes the start datagrams may have the active position
		     sensor set == 1 (which means position system 2 is active) while
		     having the position system 2 quality flag set to off  - in this case
		     force the position system 2 quality flag to on so that the sensor
		     structure is created */
		if (store->par_aps == 1 && store->par_p2q == 0)
			store->par_p2q = 1;
		if (platform->source_position2 < 0 && store->par_p2q) {
			/* set sensor 2 (position) */
			// TODO(schwehr): Bug?
			// capability1 = MB_SENSOR_CAPABILITY1_POSITION + MB_SENSOR_CAPABILITY1_HEADING;
			// capability2 = MB_SENSOR_CAPABILITY2_NONE;
			capability1 = 0;
			capability2 = 0;
			const int num_offsets = 1;
			const int num_time_latency = 0;
			status = mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_POSITION, NULL, NULL, NULL, capability1,
			                                capability2, num_offsets, num_time_latency, error);
			if (status == MB_SUCCESS) {
				platform->source_position2 = platform->num_sensors - 1;
			}
		}

		/* set offsets for position sensor 2 */
		if (store->par_p2q && platform->source_position2 >= 0 && platform->sensors[platform->source_position2].num_offsets == 1) {
			/* set offsets based on whether position data are already motion compensated */
			int position_offset_mode;
			double position_offset_x;
			double position_offset_y;
			double position_offset_z;
			int attitude_offset_mode;
			double attitude_offset_heading;
			double attitude_offset_roll;
			double attitude_offset_pitch;
			if (store->par_p2m) {
				position_offset_mode = MB_SENSOR_POSITION_OFFSET_STATIC;
				position_offset_x = 0.0;
				position_offset_y = 0.0;
				position_offset_z = 0.0;
				attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_NONE;
				attitude_offset_heading = 0.0;
				attitude_offset_roll = 0.0;
				attitude_offset_pitch = 0.0;
			}
			else {
				position_offset_mode = MB_SENSOR_POSITION_OFFSET_STATIC;
				position_offset_x = store->par_p2y;
				position_offset_y = store->par_p2x;
				position_offset_z = -store->par_p2z;
				attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_NONE;
				attitude_offset_heading = 0.0;
				attitude_offset_roll = 0.0;
				attitude_offset_pitch = 0.0;
			}

			/* if position sensor 2 is also the heading sensor, add heading offset */
			if (store->par_ahs == 2 && !store->par_p2m) {
				attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_STATIC;
				attitude_offset_heading = store->par_gcg;
			}

			/* now set the offsets for position sensor 2 */
			status = mb_platform_set_sensor_offset(verbose, (void *)platform, platform->source_position2, 0, position_offset_mode,
			                                       position_offset_x, position_offset_y, position_offset_z, attitude_offset_mode,
			                                       attitude_offset_heading, attitude_offset_roll, attitude_offset_pitch, error);

			/* set time latency for position sensor 2 */
			if (status == MB_SUCCESS && store->par_p2d != 0.0) {
				status = mb_platform_set_sensor_timelatency(verbose, (void *)platform, platform->source_position2,
				                                            MB_SENSOR_TIME_LATENCY_STATIC, (double)store->par_p2d, 0, NULL, NULL,
				                                            error);
			}
		}

		/* set position sensor 3, add it if necessary
		   - note that sometimes the start datagrams may have the active position
		     sensor set == 2 (which means position system 3 is active) while
		     having the position system 3 quality flag set to off  - in this case
		     force the position system 3 quality flag to on so that the sensor
		     structure is created */
		if (store->par_aps == 2 && store->par_p3q == 0)
			store->par_p3q = 1;
		if (platform->source_position3 < 0 && store->par_p3q) {
			/* set sensor 3 (position) */
			// TODO(schwehr): Bug?
			// capability1 = MB_SENSOR_CAPABILITY1_POSITION + MB_SENSOR_CAPABILITY1_HEADING;
			// capability2 = MB_SENSOR_CAPABILITY2_NONE;
			capability1 = 0;
			capability2 = 0;
			const int num_offsets = 1;
			const int num_time_latency = 0;
			status = mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_POSITION, NULL, NULL, NULL, capability1,
			                                capability2, num_offsets, num_time_latency, error);
			if (status == MB_SUCCESS) {
				platform->source_position3 = platform->num_sensors - 1;
			}
		}

		/* set offsets for position sensor 3 */
		if (store->par_p3q && platform->source_position3 >= 0 && platform->sensors[platform->source_position3].num_offsets == 1) {
			/* set offsets based on whether position data are already motion compensated */
			int position_offset_mode;
			double position_offset_x;
			double position_offset_y;
			double position_offset_z;
			int attitude_offset_mode;
			double attitude_offset_heading;
			double attitude_offset_roll;
			double attitude_offset_pitch;
			if (store->par_p3m) {
				position_offset_mode = MB_SENSOR_POSITION_OFFSET_STATIC;
				position_offset_x = 0.0;
				position_offset_y = 0.0;
				position_offset_z = 0.0;
				attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_NONE;
				attitude_offset_heading = 0.0;
				attitude_offset_roll = 0.0;
				attitude_offset_pitch = 0.0;
			}
			else {
				position_offset_mode = MB_SENSOR_POSITION_OFFSET_STATIC;
				position_offset_x = store->par_p3y;
				position_offset_y = store->par_p3x;
				position_offset_z = -store->par_p3z;
				attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_NONE;
				attitude_offset_heading = 0.0;
				attitude_offset_roll = 0.0;
				attitude_offset_pitch = 0.0;
			}

			/* if position sensor 3 is also the heading sensor, add heading offset */
			if (store->par_ahs == 3 && !store->par_p3m) {
				attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_STATIC;
				attitude_offset_heading = store->par_gcg;
			}

			/* now set the offsets for position sensor 3 */
			status = mb_platform_set_sensor_offset(verbose, (void *)platform, platform->source_position3, 0, position_offset_mode,
			                                       position_offset_x, position_offset_y, position_offset_z, attitude_offset_mode,
			                                       attitude_offset_heading, attitude_offset_roll, attitude_offset_pitch, error);

			/* set time latency for position sensor 3 */
			if (status == MB_SUCCESS && store->par_p3d != 0.0) {
				status = mb_platform_set_sensor_timelatency(verbose, (void *)platform, platform->source_position3,
				                                            MB_SENSOR_TIME_LATENCY_STATIC, (double)store->par_p3d, 0, NULL, NULL,
				                                            error);
			}
		}

		/* add depth sensor if needed */
		if (platform->source_depth1 < 0 && store->par_dsh[0] == 'I' && store->par_dsh[1] == 'N') {
			// TODO(schwehr): Bug?
			// capability1 = MB_SENSOR_CAPABILITY1_DEPTH;
			// capability2 = MB_SENSOR_CAPABILITY2_NONE;
			capability1 = 0;
			capability2 = 0;
			const int num_offsets = 1;
			const int num_time_latency = 0;
			status = mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_PRESSURE, NULL, NULL, NULL, capability1,
			                                capability2, num_offsets, num_time_latency, error);
			if (status == MB_SUCCESS) {
				platform->source_depth1 = platform->num_sensors - 1;
			}
		}
		if (platform->source_depth1 >= 0 && platform->sensors[platform->source_depth1].num_offsets == 1) {
			const int position_offset_mode = MB_SENSOR_POSITION_OFFSET_STATIC;
			const double position_offset_x = store->par_dsy;
			const double position_offset_y = store->par_dsx;
			const double position_offset_z = -store->par_dsz;
			const int attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_NONE;
			const double attitude_offset_heading = 0.0;
			const double attitude_offset_roll = 0.0;
			const double attitude_offset_pitch = 0.0;
			status = mb_platform_set_sensor_offset(verbose, (void *)platform, platform->source_depth1, 0, position_offset_mode,
			                                       position_offset_x, position_offset_y, position_offset_z, attitude_offset_mode,
			                                       attitude_offset_heading, attitude_offset_roll, attitude_offset_pitch, error);
			if (status == MB_SUCCESS && store->par_dsd != 0.0) {
				status = mb_platform_set_sensor_timelatency(verbose, (void *)platform, platform->source_depth1,
				                                            MB_SENSOR_TIME_LATENCY_STATIC, (double)store->par_dsd, 0, NULL, NULL,
				                                            error);
			}
		}

		/* set motion sensor 1, add it if necessary */
		if (platform->source_rollpitch1 < 0) {
			/* set sensor 1 (position) */
			// TODO(schwehr): Bug?
			// capability1 = MB_SENSOR_CAPABILITY1_ROLLPITCH + MB_SENSOR_CAPABILITY1_HEADING + MB_SENSOR_CAPABILITY1_HEAVE;
			// capability2 = MB_SENSOR_CAPABILITY2_NONE;
			capability1 = 0;
			capability2 = 0;
			const int num_offsets = 1;
			const int num_time_latency = 0;
			status = mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_VRU, NULL, NULL, NULL, capability1,
			                                capability2, num_offsets, num_time_latency, error);
			if (status == MB_SUCCESS) {
				platform->source_rollpitch1 = platform->num_sensors - 1;
			}
		}

		/* set motion sensor 1 offsets */
		if (platform->source_rollpitch1 >= 0 && platform->sensors[platform->source_rollpitch1].num_offsets == 1) {
			/* set offsets */
			const int position_offset_mode = MB_SENSOR_POSITION_OFFSET_STATIC;
			const double position_offset_x = store->par_msy;
			const double position_offset_y = store->par_msx;
			const double position_offset_z = -store->par_msz;
			int attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_STATIC;
			const double attitude_offset_heading = store->par_msg;
			const double attitude_offset_roll = store->par_msr;
			const double attitude_offset_pitch = store->par_msp;
			status =
			    mb_platform_set_sensor_offset(verbose, (void *)platform, platform->source_rollpitch1, 0, position_offset_mode,
			                                  position_offset_x, position_offset_y, position_offset_z, attitude_offset_mode,
			                                  attitude_offset_heading, attitude_offset_roll, attitude_offset_pitch, error);

			/* set time latency */
			if (status == MB_SUCCESS && store->par_msd != 0.0) {
				status = mb_platform_set_sensor_timelatency(verbose, (void *)platform, platform->source_rollpitch1,
				                                            MB_SENSOR_TIME_LATENCY_STATIC, (double)store->par_msd, 0, NULL, NULL,
				                                            error);
			}
		}

		/* set motion sensor 2, add it if necessary */
		if (platform->source_rollpitch2 < 0) {
			// TODO(schwehr): Bug?
			// capability1 = MB_SENSOR_CAPABILITY1_ROLLPITCH + MB_SENSOR_CAPABILITY1_HEADING + MB_SENSOR_CAPABILITY1_HEAVE;
			// capability2 = MB_SENSOR_CAPABILITY2_NONE;
			capability1 = 0;
			capability2 = 0;
			const int num_offsets = 1;
			const int num_time_latency = 0;
			status = mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_VRU, NULL, NULL, NULL, capability1,
			                                capability2, num_offsets, num_time_latency, error);
			if (status == MB_SUCCESS) {
				platform->source_rollpitch2 = platform->num_sensors - 1;
			}
		}

		/* set motion sensor 2 offsets */
		if (platform->source_rollpitch2 >= 0 && platform->sensors[platform->source_rollpitch2].num_offsets == 1) {
			/* set offsets - same as motion sensor 1 if not specified */
			int position_offset_mode;
			double position_offset_x;
			double position_offset_y;
			double position_offset_z;
			int attitude_offset_mode;
			double attitude_offset_heading;
			double attitude_offset_roll;
			double attitude_offset_pitch;
			if (store->par_nsx != 0.0 || store->par_nsy != 0.0 || store->par_nsz != 0.0 || store->par_nsg != 0.0 ||
			    store->par_nsr != 0.0 || store->par_nsp != 0.0) {
				position_offset_mode = MB_SENSOR_POSITION_OFFSET_STATIC;
				position_offset_x = store->par_nsy;
				position_offset_y = store->par_nsx;
				position_offset_z = -store->par_nsz;
				attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_STATIC;
				attitude_offset_heading = store->par_nsg;
				attitude_offset_roll = store->par_nsr;
				attitude_offset_pitch = store->par_nsp;
			}
			else {
				position_offset_mode = MB_SENSOR_POSITION_OFFSET_STATIC;
				position_offset_x = store->par_msy;
				position_offset_y = store->par_msx;
				position_offset_z = -store->par_msz;
				attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_STATIC;
				attitude_offset_heading = store->par_msg;
				attitude_offset_roll = store->par_msr;
				attitude_offset_pitch = store->par_msp;
			}
			status =
			    mb_platform_set_sensor_offset(verbose, (void *)platform, platform->source_rollpitch2, 0, position_offset_mode,
			                                  position_offset_x, position_offset_y, position_offset_z, attitude_offset_mode,
			                                  attitude_offset_heading, attitude_offset_roll, attitude_offset_pitch, error);

			/* set time latency */
			if (status == MB_SUCCESS && store->par_nsd != 0.0) {
				status = mb_platform_set_sensor_timelatency(verbose, (void *)platform, platform->source_rollpitch2,
				                                            MB_SENSOR_TIME_LATENCY_STATIC, (double)store->par_nsd, 0, NULL, NULL,
				                                            error);
			}
		}

		/* now set primary data sources */
		if (store->par_aps == 0)
			platform->source_position = platform->source_position1;
		else if (store->par_aps == 1)
			platform->source_position = platform->source_position2;
		else if (store->par_aps == 2)
			platform->source_position = platform->source_position3;
		else
			platform->source_position = platform->source_position1;
		if (store->par_aro == 2)
			platform->source_rollpitch = platform->source_rollpitch1;
		else if (store->par_aro == 3)
			platform->source_rollpitch = platform->source_rollpitch2;
		else if (store->par_aro == 8)
			platform->source_rollpitch = platform->source_rollpitch1;
		else if (store->par_aro == 9)
			platform->source_rollpitch = platform->source_rollpitch2;
		else
			platform->source_rollpitch = platform->source_rollpitch1;
		if (store->par_dsh[0] == 'I' && store->par_dsh[1] == 'N')
			platform->source_depth = platform->source_depth1;
		if (store->par_ahe == 2 || store->par_ahe == 8)
			platform->source_heave = platform->source_rollpitch1;
		else if (store->par_ahe == 3 || store->par_ahe == 9)
			platform->source_heave = platform->source_rollpitch2;
		if (store->par_ahs == 0)
			platform->source_heading = platform->source_position3;
		else if (store->par_ahs == 1)
			platform->source_heading = platform->source_position1;
		else if (store->par_ahs == 2)
			platform->source_heading = platform->source_rollpitch1;
		else if (store->par_ahs == 3 && platform->source_rollpitch2 >= 0)
			platform->source_heading = platform->source_rollpitch2;
		else if (store->par_ahs == 3)
			platform->source_heading = platform->source_position2;
		else if (store->par_ahs == 4)
			platform->source_heading = platform->source_position3;
		else if (store->par_ahs == 5)
			platform->source_heading = platform->source_position1;
		else if (store->par_ahs == 6)
			platform->source_heading = platform->source_position1;
		else if (store->par_ahs == 7)
			platform->source_heading = platform->source_position1;
		else if (store->par_ahs == 8)
			platform->source_heading = platform->source_rollpitch1;
		else if (store->par_ahs == 9)
			platform->source_heading = platform->source_rollpitch2;

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
		fprintf(stderr, "dbg2       error:		   %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:		   %d\n", status);
	// }
	// if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:          %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:         %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad3_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                          double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag,
                          double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                          double *ssacrosstrack, double *ssalongtrack, char *comment, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get survey data structure */
		struct mbsys_simrad3_ping_struct *ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);

		/* get time */
		time_i[0] = ping->png_date / 10000;
		time_i[1] = (ping->png_date % 10000) / 100;
		time_i[2] = ping->png_date % 100;
		time_i[3] = ping->png_msec / 3600000;
		time_i[4] = (ping->png_msec % 3600000) / 60000;
		time_i[5] = (ping->png_msec % 60000) / 1000;
		time_i[6] = (ping->png_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, time_d);

		/* get navigation */
		if (ping->png_longitude != EM3_INVALID_INT)
			*navlon = 0.0000001 * ping->png_longitude;
		else
			*navlon = 0.0;
		if (ping->png_latitude != EM3_INVALID_INT)
			*navlat = 0.00000005 * ping->png_latitude;
		else
			*navlat = 0.0;

		/* get heading */
		*heading = 0.01 * ping->png_heading;

		/* get speed  */
		if (ping->png_speed != EM3_INVALID_SHORT)
			*speed = 0.036 * ping->png_speed;
		else
			*speed = 0.0;

		/* set beamwidths in mb_io structure */
		if (store->run_rec_beam > 0)
			mb_io_ptr->beamwidth_xtrack = 0.1 * store->run_rec_beam;
		if (ping->png_tx > 0) {
			mb_io_ptr->beamwidth_ltrack = 0.1 * ping->png_tx;
		}
		else if (store->run_tran_beam > 0) {
			mb_io_ptr->beamwidth_ltrack = 0.1 * store->run_tran_beam;
		}

		/* read distance and depth values into storage arrays */
		const double reflscale = 0.1;
		*nbath = 0;
		for (int i = 0; i < ping->png_nbeams; i++) {
			bath[i] = ping->png_depth[i] + ping->png_xducer_depth;
			beamflag[i] = ping->png_beamflag[i];
			bathacrosstrack[i] = ping->png_acrosstrack[i];
			bathalongtrack[i] = ping->png_alongtrack[i];
			amp[i] = reflscale * ping->png_amp[i];
		}
		*nbath = ping->png_nbeams;
		*namp = *nbath;
		*nss = MBSYS_SIMRAD3_MAXPIXELS;
		const double pixel_size = ping->png_pixel_size;
		for (int i = 0; i < MBSYS_SIMRAD3_MAXPIXELS; i++) {
			if (ping->png_ss[i] == EM3_INVALID_SS || (ping->png_ss[i] == EM3_INVALID_AMP && ping->png_ssalongtrack[i] == 0)) {
				ss[i] = MB_SIDESCAN_NULL;
				ssacrosstrack[i] = pixel_size * (i - MBSYS_SIMRAD3_MAXPIXELS / 2);
				ssalongtrack[i] = 0.0;
			}
			else {
				ss[i] = 0.01 * ping->png_ss[i];
				ssacrosstrack[i] = pixel_size * (i - MBSYS_SIMRAD3_MAXPIXELS / 2);
				ssalongtrack[i] = 0.01 * ping->png_ssalongtrack[i];
			}
		}

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
			fprintf(stderr, "dbg4       nbath:      %d\n", *nbath);
			for (int i = 0; i < *nbath; i++)
				fprintf(stderr, "dbg4       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
				        bathacrosstrack[i], bathalongtrack[i]);
			fprintf(stderr, "dbg4        namp:     %d\n", *namp);
			for (int i = 0; i < *namp; i++)
				fprintf(stderr, "dbg4        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
				        bathalongtrack[i]);
			fprintf(stderr, "dbg4        nss:      %d\n", *nss);
			for (int i = 0; i < *nss; i++)
				fprintf(stderr, "dbg4        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
				        ssalongtrack[i]);
		}

		/* done translating values */
	}

	/* extract data from structure */
	else if (*kind == MB_DATA_NAV || *kind == MB_DATA_NAV1 || *kind == MB_DATA_NAV2 || *kind == MB_DATA_NAV3) {
		/* get time */
		time_i[0] = store->pos_date / 10000;
		time_i[1] = (store->pos_date % 10000) / 100;
		time_i[2] = store->pos_date % 100;
		time_i[3] = store->pos_msec / 3600000;
		time_i[4] = (store->pos_msec % 3600000) / 60000;
		time_i[5] = (store->pos_msec % 60000) / 1000;
		time_i[6] = (store->pos_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, time_d);

		/* get navigation */
		if (store->pos_longitude != EM3_INVALID_INT)
			*navlon = 0.0000001 * store->pos_longitude;
		else
			*navlon = 0.0;
		if (store->pos_latitude != EM3_INVALID_INT)
			*navlat = 0.00000005 * store->pos_latitude;
		else
			*navlat = 0.0;

		/* get heading */
		*heading = 0.01 * store->pos_heading;

		/* get speed  */
		if (store->pos_speed != EM3_INVALID_SHORT)
			*speed = 0.036 * store->pos_speed;
		else
			*speed = 0.0;

		*nbath = 0;
		*namp = 0;
		*nss = 0;

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
		}

		/* done translating values */
	}

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT) {
		/* copy comment */
    memset((void *)comment, 0, MB_COMMENT_MAXLINE);
    strncpy(comment, store->par_com, MIN(MB_COMMENT_MAXLINE, MBSYS_SIMRAD3_COMMENT_LENGTH) - 1);

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
		fprintf(stderr, "dbg2        namp:     %d\n", *namp);
		for (int i = 0; i < *namp; i++)
			fprintf(stderr, "dbg2       beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
			        bathalongtrack[i]);
		fprintf(stderr, "dbg2        nss:      %d\n", *nss);
		for (int i = 0; i < *nss; i++)
			fprintf(stderr, "dbg2        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
			        ssalongtrack[i]);
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
int mbsys_simrad3_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                         double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                         double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                         double *ssalongtrack, char *comment, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
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
				fprintf(stderr, "dbg3        beam:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
				        ssalongtrack[i]);
	}
	if (verbose >= 2 && kind == MB_DATA_COMMENT) {
		fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	/* set data kind */
	store->kind = kind;

	int status = MB_SUCCESS;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA) {
		struct mbsys_simrad3_ping_struct *ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);

		/* get time */
		ping->png_date = 10000 * time_i[0] + 100 * time_i[1] + time_i[2];
		ping->png_msec = 3600000 * time_i[3] + 60000 * time_i[4] + 1000 * time_i[5] + 0.001 * time_i[6];
		store->date = ping->png_date;
		store->msec = ping->png_msec;

		/* get navigation */
		if (navlon < -180.0)
			navlon += 360.0;
		else if (navlon > 180.0)
			navlon -= 360.0;
		ping->png_longitude = 10000000 * navlon;
		ping->png_latitude = 20000000 * navlat;

		/* get heading */
		if (heading < 0.0)
			heading += 360.0;
		else if (heading >= 360.0)
			heading -= 360.0;
		ping->png_heading = (int)rint(heading * 100);

		/* get speed  */
		ping->png_speed = (int)rint(speed / 0.036);

		/* insert bathymetry and amplitude */
		double reflscale = 0.1;
		if (status == MB_SUCCESS && ping->png_nbeams == 0) {
			ping->png_nbeams_valid = 0;
			for (int i = 0; i < nbath; i++) {
				if (beamflag[i] != MB_FLAG_NULL) {
					ping->png_depth[i] = bath[i] - ping->png_xducer_depth;
					ping->png_beamflag[i] = beamflag[i];
					ping->png_acrosstrack[i] = bathacrosstrack[i];
					ping->png_alongtrack[i] = bathalongtrack[i];
					ping->png_amp[i] = (int)rint(amp[i] / reflscale);
					ping->png_nbeams++;
					ping->png_nbeams_valid++;
				}
				else {
					ping->png_depth[i] = 0.0;
					ping->png_beamflag[i] = MB_FLAG_NULL;
					ping->png_acrosstrack[i] = 0.0;
					ping->png_alongtrack[i] = 0.0;
					ping->png_amp[i] = 0;
					ping->png_nbeams++;
				}
			}
			ping->png_nbeams = nbath;
		}
		else if (status == MB_SUCCESS) {
			for (int i = 0; i < ping->png_nbeams; i++) {
				ping->png_depth[i] = bath[i] - ping->png_xducer_depth;
				ping->png_beamflag[i] = beamflag[i];
				ping->png_acrosstrack[i] = bathacrosstrack[i];
				ping->png_alongtrack[i] = bathalongtrack[i];
				ping->png_amp[i] = (int)rint(amp[i] / reflscale);
			}
		}
		if (status == MB_SUCCESS) {
			for (int i = 0; i < nss; i++) {
				if (ss[i] > MB_SIDESCAN_NULL) {
					ping->png_ss[i] = (short)rint(100 * ss[i]);
					ping->png_ssalongtrack[i] = (short)rint(100 * ssalongtrack[i]);
				}
				else {
					ping->png_ss[i] = EM3_INVALID_SS;
					ping->png_ssalongtrack[i] = 0;
				}
			}
		}
	}

	/* insert data in nav structure */
	else if (store->kind == MB_DATA_NAV || store->kind == MB_DATA_NAV1 || store->kind == MB_DATA_NAV2 ||
	         store->kind == MB_DATA_NAV3) {

		/* get time */
		store->pos_date = 10000 * time_i[0] + 100 * time_i[1] + time_i[2];
		store->pos_msec = 3600000 * time_i[3] + 60000 * time_i[4] + 1000 * time_i[5] + 0.001 * time_i[6];
		store->msec = store->pos_msec;
		store->date = store->pos_date;

		/* get navigation */
		if (navlon < -180.0)
			navlon += 360.0;
		else if (navlon > 180.0)
			navlon -= 360.0;
		store->pos_longitude = 10000000 * navlon;
		store->pos_latitude = 20000000 * navlat;

		/* get heading */
		store->pos_heading = (int)rint(heading * 100);

		/* get speed  */
		store->pos_speed = (int)rint(speed / 0.036);

		/* get roll pitch and heave */

		/* set "active" flag if needed */
		if (store->kind == MB_DATA_NAV) {
			store->pos_system = store->pos_system | 128;
		}

		/* set secondary nav flag if needed */
		else if (store->kind == MB_DATA_NAV1) {
			store->pos_system = store->pos_system | 1;
		}
		else if (store->kind == MB_DATA_NAV2) {
			store->pos_system = store->pos_system | 2;
		}
		else if (store->kind == MB_DATA_NAV3) {
			store->pos_system = store->pos_system | 3;
		}
	}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT) {
    memset((void *)store->par_com, 0, MBSYS_SIMRAD3_COMMENT_LENGTH);
    strncpy(store->par_com, comment, MIN(MBSYS_SIMRAD3_COMMENT_LENGTH, MB_COMMENT_MAXLINE) - 1);
	}

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
int mbsys_simrad3_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
                         double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft,
                         double *ssv, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
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
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get survey data structure */
		struct mbsys_simrad3_ping_struct *ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);

		/* get ping time */
		int time_i[7];
		time_i[0] = ping->png_date / 10000;
		time_i[1] = (ping->png_date % 10000) / 100;
		time_i[2] = ping->png_date % 100;
		time_i[3] = ping->png_msec / 3600000;
		time_i[4] = (ping->png_msec % 3600000) / 60000;
		time_i[5] = (ping->png_msec % 60000) / 1000;
		time_i[6] = (ping->png_msec % 1000) * 1000;
		double ptime_d;
		mb_get_time(verbose, time_i, &ptime_d);

		/* obtain lever arm offset for sonar relative to the position sensor */
		// mb_lever(verbose, store->par_s1y, store->par_s1x, store->par_s1z - store->par_wlz,
		//		store->par_p1y, store->par_p1x, store->par_p1z,
		//		store->par_msy, store->par_msx, store->par_msz,
		//		-0.01 * ping->png_pitch + store->par_msp, -0.01 * ping->png_roll + store->par_msr,
		//		&lever_x, &lever_y, &lever_z, error);

		/* obtain position offset for beam */
		// offset_x = store->par_s1y - store->par_p1y + lever_x;
		// offset_y = store->par_s1x - store->par_p1x + lever_y;
		// offset_z = lever_z;

		/* get alongtrack position offset */
		double offset_y;
		if (store->par_aps == 0) {
			offset_y = store->par_p1x;
		}
		else if (store->par_aps == 1) {
			offset_y = store->par_p2x;
		}
		else if (store->par_aps == 2) {
			offset_y = store->par_p3x;
		}
		else {
			offset_y = store->par_p1x;
		}

		/* special case when par_aps value is wrong */
		if (offset_y == 0.0 && store->par_p1x != 0.0)
			offset_y = store->par_p1x;
		else if (offset_y == 0.0 && store->par_p2x != 0.0)
			offset_y = store->par_p2x;
		else if (offset_y == 0.0 && store->par_p3x != 0.0)
			offset_y = store->par_p3x;

		/* get sonar depth */
		*ssv = 0.1 * ping->png_ssv;
		*draft = ping->png_xducer_depth - 0.01 * ping->png_heave;

		/* get travel times, angles */
		*nbeams = ping->png_nbeams;
		// soundspeed = 0.1 * ((double)ping->png_ssv);
		for (int i = 0; i < ping->png_nbeams; i++) {
			ttimes[i] = ping->png_range[i];
			angles[i] = ping->png_depression[i];
			angles_forward[i] = 180.0 - ping->png_azimuth[i];
			if (angles_forward[i] < 0.0)
				angles_forward[i] += 360.0;
			angles_null[i] = 0.0;
			heave[i] = ping->png_bheave[i] + 0.01 * ping->png_heave;
			alongtrack_offset[i] =
			    (0.01 * ((double)ping->png_speed)) * ((double)ping->png_raw_txoffset[ping->png_raw_rxsector[i]]) + offset_y;
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
int mbsys_simrad3_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error) {
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
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get survey data structure */
		struct mbsys_simrad3_ping_struct *ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);

		*nbeams = ping->png_nbeams;
		for (int i = 0; i < ping->png_nbeams; i++) {
			if (ping->png_detection[i] & 128) {
				detects[i] = MB_DETECT_UNKNOWN;
			}
			else {
				if (ping->png_detection[i] & 1)
					detects[i] = MB_DETECT_PHASE;
				else
					detects[i] = MB_DETECT_AMPLITUDE;
			}
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
int mbsys_simrad3_pulses(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *pulses, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       pulses:     %p\n", (void *)pulses);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get survey data structure */
		struct mbsys_simrad3_ping_struct *ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);

		*nbeams = ping->png_nbeams;
		for (int j = 0; j < ping->png_nbeams; j++) {
			pulses[j] = MB_PULSE_UNKNOWN;
		}
		for (int i = 0; i < ping->png_nbeams; i++) {
			if (ping->png_raw_txwaveform[ping->png_raw_rxsector[i]] == 0)
				pulses[i] = MB_PULSE_CW;
			else if (ping->png_raw_txwaveform[ping->png_raw_rxsector[i]] == 1)
				pulses[i] = MB_PULSE_UPCHIRP;
			else if (ping->png_raw_txwaveform[ping->png_raw_rxsector[i]] == 2)
				pulses[i] = MB_PULSE_DOWNCHIRP;
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
			fprintf(stderr, "dbg2       beam %d: pulses:%d\n", i, pulses[i]);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad3_gains(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transmit_gain, double *pulse_length,
                        double *receive_gain, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get survey data structure */
		// struct mbsys_simrad3_ping_struct *ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);

		/* get transmit_gain (dB) */
		*transmit_gain = (double)store->run_tran_pow;

		/* get pulse_length (sec) */
		*pulse_length = 0.000001 * (double)store->run_tran_pulse;

		/* get receive_gain (dB) */
		*receive_gain = (double)store->run_rec_gain;

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
int mbsys_simrad3_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                   double *altitude, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get survey data structure */
		struct mbsys_simrad3_ping_struct *ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);

		/* get transducer depth and altitude */
		*transducer_depth = ping->png_xducer_depth;
		bool found = false;
		double altitude_best = 0.0;
		double xtrack_min = 99999999.9;
		for (int i = 0; i < ping->png_nbeams; i++) {
			if (mb_beam_ok(ping->png_beamflag[i]) && fabs(ping->png_acrosstrack[i]) < xtrack_min) {
				xtrack_min = fabs(ping->png_acrosstrack[i]);
				altitude_best = ping->png_depth[i];
				found = true;
			}
		}
		if (!found) {
			xtrack_min = 99999999.9;
			for (int i = 0; i < ping->png_nbeams; i++) {
				if (ping->png_quality[i] > 0 && fabs(ping->png_acrosstrack[i]) < xtrack_min) {
					xtrack_min = fabs(ping->png_acrosstrack[i]);
					altitude_best = ping->png_depth[i];
					found = true;
				}
			}
		}
		if (found)
			*altitude = altitude_best;
		else
			*altitude = 0.0;

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
		fprintf(stderr, "dbg2       kind:              %d\n", *kind);
		fprintf(stderr, "dbg2       transducer_depth:  %f\n", *transducer_depth);
		fprintf(stderr, "dbg2       altitude:          %f\n", *altitude);
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:            %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad3_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr, int nmax, int *kind, int *n, int *time_i,
                               double *time_d, double *navlon, double *navlat, double *speed, double *heading, double *draft,
                               double *roll, double *pitch, double *heave, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       nmax:       %d\n", nmax);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from ping structure */
	if (*kind == MB_DATA_DATA) {
		/* get survey data structure */
		struct mbsys_simrad3_ping_struct *ping =
			(struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);

		/* just one navigation value */
		*n = 1;

		/* get time */
		time_i[0] = ping->png_date / 10000;
		time_i[1] = (ping->png_date % 10000) / 100;
		time_i[2] = ping->png_date % 100;
		time_i[3] = ping->png_msec / 3600000;
		time_i[4] = (ping->png_msec % 3600000) / 60000;
		time_i[5] = (ping->png_msec % 60000) / 1000;
		time_i[6] = (ping->png_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, time_d);

		/* get navigation */
		if (ping->png_longitude != EM3_INVALID_INT)
			*navlon = 0.0000001 * ping->png_longitude;
		else
			*navlon = 0.0;
		if (ping->png_latitude != EM3_INVALID_INT)
			*navlat = 0.00000005 * ping->png_latitude;
		else
			*navlat = 0.0;

		/* get heading */
		*heading = 0.01 * ping->png_heading;

		/* get speed  */
		if (ping->png_speed != EM3_INVALID_SHORT)
			*speed = 0.036 * ping->png_speed;
		else
			*speed = 0.0;

		/* get draft  */
		*draft = ping->png_xducer_depth - 0.01 * ping->png_heave;

		/* get roll pitch and heave */
		*roll = 0.01 * ping->png_roll;
		*pitch = 0.01 * ping->png_pitch;
		*heave = 0.01 * ping->png_heave;

		/* done translating values */
	}

	/* extract data from nav structure */
	else if (*kind == MB_DATA_NAV || *kind == MB_DATA_NAV1 || *kind == MB_DATA_NAV2 || *kind == MB_DATA_NAV3) {
		/* get survey data structure */
		struct mbsys_simrad3_ping_struct *ping =
			(struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);

		/* just one navigation value */
		*n = 1;

		/* get time */
		time_i[0] = store->pos_date / 10000;
		time_i[1] = (store->pos_date % 10000) / 100;
		time_i[2] = store->pos_date % 100;
		time_i[3] = store->pos_msec / 3600000;
		time_i[4] = (store->pos_msec % 3600000) / 60000;
		time_i[5] = (store->pos_msec % 60000) / 1000;
		time_i[6] = (store->pos_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, time_d);

		/* get navigation */
		if (store->pos_longitude != EM3_INVALID_INT)
			*navlon = 0.0000001 * store->pos_longitude;
		else
			*navlon = 0.0;
		if (store->pos_latitude != EM3_INVALID_INT)
			*navlat = 0.00000005 * store->pos_latitude;
		else
			*navlat = 0.0;

		/* get heading */
		if (store->pos_heading != EM3_INVALID_SHORT)
			*heading = 0.01 * store->pos_heading;
		else
			*heading = 0.0;

		/* get speed  */
		if (store->pos_speed != EM3_INVALID_SHORT)
			*speed = 0.036 * store->pos_speed;
		else
			*speed = 0.0;

		/* get draft  */
		if (ping != NULL)
			*draft = ping->png_xducer_depth - 0.01 * store->pos_heave;
		else
			*draft = 0.0;

		/* get roll pitch and heave */
		*roll = 0.01 * store->pos_roll;
		*pitch = 0.01 * store->pos_pitch;
		*heave = 0.01 * store->pos_heave;

		/* done translating values */
	}

	/* extract data from attitude structure */
	else if (store->type == EM3_ATTITUDE && store->attitude != NULL) {
		/* get attitude data structure */
		struct mbsys_simrad3_attitude_struct *attitude = (struct mbsys_simrad3_attitude_struct *)store->attitude;

		/* get n */
		*n = MIN(attitude->att_ndata, MB_ASYNCH_SAVE_MAX);

		/* get attitude time */
		int atime_i[7];
		atime_i[0] = attitude->att_date / 10000;
		atime_i[1] = (attitude->att_date % 10000) / 100;
		atime_i[2] = attitude->att_date % 100;
		atime_i[3] = attitude->att_msec / 3600000;
		atime_i[4] = (attitude->att_msec % 3600000) / 60000;
		atime_i[5] = (attitude->att_msec % 60000) / 1000;
		atime_i[6] = (attitude->att_msec % 1000) * 1000;
		double atime_d;
		mb_get_time(verbose, atime_i, &atime_d);

		int interp_error = MB_ERROR_NO_ERROR;
		/* loop over the data */
		for (int i = 0; i < *n; i++) {
			/* get time from the data record */
			time_d[i] = (double)(atime_d + 0.001 * attitude->att_time[i]);
			mb_get_date(verbose, time_d[i], &(time_i[7 * i]));

			/* get attitude from the data record */
			heave[i] = (double)(0.01 * attitude->att_heave[i]);
			roll[i] = (double)(0.01 * attitude->att_roll[i]);
			pitch[i] = (double)(0.01 * attitude->att_pitch[i]);

			/* interpolate the heading */
			mb_hedint_interp(verbose, mbio_ptr, time_d[i], &heading[i], &interp_error);

			/* interpolate the navigation */
			mb_navint_interp(verbose, mbio_ptr, time_d[i], heading[i], 0.0, &navlon[i], &navlat[i], &speed[i], &interp_error);

			/* interpolate the sonar depth */
			mb_depint_interp(verbose, mbio_ptr, time_d[i], &draft[i], &interp_error);
		}

//		if (interp_error != MB_ERROR_NO_ERROR) {
//			fprintf(stderr, "%s:%4.4d:%s: WARNING: interp error is %d\n",
//              __FILE__, __LINE__, __func__, interp_error);
//		}
		/* done translating values */
	} else if (store->type == EM3_NETATTITUDE && store->netattitude != NULL) {
		/* extract data from netattitude structure */

		/* get attitude data structure */
		struct mbsys_simrad3_netattitude_struct *netattitude =
			(struct mbsys_simrad3_netattitude_struct *)store->netattitude;

		/* get n */
		*n = MIN(netattitude->nat_ndata, MB_ASYNCH_SAVE_MAX);

		/* get attitude time */
		int atime_i[7];
		atime_i[0] = netattitude->nat_date / 10000;
		atime_i[1] = (netattitude->nat_date % 10000) / 100;
		atime_i[2] = netattitude->nat_date % 100;
		atime_i[3] = netattitude->nat_msec / 3600000;
		atime_i[4] = (netattitude->nat_msec % 3600000) / 60000;
		atime_i[5] = (netattitude->nat_msec % 60000) / 1000;
		atime_i[6] = (netattitude->nat_msec % 1000) * 1000;
		double atime_d;
		mb_get_time(verbose, atime_i, &atime_d);

		/* loop over the data */
		int interp_error = MB_ERROR_NO_ERROR;
		for (int i = 0; i < *n; i++) {
			/* get time from the data record */
			time_d[i] = (double)(atime_d + 0.001 * netattitude->nat_time[i]);
			mb_get_date(verbose, time_d[i], &(time_i[7 * i]));

			/* get attitude from the data record */
			heave[i] = (double)(0.01 * netattitude->nat_heave[i]);
			roll[i] = (double)(0.01 * netattitude->nat_roll[i]);
			pitch[i] = (double)(0.01 * netattitude->nat_pitch[i]);

			/* interpolate the heading */
			mb_hedint_interp(verbose, mbio_ptr, time_d[i], &heading[i], &interp_error);

			/* interpolate the navigation */
			mb_navint_interp(verbose, mbio_ptr, time_d[i], heading[i], 0.0, &navlon[i], &navlat[i], &speed[i], &interp_error);

			/* interpolate the sonar depth */
			mb_depint_interp(verbose, mbio_ptr, time_d[i], &draft[i], &interp_error);
		}

//		if (interp_error != MB_ERROR_NO_ERROR) {
//			fprintf(stderr, "WARNING: interp_error is %d\n", interp_error);
//		}

		/* done translating values */
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*n = 0;
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* deal with other record type */
	else {
		/* set status */
		*n = 0;
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		fprintf(stderr, "dbg2       n:          %d\n", *n);
		for (int inav = 0; inav < *n; inav++) {
			for (int i = 0; i < 7; i++)
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
int mbsys_simrad3_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                              double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                              double *pitch, double *heave, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from ping structure */
	if (*kind == MB_DATA_DATA) {
		/* get survey data structure */
		struct mbsys_simrad3_ping_struct *ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);

		/* get time */
		time_i[0] = ping->png_date / 10000;
		time_i[1] = (ping->png_date % 10000) / 100;
		time_i[2] = ping->png_date % 100;
		time_i[3] = ping->png_msec / 3600000;
		time_i[4] = (ping->png_msec % 3600000) / 60000;
		time_i[5] = (ping->png_msec % 60000) / 1000;
		time_i[6] = (ping->png_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, time_d);

		/* get navigation */
		if (ping->png_longitude != EM3_INVALID_INT)
			*navlon = 0.0000001 * ping->png_longitude;
		else
			*navlon = 0.0;
		if (ping->png_latitude != EM3_INVALID_INT)
			*navlat = 0.00000005 * ping->png_latitude;
		else
			*navlat = 0.0;

		/* get heading */
		*heading = 0.01 * ping->png_heading;

		/* get speed  */
		if (ping->png_speed != EM3_INVALID_SHORT)
			*speed = 0.036 * ping->png_speed;
		else
			*speed = 0.0;

		/* get draft  */
		*draft = ping->png_xducer_depth - 0.01 * ping->png_heave;

		/* get roll pitch and heave */
		*roll = 0.01 * ping->png_roll;
		*pitch = 0.01 * ping->png_pitch;
		*heave = 0.01 * ping->png_heave;

		/* done translating values */
	}

	/* extract data from nav structure */
	else if (*kind == MB_DATA_NAV || *kind == MB_DATA_NAV1 || *kind == MB_DATA_NAV2 || *kind == MB_DATA_NAV3) {
		/* get survey data structure */
		/* get survey data structure */
		struct mbsys_simrad3_ping_struct *ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);

		/* get time */
		time_i[0] = store->pos_date / 10000;
		time_i[1] = (store->pos_date % 10000) / 100;
		time_i[2] = store->pos_date % 100;
		time_i[3] = store->pos_msec / 3600000;
		time_i[4] = (store->pos_msec % 3600000) / 60000;
		time_i[5] = (store->pos_msec % 60000) / 1000;
		time_i[6] = (store->pos_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, time_d);

		/* get navigation */
		if (store->pos_longitude != EM3_INVALID_INT)
			*navlon = 0.0000001 * store->pos_longitude;
		else
			*navlon = 0.0;
		if (store->pos_latitude != EM3_INVALID_INT)
			*navlat = 0.00000005 * store->pos_latitude;
		else
			*navlat = 0.0;

		/* get heading */
		if (store->pos_heading != EM3_INVALID_SHORT)
			*heading = 0.01 * store->pos_heading;
		else
			*heading = 0.0;

		/* get speed  */
		if (store->pos_speed != EM3_INVALID_SHORT)
			*speed = 0.036 * store->pos_speed;
		else
			*speed = 0.0;

		/* get draft  */
		if (ping != NULL)
			*draft = ping->png_xducer_depth - 0.01 * store->pos_heave;
		else
			*draft = 0.0;

		/* get roll pitch and heave */
		*roll = 0.01 * store->pos_roll;
		*pitch = 0.01 * store->pos_pitch;
		*heave = 0.01 * store->pos_heave;

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
int mbsys_simrad3_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
                             double navlat, double speed, double heading, double draft, double roll, double pitch, double heave,
                             int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
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
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	/* insert data in ping structure */
	if (store->kind == MB_DATA_DATA) {
		struct mbsys_simrad3_ping_struct *ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);

		/* get time */
		ping->png_date = 10000 * time_i[0] + 100 * time_i[1] + time_i[2];
		ping->png_msec = 3600000 * time_i[3] + 60000 * time_i[4] + 1000 * time_i[5] + 0.001 * time_i[6];
		store->msec = ping->png_msec;
		store->date = ping->png_date;

		/* get navigation */
		if (navlon < -180.0)
			navlon += 360.0;
		else if (navlon > 180.0)
			navlon -= 360.0;
		ping->png_longitude = 10000000 * navlon;
		ping->png_latitude = 20000000 * navlat;

		/* get heading */
		if (heading < 0.0)
			heading += 360.0;
		else if (heading >= 360.0)
			heading -= 360.0;
		ping->png_heading = (int)rint(heading * 100);

		/* get speed  */
		ping->png_speed = (int)rint(speed / 0.036);

		/* get draft  */
		ping->png_xducer_depth = draft + heave;

		/* get roll pitch and heave */
		ping->png_roll = (int)rint(roll / 0.01);
		ping->png_pitch = (int)rint(pitch / 0.01);
		ping->png_heave = (int)rint(heave / 0.01);
	}

	/* insert data in nav structure */
	else if (store->kind == MB_DATA_NAV || store->kind == MB_DATA_NAV1 || store->kind == MB_DATA_NAV2 ||
	         store->kind == MB_DATA_NAV3) {

		/* get time */
		store->pos_date = 10000 * time_i[0] + 100 * time_i[1] + time_i[2];
		store->pos_msec = 3600000 * time_i[3] + 60000 * time_i[4] + 1000 * time_i[5] + 0.001 * time_i[6];
		store->msec = store->pos_msec;
		store->date = store->pos_date;

		/* get navigation */
		if (navlon < -180.0)
			navlon += 360.0;
		else if (navlon > 180.0)
			navlon -= 360.0;
		store->pos_longitude = 10000000 * navlon;
		store->pos_latitude = 20000000 * navlat;

		/* get heading */
		store->pos_heading = (int)rint(heading * 100);

		/* get speed  */
		store->pos_speed = (int)rint(speed / 0.036);

		/* get roll pitch and heave */
		store->pos_roll = (int)rint(roll / 0.01);
		store->pos_pitch = (int)rint(pitch / 0.01);
		store->pos_heave = (int)rint(heave / 0.01);

		/* set "active" flag if needed */
		if (store->kind == MB_DATA_NAV) {
			store->pos_system = store->pos_system | 128;
		}

		/* set secondary nav flag if needed */
		else if (store->kind == MB_DATA_NAV1) {
			store->pos_system = store->pos_system | 1;
		}
		else if (store->kind == MB_DATA_NAV2) {
			store->pos_system = store->pos_system | 2;
		}
		else if (store->kind == MB_DATA_NAV3) {
			store->pos_system = store->pos_system | 3;
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
int mbsys_simrad3_extract_svp(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsvp, double *depth, double *velocity,
                              int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_VELOCITY_PROFILE) {
		/* get number of depth-velocity pairs */
		*nsvp = store->svp_num;

		/* get profile */
		for (int i = 0; i < *nsvp; i++) {
			depth[i] = 0.01 * store->svp_depth_res * store->svp_depth[i];
			velocity[i] = 0.1 * store->svp_vel[i];
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
		fprintf(stderr, "dbg2       kind:              %d\n", *kind);
		fprintf(stderr, "dbg2       nsvp:              %d\n", *nsvp);
		for (int i = 0; i < *nsvp; i++)
			fprintf(stderr, "dbg2       depth[%d]: %f   velocity[%d]: %f\n", i, depth[i], i, velocity[i]);
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:            %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad3_insert_svp(int verbose, void *mbio_ptr, void *store_ptr, int nsvp, double *depth, double *velocity,
                             int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       nsvp:       %d\n", nsvp);
		for (int i = 0; i < nsvp; i++)
			fprintf(stderr, "dbg2       depth[%d]: %f   velocity[%d]: %f\n", i, depth[i], i, velocity[i]);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_VELOCITY_PROFILE) {
		/* get number of depth-velocity pairs */
		store->svp_num = MIN(nsvp, MBSYS_SIMRAD3_MAXSVP);
		store->svp_depth_res = 1;

		/* get profile */
		for (int i = 0; i < store->svp_num; i++) {
			store->svp_depth[i] = (int)(100 * depth[i] / store->svp_depth_res);
			store->svp_vel[i] = (int)(10 * velocity[i]);
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
int mbsys_simrad3_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       copy_ptr:   %p\n", (void *)copy_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointers */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;
	struct mbsys_simrad3_struct *copy = (struct mbsys_simrad3_struct *)copy_ptr;

	int status = MB_SUCCESS;

	char *attitude_save;

	/* check if attitude data needs to be copied */
	if (store->attitude != NULL) {
		/* make sure a attitude data structure exists to
		    be copied into */
		if (copy->attitude == NULL) {
			status = mbsys_simrad3_attitude_alloc(verbose, mbio_ptr, copy_ptr, error);
		}

		/* save pointer value */
		attitude_save = (char *)copy->attitude;
	}

	char *netattitude_save;

	/* check if netattitude data needs to be copied */
	if (store->netattitude != NULL) {
		/* make sure a netattitude data structure exists to
		    be copied into */
		if (copy->netattitude == NULL) {
			status = mbsys_simrad3_netattitude_alloc(verbose, mbio_ptr, copy_ptr, error);
		}

		/* save pointer value */
		netattitude_save = (char *)copy->netattitude;
	}

	char *heading_save;

	/* check if heading data needs to be copied */
	if (store->heading != NULL) {
		/* make sure a heading data structure exists to
		    be copied into */
		if (copy->heading == NULL) {
			status = mbsys_simrad3_heading_alloc(verbose, mbio_ptr, copy_ptr, error);
		}

		/* save pointer value */
		heading_save = (char *)copy->heading;
	}

	char *ssv_save;

	/* check if ssv data needs to be copied */
	if (store->ssv != NULL) {
		/* make sure a ssv data structure exists to
		    be copied into */
		if (copy->ssv == NULL) {
			status = mbsys_simrad3_ssv_alloc(verbose, mbio_ptr, copy_ptr, error);
		}

		/* save pointer value */
		ssv_save = (char *)copy->ssv;
	}

	char *tilt_save;

	/* check if tilt data needs to be copied */
	if (store->tilt != NULL) {
		/* make sure a tilt data structure exists to
		    be copied into */
		if (copy->tilt == NULL) {
			status = mbsys_simrad3_tilt_alloc(verbose, mbio_ptr, copy_ptr, error);
		}

		/* save pointer value */
		tilt_save = (char *)copy->tilt;
	}

	/* copy the main structure */
	*copy = *store;

	/* if needed copy the attitude data structure */
	if (store->attitude != NULL && status == MB_SUCCESS) {
		copy->attitude = (struct mbsys_simrad3_attitude_struct *)attitude_save;
		struct mbsys_simrad3_attitude_struct *attitude_store =
                    (struct mbsys_simrad3_attitude_struct *)store->attitude;
		struct mbsys_simrad3_attitude_struct *attitude_copy =
                    (struct mbsys_simrad3_attitude_struct *)copy->attitude;
		*attitude_copy = *attitude_store;
	}

	/* if needed copy the netattitude data structure */
	if (store->netattitude != NULL && status == MB_SUCCESS) {
		copy->netattitude = (struct mbsys_simrad3_netattitude_struct *)netattitude_save;
		struct mbsys_simrad3_netattitude_struct *netattitude_store =
                    (struct mbsys_simrad3_netattitude_struct *)store->netattitude;
		struct mbsys_simrad3_netattitude_struct *netattitude_copy =
                    (struct mbsys_simrad3_netattitude_struct *)copy->netattitude;
		*netattitude_copy = *netattitude_store;
	}

	/* if needed copy the heading data structure */
	if (store->heading != NULL && status == MB_SUCCESS) {
		copy->heading = (struct mbsys_simrad3_heading_struct *)heading_save;
		struct mbsys_simrad3_heading_struct *heading_store = (struct mbsys_simrad3_heading_struct *)store->heading;
		struct mbsys_simrad3_heading_struct *heading_copy = (struct mbsys_simrad3_heading_struct *)copy->heading;
		*heading_copy = *heading_store;
	}

	/* if needed copy the ssv data structure */
	if (store->ssv != NULL && status == MB_SUCCESS) {
		copy->ssv = (struct mbsys_simrad3_ssv_struct *)ssv_save;
		struct mbsys_simrad3_ssv_struct *ssv_store = (struct mbsys_simrad3_ssv_struct *)store->ssv;
		struct mbsys_simrad3_ssv_struct *ssv_copy = (struct mbsys_simrad3_ssv_struct *)copy->ssv;
		*ssv_copy = *ssv_store;
	}

	/* if needed copy the tilt data structure */
	if (store->tilt != NULL && status == MB_SUCCESS) {
		copy->tilt = (struct mbsys_simrad3_tilt_struct *)tilt_save;
		struct mbsys_simrad3_tilt_struct *tilt_store = (struct mbsys_simrad3_tilt_struct *)store->tilt;
		struct mbsys_simrad3_tilt_struct *tilt_copy = (struct mbsys_simrad3_tilt_struct *)copy->tilt;
		*tilt_copy = *tilt_store;
	}

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
int mbsys_simrad3_makess(int verbose, void *mbio_ptr, void *store_ptr, int pixel_size_set, double *pixel_size,
                         int swath_width_set, double *swath_width, int pixel_int, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:        %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:       %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       pixel_size_set:  %d\n", pixel_size_set);
		fprintf(stderr, "dbg2       pixel_size:      %f\n", *pixel_size);
		fprintf(stderr, "dbg2       swath_width_set: %d\n", swath_width_set);
		fprintf(stderr, "dbg2       swath_width:     %f\n", *swath_width);
		fprintf(stderr, "dbg2       pixel_int:       %d\n", pixel_int);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA) {
		double ss[MBSYS_SIMRAD3_MAXPIXELS];
		int ss_cnt[MBSYS_SIMRAD3_MAXPIXELS];
		double ssacrosstrack[MBSYS_SIMRAD3_MAXPIXELS];
		double ssalongtrack[MBSYS_SIMRAD3_MAXPIXELS];

		/* get survey data structure */
		struct mbsys_simrad3_ping_struct *ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);

		/* zero the sidescan */
		for (int i = 0; i < MBSYS_SIMRAD3_MAXPIXELS; i++) {
			ss[i] = 0.0;
			ssacrosstrack[i] = 0.0;
			ssalongtrack[i] = 0.0;
			ss_cnt[i] = 0;
		}

		/* set scaling parameters */
		const double depthoffset = ping->png_xducer_depth;
		const double reflscale = 0.1;

		/* get raw pixel size */
		const double ss_spacing = 750.0 / ping->png_sample_rate;

		/* get beam angle size */
		double beamwidth;
		if (store->sonar == MBSYS_SIMRAD3_EM1000) {
			beamwidth = 2.5;
		}
		else if (ping->png_tx > 0) {
			beamwidth = 0.1 * ping->png_tx;
		}
		else if (store->run_tran_beam > 0) {
			beamwidth = 0.1 * store->run_tran_beam;
		} else {
			assert(false);
		}

		/* get median depth */
		int nbathsort = 0;
		double bathsort[MBSYS_SIMRAD3_MAXBEAMS];
		for (int i = 0; i < ping->png_nbeams; i++) {
			if (mb_beam_ok(ping->png_beamflag[i])) {
				bathsort[nbathsort] = ping->png_depth[i] + depthoffset;
				nbathsort++;
			}
		}

		/* get sidescan pixel size */
		if (!swath_width_set) {
			if (store->run_swath_angle > 0)
				*swath_width = (double)store->run_swath_angle;
			else
				*swath_width = 2.5 + MAX(90.0 - ping->png_depression[0], 90.0 - ping->png_depression[ping->png_nbeams - 1]);
		}

		if (!pixel_size_set && nbathsort > 0) {
			double pixel_size_calc;
			qsort((char *)bathsort, nbathsort, sizeof(double), (void *)mb_double_compare);
			pixel_size_calc = 2 * tan(DTR * (*swath_width)) * bathsort[nbathsort / 2] / MBSYS_SIMRAD3_MAXPIXELS;
			if (store->run_max_swath > 0) {
				const double pixel_size_max_swath = 2 * ((double)store->run_max_swath) / ((double)MBSYS_SIMRAD3_MAXPIXELS);
				if (pixel_size_max_swath < pixel_size_calc)
					pixel_size_calc = pixel_size_max_swath;
			}
			pixel_size_calc = MAX(pixel_size_calc, bathsort[nbathsort / 2] * sin(DTR * 0.1));
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
		const int pixel_int_use = pixel_int + 1;

		/* check that sidescan can be used */
		/* get times of bath and sidescan records */
		int time_i[7];
		time_i[0] = ping->png_date / 10000;
		time_i[1] = (ping->png_date % 10000) / 100;
		time_i[2] = ping->png_date % 100;
		time_i[3] = ping->png_msec / 3600000;
		time_i[4] = (ping->png_msec % 3600000) / 60000;
		time_i[5] = (ping->png_msec % 60000) / 1000;
		time_i[6] = (ping->png_msec % 1000) * 1000;
		double bath_time_d;
		mb_get_time(verbose, time_i, &bath_time_d);
		time_i[0] = ping->png_ss_date / 10000;
		time_i[1] = (ping->png_ss_date % 10000) / 100;
		time_i[2] = ping->png_ss_date % 100;
		time_i[3] = ping->png_ss_msec / 3600000;
		time_i[4] = (ping->png_ss_msec % 3600000) / 60000;
		time_i[5] = (ping->png_ss_msec % 60000) / 1000;
		time_i[6] = (ping->png_ss_msec % 1000) * 1000;
		double ss_time_d;
		mb_get_time(verbose, time_i, &ss_time_d);

		bool ss_ok = true;
		if (ping->png_nbeams < ping->png_nbeams_ss || ping->png_nbeams > ping->png_nbeams_ss + 1) {
			ss_ok = false;
			if (verbose > 0)
				fprintf(stderr,
				        "%s: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d Sidescan ignored: num bath beams != num ss beams: %d %d\n",
				        __func__, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
				        ping->png_nbeams, ping->png_nbeams_ss);
		}



		/* loop over raw sidescan, putting each raw pixel into
		    the binning arrays */
		if (ss_ok) {
			for (int i = 0; i < ping->png_nbeams_ss; i++) {
				short *beam_ss = &ping->png_ssraw[ping->png_start_sample[i]];
				if (mb_beam_ok(ping->png_beamflag[i])) {
					double ss_spacing_use;
					if (ping->png_beam_samples[i] > 0) {
						const double range =
						    sqrt(ping->png_depth[i] * ping->png_depth[i] + ping->png_acrosstrack[i] * ping->png_acrosstrack[i]);
						const double angle = 90.0 - ping->png_depression[i];
						const double beam_foot = range * sin(DTR * beamwidth) / cos(DTR * angle);
						const double sint = fabs(sin(DTR * angle));
						if (sint < ping->png_beam_samples[i] * ss_spacing / beam_foot)
							ss_spacing_use = beam_foot / ping->png_beam_samples[i];
						else
							ss_spacing_use = ss_spacing / sint;
					}
					for (int k = 0; k < ping->png_beam_samples[i]; k++) {
						if (beam_ss[k] != EM3_INVALID_AMP) {
							const double xtrackss = ping->png_acrosstrack[i] + ss_spacing_use * (k - ping->png_center_sample[i]);
							const int kk = MBSYS_SIMRAD3_MAXPIXELS / 2 + (int)(xtrackss / (*pixel_size));
							if (kk > 0 && kk < MBSYS_SIMRAD3_MAXPIXELS) {
								ss[kk] += reflscale * ((double)beam_ss[k]);
								ssalongtrack[kk] += ping->png_alongtrack[i];
								ss_cnt[kk]++;
							}
						}
					}
				}
			}
		}

		/* average the sidescan */
		int first = MBSYS_SIMRAD3_MAXPIXELS;
		int last = -1;
		for (int k = 0; k < MBSYS_SIMRAD3_MAXPIXELS; k++) {
			if (ss_cnt[k] > 0) {
				ss[k] /= ss_cnt[k];
				ssalongtrack[k] /= ss_cnt[k];
				ssacrosstrack[k] = (k - MBSYS_SIMRAD3_MAXPIXELS / 2) * (*pixel_size);
				first = MIN(first, k);
				last = k;
			}
			else
				ss[k] = MB_SIDESCAN_NULL;
		}

		/* interpolate the sidescan */
		int k1 = first;
		int k2 = first;
		for (int k = first + 1; k < last; k++) {
			if (ss_cnt[k] <= 0) {
				if (k2 <= k) {
					k2 = k + 1;
					while (k2 < last && ss_cnt[k2] <= 0)
						k2++;
				}
				if (k2 - k1 <= pixel_int_use) {
					ss[k] = ss[k1] + (ss[k2] - ss[k1]) * ((double)(k - k1)) / ((double)(k2 - k1));
					ssacrosstrack[k] = (k - MBSYS_SIMRAD3_MAXPIXELS / 2) * (*pixel_size);
					ssalongtrack[k] =
					    ssalongtrack[k1] + (ssalongtrack[k2] - ssalongtrack[k1]) * ((double)(k - k1)) / ((double)(k2 - k1));
				}
			}
			else {
				k1 = k;
			}
		}

		/* insert the new sidescan into store */
		ping->png_pixel_size = *pixel_size;
		if (last > first)
			ping->png_pixels_ss = MBSYS_SIMRAD3_MAXPIXELS;
		else
			ping->png_pixels_ss = 0;
		for (int i = 0; i < MBSYS_SIMRAD3_MAXPIXELS; i++) {
			if (ss[i] > MB_SIDESCAN_NULL) {
				ping->png_ss[i] = (short)(100 * ss[i]);
				ping->png_ssalongtrack[i] = (short)(100 * ssalongtrack[i]);
			}
			else {
				ping->png_ss[i] = EM3_INVALID_SS;
				ping->png_ssalongtrack[i] = 0;
			}
		}

		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  Sidescan regenerated in <%s>\n", __func__);
			fprintf(stderr, "dbg2       png_nbeams_ss: %d\n", ping->png_nbeams_ss);
			for (int i = 0; i < ping->png_nbeams_ss; i++)
				fprintf(stderr, "dbg2       beam:%d  flag:%3d  bath:%f  amp:%d  acrosstrack:%f  alongtrack:%f\n", i,
				        ping->png_beamflag[i], ping->png_depth[i], ping->png_amp[i], ping->png_acrosstrack[i],
				        ping->png_alongtrack[i]);
			fprintf(stderr, "dbg2       pixels_ss:  %d\n", MBSYS_SIMRAD3_MAXPIXELS);
			for (int i = 0; i < MBSYS_SIMRAD3_MAXPIXELS; i++)
				fprintf(stderr, "dbg2       pixel:%4d  cnt:%3d  ss:%10f  xtrack:%10f  ltrack:%10f\n", i, ss_cnt[i], ss[i],
				        ssacrosstrack[i], ssalongtrack[i]);
			fprintf(stderr, "dbg2       pixels_ss:  %d\n", ping->png_pixels_ss);
			for (int i = 0; i < MBSYS_SIMRAD3_MAXPIXELS; i++)
				fprintf(stderr, "dbg2       pixel:%4d  ss:%8d  ltrack:%8d\n", i, ping->png_ss[i], ping->png_ssalongtrack[i]);
		}
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
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

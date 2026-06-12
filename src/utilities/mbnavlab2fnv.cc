/*--------------------------------------------------------------------
 *    The MB-system:  mbnavlab2fnv.c  6/4/2026
 *
 *    Copyright (c) 2026-2026 by
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
 * mbnavlab2fnv converts a Kongsberg Navlab smoothed navigation binary file
 * (navlab_smooth.bin) to an MB-System fast navigation (fnv) text file.
 *
 * The navlab_smooth.bin format consists of records each containing 21
 * IEEE 754 double-precision (64-bit) floating-point values in little-endian
 * byte order with no file header. The fields used are:
 *
 *   field  0: time (Unix epoch seconds)
 *   field  1: latitude (decimal degrees, positive north)
 *   field  2: longitude (decimal degrees, positive east)
 *   field  3: depth (meters, positive down) - used as draft
 *   field  6: heading (degrees, 0-360, clockwise from north)
 *   field  7: roll (degrees, positive starboard up)
 *   field  8: pitch (degrees, positive bow up)
 *   field 10: speed through water (m/s) - converted to km/hr for fnv
 *
 * Heave is not available in navlab_smooth.bin and is output as 0.0.
 *
 * The fnv output format is a tab-separated ASCII text file with the
 * following columns:
 *
 *   yyyy mm dd hh mm ss.ssssss  epoch_seconds  longitude  latitude
 *   heading  speed_kmhr  draft_m  roll  pitch  heave
 *
 * Author:	D. W. Caress
 * Date:	2026
 *
 *--------------------------------------------------------------------*/

#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*--------------------------------------------------------------------*/
/* Program name and version */
static const char program_name[] = "mbnavlab2fnv";
static const char help_message[] =
    "mbnavlab2fnv converts a Kongsberg Navlab smoothed navigation binary\n"
    "file (navlab_smooth.bin) to an MB-System fast navigation (fnv)\n"
    "text file.\n\n"
    "The navlab_smooth.bin format contains records of 21 little-endian\n"
    "IEEE 754 double-precision floats per record with no file header:\n"
    "  field  0: time (Unix epoch seconds)\n"
    "  field  1: latitude (decimal degrees, positive north)\n"
    "  field  2: longitude (decimal degrees, positive east)\n"
    "  field  3: depth (meters, positive down) - output as draft\n"
    "  field  6: heading (degrees, 0-360)\n"
    "  field  7: roll (degrees, positive starboard up)\n"
    "  field  8: pitch (degrees, positive bow up)\n"
    "  field 10: speed through water (m/s)\n"
    "Heave is not available and is output as 0.0.";

static const char usage_message[] =
    "mbnavlab2fnv [-I navlab_smooth.bin] [-O output.fnv] [-V] [-H]";

/*--------------------------------------------------------------------*/
/* Navlab binary record layout */
#define NAVLAB_RECORD_NFIELDS  21
#define NAVLAB_RECORD_SIZE     (NAVLAB_RECORD_NFIELDS * sizeof(double))

#define NAVLAB_FIELD_TIME      0
#define NAVLAB_FIELD_LAT       1
#define NAVLAB_FIELD_LON       2
#define NAVLAB_FIELD_DEPTH     3
#define NAVLAB_FIELD_HEADING   6
#define NAVLAB_FIELD_ROLL      7
#define NAVLAB_FIELD_PITCH     8
#define NAVLAB_FIELD_SPEED     10

/*--------------------------------------------------------------------*/
/* Conversion constants */
#define MS_TO_KMHR             3.6

/*--------------------------------------------------------------------*/
/* Check host byte order and swap a double if needed.
 * navlab_smooth.bin is always little-endian. */
static int host_is_bigendian(void) {
    const uint16_t test = 0x0001;
    return (*((const uint8_t *)&test) == 0x00);
}

static double swap_double(double val) {
    double out;
    const uint8_t *src = (const uint8_t *)&val;
    uint8_t *dst = (uint8_t *)&out;
    dst[0] = src[7];
    dst[1] = src[6];
    dst[2] = src[5];
    dst[3] = src[4];
    dst[4] = src[3];
    dst[5] = src[2];
    dst[6] = src[1];
    dst[7] = src[0];
    return out;
}

/*--------------------------------------------------------------------*/
/* Break a Unix epoch time (seconds) into calendar components.
 * Uses gmtime_r for thread safety; falls back to gmtime on platforms
 * that lack gmtime_r. */
static void epoch_to_calendar(double time_d,
                               int *year, int *month, int *day,
                               int *hour, int *minute, double *second) {
    time_t t = (time_t)time_d;
    double frac = time_d - (double)t;
    struct tm ts;
#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 1
    gmtime_r(&t, &ts);
#else
    ts = *gmtime(&t);
#endif
    *year   = ts.tm_year + 1900;
    *month  = ts.tm_mon + 1;
    *day    = ts.tm_mday;
    *hour   = ts.tm_hour;
    *minute = ts.tm_min;
    *second = (double)ts.tm_sec + frac;
}

/*--------------------------------------------------------------------*/
int main(int argc, char **argv) {

    /* --- option defaults --- */
    char input_file[4096]  = "";   /* read from stdin if empty */
    char output_file[4096] = "";   /* write to stdout if empty */
    int  verbose           = 0;

    /* --- long option table --- */
    static struct option long_options[] = {
        { "help",    no_argument,       NULL, 'H' },
        { "verbose", no_argument,       NULL, 'V' },
        { "input",   required_argument, NULL, 'I' },
        { "output",  required_argument, NULL, 'O' },
        { NULL,      0,                 NULL,  0  }
    };

    /* --- parse command-line options --- */
    int opt;
    while ((opt = getopt_long(argc, argv, "HVI:O:", long_options, NULL)) != -1) {
        switch (opt) {
        case 'H':
            fprintf(stdout, "\nProgram %s\n\n", program_name);
            fprintf(stdout, "Usage:  %s\n\n", usage_message);
            fprintf(stdout, "%s\n\n", help_message);
            exit(EXIT_SUCCESS);
            break;
        case 'V':
            verbose = 1;
            break;
        case 'I':
            strncpy(input_file, optarg, sizeof(input_file) - 1);
            input_file[sizeof(input_file) - 1] = '\0';
            break;
        case 'O':
            strncpy(output_file, optarg, sizeof(output_file) - 1);
            output_file[sizeof(output_file) - 1] = '\0';
            break;
        default:
            fprintf(stderr, "Usage:  %s\n", usage_message);
            exit(EXIT_FAILURE);
        }
    }

    /* --- verbose banner --- */
    if (verbose) {
        fprintf(stderr, "\nProgram %s\n", program_name);
        fprintf(stderr, "Input file:  %s\n",
                strlen(input_file)  > 0 ? input_file  : "(stdin)");
        fprintf(stderr, "Output file: %s\n",
                strlen(output_file) > 0 ? output_file : "(stdout)");
    }

    /* --- open input --- */
    FILE *ifp;
    if (strlen(input_file) > 0) {
        ifp = fopen(input_file, "rb");
        if (ifp == NULL) {
            fprintf(stderr, "\n%s: ERROR - cannot open input file '%s': %s\n",
                    program_name, input_file, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    else {
        ifp = stdin;
    }

    /* --- open output --- */
    FILE *ofp;
    if (strlen(output_file) > 0) {
        ofp = fopen(output_file, "w");
        if (ofp == NULL) {
            fprintf(stderr, "\n%s: ERROR - cannot open output file '%s': %s\n",
                    program_name, output_file, strerror(errno));
            if (ifp != stdin)
                fclose(ifp);
            exit(EXIT_FAILURE);
        }
    }
    else {
        ofp = stdout;
    }

    /* --- determine whether byte-swapping is needed --- */
    const int swap = host_is_bigendian();

    /* --- write fnv header --- */
    fprintf(ofp,
        "## <yyyy mm dd hh mm ss.ssssss> <epoch seconds> "
        "<longitude (deg)> <latitude (deg)> "
        "<heading (deg)> <speed (km/hr)> <draft (m)> "
        "<roll (deg)> <pitch (deg)> <heave (m)>\n");

    /* --- main read/write loop --- */
    double rec[NAVLAB_RECORD_NFIELDS];
    long   nread    = 0;
    long   nwritten = 0;
    double prev_time_d = -1.0;

    while (fread(rec, sizeof(double), NAVLAB_RECORD_NFIELDS, ifp)
               == (size_t)NAVLAB_RECORD_NFIELDS) {
        nread++;

        /* byte-swap if host is big-endian */
        if (swap) {
            for (int i = 0; i < NAVLAB_RECORD_NFIELDS; i++)
                rec[i] = swap_double(rec[i]);
        }

        /* extract fields */
        const double time_d  = rec[NAVLAB_FIELD_TIME];
        const double lat     = rec[NAVLAB_FIELD_LAT];
        const double lon     = rec[NAVLAB_FIELD_LON];
        const double depth   = rec[NAVLAB_FIELD_DEPTH];
        const double heading = rec[NAVLAB_FIELD_HEADING];
        const double roll    = rec[NAVLAB_FIELD_ROLL];
        const double pitch   = rec[NAVLAB_FIELD_PITCH];
        const double speed   = rec[NAVLAB_FIELD_SPEED] * MS_TO_KMHR;
        const double heave   = 0.0;

        /* skip records with retrograde timestamps */
        if (time_d <= prev_time_d) {
            if (verbose)
                fprintf(stderr,
                    "%s: WARNING - skipping retrograde timestamp "
                    "%.6f (previous %.6f) at record %ld\n",
                    program_name, time_d, prev_time_d, nread);
            continue;
        }
        prev_time_d = time_d;

        /* decompose epoch time to calendar */
        int    year, month, day, hour, minute;
        double second;
        epoch_to_calendar(time_d, &year, &month, &day,
                          &hour, &minute, &second);

        /* write fnv record - all fields tab-separated:
         *
         *   yyyy mm dd hh mm ss.ssssss <TAB>
         *   epoch_seconds <TAB>
         *   longitude <TAB>
         *   latitude <TAB>
         *   heading <TAB>
         *   speed_kmhr <TAB>
         *   draft_m <TAB>
         *   roll <TAB>
         *   pitch <TAB>
         *   heave
         */
        fprintf(ofp,
            "%4d %02d %02d %02d %02d %09.6f\t"   /* date and time      */
            "%.6f\t"                               /* epoch seconds      */
            "%15.10f\t"                            /* longitude          */
            "%15.10f\t"                            /* latitude           */
            "%.3f\t"                               /* heading            */
            "%.3f\t"                               /* speed (km/hr)      */
            "%.4f\t"                               /* draft / depth      */
            "%.3f\t"                               /* roll               */
            "%.3f\t"                               /* pitch              */
            "%7.4f\n",                             /* heave              */
            year, month, day, hour, minute, second,
            time_d,
            lon,
            lat,
            heading,
            speed,
            depth,
            roll,
            pitch,
            heave);

        nwritten++;
    }

    /* --- check for read errors --- */
    if (ferror(ifp)) {
        fprintf(stderr, "\n%s: ERROR reading input: %s\n",
                program_name, strerror(errno));
    }

    /* --- close files --- */
    if (ifp != stdin)
        fclose(ifp);
    if (ofp != stdout)
        fclose(ofp);

    /* --- final status --- */
    if (verbose) {
        fprintf(stderr, "\n%s: %ld records read, %ld records written\n",
                program_name, nread, nwritten);
        if (nread != nwritten)
            fprintf(stderr, "%s: %ld records skipped (retrograde timestamps)\n",
                    program_name, nread - nwritten);
    }

    return EXIT_SUCCESS;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
 *    The MB-system:  mbusbl2fnv.c  7/20/2026
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
 * mbusbl2fnv converts a USBL (ultra-short baseline) ROV tracking CSV file
 * to an MB-System fast navigation (fnv) text file.
 *
 * The input CSV file has one header line followed by data lines with
 * four comma-separated fields:
 *
 *   epoch time (Unix epoch seconds)
 *   latitude (decimal degrees, positive north)
 *   longitude (decimal degrees, positive east)
 *   ROV depth (meters, positive down) - used as draft
 *
 * Heading, speed, roll, pitch, and heave are not present in the USBL
 * tracking data and are output as 0.0.
 *
 * The fnv output format is a tab-separated ASCII text file with the
 * following columns:
 *
 *   yyyy mm dd hh mm ss.ssssss  epoch_seconds  longitude  latitude
 *   heading  speed_kmhr  draft_m  roll  pitch  heave
 * 
 * This program was written with the assistance of the AI coding assistant 
 * Claude Sonnet 5 (Anthropic, model claude-sonnet-5), operating as Claude Code
 * under developer supervision and review.
 *
 * Author:	D. W. Caress
 * Date:	2026
 *
 *--------------------------------------------------------------------*/

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*--------------------------------------------------------------------*/
/* Program name and version */
static const char program_name[] = "mbusbl2fnv";
static const char help_message[] =
    "mbusbl2fnv converts a USBL (ultra-short baseline) ROV tracking CSV\n"
    "file to an MB-System fast navigation (fnv) text file.\n\n"
    "The input CSV file has one header line followed by data lines with\n"
    "four comma-separated fields:\n"
    "  field 0: epoch time (Unix epoch seconds)\n"
    "  field 1: latitude (decimal degrees, positive north)\n"
    "  field 2: longitude (decimal degrees, positive east)\n"
    "  field 3: ROV depth (meters, positive down) - output as draft\n"
    "Heading, speed, roll, pitch, and heave are not available in the\n"
    "USBL tracking data and are output as 0.0.";

static const char usage_message[] =
    "mbusbl2fnv [-I usbl.csv] [-O output.fnv] [-V] [-H]";

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
/* Return true if the given line looks like a data line (i.e. its first
 * non-whitespace character can begin a number) rather than a header or
 * comment line. */
static int line_is_data(const char *line) {
    while (*line == ' ' || *line == '\t')
        line++;
    if (*line == '\0' || *line == '\n' || *line == '\r' || *line == '#')
        return 0;
    return (isdigit((unsigned char)*line) || *line == '+' || *line == '-' ||
            *line == '.');
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
        ifp = fopen(input_file, "r");
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

    /* --- write fnv header --- */
    fprintf(ofp,
        "## <yyyy mm dd hh mm ss.ssssss> <epoch seconds> "
        "<longitude (deg)> <latitude (deg)> "
        "<heading (deg)> <speed (km/hr)> <draft (m)> "
        "<roll (deg)> <pitch (deg)> <heave (m)>\n");

    /* --- main read/write loop --- */
    char   line[4096];
    long   nread    = 0;
    long   nwritten = 0;
    long   nskipped = 0;
    double prev_time_d = -1.0;

    while (fgets(line, sizeof(line), ifp) != NULL) {

        /* skip header and any other non-data lines */
        if (!line_is_data(line))
            continue;

        nread++;

        double time_d, lat, lon, depth;
        const int nfields = sscanf(line, "%lf,%lf,%lf,%lf",
                                    &time_d, &lat, &lon, &depth);
        if (nfields != 4) {
            if (verbose)
                fprintf(stderr,
                    "%s: WARNING - skipping malformed line %ld: %s",
                    program_name, nread, line);
            nskipped++;
            continue;
        }

        const double heading = 0.0;
        const double speed   = 0.0;
        const double roll    = 0.0;
        const double pitch   = 0.0;
        const double heave   = 0.0;

        /* skip records with retrograde timestamps */
        if (time_d <= prev_time_d) {
            if (verbose)
                fprintf(stderr,
                    "%s: WARNING - skipping retrograde timestamp "
                    "%.6f (previous %.6f) at record %ld\n",
                    program_name, time_d, prev_time_d, nread);
            nskipped++;
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
            "%6.3f\t"                              /* speed (km/hr)      */
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
        if (nskipped > 0)
            fprintf(stderr, "%s: %ld records skipped\n",
                    program_name, nskipped);
    }

    return EXIT_SUCCESS;
}
/*--------------------------------------------------------------------*/

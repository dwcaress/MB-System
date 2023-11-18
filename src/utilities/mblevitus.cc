/*--------------------------------------------------------------------
 *    The MB-system:  mblevitus.c  4/15/93
 *
 *    Copyright (c) 1993-2023 by
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
 * MBLEVITUS generates an average water velocity profile for a
 * specified location from the Levitus temperature and salinity
 * database.
 *
 * The calculation of water sound velocity from salinity and
 * temperature observations proceeds in two steps. The first
 * step is to calculate the pressure as a function of depth
 * and latitude. We use equations from a 1989 book by Coates:
 * *
 * The second step is to calculate the water sound velocity.
 * We use the DelGrosso equation because of the results presented in
 *    Dusha, Brian D. Worcester, Peter F., Cornuelle, Bruce D.,
 *      Howe, Bruce. M. "On equations for the speed of sound
 *      in seawater", J. Acoust. Soc. Am., Vol 93, No 1,
 *      January 1993, pp 255-275.
 *
 * Author:  D. W. Caress
 * Date:  April 15, 1993
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <time.h>
#include <unistd.h>

// CMake build system
#ifdef CMAKE_BUILD_SYSTEM
const char *levitusfile = "$(levitusDir)/LevitusAnnual82.dat";

// Autotools build system
#else
#ifndef _WIN32
#include "levitus.h"
#endif
#endif

#include "mb_define.h"
#include "mb_status.h"

#ifdef _WIN32
#include <windows.h>
#endif

constexpr double MBLEVITUS_NO_DATA = -1000000000.0;;
constexpr int NDEPTH_MAX = 46;
constexpr int NLEVITUS_MAX = 33;

// TODO(schwehr): warning: excess elements in array initializer
constexpr float depth[48 /* NDEPTH_MAX + 2 */] =
    {0.0,    10.0,    20.0,    30.0,    50.0,    75.0,   100.0,  125.0,
     150.0,  200.0,   250.0,   300.0,   400.0,   500.0,  600.0,  700.0,
     800.0,  900.0,   1000.0,  1100.0,  1200.0,  1300.0, 1400.0, 1500.0,
     1750.0, 2000.0,  2500.0,  3000.0,  3500.0,  4000.0, 4500.0, 5000.0,
     5500.0, 6000.0,  6500.0,  7000.0,  7500.0,  8000.0, 8500.0, 9000.0,
     9500.0, 10000.0, 10500.0, 11000.0, 11500.0, 12000.0};

constexpr char program_name[] = "MBLEVITUS";
constexpr char help_message[] =
    "MBLEVITUS generates an average water velocity profile for a\n"
    "specified location from the Levitus temperature and salinity database.";
constexpr char usage_message[] =
    "mblevitus [-Rlon/lat -Ooutfile -V -H]";

/*--------------------------------------------------------------------*/

/* Windows implementation of GMT_runtime_bindir */
#ifdef _WIN32
char *GMT_runtime_bindir_win32(char *result) {
  TCHAR path[PATH_MAX + 1];

  /* Get absolute path of executable */
  if (GetModuleFileName(nullptr, path, PATH_MAX) == PATH_MAX)
    return nullptr;  /* Path too long */

/* Convert to cstring */
#ifdef _UNICODE
  wcstombs(result, path, PATH_MAX);  /* TCHAR is wchar_t* */
#else
  strncpy(result, path, PATH_MAX);  /* TCHAR is char * */
#endif

  /* Truncate full path to dirname */
  char *c = strrchr(result, '\\');
  if (c && c != result)
    *c = '\0';

  return result;
}
#endif
/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
  int verbose = 0;

  double longitude = 0.0;
  double latitude = 0.0;

  mb_path ofile;
  strcpy(ofile, "velocity");

  /* process argument list */
  bool help = false;
  {
    bool errflg = 0;
    int c;
    while ((c = getopt(argc, argv, "VvHhR:r:O:o:")) != -1)
      switch (c) {
      case 'H':
      case 'h':
        help = true;
        break;
      case 'V':
      case 'v':
        verbose++;
        break;
      case 'R':
      case 'r':
      {
        char *saveptr;
        const char *lonptr = strtok_r(optarg, "/", &saveptr);
        const char *latptr = strtok_r(nullptr, "/", &saveptr);
        if (lonptr != nullptr && latptr != nullptr) {
          longitude = mb_ddmmss_to_degree(lonptr);
          latitude = mb_ddmmss_to_degree(latptr);
        }
        break;
      }
      case 'O':
      case 'o':
        sscanf(optarg, "%s", ofile);
        break;
      case '?':
        errflg = true;
      }

    if (errflg) {
      fprintf(stderr, "usage: %s\n", usage_message);
      exit(MB_ERROR_BAD_USAGE);
    }
  }

  FILE *outfp;
  if (verbose <= 1)
    outfp = stdout;
  else
    outfp = stderr;

#ifdef _WIN32
  /* Find the path to the bin directory and from it, the location of the Levitus file */
  char levitusfile[PATH_MAX + 1];

  GMT_runtime_bindir_win32(levitusfile);
  char *pch = strrchr(levitusfile, '\\');
  pch[0] = '\0';
  strcat(levitusfile, "\\share\\mbsystem\\LevitusAnnual82.dat");
#endif

  if (verbose == 1 || help) {
    fprintf(outfp, "\nProgram %s\n", program_name);
    fprintf(outfp, "MB-system Version %s\n", MB_VERSION);
  }

  if (verbose >= 2) {
    fprintf(outfp, "\ndbg2  Program <%s>\n", program_name);
    fprintf(outfp, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(outfp, "dbg2  Control Parameters:\n");
    fprintf(outfp, "dbg2       verbose:          %d\n", verbose);
    fprintf(outfp, "dbg2       help:             %d\n", help);
    fprintf(outfp, "dbg2       levitusfile:      %s\n", levitusfile);
    fprintf(outfp, "dbg2       ofile:            %s\n", ofile);
    fprintf(outfp, "dbg2       longitude:        %f\n", longitude);
    fprintf(outfp, "dbg2       latitude:         %f\n", latitude);
  }

  if (help) {
    fprintf(outfp, "\n%s\n", help_message);
    fprintf(outfp, "\nusage: %s\n", usage_message);
    exit(MB_ERROR_NO_ERROR);
  }

  FILE *ifp = fopen(levitusfile, "rb");
  if (ifp == nullptr) {
    fprintf(stderr, "\nUnable to Open Levitus database file <%s> for reading\n", levitusfile);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(MB_ERROR_OPEN_FAIL);
  }

  if (longitude < -360.0 || longitude > 360.0 || latitude < -90.0 || latitude > 90.0) {
    fprintf(stderr, "\nInvalid location specified:  longitude: %f  latitude: %f\n", longitude, latitude);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(MB_ERROR_BAD_PARAMETER);
  }

  /* get the longitude and latitude indices */
  int ilon;
  if (longitude < 0.0)
    ilon = (int)(longitude + 360.0);
  else if (longitude >= 360.0)
    ilon = (int)(longitude - 360.0);
  else
    ilon = (int)longitude;
  const double lon_actual = ilon + 0.5;
  const int ilat = (int)(latitude + 90.0);
  const double lat_actual = ilat - 89.5;
  fprintf(outfp, "\nLocation for mean annual water velocity profile:\n");
  fprintf(outfp, "  Requested:  %6.4f longitude   %6.4f latitude\n", longitude, latitude);
  fprintf(outfp, "  Used:       %6.4f longitude   %6.4f latitude\n", lon_actual, lat_actual);

  int status = MB_SUCCESS;
  int error = MB_ERROR_NO_ERROR;

  /* read the temperature */
  const int record_size = sizeof(float) * NLEVITUS_MAX * 180;
  long location = ilon * record_size;
  /* int status = */ fseek(ifp, location, 0);
  float temperature[NLEVITUS_MAX][180];
  if (fread(&temperature[0][0], 1, record_size, ifp) != record_size) {
    status = MB_FAILURE;
    error = MB_ERROR_EOF;
    fprintf(stderr, "ERROR: EOF reading temperature\n");
  }

  /* read the salinity */
  location = location + 360 * record_size;
  /* status = */ fseek(ifp, location, 0);
  float salinity[NLEVITUS_MAX][180];
  if (fread(&salinity[0][0], 1, record_size, ifp) != record_size) {
    status = MB_FAILURE;
    error = MB_ERROR_EOF;
    fprintf(stderr, "ERROR: EOF reading salinity\n");
  }

  fclose(ifp);

#ifdef BYTESWAPPED
  for (int i = 0; i < NLEVITUS_MAX; i++) {
    mb_swap_float(&temperature[i][ilat]);
    mb_swap_float(&salinity[i][ilat]);
  }
#endif

  /* calculate velocity from temperature and salinity */
  int nvelocity = 0;
  int nvelocity_tot = 0;
  int last_good = -1;
  float velocity[NDEPTH_MAX];
  for (int i = 0; i < NDEPTH_MAX; i++) {
    if (i < NLEVITUS_MAX && salinity[i][ilat] > MBLEVITUS_NO_DATA) {
      last_good = i;
      nvelocity++;
    }
    if (last_good >= 0) {
      /* set counter */
      nvelocity_tot++;

      /* get pressure for a given depth
        as a function of latitude */
      const double pressure_dbar = 1.0052405 * depth[i] * (1.0 + 0.00528 * sin (DTR * lat_actual) * sin(DTR * lat_actual))
                                    + 0.00000236 * depth[i] * depth[i];

      /* calculate water sound speed using
        DelGrosso equations */
      /* convert decibar to kg/cm**2 */
      const double pressure = pressure_dbar * 0.1019716;
      const double c0 = 1402.392;
      const double dltact  = temperature[last_good][ilat]
            * ( 5.01109398873 + temperature[last_good][ilat]
              * (-0.0550946843172 + temperature[last_good][ilat] * 0.000221535969240));
      const double dltacs = salinity[last_good][ilat] * (1.32952290781 + salinity[last_good][ilat] * 0.000128955756844);

      const double dltacp = pressure * (0.156059257041E0 + pressure * (0.000024499868841 + pressure * -0.00000000883392332513));
      const double dcstp = temperature[last_good][ilat] *
                  (-0.0127562783426 * salinity[last_good][ilat] +
                   pressure * (0.00635191613389 +
                               pressure * (0.265484716608E-7 * temperature[last_good][ilat] - 0.00000159349479045 +
                                           0.522116437235E-9 * pressure) -
                               0.000000438031096213 * temperature[last_good][ilat] * temperature[last_good][ilat])) +
              salinity[last_good][ilat] *
                  (-0.161674495909E-8 * salinity[last_good][ilat] * pressure * pressure +
                   temperature[last_good][ilat] *
                       (0.0000968403156410 * temperature[last_good][ilat] +
                        pressure * (0.00000485639620015 * salinity[last_good][ilat] - 0.000340597039004)));
      velocity[i] = c0 + dltact + dltacs + dltacp + dcstp;
    }
  }

  /* check for existence of water velocity profile */
  if (nvelocity < 1) {
    fprintf(stderr, "\nNo water velocity profile available for specified location.\n");
    fprintf(stderr, "This place is probably subaerial!\n");
    fprintf(stderr, "No output file created.\n");
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(MB_ERROR_BAD_PARAMETER);
  }

  FILE *ofp = fopen(ofile, "w");
  if (ofp == nullptr) {
    fprintf(stderr, "\nUnable to Open output file <%s> for writing\n", ofile);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(MB_ERROR_OPEN_FAIL);
  }

  fprintf(ofp, "# Water velocity profile created by program %s\n", program_name);
  fprintf(ofp, "# MB-system Version %s\n", MB_VERSION);
  {
    char user[256], host[256], date[32];
    status = mb_user_host_date(verbose, user, host, date, &error);
    fprintf(ofp, "# Run by user <%s> on cpu <%s> at <%s>\n", user, host, date);
  }

  fprintf(ofp, "# Water velocity profile derived from Levitus\n");
  fprintf(ofp, "# temperature and salinity database.  This profile\n");
  fprintf(ofp, "# represents the annual average water velocity\n");
  fprintf(ofp, "# structure for a 1 degree X 1 degree area centered\n");
  fprintf(ofp, "# at %6.4f longitude and %6.4f latitude.\n", lon_actual, lat_actual);
  fprintf(ofp, "# This water velocity profile is in the form\n");
  fprintf(ofp, "# of discrete (depth, velocity) points where\n");
  fprintf(ofp, "# the depth is in meters and the velocity in\n");
  fprintf(ofp, "# meters/second.\n");
  fprintf(ofp, "# The first %d velocity values are defined using the\n", nvelocity);
  fprintf(ofp, "# salinity and temperature values available in the\n");
  fprintf(ofp, "# Levitus database; the remaining %d velocity values are\n", nvelocity_tot - nvelocity);
  fprintf(ofp, "# calculated using the deepest temperature\n");
  fprintf(ofp, "# and salinity value available.\n");

  for (int i = 0; i < nvelocity_tot; i++)
    fprintf(ofp, "%f %f\n", depth[i], velocity[i]);
  fprintf(outfp, "Values defined directly by Levitus database:      %2d\n", nvelocity);
  fprintf(outfp, "Values assuming deepest salinity and temperature: %2d\n", nvelocity_tot - nvelocity);
  fprintf(outfp, "Velocity points written:                          %2d\n", nvelocity_tot);
  fprintf(outfp, "Output file: %s\n", ofile);
  if (verbose >= 1) {
    fprintf(outfp, "\nMean annual water column profile:\n");
    fprintf(outfp, "     Depth Temperature Salinity   Velocity\n");
    for (int i = 0; i < nvelocity_tot; i++) {
      const double zero = 0.0;
      if (i < nvelocity)
        fprintf(outfp, "%10.4f %9.4f %9.4f   %9.4f\n", depth[i], temperature[i][ilat], salinity[i][ilat], velocity[i]);
      else
        fprintf(outfp, "%10.4f %9.4f %9.4f   %9.4f\n", depth[i], zero, zero, velocity[i]);
    }
  }

  fclose(ofp);

  if (verbose >= 2) {
    fprintf(outfp, "\ndbg2  Program <%s> completed\n", program_name);
    fprintf(outfp, "dbg2  Ending status:\n");
    fprintf(outfp, "dbg2       status:  %d\n", status);
    fprintf(outfp, "dbg2       error:   %d\n", error);
  }

  exit(error);
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
 *    The MB-system:  mbswath2las.c  11/26/20
 *
 *    Copyright (c) 2020-2025 by
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
 * MBswath2las exports swath bathymetry data from swath files to LAS format files.
 *
 * Author:  D. W. Caress
 * Date:  November 26, 2020
 *
 */

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <string>
#include <unistd.h>

#include <proj.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_info.h"
#include "mb_io.h"
#include "mb_status.h"

constexpr char program_name[] = "mbswath2las";
constexpr char help_message[] =
    "MBswath2las exports swath bathymetry data from swath files to LAS format files.";
constexpr char usage_message[] =
    "mbswath2las [-A -Bbegintime -Eendtime -Fformat -Iinputfile -Jprojection -Llonflip "
    "-Ooutputfile -P -Rwest/east/south/north -Sspeedmin -Ttimegap -V -H]";

namespace {

/*--------------------------------------------------------------------*/
/* LAS 1.4 output support
 *
 * MBswath2las writes LAS point files directly rather than depending on
 * a third-party LAS/LIDAR library (libLAS is no longer packaged by
 * Homebrew or MacPorts, and PDAL/LASzip pull in far more than this
 * simple point export needs). The LAS public header block, a single
 * "OGC Coordinate System WKT" variable length record, and point data
 * record format 1 are simple enough to emit with a handful of
 * primitive-field writes, which also keeps the output free of any
 * struct-packing/alignment surprises across gcc, clang, and MSVC.
 *
 * LAS 1.4 (rather than 1.2) is used specifically so that the sounding
 * coordinate reference system can be recorded via an OGC WKT VLR: LAS
 * 1.5 requires WKT and drops GeoTIFF-key CRS encoding entirely, while
 * 1.2/1.3 predate the WKT VLR and only support GeoTIFF keys. Point data
 * record format 1 (one of the "legacy" formats present since LAS 1.0)
 * remains valid through LAS 1.4, so the point records themselves are
 * unchanged from the original LAS 1.2 output.
 *
 * The LAS specification requires every multi-byte field to be stored
 * little-endian regardless of host byte order, so writes go through
 * byte-swapping helpers that are no-ops on the little-endian hosts
 * (macOS, Linux, Windows) this program targets and correct on any
 * big-endian host as well.
 */

constexpr int LAS_HEADER_SIZE = 375;
constexpr int LAS_VLR_HEADER_SIZE = 54;
constexpr uint16_t LAS_VLR_WKT_RECORD_ID = 2112;
constexpr uint16_t LAS_GLOBAL_ENCODING_WKT_BIT = 0x0010;
constexpr int LAS_POINT_RECORD_LENGTH = 28;
constexpr uint8_t LAS_POINT_DATA_FORMAT = 1;

int host_is_bigendian() {
  const uint16_t test = 0x0001;
  return *(reinterpret_cast<const uint8_t *>(&test)) == 0x00;
}

void las_put_u8(FILE *fp, uint8_t v) { fwrite(&v, 1, 1, fp); }
void las_put_i8(FILE *fp, int8_t v) { fwrite(&v, 1, 1, fp); }

void las_put_u16(FILE *fp, uint16_t v) {
  if (host_is_bigendian())
    v = static_cast<uint16_t>((v << 8) | (v >> 8));
  fwrite(&v, 2, 1, fp);
}

void las_put_u32(FILE *fp, uint32_t v) {
  if (host_is_bigendian())
    v = (v << 24) | ((v << 8) & 0x00FF0000) | ((v >> 8) & 0x0000FF00) | (v >> 24);
  fwrite(&v, 4, 1, fp);
}

void las_put_i32(FILE *fp, int32_t v) {
  uint32_t u;
  memcpy(&u, &v, 4);
  las_put_u32(fp, u);
}

void las_put_u64(FILE *fp, uint64_t v) {
  if (host_is_bigendian()) {
    uint64_t swapped = 0;
    for (int i = 0; i < 8; i++) {
      swapped = (swapped << 8) | (v & 0xFF);
      v >>= 8;
    }
    v = swapped;
  }
  fwrite(&v, 8, 1, fp);
}

void las_put_f64(FILE *fp, double v) {
  uint64_t u;
  memcpy(&u, &v, 8);
  las_put_u64(fp, u);
}

void las_put_str(FILE *fp, const char *s, size_t len) {
  char buf[64];
  memset(buf, 0, sizeof(buf));
  strncpy(buf, s, len < sizeof(buf) ? len : sizeof(buf) - 1);
  fwrite(buf, 1, len, fp);
}

/* Returns a WKT description of the coordinate system in which sounding
 * positions are expressed: the target CRS of the active PROJ transform
 * when a projection was requested (-J), or plain geographic WGS84
 * (EPSG:4326) otherwise. Returns an empty string on failure. The WKT
 * flavor defaults to WKT2 (used for the LAS file's own WKT VLR); pass
 * PJ_WKT1_ESRI to instead get the classic Esri-style WKT expected in a
 * ".prj" sidecar file (see write_prj_file()). */
std::string las_query_wkt(bool projected, void *pjptr, PJ_WKT_TYPE type = PJ_WKT2_2019) {
  PJ *crs = projected ? proj_get_target_crs(PJ_DEFAULT_CTX, static_cast<PJ *>(pjptr))
                      : proj_create(PJ_DEFAULT_CTX, "EPSG:4326");
  if (crs == nullptr)
    return std::string();

  const char *options[] = {"MULTILINE=NO", nullptr};
  const char *wkt = proj_as_wkt(PJ_DEFAULT_CTX, crs, type, options);
  std::string result = wkt != nullptr ? wkt : "";
  proj_destroy(crs);
  return result;
}

/* Writes an Esri-style ".prj" sidecar file next to a LAS output file,
 * for GIS software (e.g. ArcGIS) that looks for a coordinate system
 * defined this way rather than (or in addition to) the LAS file's own
 * WKT VLR. Named by replacing a trailing ".las"/".LAS" suffix on
 * las_filename with ".prj", or simply appending ".prj" otherwise. */
void write_prj_file(const char *las_filename, const std::string &wkt_esri) {
  std::string prj_filename(las_filename);
  const size_t len = prj_filename.size();
  if (len >= 4 && (prj_filename.compare(len - 4, 4, ".las") == 0 || prj_filename.compare(len - 4, 4, ".LAS") == 0))
    prj_filename.resize(len - 4);
  prj_filename += ".prj";

  FILE *fp = fopen(prj_filename.c_str(), "w");
  if (fp == nullptr) {
    fprintf(stderr, "\nWarning: unable to write projection file: %s\n", prj_filename.c_str());
    return;
  }
  fwrite(wkt_esri.c_str(), 1, wkt_esri.size(), fp);
  fputc('\n', fp);
  fclose(fp);
}

/* Accumulates point statistics for one output file and writes/rewrites
 * the LAS public header block. Point coordinates are stored using a
 * fixed scale/offset chosen at open() time so that points can be
 * streamed to disk as they are read, with only the header (bounds and
 * point count) patched in afterward via a seek back to the start. The
 * coordinate system WKT VLR is written once, immediately after the
 * header, when the file is opened, and is never rewritten. */
struct LasWriter {
  FILE *fp = nullptr;
  double xscale = 1.0, yscale = 1.0, zscale = 0.001;
  double xoffset = 0.0, yoffset = 0.0, zoffset = 0.0;
  double xmin = 0.0, xmax = 0.0, ymin = 0.0, ymax = 0.0, zmin = 0.0, zmax = 0.0;
  uint64_t npoints = 0;
  bool bounds_set = false;
  int creation_day = 1;
  int creation_year = 1970;
  uint32_t offset_to_point_data = LAS_HEADER_SIZE;
};

void las_write_header(LasWriter &w) {
  fseek(w.fp, 0, SEEK_SET);
  fwrite("LASF", 1, 4, w.fp);          /* file signature */
  las_put_u16(w.fp, 0);                /* file source ID */
  las_put_u16(w.fp, LAS_GLOBAL_ENCODING_WKT_BIT); /* global encoding: CRS is WKT, not GeoTIFF */
  las_put_u32(w.fp, 0);                /* project ID GUID data 1 */
  las_put_u16(w.fp, 0);                /* project ID GUID data 2 */
  las_put_u16(w.fp, 0);                /* project ID GUID data 3 */
  for (int i = 0; i < 8; i++)
    las_put_u8(w.fp, 0);               /* project ID GUID data 4 */
  las_put_u8(w.fp, 1);                 /* version major */
  las_put_u8(w.fp, 4);                 /* version minor */
  las_put_str(w.fp, "MB-System", 32);  /* system identifier */
  las_put_str(w.fp, program_name, 32); /* generating software */
  las_put_u16(w.fp, static_cast<uint16_t>(w.creation_day));
  las_put_u16(w.fp, static_cast<uint16_t>(w.creation_year));
  las_put_u16(w.fp, LAS_HEADER_SIZE);      /* header size */
  las_put_u32(w.fp, w.offset_to_point_data); /* offset to point data */
  las_put_u32(w.fp, 1);                    /* number of variable length records (the WKT VLR) */
  las_put_u8(w.fp, LAS_POINT_DATA_FORMAT);
  las_put_u16(w.fp, LAS_POINT_RECORD_LENGTH);
  const uint32_t legacy_npoints = w.npoints <= 0xFFFFFFFFu ? static_cast<uint32_t>(w.npoints) : 0;
  las_put_u32(w.fp, legacy_npoints);       /* legacy number of point records */
  las_put_u32(w.fp, legacy_npoints);       /* legacy number of points by return, 1st return */
  for (int i = 0; i < 4; i++)
    las_put_u32(w.fp, 0);                 /* legacy number of points by return, 2nd-5th */
  las_put_f64(w.fp, w.xscale);
  las_put_f64(w.fp, w.yscale);
  las_put_f64(w.fp, w.zscale);
  las_put_f64(w.fp, w.xoffset);
  las_put_f64(w.fp, w.yoffset);
  las_put_f64(w.fp, w.zoffset);
  las_put_f64(w.fp, w.bounds_set ? w.xmax : 0.0);
  las_put_f64(w.fp, w.bounds_set ? w.xmin : 0.0);
  las_put_f64(w.fp, w.bounds_set ? w.ymax : 0.0);
  las_put_f64(w.fp, w.bounds_set ? w.ymin : 0.0);
  las_put_f64(w.fp, w.bounds_set ? w.zmax : 0.0);
  las_put_f64(w.fp, w.bounds_set ? w.zmin : 0.0);
  las_put_u64(w.fp, 0);                    /* start of waveform data packet record (unused) */
  las_put_u64(w.fp, 0);                    /* start of first extended VLR (unused) */
  las_put_u32(w.fp, 0);                    /* number of extended VLRs (unused) */
  las_put_u64(w.fp, w.npoints);             /* number of point records */
  las_put_u64(w.fp, w.npoints);             /* number of points by return, 1st return */
  for (int i = 0; i < 14; i++)
    las_put_u64(w.fp, 0);                  /* number of points by return, 2nd-15th */
  fflush(w.fp);
}

/* Writes the "OGC Coordinate System WKT" VLR (LASF_Projection/2112)
 * immediately following the header. Called once at open() time; unlike
 * the header, this record is never rewritten. */
void las_write_wkt_vlr(LasWriter &w, const std::string &wkt) {
  las_put_u16(w.fp, 0);               /* reserved */
  las_put_str(w.fp, "LASF_Projection", 16);
  las_put_u16(w.fp, LAS_VLR_WKT_RECORD_ID);
  las_put_u16(w.fp, static_cast<uint16_t>(wkt.size() + 1)); /* record length after header, incl. NUL */
  las_put_str(w.fp, "OGC Coordinate System WKT", 32);
  fwrite(wkt.c_str(), 1, wkt.size() + 1, w.fp);
}

bool las_open(LasWriter &w, const char *filename, bool projected, const std::string &wkt) {
  w = LasWriter();
  w.fp = fopen(filename, "wb");
  if (w.fp == nullptr)
    return false;

  /* Choose a coordinate scale generous enough to cover a full survey
   * without an offset: geographic coordinates need ~1e-7 degree
   * (about 1 cm at the equator) to stay within +-180 degrees, while
   * projected (e.g. UTM) coordinates need ~1 cm to stay within a
   * +-10,000 km easting/northing range -- both comfortably fit a
   * signed 32-bit integer. */
  w.xscale = projected ? 0.01 : 0.0000001;
  w.yscale = w.xscale;
  w.zscale = 0.001;

  const time_t now = time(nullptr);
  struct tm gmt;
#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 1
  gmtime_r(&now, &gmt);
#else
  gmt = *gmtime(&now);
#endif
  w.creation_day = gmt.tm_yday + 1;
  w.creation_year = gmt.tm_year + 1900;

  w.offset_to_point_data = LAS_HEADER_SIZE + LAS_VLR_HEADER_SIZE + static_cast<uint32_t>(wkt.size() + 1);

  las_write_header(w);
  las_write_wkt_vlr(w, wkt);
  return true;
}

/* GPS time is stored as raw Unix epoch seconds (UTC) rather than the
 * GPS-week or leap-second-adjusted GPS time the LAS format nominally
 * expects -- sufficient for per-point ordering/lookup in this export,
 * and global encoding bit 0 is left clear to reflect that no GPS time
 * adjustment has been applied. */
void las_write_point(LasWriter &w, double x, double y, double z, double gps_time, uint16_t intensity) {
  const int32_t ix = static_cast<int32_t>(lround((x - w.xoffset) / w.xscale));
  const int32_t iy = static_cast<int32_t>(lround((y - w.yoffset) / w.yscale));
  const int32_t iz = static_cast<int32_t>(lround((z - w.zoffset) / w.zscale));
  las_put_i32(w.fp, ix);
  las_put_i32(w.fp, iy);
  las_put_i32(w.fp, iz);
  las_put_u16(w.fp, intensity);
  const uint8_t return_flags = 1 | (1 << 3); /* return number 1 of 1, no scan direction/edge flags */
  las_put_u8(w.fp, return_flags);
  las_put_u8(w.fp, 0);   /* classification: created, never classified */
  las_put_i8(w.fp, 0);   /* scan angle rank */
  las_put_u8(w.fp, 0);   /* user data */
  las_put_u16(w.fp, 0);  /* point source ID */
  las_put_f64(w.fp, gps_time);

  w.npoints++;
  if (!w.bounds_set) {
    w.xmin = w.xmax = x;
    w.ymin = w.ymax = y;
    w.zmin = w.zmax = z;
    w.bounds_set = true;
  } else {
    if (x < w.xmin) w.xmin = x;
    if (x > w.xmax) w.xmax = x;
    if (y < w.ymin) w.ymin = y;
    if (y > w.ymax) w.ymax = y;
    if (z < w.zmin) w.zmin = z;
    if (z > w.zmax) w.zmax = z;
  }
}

void las_close(LasWriter &w) {
  if (w.fp == nullptr)
    return;
  las_write_header(w);
  fclose(w.fp);
  w.fp = nullptr;
}

/* Closes any previously open file in w, then opens filename as a new LAS
 * output file, exiting the program on failure. If write_prj is set, also
 * writes an accompanying ".prj" sidecar (see write_prj_file()). */
void open_las_output(LasWriter &w, const char *filename, bool projected, const std::string &wkt, bool write_prj,
                      const std::string &wkt_esri, int verbose) {
  if (w.fp != nullptr)
    las_close(w);
  if (!las_open(w, filename, projected, wkt)) {
    fprintf(stderr, "\nUnable to open LAS output file: %s\n", filename);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(MB_ERROR_OPEN_FAIL);
  }
  if (verbose >= 1)
    fprintf(stderr, "Writing LAS output to %s\n", filename);
  if (write_prj)
    write_prj_file(filename, wkt_esri);
}

} // namespace

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
  int verbose = 0;
  int format;
  int pings;
  int pings_read;
  int lonflip;
  double bounds[4];
  int btime_i[7];
  int etime_i[7];
  double speedmin;
  double timegap;
  int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

  bool input_datalist = true;
  char read_file[MB_PATH_MAXLINE] = "datalist.mb-1";
  char projection_pars[MB_PATH_MAXLINE] = "";
  bool use_projection = false;

  /* output LAS file */
  char output_file[MB_PATH_MAXLINE] = "";
  bool output_file_set = false;

  /* if set, read the original swath files (so per-beam amplitude is
   * available for the LAS intensity field) rather than substituting
   * faster ".fbt" files, which do not carry amplitude */
  bool use_amplitude = false;

  /* if set, write an Esri-style ".prj" sidecar file next to each LAS
   * output file, for GIS software that looks for one */
  bool write_prj = false;

  /* swathbounds variables */
  int beam_port = 0;
  int beam_stbd = 0;
  int pixel_port = 0;
  int pixel_stbd = 0;

  /* projected coordinate system */
  char projection_id[MB_PATH_MAXLINE] = "";
  int proj_status;
  void *pjptr = nullptr;
  double reference_lon, reference_lat;
  int utm_zone;
  double naveasting, navnorthing;
  double headingx, headingy, mtodeglon, mtodeglat;

  /* process argument list */
  {
    static struct option options[] = {{"use-amplitude", no_argument, nullptr, 'A'},
                                      {"begin-time", required_argument, nullptr, 'B'},
                                      {"end-time", required_argument, nullptr, 'E'},
                                      {"format", required_argument, nullptr, 'F'},
                                      {"input", required_argument, nullptr, 'I'},
                                      {"projection", required_argument, nullptr, 'J'},
                                      {"lonflip", required_argument, nullptr, 'L'},
                                      {"output", required_argument, nullptr, 'O'},
                                      {"write-prj", no_argument, nullptr, 'P'},
                                      {"bounds", required_argument, nullptr, 'R'},
                                      {"speed-minimum", required_argument, nullptr, 'S'},
                                      {"time-gap", required_argument, nullptr, 'T'},
                                      {"verbose", no_argument, nullptr, 'V'},
                                      {"help", no_argument, nullptr, 'H'},
                                      {nullptr, 0, nullptr, 0}};

    bool errflg = false;
    bool help = false;
    int c;
    int option_index;
    while ((c = getopt_long(argc, argv, "AaB:b:E:e:F:f:I:i:J:j:L:l:O:o:PpR:r:S:s:T:t:VvHh", options, &option_index)) !=
           -1)
    {
      switch (c) {
      case 'A':
      case 'a':
        use_amplitude = true;
        break;
      case 'H':
      case 'h':
        help = true;
        break;
      case 'V':
      case 'v':
        verbose++;
        break;
      case 'B':
      case 'b':
        sscanf(optarg, "%d/%d/%d/%d/%d/%d", &btime_i[0], &btime_i[1], &btime_i[2], &btime_i[3], &btime_i[4], &btime_i[5]);
        btime_i[6] = 0;
        break;
      case 'E':
      case 'e':
        sscanf(optarg, "%d/%d/%d/%d/%d/%d", &etime_i[0], &etime_i[1], &etime_i[2], &etime_i[3], &etime_i[4], &etime_i[5]);
        etime_i[6] = 0;
        break;
      case 'F':
      case 'f':
        sscanf(optarg, "%d", &format);
        break;
      case 'I':
      case 'i':
        sscanf(optarg, "%1023s", read_file);
        break;
      case 'J':
      case 'j':
        sscanf(optarg, "%1023s", projection_pars);
        use_projection = true;
        break;
      case 'L':
      case 'l':
        sscanf(optarg, "%d", &lonflip);
        break;
      case 'O':
      case 'o':
        sscanf(optarg, "%1023s", output_file);
        output_file_set = true;
        break;
      case 'P':
      case 'p':
        write_prj = true;
        break;
      case 'R':
      case 'r':
        mb_get_bounds(optarg, bounds);
        break;
      case 'S':
      case 's':
        sscanf(optarg, "%lf", &speedmin);
        break;
      case 'T':
      case 't':
        sscanf(optarg, "%lf", &timegap);
        break;
      case '?':
        errflg = true;
      }
    }

    if (errflg) {
      fprintf(stderr, "usage: %s\n", usage_message);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_BAD_USAGE);
    }

    if (verbose == 1 || help) {
      fprintf(stderr, "\nProgram %s\n", program_name);
      fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
    }

    if (verbose >= 2) {
      fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
      fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
      fprintf(stderr, "dbg2  Control Parameters:\n");
      fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
      fprintf(stderr, "dbg2       help:           %d\n", help);
      fprintf(stderr, "dbg2       use_amplitude:  %d\n", use_amplitude);
      fprintf(stderr, "dbg2       write_prj:      %d\n", write_prj);
      fprintf(stderr, "dbg2       format:         %d\n", format);
      fprintf(stderr, "dbg2       lonflip:        %d\n", lonflip);
      fprintf(stderr, "dbg2       bounds[0]:      %f\n", bounds[0]);
      fprintf(stderr, "dbg2       bounds[1]:      %f\n", bounds[1]);
      fprintf(stderr, "dbg2       bounds[2]:      %f\n", bounds[2]);
      fprintf(stderr, "dbg2       bounds[3]:      %f\n", bounds[3]);
      fprintf(stderr, "dbg2       btime_i[0]:     %d\n", btime_i[0]);
      fprintf(stderr, "dbg2       btime_i[1]:     %d\n", btime_i[1]);
      fprintf(stderr, "dbg2       btime_i[2]:     %d\n", btime_i[2]);
      fprintf(stderr, "dbg2       btime_i[3]:     %d\n", btime_i[3]);
      fprintf(stderr, "dbg2       btime_i[4]:     %d\n", btime_i[4]);
      fprintf(stderr, "dbg2       btime_i[5]:     %d\n", btime_i[5]);
      fprintf(stderr, "dbg2       btime_i[6]:     %d\n", btime_i[6]);
      fprintf(stderr, "dbg2       etime_i[0]:     %d\n", etime_i[0]);
      fprintf(stderr, "dbg2       etime_i[1]:     %d\n", etime_i[1]);
      fprintf(stderr, "dbg2       etime_i[2]:     %d\n", etime_i[2]);
      fprintf(stderr, "dbg2       etime_i[3]:     %d\n", etime_i[3]);
      fprintf(stderr, "dbg2       etime_i[4]:     %d\n", etime_i[4]);
      fprintf(stderr, "dbg2       etime_i[5]:     %d\n", etime_i[5]);
      fprintf(stderr, "dbg2       etime_i[6]:     %d\n", etime_i[6]);
      fprintf(stderr, "dbg2       speedmin:       %f\n", speedmin);
      fprintf(stderr, "dbg2       timegap:        %f\n", timegap);
      fprintf(stderr, "dbg2       output_file:    %s\n", output_file);
    }

    if (help) {
      fprintf(stderr, "\n%s\n", help_message);
      fprintf(stderr, "\nusage: %s\n", usage_message);
      exit(MB_ERROR_NO_ERROR);
    }
  }

  int error = MB_ERROR_NO_ERROR;

  if (format == 0)
    mb_get_format(verbose, read_file, nullptr, &format, &error);

  /* format as resolved from read_file alone (negative if read_file is a
   * datalist), preserved for later use: the main per-file loop below
   * mutates "format" in place (via mb_datalist_read) to the format of
   * whichever swath file it is currently on, so anything that needs to
   * know whether the original input was a datalist must use this copy
   * instead of "format" itself. */
  const int format_original = format;

  /* determine whether to read one file or a list of files */
  const bool read_datalist = format < 0;
  bool read_data;
  void *datalist;
  char file[MB_PATH_MAXLINE] = "";
  char dfile[MB_PATH_MAXLINE] = "";
  double file_weight;

  /* open file list */
  if (read_datalist) {
    const int look_processed = MB_DATALIST_LOOK_UNSET;
    if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
      fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    read_data = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS;
  } else {
    // else copy single filename to be read
    strcpy(file, read_file);
    read_data = true;
  }

  double btime_d;
  double etime_d;
  int beams_bath;
  int beams_amp;
  int pixels_ss;

  /* MBIO read values */
  void *mbio_ptr = nullptr;
  void *store_ptr = nullptr;
  int kind;
  int time_i[7];
  double time_d;
  double navlon;
  double navlat;
  double speed;
  double heading;
  double distance;
  double altitude;
  double sensordepth;
  double draft;
  double roll;
  double pitch;
  double heave;
  char *beamflag = nullptr;
  double *bath = nullptr;
  double *bathacrosstrack = nullptr;
  double *bathalongtrack = nullptr;
  int *detect = nullptr;
  double *amp = nullptr;
  double *ss = nullptr;
  double *ssacrosstrack = nullptr;
  double *ssalongtrack = nullptr;
  char comment[MB_COMMENT_MAXLINE];
  int icomment = 0;

  /* LAS output file(s): a single combined file when -O/--output names one
   * explicitly, otherwise one LAS file per input swath file, named by
   * appending ".las" to the swath file name */
  LasWriter las;
  unsigned long long las_points_written = 0;

  /* WKT description of the sounding coordinate system, written into each
   * LAS output file's "OGC Coordinate System WKT" VLR. */
  std::string las_wkt;

  /* Amplitude range used to rescale sonar amplitude into the LAS
   * intensity field's unsigned 16-bit range (see the beam loop below).
   * Sonar amplitude units vary by format -- often signed dB, sometimes
   * raw positive counts -- so a fixed zero point cannot be assumed;
   * only a linear rescaling of the actual observed range preserves the
   * data. Left at 0/0 (and so unused, see below) unless -A/--use-amplitude
   * is given. */
  double amp_min = 0.0;
  double amp_max = 0.0;

  if (use_projection || use_amplitude) {
    /* Ensure every input file's .inf/.fbt/.fnv sidecars exist and are
     * current, then use the resulting merged bounds/amplitude range --
     * rather than reading any sounding data -- to resolve a bare UTM
     * zone or local Transverse Mercator origin (the same approach used
     * by mbgrid and mbmosaic) and/or the amplitude rescaling above.
     * This also lets the projection (and hence its WKT) be fully
     * resolved before any output file needs to open.
     * mb_make_info_datalist/mb_get_info_datalist each mutate their
     * format argument while walking the datalist, so each gets its own
     * copy of format_original (the format as resolved from read_file
     * itself, before the main per-file loop's own mb_datalist_read call
     * overwrites "format" with whatever swath file it is currently on)
     * rather than sharing that mutated variable. */
    int format_for_inf = format_original;
    mb_make_info_datalist(verbose, false, read_file, &format_for_inf, &error);

    struct mb_info_struct mb_info;
    int format_for_bounds = format_original;
    mb_get_info_datalist(verbose, read_file, &format_for_bounds, &mb_info, lonflip, &error);
    amp_min = mb_info.amp_min;
    amp_max = mb_info.amp_max;

    if (use_projection) {
      const double gbnd[4] = {mb_info.lon_min, mb_info.lon_max, mb_info.lat_min, mb_info.lat_max};

      /* Default projection is UTM */
      if (strlen(projection_pars) == 0)
        strcpy(projection_pars, "U");

      /* check for UTM with undefined zone */
      if (strcmp(projection_pars, "UTM") == 0 || strcmp(projection_pars, "U") == 0 ||
          strcmp(projection_pars, "utm") == 0 || strcmp(projection_pars, "u") == 0) {
        reference_lon = 0.5 * (gbnd[0] + gbnd[1]);
        if (reference_lon < 180.0)
          reference_lon += 360.0;
        if (reference_lon >= 180.0)
          reference_lon -= 360.0;
        utm_zone = (int)(((reference_lon + 183.0) / 6.0) + 0.5);
        reference_lat = 0.5 * (gbnd[2] + gbnd[3]);
        if (reference_lat >= 0.0)
          snprintf(projection_id, sizeof(projection_id), "UTM%2.2dN", utm_zone);
        else
          snprintf(projection_id, sizeof(projection_id), "UTM%2.2dS", utm_zone);
      }
      /* check for local Transverse Mercator with undefined origin */
      else if (strncmp(projection_pars, "LTM", 3) == 0 || strncmp(projection_pars, "ltm", 3) == 0 ||
               strcmp(projection_pars, "L") == 0 || strcmp(projection_pars, "l") == 0) {
        if (sscanf(projection_pars, "LTM%lf/%lf", &reference_lon, &reference_lat) == 2 ||
            sscanf(projection_pars, "ltm%lf/%lf", &reference_lon, &reference_lat) == 2) {
          strncpy(projection_id, projection_pars, sizeof(projection_id));
        } else {
          reference_lon = 0.5 * (gbnd[0] + gbnd[1]);
          reference_lat = 0.5 * (gbnd[2] + gbnd[3]);
          snprintf(projection_id, sizeof(projection_id), "LTM%.5f/%.5f", reference_lon, reference_lat);
        }
      }
      else
        strcpy(projection_id, projection_pars);

      /* set projection flag */
      proj_status = mb_proj_init(verbose, projection_id, &(pjptr), &error);

      /* if projection not successfully initialized then quit */
      if (proj_status != MB_SUCCESS) {
        fprintf(stderr, "\nOutput projection %s not found in database\n", projection_id);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        error = MB_ERROR_BAD_PARAMETER;
        mb_memory_clear(verbose, &error);
        exit(MB_ERROR_BAD_PARAMETER);
      }
    }
  }

  las_wkt = las_query_wkt(use_projection, pjptr);

  /* Report the coordinate system actually used, including the default
   * geographic case, since the bare -J U/UTM and -J L/LTM forms do not
   * otherwise reveal which zone or origin was selected. */
  if (verbose >= 1) {
    if (use_projection)
      fprintf(stderr, "Projection ID: %s\n", projection_id);
    else
      fprintf(stderr, "Projection ID: Geographic (WGS84 / EPSG:4326)\n");
  }

  /* Esri-style WKT1 for the optional ".prj" sidecar (-P/--write-prj);
   * only computed when actually needed. */
  std::string las_wkt_esri;
  if (write_prj)
    las_wkt_esri = las_query_wkt(use_projection, pjptr, PJ_WKT1_ESRI);

  char rfile[MB_PATH_MAXLINE] = "";
  int rformat = 0;

  /* loop over all files to be read */
  while (read_data) {

    /* Substitute the faster ".fbt" file for the original swath file when
     * available, unless -A/--use-amplitude was given: .fbt files are
     * generated bathymetry-only (mbcopy's "-D" bathonly mode), so using
     * one would silently make every LAS intensity value 0. */
    strcpy(rfile, file);
    rformat = format;
    if (!use_amplitude)
      mb_get_fbt(verbose, rfile, &rformat, &error);

    /* initialize reading the swath file */
    if (mb_read_init(verbose, rfile, rformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &mbio_ptr,
                               &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
      char *message;
      mb_error(verbose, error, &message);
      fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
      fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", rfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(error);
    }

    /* allocate memory for data arrays */
    if (error == MB_ERROR_NO_ERROR)
      /* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
    if (error == MB_ERROR_NO_ERROR)
      /* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
    if (error == MB_ERROR_NO_ERROR)
      /* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
    if (error == MB_ERROR_NO_ERROR)
      /* status &= */
          mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
    if (error == MB_ERROR_NO_ERROR)
      /* status &= */
          mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
    if (error == MB_ERROR_NO_ERROR)
      /* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
    if (error == MB_ERROR_NO_ERROR)
      /* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
    if (error == MB_ERROR_NO_ERROR)
      /* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);

    /* if error initializing memory then quit */
    if (error != MB_ERROR_NO_ERROR) {
      char *message;
      mb_error(verbose, error, &message);
      fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(error);
    }

    /* set up the output LAS file: one combined file opened once when the
     * user supplied an explicit name, otherwise a fresh file per input
     * swath file (closing/finalizing the previous one first) */
    bool new_output_file = false;
    if (output_file_set) {
      new_output_file = (las.fp == nullptr);
    } else {
      new_output_file = true;
      char fileroot[MB_PATH_MAXLINE];
      int format_guess = 0;
      int format_status = mb_get_format(verbose, file, fileroot, &format_guess, &error);
      if (format_status == MB_SUCCESS && format_guess == format)
        snprintf(output_file, sizeof(output_file), "%s.las", fileroot);
      else
        snprintf(output_file, sizeof(output_file), "%s.las", file);
    }

    if (new_output_file)
      open_las_output(las, output_file, use_projection, las_wkt, write_prj, las_wkt_esri, verbose);

    /* read and print data */
    int nread = 0;
    bool first = true;
    while (error <= MB_ERROR_NO_ERROR) {
      /* reset error */
      error = MB_ERROR_NO_ERROR;

      /* read next data record */
      status = mb_get_all(verbose, mbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
                          &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
                          bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

      /* time gaps are not a problem here */
      if (error == MB_ERROR_TIME_GAP) {
        error = MB_ERROR_NO_ERROR;
        status = MB_SUCCESS;
      }

      /* if survey data extract nav */
      if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA)
        status = mb_extract_nav(verbose, mbio_ptr, store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed,
                                &heading, &draft, &roll, &pitch, &heave, &error);

      /* make sure non survey data records are ignored */
      if (error == MB_ERROR_NO_ERROR && kind != MB_DATA_DATA)
        error = MB_ERROR_OTHER;

      /* get projected navigation if needed (projection already resolved
       * above, before the file loop began) */
      if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA && use_projection)
        mb_proj_forward(verbose, pjptr, navlon, navlat, &naveasting, &navnorthing, &error);

      if (verbose >= 2) {
        fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
        fprintf(stderr, "dbg2       kind:           %d\n", kind);
        fprintf(stderr, "dbg2       error:          %d\n", error);
        fprintf(stderr, "dbg2       status:         %d\n", status);
      }

      if (verbose >= 1 && kind == MB_DATA_COMMENT) {
        if (icomment == 0) {
          fprintf(stderr, "\nComments:\n");
          icomment++;
        }
        fprintf(stderr, "%s\n", comment);
      }

      /* get factors for lon lat calculations */
      if (error == MB_ERROR_NO_ERROR) {
        mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
        headingx = sin(DTR * heading);
        headingy = cos(DTR * heading);
      }

      /* now loop over beams, writing one LAS point per valid sounding */
      if (error == MB_ERROR_NO_ERROR)
        for (int j = 0; j < beams_bath; j++) {
          if (!mb_beam_ok(beamflag[j]))
            continue;

          double x, y;
          if (use_projection) {
            x = naveasting + headingy * bathacrosstrack[j] + headingx * bathalongtrack[j];
            y = navnorthing - headingx * bathacrosstrack[j] + headingy * bathalongtrack[j];
          } else {
            x = navlon + headingy * mtodeglon * bathacrosstrack[j] + headingx * mtodeglon * bathalongtrack[j];
            y = navlat - headingx * mtodeglat * bathacrosstrack[j] + headingy * mtodeglat * bathalongtrack[j];
          }

          /* LAS elevation is positive up; bathymetry depths are positive down */
          const double z = -bath[j];

          uint16_t intensity = 0;
          if (use_amplitude && amp != nullptr && j < beams_amp && amp_max > amp_min) {
            double scaled = (amp[j] - amp_min) / (amp_max - amp_min) * 65535.0;
            if (scaled < 0.0) scaled = 0.0;
            if (scaled > 65535.0) scaled = 65535.0;
            intensity = static_cast<uint16_t>(lround(scaled));
          }

          las_write_point(las, x, y, z, time_d, intensity);
          las_points_written++;
        }
    }

    /* close the swath file */
    status &= mb_close(verbose, &mbio_ptr, &error);

    /* figure out whether and what to read next */
    if (read_datalist) {
      read_data = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS;
    } else {
      read_data = false;
    }

    /* if writing one LAS file per input file, finalize this one now */
    if (!output_file_set)
      las_close(las);

    /* end loop over files in list */
  }
  if (read_datalist)
    mb_datalist_close(verbose, &datalist, &error);

  /* finalize the combined output file, if any */
  las_close(las);

  if (verbose >= 1)
    fprintf(stderr, "\nTotal LAS points written: %llu\n", las_points_written);

  if (pjptr != nullptr)
    mb_proj_free(verbose, &pjptr, &error);

  if (verbose >= 4)
    status &= mb_memory_list(verbose, &error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
    fprintf(stderr, "dbg2  Ending status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  exit(error);
}
/*--------------------------------------------------------------------*/

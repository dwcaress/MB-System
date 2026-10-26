// Writes a synthetic swath bathymetry file (format 71, MBF_MBLDEOIH) over a
// known seafloor, for tests of CUBE gridding in mbgrid (mbgrid_test.py).
//
// Usage: mbcube_synthetic output_file seafloor depth sensordepth [seed]
//   seafloor:    flat, step (0.5 m deeper east of the first line) or spikes
//                (2% of the soundings 5 to 20 m off the seafloor)
//   depth:       seafloor depth (m, positive down)
//   sensordepth: sonar depth (m, positive down), e.g. 0 for a surface
//                vessel or depth - 50 for an AUV
//
// Two survey lines run north, 120 m apart, with 300 pings 1 m apart and
// 101 beams over +/-60 degrees, and 0.05 m of vertical noise.
//
// See README.md file for copying and redistribution conditions.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"

namespace {

const int verbose = 0;
const double kLon0 = -124.5;
const double kLat0 = 40.8;
const int kLines = 2;
const double kLineSpacing = 120.0;
const int kPings = 300;
const double kPingSpacing = 1.0;
const int kBeams = 101;
const double kSwathAngle = 60.0;
const double kNoise = 0.05;
const double kStep = 0.5;

// Deterministic random numbers, identical on all platforms (the standard
// library distributions are implementation specific)
uint64_t rng_state = 1;
double rng_uniform() {
  rng_state ^= rng_state << 13;
  rng_state ^= rng_state >> 7;
  rng_state ^= rng_state << 17;
  return (double)(rng_state >> 11) / 9007199254740992.0;
}
double rng_normal() {
  const double u1 = std::max(rng_uniform(), 1.0e-300);
  const double u2 = rng_uniform();
  return sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
}

}  // namespace

int main(int argc, char **argv) {
  if (argc < 5) {
    fprintf(stderr, "usage: %s output_file flat|step|spikes depth sensordepth [seed]\n", argv[0]);
    return 2;
  }
  const std::string seafloor = argv[2];
  const double depth0 = atof(argv[3]);
  const double sensordepth = atof(argv[4]);
  const unsigned int seed = argc > 5 ? (unsigned int)atoi(argv[5]) : 1;
  if (seafloor != "flat" && seafloor != "step" && seafloor != "spikes") {
    fprintf(stderr, "unknown seafloor %s\n", seafloor.c_str());
    return 2;
  }

  int error = MB_ERROR_NO_ERROR;
  void *mbio_ptr = nullptr;
  int beams_bath, beams_amp, pixels_ss;
  if (mb_write_init(verbose, argv[1], MBF_MBLDEOIH, &mbio_ptr, &beams_bath, &beams_amp, &pixels_ss, &error) !=
      MB_SUCCESS) {
    fprintf(stderr, "unable to open %s for writing, error %d\n", argv[1], error);
    return 1;
  }
  void *store_ptr = ((struct mb_io_struct *)mbio_ptr)->store_data;

  double mtodeglon, mtodeglat;
  mb_coor_scale(verbose, kLat0, &mtodeglon, &mtodeglat);

  rng_state = 88172645463325252ULL + seed;

  std::vector<char> beamflag(kBeams, MB_FLAG_NONE);
  std::vector<double> bath(kBeams), acrosstrack(kBeams), alongtrack(kBeams, 0.0);
  double amp = 0.0, ss = 0.0, ssacrosstrack = 0.0, ssalongtrack = 0.0;
  char comment[MB_COMMENT_MAXLINE] = "";
  int time_i[7];
  double time_d = 1.7e9;
  const double altitude = depth0 - sensordepth;
  const double speed = 3.6 * kPingSpacing;  // one ping per second, km/hr

  int nsoundings = 0;
  for (int line = 0; line < kLines; line++) {
    const double xline = line * kLineSpacing;
    for (int ping = 0; ping < kPings; ping++) {
      const double yping = ping * kPingSpacing;
      for (int ib = 0; ib < kBeams; ib++) {
        const double angle = (-kSwathAngle + ib * 2.0 * kSwathAngle / (kBeams - 1)) * M_PI / 180.0;
        acrosstrack[ib] = altitude * tan(angle);
        const double xbeam = xline + acrosstrack[ib];
        double depth = depth0;
        if (seafloor == "step" && xbeam > 0.0)
          depth += kStep;
        depth += kNoise * rng_normal();
        if (seafloor == "spikes" && rng_uniform() < 0.02)
          depth += (rng_uniform() < 0.5 ? -1.0 : 1.0) * (5.0 + 15.0 * rng_uniform());
        bath[ib] = depth;
        nsoundings++;
      }
      mb_get_date(verbose, time_d, time_i);
      const double navlon = kLon0 + xline * mtodeglon;
      const double navlat = kLat0 + yping * mtodeglat;
      const double heading = 0.0;
      // format 71 stores depths relative to the sonar depth, which can only
      // be set once the record is a data record: insert the beams again
      // after setting the navigation and sonar depth
      for (int pass = 0; pass < 2; pass++) {
        mb_insert(verbose, mbio_ptr, store_ptr, MB_DATA_DATA, time_i, time_d, navlon, navlat, speed, heading, kBeams, 0,
                  0, beamflag.data(), bath.data(), &amp, acrosstrack.data(), alongtrack.data(), &ss, &ssacrosstrack,
                  &ssalongtrack, comment, &error);
        mb_insert_nav(verbose, mbio_ptr, store_ptr, time_i, time_d, navlon, navlat, speed, heading, sensordepth, 0.0,
                      0.0, 0.0, &error);
        mb_insert_altitude(verbose, mbio_ptr, store_ptr, sensordepth, altitude, &error);
      }
      if (mb_write_ping(verbose, mbio_ptr, store_ptr, &error) != MB_SUCCESS) {
        fprintf(stderr, "error %d writing ping %d of line %d\n", error, ping, line);
        mb_close(verbose, &mbio_ptr, &error);
        return 1;
      }
      time_d += 1.0;
    }
  }
  mb_close(verbose, &mbio_ptr, &error);
  printf("%s: %d soundings, seafloor %s at %.2f m, sonar at %.2f m\n", argv[1], nsoundings, seafloor.c_str(), depth0,
         sensordepth);
  return 0;
}

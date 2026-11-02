/*--------------------------------------------------------------------
 *    The MB-system:  mbvt_core.cc  2/11/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

#include "mbvt_core.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <random>
#include <thread>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_status.h"

namespace mbvt {

namespace fs = std::filesystem;

namespace {

constexpr int NUM_EDIT_START = 6;
constexpr double DEPTH_EDIT[NUM_EDIT_START] = {0.0, 300.0, 1000.0, 3000.0, 7000.0, 12000.0};
constexpr double VELOCITY_EDIT = 1500.0;
constexpr double MODEL_BOTTOM = 12000.0;
constexpr int RAY_OUT_BOTTOM = 5;  // MB_RT_OUT_BOTTOM, private to mb_rt.c
constexpr int BATCH_PINGS_PER_THREAD = 64;
constexpr double LISTED_TOLERANCE = 0.0051;  // mbsvplist writes depth and velocity with two decimals
constexpr int SVP_BUFFER = 65536;            // mb_extract_svp() has no size argument, and the drivers hold up to 16 bit counts

/* copy into the char buffer the MBIO functions take */
bool to_mb_path(const std::string &in, mb_path out, std::string &error) {
  if (in.size() >= sizeof(mb_path)) {
    error = "File name too long: " + in;
    return false;
  }
  snprintf(out, sizeof(mb_path), "%s", in.c_str());
  return true;
}

void write_header(FILE *fp, const char *title) {
  char user[256] = "", host[256] = "", date[32] = "";
  int error = MB_ERROR_NO_ERROR;
  mb_user_host_date(0, user, host, date, &error);
  fprintf(fp, "## %s\n", title);
  fprintf(fp, "## Output by Program %s\n", PROGRAM_NAME);
  fprintf(fp, "## MB-System Version %s\n", MB_VERSION);
  fprintf(fp, "## Run by user <%s> on cpu <%s> at <%s>\n", user, host, date);
}

/* One ping traced with mb_rt(): filled by a worker thread, added to the sums in ping order */
struct PingResult {
  std::vector<char> used;
  std::vector<double> acrosstrack, depth, residual;
  double sensordepth = 0.0;
  double bath_min = 10000.0, bath_max = 0.0;
  int nbeams = 0;
  long traced = 0, failed = 0, at_bottom = 0;
};

void trace_ping(void *model, const Ping &ping, int anglemode, PingResult &r) {
  /* a negative sensor depth is traced from the surface and shifted back */
  r.sensordepth = ping.heave + ping.sensordepth;
  double shift = 0.0;
  if (r.sensordepth < 0.0) {
    shift = r.sensordepth;
    r.sensordepth = 0.0;
  }
  r.bath_min = 10000.0;
  r.bath_max = 0.0;
  r.nbeams = 0;
  r.traced = r.failed = r.at_bottom = 0;

  double sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0;
  int ns = 0;
  int error = MB_ERROR_NO_ERROR;
  for (int i = 0; i < ping.beams_bath; i++) {
    r.used[i] = 0;
    if (!mb_beam_ok(ping.beamflag[i]))
      continue;
    r.nbeams = i + 1;
    r.traced++;
    double ttime;
    int ray_stat;
    if (mb_rt(0, model, r.sensordepth, ping.angles[i], 0.5 * ping.ttimes[i], anglemode, ping.ssv, ping.angles_null[i], 0, nullptr,
              nullptr, nullptr, nullptr, &r.acrosstrack[i], &r.depth[i], &ttime, &ray_stat, &error) != MB_SUCCESS) {
      error = MB_ERROR_NO_ERROR;
      r.failed++;
      continue;
    }
    if (ray_stat == RAY_OUT_BOTTOM)
      r.at_bottom++;
    r.used[i] = 1;
    r.acrosstrack[i] *= cos(DTR * ping.angles_forward[i]);
    r.depth[i] += shift;
    r.bath_min = std::min(r.bath_min, r.depth[i]);
    r.bath_max = std::max(r.bath_max, r.depth[i]);
    sx += r.acrosstrack[i];
    sy += r.depth[i];
    sxx += r.acrosstrack[i] * r.acrosstrack[i];
    sxy += r.acrosstrack[i] * r.depth[i];
    ns++;
  }

  /* residuals to the straight line through the ping; two points at least. When all
     beams have the same acrosstrack the line is flat, at the mean depth. */
  if (ns < 2) {
    std::fill(r.used.begin(), r.used.begin() + ping.beams_bath, 0);
    return;
  }
  const double delta = ns * sxx - sx * sx;
  const double a = delta != 0.0 ? (sxx * sy - sx * sxy) / delta : sy / ns;
  const double b = delta != 0.0 ? (ns * sxy - sx * sy) / delta : 0.0;
  for (int i = 0; i < ping.beams_bath; i++)
    if (r.used[i])
      r.residual[i] = r.depth[i] - (a + b * r.acrosstrack[i]);
}

/* the rays drawn in the raypath plot: those of the first ping with good beams */
void trace_raypaths(void *model, const SwathFile &swath, int anglemode, int nnodes, Residuals &out) {
  const int nmax = 100 * nnodes;
  std::vector<double> x(nmax), z(nmax), t(nmax);
  int error = MB_ERROR_NO_ERROR;
  out.raypaths.assign(swath.beams_bath, RayPath());
  for (const Ping &ping : swath.pings) {
    double sensordepth = ping.heave + ping.sensordepth;
    out.ray_depth_min = std::min(out.ray_depth_min, sensordepth);
    double shift = 0.0;
    if (sensordepth < 0.0) {
      shift = sensordepth;
      sensordepth = 0.0;
    }
    int ns = 0;
    for (int i = 0; i < ping.beams_bath; i++) {
      if (!mb_beam_ok(ping.beamflag[i]))
        continue;
      int nplot = 0, ray_stat;
      double xx, zz, ttime;
      if (mb_rt(0, model, sensordepth, ping.angles[i], 0.5 * ping.ttimes[i], anglemode, ping.ssv, ping.angles_null[i], nmax, &nplot,
                x.data(), z.data(), t.data(), &xx, &zz, &ttime, &ray_stat, &error) != MB_SUCCESS) {
        error = MB_ERROR_NO_ERROR;
        continue;
      }
      const double factor = cos(DTR * ping.angles_forward[i]);
      RayPath &ray = out.raypaths[i];
      ray.x.resize(nplot);
      ray.z.assign(z.begin(), z.begin() + nplot);
      for (int j = 0; j < nplot; j++)
        ray.x[j] = factor * x[j];
      out.ray_x_max = std::max(out.ray_x_max, fabs(factor * xx));
      out.ray_depth_max = std::max(out.ray_depth_max, zz + shift);
      ns++;
    }
    if (ns > 0)
      break;
  }
}

}  // namespace

bool read_profile(const std::string &path, ProfileSource source, Profile &profile, std::string &error) {
  FILE *fp = fopen(path.c_str(), "r");
  if (fp == nullptr) {
    error = "Unable to open velocity profile file " + path;
    return false;
  }
  Profile p;
  p.name = path;
  p.source = source;
  mb_path buffer;
  while (fgets(buffer, sizeof(buffer), fp) == buffer) {
    double depth, velocity;
    if (buffer[0] != '#' && sscanf(buffer, "%lf %lf", &depth, &velocity) == 2) {
      p.depth.push_back(depth);
      p.velocity.push_back(velocity);
    }
  }
  fclose(fp);
  if (p.depth.empty()) {
    error = "No depth and velocity values found in " + path;
    return false;
  }
  profile = std::move(p);
  return true;
}

Profile default_profile(double bath_min, double bath_max) {
  Profile p;
  p.name = "new";
  p.source = ProfileSource::Default;
  p.depth.assign(DEPTH_EDIT, DEPTH_EDIT + NUM_EDIT_START);
  p.velocity.assign(NUM_EDIT_START, VELOCITY_EDIT);
  if (bath_max > bath_min && bath_max < 2000.0) {
    const double dz = 1.25 * bath_max / (NUM_EDIT_START - 2);
    for (int i = 0; i < NUM_EDIT_START - 1; i++)
      p.depth[i] = i * dz;
  }
  return p;
}

bool write_profile(const std::string &path, const Profile &profile, const std::string &swath_path, std::string &error) {
  FILE *fp = fopen(path.c_str(), "w");
  if (fp == nullptr) {
    error = "Unable to open velocity profile file " + path + " for writing";
    return false;
  }
  write_header(fp, "Water Sound Velocity Profile (SVP)");
  if (!swath_path.empty())
    fprintf(fp, "## Swath File: %s\n", swath_path.c_str());
  fprintf(fp, "## Number of SVP Points: %d\n", profile.size());
  for (int i = 0; i < profile.size(); i++)
    fprintf(fp, "%f %f\n", profile.depth[i], profile.velocity[i]);
  fclose(fp);
  return true;
}

void ray_model_nodes(const Profile &profile, std::vector<double> &depth, std::vector<double> &velocity) {
  depth.clear();
  velocity.clear();
  for (int i = 0; i < profile.size(); i++) {
    if (depth.empty()) {
      if (profile.depth[i] > 0.0) {
        depth.push_back(0.0);
        velocity.push_back(profile.velocity[i]);
      }
      depth.push_back(std::max(profile.depth[i], 0.0));
      velocity.push_back(profile.velocity[i]);
    }
    else if (profile.depth[i] > depth.back()) {
      depth.push_back(profile.depth[i]);
      velocity.push_back(profile.velocity[i]);
    }
  }
  if (!depth.empty() && depth.back() < MODEL_BOTTOM) {
    depth.push_back(MODEL_BOTTOM);
    velocity.push_back(velocity.back());
  }
}

bool levitus_profile(double lon, double lat, Profile &profile, std::string &error) {
  std::error_code ec;
  const fs::path dir = fs::temp_directory_path(ec);
  if (ec) {
    error = "No temporary directory for the output of mblevitus: " + ec.message();
    return false;
  }
  char name[64];
  snprintf(name, sizeof(name), "mbvelocitytool-qt-%08x.svp", (unsigned int)std::random_device{}());
  const std::string file = (dir / name).string();

  char command[2 * MB_PATH_MAXLINE];
  snprintf(command, sizeof(command), "mblevitus -R%f/%f \"-O%s\"", lon, lat, file.c_str());

  /* mblevitus reports on stdout: read it here so that it does not mix with our output */
#ifdef _WIN32
  FILE *pipe = _popen(command, "r");
#else
  FILE *pipe = popen(command, "r");
#endif
  if (pipe == nullptr) {
    error = "Unable to run mblevitus";
    return false;
  }
  char line[256];
  while (fgets(line, sizeof(line), pipe) != nullptr) {
  }
#ifdef _WIN32
  _pclose(pipe);
#else
  pclose(pipe);
#endif

  const bool ok = read_profile(file, ProfileSource::Levitus, profile, error);
  fs::remove(file, ec);
  if (!ok) {
    error = "mblevitus gave no profile at this position";
    return false;
  }
  char label[64];
  snprintf(label, sizeof(label), "Levitus %.4f %.4f", lon, lat);
  profile.name = label;
  return true;
}

bool same_nodes(const Profile &a, const Profile &b, double tolerance) {
  if (a.size() != b.size())
    return false;
  for (int i = 0; i < a.size(); i++)
    if (std::abs(a.depth[i] - b.depth[i]) > tolerance || std::abs(a.velocity[i] - b.velocity[i]) > tolerance)
      return false;
  return true;
}

Profile simplify_profile(const Profile &profile, double tolerance) {
  const int n = profile.size();
  if (n <= 2)
    return profile;
  std::vector<char> keep(n, 0);
  keep[0] = keep[n - 1] = 1;
  std::vector<std::pair<int, int>> spans = {{0, n - 1}};
  while (!spans.empty()) {
    const auto [first, last] = spans.back();
    spans.pop_back();
    const double dz = profile.depth[last] - profile.depth[first];
    int worst = -1;
    double worst_error = tolerance;
    for (int i = first + 1; i < last; i++) {
      const double t = dz > 0.0 ? (profile.depth[i] - profile.depth[first]) / dz : 0.0;
      const double line = profile.velocity[first] + t * (profile.velocity[last] - profile.velocity[first]);
      const double error = std::abs(profile.velocity[i] - line);
      if (error > worst_error) {
        worst_error = error;
        worst = i;
      }
    }
    if (worst >= 0) {
      keep[worst] = 1;
      spans.push_back({first, worst});
      spans.push_back({worst, last});
    }
  }
  Profile out = profile;
  out.depth.clear();
  out.velocity.clear();
  for (int i = 0; i < n; i++)
    if (keep[i]) {
      out.depth.push_back(profile.depth[i]);
      out.velocity.push_back(profile.velocity[i]);
    }
  return out;
}

std::vector<Profile> listed_profiles(const std::string &swath_path) {
  std::vector<Profile> profiles;
  for (int count = 0;; count++) {
    char suffix[32];
    snprintf(suffix, sizeof(suffix), "_%3.3d.svp", count);
    const std::string file = swath_path + suffix;
    std::error_code ec;
    if (fs::exists(file, ec) && !fs::is_directory(file, ec)) {
      Profile p;
      std::string error;
      if (read_profile(file, ProfileSource::SwathListing, p, error))
        profiles.push_back(std::move(p));
    }
    else if (count > 0) {
      break;
    }
  }
  return profiles;
}

std::vector<Profile> swath_display_profiles(const SwathFile &swath, std::string &levitus_error) {
  std::vector<Profile> profiles;
  if (swath.first_lon != 0.0 || swath.first_lat != 0.0) {
    Profile levitus;
    if (levitus_profile(swath.first_lon, swath.first_lat, levitus, levitus_error))
      profiles.push_back(std::move(levitus));
  }
  profiles.insert(profiles.end(), swath.profiles.begin(), swath.profiles.end());
  for (Profile &listed : listed_profiles(swath.path)) {
    bool copy = false;
    for (const Profile &stored : swath.profiles)
      copy = copy || same_nodes(listed, stored, LISTED_TOLERANCE);
    if (!copy)
      profiles.push_back(std::move(listed));
  }
  return profiles;
}

bool identify_format(const std::string &path, int &format) {
  mb_path file;
  std::string error;
  if (!to_mb_path(path, file, error))
    return false;
  int found = 0, mb_error_code = MB_ERROR_NO_ERROR;
  if (mb_get_format(0, file, nullptr, &found, &mb_error_code) != MB_SUCCESS || found <= 0)
    return false;
  format = found;
  return true;
}

bool load_swath(const std::string &path, int format, const int btime_i[7], const int etime_i[7], int verbose, SwathFile &swath,
                std::string &warning, std::string &error) {
  mb_path file;
  if (!to_mb_path(path, file, error))
    return false;

  int mb_error_code = MB_ERROR_NO_ERROR;
  if (format == 0)
    mb_get_format(verbose, file, nullptr, &format, &mb_error_code);
  mb_error_code = MB_ERROR_NO_ERROR;
  bool variable_beams, traveltime, beam_flagging;
  if (mb_format_flags(verbose, &format, &variable_beams, &traveltime, &beam_flagging, &mb_error_code) != MB_SUCCESS) {
    error = "Format id " + std::to_string(format) + " does not correspond to a supported format";
    return false;
  }
  warning.clear();
  if (!traveltime)
    warning = "Format " + std::to_string(format) +
              " does not include travel times: travel times and angles estimated assuming 1500 m/s sound speed";

  /* the settings of mbvelocitytool: all of the file, without any limit of position or speed */
  int fformat, pings, lonflip, unused_b[7], unused_e[7];
  double bounds[4], speedmin, timegap;
  mb_defaults(verbose, &fformat, &pings, &lonflip, bounds, unused_b, unused_e, &speedmin, &timegap);
  pings = 1;
  lonflip = 0;
  bounds[0] = -360.0;
  bounds[1] = 360.0;
  bounds[2] = -90.0;
  bounds[3] = 90.0;
  speedmin = 0.0;
  timegap = 1000000000.0;
  int btime[7], etime[7];
  memcpy(btime, btime_i, sizeof(btime));
  memcpy(etime, etime_i, sizeof(etime));

  void *mbio_ptr = nullptr;
  double btime_d, etime_d;
  int beams_bath, beams_amp, pixels_ss;
  mb_error_code = MB_ERROR_NO_ERROR;
  if (mb_read_init(verbose, file, format, pings, lonflip, bounds, btime, etime, speedmin, timegap, &mbio_ptr, &btime_d, &etime_d,
                   &beams_bath, &beams_amp, &pixels_ss, &mb_error_code) != MB_SUCCESS) {
    char *message = nullptr;
    mb_error(verbose, mb_error_code, &message);
    error = "Unable to open swath file " + path + ": " + (message != nullptr ? message : "");
    return false;
  }

  char *beamflag = nullptr;
  double *bath = nullptr, *amp = nullptr, *bathacrosstrack = nullptr, *bathalongtrack = nullptr;
  double *ss = nullptr, *ssacrosstrack = nullptr, *ssalongtrack = nullptr;
  mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &mb_error_code);
  if (mb_error_code == MB_ERROR_NO_ERROR)
    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &mb_error_code);
  if (mb_error_code == MB_ERROR_NO_ERROR)
    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &mb_error_code);
  if (mb_error_code == MB_ERROR_NO_ERROR)
    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &mb_error_code);
  if (mb_error_code == MB_ERROR_NO_ERROR)
    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &mb_error_code);
  if (mb_error_code == MB_ERROR_NO_ERROR)
    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &mb_error_code);
  if (mb_error_code == MB_ERROR_NO_ERROR)
    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &mb_error_code);
  if (mb_error_code == MB_ERROR_NO_ERROR)
    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &mb_error_code);
  if (mb_error_code != MB_ERROR_NO_ERROR) {
    error = "Unable to allocate memory to read " + path;
    int close_error = MB_ERROR_NO_ERROR;
    mb_close(verbose, &mbio_ptr, &close_error);
    return false;
  }

  struct mb_esf_struct esf;
  memset(&esf, 0, sizeof(esf));
  mb_path esffile = "";
  mb_esf_load(verbose, PROGRAM_NAME, file, true, false, esffile, &esf, &mb_error_code);
  mb_error_code = MB_ERROR_NO_ERROR;

  SwathFile out;
  out.path = path;
  out.format = format;
  out.traveltime = traveltime;
  out.beams_bath = beams_bath;
  out.esf_edits = esf.nedit;
  out.bath_min = 10000.0;
  out.bath_max = 0.0;
  double ssv_start = 0.0;

  int platform_source, nav_source, sensordepth_source, heading_source, attitude_source, svp_source = MB_DATA_NONE;
  mb_format_source(verbose, &format, &platform_source, &nav_source, &sensordepth_source, &heading_source, &attitude_source,
                   &svp_source, &mb_error_code);
  mb_error_code = MB_ERROR_NO_ERROR;
  std::vector<double> svp_depth(SVP_BUFFER), svp_velocity(SVP_BUFFER);
  double last_time_d = 0.0, last_lon = 0.0, last_lat = 0.0;

  std::vector<double> ttimes, angles, angles_forward, angles_null, heave, alongtrack_offset;
  void *store_ptr = nullptr;
  char comment[MB_COMMENT_MAXLINE];
  int time_i[7], kind, nbath, namp, nss, nbeams;
  double time_d, navlon, navlat, speed, heading, distance, altitude, sensordepth;
  while (mb_error_code <= MB_ERROR_NO_ERROR && (int)out.pings.size() < MAX_PINGS) {
    int status = mb_get_all(verbose, mbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance,
                            &altitude, &sensordepth, &nbath, &namp, &nss, beamflag, bath, amp, bathacrosstrack, bathalongtrack, ss,
                            ssacrosstrack, ssalongtrack, comment, &mb_error_code);

    /* two formats keep the profile in the survey records, so the record may still be a ping after this */
    if (mb_error_code <= MB_ERROR_NO_ERROR && kind == svp_source && svp_source != MB_DATA_NONE) {
      int svp_kind = kind, svp_error = MB_ERROR_NO_ERROR, nsvp = 0;
      if (mb_extract_svp(verbose, mbio_ptr, store_ptr, &svp_kind, &nsvp, svp_depth.data(), svp_velocity.data(), &svp_error) == MB_SUCCESS &&
          nsvp >= 2) {
        Profile p;
        p.source = ProfileSource::SwathRecord;
        p.depth.assign(svp_depth.begin(), svp_depth.begin() + nsvp);
        p.velocity.assign(svp_velocity.begin(), svp_velocity.begin() + nsvp);
        p.time_d = last_time_d;
        p.lon = last_lon;
        p.lat = last_lat;
        bool repeat = false;
        for (const Profile &stored : out.profiles)
          repeat = repeat || same_nodes(p, stored, 0.0);
        if (!repeat)
          out.profiles.push_back(std::move(p));
      }
    }

    if (mb_error_code <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA &&
        (mb_error_code == MB_ERROR_NO_ERROR || mb_error_code == MB_ERROR_TIME_GAP || mb_error_code == MB_ERROR_OUT_BOUNDS ||
         mb_error_code == MB_ERROR_SPEED_TOO_SMALL)) {
      status = MB_SUCCESS;
      mb_error_code = MB_ERROR_NO_ERROR;
    }
    else if (mb_error_code <= MB_ERROR_NO_ERROR) {
      status = MB_FAILURE;
      mb_error_code = MB_ERROR_OTHER;
    }
    if (status != MB_SUCCESS)
      continue;

    if (esf.nedit > 0 && mb_esf_apply(verbose, &esf, time_d, 0, nbath, beamflag, &mb_error_code) != MB_SUCCESS)
      continue;

    /* mb_ttimes() fills as many beams as the format has, which may be more than this ping */
    const int capacity = std::max(nbath, ((struct mb_io_struct *)mbio_ptr)->beams_bath_max);
    ttimes.assign(capacity, 0.0);
    angles.assign(capacity, 0.0);
    angles_forward.assign(capacity, 0.0);
    angles_null.assign(capacity, 0.0);
    heave.assign(capacity, 0.0);
    alongtrack_offset.assign(capacity, 0.0);

    Ping ping;
    ping.beams_bath = nbath;
    ping.time_d = time_d;
    if (traveltime) {
      if (mb_ttimes(verbose, mbio_ptr, store_ptr, &kind, &nbeams, ttimes.data(), angles.data(), angles_forward.data(),
                    angles_null.data(), heave.data(), alongtrack_offset.data(), &ping.sensordepth, &ping.ssv,
                    &mb_error_code) != MB_SUCCESS)
        continue;
    }
    else {
      ping.sensordepth = sensordepth;
      ping.ssv = 1500.0;
      for (int i = 0; i < nbath; i++)
        if (mb_beam_ok(beamflag[i])) {
          const double zz = bath[i] - sensordepth;
          const double rr = sqrt(zz * zz + bathacrosstrack[i] * bathacrosstrack[i] + bathalongtrack[i] * bathalongtrack[i]);
          ttimes[i] = rr / 750.0;
          mb_xyz_to_takeoff(verbose, bathacrosstrack[i], bathalongtrack[i], zz, &angles[i], &angles_forward[i], &mb_error_code);
        }
    }

    bool found = false;
    for (int i = 0; i < nbath; i++)
      if (mb_beam_ok(beamflag[i])) {
        if (!found) {
          ping.heave = heave[i];
          found = true;
        }
        const double depth = 750 * ttimes[i] * cos(DTR * angles[i]) + ping.sensordepth + heave[i];
        out.bath_min = std::min(out.bath_min, depth);
        out.bath_max = std::max(out.bath_max, depth);
      }

    ping.beamflag.assign(beamflag, beamflag + nbath);
    ping.ttimes.assign(ttimes.begin(), ttimes.begin() + nbath);
    ping.angles.assign(angles.begin(), angles.begin() + nbath);
    ping.angles_forward.assign(angles_forward.begin(), angles_forward.begin() + nbath);
    ping.angles_null.assign(angles_null.begin(), angles_null.begin() + nbath);
    out.beams_bath = std::max(out.beams_bath, nbath);
    /* a profile recorded before the first ping takes the time and position of that ping */
    last_time_d = time_d;
    last_lon = navlon;
    last_lat = navlat;
    for (Profile &p : out.profiles)
      if (p.time_d == 0.0) {
        p.time_d = time_d;
        p.lon = navlon;
        p.lat = navlat;
      }

    if (out.first_lon == 0.0 && out.first_lat == 0.0) {
      out.first_lon = navlon;
      out.first_lat = navlat;
    }
    if (ping.ssv > 0.0 && ssv_start == 0.0)
      ssv_start = ping.ssv;
    out.pings.push_back(std::move(ping));
  }
  out.truncated = (int)out.pings.size() >= MAX_PINGS && mb_error_code <= MB_ERROR_NO_ERROR;

  mb_close(verbose, &mbio_ptr, &mb_error_code);
  if (esf.edit != nullptr || esf.esffp != nullptr)
    mb_esf_close(verbose, &esf, &mb_error_code);

  if (out.pings.empty()) {
    error = "No data were read from " + path + ": the MB-System format id may be incorrect";
    return false;
  }

  /* named once the records without their own time have one */
  for (size_t i = 0; i < out.profiles.size(); i++) {
    int time_i[7] = {0, 0, 0, 0, 0, 0, 0};
    char label[64];
    mb_get_date(verbose, out.profiles[i].time_d, time_i);
    snprintf(label, sizeof(label), "Swath SVP %d: %4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", (int)i + 1, time_i[0], time_i[1], time_i[2],
             time_i[3], time_i[4], time_i[5]);
    out.profiles[i].name = label;
  }

  /* a ping without a surface sound speed uses that of the previous ping, the first ones the first available */
  double ssv = ssv_start > 0.0 ? ssv_start : 1500.0;
  for (Ping &ping : out.pings) {
    if (ping.ssv <= 0.0)
      ping.ssv = ssv;
    else
      ssv = ping.ssv;
  }

  swath = std::move(out);
  return true;
}

bool process(const SwathFile &swath, const Profile &profile, int anglemode, int nthreads, Residuals &residuals,
             std::string &error) {
  if (profile.size() <= 0) {
    error = "No edit velocity profile available";
    return false;
  }
  if (swath.pings.empty()) {
    error = "No swath sonar data available";
    return false;
  }

  std::vector<double> depth, velocity;
  ray_model_nodes(profile, depth, velocity);
  const int npings = (int)swath.pings.size();
  if (nthreads <= 0)
    nthreads = (int)std::thread::hardware_concurrency();
  nthreads = std::max(1, std::min(nthreads, npings));

  std::vector<void *> models(nthreads, nullptr);
  auto free_models = [&models]() {
    int rt_error = MB_ERROR_NO_ERROR;
    for (void *&model : models)
      if (model != nullptr)
        mb_rt_deall(0, &model, &rt_error);
  };
  for (void *&model : models) {
    int rt_error = MB_ERROR_NO_ERROR;
    if (mb_rt_init(0, (int)depth.size(), depth.data(), velocity.data(), &model, &rt_error) != MB_SUCCESS) {
      error = "Unable to set up the ray tracing model";
      free_models();
      return false;
    }
  }

  Residuals out;
  out.angle.assign(swath.beams_bath, 0.0);
  out.acrosstrack.assign(swath.beams_bath, 0.0);
  out.altitude.assign(swath.beams_bath, 0.0);
  out.residual.assign(swath.beams_bath, 0.0);
  out.sigma.assign(swath.beams_bath, 0.0);
  out.count.assign(swath.beams_bath, 0);

  trace_raypaths(models[0], swath, anglemode, (int)depth.size(), out);

  /* The pings are traced in batches and added in ping order, as floating point
     sums depend on their order: this way the result is the same for any number of threads. */
  const int batch_size = BATCH_PINGS_PER_THREAD * nthreads;
  std::vector<PingResult> batch(std::min(batch_size, npings));
  for (PingResult &r : batch) {
    r.used.resize(swath.beams_bath);
    r.acrosstrack.resize(swath.beams_bath);
    r.depth.resize(swath.beams_bath);
    r.residual.resize(swath.beams_bath);
  }

  for (int start = 0; start < npings; start += batch_size) {
    const int count = std::min(batch_size, npings - start);
    std::atomic<int> next(0);
    auto work = [&](int t) {
      for (int j = next++; j < count; j = next++)
        trace_ping(models[t], swath.pings[start + j], anglemode, batch[j]);
    };
    std::vector<std::thread> workers;
    for (int t = 1; t < nthreads; t++)
      workers.emplace_back(work, t);
    work(0);
    for (std::thread &worker : workers)
      worker.join();

    for (int j = 0; j < count; j++) {
      const Ping &ping = swath.pings[start + j];
      const PingResult &r = batch[j];
      out.nbeams = std::max(out.nbeams, r.nbeams);
      out.bath_min = std::min(out.bath_min, r.bath_min);
      out.bath_max = std::max(out.bath_max, r.bath_max);
      out.rays_traced += r.traced;
      out.rays_failed += r.failed;
      out.rays_at_bottom += r.at_bottom;
      for (int i = 0; i < ping.beams_bath; i++)
        if (r.used[i]) {
          out.angle[i] += ping.angles[i];
          out.altitude[i] += r.depth[i] - r.sensordepth;
          out.acrosstrack[i] += r.acrosstrack[i];
          out.residual[i] += r.residual[i];
          out.sigma[i] += r.residual[i] * r.residual[i];
          out.count[i]++;
        }
    }
  }
  free_models();

  out.beam_first = out.nbeams;
  out.beam_last = -1;
  for (int i = 0; i < out.nbeams; i++)
    if (out.count[i] > 0) {
      out.angle[i] /= out.count[i];
      out.acrosstrack[i] /= out.count[i];
      out.altitude[i] /= out.count[i];
      out.residual[i] /= out.count[i];
      out.sigma[i] = sqrt(std::max(out.sigma[i] / out.count[i] - out.residual[i] * out.residual[i], 0.0));
      out.beam_first = std::min(out.beam_first, i);
      out.beam_last = std::max(out.beam_last, i);
    }

  residuals = std::move(out);
  return true;
}

bool save_swath_profile(const SwathFile &swath, const Profile &profile, int anglemode, std::string &saved, std::string &error) {
  if (profile.size() <= 2 || swath.pings.empty()) {
    error = "Saving needs an edit profile with more than two nodes and swath data";
    return false;
  }
  saved = swath.path + ".svp";
  mb_path file, svpfile, oldfile;
  if (!to_mb_path(swath.path, file, error) || !to_mb_path(saved, svpfile, error))
    return false;
  if (!write_profile(saved, profile, swath.path, error))
    return false;

  /* set the par file for use with mbprocess */
  int oldmode, oldanglemode, corrected = 0, mb_error_code = MB_ERROR_NO_ERROR;
  mb_pr_get_svp(0, file, &oldmode, oldfile, &oldanglemode, &corrected, &mb_error_code);
  if (mb_pr_update_svp(0, file, true, svpfile, anglemode, corrected, &mb_error_code) != MB_SUCCESS) {
    error = "Unable to update the parameter file of " + swath.path;
    return false;
  }
  return true;
}

bool save_residuals(const SwathFile &swath, const Profile &profile, const Residuals &residuals, std::string &saved,
                    std::string &error) {
  if (profile.size() <= 2 || swath.pings.empty()) {
    error = "Saving needs an edit profile with more than two nodes and swath data";
    return false;
  }
  saved = swath.path + ".sbo";
  const std::string sbao = swath.path + ".sbao";
  mb_path file, sbofile, oldfile;
  if (!to_mb_path(swath.path, file, error) || !to_mb_path(saved, sbofile, error))
    return false;

  FILE *fp = fopen(saved.c_str(), "w");
  if (fp == nullptr) {
    error = "Unable to open static beam offset file " + saved + " for writing";
    return false;
  }
  write_header(fp, "Static Beam Offset (SBO)");
  fprintf(fp, "## Swath File: %s\n", swath.path.c_str());
  fprintf(fp, "## Number of Static Beam Offset Points: %d\n", residuals.nbeams);
  for (int i = 0; i < residuals.nbeams; i++)
    fprintf(fp, " %4d  %9.3f  %9.3f\n", i, residuals.residual[i], residuals.sigma[i]);
  fclose(fp);

  /* set the par file for use with mbprocess */
  int oldmode, mb_error_code = MB_ERROR_NO_ERROR;
  mb_pr_get_static(0, file, &oldmode, oldfile, &mb_error_code);
  if (mb_pr_update_static(0, file, true, sbofile, &mb_error_code) != MB_SUCCESS) {
    error = "Unable to update the parameter file of " + swath.path;
    return false;
  }

  fp = fopen(sbao.c_str(), "w");
  if (fp == nullptr) {
    error = "Unable to open static beam angle offset file " + sbao + " for writing";
    return false;
  }
  write_header(fp, "Static Beam Angle Offset (SBAO)");
  fprintf(fp, "## Swath File: %s\n", swath.path.c_str());
  fprintf(fp, "## Number of Static Beam Angle Offset Points: %d\n", residuals.nbeams);
  for (int i = 0; i < residuals.nbeams; i++) {
    const double across = residuals.acrosstrack[i], altitude = residuals.altitude[i], res = residuals.residual[i];
    const double rr = sqrt(across * across + (altitude + res) * (altitude + res));
    const double xx = copysign(sqrt(std::max(rr * rr - altitude * altitude, 0.0)), across);
    const double dangle = rr > 0.0 ? asin(across / rr) - asin(xx / rr) : 0.0;
    fprintf(fp, " %4d  %9.3f  %9.3f\n", i, residuals.angle[i], dangle);
  }
  fclose(fp);
  saved += " " + sbao;
  return true;
}

}  // namespace mbvt

/*--------------------------------------------------------------------
 *    The MB-system:  mbvt_core.h  2/11/2026
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
/*
 * Computing layer of mbvelocitytool-qt (no Qt): the sound velocity profiles, the
 * swath data read through MBIO, the ray tracing with the per ping residuals of
 * Reprocess, and the output files. The functions keep no global state, so they
 * can run outside the thread of a window.
 *
 * Conventions as in MBIO: time_d seconds since 1970, depths positive down,
 * acrosstrack positive to starboard, angles in degrees, velocities in m/s.
 */

#ifndef MBVELOCITYTOOL_QT_MBVT_CORE_H
#define MBVELOCITYTOOL_QT_MBVT_CORE_H

#include <string>
#include <vector>

namespace mbvt {

constexpr char PROGRAM_NAME[] = "MBVELOCITYTOOL-QT";
constexpr int MAX_PINGS = 25000;  // pings kept in memory, as MBVT_BUFFER_SIZE of mbvelocitytool

enum class ProfileSource { Default, File, Levitus, SwathRecord, SwathListing, Edited };

struct Profile {
  std::string name;
  ProfileSource source = ProfileSource::File;
  std::vector<double> depth;
  std::vector<double> velocity;
  double time_d = 0.0;  // of a SwathRecord: the survey ping before the record, or after it if there is none; else 0
  double lon = 0.0;     // of the same ping, both 0 when there is none
  double lat = 0.0;
  bool hidden = false;   // a display profile not drawn for now
  bool unsaved = false;  // a display profile with edits that are not in any file
  int size() const { return (int)depth.size(); }
};

/* One survey ping, with only what Reprocess needs */
struct Ping {
  int beams_bath = 0;
  double time_d = 0.0;
  double sensordepth = 0.0;
  double heave = 0.0;  // heave of the first good beam, used for the whole ping
  double ssv = 0.0;    // surface sound speed, gaps filled from the previous ping
  std::vector<char> beamflag;
  std::vector<double> ttimes;          // two way travel time, s
  std::vector<double> angles;          // takeoff angle from vertical
  std::vector<double> angles_forward;  // takeoff azimuth
  std::vector<double> angles_null;     // array normal angle
};

struct SwathFile {
  std::string path;
  int format = 0;
  bool traveltime = false;  // false: travel times and angles estimated for 1500 m/s
  bool truncated = false;   // MAX_PINGS reached before the end of the file
  int beams_bath = 0;       // largest number of beams
  int esf_edits = 0;        // edits loaded from the *.esf file
  double first_lon = 0.0;   // first navigation, both 0 when there is none
  double first_lat = 0.0;
  double bath_min = 0.0;  // approximate depth range for 1500 m/s
  double bath_max = 0.0;
  std::vector<Ping> pings;
  std::vector<Profile> profiles;  // the different sound velocity profiles stored in the file
};

struct RayPath {
  std::vector<double> x;  // acrosstrack, m
  std::vector<double> z;  // depth, m
};

/* Result of one Reprocess. Beam arrays have SwathFile::beams_bath entries. */
struct Residuals {
  int nbeams = 0;  // largest beam number traced + 1
  int beam_first = 0;
  int beam_last = -1;
  std::vector<double> angle;        // mean takeoff angle
  std::vector<double> acrosstrack;  // mean acrosstrack, m
  std::vector<double> altitude;     // mean depth below the sonar, m
  std::vector<double> residual;     // mean depth minus the straight line fit of its ping, m
  std::vector<double> sigma;        // standard deviation of the residual, m
  std::vector<int> count;           // number of residuals
  double bath_min = 10000.0;
  double bath_max = 0.0;
  long rays_traced = 0;
  long rays_failed = 0;      // mb_rt() failed, e.g. the sonar is outside the profile
  long rays_at_bottom = 0;   // the ray reached the bottom of the profile before its travel time ended
  std::vector<RayPath> raypaths;  // per beam of the first ping with good beams
  double ray_x_max = 0.0;
  double ray_depth_min = 10000.0;
  double ray_depth_max = 0.0;
};

/* profiles; on failure the functions return false and set error */
bool read_profile(const std::string &path, ProfileSource source, Profile &profile, std::string &error);
Profile default_profile(double bath_min, double bath_max);
bool write_profile(const std::string &path, const Profile &profile, const std::string &swath_path, std::string &error);

/* the profile mb_rt() is given: starting at 0 m, depth increasing and reaching 12000 m, as mbprocess does */
void ray_model_nodes(const Profile &profile, std::vector<double> &depth, std::vector<double> &velocity);

/* true if a and b have the same nodes within tolerance */
bool same_nodes(const Profile &a, const Profile &b, double tolerance);

/* the nodes of a profile needed to keep its velocity within tolerance (m/s) at every node left out (Douglas-Peucker in
   velocity against depth); the first and last nodes always stay */
Profile simplify_profile(const Profile &profile, double tolerance);

/* mean annual profile at a position from the program mblevitus */
bool levitus_profile(double lon, double lat, Profile &profile, std::string &error);

/* the <swath file>_NNN.svp files written by mbsvplist */
std::vector<Profile> listed_profiles(const std::string &swath_path);

/* the format from the file name; false when the name does not tell */
bool identify_format(const std::string &path, int &format);

/* format 0 is identified from the file name */
bool load_swath(const std::string &path, int format, const int btime_i[7], const int etime_i[7], int verbose, SwathFile &swath,
                std::string &warning, std::string &error);

/* the display profiles that come with a loaded file: the Levitus profile, the profiles stored in the file, and the
   <swath file>_NNN.svp files that are not just copies of those. A failure of mblevitus sets levitus_error. */
std::vector<Profile> swath_display_profiles(const SwathFile &swath, std::string &levitus_error);

/* anglemode is one of MBP_ANGLES_*; nthreads 0 uses all processors. The result does not depend on nthreads. */
bool process(const SwathFile &swath, const Profile &profile, int anglemode, int nthreads, Residuals &residuals,
             std::string &error);

/* <swath file>.svp and the *.par file for mbprocess */
bool save_swath_profile(const SwathFile &swath, const Profile &profile, int anglemode, std::string &saved, std::string &error);

/* <swath file>.sbo and .sbao and the *.par file for mbprocess */
bool save_residuals(const SwathFile &swath, const Profile &profile, const Residuals &residuals, std::string &saved,
                    std::string &error);

}  // namespace mbvt

#endif

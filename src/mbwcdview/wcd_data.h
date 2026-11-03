/*--------------------------------------------------------------------
 *    The MB-system:  wcd_data.h  1/11/2026
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
 * Data layer of mbwcdview (no Qt): reads water column and survey data through
 * MBIO, keeps all water column pings of a file in memory and pairs a Kongsberg
 * *.kmall or *.all (navigation, attitude, bathymetry) with its *.kmwcd or *.wcd
 * (water column).
 *
 * Conventions as in MBIO: time_d seconds since 1970, depths positive down,
 * acrosstrack positive to starboard, angles in degrees.
 */

#ifndef MBWCDVIEW_WCD_DATA_H
#define MBWCDVIEW_WCD_DATA_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace wcd {

/* Geometry of one water column ping (all receive fans), as given by mb_extract_wc() */
struct WcPing {
  double time_d = 0.0;
  unsigned int ping_number = 0;
  std::vector<int> swath;              // swath index along track per beam (0 = aftmost)
  std::vector<double> angle_across;    // deg from vertical, positive starboard
  std::vector<double> angle_along;     // deg, positive forward
  std::vector<double> sample_interval; // s
  std::vector<double> soundspeed;      // m/s at the transducer
  std::vector<int> start_sample;       // sample number of the first amplitude
  std::vector<int> nsamples;           // amplitudes per beam
  std::vector<double> detect_sample;   // two-way bottom range in samples, 0 = none
  size_t amp_offset = 0;               // first amplitude of the ping in WaterColumnFile::amp
  int nbeams() const { return (int)swath.size(); }
  int nsamples_max() const;
};

/* Sound velocity profile */
struct Svp {
  double time_d = 0.0;
  std::vector<double> depth;
  std::vector<double> velocity;
};

/* Positions with times (position records or ping positions) */
struct NavSeries {
  std::vector<double> time_d, lon, lat;
  size_t size() const { return time_d.size(); }
};

using Progress = std::function<void(int npings, double fraction)>;

/* All water column pings of one file. Amplitudes are kept as int16 in 0.1 dB
   (exact for data in 0.5 dB steps) without padding. */
class WaterColumnFile {
 public:
  bool load(const std::string &path, int format, const Progress &progress, std::string *message);

  std::string path;
  int format = 0;
  std::vector<WcPing> pings;
  std::vector<int16_t> amp;     // 0.1 dB, beam by beam, ping by ping
  bool has_fill = false;        // lowest value is a fill value (e.g. Kongsberg -128 raw)
  double fill_value = 0.0;      // dB
  double amp_min = 0.0, amp_max = 0.0;  // dB, without the fill value
  double range_max_m = 0.0;     // longest slant range of any sample (straight rays)
  NavSeries nav;                // the file's own position records
  std::vector<Svp> svps;

  size_t size() const { return pings.size(); }
  std::vector<int> swaths(size_t i) const;
  /* amplitudes (dB) of the beams of ping i in swath s as rows of nsamples_max values;
     values beyond a beam's samples are NaN; beams lists the ping's beam indexes */
  void amplitudes(size_t i, int s, std::vector<int> &beams, int &nsamples_max, std::vector<float> &out) const;
};

/* The survey pings of a *.kmall or *.all */
struct SurveyPings {
  std::vector<double> time_d, navlon, navlat, speed, heading, sensordepth;
  std::vector<unsigned int> ping_number;
  std::vector<std::vector<char>> beamflag;
  std::vector<std::vector<double>> bath, bathacrosstrack;
  std::vector<std::vector<int>> detects;
  NavSeries nav;             // position records
  std::vector<Svp> svps;
  size_t size() const { return time_d.size(); }
};

bool load_survey(const std::string &path, int format, SurveyPings &survey, std::string *message);

/* MBIO format of a file name (0 if unknown); *.kmwcd gives 261, *.wcd 58 */
int get_format(const std::string &path);

/* The *.kmwcd of a *.kmall, the *.wcd of a *.all or the reverse: same line number,
   nearest file name time */
std::string find_companion(const std::string &path, double max_offset_s = 600.0);

/* A *.kmall and its *.kmwcd or a *.all and its *.wcd, either may be missing; kmall and
   kmwcd name the survey and water column files of either kind */
class Line {
 public:
  /* path: either file of a line; format 0 = from the file name (applies to path only) */
  bool load(const std::string &path, const Progress &progress, std::string *message, int format = 0);

  std::string kmall, kmwcd;
  bool has_wc = false, has_survey = false;
  WaterColumnFile wc;
  SurveyPings survey;

  int survey_index(size_t i) const;      // matching survey ping of water column ping i, or -1
  int matched() const;
  /* lon, lat, speed over ground (knots), course over ground (deg); NaN if unavailable */
  void nav_at(double time_d, double &lon, double &lat, double &sog_kn, double &cog) const;
  double heading_at(double time_d) const;  // NaN if unavailable
  const Svp *svp(double time_d) const;     // nullptr if none
  bool draft(size_t i, double &sensordepth) const;

 private:
  std::vector<int> match_;
};

}  // namespace wcd

#endif

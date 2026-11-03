/*--------------------------------------------------------------------
 *    The MB-system:  wcd_data.cc  1/11/2026
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

#include "wcd_data.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <limits>
#include <map>
#include <regex>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_status.h"

namespace wcd {

namespace {

constexpr double NaN = std::numeric_limits<double>::quiet_NaN();
constexpr int SVP_ALLOC = 10000;  // mb_extract_svp() has no array size argument

/* Sequential reading of one file with mb_get_all(); output arrays registered with
   MBIO so that they grow with the beam counts */
class MbioReader {
 public:
  ~MbioReader() { close(); }

  bool open(const std::string &path, int format, std::string *message) {
    std::vector<char> file(path.begin(), path.end());
    file.push_back('\0');
    double bounds[4] = {-360.0, 360.0, -90.0, 90.0};
    int btime_i[7] = {1962, 2, 21, 10, 30, 0, 0};
    int etime_i[7] = {2062, 2, 21, 10, 30, 0, 0};
    double btime_d, etime_d;
    int beams_bath, beams_amp, pixels_ss;
    int error = MB_ERROR_NO_ERROR;
    if (mb_read_init(0, file.data(), format, 1, 0, bounds, btime_i, etime_i, 0.0, 1.0e6, &mbio_, &btime_d, &etime_d,
                     &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
      mbio_ = nullptr;
      if (message != nullptr)
        *message = "cannot open " + path + ": " + error_message(error);
      return false;
    }
    const int types[8] = {MB_MEM_TYPE_BATHYMETRY, MB_MEM_TYPE_BATHYMETRY, MB_MEM_TYPE_AMPLITUDE,
                          MB_MEM_TYPE_BATHYMETRY, MB_MEM_TYPE_BATHYMETRY, MB_MEM_TYPE_SIDESCAN,
                          MB_MEM_TYPE_SIDESCAN,   MB_MEM_TYPE_SIDESCAN};
    void **handles[8] = {(void **)&beamflag, (void **)&bath, (void **)&amp, (void **)&bathacrosstrack,
                         (void **)&bathalongtrack, (void **)&ss, (void **)&ssacrosstrack, (void **)&ssalongtrack};
    const size_t sizes[8] = {sizeof(char), sizeof(double), sizeof(double), sizeof(double),
                             sizeof(double), sizeof(double), sizeof(double), sizeof(double)};
    for (int i = 0; i < 8; i++) {
      if (mb_register_array(0, mbio_, types[i], sizes[i], handles[i], &error) != MB_SUCCESS) {
        if (message != nullptr)
          *message = "cannot allocate arrays for " + path + ": " + error_message(error);
        close();
        return false;
      }
    }
    return true;
  }

  void close() {
    if (mbio_ != nullptr) {
      int error = MB_ERROR_NO_ERROR;
      mb_close(0, &mbio_, &error);
      mbio_ = nullptr;
    }
  }

  /* next record: false at end of file or on a fatal error (message set) */
  bool next(std::string *message) {
    error = MB_ERROR_NO_ERROR;
    status = mb_get_all(0, mbio_, &store, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance,
                        &altitude, &sensordepth, &nbath, &namp, &nss, beamflag, bath, amp, bathacrosstrack,
                        bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);
    if (error == MB_ERROR_EOF)
      return false;
    if (error > MB_ERROR_NO_ERROR) {
      if (message != nullptr)
        *message = "read error: " + error_message(error);
      return false;
    }
    return true;
  }

  bool ok() const { return status == MB_SUCCESS; }

  unsigned int ping_number() {
    unsigned int ping = 0;
    int err = MB_ERROR_NO_ERROR;
    mb_pingnumber(0, mbio_, &ping, &err);
    return ping;
  }

  bool extract_svp(Svp &svp) {
    svp.depth.assign(SVP_ALLOC, 0.0);
    svp.velocity.assign(SVP_ALLOC, 0.0);
    int nsvp = 0, svp_kind, err = MB_ERROR_NO_ERROR;
    if (mb_extract_svp(0, mbio_, store, &svp_kind, &nsvp, svp.depth.data(), svp.velocity.data(), &err) !=
            MB_SUCCESS || nsvp <= 0 || nsvp > SVP_ALLOC)
      return false;
    svp.depth.resize(nsvp);
    svp.velocity.resize(nsvp);
    svp.time_d = time_d;
    return true;
  }

  /* water column of the current record into ping (geometry) and amplitudes (dB) */
  bool extract_wc(WcPing &ping, std::vector<float> &amplitude, int &nsamples_alloc) {
    int wc_kind, nbeams = 0, nsamples_max = 0, err = MB_ERROR_NO_ERROR;
    if (mb_extract_wcdimensions(0, mbio_, store, &wc_kind, &nbeams, &nsamples_max, &err) != MB_SUCCESS)
      return false;
    ping.swath.resize(nbeams);
    ping.angle_across.resize(nbeams);
    ping.angle_along.resize(nbeams);
    ping.sample_interval.resize(nbeams);
    ping.soundspeed.resize(nbeams);
    ping.start_sample.resize(nbeams);
    ping.nsamples.resize(nbeams);
    ping.detect_sample.resize(nbeams);
    nsamples_alloc = std::max(nsamples_max, 1);
    amplitude.resize((size_t)nbeams * nsamples_alloc);
    int wc_time_i[7];
    if (mb_extract_wc(0, mbio_, store, nbeams, nsamples_alloc, &wc_kind, wc_time_i, &ping.time_d, &nbeams,
                      &nsamples_max, ping.swath.data(), ping.angle_across.data(), ping.angle_along.data(),
                      ping.sample_interval.data(), ping.soundspeed.data(), ping.start_sample.data(),
                      ping.nsamples.data(), ping.detect_sample.data(), amplitude.data(), &err) != MB_SUCCESS)
      return false;
    return true;
  }

  std::vector<int> detects() {
    std::vector<int> det(std::max(nbath, 1), MB_DETECT_UNKNOWN);
    int dkind, nb = 0, err = MB_ERROR_NO_ERROR;
    if (mb_detects(0, mbio_, store, &dkind, &nb, det.data(), &err) != MB_SUCCESS)
      nb = 0;
    det.resize(std::max(0, std::min(nb, nbath)));
    det.resize(nbath, MB_DETECT_UNKNOWN);
    return det;
  }

  static std::string error_message(int error) {
    char *msg = nullptr;
    mb_error(0, error, &msg);
    return msg != nullptr ? std::string(msg) : "MBIO error " + std::to_string(error);
  }

  void *mbio_ = nullptr;
  void *store = nullptr;
  int kind = MB_DATA_NONE, status = MB_FAILURE, error = MB_ERROR_NO_ERROR;
  int time_i[7];
  double time_d = 0.0, navlon = 0.0, navlat = 0.0, speed = 0.0, heading = 0.0, distance = 0.0, altitude = 0.0,
         sensordepth = 0.0;
  int nbath = 0, namp = 0, nss = 0;
  char *beamflag = nullptr;
  double *bath = nullptr, *amp = nullptr, *bathacrosstrack = nullptr, *bathalongtrack = nullptr;
  double *ss = nullptr, *ssacrosstrack = nullptr, *ssalongtrack = nullptr;
  char comment[MB_COMMENT_MAXLINE];
};

/* seconds since 1970 of a UTC calendar date (no timegm() on all platforms) */
double epoch_seconds(int year, int month, int day, int hour, int minute, int second) {
  const int y = year - (month <= 2 ? 1 : 0);
  const int era = (y >= 0 ? y : y - 399) / 400;
  const int yoe = y - era * 400;
  const int doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
  const int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  const double days = (double)era * 146097 + doe - 719468;
  return days * 86400.0 + hour * 3600.0 + minute * 60.0 + second;
}

/* line number and time from a Kongsberg file name nnnn_YYYYMMDD_HHMMSS... */
bool name_time(const std::string &path, std::string &line, double &time_d) {
  static const std::regex pattern("^(\\d{4})_(\\d{4})(\\d{2})(\\d{2})_(\\d{2})(\\d{2})(\\d{2})");
  std::smatch m;
  const std::string name = std::filesystem::path(path).filename().string();
  if (!std::regex_search(name, m, pattern))
    return false;
  line = m[1];
  time_d = epoch_seconds(std::stoi(m[2]), std::stoi(m[3]), std::stoi(m[4]), std::stoi(m[5]), std::stoi(m[6]),
                         std::stoi(m[7]));
  return true;
}

std::string lower_extension(const std::string &path) {
  std::string ext = std::filesystem::path(path).extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return (char)std::tolower(c); });
  return ext;
}

/* linear interpolation of y(t) at time; false outside [t0, tn] */
bool interp(const std::vector<double> &t, const std::vector<double> &y, double time, double &value, size_t &k) {
  if (t.size() < 2 || time < t.front() || time > t.back())
    return false;
  k = (size_t)(std::upper_bound(t.begin(), t.end(), time) - t.begin());
  k = std::min(std::max(k, (size_t)1), t.size() - 1);
  const double dt = t[k] - t[k - 1];
  const double f = dt > 0.0 ? (time - t[k - 1]) / dt : 0.0;
  value = y[k - 1] + f * (y[k] - y[k - 1]);
  return true;
}

}  // namespace

/*--------------------------------------------------------------------*/
int WcPing::nsamples_max() const {
  int n = 0;
  for (const int ns : nsamples)
    n = std::max(n, ns);
  return n;
}

/*--------------------------------------------------------------------*/
int get_format(const std::string &path) {
  if (lower_extension(path) == ".kmwcd")
    return MBF_KEMKMALL;  // not recognised by mb_get_format() on purpose (datalists)
  if (lower_extension(path) == ".wcd")
    return MBF_EM710RAW;  // the same
  std::vector<char> file(path.begin(), path.end());
  file.push_back('\0');
  std::vector<char> root(MB_PATH_MAXLINE, '\0');
  int format = 0, error = MB_ERROR_NO_ERROR;
  mb_get_format(0, file.data(), root.data(), &format, &error);
  return format;
}

/*--------------------------------------------------------------------*/
std::string find_companion(const std::string &path, double max_offset_s) {
  std::string line;
  double time_d;
  if (!name_time(path, line, time_d))
    return std::string();
  const std::string ext = lower_extension(path);
  const std::string other = ext == ".kmall" ? ".kmwcd" : ext == ".kmwcd" ? ".kmall" : ext == ".all" ? ".wcd" : ".all";
  std::filesystem::path dir = std::filesystem::absolute(std::filesystem::path(path)).parent_path();
  std::string best;
  double best_dt = max_offset_s;
  std::error_code ec;
  for (const auto &entry : std::filesystem::directory_iterator(dir, ec)) {
    const std::string cand = entry.path().string();
    std::string cand_line;
    double cand_time;
    if (lower_extension(cand) == other && name_time(cand, cand_line, cand_time) && cand_line == line &&
        std::fabs(cand_time - time_d) <= best_dt) {
      best = cand;
      best_dt = std::fabs(cand_time - time_d);
    }
  }
  return best;
}

/*--------------------------------------------------------------------*/
bool WaterColumnFile::load(const std::string &file, int fmt, const Progress &progress, std::string *message) {
  path = file;
  format = fmt != 0 ? fmt : get_format(file);
  pings.clear();
  amp.clear();
  nav = NavSeries();
  svps.clear();
  if (format == 0) {
    if (message != nullptr)
      *message = "unknown format: " + file;
    return false;
  }
  std::error_code ec;
  const double file_size = (double)std::max<std::uintmax_t>(std::filesystem::file_size(file, ec), 1);

  MbioReader reader;
  if (!reader.open(file, format, message))
    return false;
  std::vector<float> amplitude;
  int nsamples_alloc = 0;
  size_t samples_read = 0;
  amp.reserve((size_t)file_size);  // no more samples than bytes; untouched memory costs nothing
  while (reader.next(message)) {
    /* mb_get_all() reports water column records of some formats as "not a data record" */
    if (!reader.ok() && reader.kind != MB_DATA_WATER_COLUMN)
      continue;
    if (reader.kind == MB_DATA_NAV && (reader.navlon != 0.0 || reader.navlat != 0.0)) {
      nav.time_d.push_back(reader.time_d);
      nav.lon.push_back(reader.navlon);
      nav.lat.push_back(reader.navlat);
      continue;
    }
    if (reader.kind == MB_DATA_VELOCITY_PROFILE) {
      Svp svp;
      if (reader.extract_svp(svp))
        svps.push_back(svp);
      continue;
    }
    if (reader.kind != MB_DATA_WATER_COLUMN && reader.kind != MB_DATA_DATA)
      continue;
    WcPing ping;
    if (!reader.extract_wc(ping, amplitude, nsamples_alloc))
      continue;
    ping.ping_number = reader.ping_number();
    ping.amp_offset = amp.size();
    for (int i = 0; i < ping.nbeams(); i++) {
      const float *a = &amplitude[(size_t)i * nsamples_alloc];
      for (int k = 0; k < ping.nsamples[i]; k++) {
        const double v = std::round(10.0 * a[k]);
        amp.push_back((int16_t)std::max(-32768.0, std::min(32767.0, v)));
      }
      samples_read += ping.nsamples[i];
      range_max_m = std::max(range_max_m, 0.5 * (ping.start_sample[i] + ping.nsamples[i]) *
                                              ping.sample_interval[i] * ping.soundspeed[i]);
    }
    pings.push_back(std::move(ping));
    if (progress)
      progress((int)pings.size(), std::min(1.0, samples_read / file_size));
  }

  /* amplitude range and fill value from a histogram of the stored values; in *.all and
     *.wcd data the lowest value is a valid level, weak samples clipped to it */
  has_fill = false;
  if (!amp.empty()) {
    std::vector<size_t> counts(65536, 0);
    for (const int16_t v : amp)
      counts[(size_t)(v + 32768)]++;
    std::vector<int> present;
    for (int v = 0; v < 65536; v++)
      if (counts[v] > 0)
        present.push_back(v);
    size_t first = 0;
    if (format == MBF_KEMKMALL && present.size() > 1 && counts[present[0]] > 0.001 * amp.size() &&
        counts[present[1]] < 0.01 * counts[present[0]]) {
      has_fill = true;
      fill_value = 0.1 * (present[0] - 32768);
      first = 1;
    }
    amp_min = 0.1 * (present[first] - 32768);
    amp_max = 0.1 * (present.back() - 32768);
  }
  return !pings.empty() || message == nullptr || message->empty();
}

/*--------------------------------------------------------------------*/
std::vector<int> WaterColumnFile::swaths(size_t i) const {
  std::vector<int> s(pings[i].swath);
  std::sort(s.begin(), s.end());
  s.erase(std::unique(s.begin(), s.end()), s.end());
  return s;
}

/*--------------------------------------------------------------------*/
void WaterColumnFile::amplitudes(size_t i, int s, std::vector<int> &beams, int &nsamples_max,
                                 std::vector<float> &out) const {
  const WcPing &p = pings[i];
  beams.clear();
  nsamples_max = 0;
  for (int b = 0; b < p.nbeams(); b++) {
    if (p.swath[b] == s) {
      beams.push_back(b);
      nsamples_max = std::max(nsamples_max, p.nsamples[b]);
    }
  }
  out.assign(beams.size() * (size_t)nsamples_max, std::numeric_limits<float>::quiet_NaN());
  size_t offset = p.amp_offset;
  size_t row = 0;
  for (int b = 0; b < p.nbeams(); b++) {
    if (p.swath[b] == s) {
      float *o = &out[row * nsamples_max];
      for (int k = 0; k < p.nsamples[b]; k++)
        o[k] = 0.1f * amp[offset + k];
      row++;
    }
    offset += p.nsamples[b];
  }
}

/*--------------------------------------------------------------------*/
bool load_survey(const std::string &path, int format, SurveyPings &survey, std::string *message) {
  survey = SurveyPings();
  if (format == 0)
    format = get_format(path);
  MbioReader reader;
  if (format == 0 || !reader.open(path, format, message))
    return false;
  while (reader.next(message)) {
    if (!reader.ok())
      continue;
    if (reader.kind == MB_DATA_VELOCITY_PROFILE) {
      Svp svp;
      if (reader.extract_svp(svp))
        survey.svps.push_back(svp);
    }
    else if (reader.kind == MB_DATA_NAV && (reader.navlon != 0.0 || reader.navlat != 0.0)) {
      survey.nav.time_d.push_back(reader.time_d);
      survey.nav.lon.push_back(reader.navlon);
      survey.nav.lat.push_back(reader.navlat);
    }
    else if (reader.kind == MB_DATA_DATA) {
      survey.time_d.push_back(reader.time_d);
      survey.navlon.push_back(reader.navlon);
      survey.navlat.push_back(reader.navlat);
      survey.speed.push_back(reader.speed);
      survey.heading.push_back(reader.heading);
      survey.sensordepth.push_back(reader.sensordepth);
      survey.ping_number.push_back(reader.ping_number());
      survey.beamflag.emplace_back(reader.beamflag, reader.beamflag + reader.nbath);
      survey.bath.emplace_back(reader.bath, reader.bath + reader.nbath);
      survey.bathacrosstrack.emplace_back(reader.bathacrosstrack, reader.bathacrosstrack + reader.nbath);
      survey.detects.push_back(reader.detects());
    }
  }
  return true;
}

/*--------------------------------------------------------------------*/
bool Line::load(const std::string &path, const Progress &progress, std::string *message, int format) {
  const std::string ext = lower_extension(path);
  const bool is_kmwcd = ext == ".kmwcd" || ext == ".wcd";
  const bool is_kmall = ext == ".kmall" || ext == ".all";
  kmwcd = is_kmwcd ? path : (is_kmall ? find_companion(path) : path);
  if (ext == ".all" && kmwcd.empty())
    kmwcd = path;  // a *.all without a *.wcd may hold the water column itself
  kmall = is_kmall ? path : (is_kmwcd ? find_companion(path) : std::string());
  has_survey = !kmall.empty() && load_survey(kmall, kmall == path ? format : 0, survey, message);
  has_wc = !kmwcd.empty() && wc.load(kmwcd, kmwcd == path ? format : 0, progress, message) && wc.size() > 0;

  match_.assign(has_wc ? wc.size() : 0, -1);
  if (has_wc && has_survey) {
    std::multimap<unsigned int, int> by_ping;
    for (size_t j = 0; j < survey.size(); j++)
      by_ping.emplace(survey.ping_number[j], (int)j);
    for (size_t i = 0; i < wc.size(); i++) {
      auto range = by_ping.equal_range(wc.pings[i].ping_number);
      for (auto it = range.first; it != range.second; ++it) {
        if (std::fabs(survey.time_d[it->second] - wc.pings[i].time_d) <= 1.0) {
          match_[i] = it->second;
          break;
        }
      }
    }
  }
  if (!has_wc && !has_survey && message != nullptr && message->empty())
    *message = "no water column or survey data in " + path;
  return has_wc || has_survey;
}

/*--------------------------------------------------------------------*/
int Line::survey_index(size_t i) const { return i < match_.size() ? match_[i] : -1; }

int Line::matched() const {
  return (int)std::count_if(match_.begin(), match_.end(), [](int j) { return j >= 0; });
}

/*--------------------------------------------------------------------*/
void Line::nav_at(double time_d, double &lon, double &lat, double &sog_kn, double &cog) const {
  lon = lat = sog_kn = cog = NaN;
  NavSeries pings_nav;
  if (has_survey) {
    /* no position before the first position record */
    for (size_t j = 0; j < survey.size(); j++) {
      if (survey.navlon[j] != 0.0 || survey.navlat[j] != 0.0) {
        pings_nav.time_d.push_back(survey.time_d[j]);
        pings_nav.lon.push_back(survey.navlon[j]);
        pings_nav.lat.push_back(survey.navlat[j]);
      }
    }
  }
  const NavSeries *sources[3] = {has_survey ? &survey.nav : nullptr, has_survey ? &pings_nav : nullptr,
                                 has_wc ? &wc.nav : nullptr};
  for (const NavSeries *s : sources) {
    size_t k;
    if (s == nullptr || !interp(s->time_d, s->lon, time_d, lon, k))
      continue;
    interp(s->time_d, s->lat, time_d, lat, k);
    const size_t k0 = k >= 5 ? k - 5 : 0;
    const size_t k1 = std::min(k + 5, s->size() - 1);
    const double dt = s->time_d[k1] - s->time_d[k0];
    if (dt > 0.0) {
      const double dn = (s->lat[k1] - s->lat[k0]) * 111320.0;
      const double de = (s->lon[k1] - s->lon[k0]) * 111320.0 * std::cos(lat * DTR);
      sog_kn = std::hypot(dn, de) / dt * 1.9438445;
      cog = std::fmod(std::atan2(de, dn) * RTD + 360.0, 360.0);
    }
    return;
  }
}

/*--------------------------------------------------------------------*/
double Line::heading_at(double time_d) const {
  if (!has_survey || survey.size() == 0)
    return NaN;
  const std::vector<double> &t = survey.time_d;
  if (time_d < t.front() - 10.0 || time_d > t.back() + 10.0)
    return NaN;
  if (survey.size() == 1 || time_d <= t.front())
    return survey.heading.front();
  if (time_d >= t.back())
    return survey.heading.back();
  const size_t k = (size_t)(std::upper_bound(t.begin(), t.end(), time_d) - t.begin());
  const double h0 = survey.heading[k - 1];
  double dh = survey.heading[k] - h0;
  dh -= 360.0 * std::round(dh / 360.0);
  const double f = (t[k] > t[k - 1]) ? (time_d - t[k - 1]) / (t[k] - t[k - 1]) : 0.0;
  return std::fmod(h0 + f * dh + 360.0, 360.0);
}

/*--------------------------------------------------------------------*/
const Svp *Line::svp(double time_d) const {
  const std::vector<Svp> *sources[2] = {has_survey ? &survey.svps : nullptr, has_wc ? &wc.svps : nullptr};
  for (const std::vector<Svp> *s : sources) {
    if (s == nullptr || s->empty())
      continue;
    const Svp *best = &s->front();
    for (const Svp &p : *s)
      if (p.time_d <= time_d || p.time_d <= 0.0)
        best = &p;
    return best;
  }
  return nullptr;
}

/*--------------------------------------------------------------------*/
bool Line::draft(size_t i, double &sensordepth) const {
  const int j = survey_index(i);
  if (j < 0)
    return false;
  sensordepth = survey.sensordepth[j];
  return true;
}

}  // namespace wcd

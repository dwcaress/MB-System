/*--------------------------------------------------------------------
 *    The MB-system:  wcd_dump.cc  1/11/2026
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
 * mbwcdview --dump: summary of a line without the GUI - pings, pairing, navigation,
 * fill value, and the water column bottom detections against the soundings of the
 * *.kmall or *.all, with straight and ray traced geometry.
 */

#include "wcd_dump.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <limits>
#include <utility>
#include <vector>

#include "mb_define.h"
#include "mb_status.h"
#include "wcd_data.h"
#include "wcd_raytrace.h"

namespace wcd {

namespace {

/* median of the finite values, NaN if there are none */
double median(std::vector<double> v) {
  v.erase(std::remove_if(v.begin(), v.end(), [](double x) { return !std::isfinite(x); }), v.end());
  if (v.empty())
    return std::nan("");
  std::nth_element(v.begin(), v.begin() + v.size() / 2, v.end());
  return v[v.size() / 2];
}

}  // namespace

int dump(const std::string &path, int format, int ping_index) {
  Line line;
  std::string message;
  if (!line.load(path, Progress(), &message, format, true)) {
    fprintf(stderr, "mbwcdview: %s\n", message.c_str());
    return 1;
  }
  printf("survey file: %s\nwater column file: %s\n", line.kmall.empty() ? "-" : line.kmall.c_str(),
         line.kmwcd.empty() ? "-" : line.kmwcd.c_str());
  printf("survey pings %zu, position records %zu, profiles %zu\n", line.has_survey ? line.survey.size() : 0,
         line.has_survey ? line.survey.nav.size() : 0, line.has_survey ? line.survey.svps.size() : 0);
  if (!line.has_wc) {
    printf("no water column data\n");
    return 0;
  }
  const WaterColumnFile &wc = line.wc;
  printf("water column pings %zu, matched %d, samples %zu, position records %zu, profiles %zu\n", wc.size(),
         line.matched(), wc.amp.size(), wc.nav.size(), wc.svps.size());
  printf("fill value %s%.1f dB, amplitude %.1f .. %.1f dB, longest slant range %.1f m, depth window from %.1f m\n",
         wc.has_fill ? "" : "none ", wc.fill_value, wc.amp_min, wc.amp_max, wc.range_max_m, line.default_depth_min());

  const size_t i0 = std::min((size_t)std::max(ping_index, 0), wc.size() - 1);
  const WcPing &p0 = wc.pings[i0];
  double lon, lat, sog, cog;
  line.nav_at(p0.time_d, lon, lat, sog, cog);
  printf("ping index %zu: ping number %u, time %.6f, beams %d, swaths %zu, lat %.6f lon %.6f SOG %.2f kn "
         "COG %.1f heading %.1f\n", i0, p0.ping_number, p0.time_d, p0.nbeams(), wc.swaths(i0).size(), lat, lon, sog,
         cog, line.heading_at(p0.time_d));
  double draft0 = 0.0;
  bool interpolated0 = false;
  if (line.draft(i0, draft0, &interpolated0))
    printf("transducer depth %.3f m%s\n", draft0, interpolated0 ? " (interpolated)" : "");

  if (!line.has_survey)
    return 0;

  /* water column bottom detections against the soundings, every 10th ping */
  std::vector<double> ratio[2], across[2];
  double seconds = 0.0;
  int traced_pings = 0, by_fraction = 0, by_whole_angle = 0, by_whole_direction = 0;
  for (size_t i = 0; i < wc.size(); i += 10) {
    const int j = line.survey_index(i);
    if (j < 0)
      continue;
    const WcPing &p = wc.pings[i];
    double draft = 0.0;
    line.draft(i, draft);
    std::vector<int> beams(p.nbeams());
    for (int b = 0; b < p.nbeams(); b++)
      beams[b] = b;
    const std::vector<double> &bz = line.survey.bath[j];
    const std::vector<double> &bx = line.survey.bathacrosstrack[j];
    const std::vector<char> &bf = line.survey.beamflag[j];
    double xmax = 0.0;
    for (size_t k = 0; k < bz.size(); k++)
      if (!mb_beam_check_flag_null(bf[k]))
        xmax = std::max(xmax, std::fabs(bx[k]));

    /* each beam's sounding, the same for both geometries: the sounding (not null) seen within
       10 deg of the direction of the beam's water column detection in straight geometry whose
       two-way travel time (mb_ttimes) is nearest to that of the detection. The detection is
       the sonar's own detection of the beam, so its travel time finds the beam's sounding
       whatever the order of the soundings (extra and multiple detections, several receive
       fans, lost water column datagrams) and however they were positioned (ray traced by the
       sonar or by mbprocess). Of several soundings as near in time: where the ping gives its
       detections in fractions of a sample (Kongsberg kmall, Reson 7k), within the float
       precision of the travel times, the one nearest in direction; where it gives them in whole
       samples (Kongsberg *.all and *.wcd, kmall water column datagrams of version 0), within a
       sample, the one whose beam angle (mb_ttimes) is nearest to the beam's own, unless the
       ping's beam angles disagree with those of the water column by more than half the beam
       spacing (MBIO computes kmall beam angles with the roll at the echo), judged on the beams
       whose soundings within two samples of the detection have one beam angle: then again the
       one nearest in direction */
    std::vector<std::pair<int, size_t>> pairs;
    static const std::vector<double> none;
    const std::vector<double> &bt = (size_t)j < line.survey.ttimes.size() ? line.survey.ttimes[j] : none;
    const std::vector<double> &ba = (size_t)j < line.survey.beam_angles.size() ? line.survey.beam_angles[j] : none;
    {
      SwathGeometry g;
      swath_geometry(p, beams, p.nsamples_max(), draft, nullptr, g, nullptr);
      std::vector<std::pair<double, size_t>> direction;
      for (size_t k = 0; k < bz.size() && k < bt.size(); k++)
        if (!mb_beam_check_flag_null(bf[k]) && std::isfinite(bt[k]))
          direction.emplace_back(std::atan2(bx[k], bz[k] - draft), k);
      std::sort(direction.begin(), direction.end());
      const double window = 10.0 * DTR;
      const double inf = std::numeric_limits<double>::infinity();

      /* the detections with their direction, travel time, candidate soundings (a range of
         direction) and nearest travel time; whole samples or fractions; the beam spacing */
      struct Detection {
        int b;
        double dir, twt, best_dt;
        size_t first, last, nearest;
      };
      std::vector<Detection> detections;
      int whole = 0;
      std::vector<double> spacing;
      for (int b = 0; b < p.nbeams(); b++) {
        if (b > 0 && p.swath[b] == p.swath[b - 1])
          spacing.push_back(std::fabs(p.angle_across[b] - p.angle_across[b - 1]));
        if (!(p.detect_sample[b] > 0.0) || !std::isfinite(g.detect_depth[b]))
          continue;
        if (p.detect_sample[b] == std::floor(p.detect_sample[b]))
          whole++;
        Detection d;
        d.b = b;
        d.dir = std::atan2(g.detect_across[b], g.detect_depth[b] - draft);
        d.twt = p.detect_sample[b] * p.sample_interval[b];
        d.first = std::lower_bound(direction.begin(), direction.end(), std::make_pair(d.dir - window, (size_t)0)) -
                  direction.begin();
        d.last = std::upper_bound(direction.begin(), direction.end(), std::make_pair(d.dir + window, (size_t)-1)) -
                 direction.begin();
        d.best_dt = inf;
        d.nearest = 0;
        for (size_t c = d.first; c < d.last; c++) {
          const double dt = std::fabs(bt[direction[c].second] - d.twt);
          if (dt < d.best_dt) {
            d.best_dt = dt;
            d.nearest = direction[c].second;
          }
        }
        if (std::isfinite(d.best_dt))
          detections.push_back(d);
      }
      const bool whole_samples = 2 * whole > (int)detections.size();
      const double half_spacing = 0.5 * median(spacing);

      /* whole samples: do the beam angles tell neighbouring beams apart? */
      bool by_angle = false;
      if (whole_samples) {
        std::vector<double> disagree;
        for (const Detection &d : detections) {
          const double interval = p.sample_interval[d.b];
          if (d.best_dt > interval || d.nearest >= ba.size() || !std::isfinite(ba[d.nearest]))
            continue;
          bool one_angle = true;
          for (size_t c = d.first; c < d.last && one_angle; c++) {
            const size_t k = direction[c].second;
            if (std::fabs(bt[k] - d.twt) <= d.best_dt + 2.0 * interval &&
                !(std::fabs(ba[k] - ba[d.nearest]) <= half_spacing))
              one_angle = false;
          }
          if (one_angle)
            disagree.push_back(std::fabs(ba[d.nearest] - p.angle_across[d.b]));
        }
        by_angle = !(disagree.size() >= 3 && median(disagree) > half_spacing);
      }

      for (const Detection &d : detections) {
        const double tie = whole_samples ? p.sample_interval[d.b] : 2.5e-7 * d.twt + 1.0e-9;
        size_t best = d.nearest;
        double best_key = inf;
        for (size_t c = d.first; c < d.last; c++) {
          const size_t k = direction[c].second;
          if (std::fabs(bt[k] - d.twt) > d.best_dt + tie)
            continue;
          double key = by_angle ? (k < ba.size() ? std::fabs(ba[k] - p.angle_across[d.b]) : inf)
                                : std::fabs(direction[c].first - d.dir);
          if (!std::isfinite(key))
            key = inf;
          if (key < best_key) {
            best_key = key;
            best = k;
          }
        }
        pairs.emplace_back(d.b, best);
      }
      (whole_samples ? (by_angle ? by_whole_angle : by_whole_direction) : by_fraction) += (int)pairs.size();
    }
    if (pairs.empty())
      continue;

    for (int mode = 0; mode < 2; mode++) {
      SwathGeometry g;
      const auto t0 = std::chrono::steady_clock::now();
      swath_geometry(p, beams, p.nsamples_max(), draft, mode == 1 ? line.svp(p.time_d) : nullptr, g, nullptr);
      if (mode == 1) {
        seconds += std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        traced_pings++;
      }

      std::vector<double> r, dx;
      for (const auto &pair : pairs) {
        const int b = pair.first;
        const size_t k = pair.second;
        if (mb_beam_check_flag_null(bf[k]) || !std::isfinite(g.detect_depth[b]) || bz[k] <= 0.0)
          continue;
        r.push_back(g.detect_depth[b] / bz[k]);
        if (std::fabs(bx[k]) > 0.6 * xmax)
          dx.push_back(std::fabs(g.detect_across[b] - bx[k]));
      }
      ratio[mode].push_back(median(r));
      across[mode].push_back(median(dx));
    }
  }
  printf("beams compared with soundings by travel time: %d (detections in fractions of a sample), %d (whole "
         "samples, ties by beam angle), %d (whole samples, ties by direction)\n",
         by_fraction, by_whole_angle, by_whole_direction);
  printf("water column / sounding depth: straight %.4f, ray traced %.4f\n", median(ratio[0]), median(ratio[1]));
  printf("outer beams |across water column - sounding|: straight %.1f m, ray traced %.1f m\n", median(across[0]),
         median(across[1]));
  printf("ray tracing %.0f ms per ping\n", traced_pings > 0 ? 1000.0 * seconds / traced_pings : 0.0);
  return 0;
}

}  // namespace wcd

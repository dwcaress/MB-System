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
 * *.kmall, with straight and ray traced geometry.
 */

#include "wcd_dump.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <vector>

#include "mb_define.h"
#include "mb_status.h"
#include "wcd_data.h"
#include "wcd_raytrace.h"

namespace wcd {

namespace {

double median(std::vector<double> v) {
  if (v.empty())
    return std::nan("");
  std::nth_element(v.begin(), v.begin() + v.size() / 2, v.end());
  return v[v.size() / 2];
}

}  // namespace

int dump(const std::string &path, int format, int ping_index) {
  Line line;
  std::string message;
  if (!line.load(path, Progress(), &message, format)) {
    fprintf(stderr, "mbwcdview: %s\n", message.c_str());
    return 1;
  }
  printf("kmall: %s\nkmwcd: %s\n", line.kmall.empty() ? "-" : line.kmall.c_str(),
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
  printf("fill value %s%.1f dB, amplitude %.1f .. %.1f dB, longest slant range %.1f m\n", wc.has_fill ? "" : "none ",
         wc.fill_value, wc.amp_min, wc.amp_max, wc.range_max_m);

  const size_t i0 = std::min((size_t)std::max(ping_index, 0), wc.size() - 1);
  const WcPing &p0 = wc.pings[i0];
  double lon, lat, sog, cog;
  line.nav_at(p0.time_d, lon, lat, sog, cog);
  printf("ping index %zu: ping number %u, time %.6f, beams %d, swaths %zu, lat %.6f lon %.6f SOG %.2f kn "
         "COG %.1f heading %.1f\n", i0, p0.ping_number, p0.time_d, p0.nbeams(), wc.swaths(i0).size(), lat, lon, sog,
         cog, line.heading_at(p0.time_d));

  if (!line.has_survey)
    return 0;

  /* water column bottom detections against the soundings, every 10th ping */
  std::vector<double> ratio[2], across[2];
  double seconds = 0.0;
  int traced_pings = 0;
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
    const size_t n = std::min(bz.size(), (size_t)p.nbeams());
    double xmax = 0.0;
    for (size_t b = 0; b < n; b++)
      if (!mb_beam_check_flag_null(bf[b]))
        xmax = std::max(xmax, std::fabs(bx[b]));
    for (int mode = 0; mode < 2; mode++) {
      SwathGeometry g;
      const auto t0 = std::chrono::steady_clock::now();
      swath_geometry(p, beams, p.nsamples_max(), draft, mode == 1 ? line.svp(p.time_d) : nullptr, g, nullptr);
      if (mode == 1) {
        seconds += std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        traced_pings++;
      }
      std::vector<double> r, dx;
      for (size_t b = 0; b < n; b++) {
        if (mb_beam_check_flag_null(bf[b]) || !std::isfinite(g.detect_depth[b]) || bz[b] <= 0.0)
          continue;
        r.push_back(g.detect_depth[b] / bz[b]);
        if (std::fabs(bx[b]) > 0.6 * xmax)
          dx.push_back(std::fabs(g.detect_across[b] - bx[b]));
      }
      ratio[mode].push_back(median(r));
      across[mode].push_back(median(dx));
    }
  }
  printf("water column / sounding depth: straight %.4f, ray traced %.4f\n", median(ratio[0]), median(ratio[1]));
  printf("outer beams |across water column - sounding|: straight %.1f m, ray traced %.1f m\n", median(across[0]),
         median(across[1]));
  printf("ray tracing %.0f ms per ping\n", traced_pings > 0 ? 1000.0 * seconds / traced_pings : 0.0);
  return 0;
}

}  // namespace wcd

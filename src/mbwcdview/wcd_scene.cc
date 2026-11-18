/*--------------------------------------------------------------------
 *    The MB-system:  wcd_scene.cc  1/11/2026
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

#include "wcd_scene.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <QDateTime>
#include <QTimeZone>

#include "mb_define.h"
#include "mb_status.h"
#include "wcd_raytrace.h"

namespace wcd {

namespace {

QString number_or_na(double v, int decimals) { return std::isfinite(v) ? QString::number(v, 'f', decimals) : "N/A"; }

}  // namespace

/*--------------------------------------------------------------------*/
bool build_scene(const Line &line, size_t i, int swath, const SceneOptions &options, PlotData &data,
                 SceneInfo &info) {
  info = SceneInfo();
  data = PlotData();
  if (!line.has_wc || i >= line.wc.size())
    return false;
  const WcPing &ping = line.wc.pings[i];

  /* amplitudes of the swath; the fill value is drawn as nothing before the first echo of
     a beam (the near field), and later in the beam, where it is a weak echo clipped to the
     lowest level, with the lowest colour */
  std::vector<int> beams;
  int nsamples = 0;
  line.wc.amplitudes(i, swath, beams, nsamples, data.amplitude);
  if (line.wc.has_fill) {
    const float fill = (float)line.wc.fill_value + 0.05f;
    for (size_t b = 0; b < beams.size(); b++) {
      float *a = &data.amplitude[b * (size_t)nsamples];
      for (int k = 0; k < nsamples && a[k] <= fill; k++)
        a[k] = std::numeric_limits<float>::quiet_NaN();
    }
  }
  data.nbeams = (int)beams.size();
  data.nsamples = nsamples;

  /* positions: depth below the waterline when the transducer depth is known */
  double draft = 0.0;
  info.has_draft = line.draft(i, draft);
  const Svp *svp = options.raytrace ? line.svp(ping.time_d) : nullptr;
  SwathGeometry g;
  std::string message;
  swath_geometry(ping, beams, nsamples, draft, svp, g, &message);
  info.traced = g.traced;
  data.across = std::move(g.across);
  data.depth = std::move(g.depth);
  data.detect_across = std::move(g.detect_across);
  data.detect_depth = std::move(g.detect_depth);
  data.show_wc_bottom = options.wc_bottom;
  data.ylabel = info.has_draft ? "Depth below waterline (m)" : "Depth below transducer (m)";
  if (info.traced)
    data.ylabel += info.has_draft ? " - ray traced" : " - ray traced, draft 0";

  /* soundings of the matched survey ping; MBIO keeps the receive fans of a ping in
     order, so with equal fans the soundings of swath k are the k-th block (dual swath) */
  if (options.bottom) {
    const int j = line.survey_index(i);
    if (j < 0) {
      info.status = "no survey ping for this water column ping";
    }
    else {
      const std::vector<double> &bx = line.survey.bathacrosstrack[j];
      const std::vector<double> &bz = line.survey.bath[j];
      const std::vector<char> &bf = line.survey.beamflag[j];
      const std::vector<int> &bd = line.survey.detects[j];
      size_t first = 0, last = bz.size();
      const std::vector<int> swaths = line.wc.swaths(i);
      if (swaths.size() > 1 && bz.size() % swaths.size() == 0) {
        const size_t n = bz.size() / swaths.size();
        const size_t k = (size_t)(std::lower_bound(swaths.begin(), swaths.end(), swath) - swaths.begin());
        first = k * n;
        last = first + n;
      }
      for (size_t b = first; b < last; b++) {
        if (mb_beam_check_flag_null(bf[b]))
          continue;
        const QColor color = bd[b] == MB_DETECT_AMPLITUDE ? QColor(Qt::red)
                             : bd[b] == MB_DETECT_PHASE   ? QColor(0, 0, 139)
                                                          : QColor(Qt::gray);
        data.soundings.push_back({bx[b], bz[b], color, mb_beam_ok(bf[b]) != 0});
      }
      info.soundings = (int)data.soundings.size();
      info.status = QString("soundings: %1").arg(info.soundings);
    }
  }
  return true;
}

/*--------------------------------------------------------------------*/
QString metadata_text(const Line &line, size_t i, int swath) {
  const WcPing &ping = line.wc.pings[i];
  double lon, lat, sog, cog;
  line.nav_at(ping.time_d, lon, lat, sog, cog);
  const QString time = QDateTime::fromMSecsSinceEpoch((qint64)std::llround(ping.time_d * 1000.0), QTimeZone::utc())
                           .toString("yyyy-MM-dd HH:mm:ss.zzz") + " UTC";
  return QString("Ping %1 swath %2: DateTime=%3, SOG=%4 kn, COG=%5, Hdg=%6, Pos=%7,%8")
      .arg(ping.ping_number)
      .arg(swath)
      .arg(time, number_or_na(sog, 2), number_or_na(cog, 1), number_or_na(line.heading_at(ping.time_d), 1),
           number_or_na(lat, 6), number_or_na(lon, 6));
}

}  // namespace wcd

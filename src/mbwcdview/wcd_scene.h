/*--------------------------------------------------------------------
 *    The MB-system:  wcd_scene.h  1/11/2026
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
 * What is drawn for one swath of one water column ping: amplitudes (fill value
 * removed), cell positions (ray traced or straight), the water column bottom
 * detections and the soundings of the matched *.kmall ping.
 */

#ifndef MBWCDVIEW_WCD_SCENE_H
#define MBWCDVIEW_WCD_SCENE_H

#include <QString>

#include "wcd_data.h"
#include "wcd_plot.h"

namespace wcd {

struct SceneOptions {
  bool raytrace = true;   // trace through the sound velocity profile when one exists
  bool bottom = false;    // soundings of the matched *.kmall ping
  bool wc_bottom = false; // water column bottom detections
};

struct SceneInfo {
  bool traced = false;    // geometry ray traced
  bool has_draft = false; // depth below the waterline (else below the transducer)
  int soundings = 0;
  QString status;         // for the status bar
};

bool build_scene(const Line &line, size_t ping, int swath, const SceneOptions &options, PlotData &data,
                 SceneInfo &info);

/* text for the metadata line: ping, swath, time, SOG, COG, heading, position */
QString metadata_text(const Line &line, size_t ping, int swath);

}  // namespace wcd

#endif

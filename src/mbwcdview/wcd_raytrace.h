/*--------------------------------------------------------------------
 *    The MB-system:  wcd_raytrace.h  1/11/2026
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
 * Positions of water column samples: straight rays with the sound speed at the
 * transducer, or ray traced through a sound velocity profile with MB-System's
 * mb_rt() (as mbprocess traces soundings). No motion during the ping is applied.
 */

#ifndef MBWCDVIEW_WCD_RAYTRACE_H
#define MBWCDVIEW_WCD_RAYTRACE_H

#include <string>
#include <vector>

#include "wcd_data.h"

namespace wcd {

/* Positions of the cells of some beams of a ping (rows = beams, nsamples_max columns):
   across (m, positive starboard) and depth (m below the waterline when draft is the
   transducer depth); NaN where a sample has no position (ray turned back) */
struct SwathGeometry {
  int nbeams = 0, nsamples_max = 0;
  std::vector<float> across, depth;
  std::vector<double> detect_across, detect_depth;  // per beam, NaN if no detection
  bool traced = false;
};

/* svp == nullptr: straight rays */
bool swath_geometry(const WcPing &ping, const std::vector<int> &beams, int nsamples_max, double draft,
                    const Svp *svp, SwathGeometry &geometry, std::string *message);

/* Ray tracing model of one sound velocity profile (wraps mb_rt_init/mb_rt_deall) */
class RayModel {
 public:
  ~RayModel();
  bool init(const Svp &svp, std::string *message);
  bool same_profile(const Svp &svp) const {
    return model_ != nullptr && svp.depth == source_.depth && svp.velocity == source_.velocity;
  }
  size_t nodes() const { return depth_.size(); }
  void *model() const { return model_; }
  double speed_at(double depth) const;  // sound speed of the (extended) profile at depth

 private:
  void *model_ = nullptr;
  Svp source_;  // copy of the profile the model was built from
  std::vector<double> depth_, velocity_;  // profile extended to the surface and to depth
};

}  // namespace wcd

#endif

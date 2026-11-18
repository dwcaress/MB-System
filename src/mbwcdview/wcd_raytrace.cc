/*--------------------------------------------------------------------
 *    The MB-system:  wcd_raytrace.cc  1/11/2026
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

#include "wcd_raytrace.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>

#include "mb_define.h"
#include "mb_process.h"
#include "mb_status.h"

namespace wcd {

namespace {

constexpr double NaN = std::numeric_limits<double>::quiet_NaN();
constexpr double SVP_DEPTH_MAX = 12000.0;  // profile extended to this depth with its last speed

/* the model of the profile last used, rebuilt only when the profile changes */
RayModel &cached_model(const Svp *svp, bool &ok, std::string *message) {
  static RayModel model;
  ok = true;
  if (!model.same_profile(*svp))
    ok = model.init(*svp, message);
  return model;
}

}  // namespace

/*--------------------------------------------------------------------*/
RayModel::~RayModel() {
  if (model_ != nullptr) {
    int error = MB_ERROR_NO_ERROR;
    mb_rt_deall(0, &model_, &error);
  }
}

/*--------------------------------------------------------------------*/
bool RayModel::init(const Svp &svp, std::string *message) {
  if (model_ != nullptr) {
    int error = MB_ERROR_NO_ERROR;
    mb_rt_deall(0, &model_, &error);
    model_ = nullptr;
  }
  source_ = Svp();

  /* depths increasing and unique, from the surface down to SVP_DEPTH_MAX */
  std::map<double, double> profile;
  for (size_t i = 0; i < svp.depth.size(); i++)
    if (std::isfinite(svp.depth[i]) && std::isfinite(svp.velocity[i]) && svp.velocity[i] > 0.0)
      profile[svp.depth[i]] = svp.velocity[i];
  if (profile.empty()) {
    if (message != nullptr)
      *message = "empty sound velocity profile";
    return false;
  }
  if (profile.begin()->first > 0.0)
    profile[0.0] = profile.begin()->second;
  if (profile.rbegin()->first < SVP_DEPTH_MAX)
    profile[SVP_DEPTH_MAX] = profile.rbegin()->second;
  depth_.clear();
  velocity_.clear();
  for (const auto &dv : profile) {
    depth_.push_back(dv.first);
    velocity_.push_back(dv.second);
  }
  int error = MB_ERROR_NO_ERROR;
  if (mb_rt_init(0, (int)depth_.size(), depth_.data(), velocity_.data(), &model_, &error) != MB_SUCCESS) {
    model_ = nullptr;
    if (message != nullptr)
      *message = "cannot set up ray tracing";
    return false;
  }
  source_ = svp;
  return true;
}

/*--------------------------------------------------------------------*/
double RayModel::speed_at(double depth) const {
  if (depth <= depth_.front())
    return velocity_.front();
  if (depth >= depth_.back())
    return velocity_.back();
  const size_t k = (size_t)(std::upper_bound(depth_.begin(), depth_.end(), depth) - depth_.begin());
  const double f = (depth - depth_[k - 1]) / (depth_[k] - depth_[k - 1]);
  return velocity_[k - 1] + f * (velocity_[k] - velocity_[k - 1]);
}

/*--------------------------------------------------------------------*/
bool swath_geometry(const WcPing &ping, const std::vector<int> &beams, int nsamples_max, double draft,
                    const Svp *svp, SwathGeometry &g, std::string *message) {
  g.nbeams = (int)beams.size();
  g.nsamples_max = nsamples_max;
  g.across.assign((size_t)g.nbeams * nsamples_max, std::numeric_limits<float>::quiet_NaN());
  g.depth.assign((size_t)g.nbeams * nsamples_max, std::numeric_limits<float>::quiet_NaN());
  g.detect_across.assign(g.nbeams, NaN);
  g.detect_depth.assign(g.nbeams, NaN);
  g.traced = false;

  bool ok = false;
  RayModel *model = svp != nullptr ? &cached_model(svp, ok, message) : nullptr;
  if (model != nullptr && !ok)
    model = nullptr;
  g.traced = model != nullptr;

  std::vector<double> xplot, zplot, tplot;
  for (int r = 0; r < g.nbeams; r++) {
    const int b = beams[r];
    const double a = DTR * ping.angle_across[b];
    const double t_along = DTR * ping.angle_along[b];
    const double down = std::cos(t_along) * std::cos(a);
    const double h_across = std::cos(t_along) * std::sin(a);
    const double h_along = std::sin(t_along);
    const double horiz = std::hypot(h_across, h_along);
    const double dir_across = horiz > 0.0 ? h_across / horiz : 0.0;
    const double interval = ping.sample_interval[b];
    const int start = ping.start_sample[b];
    const int ns = ping.nsamples[b];
    float *row_x = &g.across[(size_t)r * nsamples_max];
    float *row_z = &g.depth[(size_t)r * nsamples_max];
    const double t_detect = ping.detect_sample[b] > 0.0 ? 0.5 * ping.detect_sample[b] * interval : NaN;

    if (model == nullptr) {
      /* straight rays: slant range from the sound speed at the transducer */
      const double c = ping.soundspeed[b];
      for (int k = 0; k < ns; k++) {
        const double range = 0.5 * c * (start + k) * interval;
        row_x[k] = (float)(range * h_across);
        row_z[k] = (float)(draft + range * down);
      }
      if (std::isfinite(t_detect)) {
        g.detect_across[r] = c * t_detect * h_across;
        g.detect_depth[r] = draft + c * t_detect * down;
      }
      continue;
    }

    /* trace the beam to its last sample with mb_rt(); its path points (layer boundaries
       and arc segments in gradient layers) give the ray's depth and distance. The times
       mb_rt() records for arc segments are not reliable, so the one-way time along the
       path is summed here from segment length / sound speed at the segment's mid depth
       (equal to mb_rt()'s own end points to about 0.01 m) */
    const double takeoff = RTD * std::acos(std::max(-1.0, std::min(1.0, down)));
    const double t_end = std::max(0.5 * (start + ns) * interval, std::isfinite(t_detect) ? t_detect : 0.0);
    const int nplot_max = 6 * ((int)model->nodes() + 2) + 64;
    xplot.resize(nplot_max);
    zplot.resize(nplot_max);
    tplot.resize(nplot_max);
    int nplot = 0, ray_stat = 0, error = MB_ERROR_NO_ERROR;
    double x, z, tt;
    if (mb_rt(0, model->model(), draft, takeoff, t_end, MBP_ANGLES_SNELL, ping.soundspeed[b], 0.0, nplot_max,
              &nplot, xplot.data(), zplot.data(), tplot.data(), &x, &z, &tt, &ray_stat, &error) != MB_SUCCESS ||
        nplot < 2)
      continue;
    tplot[0] = 0.0;
    for (int k = 1; k < nplot; k++)
      tplot[k] = tplot[k - 1] + std::hypot(xplot[k] - xplot[k - 1], zplot[k] - zplot[k - 1]) /
                                    model->speed_at(0.5 * (zplot[k] + zplot[k - 1]));
    auto locate = [&](double t, double &px, double &pz) {
      /* the summed path time may end a little short of the traced time */
      if (t < tplot[0] || t > tplot[nplot - 1] + 1.0e-5)
        return false;
      t = std::min(t, tplot[nplot - 1]);
      const int k = (int)(std::upper_bound(tplot.begin(), tplot.begin() + nplot, t) - tplot.begin());
      const int k1 = std::min(std::max(k, 1), nplot - 1);
      const double dt = tplot[k1] - tplot[k1 - 1];
      const double f = dt > 0.0 ? (t - tplot[k1 - 1]) / dt : 0.0;
      px = xplot[k1 - 1] + f * (xplot[k1] - xplot[k1 - 1]);
      pz = zplot[k1 - 1] + f * (zplot[k1] - zplot[k1 - 1]);
      return true;
    };
    for (int k = 0; k < ns; k++) {
      double px, pz;
      if (locate(0.5 * (start + k) * interval, px, pz)) {
        row_x[k] = (float)(px * dir_across);
        row_z[k] = (float)pz;
      }
    }
    double px, pz;
    if (std::isfinite(t_detect) && locate(t_detect, px, pz)) {
      g.detect_across[r] = px * dir_across;
      g.detect_depth[r] = pz;
    }
  }
  return true;
}

}  // namespace wcd

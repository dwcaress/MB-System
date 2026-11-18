/*--------------------------------------------------------------------
 *    The MB-system:  mb_rt_test.cc  2/11/2026
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
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

#include <cmath>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "mbio/mb_define.h"
#include "mbio/mb_status.h"

namespace {

constexpr int kPlotMax = 20000;
constexpr int kSnell = 1;  // ssv_mode of mb_rt(), as MBP_ANGLES_SNELL in mb_process.h
constexpr int kOutBottom = 5;  // MB_RT_OUT_BOTTOM in mb_rt.c

// sound speed decreasing, then increasing with depth: a channel where rays turn both ways
std::vector<double> depths = {0.0, 50.0, 200.0, 800.0, 1500.0, 12000.0};
std::vector<double> speeds = {1540.0, 1538.0, 1510.0, 1485.0, 1490.0, 1660.0};

struct Ray {
  int nplot = 0;
  std::vector<double> x, z, t;
  double xf = 0.0, zf = 0.0, tt = 0.0;
};

double speed_at(double z) {
  size_t j = 1;
  while (j + 1 < depths.size() && depths[j] < z) j++;
  return speeds[j - 1] + (speeds[j] - speeds[j - 1]) * (z - depths[j - 1]) / (depths[j] - depths[j - 1]);
}

Ray trace(void *model, double source, double angle, double end_time, bool path) {
  Ray r;
  int stat = 0, error = MB_ERROR_NO_ERROR;
  r.x.resize(kPlotMax);
  r.z.resize(kPlotMax);
  r.t.resize(kPlotMax);
  EXPECT_EQ(MB_SUCCESS, mb_rt(0, model, source, angle, end_time, kSnell, speed_at(source), 0.0, path ? kPlotMax : 0,
                              path ? &r.nplot : nullptr, path ? r.x.data() : nullptr, path ? r.z.data() : nullptr,
                              path ? r.t.data() : nullptr, &r.xf, &r.zf, &r.tt, &stat, &error));
  return r;
}

class MbRtTest : public testing::Test {
 protected:
  void SetUp() override {
    int error = MB_ERROR_NO_ERROR;
    ASSERT_EQ(MB_SUCCESS, mb_rt_init(0, (int)depths.size(), depths.data(), speeds.data(), &model_, &error));
  }
  void TearDown() override {
    int error = MB_ERROR_NO_ERROR;
    mb_rt_deall(0, &model_, &error);
  }
  void *model_ = nullptr;
};

// the path times must increase along the ray and end at the traced time, and each point of the
// path must be the end point of a ray traced for that point's time
void check_path(void *model, double source, double angle, double end_time) {
  SCOPED_TRACE(testing::Message() << "source " << source << " angle " << angle << " time " << end_time);
  const Ray r = trace(model, source, angle, end_time, true);
  ASSERT_GT(r.nplot, 2);
  EXPECT_NEAR(r.x[r.nplot - 1], r.xf, 1.0e-6);
  EXPECT_NEAR(r.z[r.nplot - 1], r.zf, 1.0e-6);
  EXPECT_NEAR(r.t[r.nplot - 1], r.tt, 1.0e-6);
  for (int k = 1; k < r.nplot; k++) {
    ASSERT_GE(r.t[k], r.t[k - 1]) << "path point " << k;
    const Ray e = trace(model, source, angle, r.t[k], false);
    ASSERT_NEAR(r.x[k], e.xf, 1.0e-3) << "path point " << k;
    ASSERT_NEAR(r.z[k], e.zf, 1.0e-3) << "path point " << k;
  }
}

TEST_F(MbRtTest, PathTimesDownward) {
  for (const double angle : {1.0, 30.0, 60.0, 75.0, 85.0})
    for (const double time : {0.5, 1.5, 3.0})
      check_path(model_, 5.0, angle, time);
}

TEST_F(MbRtTest, PathTimesTurningRays) {
  // from the channel axis, close to horizontal: the ray turns below and above the axis
  for (const double angle : {70.0, 80.0, 85.0, 88.0, 89.5})
    check_path(model_, 800.0, angle, 20.0);
}

TEST_F(MbRtTest, PathTimesRayLeavesModel) {
  std::vector<double> d = {0.0, 100.0}, c = {1500.0, 1520.0};
  void *model = nullptr;
  int error = MB_ERROR_NO_ERROR, nplot = 0, stat = 0;
  ASSERT_EQ(MB_SUCCESS, mb_rt_init(0, 2, d.data(), c.data(), &model, &error));
  std::vector<double> x(kPlotMax), z(kPlotMax), t(kPlotMax);
  double xf, zf, tt;
  mb_rt(0, model, 5.0, 45.0, 1.0, kSnell, 1500.0, 0.0, kPlotMax, &nplot, x.data(), z.data(), t.data(), &xf, &zf, &tt,
        &stat, &error);
  EXPECT_EQ(kOutBottom, stat);
  EXPECT_LT(tt, 1.0);
  EXPECT_NEAR(t[nplot - 1], tt, 1.0e-6);
  mb_rt_deall(0, &model, &error);
}

}  // namespace

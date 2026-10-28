// Tests of the CUBE gridding algorithm in src/mbaux/mb_cube.c.
//
// See README.md file for copying and redistribution conditions.

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

#include "mb_cube.h"
#include "mb_define.h"
#include "mb_status.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {

const int verbose = 0;
const float kNoData = 99999.0f;

// Grid of CUBE nodes released after each test
class MbCube : public ::testing::Test {
 protected:
  void SetUp() override {
    int error = MB_ERROR_NO_ERROR;
    ASSERT_EQ(MB_SUCCESS, mb_cube_default_param(verbose, MB_CUBE_IHO_ORDER_2, &param_, &error));
  }
  void TearDown() override {
    int error = MB_ERROR_NO_ERROR;
    if (cube_ != nullptr)
      EXPECT_EQ(MB_SUCCESS, mb_cube_deall(verbose, &cube_, &error));
  }
  void Init(int nx, int ny, double dx, double dy) {
    int error = MB_ERROR_NO_ERROR;
    nx_ = nx;
    ny_ = ny;
    ASSERT_EQ(MB_SUCCESS, mb_cube_init(verbose, &param_, nx, ny, dx, dy, &cube_, &error));
    ASSERT_EQ(MB_ERROR_NO_ERROR, error);
  }
  int Insert(double x, double y, double depth, double var_depth, double var_horiz, double depth_sonar = -1.0) {
    int error = MB_ERROR_NO_ERROR;
    int nused = 0;
    EXPECT_EQ(MB_SUCCESS, mb_cube_insert(verbose, cube_, x, y, depth, depth_sonar < 0.0 ? depth : depth_sonar,
                                         var_depth, var_horiz, &nused, &error));
    return nused;
  }
  void Flush() {
    int error = MB_ERROR_NO_ERROR;
    ASSERT_EQ(MB_SUCCESS, mb_cube_flush(verbose, cube_, &error));
  }
  void Extract() {
    const size_t n = static_cast<size_t>(nx_) * ny_;
    depth_.assign(n, 0.0f);
    unct_.assign(n, 0.0f);
    nhyp_.assign(n, 0.0f);
    strength_.assign(n, 0.0f);
    nsamples_.assign(n, 0);
    int error = MB_ERROR_NO_ERROR;
    ASSERT_EQ(MB_SUCCESS, mb_cube_extract(verbose, cube_, kNoData, depth_.data(), unct_.data(), nhyp_.data(),
                                          strength_.data(), nsamples_.data(), &error));
  }
  size_t K(int i, int j) const { return static_cast<size_t>(i) * ny_ + j; }

  struct mb_cube_param_struct param_;
  void *cube_ = nullptr;
  int nx_ = 0;
  int ny_ = 0;
  std::vector<float> depth_, unct_, nhyp_, strength_;
  std::vector<int> nsamples_;
};

TEST(MbCubeParam, IhoLimitsS44Edition6) {
  int error = MB_ERROR_NO_ERROR;
  double a, b, thu, thu_fraction;
  ASSERT_EQ(MB_SUCCESS, mb_cube_iho_limits(verbose, MB_CUBE_IHO_ORDER_2, &a, &b, &thu, &thu_fraction, &error));
  EXPECT_DOUBLE_EQ(1.0, a);
  EXPECT_DOUBLE_EQ(0.023, b);
  EXPECT_DOUBLE_EQ(20.0, thu);
  EXPECT_DOUBLE_EQ(0.10, thu_fraction);
  ASSERT_EQ(MB_SUCCESS, mb_cube_iho_limits(verbose, MB_CUBE_IHO_ORDER_1A, &a, &b, &thu, &thu_fraction, &error));
  EXPECT_DOUBLE_EQ(0.5, a);
  EXPECT_DOUBLE_EQ(0.013, b);
  EXPECT_DOUBLE_EQ(5.0, thu);
  EXPECT_DOUBLE_EQ(0.05, thu_fraction);
  ASSERT_EQ(MB_SUCCESS, mb_cube_iho_limits(verbose, MB_CUBE_IHO_SPECIAL, &a, &b, &thu, &thu_fraction, &error));
  EXPECT_DOUBLE_EQ(0.25, a);
  EXPECT_DOUBLE_EQ(0.0075, b);
  EXPECT_DOUBLE_EQ(2.0, thu);
  ASSERT_EQ(MB_SUCCESS, mb_cube_iho_limits(verbose, MB_CUBE_IHO_EXCLUSIVE, &a, &b, &thu, &thu_fraction, &error));
  EXPECT_DOUBLE_EQ(0.15, a);
  EXPECT_DOUBLE_EQ(1.0, thu);
  EXPECT_EQ(MB_FAILURE, mb_cube_iho_limits(verbose, 7, &a, &b, &thu, &thu_fraction, &error));
  EXPECT_EQ(MB_ERROR_BAD_PARAMETER, error);
}

// Table 1 of S-44 4th edition (1998), 5th edition (2008) and 6.0.0/6.1.0
// (2020/2022), and the CCOM/JHC errmod.c table
TEST(MbCubeParam, IhoEditionLimits) {
  struct Row {
    int edition;
    const char *order;
    double a, b, thu, thu_fraction;
  };
  const Row rows[] = {{MB_CUBE_IHO_EDITION_4, "special", 0.25, 0.0075, 2.0, 0.0},
                      {MB_CUBE_IHO_EDITION_4, "1", 0.5, 0.013, 5.0, 0.05},
                      {MB_CUBE_IHO_EDITION_4, "2", 1.0, 0.023, 20.0, 0.05},
                      {MB_CUBE_IHO_EDITION_4, "3", 1.0, 0.023, 150.0, 0.05},
                      {MB_CUBE_IHO_EDITION_5, "special", 0.25, 0.0075, 2.0, 0.0},
                      {MB_CUBE_IHO_EDITION_5, "1a", 0.5, 0.013, 5.0, 0.05},
                      {MB_CUBE_IHO_EDITION_5, "1b", 0.5, 0.013, 5.0, 0.05},
                      {MB_CUBE_IHO_EDITION_5, "2", 1.0, 0.023, 20.0, 0.10},
                      {MB_CUBE_IHO_EDITION_6, "exclusive", 0.15, 0.0075, 1.0, 0.0},
                      {MB_CUBE_IHO_EDITION_6, "special", 0.25, 0.0075, 2.0, 0.0},
                      {MB_CUBE_IHO_EDITION_6, "1a", 0.5, 0.013, 5.0, 0.05},
                      {MB_CUBE_IHO_EDITION_6, "1b", 0.5, 0.013, 5.0, 0.05},
                      {MB_CUBE_IHO_EDITION_6, "2", 1.0, 0.023, 20.0, 0.10},
                      {MB_CUBE_IHO_CCOM, "special", 0.2, 0.0075, 2.0, 0.0},
                      {MB_CUBE_IHO_CCOM, "1", 0.5, 0.013, 5.0, 0.0},
                      {MB_CUBE_IHO_CCOM, "2", 1.0, 0.023, 20.0, 0.0},
                      {MB_CUBE_IHO_CCOM, "3", 1.0, 0.023, 150.0, 0.0}};
  for (const Row &r : rows) {
    int error = MB_ERROR_NO_ERROR;
    double a, b, thu, thu_fraction;
    ASSERT_EQ(MB_SUCCESS, mb_cube_iho_edition_limits(verbose, r.edition, r.order, &a, &b, &thu, &thu_fraction, &error))
        << "edition " << r.edition << " order " << r.order;
    EXPECT_EQ(MB_ERROR_NO_ERROR, error);
    EXPECT_DOUBLE_EQ(r.a, a) << "edition " << r.edition << " order " << r.order;
    EXPECT_DOUBLE_EQ(r.b, b) << "edition " << r.edition << " order " << r.order;
    EXPECT_DOUBLE_EQ(r.thu, thu) << "edition " << r.edition << " order " << r.order;
    EXPECT_DOUBLE_EQ(r.thu_fraction, thu_fraction) << "edition " << r.edition << " order " << r.order;
  }
}

TEST(MbCubeParam, IhoEditionRejectsOrdersOfOtherEditions) {
  struct Bad {
    int edition;
    const char *order;
  };
  const Bad bad[] = {{MB_CUBE_IHO_EDITION_4, "1a"}, {MB_CUBE_IHO_EDITION_4, "exclusive"}, {MB_CUBE_IHO_EDITION_5, "1"},
                     {MB_CUBE_IHO_EDITION_5, "3"},  {MB_CUBE_IHO_EDITION_5, "exclusive"}, {MB_CUBE_IHO_EDITION_6, "1"},
                     {MB_CUBE_IHO_EDITION_6, "3"},  {MB_CUBE_IHO_CCOM, "1a"},             {MB_CUBE_IHO_CCOM, "exclusive"},
                     {MB_CUBE_IHO_CCOM, "4"},       {3, "2"},                             {7, "2"},
                     {MB_CUBE_IHO_EDITION_6, ""}};
  for (const Bad &x : bad) {
    int error = MB_ERROR_NO_ERROR;
    double a = -1.0, b = -1.0, thu = -1.0, thu_fraction = -1.0;
    EXPECT_EQ(MB_FAILURE, mb_cube_iho_edition_limits(verbose, x.edition, x.order, &a, &b, &thu, &thu_fraction, &error))
        << "edition " << x.edition << " order " << x.order;
    EXPECT_EQ(MB_ERROR_BAD_PARAMETER, error);
    EXPECT_DOUBLE_EQ(-1.0, a);
  }
  int error = MB_ERROR_NO_ERROR;
  double a, b, thu, thu_fraction;
  EXPECT_EQ(MB_FAILURE, mb_cube_iho_edition_limits(verbose, MB_CUBE_IHO_EDITION_6, nullptr, &a, &b, &thu, &thu_fraction,
                                                   &error));
}

TEST(MbCubeParam, IhoEditionOrders) {
  int error = MB_ERROR_NO_ERROR;
  char orders[64];
  ASSERT_EQ(MB_SUCCESS, mb_cube_iho_edition_orders(verbose, MB_CUBE_IHO_EDITION_4, orders, sizeof(orders), &error));
  EXPECT_STREQ("special|1|2|3", orders);
  ASSERT_EQ(MB_SUCCESS, mb_cube_iho_edition_orders(verbose, MB_CUBE_IHO_EDITION_5, orders, sizeof(orders), &error));
  EXPECT_STREQ("special|1a|1b|2", orders);
  ASSERT_EQ(MB_SUCCESS, mb_cube_iho_edition_orders(verbose, MB_CUBE_IHO_EDITION_6, orders, sizeof(orders), &error));
  EXPECT_STREQ("exclusive|special|1a|1b|2", orders);
  ASSERT_EQ(MB_SUCCESS, mb_cube_iho_edition_orders(verbose, MB_CUBE_IHO_CCOM, orders, sizeof(orders), &error));
  EXPECT_STREQ("special|1|2|3", orders);
  EXPECT_EQ(MB_FAILURE, mb_cube_iho_edition_orders(verbose, 3, orders, sizeof(orders), &error));
  EXPECT_EQ(MB_ERROR_BAD_PARAMETER, error);
  char small[8];
  EXPECT_EQ(MB_FAILURE, mb_cube_iho_edition_orders(verbose, MB_CUBE_IHO_EDITION_6, small, sizeof(small), &error));
}

// edition 6 by name gives the same limits as mb_cube_iho_limits() by number
TEST(MbCubeParam, IhoEdition6MatchesIhoLimits) {
  const int numbers[] = {MB_CUBE_IHO_EXCLUSIVE, MB_CUBE_IHO_SPECIAL, MB_CUBE_IHO_ORDER_1A, MB_CUBE_IHO_ORDER_1B,
                         MB_CUBE_IHO_ORDER_2};
  const char *names[] = {"exclusive", "special", "1a", "1b", "2"};
  for (int k = 0; k < 5; k++) {
    int error = MB_ERROR_NO_ERROR;
    double a1, b1, t1, f1, a2, b2, t2, f2;
    ASSERT_EQ(MB_SUCCESS, mb_cube_iho_limits(verbose, numbers[k], &a1, &b1, &t1, &f1, &error));
    ASSERT_EQ(MB_SUCCESS, mb_cube_iho_edition_limits(verbose, MB_CUBE_IHO_EDITION_6, names[k], &a2, &b2, &t2, &f2, &error));
    EXPECT_DOUBLE_EQ(a1, a2);
    EXPECT_DOUBLE_EQ(b1, b2);
    EXPECT_DOUBLE_EQ(t1, t2);
    EXPECT_DOUBLE_EQ(f1, f2);
  }
}

TEST(MbCubeParam, DefaultsAndChecks) {
  int error = MB_ERROR_NO_ERROR;
  struct mb_cube_param_struct param;
  ASSERT_EQ(MB_SUCCESS, mb_cube_default_param(verbose, MB_CUBE_IHO_ORDER_2, &param, &error));
  EXPECT_EQ(MB_CUBE_EXTRACT_PRIOR, param.extractor);
  EXPECT_EQ(MB_CUBE_UNCT_SAMPLE, param.uncertainty_mode);
  EXPECT_EQ(11, param.median_length);
  EXPECT_DOUBLE_EQ(4.0, param.est_offset);
  EXPECT_DOUBLE_EQ(0.135, param.bayes_fac_t);
  EXPECT_EQ(5, param.runlength_t);
  EXPECT_DOUBLE_EQ(0.05, param.capture_scale);
  EXPECT_DOUBLE_EQ(0.5, param.capture_min);
  EXPECT_DOUBLE_EQ(1.96, param.horiz_scale);
  EXPECT_DOUBLE_EQ(2.95, param.influence_scale);
  EXPECT_EQ(MB_SUCCESS, mb_cube_check_param(verbose, &param, &error));

  struct mb_cube_param_struct bad = param;
  bad.median_length = 10;  // must be odd
  EXPECT_EQ(MB_FAILURE, mb_cube_check_param(verbose, &bad, &error));
  EXPECT_EQ(MB_ERROR_BAD_PARAMETER, error);
  bad = param;
  bad.discount = NAN;
  EXPECT_EQ(MB_FAILURE, mb_cube_check_param(verbose, &bad, &error));
  bad = param;
  bad.max_context = 1.0;  // below min_context
  EXPECT_EQ(MB_FAILURE, mb_cube_check_param(verbose, &bad, &error));

  void *cube = nullptr;
  EXPECT_EQ(MB_FAILURE, mb_cube_init(verbose, &param, 0, 5, 1.0, 1.0, &cube, &error));
  EXPECT_EQ(MB_ERROR_BAD_PARAMETER, error);
  EXPECT_EQ(nullptr, cube);
  EXPECT_EQ(MB_FAILURE, mb_cube_init(verbose, &param, 5, 5, 0.0, 1.0, &cube, &error));
  EXPECT_EQ(nullptr, cube);
}

TEST_F(MbCube, InsertRejectsBadValues) {
  Init(3, 3, 1.0, 1.0);
  int error = MB_ERROR_NO_ERROR;
  int nused = 0;
  EXPECT_EQ(MB_FAILURE, mb_cube_insert(verbose, cube_, 1.0, 1.0, 100.0, 100.0, 0.0, 0.01, &nused, &error));
  EXPECT_EQ(MB_ERROR_BAD_PARAMETER, error);
  EXPECT_EQ(MB_FAILURE, mb_cube_insert(verbose, cube_, NAN, 1.0, 100.0, 100.0, 0.01, 0.01, &nused, &error));
  EXPECT_EQ(MB_FAILURE, mb_cube_insert(verbose, cube_, 1.0, 1.0, 100.0, 100.0, 0.01, -1.0, &nused, &error));
  EXPECT_EQ(0, nused);
  // outside the grid is not an error
  EXPECT_EQ(0, Insert(-50.0, 1.0, 100.0, 0.01, 0.0001));
  EXPECT_EQ(0, Insert(1.0, 50.0, 100.0, 0.01, 0.0001));
}

TEST_F(MbCube, EmptyGridHasNoData) {
  Init(4, 3, 1.0, 1.0);
  Flush();
  Extract();
  for (size_t k = 0; k < depth_.size(); k++) {
    EXPECT_EQ(kNoData, depth_[k]);
    EXPECT_EQ(kNoData, unct_[k]);
    EXPECT_EQ(kNoData, strength_[k]);
    EXPECT_EQ(0.0f, nhyp_[k]);
    EXPECT_EQ(0, nsamples_[k]);
  }
}

TEST_F(MbCube, QueueHoldsSamplesUntilFullOrFlushed) {
  Init(1, 1, 1.0, 1.0);
  int error = MB_ERROR_NO_ERROR;
  int nhyp = 0;
  double d[4], u[4];
  int n[4];
  for (int k = 0; k < param_.median_length; k++)
    EXPECT_EQ(1, Insert(0.0, 0.0, 100.0 + 0.001 * k, 0.01, 0.0));
  ASSERT_EQ(MB_SUCCESS, mb_cube_node_hypotheses(verbose, cube_, 0, 0, 4, &nhyp, d, u, n, &error));
  EXPECT_EQ(0, nhyp);  // all samples still queued

  EXPECT_EQ(1, Insert(0.0, 0.0, 100.0, 0.01, 0.0));  // pushes out the median
  ASSERT_EQ(MB_SUCCESS, mb_cube_node_hypotheses(verbose, cube_, 0, 0, 4, &nhyp, d, u, n, &error));
  ASSERT_EQ(1, nhyp);
  EXPECT_EQ(1, n[0]);
  EXPECT_NEAR(100.005, d[0], 1.0e-4);

  Flush();
  ASSERT_EQ(MB_SUCCESS, mb_cube_node_hypotheses(verbose, cube_, 0, 0, 4, &nhyp, d, u, n, &error));
  ASSERT_EQ(1, nhyp);
  EXPECT_EQ(param_.median_length + 1, n[0]);
}

TEST_F(MbCube, KalmanUpdateOfTwoSamples) {
  param_.uncertainty_mode = MB_CUBE_UNCT_POSTERIOR;
  Init(1, 1, 1.0, 1.0);
  const double v = 0.04;
  EXPECT_EQ(1, Insert(0.0, 0.0, 100.0, v, 0.0));
  EXPECT_EQ(1, Insert(0.0, 0.0, 100.2, v, 0.0));
  Flush();
  Extract();
  // equal variances at zero distance: the estimate is the mean and the
  // posterior variance is halved
  EXPECT_NEAR(100.1, depth_[0], 1.0e-4);
  EXPECT_NEAR(1.96 * std::sqrt(v / 2.0), unct_[0], 1.0e-5);
  EXPECT_EQ(1.0f, nhyp_[0]);
  EXPECT_EQ(0.0f, strength_[0]);
  EXPECT_EQ(2, nsamples_[0]);
}

TEST_F(MbCube, UncertaintyModes) {
  Init(1, 1, 1.0, 1.0);
  const double v = 0.04;
  Insert(0.0, 0.0, 100.0, v, 0.0);
  Insert(0.0, 0.0, 100.2, v, 0.0);
  Flush();
  int error = MB_ERROR_NO_ERROR;
  int nhyp = 0;
  double d[2], u[2];
  int n[2];
  ASSERT_EQ(MB_SUCCESS, mb_cube_node_hypotheses(verbose, cube_, 0, 0, 2, &nhyp, d, u, n, &error));
  // sample mode (default): running variance of the samples, here the
  // squared difference of the two samples
  EXPECT_NEAR(1.96 * 0.2, u[0], 1.0e-4);
}

TEST_F(MbCube, SingleSampleSampleVarianceIsZero) {
  Init(1, 1, 1.0, 1.0);
  Insert(0.0, 0.0, 100.0, 0.04, 0.0);
  Flush();
  Extract();
  EXPECT_FLOAT_EQ(100.0f, depth_[0]);
  EXPECT_EQ(0.0f, unct_[0]);
  EXPECT_EQ(1, nsamples_[0]);
}

TEST_F(MbCube, VarianceGrowsWithDistanceFromNode) {
  param_.uncertainty_mode = MB_CUBE_UNCT_POSTERIOR;
  param_.capture_min = 5.0;
  Init(1, 1, 1.0, 1.0);
  const double v = 0.01;
  // radius is at least the node spacing (1 m): a sounding 0.5 m away with
  // no horizontal uncertainty has variance v * (1 + 0.5^2)
  EXPECT_EQ(1, Insert(0.5, 0.0, 100.0, v, 0.0));
  Flush();
  Extract();
  EXPECT_NEAR(1.96 * std::sqrt(v * 1.25), unct_[0], 1.0e-5);
}

TEST_F(MbCube, CaptureDistanceUsesDepthBelowSonar) {
  Init(2, 1, 1.0, 1.0);
  // horizontal sigma of 1 m gives an influence radius of 2.95 m; node 1 is
  // 0.7 m away. Capture distance is max(0.05 * h, 0.5 m).
  EXPECT_EQ(1, Insert(0.3, 0.0, 3000.0, 0.01, 1.0, 10.0));  // AUV 10 m above the seafloor
  EXPECT_EQ(2, Insert(0.3, 0.0, 3000.0, 0.01, 1.0, 100.0));
}

TEST_F(MbCube, InfluenceRadiusLimitedByHorizontalUncertainty) {
  Init(5, 1, 1.0, 1.0);
  // with no horizontal uncertainty only nodes closer than the node spacing
  // receive the sounding
  EXPECT_EQ(1, Insert(2.0, 0.0, 100.0, 0.0001, 0.0));
  // with a horizontal sigma of 0.5 m the radius is 2.95 * 0.5 = 1.475 m
  EXPECT_EQ(3, Insert(2.0, 0.0, 100.0, 0.0001, 0.25));
}

TEST_F(MbCube, InfluenceRadiusReachesDiagonalNodes) {
  Init(5, 5, 2.0, 2.0);
  // horizontal sigma 1 m: radius 2.95 m reaches the 4 nodes at 2 m and the
  // 4 diagonal nodes at 2.83 m (found by the reference comparison)
  EXPECT_EQ(9, Insert(4.0, 4.0, 100.0, 0.0001, 1.0));
}

TEST_F(MbCube, FlatNoisySurface) {
  Init(10, 10, 1.0, 1.0);
  std::mt19937 rng(12345);
  std::uniform_real_distribution<double> pos(-0.5, 9.5);
  std::normal_distribution<double> noise(0.0, 0.05);
  for (int k = 0; k < 5000; k++)
    Insert(pos(rng), pos(rng), 100.0 + noise(rng), 0.05 * 0.05, 0.1 * 0.1);
  Flush();
  Extract();
  // With uncertainties equal to the true noise, the most extreme samples,
  // which the median queue holds back until the flush, can start small
  // extra hypotheses (a few samples each); they must not win or weaken
  // the selection.
  int error = MB_ERROR_NO_ERROR;
  for (int i = 0; i < nx_; i++)
    for (int j = 0; j < ny_; j++) {
      const size_t k = K(i, j);
      ASSERT_GT(nsamples_[k], 0);
      EXPECT_NEAR(100.0, depth_[k], 0.05);
      EXPECT_EQ(0.0f, strength_[k]);
      int nhyp = 0;
      double d[16], u[16];
      int n[16];
      ASSERT_EQ(MB_SUCCESS, mb_cube_node_hypotheses(verbose, cube_, i, j, 16, &nhyp, d, u, n, &error));
      int total = 0;
      for (int h = 0; h < std::min(nhyp, 16); h++)
        total += n[h];
      EXPECT_GE(nsamples_[k], 0.95 * total);
    }
}

TEST_F(MbCube, SpikeDoesNotReachSurface) {
  Init(1, 1, 1.0, 1.0);
  std::mt19937 rng(54321);
  std::normal_distribution<double> noise(0.0, 0.02);
  for (int k = 0; k < 30; k++) {
    Insert(0.0, 0.0, 100.0 + noise(rng), 0.02 * 0.02, 0.0);
    if (k == 15)
      Insert(0.0, 0.0, 150.0, 0.02 * 0.02, 0.0);
  }
  Flush();
  Extract();
  EXPECT_NEAR(100.0, depth_[0], 0.02);
}

TEST_F(MbCube, StepGivesTwoHypothesesAndPriorPicksTheLarger) {
  Init(1, 1, 1.0, 1.0);
  for (int k = 0; k < 25; k++)
    Insert(0.0, 0.0, 100.0, 0.01, 0.0);
  for (int k = 0; k < 15; k++)
    Insert(0.0, 0.0, 110.0, 0.01, 0.0);
  Flush();
  Extract();
  EXPECT_EQ(2.0f, nhyp_[0]);
  EXPECT_FLOAT_EQ(100.0f, depth_[0]);
  EXPECT_GT(strength_[0], 0.0f);
  EXPECT_LT(strength_[0], 5.0f);

  int error = MB_ERROR_NO_ERROR;
  int nhyp = 0;
  double d[4], u[4];
  int n[4];
  ASSERT_EQ(MB_SUCCESS, mb_cube_node_hypotheses(verbose, cube_, 0, 0, 4, &nhyp, d, u, n, &error));
  ASSERT_EQ(2, nhyp);
  // the first deep sample meets a queue of identical shallow samples and is
  // removed by the quotient test; all others are assimilated
  EXPECT_EQ(39, n[0] + n[1]);
  const int n100 = d[0] < 105.0 ? n[0] : n[1];
  const int n110 = d[0] < 105.0 ? n[1] : n[0];
  EXPECT_EQ(25, n100);
  EXPECT_EQ(14, n110);
  EXPECT_NEAR(5.0 - 25.0 / 14.0, strength_[0], 1.0e-5);
}

TEST_F(MbCube, QuotientTestRemovesLoneOutlier) {
  param_.median_length = 5;
  Init(1, 1, 1.0, 1.0);
  for (int k = 0; k < 4; k++)
    Insert(0.0, 0.0, 100.0 + 0.01 * k, 0.01, 0.0);
  Insert(0.0, 0.0, 130.0, 0.01, 0.0);
  Flush();
  int error = MB_ERROR_NO_ERROR;
  int nhyp = 0;
  double d[2], u[2];
  int n[2];
  ASSERT_EQ(MB_SUCCESS, mb_cube_node_hypotheses(verbose, cube_, 0, 0, 2, &nhyp, d, u, n, &error));
  ASSERT_EQ(1, nhyp);
  EXPECT_EQ(4, n[0]);
  EXPECT_NEAR(100.015, d[0], 1.0e-4);
}

TEST_F(MbCube, PriorTieGoesToOldestHypothesis) {
  Init(1, 1, 1.0, 1.0);
  for (int k = 0; k < 20; k++)
    Insert(0.0, 0.0, 100.0, 0.01, 0.0);
  for (int k = 0; k < 21; k++)
    Insert(0.0, 0.0, 110.0, 0.01, 0.0);
  Flush();
  Extract();
  // the first deep sample is removed by the quotient test, leaving 20 and 20
  int error = MB_ERROR_NO_ERROR;
  int nhyp = 0;
  double d[2], u[2];
  int n[2];
  ASSERT_EQ(MB_SUCCESS, mb_cube_node_hypotheses(verbose, cube_, 0, 0, 2, &nhyp, d, u, n, &error));
  ASSERT_EQ(2, nhyp);
  ASSERT_EQ(n[0], n[1]);
  EXPECT_FLOAT_EQ(100.0f, depth_[0]);
  EXPECT_NEAR(4.0, strength_[0], 1.0e-5);
}

TEST_F(MbCube, SlowDriftStartsNewHypothesis) {
  param_.median_length = 3;
  Init(1, 1, 1.0, 1.0);
  // increasing depths leave the queue in order; the drift of 2 sigma per
  // sample is caught by the cumulative Bayes factor or the run length
  for (int k = 0; k < 40; k++)
    Insert(0.0, 0.0, 100.0 + 0.02 * k, 0.01 * 0.01, 0.0);
  Flush();
  Extract();
  EXPECT_GE(nhyp_[0], 2.0f);
}

// Predicted surface on a small grid: depth 100 m plus 1 m per meter east
class MbCubePredicted : public MbCube {
 protected:
  void Predict(double variance, int initial_hypotheses, float nodata_node = -1.0f) {
    std::vector<float> pd(static_cast<size_t>(nx_) * ny_), pv(pd.size());
    for (int i = 0; i < nx_; i++)
      for (int j = 0; j < ny_; j++) {
        pd[K(i, j)] = 100.0f + i;
        pv[K(i, j)] = static_cast<float>(variance);
      }
    if (nodata_node >= 0.0f)
      pd[static_cast<size_t>(nodata_node)] = kNoData;
    int error = MB_ERROR_NO_ERROR;
    ASSERT_EQ(MB_SUCCESS, mb_cube_set_prediction(verbose, cube_, pd.data(), pv.data(), kNoData, 0.0, initial_hypotheses, &error));
  }
};

TEST_F(MbCubePredicted, InitialHypothesesAloneAreNotReported) {
  Init(2, 2, 1.0, 1.0);
  Predict(1.0, 1);
  Flush();
  Extract();
  for (size_t k = 0; k < depth_.size(); k++) {
    EXPECT_EQ(kNoData, depth_[k]);
    EXPECT_EQ(0.0f, nhyp_[k]);
  }
}

TEST_F(MbCubePredicted, InitialHypothesisActsAsPrior) {
  param_.uncertainty_mode = MB_CUBE_UNCT_SAMPLE;
  Init(1, 1, 1.0, 1.0);
  Predict(0.01, 1);
  Insert(0.0, 0.0, 100.2, 0.01, 0.0);
  Flush();
  Extract();
  // equal variances: the estimate is halfway between prediction and sounding,
  // and the sample variance of the first sounding is 0 (not NaN as in the
  // reference)
  EXPECT_NEAR(100.1, depth_[0], 1.0e-4);
  EXPECT_EQ(1.0f, nhyp_[0]);
  EXPECT_EQ(1, nsamples_[0]);
  EXPECT_EQ(0.0f, unct_[0]);
}

TEST_F(MbCubePredicted, BlunderTestRejectsOnlyDeepSoundings) {
  Init(1, 1, 1.0, 1.0);
  Predict(0.01, 0);
  for (int k = 0; k < 5; k++)
    Insert(0.0, 0.0, 100.0, 0.01, 0.0);
  // pairs, so that the queue outlier test does not remove them first
  for (int k = 0; k < 2; k++) {
    Insert(0.0, 0.0, 130.0, 0.01, 0.0);  // deeper than 100 + max(10, 25, 0.3) m
    Insert(0.0, 0.0, 70.0, 0.01, 0.0);   // shallow soundings are never blunders
  }
  Flush();
  int error = MB_ERROR_NO_ERROR;
  int nhyp = 0;
  double d[4], u[4];
  int n[4];
  ASSERT_EQ(MB_SUCCESS, mb_cube_node_hypotheses(verbose, cube_, 0, 0, 4, &nhyp, d, u, n, &error));
  ASSERT_EQ(2, nhyp);
  EXPECT_EQ(7, n[0] + n[1]);
  for (int h = 0; h < nhyp; h++)
    EXPECT_LT(d[h], 120.0);
}

TEST_F(MbCubePredicted, BlunderTestCanBeSwitchedOff) {
  param_.blunder = 0;
  Init(1, 1, 1.0, 1.0);
  Predict(0.01, 0);
  for (int k = 0; k < 5; k++)
    Insert(0.0, 0.0, 100.0, 0.01, 0.0);
  Insert(0.0, 0.0, 130.0, 0.01, 0.0);
  Insert(0.0, 0.0, 130.0, 0.01, 0.0);
  Flush();
  Extract();
  EXPECT_EQ(2.0f, nhyp_[0]);
}

TEST_F(MbCubePredicted, SlopeCorrection) {
  Init(2, 2, 1.0, 1.0);
  Predict(0.01, 0);
  // the predicted surface is 100.5 m at (0.5, 0.5); each node receives the
  // sounding moved by its predicted depth difference
  EXPECT_EQ(4, Insert(0.5, 0.5, 100.5, 0.01, 0.25));
  Flush();
  Extract();
  EXPECT_NEAR(100.0, depth_[K(0, 0)], 1.0e-4);
  EXPECT_NEAR(101.0, depth_[K(1, 0)], 1.0e-4);
  EXPECT_NEAR(100.0, depth_[K(0, 1)], 1.0e-4);
  EXPECT_NEAR(101.0, depth_[K(1, 1)], 1.0e-4);
}

TEST_F(MbCubePredicted, NoSlopeCorrectionOnSouthernEdgeOrWithoutFullCell) {
  Init(2, 2, 1.0, 1.0);
  Predict(0.01, 0, 3.0f);  // node (1,1) has no prediction
  // inside a cell with a missing prediction: no correction at any node
  Insert(0.5, 0.5, 100.5, 0.01, 0.25);
  Flush();
  Extract();
  EXPECT_NEAR(100.5, depth_[K(0, 0)], 1.0e-4);
  EXPECT_NEAR(100.5, depth_[K(1, 0)], 1.0e-4);
}

TEST_F(MbCubePredicted, PredictedRuleChoosesHypothesisClosestToSurface) {
  for (int extractor : {MB_CUBE_EXTRACT_PRIOR, MB_CUBE_EXTRACT_PREDICTED}) {
    param_.extractor = extractor;
    Init(1, 1, 1.0, 1.0);
    Predict(1.0, 0);
    for (int k = 0; k < 10; k++)
      Insert(0.0, 0.0, 80.0, 0.01, 0.0);
    for (int k = 0; k < 5; k++)
      Insert(0.0, 0.0, 100.0, 0.01, 0.0);
    Flush();
    Extract();
    ASSERT_EQ(2.0f, nhyp_[0]);
    if (extractor == MB_CUBE_EXTRACT_PRIOR) {
      EXPECT_FLOAT_EQ(80.0f, depth_[0]);
    }
    else {
      EXPECT_FLOAT_EQ(100.0f, depth_[0]);
      EXPECT_NEAR(5.0 - 5.0 / 10.0, strength_[0], 1.0e-5);
    }
    int error = MB_ERROR_NO_ERROR;
    mb_cube_deall(verbose, &cube_, &error);
  }
}

// AUV 50 m above a seafloor predicted at 3000 m: 5% of the depth below the
// sonar is 2.5 m (95%), 5% of the depth below the sea surface is 150 m
class MbCubeAuvPrediction : public MbCube {
 protected:
  void Predict(bool below_sonar) {
    float pd = 3000.0f;
    int error = MB_ERROR_NO_ERROR;
    if (below_sonar) {
      ASSERT_EQ(MB_SUCCESS, mb_cube_set_prediction(verbose, cube_, &pd, nullptr, kNoData, 0.05, 1, &error));
    }
    else {
      const double sd = 0.05 * 3000.0 / 1.96;
      float pv = static_cast<float>(sd * sd);
      ASSERT_EQ(MB_SUCCESS, mb_cube_set_prediction(verbose, cube_, &pd, &pv, kNoData, 0.0, 1, &error));
    }
  }
  // sounding from a sonar 2950 m deep
  void InsertAuv(double depth, double var_depth) { Insert(0.0, 0.0, depth, var_depth, 0.0, depth - 2950.0); }
};

TEST_F(MbCubeAuvPrediction, VarianceFromDepthBelowSonar) {
  // the initial hypothesis has the variance of the sounding (sigma 0.05 *
  // 50 m / 1.96, from the predicted depth below the sonar), so the estimate
  // is halfway between prediction and sounding
  const double sd = 0.05 * 50.0 / 1.96;
  for (bool below_sonar : {true, false}) {
    Init(1, 1, 1.0, 1.0);
    Predict(below_sonar);
    InsertAuv(3000.4, sd * sd);
    Flush();
    Extract();
    if (below_sonar)
      EXPECT_NEAR(3000.2, depth_[0], 1.0e-3);
    else
      EXPECT_NEAR(3000.4, depth_[0], 1.0e-3);
    int error = MB_ERROR_NO_ERROR;
    mb_cube_deall(verbose, &cube_, &error);
  }
}

TEST_F(MbCubeAuvPrediction, BlunderMarginFromDepthBelowSonar) {
  // margin max(10 m, 0.25 * 50 m, 3 sigma): 12.5 m below the sonar, but 3 *
  // 76.5 m with the variance from the depth below the sea surface
  for (bool below_sonar : {true, false}) {
    Init(1, 1, 1.0, 1.0);
    Predict(below_sonar);
    for (int k = 0; k < 5; k++)
      InsertAuv(3000.0, 0.01);
    InsertAuv(3015.0, 0.01);
    InsertAuv(3015.0, 0.01);
    Flush();
    Extract();
    EXPECT_EQ(below_sonar ? 1.0f : 2.0f, nhyp_[0]);
    int error = MB_ERROR_NO_ERROR;
    mb_cube_deall(verbose, &cube_, &error);
  }
}

TEST_F(MbCubePredicted, PendingPredictionGivesSlopeCorrection) {
  Init(2, 2, 1.0, 1.0);
  std::vector<float> pd(4);
  for (int i = 0; i < 2; i++)
    for (int j = 0; j < 2; j++)
      pd[K(i, j)] = 100.0f + i;
  int error = MB_ERROR_NO_ERROR;
  ASSERT_EQ(MB_SUCCESS, mb_cube_set_prediction(verbose, cube_, pd.data(), nullptr, kNoData, 0.05, 0, &error));
  EXPECT_EQ(4, Insert(0.5, 0.5, 100.5, 0.01, 0.25));
  Flush();
  Extract();
  EXPECT_NEAR(100.0, depth_[K(0, 0)], 1.0e-4);
  EXPECT_NEAR(101.0, depth_[K(1, 1)], 1.0e-4);
}

TEST_F(MbCubePredicted, SonarFractionMustBeValid) {
  Init(1, 1, 1.0, 1.0);
  float pd = 100.0f;
  int error = MB_ERROR_NO_ERROR;
  EXPECT_EQ(MB_FAILURE, mb_cube_set_prediction(verbose, cube_, &pd, nullptr, kNoData, 0.0, 1, &error));
  EXPECT_EQ(MB_ERROR_BAD_PARAMETER, error);
}

TEST_F(MbCubePredicted, PredictionOnlyBeforeData) {
  Init(1, 1, 1.0, 1.0);
  Insert(0.0, 0.0, 100.0, 0.01, 0.0);
  float pd = 100.0f, pv = 1.0f;
  int error = MB_ERROR_NO_ERROR;
  EXPECT_EQ(MB_FAILURE, mb_cube_set_prediction(verbose, cube_, &pd, &pv, kNoData, 0.0, 1, &error));
  EXPECT_EQ(MB_ERROR_BAD_PARAMETER, error);
}

// Five nodes in a row with 5 seabed samples each; the middle node also has
// a burst of 10 samples from a layer 20 m shallower.
class MbCubeBurst : public MbCube {
 protected:
  void Run(int extractor) {
    param_.extractor = extractor;
    param_.min_context = 0.0;
    param_.max_context = 3.0;
    Init(5, 1, 1.0, 1.0);
    for (int k = 0; k < 10; k++)
      Insert(2.0, 0.0, 80.0, 0.01, 0.0);
    for (int i = 0; i < 5; i++)
      for (int k = 0; k < 5; k++)
        Insert(i, 0.0, 100.0, 0.01, 0.0);
    Flush();
    Extract();
    ASSERT_EQ(2.0f, nhyp_[K(2, 0)]);
    for (int i = 0; i < 5; i++)
      if (i != 2)
        ASSERT_EQ(1.0f, nhyp_[K(i, 0)]);
  }
};

TEST_F(MbCubeBurst, PriorFollowsTheBurst) {
  Run(MB_CUBE_EXTRACT_PRIOR);
  EXPECT_FLOAT_EQ(80.0f, depth_[K(2, 0)]);
  EXPECT_NEAR(5.0 - 10.0 / 5.0, strength_[K(2, 0)], 1.0e-5);
}

TEST_F(MbCubeBurst, LikelihoodFollowsTheNeighbours) {
  Run(MB_CUBE_EXTRACT_LIKELIHOOD);
  EXPECT_FLOAT_EQ(100.0f, depth_[K(2, 0)]);
  EXPECT_NEAR(5.0 - 5.0 / 10.0, strength_[K(2, 0)], 1.0e-5);
  EXPECT_EQ(5, nsamples_[K(2, 0)]);
}

TEST_F(MbCubeBurst, PosteriorFollowsTheNeighbours) {
  Run(MB_CUBE_EXTRACT_POSTERIOR);
  EXPECT_FLOAT_EQ(100.0f, depth_[K(2, 0)]);
}

TEST_F(MbCube, LikelihoodGuideFollowsReferenceScanOrder) {
  // ring of 2 nodes around (2,2): the guide at (2,4) in the northern row is
  // nearer than the one at (4,3) in the eastern column, but the reference
  // scans the columns last and uses the last guide found. Neither the prior
  // rule (100 m has more samples) nor the nearest guide would give 80 m.
  param_.extractor = MB_CUBE_EXTRACT_LIKELIHOOD;
  param_.min_context = 2.0;
  param_.max_context = 2.0;
  Init(5, 5, 1.0, 1.0);
  for (int k = 0; k < 10; k++)
    Insert(2.0, 2.0, 100.0, 0.01, 0.0);
  for (int k = 0; k < 5; k++) {
    Insert(2.0, 2.0, 80.0, 0.01, 0.0);
    Insert(2.0, 4.0, 100.0, 0.01, 0.0);
    Insert(4.0, 3.0, 80.0, 0.01, 0.0);
  }
  Flush();
  Extract();
  ASSERT_EQ(2.0f, nhyp_[K(2, 2)]);
  EXPECT_FLOAT_EQ(80.0f, depth_[K(2, 2)]);
}

TEST_F(MbCubeBurst, LikelihoodWithoutGuideFallsBackToPrior) {
  param_.extractor = MB_CUBE_EXTRACT_LIKELIHOOD;
  param_.min_context = 0.0;
  param_.max_context = 0.0;  // only the node itself is searched
  Init(5, 1, 1.0, 1.0);
  for (int k = 0; k < 10; k++)
    Insert(2.0, 0.0, 80.0, 0.01, 0.0);
  for (int k = 0; k < 5; k++)
    Insert(2.0, 0.0, 100.0, 0.01, 0.0);
  Flush();
  Extract();
  EXPECT_FLOAT_EQ(80.0f, depth_[K(2, 0)]);
}

}  // namespace

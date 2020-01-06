#include <cstdio>
#include <cstdlib>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {

TEST(MbReadInitTest, errorBadFormat) {
  const int verbose = 0;
  const int format = 0;
  const int pings_get = 1;
  const int lonflip = 0;
  double bounds[4] = {0.0, 0.0, 0.0, 0.0};
  int btime_i[7] = {0, 0, 0, 0, 0, 0, 0};
  int etime_i[7] = {0, 0, 0, 0, 0, 0, 0};
  const double speedmin = 0.0;
  const double timegap = 0.0;
  char file[MB_PATH_MAXLINE] = "/does/not/exist";
  void *mbio_ptr = nullptr;
  double btime_d = 0.0;
  double etime_d = 0.0;
  int beams_bath_alloc = 0;
  int beams_amp_alloc = 0;
  int pixels_ss_alloc = 0;
  int error = MB_ERROR_NO_ERROR;

  ASSERT_EQ(MB_FAILURE,
            mb_read_init(verbose, file, format, pings_get, lonflip, bounds,
                         btime_i, etime_i, speedmin, timegap, &mbio_ptr,
                         &btime_d, &etime_d, &beams_bath_alloc,
                         &beams_amp_alloc, &pixels_ss_alloc, &error));
  ASSERT_EQ(MB_ERROR_BAD_FORMAT, error);
}

// TODO(schwehr): Causes a segfault.  See:
// https://github.com/dwcaress/MB-System/pull/795
TEST(MbReadInitTest, DISABLED_errorFileDoesNotExist) {
  const int verbose = 0;
  const int format = MBF_SBSIOMRG;
  const int pings_get = 1;
  const int lonflip = 0;
  double bounds[4] = {0.0, 0.0, 0.0, 0.0};
  int btime_i[7] = {0, 0, 0, 0, 0, 0, 0};
  int etime_i[7] = {0, 0, 0, 0, 0, 0, 0};
  const double speedmin = 0.0;
  const double timegap = 0.0;
  char file[MB_PATH_MAXLINE] = "/does/not/exist";
  void *mbio_ptr = nullptr;
  double btime_d = 0.0;
  double etime_d = 0.0;
  int beams_bath_alloc = 0;
  int beams_amp_alloc = 0;
  int pixels_ss_alloc = 0;
  int error = MB_ERROR_NO_ERROR;

  ASSERT_EQ(MB_FAILURE,
            mb_read_init(verbose, file, format, pings_get, lonflip, bounds,
                         btime_i, etime_i, speedmin, timegap, &mbio_ptr,
                         &btime_d, &etime_d, &beams_bath_alloc,
                         &beams_amp_alloc, &pixels_ss_alloc, &error));
  ASSERT_EQ(MB_ERROR_OPEN_FAIL, error);
}

}  // namespace

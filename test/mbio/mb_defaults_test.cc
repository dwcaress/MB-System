// Copyright 2020 Google Inc. All Rights Reserved.
//
// See README file for copying and redistribution conditions.

#include <cstdlib>

#include "mb_define.h"
#include "mb_status.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {

TEST(MbVersion, Basic) {
  int error = MB_ERROR_NO_ERROR;
  int verbose = 0;
  mb_path version_string;
  int version_id;
  int version_major;
  int version_minor;
  int version_archive;

  EXPECT_EQ(MB_SUCCESS,
            mb_version(verbose, version_string, &version_id, &version_major,
                       &version_minor, &version_archive, &error));
  EXPECT_EQ(MB_ERROR_NO_ERROR, error);
  EXPECT_THAT(version_string,
              testing::ContainsRegex("^[0-9]+[.][0-9]+[.][0-9]+"));
  EXPECT_GT(version_id, 50700005);
  EXPECT_LT(version_id, 60000000);

  EXPECT_EQ(5, version_major);

  EXPECT_GT(version_minor, 0);
  EXPECT_LT(version_minor, 99);
  EXPECT_GT(version_archive, 0);
  EXPECT_LT(version_archive, 99);
}

TEST(MbDefaultDefaults, Basic) {
  const int verbose = 0;
  int format;
  int pings;
  int lonflip;
  double bounds[4];
  int btime_i[7];
  int etime_i[7];
  double speedmin;
  double timegap;
  EXPECT_EQ(MB_SUCCESS,
            mb_default_defaults(verbose, &format, &pings, &lonflip, bounds,
                                btime_i, etime_i, &speedmin, &timegap));
  EXPECT_EQ(0, format);
  EXPECT_EQ(1, pings);
  EXPECT_EQ(0, lonflip);  // int boolean
  EXPECT_THAT(bounds, testing::ElementsAre(-360.0, 360.0, -90.0, 90.0));
  EXPECT_THAT(btime_i, testing::ElementsAre(1962, 2, 21, 10, 30, 0, 0));
  EXPECT_THAT(etime_i, testing::ElementsAre(2062, 2, 21, 10, 30, 0, 0));
  EXPECT_DOUBLE_EQ(0.0, speedmin);
  EXPECT_DOUBLE_EQ(1.0, timegap);
}

// TODO(schwehr): Test mb_defaults by changing HOME to be a tmpdir.
// TODO(schwehr): Test mb_env by changing HOME to be a tmpdir.
// TODO(schwehr): Test mb_longflip by changing HOME to be a tmpdir.
// TODO(schwehr): Test mb_mbview_defaults by changing HOME to be a tmpdir.
// TODO(schwehr): Test mb_fbtversion by changing HOME to be a tmpdir.
// TODO(schwehr): Test mb_uselockfiles by changing HOME to be a tmpdir.
// TODO(schwehr): Test mb_fileiobuffer by changing HOME to be a tmpdir.

}  // namespace

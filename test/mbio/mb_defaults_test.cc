// Copyright 2020 Google Inc. All Rights Reserved.
//
// See README file for copying and redistribution conditions.

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

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
  EXPECT_GE(version_id, 50700009);
  EXPECT_LT(version_id, 70000000);
  EXPECT_GE(version_major, 4);
  EXPECT_LE(version_major, 7);
  EXPECT_GE(version_minor, 0);
  EXPECT_LE(version_minor, 99);
  EXPECT_GE(version_archive, 0);
  EXPECT_LE(version_archive, 99);
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

// Test the functions that read ~/.mbio_defaults, with HOME set to a
// temporary directory holding a file in the form written by mbdefaults.
class MbioDefaultsFileTest : public testing::Test {
 protected:
  void SetUp() override {
    const char *home = getenv("HOME");
    had_home_ = home != nullptr;
    if (had_home_)
      old_home_ = home;
    dir_ = std::filesystem::temp_directory_path() / "mb_defaults_test_home";
    std::filesystem::remove_all(dir_);
    std::filesystem::create_directories(dir_);
    ASSERT_EQ(0, setenv("HOME", dir_.string().c_str(), 1));
    std::ofstream out(dir_ / ".mbio_defaults");
    out << "MBIO Default Control Parameters\n"
        << "lonflip:    1\n"
        << "timegap:    2.500000\n"
        << "ps viewer:  my_ps_viewer\n"
        << "img viewer: my_img_viewer\n"
        << "project:    my_project\n"
        << "fbtversion: 2\n"
        << "uselockfiles:0\n"
        << "fileiobuffer:64\n"
        << "mbview_primary_colortable:        3\n"
        << "mbview_primary_colortable_mode:   1\n"
        << "mbview_primary_shade_mode:        2\n"
        << "mbview_slope_colortable:          4\n"
        << "mbview_slope_colortable_mode:     1\n"
        << "mbview_secondary_colortable:      5\n"
        << "mbview_secondary_colortable_mode: 1\n"
        << "mbview_illuminate_magnitude:      2.500000\n"
        << "mbview_illuminate_elevation:      30.000000\n"
        << "mbview_illuminate_azimuth:        45.000000\n"
        << "mbview_slope_magnitude:           3.500000\n";
  }
  void TearDown() override {
    if (had_home_)
      setenv("HOME", old_home_.c_str(), 1);
    else
      unsetenv("HOME");
    std::filesystem::remove_all(dir_);
  }
  std::filesystem::path dir_;
  std::string old_home_;
  bool had_home_ = false;
};

TEST_F(MbioDefaultsFileTest, Defaults) {
  int format;
  int pings;
  int lonflip;
  double bounds[4];
  int btime_i[7];
  int etime_i[7];
  double speedmin;
  double timegap;
  EXPECT_EQ(MB_SUCCESS, mb_defaults(0, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap));
  EXPECT_EQ(1, lonflip);
  EXPECT_DOUBLE_EQ(2.5, timegap);
  // values not stored in the file keep the built-in defaults
  EXPECT_EQ(0, format);
  EXPECT_EQ(1, pings);
  EXPECT_DOUBLE_EQ(0.0, speedmin);
}

TEST_F(MbioDefaultsFileTest, Env) {
  mb_path psdisplay;
  mb_path imgdisplay;
  mb_path mbproject;
  EXPECT_EQ(MB_SUCCESS, mb_env(0, psdisplay, imgdisplay, mbproject));
  EXPECT_STREQ("my_ps_viewer", psdisplay);
  EXPECT_STREQ("my_img_viewer", imgdisplay);
  EXPECT_STREQ("my_project", mbproject);
}

TEST_F(MbioDefaultsFileTest, Lonflip) {
  int lonflip = 0;
  EXPECT_EQ(MB_SUCCESS, mb_lonflip(0, &lonflip));
  EXPECT_EQ(1, lonflip);
}

TEST_F(MbioDefaultsFileTest, MbviewDefaults) {
  int primary_colortable;
  int primary_colortable_mode;
  int primary_shade_mode;
  int slope_colortable;
  int slope_colortable_mode;
  int secondary_colortable;
  int secondary_colortable_mode;
  double illuminate_magnitude;
  double illuminate_elevation;
  double illuminate_azimuth;
  double slope_magnitude;
  EXPECT_EQ(MB_SUCCESS,
            mb_mbview_defaults(0, &primary_colortable, &primary_colortable_mode, &primary_shade_mode, &slope_colortable,
                               &slope_colortable_mode, &secondary_colortable, &secondary_colortable_mode,
                               &illuminate_magnitude, &illuminate_elevation, &illuminate_azimuth, &slope_magnitude));
  EXPECT_EQ(3, primary_colortable);
  EXPECT_EQ(1, primary_colortable_mode);
  EXPECT_EQ(2, primary_shade_mode);
  EXPECT_EQ(4, slope_colortable);
  EXPECT_EQ(1, slope_colortable_mode);
  EXPECT_EQ(5, secondary_colortable);
  EXPECT_EQ(1, secondary_colortable_mode);
  EXPECT_DOUBLE_EQ(2.5, illuminate_magnitude);
  EXPECT_DOUBLE_EQ(30.0, illuminate_elevation);
  EXPECT_DOUBLE_EQ(45.0, illuminate_azimuth);
  EXPECT_DOUBLE_EQ(3.5, slope_magnitude);
}

TEST_F(MbioDefaultsFileTest, FbtversionUselockfilesFileiobuffer) {
  int fbtversion = 0;
  EXPECT_EQ(MB_SUCCESS, mb_fbtversion(0, &fbtversion));
  EXPECT_EQ(2, fbtversion);
  bool uselockfiles = true;
  EXPECT_EQ(MB_SUCCESS, mb_uselockfiles(0, &uselockfiles));
  EXPECT_FALSE(uselockfiles);
  int fileiobuffer = 0;
  EXPECT_EQ(MB_SUCCESS, mb_fileiobuffer(0, &fileiobuffer));
  EXPECT_EQ(64, fileiobuffer);
}

}  // namespace

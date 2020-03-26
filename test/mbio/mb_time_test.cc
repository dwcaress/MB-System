// Copyright 2018 Google Inc. All Rights Reserved.
//
// See README file for copying and redistribution conditions.

#include "mbio/mb_define.h"

#include "mbio/mb_status.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {

TEST(MbGetTime, Basic) {
  double t = -1.0;
  int time[7] = {1970, 1, 1, 0, 0, 0, 0};
  EXPECT_EQ(MB_SUCCESS, mb_get_time(2, time, &t));
  EXPECT_DOUBLE_EQ(0.0, t);
}

TEST(MbGetTime, MicroSec) {
  double t = -1.0;
  int time[7] = {1970, 1, 1, 0, 0, 0, 1};
  EXPECT_EQ(MB_SUCCESS, mb_get_time(0, time, &t));
  EXPECT_DOUBLE_EQ(1.0e-6, t);
}

TEST(MbGetTime, Sec) {
  double t = -1.0;
  int time[7] = {1970, 1, 1, 0, 0, 1, 0};
  EXPECT_EQ(MB_SUCCESS, mb_get_time(0, time, &t));
  EXPECT_DOUBLE_EQ(1.0, t);
}

TEST(MbGetTime, Min) {
  double t = -1.0;
  int time[7] = {1970, 1, 1, 0, 1, 0, 0};
  EXPECT_EQ(MB_SUCCESS, mb_get_time(0, time, &t));
  EXPECT_DOUBLE_EQ(60.0, t);
}

TEST(MbGetTime, Hour) {
  double t = -1.0;
  int time[7] = {1970, 1, 1, 1, 0, 0, 0};
  EXPECT_EQ(MB_SUCCESS, mb_get_time(0, time, &t));
  EXPECT_DOUBLE_EQ(3600.0, t);
}

TEST(MbGetTime, Day) {
  double t = -1.0;
  int time[7] = {1970, 1, 2, 0, 0, 0, 0};
  EXPECT_EQ(MB_SUCCESS, mb_get_time(0, time, &t));
  EXPECT_DOUBLE_EQ(86400.0, t);
}

TEST(MbGetTime, Month) {
  double t = -1.0;
  int time[7] = {1970, 2, 1, 0, 0, 0, 0};
  EXPECT_EQ(MB_SUCCESS, mb_get_time(0, time, &t));
  EXPECT_DOUBLE_EQ(2678400.0, t);
}

TEST(MbGetTime, Year) {
  double t = -1.0;
  int time[7] = {1971, 1, 1, 0, 0, 0, 0};
  EXPECT_EQ(MB_SUCCESS, mb_get_time(0, time, &t));
  EXPECT_DOUBLE_EQ(31536000.0, t);
}

TEST(MbGetTime, Negative) {
  double t = -1.0;
  int time[7] = {1969, 12, 31, 23, 59, 59, 999999};
  EXPECT_EQ(MB_SUCCESS, mb_get_time(0, time, &t));
  EXPECT_NEAR(-1.0e-6, t, 1.0e-7);
}

TEST(MbGetTime, Positive) {
  double t = -1.0;
  int time[7] = {2018, 1, 9, 13, 57, 16, 137096};
  EXPECT_EQ(MB_SUCCESS, mb_get_time(0, time, &t));
  EXPECT_DOUBLE_EQ(1515506236.1370959, t);
}

TEST(MbGetTime, Invalid) {
  const double expected_t = 0.0;
  double t = -1.0;
  // Year
  {
    int time[7] = {1929, 1, 9, 13, 57, 16, 137096};
    EXPECT_EQ(MB_FAILURE, mb_get_time(0, time, &t));
  }
  {
    int time[7] = {3001, 1, 9, 13, 57, 16, 137096};
    EXPECT_EQ(MB_FAILURE, mb_get_time(0, time, &t));
  }
  // Month
  {
    int time[7] = {2018, 0, 9, 13, 57, 16, 137096};
    EXPECT_EQ(MB_FAILURE, mb_get_time(0, time, &t));
  }
  {
    int time[7] = {2018, 13, 9, 13, 57, 16, 137096};
    EXPECT_EQ(MB_FAILURE, mb_get_time(0, time, &t));
  }
  // Day
  {
    int time[7] = {2018, 1, 0, 13, 57, 16, 137096};
    EXPECT_EQ(MB_FAILURE, mb_get_time(0, time, &t));
  }
  {
    int time[7] = {2018, 1, 32, 13, 57, 16, 137096};
    EXPECT_EQ(MB_FAILURE, mb_get_time(0, time, &t));
  }
  // Hour
  {
    int time[7] = {2018, 1, 9, -1, 57, 16, 137096};
    EXPECT_EQ(MB_FAILURE, mb_get_time(0, time, &t));
  }
  {
    int time[7] = {2018, 1, 9, 24, 57, 16, 137096};
    EXPECT_EQ(MB_FAILURE, mb_get_time(0, time, &t));
  }
  // Minute
  {
    int time[7] = {2018, 1, 9, 13, -1, 16, 137096};
    EXPECT_EQ(MB_FAILURE, mb_get_time(0, time, &t));
  }
  {
    int time[7] = {2018, 1, 9, 13, 60, 16, 137096};
    EXPECT_EQ(MB_FAILURE, mb_get_time(0, time, &t));
  }
  // Second
  {
    int time[7] = {2018, 1, 9, 13, 57, -1, 137096};
    EXPECT_EQ(MB_FAILURE, mb_get_time(0, time, &t));
  }
  {
    int time[7] = {2018, 1, 9, 13, 57, 60, 137096};
    EXPECT_EQ(MB_FAILURE, mb_get_time(0, time, &t));
  }
  // Microsec
  {
    int time[7] = {2018, 1, 9, 13, 57, 16, -1};
    EXPECT_EQ(MB_FAILURE, mb_get_time(0, time, &t));
  }
  {
    int time[7] = {2018, 1, 9, 13, 57, 16, 1000000};
    EXPECT_EQ(MB_FAILURE, mb_get_time(0, time, &t));
  }
  // t should be set to zero
  EXPECT_EQ(expected_t, t);
}

TEST(MbGetDate, Basic) {
  double t = 0.0;
  int time[7] = {-1, -1, -1, -1, -1, -1, -1};
  EXPECT_EQ(MB_SUCCESS, mb_get_date(0, t, time));
  EXPECT_THAT(time, testing::ElementsAre(1970, 1, 1, 0, 0, 0, 0));
}

TEST(MbGetDate, Negative) {
  double t = -1.0e-6;
  int time[7] = {-1, -1, -1, -1, -1, -1, -1};
  EXPECT_EQ(MB_SUCCESS, mb_get_date(0, t, time));
  // TODO(schwehr): Bogus result.
  EXPECT_THAT(time, testing::ElementsAre(1970, 1, 1, 0, 0, 0, -1));
}

TEST(MbGetDate, PositiveBadMicrosec) {
  double t = 915506236.000001;
  int time[7] = {-1, -1, -1, -1, -1, -1, -1};
  EXPECT_EQ(MB_SUCCESS, mb_get_date(0, t, time));
  // TODO(schwehr): Bad microsec
  EXPECT_THAT(time, testing::ElementsAre(1999, 1, 5, 3, 17, 16, 0));
}

TEST(MbGetDate, Positive) {
  double t = 915506236.00001;
  int time[7] = {-1, -1, -1, -1, -1, -1, -1};
  EXPECT_EQ(MB_SUCCESS, mb_get_date(0, t, time));
  // Bad microsec
  EXPECT_THAT(time, testing::ElementsAre(1999, 1, 5, 3, 17, 16, 10));
}

}  // namespace

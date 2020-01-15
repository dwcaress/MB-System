// Copyright 2020 Google Inc. All Rights Reserved.
//
// See README file for copying and redistribution conditions.

#include <cstdlib>

#include "mb_define.h"
#include "mb_status.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {

TEST(MbError, Failure) {
  const int verbose = 0;
  char *message;
  EXPECT_EQ(MB_FAILURE, mb_error(verbose, MB_ERROR_MAX + 1, &message));
  EXPECT_THAT(message, testing::HasSubstr("Unknown error"));
  EXPECT_EQ(MB_FAILURE, mb_error(verbose, MB_ERROR_MIN - 1, &message));
  EXPECT_THAT(message, testing::HasSubstr("Unknown error"));
}

TEST(MbError, NonFatal) {
  const int verbose = 0;
  char *message;
  EXPECT_EQ(MB_SUCCESS, mb_error(verbose, MB_ERROR_NO_ERROR, &message));
  EXPECT_STREQ("No error", message);
  EXPECT_EQ(MB_SUCCESS, mb_error(verbose, MB_ERROR_SIDESCAN_IGNORED, &message));
  EXPECT_STREQ("Sidescan ignored", message);
  EXPECT_EQ(MB_SUCCESS, mb_error(verbose, MB_ERROR_MIN, &message));
}

TEST(MbError, Fatal) {
  const int verbose = 0;
  char *message;
  EXPECT_EQ(MB_SUCCESS, mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message));
  EXPECT_THAT(message, testing::HasSubstr("memory"));
  EXPECT_EQ(MB_SUCCESS, mb_error(verbose, MB_ERROR_BAD_TIME, &message));
  EXPECT_THAT(message, testing::HasSubstr("time"));
  EXPECT_EQ(MB_SUCCESS, mb_error(verbose, MB_ERROR_MAX, &message));
}

// TODO(schwehr): What is this notice stuff?
// TODO(schwehr): Test mb_notice_log_datatype
// TODO(schwehr): Test mb_notice_log_error
// TODO(schwehr): Test mb_notice_log_problem
// TODO(schwehr): Test mb_notice_get_list
// TODO(schwehr): Test mb_notice_message

}  // namespace

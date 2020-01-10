// Copyright 2019 Google Inc. All Rights Reserved.
//
// See README file for copying and redistribution conditions.

#include "mbio/mb_define.h"  // TODO(schwehr): Move prototypes to mb_format.h.
#include "mbio/mb_format.h"

#include <cstdlib>
#include <cstring>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {

TEST(MbGetShortestPathTest, FullPath) {
  for (const auto initial :
       {"/a/c.d", "/a/../a/c.d", "/a/./c.d", "/a/./../a/c.d"}) {
    const int verbose = 0;
    int error = -999;  // Not one of MB_ERROR_*
    char *path = strdup(initial);
    EXPECT_EQ(MB_SUCCESS, mb_get_shortest_path(verbose, path, &error));
    EXPECT_EQ(MB_ERROR_NO_ERROR, error);
    EXPECT_STREQ("/a/c.d", path);
    free(path);
  }
}

TEST(MbGetShortestPathTest, RelativePath) {
  for (const auto initial : {"a/c.d", "a/../a/c.d", "a/./c.d",
       "a/./../a/c.d"}) {
    const int verbose = 0;
    int error = -999;  // Not one of MB_ERROR_*
    char *path = strdup(initial);
    EXPECT_EQ(MB_SUCCESS, mb_get_shortest_path(verbose, path, &error));
    EXPECT_EQ(MB_ERROR_NO_ERROR, error);
    EXPECT_STREQ("a/c.d", path);
    free(path);
  }
}

TEST(MbGetShortestPathTest, Empty) {
  for (const auto initial : {"", ".", "./"}) {
    const int verbose = 0;
    int error = -999;  // Not one of MB_ERROR_*
    char *path = strdup(initial);
    EXPECT_EQ(MB_SUCCESS, mb_get_shortest_path(verbose, path, &error));
    EXPECT_EQ(MB_ERROR_NO_ERROR, error);
    EXPECT_STREQ("", path);
    free(path);
  }
}

// TODO(schwehr): MbGetShortestPathTest crashed with ".." or "../"

TEST(MbGetBasenameTest, Basic) {
  for (const auto initial : {"b.a", "/b.a", "/c/b.a", "/d/c/b.a"}) {
    const int verbose = 0;
    int error = -999;  // Not one of MB_ERROR_*
    char *path = strdup(initial);
    EXPECT_EQ(MB_SUCCESS, mb_get_basename(verbose, path, &error));
    EXPECT_EQ(MB_ERROR_NO_ERROR, error);
    EXPECT_STREQ("b.a", path);
    free(path);
  }
}

TEST(MbGetBasenameTest, Empty) {
  const int verbose = 0;
  int error = -999;  // Not one of MB_ERROR_*
  char *path = strdup("");
  EXPECT_EQ(MB_SUCCESS, mb_get_basename(verbose, path, &error));
  EXPECT_EQ(MB_ERROR_NO_ERROR, error);
  EXPECT_STREQ("", path);
  free(path);
}

TEST(MbGetBasenameTest, Slash) {
  const int verbose = 0;
  int error = -999;  // Not one of MB_ERROR_*
  char *path = strdup("/");
  EXPECT_EQ(MB_SUCCESS, mb_get_basename(verbose, path, &error));
  EXPECT_EQ(MB_ERROR_NO_ERROR, error);
  EXPECT_STREQ("/", path);
  free(path);
}

TEST(MbGetBasenameTest, Slashes) {
  const int verbose = 0;
  int error = -999;  // Not one of MB_ERROR_*
  char *path = strdup("///a.b.c");
  EXPECT_EQ(MB_SUCCESS, mb_get_basename(verbose, path, &error));
  EXPECT_EQ(MB_ERROR_NO_ERROR, error);
  EXPECT_STREQ("a.b.c", path);
  free(path);
}

TEST(MbGetBasenameTest, SpecialExtension) {
  for (const auto initial : {"/a.fbt", "a.fbt", "a.fnv", "a.inf", "a.esf"}) {
    const int verbose = 0;
    int error = -999;  // Not one of MB_ERROR_*
    char *path = strdup(initial);
    EXPECT_EQ(MB_SUCCESS, mb_get_basename(verbose, path, &error));
    EXPECT_EQ(MB_ERROR_NO_ERROR, error);
    EXPECT_STREQ("a", path);
    free(path);
  }
}

TEST(MbGetBasenameTest, FbtExtra) {
  const int verbose = 0;
  int error = -999;  // Not one of MB_ERROR_*
  char *path = strdup("a.fbtyada");
  EXPECT_EQ(MB_SUCCESS, mb_get_basename(verbose, path, &error));
  EXPECT_EQ(MB_ERROR_NO_ERROR, error);
  EXPECT_STREQ("a.fbtyada", path);
  free(path);
}

}  // namespace

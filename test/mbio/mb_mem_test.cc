// Copyright 2020 Google Inc. All Rights Reserved.
//
// See README file for copying and redistribution conditions.

#include <cstdio>
#include <cstdlib>

#include "mb_define.h"
#include "mb_status.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {

TEST(MbDebug, Basic) {
  int error = MB_ERROR_NO_ERROR;
  int verbose = 0;
  EXPECT_EQ(MB_SUCCESS, mb_mem_debug_on(verbose, &error));
  EXPECT_EQ(MB_SUCCESS, mb_mem_debug_off(verbose, &error));
  verbose = 6;
  EXPECT_EQ(MB_SUCCESS, mb_mem_debug_on(verbose, &error));
  EXPECT_EQ(MB_SUCCESS, mb_mem_debug_off(verbose, &error));
  EXPECT_EQ(MB_ERROR_NO_ERROR, error);
}

TEST(MbDebug, Malloc0) {
  int error = MB_ERROR_NO_ERROR;
  int verbose = 0;

  void *ptr = nullptr;
  const size_t size = 0;
  EXPECT_EQ(MB_SUCCESS, mb_malloc(verbose, size, &ptr, &error));
  EXPECT_EQ(MB_ERROR_NO_ERROR, error);
  EXPECT_EQ(MB_SUCCESS, mb_free(verbose, &ptr, &error));
  EXPECT_EQ(MB_ERROR_NO_ERROR, error);
}

TEST(MbDebug, Malloc1) {
  int error = MB_ERROR_NO_ERROR;
  int verbose = 0;

  void *ptr = nullptr;
  const size_t size = 1;
  EXPECT_EQ(MB_SUCCESS, mb_malloc(verbose, size, &ptr, &error));
  EXPECT_EQ(MB_ERROR_NO_ERROR, error);
  EXPECT_EQ(MB_SUCCESS, mb_free(verbose, &ptr, &error));
  EXPECT_EQ(MB_ERROR_NO_ERROR, error);
}

TEST(MbDebug, MallocLarge) {
  int error = MB_ERROR_NO_ERROR;
  int verbose = 0;

  void *ptr = nullptr;
  const size_t size = 1000000;
  EXPECT_EQ(MB_SUCCESS, mb_malloc(verbose, size, &ptr, &error));
  EXPECT_EQ(MB_ERROR_NO_ERROR, error);
  EXPECT_EQ(MB_SUCCESS, mb_free(verbose, &ptr, &error));
  EXPECT_EQ(MB_ERROR_NO_ERROR, error);
}

TEST(MbDebug, MallocThenClear) {
  int error = MB_ERROR_NO_ERROR;
  int verbose = 0;

  void *ptr = nullptr;
  const size_t size = 1;
  EXPECT_EQ(MB_SUCCESS, mb_malloc(verbose, size, &ptr, &error));
  EXPECT_EQ(MB_ERROR_NO_ERROR, error);
  EXPECT_EQ(MB_SUCCESS, mb_memory_clear(verbose, &error));
  EXPECT_EQ(MB_ERROR_NO_ERROR, error);
}

TEST(MbDebug, FreeBadPtr) {
  int error = MB_ERROR_NO_ERROR;
  int verbose = 0;
  void *ptr = reinterpret_cast<void *>(0xDEADBEEF);

  EXPECT_EQ(MB_SUCCESS, mb_free(verbose, &ptr, &error));
  EXPECT_EQ(MB_ERROR_NO_ERROR, error);
}

TEST(MbDebug, FreeNullptr) {
  int error = MB_ERROR_NO_ERROR;
  int verbose = 0;
  void *ptr = nullptr;

  EXPECT_EQ(MB_SUCCESS, mb_free(verbose, &ptr, &error));
  EXPECT_EQ(MB_ERROR_NO_ERROR, error);
}

// TODO(schwehr): Test mb_mallocd
// TODO(schwehr): Test mb_realloc
// TODO(schwehr): Test mb_reallocd
// TODO(schwehr): Test mb_freed
// TODO(schwehr): Test mb_memory_clear
// TODO(schwehr): Test mb_memory_status
// TODO(schwehr): Test mb_memory_list
// TODO(schwehr): Test mb_register_array
// TODO(schwehr): Test mb_update_arrays
// TODO(schwehr): Test mb_update_arrayptr
// TODO(schwehr): Test mb_list_arrays
// TODO(schwehr): Test mb_deall_ioarrays

}  // namespace

// Copyright 2020 Google Inc. All Rights Reserved.
//
// See README file for copying and redistribution conditions.

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "mb_define.h"
#include "mb_io.h"
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
// TODO(schwehr): Test mb_list_arrays

void InitMinimalMbio(struct mb_io_struct *mbio, int beams, int pixels) {
  memset(mbio, 0, sizeof(*mbio));
  mbio->beams_bath_max = beams;
  mbio->beams_amp_max = beams;
  mbio->pixels_ss_max = pixels;
  mbio->beams_bath_alloc = beams;
  mbio->beams_amp_alloc = beams;
  mbio->pixels_ss_alloc = pixels;
}

// Simulate mb_read() after bathymetry and sidescan arrays grow in one cycle;
// each local pointer copy must be rebound to its registered allocation.
TEST(MbMem, UpdateArrayptrLocalCopyAfterBathAndSidescanGrow) {
  int error = MB_ERROR_NO_ERROR;
  const int verbose = 0;
  struct mb_io_struct mbio;
  InitMinimalMbio(&mbio, 254, 8192);

  double *bath = nullptr;
  double *sslon = nullptr;
  ASSERT_EQ(MB_SUCCESS, mb_register_array(verbose, &mbio, MB_MEM_TYPE_BATHYMETRY,
                                          sizeof(double), (void **)&bath, &error));
  ASSERT_EQ(MB_SUCCESS, mb_register_array(verbose, &mbio, MB_MEM_TYPE_SIDESCAN,
                                          sizeof(double), (void **)&sslon, &error));
  ASSERT_NE(bath, nullptr);
  ASSERT_NE(sslon, nullptr);
  ASSERT_NE(bath, sslon);

  double *bath_local = bath;
  double *sslon_local = sslon;

  ASSERT_EQ(MB_SUCCESS, mb_update_arrays(verbose, &mbio, 800, 800, 9664, &error));
  ASSERT_EQ(MB_SUCCESS, mb_update_arrayptr(verbose, &mbio, (void **)&bath_local, &error));
  ASSERT_EQ(MB_SUCCESS, mb_update_arrayptr(verbose, &mbio, (void **)&sslon_local, &error));

  EXPECT_EQ(bath_local, bath);
  EXPECT_EQ(sslon_local, sslon);
  EXPECT_NE(bath_local, sslon_local);

  sslon_local[mbio.pixels_ss_max - 1] = 42.0;
  EXPECT_EQ(sslon_local[mbio.pixels_ss_max - 1], 42.0);

  EXPECT_EQ(MB_SUCCESS, mb_deall_ioarrays(verbose, &mbio, &error));
}

TEST(MbMem, UpdateArrayptrMatchesHandleNotStaleOldptr) {
  int error = MB_ERROR_NO_ERROR;
  const int verbose = 0;
  struct mb_io_struct mbio;
  InitMinimalMbio(&mbio, 254, 8192);

  double *bath = nullptr;
  double *sslon = nullptr;
  ASSERT_EQ(MB_SUCCESS, mb_register_array(verbose, &mbio, MB_MEM_TYPE_BATHYMETRY,
                                          sizeof(double), (void **)&bath, &error));
  ASSERT_EQ(MB_SUCCESS, mb_register_array(verbose, &mbio, MB_MEM_TYPE_SIDESCAN,
                                          sizeof(double), (void **)&sslon, &error));

  double *const sslon_allocation = sslon;
  double *const bath_allocation = bath;
  mbio.regarray_oldptr[0] = sslon;

  ASSERT_EQ(MB_SUCCESS, mb_update_arrayptr(verbose, &mbio, (void **)&sslon, &error));
  EXPECT_EQ(sslon, sslon_allocation);
  EXPECT_NE(sslon, bath_allocation);
  EXPECT_EQ(sslon, mbio.regarray_ptr[1]);
  EXPECT_EQ(mbio.regarray_oldptr[0], sslon_allocation);

  EXPECT_EQ(MB_SUCCESS, mb_deall_ioarrays(verbose, &mbio, &error));
}

TEST(MbMem, UpdateArrayptrLocalCopyAfterStagedBathThenSidescanGrow) {
  int error = MB_ERROR_NO_ERROR;
  const int verbose = 0;
  struct mb_io_struct mbio;
  InitMinimalMbio(&mbio, 254, 8192);

  double *bath = nullptr;
  double *sslon = nullptr;
  ASSERT_EQ(MB_SUCCESS, mb_register_array(verbose, &mbio, MB_MEM_TYPE_BATHYMETRY,
                                          sizeof(double), (void **)&bath, &error));
  ASSERT_EQ(MB_SUCCESS, mb_register_array(verbose, &mbio, MB_MEM_TYPE_SIDESCAN,
                                          sizeof(double), (void **)&sslon, &error));

  double *bath_local = bath;
  ASSERT_EQ(MB_SUCCESS, mb_update_arrays(verbose, &mbio, 800, 254, 8192, &error));
  ASSERT_EQ(MB_SUCCESS, mb_update_arrayptr(verbose, &mbio, (void **)&bath_local, &error));
  EXPECT_EQ(bath_local, bath);

  // Simulate allocator reuse: the stale bathymetry address now identifies the
  // current sidescan allocation.
  mbio.regarray_oldptr[0] = sslon;
  double *sslon_local = sslon;
  ASSERT_EQ(MB_SUCCESS, mb_update_arrays(verbose, &mbio, 800, 800, 9664, &error));
  EXPECT_EQ(mbio.regarray_oldptr[0], nullptr);
  ASSERT_EQ(MB_SUCCESS, mb_update_arrayptr(verbose, &mbio, (void **)&sslon_local, &error));
  EXPECT_EQ(sslon_local, sslon);
  EXPECT_NE(sslon_local, bath_local);

  EXPECT_EQ(MB_SUCCESS, mb_deall_ioarrays(verbose, &mbio, &error));
}

}  // namespace

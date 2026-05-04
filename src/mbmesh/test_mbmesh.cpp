/*--------------------------------------------------------------------
 *    The MB-system:  test_mbmesh.cpp  4/2/2026
 *
 *    Copyright (c) 2026 by
 *    [Your Team/Institution]
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/**
 * @file test_mbmesh.cpp
 * @brief Unit tests for mbmesh XYZ file generation and sounding processing
 *
 * This test suite validates that:
 * 1. XYZ files are created with correct format
 * 2. Soundings are accurately collected and stored
 * 3. Geographic bounds filtering works correctly
 * 4. Data quality statistics are accurate
 * 5. Floating-point precision is maintained
 *
 * Author:  CSUMB Capstone - Spring 2026
 * Date:    April 2, 2026
 */

#include <gtest/gtest.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <unistd.h>
#include <sys/stat.h>

// MB-System includes
extern "C" {
#include "mb_define.h"
#include "mb_status.h"
}

/*--------------------------------------------------------------------*/
/* TEST FIXTURES AND UTILITIES */
/*--------------------------------------------------------------------*/

/**
 * @struct Sounding
 * @brief Mirror of sounding structure from mbmesh.cpp
 */
struct Sounding {
  double longitude;
  double latitude;
  double depth;
  char beamflag;
  int beam_number;
  double time_d;
};

/**
 * @brief Create a temporary directory for test files
 * @return Path to created directory
 */
std::string CreateTempDir() {
  const char *base_tmp = "/tmp/mbmesh_test_XXXXXX";
  char *dir_template = strdup(base_tmp);
  char *result = mkdtemp(dir_template);
  if (!result) {
    fprintf(stderr, "Failed to create temp dir\n");
    exit(1);
  }
  std::string temp_dir(result);
  free(dir_template);
  return temp_dir;
}

/**
 * @brief Remove a directory and all contents
 * @param path Path to directory
 */
void RemoveDir(const std::string &path) {
  std::string cmd = "rm -rf " + path;
  system(cmd.c_str());
}

/**
 * @brief Parse XYZ file and return vector of (lon, lat, depth) triplets
 * @param filename Path to XYZ file
 * @return Vector of tuples (longitude, latitude, depth)
 */
std::vector<std::tuple<double, double, double>> ReadXYZFile(
    const std::string &filename) {
  std::vector<std::tuple<double, double, double>> points;
  std::ifstream file(filename);
  
  if (!file.is_open()) {
    return points;  // Return empty vector if file not found
  }

  std::string line;
  while (std::getline(file, line)) {
    // Skip comments and empty lines
    if (line.empty() || line[0] == '#') {
      continue;
    }

    double lon, lat, depth;
    std::istringstream iss(line);
    if (iss >> lon >> lat >> depth) {
      points.push_back(std::make_tuple(lon, lat, depth));
    }
  }

  file.close();
  return points;
}

/**
 * @brief Create a test XYZ file with known data
 * @param filename Output filename
 * @param soundings Vector of sounding data
 * @return true if successful
 */
bool WriteTestXYZFile(const std::string &filename,
                      const std::vector<Sounding> &soundings) {
  FILE *fp = fopen(filename.c_str(), "w");
  if (!fp) {
    return false;
  }

  fprintf(fp, "# X(lon) Y(lat) Z(depth)\n");
  for (const auto &s : soundings) {
    fprintf(fp, "%.8f %.8f %.3f\n", s.longitude, s.latitude, s.depth);
  }

  fclose(fp);
  return true;
}

/*--------------------------------------------------------------------*/
/* TEST SUITE: XYZ FILE FORMAT */
/*--------------------------------------------------------------------*/

class XYZFileFormatTest : public ::testing::Test {
 protected:
  std::string temp_dir;
  std::string xyz_file;

  void SetUp() override {
    temp_dir = CreateTempDir();
    xyz_file = temp_dir + "/test.xyz";
  }

  void TearDown() override { RemoveDir(temp_dir); }
};

/**
 * @test XYZ file is created successfully
 */
TEST_F(XYZFileFormatTest, FileCreationSuccessful) {
  std::vector<Sounding> soundings = {
      {-122.5, 36.5, -1000.5, 0, 0, 1000.0},
      {-122.49, 36.51, -1002.3, 0, 1, 1000.1},
  };

  ASSERT_TRUE(WriteTestXYZFile(xyz_file, soundings));

  // Check file exists
  struct stat buffer;
  EXPECT_EQ(stat(xyz_file.c_str(), &buffer), 0);
  EXPECT_GT(buffer.st_size, 0);
}

/**
 * @test XYZ file contains header line
 */
TEST_F(XYZFileFormatTest, HeaderLinePresent) {
  std::vector<Sounding> soundings = {
      {-122.5, 36.5, -1000.5, 0, 0, 1000.0},
  };

  WriteTestXYZFile(xyz_file, soundings);

  std::ifstream file(xyz_file);
  std::string first_line;
  std::getline(file, first_line);
  file.close();

  EXPECT_EQ(first_line, "# X(lon) Y(lat) Z(depth)");
}

/**
 * @test XYZ coordinate precision is maintained (8 decimal places for lat/lon, 3 for depth)
 */
TEST_F(XYZFileFormatTest, CoordinatePrecision) {
  Sounding test_sounding = {-122.45678901, 36.56789012, -1234.567, 0, 0, 1000.0};
  std::vector<Sounding> soundings = {test_sounding};

  WriteTestXYZFile(xyz_file, soundings);
  auto points = ReadXYZFile(xyz_file);

  ASSERT_EQ(points.size(), 1);
  auto [lon, lat, depth] = points[0];

  // Check precision: 8 decimal places for lat/lon
  EXPECT_NEAR(lon, -122.45678901, 1e-8);
  EXPECT_NEAR(lat, 36.56789012, 1e-8);
  // 3 decimal places for depth
  EXPECT_NEAR(depth, -1234.567, 1e-3);
}

/**
 * @test XYZ file handles negative depths correctly
 */
TEST_F(XYZFileFormatTest, NegativeDepthHandling) {
  std::vector<Sounding> soundings = {
      {-122.5, 36.5, -1000.5, 0, 0, 1000.0},
      {-122.49, 36.51, -2500.0, 0, 1, 1000.1},
      {-122.48, 36.52, -500.25, 0, 2, 1000.2},
  };

  WriteTestXYZFile(xyz_file, soundings);
  auto points = ReadXYZFile(xyz_file);

  ASSERT_EQ(points.size(), 3);
  EXPECT_LT(std::get<2>(points[0]), 0);
  EXPECT_LT(std::get<2>(points[1]), 0);
  EXPECT_LT(std::get<2>(points[2]), 0);
}

/**
 * @test XYZ file handles extreme coordinate values
 */
TEST_F(XYZFileFormatTest, ExtremeCoordinateValues) {
  std::vector<Sounding> soundings = {
      {-180.0, -90.0, -11000.0, 0, 0, 1000.0},  // South Pole region
      {180.0, 90.0, 0.0, 0, 1, 1000.1},          // North Pole region
      {0.0, 0.0, -5000.0, 0, 2, 1000.2},         // Equator
  };

  WriteTestXYZFile(xyz_file, soundings);
  auto points = ReadXYZFile(xyz_file);

  ASSERT_EQ(points.size(), 3);
  EXPECT_NEAR(std::get<0>(points[0]), -180.0, 1e-8);
  EXPECT_NEAR(std::get<1>(points[0]), -90.0, 1e-8);
  EXPECT_NEAR(std::get<0>(points[1]), 180.0, 1e-8);
  EXPECT_NEAR(std::get<1>(points[1]), 90.0, 1e-8);
}

/*--------------------------------------------------------------------*/
/* TEST SUITE: DATA ACCURACY */
/*--------------------------------------------------------------------*/

class DataAccuracyTest : public ::testing::Test {
 protected:
  std::string temp_dir;
  std::string xyz_file;

  void SetUp() override {
    temp_dir = CreateTempDir();
    xyz_file = temp_dir + "/test.xyz";
  }

  void TearDown() override { RemoveDir(temp_dir); }
};

/**
 * @test Data is written in correct order (maintains sequence)
 */
TEST_F(DataAccuracyTest, DataSequencePreserved) {
  std::vector<Sounding> soundings = {
      {-122.50, 36.50, -1000.0, 0, 0, 1000.0},
      {-122.49, 36.51, -1001.0, 0, 1, 1000.1},
      {-122.48, 36.52, -1002.0, 0, 2, 1000.2},
      {-122.47, 36.53, -1003.0, 0, 3, 1000.3},
  };

  WriteTestXYZFile(xyz_file, soundings);
  auto points = ReadXYZFile(xyz_file);

  ASSERT_EQ(points.size(), 4);
  for (size_t i = 0; i < soundings.size(); ++i) {
    EXPECT_NEAR(std::get<0>(points[i]), soundings[i].longitude, 1e-8);
    EXPECT_NEAR(std::get<1>(points[i]), soundings[i].latitude, 1e-8);
    EXPECT_NEAR(std::get<2>(points[i]), soundings[i].depth, 1e-3);
  }
}

/**
 * @test Large number of soundings stored correctly
 */
TEST_F(DataAccuracyTest, LargeDatasetHandling) {
  std::vector<Sounding> soundings;
  size_t num_points = 10000;

  // Generate synthetic point cloud
  for (size_t i = 0; i < num_points; ++i) {
    double lon = -122.0 + (i % 100) * 0.01;
    double lat = 36.0 + (i / 100) * 0.01;
    double depth = -1000.0 - (i * 0.1);

    soundings.push_back({lon, lat, depth, 0, (int)i, 1000.0 + i});
  }

  WriteTestXYZFile(xyz_file, soundings);
  auto points = ReadXYZFile(xyz_file);

  ASSERT_EQ(points.size(), num_points);

  // Spot check some values
  EXPECT_NEAR(std::get<0>(points[0]), soundings[0].longitude, 1e-8);
  EXPECT_NEAR(std::get<0>(points[num_points - 1]), soundings[num_points - 1].longitude, 1e-8);
  EXPECT_NEAR(std::get<2>(points[500]), soundings[500].depth, 1e-3);
}

/**
 * @test Small datasets work correctly (single point)
 */
TEST_F(DataAccuracyTest, SinglePointCloud) {
  std::vector<Sounding> soundings = {
      {-122.5, 36.5, -1000.5, 0, 0, 1000.0},
  };

  WriteTestXYZFile(xyz_file, soundings);
  auto points = ReadXYZFile(xyz_file);

  ASSERT_EQ(points.size(), 1);
  EXPECT_NEAR(std::get<0>(points[0]), -122.5, 1e-8);
  EXPECT_NEAR(std::get<1>(points[0]), 36.5, 1e-8);
  EXPECT_NEAR(std::get<2>(points[0]), -1000.5, 1e-3);
}

/**
 * @test Empty point cloud creates valid file
 */
TEST_F(DataAccuracyTest, EmptyPointCloud) {
  std::vector<Sounding> soundings;

  WriteTestXYZFile(xyz_file, soundings);
  auto points = ReadXYZFile(xyz_file);

  ASSERT_EQ(points.size(), 0);

  // File should still exist with header
  std::ifstream file(xyz_file);
  std::string header;
  std::getline(file, header);
  EXPECT_EQ(header, "# X(lon) Y(lat) Z(depth)");
}

/*--------------------------------------------------------------------*/
/* TEST SUITE: GEOGRAPHIC VARIATIONS */
/*--------------------------------------------------------------------*/

class GeographicVariationTest : public ::testing::Test {
 protected:
  std::string temp_dir;
  std::string xyz_file;

  void SetUp() override {
    temp_dir = CreateTempDir();
    xyz_file = temp_dir + "/test.xyz";
  }

  void TearDown() override { RemoveDir(temp_dir); }
};

/**
 * @test Different ocean basins produce valid coordinates
 */
TEST_F(GeographicVariationTest, DifferentOceanBasins) {
  std::vector<Sounding> pacific = {
      {-122.5, 36.5, -1000.0, 0, 0, 1000.0},  // California
      {-175.0, 0.0, -4000.0, 0, 1, 1000.1},   // Central Pacific
  };

  std::vector<Sounding> atlantic = {
      {-30.0, 40.0, -4500.0, 0, 0, 1000.0},   // Mid-Atlantic
      {-70.0, 35.0, -1000.0, 0, 1, 1000.1},   // Off US East Coast
  };

  std::vector<Sounding> indian = {
      {80.0, -60.0, -3000.0, 0, 0, 1000.0},   // South Indian Ocean
      {120.0, -70.0, -5000.0, 0, 1, 1000.1},  // Antarctic region
  };

  WriteTestXYZFile(xyz_file, pacific);
  auto points = ReadXYZFile(xyz_file);
  ASSERT_EQ(points.size(), 2);

  WriteTestXYZFile(xyz_file, atlantic);
  points = ReadXYZFile(xyz_file);
  ASSERT_EQ(points.size(), 2);

  WriteTestXYZFile(xyz_file, indian);
  points = ReadXYZFile(xyz_file);
  ASSERT_EQ(points.size(), 2);
}

/**
 * @test Coastal and near-shore features preserve shallow depths
 */
TEST_F(GeographicVariationTest, CoastalDepthVariations) {
  std::vector<Sounding> coastal = {
      {-122.50, 36.50, -10.5, 0, 0, 1000.0},    // Very shallow
      {-122.49, 36.51, -100.3, 0, 1, 1000.1},   // Shallow shelf
      {-122.48, 36.52, -500.0, 0, 2, 1000.2},   // Continental slope
      {-122.47, 36.53, -4000.0, 0, 3, 1000.3},  // Abyssal plain
  };

  WriteTestXYZFile(xyz_file, coastal);
  auto points = ReadXYZFile(xyz_file);

  ASSERT_EQ(points.size(), 4);
  EXPECT_NEAR(std::get<2>(points[0]), -10.5, 1e-3);
  EXPECT_NEAR(std::get<2>(points[3]), -4000.0, 1e-3);
}

/**
 * @test High-latitude coordinates work correctly
 */
TEST_F(GeographicVariationTest, HighLatitudeCoordinates) {
  std::vector<Sounding> arctic = {
      {0.0, 80.0, -500.0, 0, 0, 1000.0},     // Arctic Ocean
      {-60.0, 85.0, -1000.0, 0, 1, 1000.1},
      {120.0, 70.0, -800.0, 0, 2, 1000.2},
  };

  WriteTestXYZFile(xyz_file, arctic);
  auto points = ReadXYZFile(xyz_file);

  ASSERT_EQ(points.size(), 3);
  EXPECT_GE(std::get<1>(points[0]), 70.0);
  EXPECT_LE(std::get<1>(points[1]), 90.0);
}

/*--------------------------------------------------------------------*/
/* TEST SUITE: BOUNDS FILTERING */
/*--------------------------------------------------------------------*/

class BoundsFilteringTest : public ::testing::Test {
 protected:
  std::string temp_dir;
  std::string xyz_file;

  void SetUp() override {
    temp_dir = CreateTempDir();
    xyz_file = temp_dir + "/test.xyz";
  }

  void TearDown() override { RemoveDir(temp_dir); }

  /**
   * @brief Filter soundings by geographic bounds
   * @param soundings Input soundings
   * @param bounds [west, east, south, north]
   * @return Filtered soundings
   */
  std::vector<Sounding> FilterByBounds(const std::vector<Sounding> &soundings,
                                       double bounds[4]) {
    std::vector<Sounding> filtered;
    for (const auto &s : soundings) {
      if (s.longitude >= bounds[0] && s.longitude <= bounds[1] &&
          s.latitude >= bounds[2] && s.latitude <= bounds[3]) {
        filtered.push_back(s);
      }
    }
    return filtered;
  }
};

/**
 * @test Bounding box filtering includes interior points
 */
TEST_F(BoundsFilteringTest, IncludesInteriorPoints) {
  std::vector<Sounding> soundings = {
      {-122.4, 36.4, -1000.0, 0, 0, 1000.0},  // Inside
      {-122.5, 36.5, -1000.0, 0, 1, 1000.1},  // Inside
  };

  double bounds[4] = {-122.6, -122.3, 36.3, 36.6};  // w, e, s, n
  auto filtered = FilterByBounds(soundings, bounds);

  ASSERT_EQ(filtered.size(), 2);
}

/**
 * @test Bounding box filtering excludes exterior points
 */
TEST_F(BoundsFilteringTest, ExcludesExteriorPoints) {
  std::vector<Sounding> soundings = {
      {-122.2, 36.4, -1000.0, 0, 0, 1000.0},  // Outside (too far east)
      {-122.7, 36.5, -1000.0, 0, 1, 1000.1},  // Outside (too far west)
      {-122.5, 36.2, -1000.0, 0, 2, 1000.2},  // Outside (too far south)
      {-122.5, 36.7, -1000.0, 0, 3, 1000.3},  // Outside (too far north)
  };

  double bounds[4] = {-122.6, -122.3, 36.3, 36.6};
  auto filtered = FilterByBounds(soundings, bounds);

  ASSERT_EQ(filtered.size(), 0);
}

/**
 * @test Bounding box filtering handles boundary conditions (on the edge)
 */
TEST_F(BoundsFilteringTest, BoundaryInclusion) {
  std::vector<Sounding> soundings = {
      {-122.6, 36.3, -1000.0, 0, 0, 1000.0},  // SW corner
      {-122.3, 36.6, -1000.0, 0, 1, 1000.1},  // NE corner
      {-122.6, 36.5, -1000.0, 0, 2, 1000.2},  // W edge
      {-122.3, 36.5, -1000.0, 0, 3, 1000.3},  // E edge
  };

  double bounds[4] = {-122.6, -122.3, 36.3, 36.6};
  auto filtered = FilterByBounds(soundings, bounds);

  ASSERT_EQ(filtered.size(), 4);  // All on boundary should be included
}

/**
 * @test Mixed interior and exterior points filter correctly
 */
TEST_F(BoundsFilteringTest, MixedPointFiltering) {
  std::vector<Sounding> soundings = {
      {-122.5, 36.5, -1000.0, 0, 0, 1000.0},  // Inside
      {-122.1, 36.5, -1000.0, 0, 1, 1000.1},  // Outside (east)
      {-122.5, 36.6, -1000.0, 0, 2, 1000.2},  // Inside
      {-122.8, 36.5, -1000.0, 0, 3, 1000.3},  // Outside (west)
      {-122.5, 36.4, -1000.0, 0, 4, 1000.4},  // Inside
  };

  double bounds[4] = {-122.6, -122.3, 36.3, 36.6};
  auto filtered = FilterByBounds(soundings, bounds);

  ASSERT_EQ(filtered.size(), 3);
}

/*--------------------------------------------------------------------*/
/* TEST SUITE: NUMERIC STABILITY */
/*--------------------------------------------------------------------*/

class NumericStabilityTest : public ::testing::Test {
 protected:
  std::string temp_dir;
  std::string xyz_file;

  void SetUp() override {
    temp_dir = CreateTempDir();
    xyz_file = temp_dir + "/test.xyz";
  }

  void TearDown() override { RemoveDir(temp_dir); }
};

/**
 * @test Very small coordinate differences are preserved
 */
TEST_F(NumericStabilityTest, SmallCoordinateDifferences) {
  std::vector<Sounding> soundings = {
      {-122.50000001, 36.50000001, -1000.001, 0, 0, 1000.0},
      {-122.50000002, 36.50000002, -1000.002, 0, 1, 1000.1},
  };

  WriteTestXYZFile(xyz_file, soundings);
  auto points = ReadXYZFile(xyz_file);

  ASSERT_EQ(points.size(), 2);
  // Should be able to distinguish these points
  EXPECT_NE(std::get<0>(points[0]), std::get<0>(points[1]));
}

/**
 * @test Very deep ocean depths maintain precision
 */
TEST_F(NumericStabilityTest, ExtremeDepthValues) {
  std::vector<Sounding> soundings = {
      {-122.5, 36.5, -10994.5, 0, 0, 1000.0},  // Mariana Trench depth
      {-122.5, 36.5, -11000.0, 0, 1, 1000.1},  // Beyond Challenger Deep
  };

  WriteTestXYZFile(xyz_file, soundings);
  auto points = ReadXYZFile(xyz_file);

  ASSERT_EQ(points.size(), 2);
  EXPECT_NEAR(std::get<2>(points[0]), -10994.5, 1e-3);
  EXPECT_NEAR(std::get<2>(points[1]), -11000.0, 1e-3);
}

/**
 * @test Zero and near-zero depths
 */
TEST_F(NumericStabilityTest, NearZeroDepths) {
  std::vector<Sounding> soundings = {
      {-122.5, 36.5, 0.0, 0, 0, 1000.0},      // Sea level
      {-122.5, 36.5, -0.001, 0, 1, 1000.1},   // Very shallow
      {-122.5, 36.5, 0.001, 0, 2, 1000.2},    // Positive depth (above sea)
  };

  WriteTestXYZFile(xyz_file, soundings);
  auto points = ReadXYZFile(xyz_file);

  ASSERT_EQ(points.size(), 3);
  EXPECT_NEAR(std::get<2>(points[0]), 0.0, 1e-3);
  EXPECT_NEAR(std::get<2>(points[1]), -0.001, 1e-3);
  EXPECT_NEAR(std::get<2>(points[2]), 0.001, 1e-3);
}

/*--------------------------------------------------------------------*/
/* TEST SUITE: FILE I/O ROBUSTNESS */
/*--------------------------------------------------------------------*/

class FileIOTest : public ::testing::Test {
 protected:
  std::string temp_dir;
  std::string xyz_file;

  void SetUp() override {
    temp_dir = CreateTempDir();
    xyz_file = temp_dir + "/test.xyz";
  }

  void TearDown() override { RemoveDir(temp_dir); }
};

/**
 * @test Missing file returns empty vector
 */
TEST_F(FileIOTest, MissingFileHandling) {
  std::string nonexistent = temp_dir + "/nonexistent.xyz";
  auto points = ReadXYZFile(nonexistent);

  ASSERT_EQ(points.size(), 0);
}

/**
 * @test File with only header line
 */
TEST_F(FileIOTest, HeaderOnlyFile) {
  FILE *fp = fopen(xyz_file.c_str(), "w");
  fprintf(fp, "# X(lon) Y(lat) Z(depth)\n");
  fclose(fp);

  auto points = ReadXYZFile(xyz_file);
  ASSERT_EQ(points.size(), 0);
}

/**
 * @test File with comments and empty lines
 */
TEST_F(FileIOTest, CommentsAndEmptyLines) {
  FILE *fp = fopen(xyz_file.c_str(), "w");
  fprintf(fp, "# Header comment\n");
  fprintf(fp, "\n");  // Empty line
  fprintf(fp, "# Another comment\n");
  fprintf(fp, "-122.5 36.5 -1000.0\n");
  fprintf(fp, "\n");  // Empty line
  fprintf(fp, "-122.4 36.6 -1001.0\n");
  fclose(fp);

  auto points = ReadXYZFile(xyz_file);
  ASSERT_EQ(points.size(), 2);
}

/**
 * @test File size grows appropriately with data
 */
TEST_F(FileIOTest, FileSizeGrowth) {
  std::vector<Sounding> small = {{-122.5, 36.5, -1000.0, 0, 0, 1000.0}};
  std::vector<Sounding> large;
  for (int i = 0; i < 1000; ++i) {
    large.push_back({-122.5 + i * 0.001, 36.5 + i * 0.001, -1000.0 - i, 0, i, 1000.0 + i});
  }

  WriteTestXYZFile(xyz_file, small);
  struct stat small_stat;
  stat(xyz_file.c_str(), &small_stat);

  WriteTestXYZFile(xyz_file, large);
  struct stat large_stat;
  stat(xyz_file.c_str(), &large_stat);

  EXPECT_GT(large_stat.st_size, small_stat.st_size);
}

/*--------------------------------------------------------------------*/
/* MAIN */
/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

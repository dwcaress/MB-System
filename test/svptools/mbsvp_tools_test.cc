/*--------------------------------------------------------------------
 *    The MB-system:  mbsvp_tools_test.cc  3 November 2026
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/* Tests of the shared code of the sound velocity profile tools. */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include <unistd.h>

#include <netcdf.h>

#include "mb_define.h"
#include "mb_status.h"
#include "mbsvp_tools.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {

const double NaN = std::numeric_limits<double>::quiet_NaN();
const double Inf = std::numeric_limits<double>::infinity();

class Profile {
 public:
  Profile() { mbsvp_profile_init(&p); }
  Profile(std::vector<double> depth, std::vector<double> speed) : z(std::move(depth)), c(std::move(speed)) {
    mbsvp_profile_init(&p);
    raw.n = static_cast<int>(z.size());
    raw.depth = z.data();
    raw.speed = c.data();
  }
  ~Profile() {
    int error = MB_ERROR_NO_ERROR;
    mbsvp_profile_free(0, &p, &error);
  }
  Profile(const Profile &) = delete;
  Profile &operator=(const Profile &) = delete;
  std::vector<double> depth() const { return std::vector<double>(p.depth, p.depth + p.n); }
  std::vector<double> speed() const { return std::vector<double>(p.speed, p.speed + p.n); }
  std::vector<double> z;
  std::vector<double> c;
  struct mbsvp_profile raw{};
  struct mbsvp_profile p;
};

/* Run the completion step, as the Python tests did. */
bool Complete(Profile *input) {
  char message[MBSVP_MESSAGE_SIZE] = "";
  int error = MB_ERROR_NO_ERROR;
  return mbsvp_profile_complete(0, &input->raw, MBSVP_TARGET_DEPTH, &input->p, message, &error) == MB_SUCCESS;
}

TEST(ProfileTest, NanRemovedBeforeJoin) {
  Profile primary({0, 1000, 2000, 3000}, {1500, 1510, NaN, NaN});
  Profile donor({0, 1000, 2000, 3000}, {1490, 1500, 1520, 1530});
  char message[MBSVP_MESSAGE_SIZE];
  int error = MB_ERROR_NO_ERROR;
  ASSERT_EQ(MB_SUCCESS, mbsvp_profile_tail(0, &primary.raw, &donor.raw, &primary.p, message, &error));
  EXPECT_THAT(primary.depth(), testing::ElementsAre(0, 1000, 2000, 3000));
  EXPECT_THAT(primary.speed(), testing::ElementsAre(1500, 1510, 1520, 1530));
}

TEST(ProfileTest, LinearEndpoint) {
  std::vector<double> z, c;
  for (double depth = 0; depth < 6000; depth += 1000) {
    z.push_back(depth);
    c.push_back(1480 + .017 * depth);
  }
  Profile profile(z, c);
  ASSERT_TRUE(Complete(&profile));
  EXPECT_EQ(11000, profile.depth().back());
  EXPECT_NEAR(1667, profile.speed().back(), 1e-9);
}

TEST(ProfileTest, TrendUsesLast2000MetersOnly) {
  Profile profile({0, 1000, 2000, 3000, 4000, 5000}, {1200, 1250, 1300, 1530, 1540, 1550});
  ASSERT_TRUE(Complete(&profile));
  EXPECT_NEAR(1610, profile.speed().back(), 1e-9);
}

TEST(ProfileTest, Chain) {
  Profile annual({0, 1000, 2000, 3000, 4000}, {1500, 1510, 1520, 1530, 1540});
  Profile seasonal({0, 1000, 2000}, {1501, 1511, NaN});
  Profile monthly({0, 500, 1000}, {1502, 1507, NaN});
  const struct mbsvp_profile *raw[MBSVP_KINDS] = {&annual.raw, &seasonal.raw, &monthly.raw};
  char message[MBSVP_MESSAGE_SIZE];
  int error = MB_ERROR_NO_ERROR;
  struct mbsvp_profile observed[MBSVP_KINDS];
  for (auto &item : observed)
    mbsvp_profile_init(&item);
  ASSERT_EQ(MB_SUCCESS, mbsvp_profile_chain(0, raw, observed, message, &error));
  const auto &m = observed[MBSVP_MONTHLY];
  EXPECT_THAT(std::vector<double>(m.depth, m.depth + m.n), testing::ElementsAre(0, 500, 1000, 2000, 3000, 4000));
  EXPECT_THAT(std::vector<double>(m.speed, m.speed + m.n), testing::ElementsAre(1502, 1507, 1511, 1520, 1530, 1540));
  for (auto &item : observed) {
    struct mbsvp_profile complete;
    mbsvp_profile_init(&complete);
    ASSERT_EQ(MB_SUCCESS, mbsvp_profile_complete(0, &item, MBSVP_TARGET_DEPTH, &complete, message, &error));
    EXPECT_EQ(11000, complete.depth[complete.n - 1]);
    mbsvp_profile_free(0, &complete, &error);
    mbsvp_profile_free(0, &item, &error);
  }
}

TEST(ProfileTest, NoSyntheticDonor) {
  Profile annual({0, 1000, 2000, 3000}, {1500, 1510, 1520, 1530});
  Profile seasonal({0, 1000}, {1500, 1510});
  const struct mbsvp_profile *raw[MBSVP_KINDS] = {&annual.raw, &seasonal.raw, nullptr};
  char message[MBSVP_MESSAGE_SIZE];
  int error = MB_ERROR_NO_ERROR;
  struct mbsvp_profile observed[MBSVP_KINDS];
  for (auto &item : observed)
    mbsvp_profile_init(&item);
  ASSERT_EQ(MB_SUCCESS, mbsvp_profile_chain(0, raw, observed, message, &error));
  EXPECT_EQ(3000, observed[MBSVP_SEASONAL].depth[observed[MBSVP_SEASONAL].n - 1]);
  EXPECT_EQ(0, observed[MBSVP_MONTHLY].n);
  for (auto &item : observed)
    mbsvp_profile_free(0, &item, &error);
}

TEST(ProfileTest, ExistingTarget) {
  Profile profile({0, 9000, 11000}, {1500, 1600, 1640});
  ASSERT_TRUE(Complete(&profile));
  EXPECT_EQ(3, profile.p.n);
  EXPECT_EQ(1640, profile.speed().back());
}

TEST(ProfileTest, ClipDeeper) {
  Profile profile({0, 10000, 12000}, {1500, 1600, 1640});
  ASSERT_TRUE(Complete(&profile));
  EXPECT_EQ(11000, profile.depth().back());
  EXPECT_EQ(1620, profile.speed().back());
}

TEST(ProfileTest, InvalidPairsRemoved) {
  Profile profile({0, NaN, 1000, 2000}, {1500, 1501, Inf, 1520});
  char message[MBSVP_MESSAGE_SIZE];
  int error = MB_ERROR_NO_ERROR;
  ASSERT_EQ(MB_SUCCESS, mbsvp_profile_clean(0, profile.raw.n, profile.raw.depth, profile.raw.speed, &profile.p, message, &error));
  EXPECT_THAT(profile.depth(), testing::ElementsAre(0, 2000));
}

TEST(ProfileTest, DuplicateDepthsRejected) {
  Profile profile({0, 1000, 1000}, {1500, 1501, 1502});
  char message[MBSVP_MESSAGE_SIZE] = "";
  int error = MB_ERROR_NO_ERROR;
  EXPECT_EQ(MB_FAILURE, mbsvp_profile_clean(0, profile.raw.n, profile.raw.depth, profile.raw.speed, &profile.p, message, &error));
  EXPECT_STREQ("Duplicate depths in profile", message);
}

TEST(ProfileTest, SparseTailRejected) {
  Profile profile({0, 1000, 5000}, {1500, 1510, 1550});
  EXPECT_FALSE(Complete(&profile));
}

TEST(ProfileTest, AllNanRejected) {
  Profile profile({0, 1000}, {NaN, NaN});
  EXPECT_FALSE(Complete(&profile));
}

TEST(ProfileTest, MissingSurfaceRejected) {
  Profile profile({5, 1000, 2000}, {1500, 1510, 1520});
  EXPECT_FALSE(Complete(&profile));
}

TEST(ProfileTest, MonthlyUsesAnnualWithoutSeasonal) {
  Profile annual({0, 1000, 2000}, {1500, 1510, 1520});
  Profile monthly({0, 500}, {1499, 1504});
  const struct mbsvp_profile *raw[MBSVP_KINDS] = {&annual.raw, nullptr, &monthly.raw};
  char message[MBSVP_MESSAGE_SIZE];
  int error = MB_ERROR_NO_ERROR;
  struct mbsvp_profile observed[MBSVP_KINDS];
  for (auto &item : observed)
    mbsvp_profile_init(&item);
  ASSERT_EQ(MB_SUCCESS, mbsvp_profile_chain(0, raw, observed, message, &error));
  EXPECT_EQ(2000, observed[MBSVP_MONTHLY].depth[observed[MBSVP_MONTHLY].n - 1]);
  for (auto &item : observed)
    mbsvp_profile_free(0, &item, &error);
}

TEST(ProfileTest, NoAnnualUsesOwnTrend) {
  Profile monthly({0, 1000, 2000}, {1500, 1510, 1520});
  const struct mbsvp_profile *raw[MBSVP_KINDS] = {nullptr, nullptr, &monthly.raw};
  char message[MBSVP_MESSAGE_SIZE];
  int error = MB_ERROR_NO_ERROR;
  struct mbsvp_profile observed[MBSVP_KINDS];
  for (auto &item : observed)
    mbsvp_profile_init(&item);
  ASSERT_EQ(MB_SUCCESS, mbsvp_profile_chain(0, raw, observed, message, &error));
  ASSERT_EQ(MB_SUCCESS, mbsvp_profile_complete(0, &observed[MBSVP_MONTHLY], MBSVP_TARGET_DEPTH, &monthly.p, message, &error));
  EXPECT_NEAR(1610, monthly.speed().back(), 1e-9);
  for (auto &item : observed)
    mbsvp_profile_free(0, &item, &error);
}

/*--------------------------------------------------------------------*/
class TempDirTest : public testing::Test {
 protected:
  void SetUp() override {
    const auto *info = testing::UnitTest::GetInstance()->current_test_info();
    dir = std::filesystem::temp_directory_path() / (std::string("mbsvp_") + info->name() + std::to_string(getpid()));
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
  }
  void TearDown() override { std::filesystem::remove_all(dir); }
  std::string Path(const std::string &name) const { return (dir / name).string(); }
  std::filesystem::path dir;
};

std::string ReadAll(const std::string &path) {
  std::ifstream in(path, std::ios::binary);
  std::stringstream text;
  text << in.rdbuf();
  return text.str();
}

/* Internal profile rules */
class InternalTest : public TempDirTest {
 protected:
  bool Valid(const std::string &text, double maximum) {
    std::ofstream(Path("p.svp")) << text;
    struct mbsvp_profile profile;
    mbsvp_profile_init(&profile);
    bool valid = false;
    int error = MB_ERROR_NO_ERROR;
    EXPECT_EQ(MB_SUCCESS, mbsvp_profile_read_internal(0, Path("p.svp").c_str(), maximum, &profile, &valid, &error));
    mbsvp_profile_free(0, &profile, &error);
    return valid;
  }
};

TEST_F(InternalTest, Rules) {
  EXPECT_TRUE(Valid("## head\n> x\n0.0 1500.0\n\n 12000.00\t1669.00\n", 5000));
  EXPECT_FALSE(Valid("0 1500\n12000 1669 3\n", 5000));     // three fields
  EXPECT_FALSE(Valid("0 1500\n12000 abc\n", 5000));        // not a number
  EXPECT_FALSE(Valid("0 1500\n", 0));                      // one sample
  EXPECT_FALSE(Valid("2 1500\n12000 1669\n", 5000));       // no surface
  EXPECT_FALSE(Valid("0 1500\n4000 1600\n", 5000));        // too short
  EXPECT_FALSE(Valid("0 1500\n0 1501\n9000 1600\n", 5000));  // not increasing
  EXPECT_FALSE(Valid("0 1200\n12000 1669\n", 5000));       // speed range
  EXPECT_FALSE(Valid("0 nan\n12000 1669\n", 5000));        // nonfinite
}

TEST(FormatTest, ShortestDouble) {
  char text[64];
  const std::vector<std::pair<double, std::string>> cases = {{1550.0, "1550.0"},
                                                             {-11.5, "-11.5"},
                                                             {0.0, "0.0"},
                                                             {93.48699345432436, "93.48699345432436"},
                                                             {1.3947890863691294e-13, "1.3947890863691294e-13"},
                                                             {1e16, "1e+16"},
                                                             {100000.0, "100000.0"},
                                                             {0.0001, "0.0001"}};
  for (const auto &item : cases) {
    mbsvp_format_double(item.first, text, sizeof(text));
    EXPECT_EQ(item.second, text);
  }
  mbsvp_format_double(NaN, text, sizeof(text));
  EXPECT_STREQ("null", text);
}

TEST(LocationTest, Comment) {
  struct mbsvp_location location;
  mbsvp_location_init(&location);
  location.has_position = true;
  location.longitude = 348.5;
  location.latitude = 37.0;
  strcpy(location.method, "nearest");
  strcpy(location.source, "WOA18");
  strcpy(location.dataset, "woa18_decav_svp00_01.nc");
  strcpy(location.formula, "chen");
  char line[1024];
  char message[MBSVP_MESSAGE_SIZE];
  int error = MB_ERROR_NO_ERROR;
  ASSERT_EQ(MB_SUCCESS, mbsvp_location_comment(0, &location, MBSVP_MARKER_LOCATION, line, sizeof(line), message, &error));
  EXPECT_STREQ("# MBSVPBEST_LOCATION {\"version\":1,\"longitude\":-11.5,\"latitude\":37.0,\"units\":\"degrees\","
               "\"method\":\"nearest\",\"source\":\"WOA18\",\"dataset\":\"woa18_decav_svp00_01.nc\",\"formula\":\"chen\"}\n",
               line);
}

TEST(LocationTest, InvalidLatitudeRejected) {
  struct mbsvp_location location;
  mbsvp_location_init(&location);
  location.has_position = true;
  location.latitude = 95.0;
  char line[1024];
  char message[MBSVP_MESSAGE_SIZE] = "";
  int error = MB_ERROR_NO_ERROR;
  EXPECT_EQ(MB_FAILURE, mbsvp_location_comment(0, &location, MBSVP_MARKER_LOCATION, line, sizeof(line), message, &error));
  EXPECT_STREQ("Invalid latitude", message);
}

TEST(LocationTest, CommentLengthLimit) {
  struct mbsvp_location location;
  mbsvp_location_init(&location);
  memset(location.dataset, 'x', sizeof(location.dataset) - 1);
  std::string marker(800, '#');
  char line[1024];
  char message[MBSVP_MESSAGE_SIZE] = "";
  int error = MB_ERROR_NO_ERROR;
  EXPECT_EQ(MB_FAILURE, mbsvp_location_comment(0, &location, marker.c_str(), line, sizeof(line), message, &error));
}

using AnnotateTest = TempDirTest;

TEST_F(AnnotateTest, KeepsCommentsAndData) {
  std::ofstream(Path("a.svp")) << "## MB-SVP head\n# MBSVPBEST_LOCATION old\n## more\n0.0 1500.0\n";
  char message[MBSVP_MESSAGE_SIZE];
  int error = MB_ERROR_NO_ERROR;
  ASSERT_EQ(MB_SUCCESS, mbsvp_location_annotate_internal(0, Path("a.svp").c_str(), message, &error));
  EXPECT_EQ("## MB-SVP head\n# MBSVPBEST_LOCATION {\"version\":1,\"longitude\":null,\"latitude\":null,\"units\":\"degrees\","
            "\"method\":\"unknown\",\"source\":\"internal\"}\n## more\n0.0 1500.0\n",
            ReadAll(Path("a.svp")));
}

TEST_F(AnnotateTest, AddsFirstComment) {
  std::ofstream(Path("a.svp")) << "0.0 1500.0\n";
  char message[MBSVP_MESSAGE_SIZE];
  int error = MB_ERROR_NO_ERROR;
  ASSERT_EQ(MB_SUCCESS, mbsvp_location_annotate_internal(0, Path("a.svp").c_str(), message, &error));
  const std::string text = ReadAll(Path("a.svp"));
  EXPECT_EQ(0u, text.find("# Internal SVP extracted by mbsvpbest\n# MBSVPBEST_LOCATION "));
  EXPECT_NE(std::string::npos, text.find("\n0.0 1500.0\n"));
}

/*--------------------------------------------------------------------*/
TEST(DiscoveryTest, Names) {
  int year = 0;
  char period[64];
  char code[3];
  char resolution[3];
  ASSERT_TRUE(mbsvp_woa_parse_name("woa23_decav91C0_svp16_04.nc", &year, period, sizeof(period), code, resolution));
  EXPECT_EQ(23, year);
  EXPECT_STREQ("decav91C0", period);
  EXPECT_STREQ("16", code);
  EXPECT_STREQ("04", resolution);
  for (const char *name : {"woa18_decav_t00_01.nc", "woa18_decav_svp17_01.nc", "woa18_decav_svp00_02.nc",
                           "woa19_decav_svp00_01.nc", "woa18__svp00_01.nc", "woa18_decav_svp00_01.nc.tmp",
                           "woa18_decav_svp00_01.txt", "woa18_decav_svpXX_01.nc", ".woa-cache"})
    EXPECT_FALSE(mbsvp_woa_parse_name(name, &year, period, sizeof(period), code, resolution)) << name;
}

TEST(BoundsTest, Circular) {
  double west = 170.0, east = -170.0;
  mbsvp_circular_bounds(&west, &east);
  EXPECT_EQ(170.0, west);
  EXPECT_EQ(190.0, east);
  west = -179.0;
  east = 179.0;
  mbsvp_circular_bounds(&west, &east);
  EXPECT_EQ(179.0, west);
  EXPECT_EQ(181.0, east);
  west = -12.0;
  east = -10.0;
  mbsvp_circular_bounds(&west, &east);
  EXPECT_EQ(-12.0, west);
  EXPECT_EQ(-10.0, east);
}

/*--------------------------------------------------------------------*/
/* A small global grid written with the shape of the real files. Speed in a cell is
   1500 + lat + (lon + 180) / 1000 at every depth, Leroy 1 m/s higher. */
class GridTest : public TempDirTest {
 protected:
  void Write(const std::string &name, std::vector<float> lat, std::vector<float> lon,
             bool (*land)(float, float) = nullptr) {
    int ncid, dlat, dlon, ddepth, vlat, vlon, vdepth, vchen, vleroy;
    ASSERT_EQ(NC_NOERR, nc_create(Path(name).c_str(), NC_CLOBBER, &ncid));
    nc_def_dim(ncid, "lat", lat.size(), &dlat);
    nc_def_dim(ncid, "lon", lon.size(), &dlon);
    const std::vector<float> depth = {0, 1000, 2000};
    nc_def_dim(ncid, "depth", depth.size(), &ddepth);
    nc_def_var(ncid, "lat", NC_FLOAT, 1, &dlat, &vlat);
    nc_def_var(ncid, "lon", NC_FLOAT, 1, &dlon, &vlon);
    nc_def_var(ncid, "depth", NC_FLOAT, 1, &ddepth, &vdepth);
    const int dims[3] = {ddepth, dlat, dlon};
    nc_def_var(ncid, "v_an_chen", NC_FLOAT, 3, dims, &vchen);
    nc_def_var(ncid, "v_an_leroy", NC_FLOAT, 3, dims, &vleroy);
    const float fill = -999.0f;
    nc_put_att_float(ncid, vchen, "_FillValue", NC_FLOAT, 1, &fill);
    nc_put_att_float(ncid, vleroy, "_FillValue", NC_FLOAT, 1, &fill);
    nc_enddef(ncid);
    nc_put_var_float(ncid, vlat, lat.data());
    nc_put_var_float(ncid, vlon, lon.data());
    nc_put_var_float(ncid, vdepth, depth.data());
    std::vector<float> chen, leroy;
    for (size_t k = 0; k < depth.size(); k++) {
      for (float y : lat) {
        for (float x : lon) {
          const bool missing = land != nullptr && land(y, x);
          const float value = 1500.0f + y + (x + 180.0f) / 1000.0f;
          chen.push_back(missing ? fill : value);
          leroy.push_back(missing ? fill : value + 1.0f);
        }
      }
    }
    nc_put_var_float(ncid, vchen, chen.data());
    nc_put_var_float(ncid, vleroy, leroy.data());
    nc_close(ncid);
  }
  static std::vector<float> Range(float first, float step, int n) {
    std::vector<float> values;
    for (int i = 0; i < n; i++)
      values.push_back(first + step * i);
    return values;
  }
  struct mbsvp_survey Survey(double west, double east, double south, double north) {
    struct mbsvp_survey survey;
    memset(&survey, 0, sizeof(survey));
    survey.west = west;
    survey.east = east;
    survey.south = south;
    survey.north = north;
    return survey;
  }
};

TEST_F(GridTest, AreaMeanBracketsSurvey) {
  Write("g.nc", Range(-89.5f, 1, 180), Range(-179.5f, 1, 360));
  auto survey = Survey(-12.0, -11.0, 36.7, 37.2);
  struct mbsvp_grid grid;
  mbsvp_grid_init(&grid);
  char message[MBSVP_MESSAGE_SIZE] = "";
  int error = MB_ERROR_NO_ERROR;
  ASSERT_EQ(MB_SUCCESS, mbsvp_woa_read_grid(0, Path("g.nc").c_str(), &survey, false, &grid, message, &error)) << message;
  EXPECT_EQ(3, grid.ndepth);
  EXPECT_EQ(-12.5, grid.location.west);
  EXPECT_EQ(-10.5, grid.location.east);
  EXPECT_EQ(36.5, grid.location.south);
  EXPECT_EQ(37.5, grid.location.north);
  EXPECT_EQ(2, grid.location.grid_lat);
  EXPECT_EQ(3, grid.location.grid_lon);
  EXPECT_EQ(-11.5, grid.location.longitude);
  EXPECT_EQ(37.0, grid.location.latitude);
  EXPECT_NEAR(1500 + 37.0 + (-11.5 + 180) / 1000, grid.chen[1], 1e-3);
  EXPECT_NEAR(grid.chen[1] + 1, grid.leroy[1], 1e-3);
  mbsvp_grid_free(0, &grid, &error);
}

TEST_F(GridTest, AreaMeanAcrossDateline) {
  Write("g.nc", Range(-89.5f, 1, 180), Range(-179.5f, 1, 360));
  auto survey = Survey(179.2, -179.2, 10.2, 10.8);
  struct mbsvp_grid grid;
  mbsvp_grid_init(&grid);
  char message[MBSVP_MESSAGE_SIZE] = "";
  int error = MB_ERROR_NO_ERROR;
  ASSERT_EQ(MB_SUCCESS, mbsvp_woa_read_grid(0, Path("g.nc").c_str(), &survey, false, &grid, message, &error)) << message;
  EXPECT_EQ(178.5, grid.location.west);
  EXPECT_EQ(-178.5, grid.location.east);
  EXPECT_EQ(4, grid.location.grid_lon);  // 178.5, 179.5, -179.5, -178.5
  EXPECT_EQ(3, grid.location.grid_lat);  // 9.5, 10.5, 11.5
  EXPECT_NEAR(1500 + 10.5 + (358.5 + 359.5 + 0.5 + 1.5) / 4000.0, grid.chen[0], 1e-3);
  mbsvp_grid_free(0, &grid, &error);
}

TEST_F(GridTest, Longitudes0To360AndDescendingLatitude) {
  auto lat = Range(89.5f, -1, 180);
  Write("g.nc", lat, Range(0.5f, 1, 360));
  auto survey = Survey(-12.0, -11.0, 36.7, 37.2);
  struct mbsvp_grid grid;
  mbsvp_grid_init(&grid);
  char message[MBSVP_MESSAGE_SIZE] = "";
  int error = MB_ERROR_NO_ERROR;
  ASSERT_EQ(MB_SUCCESS, mbsvp_woa_read_grid(0, Path("g.nc").c_str(), &survey, false, &grid, message, &error)) << message;
  EXPECT_EQ(-11.5, grid.location.longitude);
  EXPECT_EQ(37.0, grid.location.latitude);
  EXPECT_EQ(3, grid.location.grid_lon);
  EXPECT_NEAR(1500 + 37.0 + (348.5 + 180) / 1000, grid.chen[0], 1e-3);
  mbsvp_grid_free(0, &grid, &error);
}

TEST_F(GridTest, MissingValuesSkippedInMean) {
  Write("g.nc", Range(-89.5f, 1, 180), Range(-179.5f, 1, 360), [](float y, float) { return y > 37.0f; });
  auto survey = Survey(-11.6, -11.4, 36.7, 37.2);
  struct mbsvp_grid grid;
  mbsvp_grid_init(&grid);
  char message[MBSVP_MESSAGE_SIZE] = "";
  int error = MB_ERROR_NO_ERROR;
  ASSERT_EQ(MB_SUCCESS, mbsvp_woa_read_grid(0, Path("g.nc").c_str(), &survey, false, &grid, message, &error)) << message;
  EXPECT_NEAR(1500 + 36.5 + (-11.5 + 180) / 1000, grid.chen[0], 1e-3);
  mbsvp_grid_free(0, &grid, &error);
}

TEST_F(GridTest, NearestCell) {
  Write("g.nc", Range(-89.5f, 1, 180), Range(-179.5f, 1, 360));
  auto survey = Survey(-11.6, -11.4, 36.6, 36.8);
  struct mbsvp_grid grid;
  mbsvp_grid_init(&grid);
  char message[MBSVP_MESSAGE_SIZE] = "";
  int error = MB_ERROR_NO_ERROR;
  ASSERT_EQ(MB_SUCCESS, mbsvp_woa_read_grid(0, Path("g.nc").c_str(), &survey, true, &grid, message, &error)) << message;
  EXPECT_EQ(-11.5, grid.location.longitude);
  EXPECT_EQ(36.5, grid.location.latitude);
  EXPECT_STREQ("nearest", grid.location.method);
  EXPECT_FALSE(grid.location.has_area);
  mbsvp_grid_free(0, &grid, &error);
}

TEST_F(GridTest, NearestUsesNeighbourOnly) {
  Write("g.nc", Range(-89.5f, 1, 180), Range(-179.5f, 1, 360),
        [](float y, float x) { return y == 36.5f && x == -11.5f; });
  auto survey = Survey(-11.6, -11.4, 36.6, 36.8);
  struct mbsvp_grid grid;
  mbsvp_grid_init(&grid);
  char message[MBSVP_MESSAGE_SIZE] = "";
  int error = MB_ERROR_NO_ERROR;
  ASSERT_EQ(MB_SUCCESS, mbsvp_woa_read_grid(0, Path("g.nc").c_str(), &survey, true, &grid, message, &error)) << message;
  EXPECT_EQ(37.5, grid.location.latitude);
  mbsvp_grid_free(0, &grid, &error);
}

TEST_F(GridTest, NearestDoesNotSearchFar) {
  Write("g.nc", Range(-89.5f, 1, 180), Range(-179.5f, 1, 360),
        [](float y, float x) { return fabsf(y - 36.5f) < 10 && fabsf(x + 11.5f) < 10; });
  auto survey = Survey(-11.6, -11.4, 36.6, 36.8);
  struct mbsvp_grid grid;
  mbsvp_grid_init(&grid);
  char message[MBSVP_MESSAGE_SIZE] = "";
  int error = MB_ERROR_NO_ERROR;
  EXPECT_EQ(MB_FAILURE, mbsvp_woa_read_grid(0, Path("g.nc").c_str(), &survey, true, &grid, message, &error));
  EXPECT_THAT(message, testing::HasSubstr("16 nearest cells"));
  mbsvp_grid_free(0, &grid, &error);
}

TEST_F(GridTest, SurveyOutsideRegionalGridRejected) {
  Write("g.nc", Range(30.5f, 1, 5), Range(-20.5f, 1, 5));
  auto survey = Survey(-11.6, -11.4, 36.6, 36.8);
  struct mbsvp_grid grid;
  mbsvp_grid_init(&grid);
  char message[MBSVP_MESSAGE_SIZE] = "";
  int error = MB_ERROR_NO_ERROR;
  EXPECT_EQ(MB_FAILURE, mbsvp_woa_read_grid(0, Path("g.nc").c_str(), &survey, false, &grid, message, &error));
  EXPECT_STREQ("Survey outside latitude coverage", message);
  mbsvp_grid_free(0, &grid, &error);
}

TEST_F(GridTest, MissingVariablesRejected) {
  int ncid;
  ASSERT_EQ(NC_NOERR, nc_create(Path("g.nc").c_str(), NC_CLOBBER, &ncid));
  nc_close(ncid);
  auto survey = Survey(-11.6, -11.4, 36.6, 36.8);
  struct mbsvp_grid grid;
  mbsvp_grid_init(&grid);
  char message[MBSVP_MESSAGE_SIZE] = "";
  int error = MB_ERROR_NO_ERROR;
  EXPECT_EQ(MB_FAILURE, mbsvp_woa_read_grid(0, Path("g.nc").c_str(), &survey, false, &grid, message, &error));
  EXPECT_THAT(message, testing::HasSubstr("Missing sound-speed variables"));
  mbsvp_grid_free(0, &grid, &error);
}

/* Discovery of candidates in a folder, with the survey in January. */
TEST_F(GridTest, GenerateMixedEditionsAndKinds) {
  std::filesystem::create_directories(dir / "out");
  const auto lat = Range(-89.5f, 1, 180);
  const auto lon = Range(-179.5f, 1, 360);
  Write("woa18_decav_svp00_01.nc", lat, lon);
  Write("woa18_decav_svp13_01.nc", lat, lon);
  Write("woa18_decav_svp01_01.nc", lat, lon);
  Write("woa23_decav_svp00_01.nc", lat, lon);
  Write("woa23_B5C2_svp00_01.nc", lat, lon);   // other period
  Write("woa23_decav_svp00_04.nc", lat, lon);  // other resolution
  Write("woa23_decav_svp02_01.nc", lat, lon);  // other month
  Write("woa18_decav_t00_01.nc", lat, lon);    // raw source
  auto survey = Survey(-12.0, -11.0, 36.7, 37.2);
  strcpy(survey.month, "01");
  strcpy(survey.season, "13");
  survey.depth = 1000;
  struct mbsvp_strings periods, resolutions, candidates, notes;
  mbsvp_strings_init(&periods);
  mbsvp_strings_init(&resolutions);
  mbsvp_strings_init(&candidates);
  mbsvp_strings_init(&notes);
  int error = MB_ERROR_NO_ERROR;
  mbsvp_strings_add(0, &periods, "decav", &error);
  mbsvp_strings_add(0, &resolutions, "01", &error);
  char message[MBSVP_MESSAGE_SIZE] = "";
  ASSERT_EQ(MB_SUCCESS, mbsvp_woa_generate(0, dir.string().c_str(), Path("out").c_str(), &survey, &periods, &resolutions, false,
                                           &candidates, &notes, message, &error))
      << message;
  std::vector<std::string> names;
  for (int i = 0; i < candidates.n; i++)
    names.push_back(mbsvp_basename(candidates.items[i]));
  EXPECT_THAT(names, testing::ElementsAre("woa18_decav_svp13_01_chen.svp", "woa18_decav_svp01_01_chen.svp",
                                          "woa18_decav_svp00_01_chen.svp", "woa18_decav_svp13_01_leroy.svp",
                                          "woa18_decav_svp01_01_leroy.svp", "woa18_decav_svp00_01_leroy.svp",
                                          "woa23_decav_svp00_01_chen.svp", "woa23_decav_svp00_01_leroy.svp"));
  // the WOA23 season and month are missing: labelled notes, the annual goes on
  ASSERT_EQ(2, notes.n);
  EXPECT_THAT(notes.items[0], testing::StartsWith("WOA23: woa23_decav_svp13_01:"));
  const std::string text = ReadAll(Path("out/woa18_decav_svp01_01_chen.svp"));
  EXPECT_THAT(text, testing::StartsWith("# WOA18; period=decav; time=01; resolution=01; formula=chen;"));
  EXPECT_THAT(text, testing::HasSubstr("\n# MBSVPBEST_LOCATION {\"version\":1,"));
  EXPECT_THAT(text, testing::HasSubstr("\n2000.000000 "));
  EXPECT_THAT(text, testing::HasSubstr("\n11000.000000 "));
  mbsvp_strings_free(0, &periods, &error);
  mbsvp_strings_free(0, &resolutions, &error);
  mbsvp_strings_free(0, &candidates, &error);
  mbsvp_strings_free(0, &notes, &error);
}

TEST_F(GridTest, GenerateNoMatchNotesAndTooDeepFails) {
  std::filesystem::create_directories(dir / "out");
  auto survey = Survey(-12.0, -11.0, 36.7, 37.2);
  strcpy(survey.month, "01");
  strcpy(survey.season, "13");
  struct mbsvp_strings periods, resolutions, candidates, notes;
  mbsvp_strings_init(&periods);
  mbsvp_strings_init(&resolutions);
  mbsvp_strings_init(&candidates);
  mbsvp_strings_init(&notes);
  int error = MB_ERROR_NO_ERROR;
  mbsvp_strings_add(0, &periods, "decav", &error);
  mbsvp_strings_add(0, &resolutions, "01", &error);
  char message[MBSVP_MESSAGE_SIZE] = "";
  ASSERT_EQ(MB_SUCCESS, mbsvp_woa_generate(0, dir.string().c_str(), Path("out").c_str(), &survey, &periods, &resolutions, false,
                                           &candidates, &notes, message, &error));
  EXPECT_EQ(0, candidates.n);
  ASSERT_EQ(1, notes.n);
  EXPECT_THAT(notes.items[0], testing::HasSubstr("No WOA18/WOA23 sound-speed NetCDF files match"));
  survey.depth = 11000.5;
  EXPECT_EQ(MB_FAILURE, mbsvp_woa_generate(0, dir.string().c_str(), Path("out").c_str(), &survey, &periods, &resolutions, false,
                                           &candidates, &notes, message, &error));
  EXPECT_THAT(message, testing::HasSubstr("exceeds"));
  mbsvp_strings_free(0, &periods, &error);
  mbsvp_strings_free(0, &resolutions, &error);
  mbsvp_strings_free(0, &candidates, &error);
  mbsvp_strings_free(0, &notes, &error);
}

/*--------------------------------------------------------------------*/
TEST(ScoreTest, PlaneHasNoResidual) {
  struct mbsvp_score score;
  mbsvp_score_init(&score, false);
  const std::vector<int> beam = {0, 1, 2, 3, 4};
  const std::vector<double> x = {-200, -100, 0, 100, 200};
  const std::vector<double> z = {4900, 4950, 5000, 5050, 5100};
  int error = MB_ERROR_NO_ERROR;
  ASSERT_EQ(MB_SUCCESS, mbsvp_score_ping(0, 5, beam.data(), x.data(), z.data(), 5, &score, &error));
  EXPECT_EQ(5, score.soundings);
  EXPECT_EQ(1, score.pings);
  EXPECT_NEAR(0.0, score.sum2, 1e-18);
}

TEST(ScoreTest, ResidualsAreSymmetricAboutFit) {
  struct mbsvp_score score;
  mbsvp_score_init(&score, true);
  const std::vector<int> beam = {0, 1, 2};
  const std::vector<double> x = {-1, 0, 1};
  const std::vector<double> z = {1, 0, 1};
  int error = MB_ERROR_NO_ERROR;
  ASSERT_EQ(MB_SUCCESS, mbsvp_score_ping(0, 3, beam.data(), x.data(), z.data(), 3, &score, &error));
  EXPECT_NEAR(1.0 / 9 + 4.0 / 9 + 1.0 / 9, score.sum2, 1e-12);
  EXPECT_NEAR(0.0, score.sum, 1e-12);
  EXPECT_EQ(3, score.nbeams);
  EXPECT_NEAR(-2.0 / 3, score.beam_sum[1], 1e-12);
  mbsvp_score_free(0, &score, &error);
}

TEST(ScoreTest, RejectedPings) {
  struct mbsvp_score score;
  mbsvp_score_init(&score, false);
  const std::vector<int> beam = {0, 1, 2, 3, 4};
  const std::vector<double> z = {1, 2, 3, 4, 5};
  const std::vector<double> few = {0, 1, NaN, Inf, NaN};
  const std::vector<double> same = {5, 5, 5, 5, 5};
  int error = MB_ERROR_NO_ERROR;
  ASSERT_EQ(MB_SUCCESS, mbsvp_score_ping(0, 5, beam.data(), few.data(), z.data(), 3, &score, &error));
  ASSERT_EQ(MB_SUCCESS, mbsvp_score_ping(0, 5, beam.data(), same.data(), z.data(), 3, &score, &error));
  EXPECT_EQ(2, score.rejected_pings);
  ASSERT_EQ(MB_SUCCESS, mbsvp_score_ping(0, 0, beam.data(), same.data(), z.data(), 3, &score, &error));
  EXPECT_EQ(2, score.rejected_pings);  // a ping without listed beams is not a rejected ping
  EXPECT_EQ(0, score.soundings);
}

using UtilTest = TempDirTest;

TEST(PathTest, BasenameAndSuffix) {
  EXPECT_STREQ("b.all", mbsvp_basename("/x/a.y/b.all"));
  EXPECT_STREQ(".all", mbsvp_suffix("/x/a.y/b.all"));
  EXPECT_STREQ("", mbsvp_suffix("/x/a.y/b"));
  EXPECT_STREQ("", mbsvp_suffix("/x/.hidden"));
}

TEST(QuoteTest, Quotes) {
  char out[64];
  EXPECT_STREQ("'a b'", mbsvp_quote("a b", out, sizeof(out)));
  EXPECT_STREQ("'it'\\''s'", mbsvp_quote("it's", out, sizeof(out)));
  EXPECT_EQ(nullptr, mbsvp_quote("too long", out, 4));
}

TEST_F(UtilTest, DirectoriesAndCommands) {
  int error = MB_ERROR_NO_ERROR;
  const std::string deep = Path("a/b c/d");
  ASSERT_EQ(MB_SUCCESS, mbsvp_make_dirs(0, deep.c_str(), &error));
  EXPECT_TRUE(mbsvp_is_dir(deep.c_str()));
  ASSERT_EQ(MB_SUCCESS, mbsvp_make_dirs(0, deep.c_str(), &error));

  char first[1024], second[1024];
  ASSERT_EQ(MB_SUCCESS, mbsvp_make_unique_dir(0, deep.c_str(), "run", first, sizeof(first), &error));
  ASSERT_EQ(MB_SUCCESS, mbsvp_make_unique_dir(0, deep.c_str(), "run", second, sizeof(second), &error));
  EXPECT_STRNE(first, second);

  const std::string out = deep + "/out file";
  const std::string log = deep + "/log";
  const char *argv[] = {"sh", "-c", "echo out; echo err >&2; exit 3", nullptr};
  int exit_code = 0;
  EXPECT_EQ(MB_FAILURE, mbsvp_run_command(0, argv, out.c_str(), log.c_str(), &exit_code, &error));
  EXPECT_EQ(3, exit_code);
  EXPECT_EQ("out\n", ReadAll(out));
  EXPECT_EQ("err\n", ReadAll(log));
  EXPECT_EQ(0, mbsvp_interrupted);

  ASSERT_EQ(MB_SUCCESS, mbsvp_copy_file(0, out.c_str(), (deep + "/copy").c_str(), &error));
  EXPECT_EQ("out\n", ReadAll(deep + "/copy"));

  struct mbsvp_strings names;
  mbsvp_strings_init(&names);
  ASSERT_EQ(MB_SUCCESS, mbsvp_list_dir(0, first, &names, &error));
  EXPECT_EQ(0, names.n);
  mbsvp_strings_free(0, &names, &error);

  error = MB_ERROR_NO_ERROR;
  ASSERT_EQ(MB_SUCCESS, mbsvp_link_or_copy(0, out.c_str(), (std::string(first) + "/link").c_str(), &error));
  ASSERT_EQ(MB_SUCCESS, mbsvp_remove_dir(0, first, &error));
  EXPECT_FALSE(mbsvp_is_dir(first));
  EXPECT_TRUE(mbsvp_is_file(out.c_str()));  // the target of a link stays
}

TEST(CommandTest, InterruptedChildIsSeen) {
  const char *argv[] = {"sh", "-c", "kill -INT $$", nullptr};
  int exit_code = 0;
  int error = MB_ERROR_NO_ERROR;
  mbsvp_interrupted = 0;
  EXPECT_EQ(MB_FAILURE, mbsvp_run_command(0, argv, nullptr, nullptr, &exit_code, &error));
  EXPECT_EQ(1, mbsvp_interrupted);
  mbsvp_interrupted = 0;
}

}  // namespace

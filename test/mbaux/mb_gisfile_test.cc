// Tests of the MB-System polyline, polygon, and point file formats and
// shapefile import and export in src/mbaux/mb_gisfile.c.
//
// See README.md file for copying and redistribution conditions.

#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_status.h"

#include "gdal.h"
#include "ogr_api.h"
#include "ogr_srs_api.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {

const int verbose = 0;

// Scratch directory removed after each test
class MbGisFile : public ::testing::Test {
 protected:
  void SetUp() override {
    const ::testing::TestInfo *info = ::testing::UnitTest::GetInstance()->current_test_info();
    dir_ = std::filesystem::temp_directory_path() / (std::string("mb_gisfile_test_") + info->name());
    std::filesystem::remove_all(dir_);
    std::filesystem::create_directories(dir_);
  }
  void TearDown() override { std::filesystem::remove_all(dir_); }
  std::string Path(const std::string &name) const { return (dir_ / name).string(); }

  std::filesystem::path dir_;
};

struct GisData {
  explicit GisData(int type) {
    int error = MB_ERROR_NO_ERROR;
    EXPECT_EQ(MB_SUCCESS, mb_gis_init(verbose, type, &gis, &error));
  }
  ~GisData() {
    int error = MB_ERROR_NO_ERROR;
    mb_gis_deall(verbose, &gis, &error);
  }
  int AddLayer(const char *name) {
    int error = MB_ERROR_NO_ERROR;
    int ilayer = -1;
    EXPECT_EQ(MB_SUCCESS, mb_gis_addlayer(verbose, gis, name, &ilayer, &error));
    return ilayer;
  }
  struct mb_gisdata_struct *gis = nullptr;
};

// Two layers; a two part polyline and a one part polyline
void AddPolylines(GisData &data) {
  int error = MB_ERROR_NO_ERROR;
  const int layer0 = data.AddLayer("cable routes");
  const int layer1 = data.AddLayer("tracks, 2026");
  const double lon[5] = {-122.1, -122.0, -121.9, -121.95, -121.85};
  const double lat[5] = {36.70, 36.75, 36.80, 36.60, 36.65};
  const double z[5] = {-800.0, -850.5, -900.25, -10.0, -20.0};
  const int partstart[2] = {0, 3};
  ASSERT_EQ(MB_SUCCESS, mb_gis_addfeature(verbose, data.gis, layer0, 6, 2, MB_GIS_COLOR_NONE, "Cable route A, west", 2, partstart,
                                          nullptr, 5, lon, lat, z, &error));
  ASSERT_EQ(MB_SUCCESS, mb_gis_addfeature(verbose, data.gis, layer1, 4, 1, MB_GIS_COLOR_NONE, "Track 2", 1, nullptr, nullptr, 2,
                                          lon, lat, nullptr, &error));
}

// One polygon with an outer ring and a hole, one with an unclosed ring
void AddPolygons(GisData &data) {
  int error = MB_ERROR_NO_ERROR;
  const int layer0 = data.AddLayer("closures");
  const double lon[10] = {-122.0, -121.0, -121.0, -122.0, -122.0, -121.8, -121.2, -121.2, -121.8, -121.8};
  const double lat[10] = {36.0, 36.0, 37.0, 37.0, 36.0, 36.2, 36.2, 36.8, 36.8, 36.2};
  const int partstart[2] = {0, 5};
  const int parttype[2] = {MB_GIS_PART_OUTER, MB_GIS_PART_HOLE};
  ASSERT_EQ(MB_SUCCESS, mb_gis_addfeature(verbose, data.gis, layer0, 7, 2, 3, "Closure with hole", 2, partstart, parttype, 10,
                                          lon, lat, nullptr, &error));
  const double lon2[3] = {-121.5, -121.4, -121.45};
  const double lat2[3] = {36.5, 36.5, 36.6};
  ASSERT_EQ(MB_SUCCESS, mb_gis_addfeature(verbose, data.gis, layer0, 1, 1, MB_GIS_COLOR_NONE, "Triangle", 1, nullptr, nullptr, 3,
                                          lon2, lat2, nullptr, &error));
}

void AddPoints(GisData &data) {
  int error = MB_ERROR_NO_ERROR;
  const int layer0 = data.AddLayer("moorings");
  const double lon[3] = {-122.0, -121.5, -121.25};
  const double lat[3] = {36.7, 36.8, 36.9};
  const double z[3] = {-1000.0, -1500.125, -2000.0};
  const char *names[3] = {"M1", "M2, deep", ""};
  for (int i = 0; i < 3; i++) {
    ASSERT_EQ(MB_SUCCESS, mb_gis_addfeature(verbose, data.gis, layer0, i + 3, i + 1, MB_GIS_COLOR_NONE, names[i], 1, nullptr,
                                            nullptr, 1, &lon[i], &lat[i], &z[i], &error));
  }
}

void ExpectSameFeatures(const struct mb_gisdata_struct *a, const struct mb_gisdata_struct *b, bool comparez) {
  ASSERT_EQ(a->type, b->type);
  ASSERT_EQ(a->nfeature, b->nfeature);
  for (int i = 0; i < a->nfeature; i++) {
    const struct mb_gisfeature_struct *fa = &a->features[i];
    const struct mb_gisfeature_struct *fb = &b->features[i];
    EXPECT_STREQ(a->layers[fa->layer].name, b->layers[fb->layer].name) << "feature " << i;
    EXPECT_EQ(fa->color, fb->color) << "feature " << i;
    EXPECT_EQ(fa->size, fb->size) << "feature " << i;
    EXPECT_EQ(fa->fillcolor, fb->fillcolor) << "feature " << i;
    EXPECT_STREQ(fa->name, fb->name) << "feature " << i;
    ASSERT_EQ(fa->npart, fb->npart) << "feature " << i;
    for (int j = 0; j < fa->npart; j++) {
      EXPECT_EQ(fa->partstart[j], fb->partstart[j]) << "feature " << i << " part " << j;
      EXPECT_EQ(fa->parttype[j], fb->parttype[j]) << "feature " << i << " part " << j;
    }
    ASSERT_EQ(fa->npoint, fb->npoint) << "feature " << i;

    // shapefiles store outer rings clockwise and holes counterclockwise, so
    // GDAL may reverse polygon rings; compare those in reverse order
    if (a->type == MB_GIS_POLYGON) {
      for (int j = 0; j < fa->npart; j++) {
        const int start = fa->partstart[j];
        const int end = j < fa->npart - 1 ? fa->partstart[j + 1] : fa->npoint;
        bool forward = true;
        bool reverse = true;
        for (int k = start; k < end; k++) {
          const int kr = end - 1 - (k - start);
          forward = forward && std::fabs(fa->lon[k] - fb->lon[k]) < 1.0e-9 && std::fabs(fa->lat[k] - fb->lat[k]) < 1.0e-9;
          reverse = reverse && std::fabs(fa->lon[k] - fb->lon[kr]) < 1.0e-9 && std::fabs(fa->lat[k] - fb->lat[kr]) < 1.0e-9;
        }
        EXPECT_TRUE(forward || reverse) << "feature " << i << " ring " << j;
      }
      continue;
    }
    for (int k = 0; k < fa->npoint; k++) {
      EXPECT_NEAR(fa->lon[k], fb->lon[k], 1.0e-9) << "feature " << i << " point " << k;
      EXPECT_NEAR(fa->lat[k], fb->lat[k], 1.0e-9) << "feature " << i << " point " << k;
      if (comparez) {
        EXPECT_NEAR(fa->z[k], fb->z[k], 1.0e-3) << "feature " << i << " point " << k;
      }
    }
  }
}

// Create a shapefile with GDAL directly: EPSG code (0 = no .prj), geometries as WKT, NAME values
void MakeShapefile(const std::string &file, int epsg, OGRwkbGeometryType type, const std::vector<std::string> &wkt,
                   const std::vector<std::string> &names) {
  GDALAllRegister();
  GDALDriverH driver = GDALGetDriverByName("ESRI Shapefile");
  ASSERT_NE(nullptr, driver);
  GDALDatasetH dataset = GDALCreate(driver, file.c_str(), 0, 0, 0, GDT_Unknown, nullptr);
  ASSERT_NE(nullptr, dataset);
  OGRSpatialReferenceH srs = nullptr;
  if (epsg > 0) {
    srs = OSRNewSpatialReference(nullptr);
    OSRImportFromEPSG(srs, epsg);
  }
  OGRLayerH layer = GDALDatasetCreateLayer(dataset, "test", srs, type, nullptr);
  ASSERT_NE(nullptr, layer);
  OGRFieldDefnH field = OGR_Fld_Create("Name", OFTString);
  OGR_L_CreateField(layer, field, TRUE);
  OGR_Fld_Destroy(field);
  for (size_t i = 0; i < wkt.size(); i++) {
    OGRFeatureH feature = OGR_F_Create(OGR_L_GetLayerDefn(layer));
    OGR_F_SetFieldString(feature, 0, names[i].c_str());
    OGRGeometryH geometry = nullptr;
    char *text = const_cast<char *>(wkt[i].c_str());
    ASSERT_EQ(OGRERR_NONE, OGR_G_CreateFromWkt(&text, nullptr, &geometry));
    OGR_F_SetGeometryDirectly(feature, geometry);
    ASSERT_EQ(OGRERR_NONE, OGR_L_CreateFeature(layer, feature));
    OGR_F_Destroy(feature);
  }
  if (srs != nullptr)
    OSRDestroySpatialReference(srs);
  GDALClose(dataset);
}

// Create a geographic polyline shapefile with the given attribute fields; each
// feature is a short line with these field values, an empty value leaves the field unset
void MakeShapefileFields(const std::string &file, const std::vector<std::pair<std::string, OGRFieldType>> &fields,
                         const std::vector<std::vector<std::string>> &values) {
  GDALAllRegister();
  GDALDriverH driver = GDALGetDriverByName("ESRI Shapefile");
  ASSERT_NE(nullptr, driver);
  GDALDatasetH dataset = GDALCreate(driver, file.c_str(), 0, 0, 0, GDT_Unknown, nullptr);
  ASSERT_NE(nullptr, dataset);
  OGRSpatialReferenceH srs = OSRNewSpatialReference(nullptr);
  OSRImportFromEPSG(srs, 4326);
  OGRLayerH layer = GDALDatasetCreateLayer(dataset, std::filesystem::path(file).stem().string().c_str(), srs, wkbLineString,
                                           nullptr);
  ASSERT_NE(nullptr, layer);
  for (const auto &f : fields) {
    OGRFieldDefnH field = OGR_Fld_Create(f.first.c_str(), f.second);
    OGR_L_CreateField(layer, field, TRUE);
    OGR_Fld_Destroy(field);
  }
  for (size_t i = 0; i < values.size(); i++) {
    OGRFeatureH feature = OGR_F_Create(OGR_L_GetLayerDefn(layer));
    for (size_t j = 0; j < fields.size(); j++) {
      if (!values[i][j].empty())
        OGR_F_SetFieldString(feature, static_cast<int>(j), values[i][j].c_str());
    }
    OGRGeometryH geometry = OGR_G_CreateGeometry(wkbLineString);
    OGR_G_AddPoint_2D(geometry, -121.0 - 0.1 * static_cast<double>(i), 34.0);
    OGR_G_AddPoint_2D(geometry, -121.0 - 0.1 * static_cast<double>(i), 34.1);
    OGR_F_SetGeometryDirectly(feature, geometry);
    ASSERT_EQ(OGRERR_NONE, OGR_L_CreateFeature(layer, feature));
    OGR_F_Destroy(feature);
  }
  OSRDestroySpatialReference(srs);
  GDALClose(dataset);
}

std::vector<std::string> ImportNames(const std::string &file) {
  int error = MB_ERROR_NO_ERROR;
  GisData in(MB_GIS_POLYLINE);
  int nimport = 0, nskip = 0;
  EXPECT_EQ(MB_SUCCESS, mb_gis_import_shapefile(verbose, file.c_str(), nullptr, 5, 1, -1, in.gis, &nimport, &nskip, &error));
  std::vector<std::string> names;
  for (int i = 0; i < in.gis->nfeature; i++)
    names.push_back(in.gis->features[i].name);
  return names;
}

TEST_F(MbGisFile, ShapefileNameRule) {
  using ::testing::ElementsAre;

  // NAME, LABEL, or TITLE (any case) wins over ID and other text fields
  const std::string named = Path("named.shp");
  MakeShapefileFields(named, {{"id", OFTInteger}, {"source", OFTString}, {"Label", OFTString}},
                      {{"1", "digitized", "North rift"}, {"2", "digitized", "South rift"}});
  EXPECT_THAT(ImportNames(named), ElementsAre("North rift", "South rift"));
  const std::string titled = Path("titled.shp");
  MakeShapefileFields(titled, {{"comment", OFTString}, {"title", OFTString}}, {{"x", "Summit"}});
  EXPECT_THAT(ImportNames(titled), ElementsAre("Summit"));

  // otherwise an ID or *_ID field gives "<layer> <id>", even with other text fields
  // (as in a shapefile whose only text field is the same for every feature);
  // a feature without an id gets "<layer> <feature id>"
  const std::string ids = Path("study_areas.shp");
  MakeShapefileFields(ids, {{"area_id", OFTInteger}, {"source", OFTString}},
                      {{"1", "Digitized from supplied PNG"}, {"12", "Digitized from supplied PNG"}, {"", "Digitized"}});
  EXPECT_THAT(ImportNames(ids), ElementsAre("study_areas 1", "study_areas 12", "study_areas 2"));

  // a field merely ending in "ID" is not an id field: use the first text field
  const std::string notid = Path("notid.shp");
  MakeShapefileFields(notid, {{"PAID", OFTInteger}, {"remark", OFTString}}, {{"7", "Vent field"}});
  EXPECT_THAT(ImportNames(notid), ElementsAre("Vent field"));

  // no usable field: "<layer> <feature id>"
  const std::string bare = Path("bare.shp");
  MakeShapefileFields(bare, {{"depth", OFTReal}}, {{"1500"}, {"1600"}});
  EXPECT_THAT(ImportNames(bare), ElementsAre("bare 0", "bare 1"));
}

TEST_F(MbGisFile, InitRejectsBadType) {
  int error = MB_ERROR_NO_ERROR;
  struct mb_gisdata_struct *gis = nullptr;
  EXPECT_EQ(MB_FAILURE, mb_gis_init(verbose, 9, &gis, &error));
  EXPECT_EQ(MB_ERROR_BAD_PARAMETER, error);
  EXPECT_EQ(nullptr, gis);
}

TEST_F(MbGisFile, AddFeatureValidates) {
  GisData polylines(MB_GIS_POLYLINE);
  int error = MB_ERROR_NO_ERROR;
  const double lon[3] = {-122.0, -121.0, -120.0};
  const double lat[3] = {36.0, 36.5, 37.0};
  const double badlat[2] = {36.0, 91.0};

  // no layer yet
  EXPECT_EQ(MB_FAILURE, mb_gis_addfeature(verbose, polylines.gis, 0, 1, 1, -1, "x", 1, nullptr, nullptr, 2, lon, lat, nullptr,
                                          &error));
  EXPECT_EQ(MB_ERROR_BAD_PARAMETER, error);
  polylines.AddLayer("layer");

  // color out of range, size < 1, single point part, bad latitude
  EXPECT_EQ(MB_FAILURE, mb_gis_addfeature(verbose, polylines.gis, 0, 8, 1, -1, "x", 1, nullptr, nullptr, 2, lon, lat, nullptr,
                                          &error));
  EXPECT_EQ(MB_FAILURE, mb_gis_addfeature(verbose, polylines.gis, 0, 1, 0, -1, "x", 1, nullptr, nullptr, 2, lon, lat, nullptr,
                                          &error));
  EXPECT_EQ(MB_FAILURE, mb_gis_addfeature(verbose, polylines.gis, 0, 1, 1, -1, "x", 1, nullptr, nullptr, 1, lon, lat, nullptr,
                                          &error));
  EXPECT_EQ(MB_FAILURE, mb_gis_addfeature(verbose, polylines.gis, 0, 1, 1, -1, "x", 1, nullptr, nullptr, 2, lon, badlat, nullptr,
                                          &error));
  const int partstart[2] = {0, 2};
  EXPECT_EQ(MB_FAILURE, mb_gis_addfeature(verbose, polylines.gis, 0, 1, 1, -1, "x", 2, partstart, nullptr, 3, lon, lat, nullptr,
                                          &error));
  EXPECT_EQ(0, polylines.gis->nfeature);

  // names lose line breaks
  EXPECT_EQ(MB_SUCCESS, mb_gis_addfeature(verbose, polylines.gis, 0, 1, 1, -1, "two\nlines\r", 1, nullptr, nullptr, 3, lon, lat,
                                          nullptr, &error));
  EXPECT_STREQ("two lines ", polylines.gis->features[0].name);
  EXPECT_EQ(MB_GIS_COLOR_NONE, polylines.gis->features[0].fillcolor);

  // points must be single points
  GisData points(MB_GIS_POINT);
  points.AddLayer("layer");
  EXPECT_EQ(MB_FAILURE, mb_gis_addfeature(verbose, points.gis, 0, 1, 1, -1, "x", 1, nullptr, nullptr, 2, lon, lat, nullptr,
                                          &error));
}

TEST_F(MbGisFile, PolygonRingsAreClosed) {
  GisData polygons(MB_GIS_POLYGON);
  AddPolygons(polygons);
  ASSERT_EQ(2, polygons.gis->nfeature);
  EXPECT_EQ(10, polygons.gis->features[0].npoint);
  const struct mb_gisfeature_struct *triangle = &polygons.gis->features[1];
  ASSERT_EQ(4, triangle->npoint);
  EXPECT_EQ(triangle->lon[0], triangle->lon[3]);
  EXPECT_EQ(triangle->lat[0], triangle->lat[3]);
  EXPECT_EQ(MB_GIS_PART_OUTER, triangle->parttype[0]);
}

TEST_F(MbGisFile, NativeRoundTrip) {
  int error = MB_ERROR_NO_ERROR;
  for (int type = MB_GIS_POLYLINE; type <= MB_GIS_POINT; type++) {
    GisData out(type);
    if (type == MB_GIS_POLYLINE)
      AddPolylines(out);
    else if (type == MB_GIS_POLYGON)
      AddPolygons(out);
    else
      AddPoints(out);
    out.gis->hasz = type != MB_GIS_POLYGON;
    const std::string file = Path("roundtrip" + std::to_string(type) + ".txt");
    ASSERT_EQ(MB_SUCCESS, mb_gis_write(verbose, file.c_str(), "mb_gisfile_test", out.gis, &error));

    // reading into a gis of unknown type sets the type
    GisData in(MB_GIS_NONE);
    ASSERT_EQ(MB_SUCCESS, mb_gis_read(verbose, file.c_str(), in.gis, &error)) << "type " << type;
    EXPECT_EQ(MB_ERROR_NO_ERROR, error);
    EXPECT_EQ(out.gis->nlayer, in.gis->nlayer);
    EXPECT_EQ(out.gis->hasz, in.gis->hasz);
    ExpectSameFeatures(out.gis, in.gis, true);

    // a file of another type is rejected
    GisData other(type == MB_GIS_POINT ? MB_GIS_POLYLINE : MB_GIS_POINT);
    EXPECT_EQ(MB_FAILURE, mb_gis_read(verbose, file.c_str(), other.gis, &error));
    EXPECT_EQ(MB_ERROR_BAD_FORMAT, error);
  }
}

TEST_F(MbGisFile, NativeReadRejectsBadFiles) {
  int error = MB_ERROR_NO_ERROR;
  const std::vector<std::string> bad = {
      // no header
      "> ## STARTPOLYLINE 1,2,1,1,0,a\n> ## PART 0,2\n-122,36,0\n-121,36,0\n> ## ENDPOLYLINE\n",
      // newer major version
      "## Polyline File Version 2.00\n",
      // point count does not match
      "## Polyline File Version 1.00\n> ## STARTPOLYLINE 1,3,1,1,0,a\n> ## PART 0,3\n-122,36,0\n-121,36,0\n> ## ENDPOLYLINE\n",
      // part count does not match
      "## Polyline File Version 1.00\n> ## STARTPOLYLINE 2,3,1,1,0,a\n> ## PART 0,3\n-122,36,0\n-121,36,0\n-120,36,0\n"
      "> ## ENDPOLYLINE\n",
      // ends inside a feature
      "## Polyline File Version 1.00\n> ## STARTPOLYLINE 1,2,1,1,0,a\n> ## PART 0,2\n-122,36,0\n-121,36,0\n",
      // point before a part
      "## Polyline File Version 1.00\n> ## STARTPOLYLINE 1,2,1,1,0,a\n-122,36,0\n",
      // color out of range
      "## Polyline File Version 1.00\n> ## STARTPOLYLINE 1,2,9,1,0,a\n> ## PART 0,2\n-122,36,0\n-121,36,0\n> ## ENDPOLYLINE\n",
      // negative layer
      "## Point File Version 1.00\n-122,36,0,1,1,-1,a\n",
      // bad ring type
      "## Polygon File Version 1.00\n> ## STARTPOLYGON 1,4,1,1,-1,0,a\n> ## RING 0,4,INNER\n",
  };
  for (size_t i = 0; i < bad.size(); i++) {
    const std::string file = Path("bad" + std::to_string(i) + ".txt");
    std::ofstream(file) << bad[i];
    GisData in(MB_GIS_NONE);
    EXPECT_EQ(MB_FAILURE, mb_gis_read(verbose, file.c_str(), in.gis, &error)) << "case " << i;
    EXPECT_EQ(MB_ERROR_BAD_FORMAT, error) << "case " << i;
  }

  GisData missing(MB_GIS_NONE);
  EXPECT_EQ(MB_FAILURE, mb_gis_read(verbose, Path("nonexistent.txt").c_str(), missing.gis, &error));
  EXPECT_EQ(MB_ERROR_OPEN_FAIL, error);
}

TEST_F(MbGisFile, NativeReadWithoutLayerLines) {
  int error = MB_ERROR_NO_ERROR;
  const std::string file = Path("sites.pts");
  std::ofstream(file) << "## Point File Version 1.00\n# a comment\n\n"
                         "-122.5,36.5,-100,3,2,0,First site\n-122.4,36.6,-200,4,2,1,\n";
  GisData in(MB_GIS_POINT);
  ASSERT_EQ(MB_SUCCESS, mb_gis_read(verbose, file.c_str(), in.gis, &error));
  ASSERT_EQ(2, in.gis->nfeature);
  ASSERT_EQ(2, in.gis->nlayer);
  EXPECT_STREQ("sites", in.gis->layers[0].name);
  EXPECT_STREQ("sites 1", in.gis->layers[1].name);
  EXPECT_STREQ("First site", in.gis->features[0].name);
  EXPECT_STREQ("", in.gis->features[1].name);
  EXPECT_EQ(1, in.gis->features[1].layer);
}

TEST_F(MbGisFile, ShapefileRoundTrip) {
  int error = MB_ERROR_NO_ERROR;
  for (int type = MB_GIS_POLYLINE; type <= MB_GIS_POINT; type++) {
    GisData out(type);
    if (type == MB_GIS_POLYLINE)
      AddPolylines(out);
    else if (type == MB_GIS_POLYGON)
      AddPolygons(out);
    else
      AddPoints(out);
    out.gis->hasz = type != MB_GIS_POLYGON;

    // the .shp suffix is added
    const std::string base = Path("export" + std::to_string(type));
    ASSERT_EQ(MB_SUCCESS, mb_gis_export_shapefile(verbose, base.c_str(), out.gis, &error)) << "type " << type;
    const std::string file = base + ".shp";
    EXPECT_TRUE(std::filesystem::exists(file));
    EXPECT_TRUE(std::filesystem::exists(base + ".prj"));
    EXPECT_TRUE(std::filesystem::exists(base + ".cpg"));

    // exporting again replaces the shapefile
    ASSERT_EQ(MB_SUCCESS, mb_gis_export_shapefile(verbose, file.c_str(), out.gis, &error));

    int filetype = MB_GIS_NONE;
    ASSERT_EQ(MB_SUCCESS, mb_gis_shapefile_type(verbose, file.c_str(), &filetype, &error));
    EXPECT_EQ(type, filetype);

    GisData in(type);
    int nimport = 0;
    int nskip = 0;
    ASSERT_EQ(MB_SUCCESS, mb_gis_import_shapefile(verbose, file.c_str(), nullptr, 0, 1, MB_GIS_COLOR_NONE, in.gis, &nimport,
                                                  &nskip, &error));
    EXPECT_EQ(out.gis->nfeature, nimport);
    EXPECT_EQ(0, nskip);
    EXPECT_EQ(out.gis->hasz, in.gis->hasz);
    ASSERT_EQ(1, in.gis->nlayer);
    EXPECT_STREQ(("export" + std::to_string(type)).c_str(), in.gis->layers[0].name);

    // features come back in one layer with their names, colors, sizes and geometry; a point
    // with an empty name gets a generated one
    ASSERT_EQ(out.gis->nfeature, in.gis->nfeature);
    for (int i = 0; i < out.gis->nfeature; i++)
      out.gis->features[i].layer = 0;
    out.gis->nlayer = 1;
    snprintf(out.gis->layers[0].name, sizeof(mb_path), "%s", in.gis->layers[0].name);
    if (type == MB_GIS_POINT)
      snprintf(out.gis->features[2].name, sizeof(mb_path), "%s", "export3 2");
    ExpectSameFeatures(out.gis, in.gis, out.gis->hasz);
  }
}

TEST_F(MbGisFile, ShapefileLongNameIsTruncated) {
  int error = MB_ERROR_NO_ERROR;
  GisData out(MB_GIS_POINT);
  out.AddLayer("layer");
  // 253 ascii bytes then a 2 byte UTF-8 character that would straddle the 254 byte limit
  std::string name(253, 'a');
  name += "\xc3\xa9zzz";
  const double lon = -122.0, lat = 36.0;
  ASSERT_EQ(MB_SUCCESS, mb_gis_addfeature(verbose, out.gis, 0, 1, 1, -1, name.c_str(), 1, nullptr, nullptr, 1, &lon, &lat, nullptr,
                                          &error));
  const std::string file = Path("longname.shp");
  ASSERT_EQ(MB_SUCCESS, mb_gis_export_shapefile(verbose, file.c_str(), out.gis, &error));
  GisData in(MB_GIS_POINT);
  int nimport = 0, nskip = 0;
  ASSERT_EQ(MB_SUCCESS, mb_gis_import_shapefile(verbose, file.c_str(), nullptr, 0, 1, -1, in.gis, &nimport, &nskip, &error));
  ASSERT_EQ(1, nimport);
  EXPECT_EQ(std::string(253, 'a'), std::string(in.gis->features[0].name));
}

TEST_F(MbGisFile, ShapefileTransformsProjectedCoordinates) {
  int error = MB_ERROR_NO_ERROR;
  // UTM zone 10N: easting 500000 is the central meridian, 123 W
  const std::string file = Path("utm.shp");
  MakeShapefile(file, 32610, wkbLineString, {"LINESTRING (500000 4000000,500000 4100000)"}, {"north"});
  GisData in(MB_GIS_POLYLINE);
  int nimport = 0, nskip = 0;
  ASSERT_EQ(MB_SUCCESS, mb_gis_import_shapefile(verbose, file.c_str(), nullptr, 5, 1, -1, in.gis, &nimport, &nskip, &error));
  ASSERT_EQ(1, nimport);
  const struct mb_gisfeature_struct *feature = &in.gis->features[0];
  ASSERT_EQ(2, feature->npoint);
  EXPECT_NEAR(-123.0, feature->lon[0], 1.0e-9);
  EXPECT_NEAR(-123.0, feature->lon[1], 1.0e-9);
  // 100 km of northing is about 0.9 degrees of latitude
  EXPECT_GT(feature->lat[0], 36.0);
  EXPECT_LT(feature->lat[0], 36.3);
  EXPECT_NEAR(0.9, feature->lat[1] - feature->lat[0], 0.01);
  EXPECT_STREQ("north", feature->name);
  EXPECT_EQ(5, feature->color);
  EXPECT_FALSE(in.gis->hasz);
}

TEST_F(MbGisFile, ShapefileMultiGeometries) {
  int error = MB_ERROR_NO_ERROR;
  const std::string points = Path("multipoint.shp");
  MakeShapefile(points, 4326, wkbMultiPoint, {"MULTIPOINT ((-122 36),(-121 36.5),(-120 37))"}, {"buoys"});
  int type = MB_GIS_NONE;
  ASSERT_EQ(MB_SUCCESS, mb_gis_shapefile_type(verbose, points.c_str(), &type, &error));
  EXPECT_EQ(MB_GIS_POINT, type);
  GisData in(MB_GIS_POINT);
  int nimport = 0, nskip = 0;
  ASSERT_EQ(MB_SUCCESS, mb_gis_import_shapefile(verbose, points.c_str(), nullptr, 3, 1, -1, in.gis, &nimport, &nskip, &error));
  ASSERT_EQ(3, nimport);
  EXPECT_STREQ("buoys 0", in.gis->features[0].name);
  EXPECT_STREQ("buoys 2", in.gis->features[2].name);
  EXPECT_DOUBLE_EQ(-120.0, in.gis->features[2].lon[0]);

  const std::string polygons = Path("multipolygon.shp");
  MakeShapefile(polygons, 4326, wkbMultiPolygon,
                {"MULTIPOLYGON (((-122 36,-121 36,-121 37,-122 37,-122 36),(-121.8 36.2,-121.8 36.8,-121.2 36.8,-121.2 36.2,"
                 "-121.8 36.2)),((-120 36,-119 36,-119 37,-120 36)))"},
                {"islands"});
  GisData in2(MB_GIS_POLYGON);
  ASSERT_EQ(MB_SUCCESS, mb_gis_import_shapefile(verbose, polygons.c_str(), nullptr, 3, 1, -1, in2.gis, &nimport, &nskip, &error));
  ASSERT_EQ(1, nimport);
  const struct mb_gisfeature_struct *feature = &in2.gis->features[0];
  ASSERT_EQ(3, feature->npart);
  EXPECT_EQ(MB_GIS_PART_OUTER, feature->parttype[0]);
  EXPECT_EQ(MB_GIS_PART_HOLE, feature->parttype[1]);
  EXPECT_EQ(MB_GIS_PART_OUTER, feature->parttype[2]);
  EXPECT_EQ(14, feature->npoint);

  // a polygon shapefile has nothing to import as polylines
  GisData in3(MB_GIS_POLYLINE);
  ASSERT_EQ(MB_SUCCESS, mb_gis_import_shapefile(verbose, polygons.c_str(), nullptr, 3, 1, -1, in3.gis, &nimport, &nskip, &error));
  EXPECT_EQ(0, nimport);
  EXPECT_EQ(1, nskip);
  EXPECT_EQ(0, in3.gis->nlayer);
}

TEST_F(MbGisFile, ShapefileImportClipsToBounds) {
  int error = MB_ERROR_NO_ERROR;
  const std::string file = Path("clip.shp");
  MakeShapefile(file, 4326, wkbLineString,
                {"LINESTRING (-122.5 36.5,-122.4 36.6)",    // inside
                 "LINESTRING (-125 36.5,-122.5 36.5)",      // crosses the west edge
                 "LINESTRING (-125 36.5,-124 36.5)",        // west of the bounds
                 "LINESTRING (-122.5 38.5,-122.4 38.6)"},   // north of the bounds
                {"inside", "straddle", "west", "north"});
  const double bounds[4] = {-123.0, -122.0, 36.0, 37.0};
  GisData in(MB_GIS_POLYLINE);
  int nimport = 0, nskip = 0;
  ASSERT_EQ(MB_SUCCESS, mb_gis_import_shapefile(verbose, file.c_str(), bounds, 5, 1, -1, in.gis, &nimport, &nskip, &error));
  EXPECT_EQ(2, nimport);
  EXPECT_EQ(2, nskip);
  ASSERT_EQ(2, in.gis->nfeature);
  EXPECT_STREQ("inside", in.gis->features[0].name);
  EXPECT_STREQ("straddle", in.gis->features[1].name);
  // straddling features are kept whole
  EXPECT_DOUBLE_EQ(-125.0, in.gis->features[1].lon[0]);

  // bounds using 0 to 360 longitude: imported longitudes are shifted to match
  const double bounds360[4] = {237.0, 238.0, 36.0, 37.0};
  GisData in360(MB_GIS_POLYLINE);
  ASSERT_EQ(MB_SUCCESS,
            mb_gis_import_shapefile(verbose, file.c_str(), bounds360, 5, 1, -1, in360.gis, &nimport, &nskip, &error));
  EXPECT_EQ(2, nimport);
  ASSERT_EQ(2, in360.gis->nfeature);
  EXPECT_DOUBLE_EQ(237.5, in360.gis->features[0].lon[0]);
  EXPECT_DOUBLE_EQ(235.0, in360.gis->features[1].lon[0]);
}

TEST_F(MbGisFile, ShapefileWithoutPrjIsGeographic) {
  int error = MB_ERROR_NO_ERROR;
  const std::string file = Path("noprj.shp");
  MakeShapefile(file, 0, wkbPoint25D, {"POINT Z (-122.25 36.75 -1234.5)"}, {"deep"});
  EXPECT_FALSE(std::filesystem::exists(Path("noprj.prj")));
  GisData in(MB_GIS_POINT);
  int nimport = 0, nskip = 0;
  ASSERT_EQ(MB_SUCCESS, mb_gis_import_shapefile(verbose, file.c_str(), nullptr, 3, 1, -1, in.gis, &nimport, &nskip, &error));
  ASSERT_EQ(1, nimport);
  EXPECT_TRUE(in.gis->hasz);
  EXPECT_DOUBLE_EQ(-122.25, in.gis->features[0].lon[0]);
  EXPECT_DOUBLE_EQ(36.75, in.gis->features[0].lat[0]);
  EXPECT_DOUBLE_EQ(-1234.5, in.gis->features[0].z[0]);

  // projected coordinates without a .prj cannot be used
  const std::string file2 = Path("projected_noprj.shp");
  MakeShapefile(file2, 0, wkbPoint, {"POINT (500000 4000000)"}, {"utm"});
  GisData in2(MB_GIS_POINT);
  ASSERT_EQ(MB_SUCCESS, mb_gis_import_shapefile(verbose, file2.c_str(), nullptr, 3, 1, -1, in2.gis, &nimport, &nskip, &error));
  EXPECT_EQ(0, nimport);
  EXPECT_EQ(1, nskip);
}

TEST_F(MbGisFile, ImportMissingFileFails) {
  int error = MB_ERROR_NO_ERROR;
  int type = MB_GIS_NONE;
  EXPECT_EQ(MB_FAILURE, mb_gis_shapefile_type(verbose, Path("missing.shp").c_str(), &type, &error));
  EXPECT_EQ(MB_ERROR_OPEN_FAIL, error);
  GisData in(MB_GIS_POINT);
  int nimport = 0, nskip = 0;
  EXPECT_EQ(MB_FAILURE,
            mb_gis_import_shapefile(verbose, Path("missing.shp").c_str(), nullptr, 3, 1, -1, in.gis, &nimport, &nskip, &error));
  EXPECT_EQ(MB_ERROR_OPEN_FAIL, error);
}

}  // namespace

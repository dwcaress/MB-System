// GeoCon.hpp
//
// C API for GeoCon class
// Wraps GCTP and Proj geo coordinate transformations
// i.e. lat/lon <> mercator projection, e.g. utm).
// See also GeoCon.hpp

#include <memory.h>
#include <string.h>
#ifdef TRN_USE_PROJ
#include <proj.h>
#endif

#ifndef GEOCON_H
#define GEOCON_H

// CRS: UTM10N (Monterey Bay)
#define GEOIF_TCRS_UTM10N "EPSG:32610"
// CRS: UTM9N (Axial Seamount)
#define GEOIF_TCRS_UTM9N "EPSG:32609"

// default source CRS (lon/lat using WGS84)"
#define GEOIF_SCRS_DFL "+proj=lonlat +datum=WGS84"
// default target CRS (UTM zone 10N)
#define GEOIF_TCRS_DFL GEOIF_TCRS_UTM10N

// GeoCon implentation ID type
typedef enum {
    GEO_UNKNOWN,
    GEO_GCTP,
    GEO_PROJ,
    GEO_TYPES
}GeoConType;

// C GeoCon instance type
struct wgeocon_s;
typedef struct wgeocon_s wgeocon_t;

#ifdef __cplusplus
extern "C" {
#endif

// GeoCon C API

// GCTP instance (caller must free using wgeocon_destroy)
wgeocon_t *wgeocon_new_gctp(long int utm);

// PROJ instance (caller must free using wgeocon_destroy)
wgeocon_t *wgeocon_new_proj(const char *crs);
wgeocon_t *wgeocon_inew_proj(void *xfm, bool autodel, const char *tcrs, const char *scrs);
wgeocon_t *wgeocon_xnew_proj(void *xfm, bool autodel);

// release GeoConverter instance
void wgeocon_destroy(wgeocon_t *self);

// get implentation type ID
GeoConType wgeocon_type(wgeocon_t *self);

// get implentation type name
const char *wgeocon_typestr(wgeocon_t *self);
// set debug level
void wgeocon_set_debug(wgeocon_t *self, int debug);
// get debug level
int wgeocon_debug(wgeocon_t *self);

// lat/lon to mercator projection
int wgeocon_geo_to_mp(wgeocon_t *self, double lat_rad, double lon_rad, double *r_northing_m, double *r_easting_m);

// mercator projection to lat/lon
int wgeocon_mp_to_geo(wgeocon_t *self, double northing_m, double easting_m, double *r_lat_rad, double *r_lon_rad);

// get a pointer to a member (optional; keys defined per implementation)
void *wgeocon_get_member(wgeocon_t *self, const char *key);

// set a member (optional; keys defined per implementation)
int wgeocon_set_member(wgeocon_t *self, const char *key, void *value);

// initialize (optional; argments defined per implementation)
void *wgeocon_init(wgeocon_t *self, int argc, void **argv);

// enable/disable deletion of members (optional; keys defined per implementation)
void wgeocon_auto_delete(wgeocon_t *self, const char *key, bool enable);

#ifdef __cplusplus
}
#endif

#endif // GEOCONVERTER_H

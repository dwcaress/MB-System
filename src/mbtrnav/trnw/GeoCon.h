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

#define GEOIF_CRS_DFL "UTM10N"
#define GEOIF_WGS_DFL "EPSG:4326"

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

// release GeoConverter instance
void wgeocon_destroy(wgeocon_t *self);

// lat/lon to mercator projection
int wgeocon_geo_to_mp(wgeocon_t *self, double lat_rad, double lon_rad, double *r_northing_m, double *r_easting_m);

// mercator projection to lat/lon
int wgeocon_mp_to_geo(wgeocon_t *self, double northing_m, double easting_m, double *r_lat_rad, double *r_lon_rad);

// get a pointer to a member (optional; keys defined per implementation)
void *wgeocon_get_member(wgeocon_t *self, const char *key);

// initialize (optional; argments defined per implementation)
void *wgeocon_init(wgeocon_t *self, int argc, void **argv);

// enable/disable deletion of members (optional; keys defined per implementation)
void wgeocon_auto_delete(wgeocon_t *self, const char *key, bool enable);

#ifdef __cplusplus
}
#endif

#endif // GEOCONVERTER_H

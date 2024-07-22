// GeoCon.hpp
//
// Wraps GCTP and Proj geo coordinate transformations
// i.e. lat/lon <> mercator projection, e.g. utm).
// Includes C++ and C APIs (see GeoCon.h).
// trnw mb1_to_pose/meas now takes a wgeocon_t (C geoconverter)
// reference argument instead of utmZone. T
// The underlying implementation  may be GCTP or Proj, and
// may be selected at run time

#include <iostream>
#include <memory.h>
#include <string.h>
#ifdef TRN_USE_PROJ
#include <proj.h>
#endif
#include <NavUtils.h>
#include <MathP.h>
#include "GeoCon.h"

#ifndef GEOCON_HPP
#define GEOCON_HPP

// GeoConIF
// Virtual base class (interface)
// Wraps various implementations for
// geo-coordinate transformation e.g. libproj, gctp
class GeoConIF
{

public:
    GeoConIF();

    virtual ~GeoConIF()
    {}

    // convert lat/lon to mercator projection (e.g. UTM)
    virtual int geo_to_mp(double lat_rad, double lon_rad, double *r_northing, double *r_easting) = 0;
    // convert mercator projection to lat/lon
    virtual int mp_to_geo(double northing_m, double easting_m, double *r_lat_rad, double *r_lon_rad) = 0;

    // get underlying context pointer
    // for implementations that have one.
    // Enables caller to manage context initialization/destruction
    virtual void *get_member(const char *key);

    // if true, caller will manage release of context resources
    virtual void auto_delete(const char *key, bool enable);

    // perform default initialization of underlying ctx
    virtual void *init(int argc, void **argv);

    // implementation type ID
    GeoConType type();

    // type ID name (null terminated string)
    virtual const char *typestr();

protected:

    GeoConType m_type;
};

#ifdef TRN_USE_PROJ
// GeoConProj
// Implementation backed by libproj coordinate transform (PJ*).
// callers may opt to manage (initialize, destroy) the underlying transform,
// or use the class implementation.
//
// get_member keys:
// "XFM" : transform pointer (PJ*)
//
// auto_delete keys:
// "XFM" : transform pointer (PJ*)
//
// init argments:
// argv[0] : source CRS (const char *) optional, default GEOIF_WGS_DFL
// argv[1] : target CRS (const char *) optional, default m_crs

class GeoConProj : public GeoConIF
{
public:
    GeoConProj();

    GeoConProj(const char *crs);

    ~GeoConProj() override;

    int geo_to_mp(double lat_rad, double lon_rad, double *r_northing, double *r_easting) override;

    int mp_to_geo(double northing_m, double easting_m, double *r_lat_rad, double *r_lon_rad) override;

    // "XFM" returns (PJ *) m_proj_xfm
    void *get_member(const char *key) override;

    // argv[0] : const char * : source crs
    // argv[1] : const char * : target crs (optional default: use m_crs)
    virtual void *init(int argc, void **argv) override;

    void auto_delete(const char *key, bool enable) override;

private:
    char *m_crs;
    void *m_proj_xfm;
    bool m_auto_delete_xfm;
};
#endif // TRN_USE_PROJ

// GeoConGCTP
// implementation backed by GCTP library (via NavUtils)
//
// get_member keys: not implemented
//
// auto_delete keys: not implemented
//
// init argments: not implemented

class GeoConGCTP : public GeoConIF
{
public:
    GeoConGCTP();

    GeoConGCTP(long int utm);

    ~GeoConGCTP() override;

    int geo_to_mp(double lat_rad, double lon_rad, double *r_northing, double *r_easting) override;

    int mp_to_geo(double northing_m, double easting_m, double *r_lat_rad, double *r_lon_rad) override;

private:
    long int m_utm;
};

// GeoCon
// Concrete implementation for GeoConIF
// The underlying implementation is determined by the
// constructor used.
class GeoCon : public GeoConIF
{

public:

    GeoCon();

    // implementation backed by GCTP library (via NavUtils)
    GeoCon(long int utm);

    // if libproj is not available, disable proj implementation
    GeoCon(const char *crs);

    ~GeoCon() override;

    // lat/lon to mercator projection
    int geo_to_mp(double lat_rad, double lon_rad, double *r_northing_m, double *r_easting_m) override;

    // mercator projection to lat/lon
    int mp_to_geo(double northing_m, double easting_m, double *r_lat_rad, double *r_lon_rad) override;

    // get a pointer to a member (optional; keys defined per implementation)
    void *get_member(const char *key) override;

    // initialize (optional; argments defined per implementation)
    void *init(int argc, void **argv) override;

    // enable/disable deletion of members (optional; keys defined per implementation)
    void auto_delete(const char *key, bool enable) override;

    // get string representation of underlying type
    const char *typestr() override;

private:
    // underlying implementation
    GeoConIF *m_geocon;
};

#endif //


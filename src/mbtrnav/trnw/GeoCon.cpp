// GeoCon.cpp
//
// GeoCon geo coordinate transformation implentation

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "GeoCon.h"
#include "GeoCon.hpp"

#ifdef __cplusplus
extern "C" {
#endif

GeoConIF::GeoConIF()
: m_type(GEO_UNKNOWN)
{}

 void *GeoConIF::get_member(const char *key)
{
    std::cerr << __func__ << " not implemented for type " << typestr() << std::endl;
    return nullptr;
}

 void GeoConIF::auto_delete(const char *key, bool enable)
{
    std::cerr << __func__ << " not implemented for type " << typestr() << std::endl;
}

 void *GeoConIF::init(int argc, void **argv)
{
    std::cerr << __func__ << " not implemented for type " << typestr() << std::endl;
    return nullptr;
}

GeoConType GeoConIF::type()
{
    return m_type;
}

 const char *GeoConIF::typestr()
{
    static const char *geotype_str[GEO_TYPES] =
    {
        "UNKNOWN","GCTP","PROJ"
    };

   if(m_type == GEO_GCTP)
        return geotype_str[1];
    if(m_type == GEO_PROJ)
        return geotype_str[2];

    return geotype_str[0];
}

#ifdef TRN_USE_PROJ

GeoConProj::GeoConProj()
: m_crs(nullptr)
, m_proj_xfm(nullptr)
, m_auto_delete_xfm(true)
{
    m_type = GEO_PROJ;
}

GeoConProj::GeoConProj(const char *crs)
: m_proj_xfm(nullptr)
, m_auto_delete_xfm(true)
{
    m_crs = (crs == NULL ? NULL : strdup(crs));
    m_type = GEO_PROJ;
}

GeoConProj::~GeoConProj()
{
    free(m_crs);
    if(m_auto_delete_xfm && m_proj_xfm != nullptr) {
        proj_destroy((PJ *)m_proj_xfm);
    }
}

int GeoConProj::geo_to_mp(double lat_rad, double lon_rad, double *r_northing, double *r_easting)
{
    if(r_northing == NULL || r_easting == NULL) {
        std::cerr << typestr() << "::" << __func__ << " ERR invalid argument (NULL)" << std::endl;
        return -1;
    }

    PJ_COORD c;
    c.v[0] = lon_rad;
    c.v[1] = lat_rad;
    c = proj_trans((PJ *)m_proj_xfm, PJ_FWD, c);

    // set output
    *r_easting = c.v[1];
    *r_northing = c.v[0];

    std::cerr << typestr() << "::" << __func__ << " crs:" << m_crs << " proj_xfm:" << m_proj_xfm << std::endl;
    return 0;
}

int GeoConProj::mp_to_geo(double northing_m, double easting_m, double *r_lat_rad, double *r_lon_rad)
{
    std::cerr << __func__ << ":" << __LINE__ << " - PROJ not implemented crs: " << m_crs << " proj_xfm:" << m_proj_xfm << std::endl;

    if(r_lat_rad == NULL || r_lon_rad == NULL) {
        std::cerr << typestr() << "::" << __func__ << " ERR invalid argument (NULL)" << std::endl;
        return -1;
    }

    PJ_COORD c;
    c.v[0] = northing_m;
    c.v[1] = northing_m;
    c = proj_trans((PJ *)m_proj_xfm, PJ_INV, c);

    // set output
    *r_lat_rad = c.v[1];
    *r_lon_rad = c.v[0];

    std::cerr << typestr() << "::" << __func__ << " crs:" << m_crs << " proj_xfm:" << m_proj_xfm << std::endl;

    return 0;
}

// key "XFM" returns (PJ *) m_proj_xfm
void *GeoConProj::get_member(const char *key)
{
    if(strcasecmp(key, "XFM") == 0)
        return m_proj_xfm;
    return nullptr;
}

// default transform initialization
// argv[0] : const char * : source crs
// argv[1] : const char * : target crs (optional default: use m_crs)
void *GeoConProj::init(int argc, void **argv)
{
    const char *source_crs = GEOIF_WGS_DFL;
    const char *target_crs = (m_crs != NULL ? m_crs : GEOIF_CRS_DFL);

    for(int i = 0; i < argc; i++)
    {
        if(i == 0) {
            source_crs = (const char *)argv[i];
        } else if(i == 1) {
            target_crs = (const char *)argv[i];
        } else {
            break;
        }
    }

    PJ *p = proj_create_crs_to_crs(PJ_DEFAULT_CTX, source_crs, target_crs, 0);

    if(p != NULL){
        void *pjptr = (void *) proj_normalize_for_visualization(PJ_DEFAULT_CTX, p);
        m_proj_xfm = pjptr;
        fprintf(stderr, "%s:%d - m_proj_xfm %p\n", __func__, __LINE__, m_proj_xfm);
    } else {
        fprintf(stderr, "%s:%d - ERR proj_create_crs_to_crs failed\n", __func__, __LINE__);
    }
    return m_proj_xfm;
}

// key "XFM" sets auto delete for m_proj_xfm
void GeoConProj::auto_delete(const char *key, bool enable)
{
    if(strcasecmp(key, "XFM") == 0)
        m_auto_delete_xfm = enable;
}
#endif

GeoConGCTP::GeoConGCTP()
: m_utm(10)
{
    m_type = GEO_GCTP;
}

GeoConGCTP::GeoConGCTP(long int utm)
: m_utm(utm)
{
    m_type = GEO_GCTP;
}

GeoConGCTP::~GeoConGCTP()
{}

int GeoConGCTP::geo_to_mp(double lat_rad, double lon_rad, double *r_northing, double *r_easting)
{
    std::cerr << typestr() << "::" << __func__ << " utm:" << m_utm << std::endl;

    if(r_northing == NULL || r_easting == NULL) {
        std::cerr << typestr() << "::" << __func__ << " ERR invalid argument (NULL)" << std::endl;
        return -1;
    }

    return NavUtils::geoToUtm(lat_rad, lon_rad, m_utm, r_northing, r_easting);
}

int GeoConGCTP::mp_to_geo(double northing_m, double easting_m, double *r_lat_rad, double *r_lon_rad)
{
    std::cerr << typestr() << "::" << __func__ << " utm:" << m_utm << std::endl;

    if(r_lat_rad == NULL || r_lon_rad == NULL)
        return -1;

    return NavUtils::utmToGeo(northing_m, easting_m, m_utm, r_lat_rad, r_lon_rad);
}

GeoCon::GeoCon()
: m_geocon(nullptr)
{}

// implementation backed by GCTP library (via NavUtils)
GeoCon::GeoCon(long int utm)
{
    // create a GCTP instance
    m_geocon = new GeoConGCTP(utm);
}

#ifdef TRN_USE_PROJ
// implementation backed by libproj coordinate transform (PJ *)
GeoCon::GeoCon(const char *crs)
{
    // create a PROJ instance
    m_geocon = new GeoConProj(crs);
}
#else
// if libproj is not available, disable proj implementation
GeoCon::GeoCon(const char *crs)
: m_geocon(nullptr)
{
    std::cerr << __func__ << ": ERR proj not supported; build using -DTRN_USE_PROJ" << std::endl;
}
#endif

GeoCon::~GeoCon()
{
    if(m_geocon != nullptr)
        delete m_geocon;
}

// lat/lon to mercator projection
int GeoCon::geo_to_mp(double lat_rad, double lon_rad, double *r_northing_m, double *r_easting_m)
{
    if(m_geocon == NULL) {
        std::cerr << __func__ << ": ERR NULL instance" << std::endl;
        return -1;
    }

    return m_geocon->geo_to_mp(lat_rad, lon_rad, r_northing_m, r_easting_m);
}

// mercator projection to lat/lon
int GeoCon::mp_to_geo(double northing_m, double easting_m, double *r_lat_rad, double *r_lon_rad)
{
    if(m_geocon == NULL) {
        std::cerr << __func__ << ": ERR NULL instance" << std::endl;
        return -1;
    }

    return m_geocon->mp_to_geo(northing_m, easting_m, r_lat_rad, r_lon_rad);
}

// get a pointer to a member (optional; keys defined per implementation)
void *GeoCon::get_member(const char *key)
{
    if(m_geocon == NULL) {
        std::cerr << __func__ << ": ERR NULL instance" << std::endl;
        return nullptr;
    }
    return m_geocon->get_member(key);
}

// initialize (optional; argments defined per implementation)
void *GeoCon::init(int argc, void **argv)
{
    if(m_geocon == NULL) {
        std::cerr << __func__ << ": ERR NULL instance" << std::endl;
        return nullptr;
    }
    return m_geocon->init(argc, argv);
}

// enable/disable deletion of members (optional; keys defined per implementation)
void GeoCon::auto_delete(const char *key, bool enable)
{
    if(m_geocon == NULL) {
        std::cerr << __func__ << ": ERR NULL instance" << std::endl;
        return;
    }

    m_geocon->auto_delete(key, enable);
}

// get string representation of underlying type
const char *GeoCon::typestr()
{
    if(m_geocon == NULL) {
        return "UNKNOWN";
    }
    return m_geocon->typestr();
}


struct wgeocon_s {
    void *obj;
};

wgeocon_t *wgeocon_new_gctp(long int utm)
{
    wgeocon_t *m = (wgeocon_t *)malloc(sizeof(*m));
    if(NULL!=m){
        memset(m,0,sizeof(*m));
        GeoCon *obj = new GeoCon(utm);
        m->obj = obj;
    }
    return m;
}

wgeocon_t *wgeocon_new_proj(const char *crs)
{
    wgeocon_t *m = (wgeocon_t *)malloc(sizeof(*m));
    if(NULL!=m){
        memset(m,0,sizeof(*m));
        GeoCon *obj = new GeoCon(crs);
        m->obj = obj;
    }
    return m;
}

void wgeocon_destroy(wgeocon_t *self)
{
    if (NULL!=self){
        delete static_cast<GeoCon *>(self->obj);
        free(self);
    }
}

int wgeocon_geo_to_mp(wgeocon_t *self, double lat_rad, double lon_rad, double *r_northing_m, double *r_easting_m)
{
    if(NULL!=self){
        GeoCon *obj = static_cast<GeoCon *>(self->obj);
        if(obj != NULL) {
            return obj->geo_to_mp(lat_rad, lon_rad, r_northing_m, r_easting_m);
        }
    }
    return -1;
}

int wgeocon_mp_to_geo(wgeocon_t *self, double northing_m, double easting_m, double *r_lat_rad, double *r_lon_rad)
{
    if(NULL!=self){
        GeoCon *obj = static_cast<GeoCon *>(self->obj);
        if(obj != NULL) {
            return obj->mp_to_geo(northing_m, easting_m, r_lat_rad, r_lon_rad);
        }
    }
    return -1;
}

void *wgeocon_get_member(wgeocon_t *self, const char *key)
{
    if(NULL!=self){
        GeoCon *obj = static_cast<GeoCon *>(self->obj);
        if(obj != NULL) {
            return obj->get_member(key);
        }
    }
    return NULL;
}

void wgeocon_auto_delete(wgeocon_t *self, const char *key, bool enable)
{
    if(NULL!=self){
        GeoCon *obj = static_cast<GeoCon *>(self->obj);
        if(obj != NULL) {
            obj->auto_delete(key, enable);
        }
    }
}

void *wgeocon_init(wgeocon_t *self, int argc, void **argv)
{
    if(NULL!=self){
        GeoCon *obj = static_cast<GeoCon *>(self->obj);
        if(obj != NULL) {
            return obj->init(argc, argv);
        }
    }
    return NULL;
}
#ifdef __cplusplus
}
#endif

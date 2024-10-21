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

int GeoConIF::set_member(const char *key, void *value)
{
   std::cerr << __func__ << " not implemented for type " << typestr() << std::endl;
   return -1;
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

void GeoConIF::set_debug(int level)
{
    m_debug = level;
}
int GeoConIF::debug()
{
    return m_debug;
}

#ifdef TRN_USE_PROJ

GeoConProj::GeoConProj()
: m_tcrs(nullptr)
, m_proj_xfm(nullptr)
, m_auto_delete_xfm(true)
{
    m_scrs = strdup(GEOIF_SCRS_DFL);
    m_type = GEO_PROJ;
}

GeoConProj::GeoConProj(const char *tcrs)
: m_proj_xfm(nullptr)
, m_auto_delete_xfm(true)
{
    m_scrs = strdup(GEOIF_SCRS_DFL);
    m_tcrs = (tcrs == NULL ? NULL : strdup(tcrs));
    m_type = GEO_PROJ;
}

GeoConProj::GeoConProj(void *xfm, bool autodel)
: m_proj_xfm(xfm)
, m_tcrs(NULL)
, m_auto_delete_xfm(autodel)
{
    m_scrs = strdup(GEOIF_SCRS_DFL);
    m_type = GEO_PROJ;
}

GeoConProj::GeoConProj(void *xfm, bool autodel, const char *tcrs, const char *scrs)
: m_proj_xfm(xfm)
, m_auto_delete_xfm(autodel)
{
    m_scrs = scrs != NULL ? strdup(scrs) : strdup(GEOIF_SCRS_DFL);
    m_tcrs = (tcrs == NULL ? NULL : strdup(tcrs));
    m_type = GEO_PROJ;
}

GeoConProj::~GeoConProj()
{
    free(m_tcrs);
    free(m_scrs);

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

    PJ_COORD cin = proj_coord(Math::radToDeg(lon_rad), Math::radToDeg(lat_rad), 0, 0);
    PJ_COORD cout = proj_trans((PJ *)m_proj_xfm, PJ_FWD, cin);

    // set output
    *r_easting = cout.v[0];
    *r_northing = cout.v[1];

    if(debug() != 0)
    std::cerr << typestr() << "::" << __func__ << " E,N: " << *r_easting << ", " << *r_northing << std::endl;
    return 0;
}

int GeoConProj::mp_to_geo(double northing_m, double easting_m, double *r_lat_rad, double *r_lon_rad)
{
    if(r_lat_rad == NULL || r_lon_rad == NULL) {
        std::cerr << typestr() << "::" << __func__ << " ERR invalid argument (NULL)" << std::endl;
        return -1;
    }

    PJ_COORD cin = proj_coord(easting_m, northing_m, 0, 0);
    PJ_COORD cout = proj_trans((PJ *)m_proj_xfm, PJ_INV, cin);

    // set output
    *r_lon_rad = Math::degToRad(cout.v[0]);
    *r_lat_rad = Math::degToRad(cout.v[1]);

    if(debug() != 0)
    std::cerr << typestr() << "::" << __func__ << " lat,lon: " << Math::radToDeg(*r_lat_rad) << ", " << Math::radToDeg(*r_lon_rad) << std::endl;

    return 0;
}

// key "XFM" returns (PJ *) m_proj_xfm
void *GeoConProj::get_member(const char *key)
{
    if(strcasecmp(key, "XFM") == 0)
        return m_proj_xfm;
    if(strcasecmp(key, "SCRS") == 0)
        return m_scrs;
    if(strcasecmp(key, "TCRS") == 0)
        return m_tcrs;
    return nullptr;
}

int GeoConProj::set_member(const char *key, void *value)
{
    if(strcasecmp(key, "XFM") == 0) {
        m_proj_xfm = value;
        return 0;
    } else if(strcasecmp(key, "SCRS") == 0) {
        free(m_scrs);
        m_scrs = strdup((const char *)value);
        return 0;
    }
    if(strcasecmp(key, "TCRS") == 0) {
        free(m_tcrs);
        m_tcrs = strdup((const char *)value);
        return 0;
    }
    return -1;
}

// default transform initialization
// argv[0] : const char * : target crs
// argv[1] : const char * : source crs
void *GeoConProj::init(int argc, void **argv)
{
    const char *source_crs = (m_scrs != NULL ? m_scrs : GEOIF_SCRS_DFL);
    const char *target_crs = (m_tcrs != NULL ? m_tcrs : GEOIF_TCRS_DFL);

    if(argc > 0 && argv != nullptr) {
        for(int i = 0; i < argc; i++)
        {
            if(i == 0 && argv[1] != nullptr) {
                target_crs = (const char *)argv[i];

                if(m_tcrs != nullptr)
                    free(m_tcrs);
                m_tcrs = strdup(target_crs);

            } else if(i == 1 && argv[i] != nullptr) {
                source_crs = (const char *)argv[i];

                if(m_scrs != nullptr)
                    free(m_scrs);
                m_scrs = strdup(source_crs);
            } else {
                break;
            }
        }
    }

    if(debug() != 0)
        std::cerr << typestr() << "::" << __func__ << " scrs: " << source_crs << " tcrs: " <<  target_crs << std::endl;

    PJ *p = proj_create_crs_to_crs(PJ_DEFAULT_CTX, source_crs, target_crs, 0);

    if(p != NULL){
        m_proj_xfm = (void *) proj_normalize_for_visualization(PJ_DEFAULT_CTX, p);

        if(debug() != 0)
            std::cerr << typestr() << "::" << __func__ << " m_proj_xfm: " << (void *)m_proj_xfm << std::endl;

    } else {
        fprintf(stderr, "%s:%d - ERR proj_create_crs_to_crs failed src: %s tgt: %s\n", __func__, __LINE__, source_crs, target_crs);
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
    if(debug() != 0)
    std::cerr << typestr() << "::" << __func__ << " utm:" << m_utm << std::endl;

    if(r_northing == NULL || r_easting == NULL) {
        std::cerr << typestr() << "::" << __func__ << " ERR invalid argument (NULL)" << std::endl;
        return -1;
    }

    int retval = NavUtils::geoToUtm(lat_rad, lon_rad, m_utm, r_northing, r_easting);

    if(debug() != 0)
    std::cerr << typestr() << "::" << __func__ << " ret:" << retval << " E,N: " << *r_easting << ", " << *r_northing << std::endl;

    return retval;
}

int GeoConGCTP::mp_to_geo(double northing_m, double easting_m, double *r_lat_rad, double *r_lon_rad)
{
    if(debug() != 0)
    std::cerr << typestr() << "::" << __func__ << " utm:" << m_utm << std::endl;

    if(r_lat_rad == NULL || r_lon_rad == NULL)
        return -1;

    int retval = NavUtils::utmToGeo(northing_m, easting_m, m_utm, r_lat_rad, r_lon_rad);

    if(debug() != 0)
    std::cerr << typestr() << "::" << __func__ << " ret: " << retval << " lat,lon: " << Math::radToDeg(*r_lat_rad) << ", " << Math::radToDeg(*r_lon_rad) << std::endl;

    return retval;
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
GeoCon::GeoCon(const char *tcrs)
{
    // create a PROJ instance
    m_geocon = new GeoConProj(tcrs);
}

GeoCon::GeoCon(void *xfm, bool autodel, const char *tcrs, const char *scrs)
{
    // create a PROJ instance
    m_geocon = new GeoConProj(xfm, autodel, tcrs, scrs);
}

GeoCon::GeoCon(void *xfm, bool autodel)
{
    // create a PROJ instance
    m_geocon = new GeoConProj(xfm, autodel);
}
#else
// if libproj is not available, disable proj implementation
GeoCon::GeoCon(const char *tcrs)
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

int GeoCon::set_member(const char *key, void *value)
{
    if(m_geocon == NULL) {
        std::cerr << __func__ << ": ERR NULL instance" << std::endl;
        return -1;
    }
    return m_geocon->set_member(key, value);
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

void GeoCon::set_debug(int level)
{
    if(m_geocon == NULL) {
        return;
    }
    m_geocon->set_debug(level);
}

int GeoCon::debug()
{
    if(m_geocon == NULL) {
        return -1;
    }
    return m_geocon->debug();
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

wgeocon_t *wgeocon_inew_proj(void *xfm, bool autodel, const char *tcrs, const char *scrs)
{
    wgeocon_t *m = (wgeocon_t *)malloc(sizeof(*m));
    if(NULL!=m){
        memset(m,0,sizeof(*m));
        GeoCon *obj = new GeoCon(xfm, autodel, tcrs, scrs);
        m->obj = obj;
    }
    return m;
}

wgeocon_t *wgeocon_xnew_proj(void *xfm, bool autodel)
{
    wgeocon_t *m = (wgeocon_t *)malloc(sizeof(*m));
    if(NULL!=m){
        memset(m,0,sizeof(*m));
        GeoCon *obj = new GeoCon(xfm, autodel);
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

GeoConType wgeocon_type(wgeocon_t *self)
{
    if(NULL!=self){
        GeoCon *obj = static_cast<GeoCon *>(self->obj);
        if(obj != NULL) {
            return obj->type();
        }
    }
    return GEO_UNKNOWN;
}

const char *wgeocon_typestr(wgeocon_t *self)
{
    if(NULL!=self){
        GeoCon *obj = static_cast<GeoCon *>(self->obj);
        if(obj != NULL) {
            return obj->typestr();
        }
    }
    return NULL;
}

void wgeocon_set_debug(wgeocon_t *self, int level)
{
    if(NULL!=self){
        GeoCon *obj = static_cast<GeoCon *>(self->obj);
        if(obj != NULL) {
            return obj->set_debug(level);
        }
    }
}
int wgeocon_debug(wgeocon_t *self)
{
    if(NULL!=self){
        GeoCon *obj = static_cast<GeoCon *>(self->obj);
        if(obj != NULL) {
            return obj->debug();
        }
    }
    return -1;
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

int wgeocon_set_member(wgeocon_t *self, const char *key, void *value)
{
    if(NULL!=self){
        GeoCon *obj = static_cast<GeoCon *>(self->obj);
        if(obj != NULL) {
            return obj->set_member(key, value);
        }
    }
    return -1;
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

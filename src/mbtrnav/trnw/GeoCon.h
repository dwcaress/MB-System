
#ifdef __cplusplus
#include <iostream>
#include "NavUtils.h"
#include "MathP.h"
#ifdef TRN_USE_PROJ
#include <proj.h>
#endif
#endif
#include <memory.h>
#include <string.h>

#ifndef GEOCONVERTER_H
#define GEOCONVERTER_H

#define GEOIF_CRS_DFL "UTM10N"
#define GEOIF_WGS_DFL "EPSG:4326"


typedef enum {
    GEO_UNKNOWN,
    GEO_GCTP,
    GEO_PROJ,
    GEO_TYPES
}GeoConType;

struct wgeocon_s;
typedef struct wgeocon_s wgeocon_t;

#ifdef __cplusplus

// factory approach

class GeoConIF
{

public:
    GeoConIF()
    : m_type(GEO_UNKNOWN)
    {}

    virtual ~GeoConIF()
    {}

    virtual int geo_to_mp(double lat_rad, double lon_rad, double *r_northing, double *r_easting) = 0;
    virtual int mp_to_geo(double northing_m, double easting_m, double *r_lat_rad, double *r_lon_rad) = 0;

    // get underlying context pointer
    // for implementations that have one.
    // Enables caller to manage context initialization/destruction
    virtual void *get_member(const char *key)
    {
        std::cerr << __func__ << " not implemented for type " << typestr() << std::endl;
        return nullptr;
    }

    // if true, caller will manage release of context resources
    virtual void auto_delete(const char *key, bool enable)
    {
        std::cerr << __func__ << " not implemented for type " << typestr() << std::endl;
    }

    // perform default initialization of underlying ctx
    virtual void *init(int argc, void **argv)
    {
        std::cerr << __func__ << " not implemented for type " << typestr() << std::endl;
        return nullptr;
    }

    GeoConType type()
    {
        return m_type;
    }

    virtual const char *typestr()
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

protected:

    GeoConType m_type;
};

#ifdef TRN_USE_PROJ
class GeoConProj : public GeoConIF
{
public:
    GeoConProj()
    : m_crs(nullptr)
    , m_proj_xfm(nullptr)
    , m_auto_delete_xfm(true)
    {
        m_type = GEO_PROJ;
    }

    GeoConProj(const char *crs)
    : m_proj_xfm(nullptr)
    , m_auto_delete_xfm(true)
    {
        m_crs = (crs == NULL ? NULL : strdup(crs));
        m_type = GEO_PROJ;
    }

    ~GeoConProj() override
    {
        free(m_crs);
        if(m_auto_delete_xfm && m_proj_xfm != nullptr) {
            proj_destroy((PJ *)m_proj_xfm);
        }
    }

    int geo_to_mp(double lat_rad, double lon_rad, double *r_northing, double *r_easting) override
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

    int mp_to_geo(double northing_m, double easting_m, double *r_lat_rad, double *r_lon_rad) override
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

    // "XFM" returns (PJ *) m_proj_xfm
    void *get_member(const char *key) override
    {
        if(strcasecmp(key, "XFM") == 0)
            return m_proj_xfm;
        return nullptr;
    }

    // argv[0] : const char * : source crs
    // argv[1] : const char * : target crs (optional default: use m_crs)
    virtual void *init(int argc, void **argv) override
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

    void auto_delete(const char *key, bool enable) override
    {
        if(strcasecmp(key, "XFM") == 0)
            m_auto_delete_xfm = enable;
    }

private:
    char *m_crs;
    void *m_proj_xfm;
    bool m_auto_delete_xfm;
};
#endif

class GeoConGCTP : public GeoConIF
{
public:
    GeoConGCTP()
    : m_utm(10)
    {
        m_type = GEO_GCTP;
    }

    GeoConGCTP(long int utm)
    : m_utm(utm)
    {
        m_type = GEO_GCTP;
    }

    ~GeoConGCTP() override
    {}

    int geo_to_mp(double lat_rad, double lon_rad, double *r_northing, double *r_easting) override
    {
        std::cerr << typestr() << "::" << __func__ << " utm:" << m_utm << std::endl; 
        
        if(r_northing == NULL || r_easting == NULL) {
            std::cerr << typestr() << "::" << __func__ << " ERR invalid argument (NULL)" << std::endl;
            return -1;
        }

        return NavUtils::geoToUtm(lat_rad, lon_rad, m_utm, r_northing, r_easting);
    }

    int mp_to_geo(double northing_m, double easting_m, double *r_lat_rad, double *r_lon_rad) override
    {
        std::cerr << typestr() << "::" << __func__ << " utm:" << m_utm << std::endl;
        
        if(r_lat_rad == NULL || r_lon_rad == NULL)
            return -1;

        return NavUtils::utmToGeo(northing_m, easting_m, m_utm, r_lat_rad, r_lon_rad);
    }

private:
    long int m_utm;
};

class GeoCon : public GeoConIF
{

public:

    GeoCon()
    : m_geocon(nullptr)
    {}

    GeoCon(long int utm)
    {
        // create a GCTP instance
        m_geocon = new GeoConGCTP(utm);
    }

#ifdef TRN_USE_PROJ
    GeoCon(const char *crs)
    {
        // create a PROJ instance
        m_geocon = new GeoConProj(crs);
    }
#else
    GeoCon(const char *crs)
    : m_geocon(nullptr)
    {
        std::cerr << __func__ << ": ERR proj not supported; build using -DTRN_USE_PROJ" << std::endl;
    }
#endif

    ~GeoCon() override
    {
        if(m_geocon != nullptr)
            delete m_geocon;
    }

    int geo_to_mp(double lat_rad, double lon_rad, double *r_northing_m, double *r_easting_m) override
    {
        if(m_geocon == NULL) {
            std::cerr << __func__ << ": ERR NULL instance" << std::endl;
            return -1;
        }

        return m_geocon->geo_to_mp(lat_rad, lon_rad, r_northing_m, r_easting_m);
    }

    int mp_to_geo(double northing_m, double easting_m, double *r_lat_rad, double *r_lon_rad) override
    {
        if(m_geocon == NULL) {
            std::cerr << __func__ << ": ERR NULL instance" << std::endl;
            return -1;
        }

        return m_geocon->mp_to_geo(northing_m, easting_m, r_lat_rad, r_lon_rad);
    }

    void *get_member(const char *key) override
    {
        if(m_geocon == NULL) {
            std::cerr << __func__ << ": ERR NULL instance" << std::endl;
            return nullptr;
        }
        return m_geocon->get_member(key);
    }

    void *init(int argc, void **argv) override
    {
        if(m_geocon == NULL) {
            std::cerr << __func__ << ": ERR NULL instance" << std::endl;
            return nullptr;
        }
        return m_geocon->init(argc, argv);
    }

    void auto_delete(const char *key, bool enable) override
    {
        if(m_geocon == NULL) {
            std::cerr << __func__ << ": ERR NULL instance" << std::endl;
            return;
        }

        m_geocon->auto_delete(key, enable);
    }

    const char *typestr() override
    {
        if(m_geocon == NULL) {
            return "UNKNOWN";
        }
        return m_geocon->typestr();
    }

private:
    GeoConIF *m_geocon;
};
#endif

#ifdef __cplusplus
extern "C" {
#endif

// GeoConverter C API

// GCTP instance (caller must free using wgeocon_destroy)
wgeocon_t *wgeocon_new_gctp(long int utm);

// PROJ instance (caller must free using wgeocon_destroy)
wgeocon_t *wgeocon_new_proj(const char *crs);

// release GeoConverter instance
void wgeocon_destroy(wgeocon_t *self);

// lat/lon to UTM
int wgeocon_geo_to_mp(wgeocon_t *self, double lat_rad, double lon_rad, double *r_northing_m, double *r_easting_m);
// UTM to lat/lon
int wgeocon_mp_to_geo(wgeocon_t *self, double northing_m, double easting_m, double *r_lat_rad, double *r_lon_rad);

void *wgeocon_get_member(wgeocon_t *self, const char *key);
void wgeocon_auto_delete(wgeocon_t *self, const char *key, bool enable);
void *wgeocon_init(wgeocon_t *self, int argc, void **argv);

#ifdef __cplusplus
}
#endif

#endif // GEOCONVERTER_H


#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "GeoCon.h"

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

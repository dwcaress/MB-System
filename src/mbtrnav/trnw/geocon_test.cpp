
// geocon_test
// Test and demonstrate GeoCon C++ and C APIs
// build using
// w/ proj
// g++ -g -O0 -std=c++11  -I../qnx-utils -I/opt/local/lib/proj9/include -DTRN_USE_PROJ -o geocon-test geocon_test.cpp GeoCon.cpp -L/opt/local/lib/proj9/lib -lproj -L../bin -lgeolib -lqnx
// w/o proj
// g++ -g -O0 -std=c++11  -I../qnx-utils -o geocon-test geocon_test.cpp GeoCon.cpp  -L../bin -lgeolib -lqnx

#include <cstdlib>
#include <memory.h>
#include <errno.h>
#include "GeoCon.hpp"
#include "GeoCon.h"

void use_cpp(GeoCon *gcon);
void use_c(wgeocon_t *gcon);

void use_cpp(GeoCon *gcon)
{
    double lat_d=36.8044, lon_d=-121.7869;
    double lat_r=Math::degToRad(lat_d), lon_r=Math::degToRad(lon_d);
    double nor=0, eas=0;

    if(gcon != nullptr) {
        std::cerr << __func__ << ": using type " << gcon->typestr() << std::endl;

        gcon->geo_to_mp(lat_r, lon_r, &nor, &eas);
        std::cerr << __func__ << ": geo to mp" << std::endl;
        std::cerr << __func__ << ": lat/lon " << lat_d << "/" << lon_d << std::endl;
        std::cerr << __func__ << ": nor/eas " << nor << "/" << eas << std::endl;

        lat_r = 0;
        lon_r = 0;

        gcon->mp_to_geo(nor, eas, &lat_r, &lon_r);
        lat_d = Math::radToDeg(lat_r);
        lon_d = Math::radToDeg(lon_r);
        std::cerr << __func__ << ": mp to geo" << std::endl;
        std::cerr << __func__ << ": lat/lon " << lat_d << "/" << lon_d << std::endl;
        std::cerr << __func__ << ": nor/eas " << nor << "/" << eas << std::endl;
    } else {
        std::cerr << __func__ << ": GeoCon is NULL" << std::endl;
    }
}

void use_c(wgeocon_t *gcon)
{
    double lat_d=36.8044, lon_d=-121.7869;
    double lat_r=Math::degToRad(lat_d), lon_r=Math::degToRad(lon_d);
    double nor=0, eas=0;

    if(gcon != nullptr) {
        std::cerr << __func__ << ": using type " << (int)wgeocon_type(gcon) << "/" << wgeocon_typestr(gcon) << std::endl;

        wgeocon_geo_to_mp(gcon, lat_r, lon_r, &nor, &eas);
        std::cerr << __func__ << ": geo to mp" << std::endl;
        std::cerr << __func__ << ": lat/lon " << lat_d << "/" << lon_d << std::endl;
        std::cerr << __func__ << ": nor/eas " << nor << "/" << eas << std::endl;

        lat_r = 0;
        lon_r = 0;

        wgeocon_mp_to_geo(gcon, nor, eas, &lat_r, &lon_r);
        lat_d = Math::radToDeg(lat_r);
        lon_d = Math::radToDeg(lon_r);
        std::cerr << __func__ << ": mp to geo" << std::endl;
        std::cerr << __func__ << ": lat/lon " << lat_d << "/" << lon_d << std::endl;
        std::cerr << __func__ << ": nor/eas " << nor << "/" << eas << std::endl;
    } else {
        std::cerr << __func__ << ": GeoCon is NULL" << std::endl;
    }
}

int main(int argc, char **argv)
{
    double lat=0, lon=0;
    double nor=0, eas=0;

    // test C++ API

    std::cerr << "# gctp_i" << std::endl;
    GeoCon *gctp_i = new GeoCon(10);
    gctp_i->geo_to_mp(lat, lon, &nor, &eas);

    use_cpp(gctp_i);

    std::cerr << std::endl;

    std::cerr << "# proj_i" << std::endl;
    GeoCon *proj_i = new GeoCon(GEOIF_WGS_DFL);
    proj_i->geo_to_mp(lat, lon, &nor, &eas);

    use_cpp(proj_i);

    if(gctp_i != NULL)
        delete gctp_i;

    if(proj_i != NULL)
        delete proj_i;

    // test C API
    wgeocon_t *gctp_c = wgeocon_new_gctp(10);
    wgeocon_t *proj_c = wgeocon_new_proj(GEOIF_WGS_DFL);

    std::cerr << std::endl;
    use_c(gctp_c);

    std::cerr << std::endl;
    use_c(proj_c);

    std::cerr << std::endl;

    wgeocon_destroy(gctp_c);
    wgeocon_destroy(proj_c);

    return 0;
}

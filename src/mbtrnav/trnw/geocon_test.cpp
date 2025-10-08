
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

static const char *SOURCE_CRS_DFL = GEOIF_SCRS_DFL;
static const char *TARGET_CRS_DFL = GEOIF_TCRS_DFL;
static double LAT_DFL = 0.; //84.
static double LON_DFL = -126.;//-120.
static int DEBUG_DFL = 0;

void use_cpp(GeoCon *gcon, double lat_d=LAT_DFL, double lon_d=LON_DFL);
void use_c(wgeocon_t *gcon, double lat_d=LAT_DFL, double lon_d=LON_DFL);

void use_cpp(GeoCon *gcon, double lat_d, double lon_d)
{
//    double lat_d=TEST_LAT, lon_d=TEST_LON;
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

void use_c(wgeocon_t *gcon, double lat_d, double lon_d)
{
//    double lat_d=TEST_LAT, lon_d=TEST_LON;
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
    void **av = (void **)malloc(2 * sizeof(void *));
    av[0] = (void *)TARGET_CRS_DFL;
    av[1] = (void *)SOURCE_CRS_DFL;

    int debug = DEBUG_DFL;
    double lat_d = LAT_DFL;
    double lon_d = LON_DFL;

    for(int i=0; i < argc; i++) {
        if(sscanf(argv[i], "--debug=%d", &debug) == 1) {
        } else if(sscanf(argv[i], "--lat=%lf", &lat_d) == 1) {
        } else if(sscanf(argv[i], "--lon=%lf", &lon_d) == 1) {
        }
    }

    // test C++ API

    std::cerr << "# gctp_i" << std::endl;
    GeoCon *gctp_i = new GeoCon(10);
    gctp_i->set_debug(debug);

    use_cpp(gctp_i, lat_d, lon_d);

    std::cerr << std::endl;

    std::cerr << "# proj_i" << std::endl;
    GeoCon *proj_i = new GeoCon(GEOIF_TCRS_DFL);
    proj_i->set_debug(debug);
    proj_i->init(2, av);

    use_cpp(proj_i, lat_d, lon_d);

    if(gctp_i != NULL)
        delete gctp_i;

    if(proj_i != NULL)
        delete proj_i;

    // test C API

    wgeocon_t *gctp_c = wgeocon_new_gctp(10);
    wgeocon_set_debug(gctp_c, debug);

    wgeocon_t *proj_c = wgeocon_new_proj(GEOIF_TCRS_DFL);
    wgeocon_set_debug(proj_c, debug);
    wgeocon_init(proj_c, 2, av);

    std::cerr << std::endl;
    use_c(gctp_c, lat_d, lon_d);

    std::cerr << std::endl;
    use_c(proj_c, lat_d, lon_d);

    std::cerr << std::endl;

    wgeocon_delete(gctp_c);
    wgeocon_delete(proj_c);

    free(av);
    return 0;
}

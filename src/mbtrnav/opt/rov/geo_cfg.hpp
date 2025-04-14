
/// @file geo_cfg.cpp
/// @authors k. headley
/// @date 08nov2022

/// Summary: geometry configuration classes

// ///////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute
// Distributed under MIT license. See LICENSE file for more information.

// /////////////////
// Includes
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <math.h>
#include <string.h>
#include <memory.h>
#include <libgen.h>
#include <map>

#ifndef DTR
#define DTR(x) ((x) * M_PI/180.)
#endif
#ifndef RTD
#define RTD(x) ((x) * 180./M_PI)
#endif

#ifndef GEO_CFG_H
#define GEO_CFG_H

typedef enum{
    GA_RADIANS=0,
    GA_DEGREES
}geo_angle_units_t;

class beam_geometry
{
public:
    beam_geometry()
    : xmap()
    {}
    ~beam_geometry(){}

    static char *trim(char *src)
    {
        char *bp = NULL;
        if(NULL!=src){
            bp = src;
            char *ep = src+strlen(src);
            while(isspace(*bp) && (*bp != '\0')){
                bp++;
            }
            while((isspace(*ep) || *ep == '\0') && (ep >= src)){
                *ep = '\0';
                ep--;
            }
        }
        return bp;
    }

    static void parse_map(const char *map_spec, std::map<std::string, double>& kvmap)
    {

        if(NULL != map_spec){
            // parse extra parameters:
            // - colon-separated key/value pairs: foo/3.14,bar/1.57...
            // - keys may contain [a-zA-Z0-9_-.]
            // - values are type double (parsed using %g)
            // - trims leading/trailing whitespace
            TRN_NDPRINT(5,  "%s:%d - parsing map_spec[%s]\n", __func__, __LINE__, map_spec);
            const char *kvdel="/";
            char *acpy = strdup(map_spec);
            char *next_pair = strtok_r(acpy, ":", &acpy);
            TRN_NDPRINT(5,  "%s:%d - next_pair[%s]\n", __func__, __LINE__, next_pair);
            while (next_pair != NULL) {
                char *kcpy = strdup(next_pair);
                char *skey = strtok(kcpy, kvdel);
                char *sval = strtok(NULL, kvdel);
                if(skey != NULL && sval != NULL) {
                    char *tkey = beam_geometry::trim(skey);
                    char *tval = beam_geometry::trim(sval);
                    TRN_NDPRINT(5,  "%s:%d - tkey[%s] tval[%s]\n", __func__, __LINE__, tkey, tval);
                    if (tkey != NULL && tval != NULL){
                        double dval = 0.0;
                        if (sscanf(tval, "%lf", &dval) == 1){
                            kvmap[tkey] = dval;
                            TRN_NDPRINT(5,  "%s:%d - added key[%s] val[%.3lf]\n", __func__, __LINE__, tkey, dval);                   }
                        }
                }
                free(kcpy);
                next_pair = strtok_r(acpy, ":", &acpy);
                TRN_NDPRINT(5,  "%s:%d - next_pair[%s]\n", __func__, __LINE__, next_pair);
            }
            free(acpy);
        }
    }

    virtual std::string tostring(int wkey=15, int wval=18) = 0;
    virtual double ro_u(int idx, geo_angle_units_t units = GA_RADIANS) = 0;
    virtual double tr_m(int idx) = 0;
    // extra parameters (key/value pairs)
    // keys may contain [a-zA-Z0-9_-.]
    std::map<std::string, double> xmap;

private:
};

class dvlgeo : public beam_geometry
{
public:


    dvlgeo()
    :beam_geometry()
    , beam_count(0)
    , yaw_rf(NULL)
    , pitch_rf(NULL)
    , rot_radius_m(0.)
    {
        memset(svr_deg,0,3*sizeof(double));
        memset(svt_m,0,3*sizeof(double));
//        fprintf(stderr,"%s:%d this[%p] yrf[%p] prf[%p]\n",__func__,__LINE__,this, yaw_rf,pitch_rf);
        yaw_rf = NULL;
        pitch_rf = NULL;
//        fprintf(stderr,"%s:%d this[%p] yrf[%p] prf[%p]\n",__func__,__LINE__,this, yaw_rf,pitch_rf);
    }

    dvlgeo(uint16_t nbeams, const char *bspec, double *rot, double *tran, double rrot=0.)
    :beam_geometry()
    , beam_count(nbeams)
    , yaw_rf(NULL)
    , pitch_rf(NULL)
    , rot_radius_m(rrot)
    {
        for(int i=0; i<3; i++)
        {
            if(rot != NULL)
                svr_deg[i] = rot[i];
            if(tran != NULL)
                svt_m[i] = tran[i];
        }

        beam_count = nbeams;
        size_t asz = sizeof(double) * nbeams;
        yaw_rf = (double *)malloc(asz);
        pitch_rf = (double *)malloc(asz);
        memset(yaw_rf, 0, asz);
        memset(pitch_rf, 0, asz);
        parse_bspec(bspec);
    }
    dvlgeo(uint16_t nbeams, const char *bspec, double *rot, double *tran, double rrot, std::map<std::string, double>& kvmap)
    : beam_geometry()
    , beam_count(nbeams)
    , yaw_rf(NULL)
    , pitch_rf(NULL)
    , rot_radius_m(rrot)
    {
        for(int i=0; i<3; i++)
        {
            if(rot != NULL)
                svr_deg[i] = rot[i];
            if(tran != NULL)
                svt_m[i] = tran[i];
        }

        beam_count = nbeams;
        size_t asz = sizeof(double) * nbeams;
        yaw_rf = (double *)malloc(asz);
        pitch_rf = (double *)malloc(asz);
        memset(yaw_rf, 0, asz);
        memset(pitch_rf, 0, asz);
        parse_bspec(bspec);
        xmap = kvmap;
    }

    ~dvlgeo()
    {
        free(yaw_rf);
        free(pitch_rf);
    }

    // caller must free sbspec
    static int parse_dvl_args(const char *spec, char **r_bspec, uint16_t *r_nbeams, double *r_svr, double *r_svt, double *r_rrot, std::map<std::string, double> *r_xmap)
    {
        int retval = 0;

        char *acpy = strdup(spec);

        TRN_NDPRINT(5,  "%s:%d - parsing dvlgeo [%s]\n", __func__, __LINE__, spec);

        char *sbspec_type = strtok(acpy, ":");
        char *snbeams = strtok(NULL, ":");
        char *sbspec = strtok(NULL, ":");
        char *srot = strtok(NULL, ":");
        char *strn = strtok(NULL, ":");
        char *srrot = strtok(NULL, ":");
        char *sxmap = strtok(NULL, ":");

        TRN_NDPRINT(5,  "%s:%d - parsing dvl/oi geo sbspec_type[%s] snbeams[%s] sbspec[%s] srot[%s] stx[%s] srrot[%s] sxmap[%s]\n", __func__, __LINE__, sbspec_type,
                    snbeams, sbspec, srot, strn, srrot, sxmap);

        char *bspec = NULL;
        uint16_t nbeams=0;
        double svr[3] = {0.};
        double svt[3] = {0.};
        double rrot = 0.;
        int n_parsed = 0;
        std::map<std::string, double> kvmap;

        if(NULL!=snbeams){
            int test = sscanf(snbeams,"%hu",&nbeams);
            if(test == 1 )
                n_parsed += test;
        }
        if(NULL!=sbspec){
            bspec = strdup(sbspec);
            n_parsed++;
        }
        if(NULL!=srot){
            int test = sscanf(srot, "%lf,%lf,%lf", &svr[0], &svr[1], &svr[2]);
            if(test == 3)
                n_parsed += test;
        }
        if(NULL!=strn){
            int test = sscanf(strn, "%lf,%lf,%lf", &svt[0], &svt[1], &svt[2]);
            if(test == 3)
                n_parsed += test;
        }
        if(NULL!=srrot){
            int test = sscanf(srrot, "%lf", &rrot);
            if(test == 1)
                n_parsed += test;
        }
        if(NULL != sxmap){

            beam_geometry::parse_map(sxmap, kvmap);
            n_parsed++;
        }
        if(n_parsed >= 8){
            if(nullptr != r_nbeams)
                *r_nbeams = nbeams;
            if(nullptr != r_bspec)
                *r_bspec = bspec;
            if(nullptr != r_svr)
                memcpy(r_svr, svr, 3 * sizeof(double));
            if(nullptr != r_svt)
                memcpy(r_svt, svt, 3 * sizeof(double));
            if(nullptr != r_rrot)
                *r_rrot = rrot;
            if(nullptr != r_xmap)
                *r_xmap = kvmap;

            retval = n_parsed;
        } else {
            retval = -1;
        }

        free(acpy);

        return retval;
    }

    static dvlgeo *parse_dvlgeo(const char *spec)
    {
        dvlgeo *retval = NULL;

        TRN_NDPRINT(5,  "%s:%d - parsing dvlgeo [%s]\n", __func__, __LINE__, spec);

        uint16_t nbeams=0;
        char *bspec = NULL;
        double svr[3] = {0.};
        double svt[3] = {0.};
        double rrot = 0.;
        std::map<std::string, double> kvmap;

        // must free bspec
        int test = parse_dvl_args(spec, &bspec, &nbeams, svr, svt, &rrot, &kvmap);

        if(test >= 8){
            retval = new dvlgeo(nbeams, bspec, &svr[0], &svt[0], rrot, kvmap);
        } else {
            fprintf(stderr, "%s:%d - parsing error expected >= 8 [%d]\n", __func__, __LINE__, test);
        }

        free(bspec);
        return retval;
    }

    void parse_bspec(const char *bspec)
    {
        if(NULL == bspec)
            return;

        char *spec = strdup(bspec);
        if(spec[0] == 'A'){
            double yb_deg = 0;
            double yi_deg = 0;
            double pb_deg = 0;
            double pi_deg = 0;
            if(sscanf(spec, "A,%lf,%lf,%lf,%lf",&yb_deg,&yi_deg,&pb_deg,&pi_deg ) == 4){
                for(int i=0 ; i<beam_count; i++){
                    yaw_rf[i] = yb_deg + i * yi_deg;
                    pitch_rf[i] = pb_deg + i * pi_deg;
                }
            } else {
                fprintf(stderr, "ERR - invalid auto beam spec [%s]\n",spec);
            }
        } else if(spec[0] == 'L') {
            fprintf(stderr, "%s:%d - spec[%s]\n", __func__, __LINE__, spec);
            char *scpy = strdup(spec);
            char *list_s = strtok(scpy,",");
            if(NULL != list_s){
                for(int i=0; i<beam_count; i++){
                    char *next_y = strtok(NULL,",");
                    char *next_p = strtok(NULL,",");
                    fprintf(stderr, "%s:%d - next_y,p[%d][%s, %s]\n", __func__, __LINE__, i, next_y, next_p);

                    if(next_y==NULL){
                        fprintf(stderr, "ERR - not enough tokens [%s]\n",spec);
                        break;
                    }
                    if(next_p==NULL){
                        fprintf(stderr, "ERR - not enough tokens [%s]\n",spec);
                        break;
                    }
                    if(sscanf(next_y,"%lf",&yaw_rf[i])!=1){
                        fprintf(stderr, "ERR - Y[%d] invalid [%s]\n",i,next_y);
                    }
                    if(sscanf(next_p,"%lf",&pitch_rf[i])!=1){
                        fprintf(stderr, "ERR - P[%d] invalid [%s]\n",i,next_p);
                    }
                }
            } else {
                fprintf(stderr, "ERR - not enough tokens [%s]\n",spec);
            }
            free(scpy);
        } else {
            fprintf(stderr, "ERR - unsupported beam spec type [%s]\n",spec);
        }

        for(int i=0 ; i<beam_count; i++){
            // normalize yaw to 0 : 360
            if(yaw_rf[i] < 0.)
                yaw_rf[i] = (fmod(yaw_rf[i],360.)) + 360.;
            if(yaw_rf[i] > 360.)
                yaw_rf[i] = fmod(yaw_rf[i],360.);
            // normalize pitch to -90 : 90
            pitch_rf[i] = fmod(pitch_rf[i],90.);
        }

        free(spec);
    }

    void tostream(std::ostream &os, int wkey=15, int wval=18)
    {
        os << std::setw(wkey) << "beam_count";
        os << std::setw(wval) << beam_count << "\n";
        os << std::setw(wkey) << "rotation" ;
        os << std::setw(wval) << "[";
        os << svr_deg[0] << "," << svr_deg[1] << "," << svr_deg[2] << "]\n";
        os << std::setw(wkey) << "translation";
        os << std::setw(wval) << "[";
        os << svt_m[0] << "," << svt_m[1] << "," << svt_m[2] << "]\n";
        if(beam_count > 0){
            os << std::setw(wkey) << "beam angles (Yi,Pi)\n";
            if(yaw_rf != nullptr && pitch_rf != nullptr){
                for(int i = 0; i < beam_count; i++){
                    os << std::setw(wkey-3) << "b[" << std::setw(2) << i << "]";
                    os << std::setw(wval) << "[" << yaw_rf[i] << "," << pitch_rf[i] << "]\n";
                }
            }
        }
        os << std::setw(wkey) << "rot_radius_m";
        os << std::setw(wval) << rot_radius_m << "\n";
        os << std::setw(wkey) << "xmap";
        os << std::setw(wval) << xmap.size() << "\n";
        std::map<std::string, double>::iterator it = xmap.begin();
        while (it != xmap.end()) {
            os << std::setw(wkey) << it->first;
            os << std::setw(wval) << static_cast<double>(it->second) << "\n";
            ++it;
        }
        os << "\n";
    }

    virtual std::string tostring(int wkey=15, int wval=18) override
    {
        std::ostringstream ss;
        tostream(ss, wkey, wval);
        return ss.str();
    }

    void show(int wkey=15, int wval=18)
    {
        tostream(std::cerr, wkey, wval);
    }

    virtual double ro_u(int idx, geo_angle_units_t units = GA_RADIANS) override
    {
        double angle = (units == GA_RADIANS ? DTR(svr_deg[idx % 3]) : svr_deg[idx % 3]);
        return angle;
    }

    virtual double tr_m(int idx) override
    {
        return svt_m[idx % 3];
    }

    // number of beams
    uint16_t beam_count;
    // sensor rotation relative to vehicle CRP (r/p/y  aka phi/theta/psi deg)
    double svr_deg[3];
    // sensor translation relative to vehicle CRP (x/y/z m)
    // +x: fwd +y: stbd, +z:down
    double svt_m[3];
    // transducer yaw angles (in sensor reference frame, deg)
    double *yaw_rf;
    // transducer pitch angles (in sensor reference frame, deg)
    double *pitch_rf;
    // DVL rotation radius (OI toolsled)
    double rot_radius_m;
};

class mbgeo : public beam_geometry
{
public:
    static const int MBG_PDEG=0;

    mbgeo()
    : beam_geometry()
    , beam_count(0.),
    swath_deg(0.)
    {
        size_t asz = 3 * sizeof(double);
        memset(svr_deg, 0, asz);
        memset(svt_m, 0, asz);
    }

    mbgeo(uint16_t nbeams, double swath, double *rot, double *tran, double rrot=0.)
    :  beam_geometry()
    , beam_count(nbeams)
    , swath_deg(swath)
    , rot_radius_m(rrot)
    {
        for(int i=0; i<3; i++)
        {
            if(rot != NULL)
                svr_deg[i] = rot[i];
            if(tran != NULL)
                svt_m[i] = tran[i];
        }
    }

    mbgeo(uint16_t nbeams, double swath, double *rot, double *tran, double rrot,  const std::map<std::string, double> &kvmap)
    :  beam_geometry()
    , beam_count(nbeams)
    , swath_deg(swath)
    , rot_radius_m(rrot)
    {
        for(int i=0; i<3; i++)
        {
            if(rot != NULL)
                svr_deg[i] = rot[i];
            if(tran != NULL)
                svt_m[i] = tran[i];
        }
        xmap = kvmap;
    }

    ~mbgeo(){}


   static mbgeo *parse_mbgeo(const char *spec)
    {
        mbgeo *retval = NULL;

       if(spec == NULL)
           return retval;

        char *acpy = strdup(spec);
        // skip name
        strtok(acpy,":");
        char *sbeams = strtok(NULL, ":");
        char *sswath = strtok(NULL, ":");
        char *srot = strtok(NULL, ":");
        char *strn = strtok(NULL, ":");
        char *srrot = strtok(NULL, ":");
        char *sxmap = strtok(NULL, ":");

       TRN_NDPRINT(5,  "%s:%d - parsing mbgeo spec[%s] sbeams[%s] sswath[%s] srot[%s] stx[%s] srrot[%s] sxmap[%s]\n", __func__, __LINE__, spec,
                    sbeams, sswath, srot, strn, srrot, sxmap);

        double svr[3] = {0};
        double svt[3] = {0};
        uint16_t beams=0;
        double swath=0;
        double rrot = 0.;
        std::map<std::string, double> kvmap;

       if(NULL != sbeams)
            sscanf(sbeams,"%hu",&beams);
        if(NULL != sswath)
            sscanf(sswath,"%lf",&swath);
        if(NULL != srot)
            sscanf(srot,"%lf,%lf,%lf",&svr[0],&svr[1],&svr[2]);
        if(NULL != strn)
            sscanf(strn,"%lf,%lf,%lf",&svt[0],&svt[1],&svt[2]);
        if(NULL != srrot)
           sscanf(srrot, "%lf", &rrot);
       if(NULL != sxmap){

           beam_geometry::parse_map(sxmap, kvmap);

       }

        retval = new mbgeo(beams, swath, &svr[0], &svt[0], rrot, kvmap);

        free(acpy);

        return retval;
    }

    void tostream(std::ostream &os, int wkey=15, int wval=18)
    {
        os << std::setw(wkey) << "beam_count";
        os << std::setw(wval) << beam_count << "\n";
        os << std::setw(wkey) << "swath";
        os << std::setw(wval) << swath_deg << "\n";
        os << std::setw(wkey) << "rotation" ;
        os << std::setw(wval) << "[";
        os << svr_deg[0] << "," << svr_deg[1] << "," << svr_deg[2] << "]\n";
        os << std::setw(wkey) << "translation";
        os << std::setw(wval) << "[";
        os << svt_m[0] << "," << svt_m[1] << "," << svt_m[2] << "]\n";
        os << std::setw(wkey) << "rot_radius_m";
        os << std::setw(wval) << rot_radius_m << "\n";
        os << std::setw(wkey) << "xmap";
        os << std::setw(wval) << xmap.size() << "\n";
        std::map<std::string, double>::iterator it = xmap.begin();
        while (it != xmap.end()) {
            os << std::setw(wkey) << it->first;
            os << std::setw(wval) << static_cast<double>(it->second) << "\n";
            ++it;
        }

        os << "\n";
    }

    virtual std::string tostring(int wkey=15, int wval=18) override
    {
        std::ostringstream ss;
        tostream(ss, wkey, wval);
        return ss.str();
    }

    void show(int wkey=15, int wval=18)
    {
        tostream(std::cerr, wkey, wval);
    }

    virtual double ro_u(int idx, geo_angle_units_t units = GA_RADIANS) override
    {
        double angle = (units == GA_RADIANS ? DTR(svr_deg[idx % 3]) : svr_deg[idx % 3]);
        return angle;
    }

    double tr_m(int idx) override
    {
        return svt_m[idx % 3];
    }

    // number of beams
    uint16_t beam_count;
    // angle between first and last beam
    double swath_deg;
    // sensor rotation relative to vehicle CRP (r/p/y  aka phi/theta/psi deg)
    double svr_deg[3];
    // sensor translation relative to vehicle CRP (x/y/z m)
    // +x: fwd +y: stbd, +z:down
    double svt_m[3];
    // device rotation radius (OI toolsled)
    double rot_radius_m;

};

class txgeo : public beam_geometry
{
public:
    txgeo()
    :  beam_geometry()
    , mRotLen(0)
    , rot_deg(nullptr)
    , mTranLen(0)
    , tran_m(nullptr)
    {
    }

    txgeo(int rlen, double *rot, int tlen, double *tran)
    : beam_geometry()
    {
        if(rlen > 0) {
            int ra_len = rlen * sizeof(double);
            rot_deg = (double *)malloc(ra_len);

            if(rot_deg != NULL){
                memset(rot_deg, 0, ra_len);
                memcpy(rot_deg, rot, ra_len);
            } else {
                fprintf(stderr, "%s:%d - ERR: could not allocate rot_deg\n", __func__, __LINE__);
            }
        } else {
            mRotLen = 0;
            rot_deg = nullptr;
        }

        if(tlen > 0) {
            int ra_len = tlen * sizeof(double);
            tran_m = (double *)malloc(ra_len);
            if(tran_m != NULL){
                memset(tran_m, 0, ra_len);
                memcpy(tran_m, tran, ra_len);
            } else {
                fprintf(stderr, "%s:%d - ERR: could not allocate tran_m\n", __func__, __LINE__);
            }
        } else {
            mTranLen = 0;
            tran_m = nullptr;
        }
    }

    txgeo(int rlen, double *rot, int tlen, double *tran, const std::map<std::string, double> &kvmap)
    : txgeo(rlen, rot, tlen, tran)
    {
    }

    ~txgeo()
    {
        free(rot_deg);
        free(tran_m);
    }

    static txgeo *parse_txgeo(const char *spec)
    {
        txgeo *retval = NULL;
        TRN_NDPRINT(5,  "%s:%d - parsing txgeo spec[%s]\n", __func__, __LINE__, spec);

        if(spec == NULL)
            return retval;

        char *acpy = strdup(spec);
        // skip name
        strtok(acpy,":");
        char *srlen = strtok(NULL, ":");
        char *srot = strtok(NULL, ":");
        char *stlen = strtok(NULL, ":");
        char *stran = strtok(NULL, ":");
        char *sxmap = strtok(NULL, "*");

        TRN_NDPRINT(5,  "%s:%d - parsing txgeo spec[%s] srlen[%s] srot[%s] stlen[%s] stran[%s] sxmap[%s]\n", __func__, __LINE__, spec,
                    srlen, srot, stlen, stran, sxmap);

        int err_count = 0;
        int rlen = 0;
        int tlen = 0;
        double *rot = NULL;
        double *tran = NULL;
        std::map<std::string, double> kvmap;

        if (NULL != srlen)
            sscanf(srlen,"%d",&rlen);
        if (NULL != stlen)
            sscanf(stlen,"%d",&tlen);

        if((rlen % 3) != 0) {
            while ( (rlen % 3) != 0) rlen++;
            fprintf(stderr, "%s:%d - WARN: rlen not a multiple of 3; padding with zero to %d\n", __func__, __LINE__, rlen);

        }
        if((tlen % 3) != 0) {
            while ( (tlen % 3) != 0) tlen++;
            fprintf(stderr, "%s:%d - WARN: tlen not a multiple of 3; padding with zero to %d\n", __func__, __LINE__, tlen);

        }

        if (rlen > 0) {
            char *xcpy = strdup(srot);
            rot = (double *) malloc(rlen * sizeof(double));

            if (rot != NULL) {

                memset(rot, 0, rlen * sizeof(double));

                char *next_s = strtok(xcpy, ",");
                for (int i = 0; i < rlen; i++) {
                    if(next_s != NULL)
                        sscanf(next_s, "%lf", &rot[i]);
                    else
                        rot[i] = 0.;
                    next_s = strtok(NULL, ",");
                }
                free(xcpy);
            } else {
                fprintf(stderr, "%s:%d - ERR: could not allocate temp rot array\n", __func__, __LINE__);
                err_count++;
            }

        } else {
            fprintf(stderr, "%s:%d - ERR: rlen <= 0\n", __func__, __LINE__);
            err_count++;
        }

        if (tlen > 0) {

            char *xcpy = strdup(stran);
            tran = (double *) malloc(tlen * sizeof(double));

            if (tran != NULL) {

                memset(tran, 0, tlen * sizeof(double));

                char *next_s = strtok(xcpy, ",");
                for (int i = 0; i < tlen; i++) {
                    if(next_s != NULL)
                        sscanf(next_s, "%lf", &tran[i]);
                    else
                        tran[i] = 0.;
                    next_s = strtok(NULL, ",");
                }
                free(xcpy);
            } else {
                fprintf(stderr, "%s:%d - ERR: could not allocate temp tran array\n", __func__, __LINE__);
            }

        } else {
            fprintf(stderr, "%s:%d - ERR: tlen <= 0\n", __func__, __LINE__);
        }

        if(NULL != sxmap){

            beam_geometry::parse_map(sxmap, kvmap);

        }

        if(err_count == 0) {
            retval = new txgeo(rlen, rot, tlen, tran, kvmap);
        } else {
            fprintf(stderr, "%s:%d - parse errors, returning NULL spec[%s]\n", __func__, __LINE__, spec);

        }

        free(acpy);

        return retval;
    }

    void tostream(std::ostream &os, int wkey=15, int wval=18)
    {
        os << std::setw(wkey) << "rotation" ;
        os << std::setw(wval) << mRotLen << std::endl;

        for (int i = 0; i < mRotLen; i += 3) {
            os << std::setw(wkey) << i/3;
            os << std::setw(wval) << "[";
            os << rot_deg[i] << "," << rot_deg[i + 1] << "," << rot_deg[i + 2] << "]\n";
        }


        os << std::setw(wkey) << "translation" ;
        os << std::setw(wval) << mTranLen << std::endl;

        for (int i = 0; i < mTranLen; i += 3) {
            os << std::setw(wkey) << i/3;
            os << std::setw(wval) << "[";
            os << tran_m[i] << "," << tran_m[i + 1] << "," << tran_m[i + 2] << "]\n";
        }

        os << std::setw(wkey) << "xmap";
        os << std::setw(wval) << xmap.size() << "\n";
        std::map<std::string, double>::iterator it = xmap.begin();
        while (it != xmap.end()) {
            os << std::setw(wkey) << it->first;
            os << std::setw(wval) << static_cast<double>(it->second) << "\n";
            ++it;
        }

        os << "\n";
    }

    virtual std::string tostring(int wkey=15, int wval=18) override
    {
        std::ostringstream ss;
        tostream(ss, wkey, wval);
        return ss.str();
    }

    void show(int wkey=15, int wval=18)
    {
        tostream(std::cerr, wkey, wval);
    }

    virtual double ro_u(int idx, geo_angle_units_t units = GA_RADIANS) override
    {
        double angle = (units == GA_RADIANS ? DTR(rot_deg[idx % 3]) : rot_deg[idx % 3]);
        return angle;
    }

    double tr_m(int idx) override
    {
        return tran_m[idx % 3];
    }

    // rot_deg array len (multiple of 3)
    int mRotLen;

    // sensor rotation relative to vehicle CRP (r/p/y  aka phi/theta/psi deg)
    double *rot_deg;

    // tran_m array len (multiple of 3)
    int mTranLen;

    // sensor translation relative to vehicle CRP (x/y/z m)
    // +x: fwd +y: stbd, +z:down
    double *tran_m;

};
#endif

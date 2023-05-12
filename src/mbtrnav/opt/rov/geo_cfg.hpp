
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

#ifndef GEO_CFG_H
#define GEO_CFG_H

class beam_geometry
{
public:
    beam_geometry(){}
    ~beam_geometry(){}
private:
};

class dvlgeo : public beam_geometry
{
public:


    dvlgeo()
    :beam_count(0)
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
    :beam_count(nbeams)
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

    ~dvlgeo()
    {
        free(yaw_rf);
        free(pitch_rf);
    }

    // caller must free sbspec
    static int parse_dvl_args(const char *spec, char **r_bspec, uint16_t *r_nbeams, double *r_svr, double *r_svt, double *r_rrot)
    {
        int retval = 0;

        char *acpy = strdup(spec);

        TRN_NDPRINT(5,  "%s:%d - parsing dvlgeo [%s]\n", __func__, __LINE__, spec);

        char *sbspec_type = strtok(acpy,":");
        char *snbeams = strtok(NULL,":");
        char *sbspec = strtok(NULL,":");
        char *srot = strtok(NULL,":");
        char *strn = strtok(NULL,":");
        char *srrot = strtok(NULL,":");

        TRN_NDPRINT(5,  "%s:%d - parsing dvl/oi geo sbspec_type[%s] snbeams[%s] sbspec[%s] srot[%s] stx[%s] srrot[%s]\n", __func__, __LINE__, sbspec_type,
                    snbeams, sbspec, srot, strn, srrot);

        char *bspec = NULL;
        uint16_t nbeams=0;
        double svr[3] = {0.};
        double svt[3] = {0.};
        double rrot = 0.;
        int n_parsed = 0;

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

        // must free bspec
        int test = parse_dvl_args(spec, &bspec, &nbeams, svr, svt, &rrot);

        if(test >= 8){
            retval = new dvlgeo(nbeams, bspec, &svr[0], &svt[0], rrot);
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
        os << "\n";
    }

    std::string tostring(int wkey=15, int wval=18)
    {
        std::ostringstream ss;
        tostream(ss, wkey, wval);
        return ss.str();
    }

    void show(int wkey=15, int wval=18)
    {
        tostream(std::cerr, wkey, wval);
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
    :beam_count(0.),swath_deg(0.)
    {
        size_t asz = 3 * sizeof(double);
        memset(svr_deg, 0, asz);
        memset(svt_m, 0, asz);
    }

    mbgeo(uint16_t nbeams, double swath, double *rot, double *tran, double rrot=0.)
    : beam_count(nbeams)
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

    ~mbgeo(){}

   static mbgeo *parse_mbgeo(const char *spec)
    {
        mbgeo *retval = NULL;

        char *acpy = strdup(spec);
        // skip name
        strtok(acpy,":");
        char *sbeams = strtok(NULL,":");
        char *sswath = strtok(NULL,":");
        char *srot = strtok(NULL,":");
        char *strn = strtok(NULL,":");
        char *srrot = strtok(NULL,":");

       TRN_NDPRINT(5,  "%s:%d - parsing mbgeo spec[%s] sbeams[%s] sswath[%s] srot[%s] stx[%s] srrot[%s]\n", __func__, __LINE__, spec,
                    sbeams, sswath, srot, strn, srrot);

        double svr[3] = {0};
        double svt[3] = {0};
        uint16_t beams=0;
        double swath=0;
        double rrot = 0.;

       if(NULL!=sbeams)
            sscanf(sbeams,"%hu",&beams);
        if(NULL!=sswath)
            sscanf(sswath,"%lf",&swath);
        if(NULL!=srot)
            sscanf(srot,"%lf,%lf,%lf",&svr[0],&svr[1],&svr[2]);
        if(NULL!=strn)
            sscanf(strn,"%lf,%lf,%lf",&svt[0],&svt[1],&svt[2]);
        if(NULL!=srrot)
           sscanf(srrot, "%lf", &rrot);

        retval = new mbgeo(beams, swath, &svr[0], &svt[0], rrot);

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
        os << "\n";
    }

    std::string tostring(int wkey=15, int wval=18)
    {
        std::ostringstream ss;
        tostream(ss, wkey, wval);
        return ss.str();
    }

    void show(int wkey=15, int wval=18)
    {
        tostream(std::cerr, wkey, wval);
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

//class oigeo : public dvlgeo
//{
//public:
//    oigeo()
//    : dvlgeo()
//    , tilt_deg(0.)
//    , dvl_rot_radius_m(0.)
//    {
//
//    }
//
//    oigeo(uint16_t nbeams, const char *bspec, double *rot, double *tran, double pivot = 0.)
//    : dvlgeo(nbeams, bspec, rot, tran)
//    , tilt_deg(0.)
//    , dvl_rot_radius_m(pivot)
//    {
//
//    }
//
//    ~oigeo()
//    {
//    }
//
//    void tostream(std::ostream &os, int wkey=15, int wval=18)
//    {
//        dvlgeo::tostream(os, wkey, wval);
//        os << std::setw(wkey) << "tilt_deg";
//        os << std::setw(wval) << tilt_deg << "\n";
//        os << std::setw(wkey) << "dvl_rot_radius_m";
//        os << std::setw(wval) << dvl_rot_radius_m << "\n";
//    }
//
//    std::string tostring(int wkey=15, int wval=18)
//    {
//        std::ostringstream ss;
//        tostream(ss, wkey, wval);
//        return ss.str();
//    }
//
//    void show(int wkey=15, int wval=18)
//    {
//        tostream(std::cerr, wkey, wval);
//    }
//
//    double tilt_deg;
//    // DVL rotation radius
//    double dvl_rot_radius_m;
//};
#endif

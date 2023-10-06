#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include "trnxpp.hpp"
#include "trn_debug.hpp"
#include "trnx_utils.hpp"

#ifndef TRNX_PLUGIN_H
#define TRNX_PLUGIN_H

extern "C" int cb_proto_oisledx(void *pargs);

extern "C" void transform_dvl(trn::bath_info *bi, trn::att_info *ai, dvlgeo *geo, mb1_t *r_snd);
extern "C" int cb_proto_dvl(void *pargs);


extern "C" void transform_deltat(trn::bath_info *bi, trn::att_info *ai, mbgeo *geo, mb1_t *r_snd);
extern "C" int cb_proto_deltat(void *pargs);

extern "C" void transform_oidvl(trn::bath_info **bi, trn::att_info **ai, dvlgeo **geo, mb1_t *r_snd);
extern "C" int cb_proto_oisled(void *pargs);// dec 2022

extern "C" void transform_oidvl2(trn::bath_info **bi, trn::att_info **ai, dvlgeo **geo, mb1_t *r_snd);
extern "C" int cb_proto_oisled2(void *pargs);

class TrnxPlugin
{
public:
    TrnxPlugin(){}
    ~TrnxPlugin(){}

    static void register_callbacks(trn::trnxpp &xpp)
    {
        xpp.register_callback("cb_proto_dvl", cb_proto_dvl);
        xpp.register_callback("cb_proto_deltat", cb_proto_deltat);
        xpp.register_callback("cb_proto_oisled", cb_proto_oisled);
        xpp.register_callback("cb_proto_oisled2", cb_proto_oisled2);
        xpp.register_callback("cb_proto_oisledx", cb_proto_oisledx);
        xpp.register_callback("cb_proto_mblass", cb_proto_oisledx);
    }
};

#endif

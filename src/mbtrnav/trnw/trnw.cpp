///
/// @file trnw.cpp
/// @authors k. headley
/// @date 2019-06-21

/// C wrappers for TerrainNav API

/// @sa doxygen-examples.c for more examples of Doxygen markup


/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information

 Copyright 2002-2019 MBARI
 Monterey Bay Aquarium Research Institute, all rights reserved.

 Terms of Use

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version. You can access the GPLv3 license at
 http://www.gnu.org/licenses/gpl-3.0.html

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details
 (http://www.gnu.org/licenses/gpl-3.0.html)

 MBARI provides the documentation and software code "as is", with no warranty,
 express or implied, as to the software, title, non-infringement of third party
 rights, merchantability, or fitness for any particular purpose, the accuracy of
 the code, or the performance or results which you may obtain from its use. You
 assume the entire risk associated with use of the code, and you agree to be
 responsible for the entire cost of repair or servicing of the program with
 which you are using the code.

 In no event shall MBARI be liable for any damages, whether general, special,
 incidental or consequential damages, arising out of your use of the software,
 including, but not limited to, the loss or corruption of your data or damages
 of any kind resulting from use of the software, any prohibited use, or your
 inability to use the software. You agree to defend, indemnify and hold harmless
 MBARI and its officers, directors, and employees against any claim, loss,
 liability or expense, including attorneys' fees, resulting from loss of or
 damage to property or the injury to or death of any person arising out of the
 use of the software.

 The MBARI software is provided without obligation on the part of the
 Monterey Bay Aquarium Research Institute to assist in its use, correction,
 modification, or enhancement.

 MBARI assumes no responsibility or liability for any third party and/or
 commercial software required for the database or applications. Licensee agrees
 to obtain and maintain valid licenses for any additional third party software
 required.
 */

/////////////////////////
// Headers
/////////////////////////
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "trnw.h"
#include "TerrainNav.h"
#include "NavUtils.h"
#include "MathP.h"
#include "structDefs.h"
#include "mb1_msg.h"
#include "trn_msg.h"

/////////////////////////
// Macros
/////////////////////////

// These macros should only be defined for
// application main files rather than general C files
/*
/// @def PRODUCT
/// @brief header software product name
#define PRODUCT "TBD_PRODUCT"

/// @def COPYRIGHT
/// @brief header software copyright info
#define COPYRIGHT "Copyright 2002-YYYY MBARI Monterey Bay Aquarium Research Institute, all rights reserved."
/// @def NOWARRANTY
/// @brief header software terms of use
#define NOWARRANTY  \
"This program is distributed in the hope that it will be useful,\n"\
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"\
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"\
"GNU General Public License for more details (http://www.gnu.org/licenses/gpl-3.0.html)\n"
*/

/////////////////////////
// Declarations
/////////////////////////

struct wtnav_s {
    void *obj;
};

struct wposet_s {
    void *obj;
};

struct wmeast_s {
    void *obj;
};
struct wcommst_s {
    void *obj;
};

void poset_show(poseT *obj, bool verbose, int indent);
void meast_show(measT *obj, bool verbose, int indent);
void trncfg_show(trn_config_t *obj, bool verbose, int indent);
wposet_t *wposet_cnew(poseT *pt);
wmeast_t *wmeast_cnew(measT *pt);

static void s_wposet_to_opose(poseT *dest, pt_cdata_t *src);
static void s_wmeast_to_omeas(measT *dest, mt_cdata_t *src);

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////


///////////////////////////
//// Function Definitions
///////////////////////////

wtnav_t *wtnav_dnew()
{
    wtnav_t *m = (wtnav_t *)malloc(sizeof(*m));
    if(NULL!=m){
        memset(m,0,sizeof(*m));
        TerrainNav *obj    = new TerrainNav();
        m->obj = obj;
    }

    return m;
}

//wtnav_t *wtnav_new(char* mapName, char* vehicleSpecs, char* particles,
//                   const int filterType, const int mapType, char* directory)
wtnav_t *wtnav_new(trn_config_t *cfg)
{
    wtnav_t *m = (wtnav_t *)malloc(sizeof(*m));
    if(NULL!=m){
        memset(m,0,sizeof(*m));
        TerrainNav *obj = new TerrainNav(cfg->map_file, cfg->cfg_file, cfg->particles_file, cfg->filter_type, cfg->map_type, cfg->log_dir);
        m->obj = obj;
        if(NULL!=obj){
            obj->setModifiedWeighting(cfg->mod_weight);
            obj->setFilterReinit(cfg->filter_reinit==0?false:true);
            switch (cfg->filter_grade) {
                case 0:
                    obj->useLowGradeFilter();
                    break;
                case 1:
                    obj->useHighGradeFilter();
                    break;
                default:
                    break;
            }
    	}
    }

    return m;
}

void wtnav_destroy(wtnav_t *self)
{
    if (NULL!=self){
        delete static_cast<TerrainNav *>(self->obj);
        free(self);
    }
}

void wtnav_estimate_pose(wtnav_t *self, wposet_t *estimate, const int type)
{
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        poseT *ptobj = static_cast<poseT *>(estimate->obj);
        if(NULL!=obj && NULL!=ptobj){
        obj->estimatePose(ptobj, type);
        }
        return ;
    }
}

void wtnav_meas_update(wtnav_t *self, wmeast_t *incomingMeas, const int type)
{
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        measT *mtobj = static_cast<measT *>(incomingMeas->obj);
        if(NULL!=obj && NULL!=mtobj){
       obj->measUpdate(mtobj, type);
        }
        return ;
    }
}

void wtnav_motion_update(wtnav_t *self, wposet_t *incomingNav)
{
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        poseT *ptobj = static_cast<poseT *>(incomingNav->obj);
        if(NULL!=obj && NULL!=ptobj){
        obj->motionUpdate(ptobj);
        }
        return ;
    }
}
bool wtnav_last_meas_successful(wtnav_t *self)
{
    bool retval=false;
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        if(NULL!=obj){
        retval = obj->lastMeasSuccessful();
        }
    }
    return retval;
}
bool wtnav_outstanding_meas(wtnav_t *self)
{
    bool retval=false;
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        if(NULL!=obj){
        retval = obj->outstandingMeas();
        }
    }
    return retval;
}
bool wtnav_initialized(wtnav_t *self)
{
    bool retval=false;
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        if(NULL!=obj){
            retval = obj->initialized();
        }
    }
    return retval;
}
bool wtnav_is_converged(wtnav_t *self)
{
    bool retval=false;
   if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
       if(NULL!=obj && NULL!=obj->tNavFilter){
       retval = obj->tNavFilter->isConverged();
       }
    }
    return retval;
}

void wtnav_set_est_nav_offset(wtnav_t *self, double ofs_x, double ofs_y, double ofs_z)
{
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        if(NULL!=obj){
            obj->setEstNavOffset(ofs_x, ofs_y, ofs_z);
        }
        return ;
    }
}

d_triplet_t *wtnav_get_est_nav_offset(wtnav_t *self, d_triplet_t *dest)
{
    d_triplet_t *retval=NULL;
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        if(NULL!=obj){
            retval=obj->getEstNavOffset(dest);
        }
    }
    return retval;
}

void wtnav_set_init_stddev_xyz(wtnav_t *self, double sdev_x, double sdev_y, double sdev_z)
{
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        if(NULL!=obj){
            obj->setInitStdDevXYZ(sdev_x, sdev_y, sdev_z);
        }
        return ;
    }
}

d_triplet_t *wtnav_get_init_stddev_xyz(wtnav_t *self, d_triplet_t *dest)
{
    d_triplet_t *retval=NULL;
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        if(NULL!=obj){
            retval=obj->getInitStdDevXYZ(dest);
        }
    }
    return retval;
}

void wtnav_reinit_filter(wtnav_t *self, bool lowInfoTransition)
{
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        if(NULL!=obj){
        obj->reinitFilter(lowInfoTransition);
        }
        return ;
    }
}

void wtnav_reinit_filter_offset(wtnav_t *self, bool lowInfoTransition,
                             double offset_x, double offset_y, double offset_z)
{
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        if(NULL!=obj){
            obj->reinitFilterOffset(lowInfoTransition, offset_x, offset_y, offset_z);
        }
    }
    return;
}

void wtnav_reinit_filter_box(wtnav_t *self, bool lowInfoTransition,
                             double offset_x, double offset_y, double offset_z,
                             double sdev_x, double sdev_y, double sdev_z)
{
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        if(NULL!=obj){
            obj->reinitFilterBox(lowInfoTransition, offset_x, offset_y, offset_z,sdev_x, sdev_y, sdev_z);
        }
    }
    return;
}

int wtnav_get_filter_type(wtnav_t *self)
{
    int retval = -1;
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        if(NULL!=obj){
        retval = obj->getFilterType();
        }
    }
    return retval;
}

int wtnav_get_filter_state(wtnav_t *self)
{
    int retval = -1;
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        if(NULL!=obj){
        retval = obj->getFilterState();
        }
    }
    return retval;
}

void wtnav_use_highgrade_filter(wtnav_t *self)
{
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        if(NULL!=obj){
        obj->useHighGradeFilter();
        }
    }
    return ;
}

void wtnav_use_lowgrade_filter(wtnav_t *self)
{
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        if(NULL!=obj){
        obj->useLowGradeFilter();
        }
    }
    return ;
}

void wtnav_set_filter_reinit(wtnav_t *self, const bool allow)
{
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        if(NULL!=obj){
        obj->setFilterReinit(allow);
        }
    }
    return ;
}

int wtnav_get_num_reinits(wtnav_t *self)
{
    int retval=-1;
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        if(NULL!=obj){
        retval = obj->getNumReinits();
        }
    }
    return retval;
}

void wtnav_set_interp_meas_attitude(wtnav_t *self, const bool set)
{
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        if(NULL!=obj){
        obj->setInterpMeasAttitude(set);
        }
    }
    return ;
}

void wtnav_set_map_interp_method(wtnav_t *self, const int type)
{
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        if(NULL!=obj){
        obj->setMapInterpMethod(type);
        }
    }
    return ;
}
void wtnav_set_vehicle_drift_rate(wtnav_t *self, const double driftRate)
{
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        if(NULL!=obj){
        obj->setVehicleDriftRate(driftRate);
        }
    }
    return ;
}
void wtnav_set_modified_weighting(wtnav_t *self, const int use)
{
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        if(NULL!=obj){
        obj->setModifiedWeighting(use);
        }
    }
    return ;
}
void wtnav_release_map(wtnav_t *self)
{
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        if(NULL!=obj){
        obj->releaseMap();
        }
    }
    return ;
}

void *wtnav_obj_addr(wtnav_t *self)
{
    void *retval=NULL;
    if(NULL!=self){
        TerrainNav *obj = static_cast<TerrainNav *>(self->obj);
        retval = obj;
    }
    return retval;
}

wcommst_t *wcommst_dnew()
{
    wcommst_t *m = (wcommst_t *)malloc(sizeof(*m));
    if(NULL!=m){
        memset(m,0,sizeof(*m));
        commsT *obj = new commsT();
        m->obj = obj;
    }

    return m;
}

void wcommst_destroy(wcommst_t *self)
{
    if (NULL!=self){
        delete static_cast<wcommst_t *>(self->obj);
        free(self);
    }
}

void commst_show(commsT *obj, bool verbose, int indent)
{
    if (NULL != obj) {
        fprintf(stderr,"%*s[self      %10p]\n",indent,(indent>0?" ":""), obj);
        fprintf(stderr,"%*s[msg_type  %10c]\n",indent,(indent>0?" ":""), obj->msg_type);
        fprintf(stderr,"%*s[parameter %10d]\n",indent,(indent>0?" ":""), obj->parameter);
        fprintf(stderr,"%*s[vdr       %10f]\n",indent,(indent>0?" ":""), obj->vdr);
        fprintf(stderr,"%*s[pt        %10p]\n",indent,(indent>0?" ":""), &obj->pt);
        if(verbose)
            poset_show(&obj->pt,false,indent+3);
        fprintf(stderr,"%*s[mt        %10p]\n",indent,(indent>0?" ":""), &obj->mt);
        if(verbose)
            meast_show(&obj->mt,false,indent+3);
        fprintf(stderr,"%*s[mapname   %10s]\n",indent,(indent>0?" ":""), obj->mapname);
        fprintf(stderr,"%*s[cfgname   %10s]\n",indent,(indent>0?" ":""), obj->cfgname);
        fprintf(stderr,"%*s[partname  %10s]\n",indent,(indent>0?" ":""), obj->particlename);
        fprintf(stderr,"%*s[logname   %10s]\n",indent,(indent>0?" ":""), obj->logname);
    }
}
void wcommst_show(wcommst_t *self, bool verbose, int indent)
{
    if (NULL != self) {
        commsT *obj = static_cast<commsT *>(self->obj);
        commst_show(obj,verbose,indent);
    }
}

ct_cdata_t *wcommst_cdata_new()
{
    ct_cdata_t *instance = (ct_cdata_t *)calloc(1,sizeof(*instance));
    if(NULL!=instance){
        instance->pt=NULL;
        instance->mt=NULL;
        instance->mapname=NULL;
        instance->cfgname=NULL;
        instance->particlename=NULL;
        instance->logname=NULL;
    }
    return instance;
}
void wcommst_cdata_destroy(ct_cdata_t **pself)
{
    if(NULL!=pself){
    	ct_cdata_t *self = *pself;
        if(NULL!=self){
            if(NULL!=self->pt)wposet_destroy(self->pt);
            if(NULL!=self->mt)wmeast_destroy(self->mt);
            if(NULL!=self->mapname)free(self->mapname);
            if(NULL!=self->cfgname)free(self->cfgname);
            if(NULL!=self->particlename)free(self->particlename);
            if(NULL!=self->logname)free(self->logname);

            free(self);
            *pself=NULL;
        }
    }
}
uint32_t  wcommst_cdata_serialize(char *dest, ct_cdata_t *src, int len)
{
    uint32_t retval=0;

    if(NULL!=dest && NULL!=src && len>0){
        // convert to commsT, then serialize
        // it's good in that it uses the
        // underlying commsT.serialize()
        // on the down side, it requires
        // a lot of copying/conversion b/c
        // commsT implements poseT and measT
        // instances instead of references (pointers)

        commsT *ct = new commsT();

        ct->msg_type = src->msg_type;
        ct->parameter = src->parameter;
        ct->vdr = src->vdr;

        pt_cdata_t *pptc = NULL;
        wposet_pose_to_cdata(&pptc,src->pt);
        s_wposet_to_opose(&ct->pt, pptc);

        mt_cdata_t *pmtc = NULL;
        wmeast_meas_to_cdata(&pmtc,src->mt);
        s_wmeast_to_omeas(&ct->mt, pmtc);

        ct->mapname = (NULL!=src->mapname?strdup(src->mapname):NULL);
        ct->cfgname = (NULL!=src->cfgname?strdup(src->cfgname):NULL);
        ct->particlename = (NULL!=src->particlename?strdup(src->particlename):NULL);
        ct->logname = (NULL!=src->logname?strdup(src->mapname):NULL);

        ct->serialize(dest,len);

        //destroy the commsT object
        delete ct;
        free(pmtc);
        free(pptc);


    }
    return retval;
}

int wcommst_cdata_unserialize(ct_cdata_t **dest, char *src)
{
    int retval=-1;
    //    fprintf(stderr,"%s:%d d[%p] s[%p]\n",__FUNCTION__,__LINE__,dest,src);
    if(NULL!=dest && NULL!=src){
        ct_cdata_t *cdata = *dest;
        if(NULL==cdata){
            cdata=wcommst_cdata_new();
            *dest=cdata;
        }
        //        fprintf(stderr,"%s:%d pose[%p] s[%p]\n",__FUNCTION__,__LINE__,pose,src);
        if(NULL!=cdata){
            commsT *ct = new commsT();
            ct->unserialize(src);
            cdata->msg_type = ct->msg_type;
            cdata->parameter = ct->parameter;
            cdata->vdr = ct->vdr;

            cdata->pt = wposet_cnew(&ct->pt);
            cdata->mt = wmeast_cnew(&ct->mt);
            cdata->mapname = (NULL!=ct->mapname?strdup(ct->mapname):NULL);
            cdata->cfgname = (NULL!=ct->cfgname?strdup(ct->cfgname):NULL);
            cdata->particlename = (NULL!=ct->particlename?strdup(ct->particlename):NULL);
            cdata->logname = (NULL!=ct->logname?strdup(ct->mapname):NULL);

            delete ct;
            retval=0;
        }
    }
    return retval;
}

int  wcommst_serialize(char **pdest, wcommst_t *src, int len)
{
    int retval=-1;
    if(NULL!=src && NULL!=pdest){
//        fprintf(stderr,"%s:%d pdest[%p/%p] len[%d]\n",__FUNCTION__,__LINE__,pdest,*pdest,len);
        char *dest = *pdest;
        if(NULL==dest){
            dest = (char *)malloc(len);
            *pdest = dest;
        }
        commsT *obj = static_cast<commsT *>(src->obj);
        obj->serialize(dest,len);
        retval=TRN_MSG_SIZE; //len;
    }
    return retval;
}

int  wcommst_unserialize(wcommst_t **dest, char *src, int len)
{
    int retval=-1;
    if(NULL!=src && NULL!=dest){
        wcommst_t *ct = *dest;
        if(NULL==ct){
            ct = wcommst_dnew();
            *dest = ct;
        }
        commsT *obj = static_cast<commsT *>(ct->obj);
        retval = obj->unserialize(src,len);

    }
    return retval;
}

int wcommst_get_pt(wposet_t **dest, wcommst_t *src)
{
    int retval=-1;
    if(NULL!=src && NULL!=dest){
        commsT *ct = static_cast<commsT *>(src->obj);

        wposet_t *pose = (wposet_t *)(*dest);
        if(NULL==pose){
            pose=wposet_dnew();
            *dest=pose;
        }
        poseT *obj = static_cast<poseT *>(pose->obj);
        *obj = ct->pt;
        retval=0;

    }
    return retval;
}
int wcommst_set_pt(wcommst_t *self, wposet_t *wpt)
{
    int retval=-1;
    if(NULL!=self && NULL!=wpt){
        commsT *ct = static_cast<commsT *>(self->obj);

        poseT *obj = static_cast<poseT *>(wpt->obj);
        ct->pt = *obj;
        retval=0;
    }
    return retval;
}

int wcommst_get_mt(wmeast_t **dest, wcommst_t *src)
{
    int retval=-1;
    if(NULL!=src && NULL!=dest){
        commsT *ct = static_cast<commsT *>(src->obj);

        wmeast_t *meas = (wmeast_t *)(*dest);
        if(NULL==meas){
            meas=wmeast_dnew();
            *dest=meas;
        }
        measT *obj = static_cast<measT *>(meas->obj);
        *obj = ct->mt;
        retval=0;
    }
    return retval;
}
int wcommst_set_mt(wcommst_t *self, wmeast_t *wmt)
{
    int retval=-1;
    if(NULL!=self && NULL!=wmt){
        commsT *ct = static_cast<commsT *>(self->obj);

        measT *obj = static_cast<measT *>(wmt->obj);
        ct->mt = *obj;
        retval=0;
    }
    return retval;
}
int wcommst_get_parameter(wcommst_t *self)
{
    int retval=-1;
    if(NULL!=self){
        commsT *ct = static_cast<commsT *>(self->obj);
        retval = ct->parameter;
    }
    return retval;
}
float wcommst_get_vdr(wcommst_t *self)
{
    float retval=-1;
    if(NULL!=self){
        commsT *ct = static_cast<commsT *>(self->obj);
        retval = ct->vdr;
    }
    return retval;
}

char wcommst_get_msg_type(wcommst_t *self)
{
    int retval=-1;
    if(NULL!=self){
        commsT *ct = static_cast<commsT *>(self->obj);
        retval = ct->msg_type;
    }
    return retval;
}

d_triplet_t *wcommst_get_xyz_sdev(wcommst_t *self, d_triplet_t *dest)
{
    d_triplet_t *retval=NULL;
    if(NULL!=self && NULL!=dest){
        commsT *ct = static_cast<commsT *>(self->obj);
        memcpy(dest,&ct->xyz_sdev, sizeof(d_triplet_t));
        retval = dest;
    }
    return retval;
}

d_triplet_t *wcommst_get_est_nav_offset(wcommst_t *self, d_triplet_t *dest)
{
    d_triplet_t *retval=NULL;
    if(NULL!=self && NULL!=dest){
        commsT *ct = static_cast<commsT *>(self->obj);
        memcpy(dest,&ct->est_nav_ofs, sizeof(d_triplet_t));
        retval = dest;
    }
    return retval;
}

void commst_meas_update(wtnav_t *self, wcommst_t *msg)
{
    if(NULL!=self && NULL!=msg){
        TerrainNav *trn = static_cast<TerrainNav *>(self->obj);
        commsT *ct = static_cast<commsT *>(msg->obj);
        if(NULL!=trn && NULL!=ct){
            trn->measUpdate(&ct->mt, ct->parameter);
        }
    }
}

void commst_motion_update(wtnav_t *self, wcommst_t *msg)
{
    if(NULL!=self && NULL!=msg){
        TerrainNav *trn = static_cast<TerrainNav *>(self->obj);
        commsT *ct = static_cast<commsT *>(msg->obj);
        if(NULL!=trn && NULL!=ct){
            trn->motionUpdate(&ct->pt);
        }
    }
}

void commst_estimate_pose(wtnav_t *self, wcommst_t *msg, const int type)
{
    if(NULL!=self && NULL!=msg){
        TerrainNav *trn = static_cast<TerrainNav *>(self->obj);
        commsT *ct = static_cast<commsT *>(msg->obj);
        if(NULL!=trn && NULL!=ct){
            trn->estimatePose(&ct->pt,type);
        }
    }
}

void commst_initialize(wtnav_t *self, wcommst_t *msg)
{
    if(NULL!=self && NULL!=msg){

        TerrainNav *trn = static_cast<TerrainNav *>(self->obj);
        commsT *ct = static_cast<commsT *>(msg->obj);

        if(NULL!=ct && NULL!=trn){

            int errors=0;
            int BUF_SIZE=512;
            char mapname[BUF_SIZE];
            char cfgname[BUF_SIZE];
            char particlename[BUF_SIZE];
            memset(mapname, 0, BUF_SIZE);
            memset(cfgname, 0, BUF_SIZE);
            memset(particlename, 0, BUF_SIZE);

            char* mapPath = getenv("TRN_MAPFILES");
            char* cfgPath = getenv("TRN_DATAFILES");
            char* logPath = getenv("TRN_LOGFILES");

            fprintf(stderr, "ENV: maps:%s, cfgs:%s, and logs:%s\n", mapPath?mapPath:"", cfgPath?cfgPath:"", logPath?logPath:"");

            char dotSlash[] = "./";

            if(mapPath == NULL) {
                mapPath = dotSlash;
            }
            if(cfgPath == NULL) {
                cfgPath = dotSlash;
            }

            if(ct->mapname[0]=='/'){
                snprintf(mapname, BUF_SIZE, "%s", ct->mapname);
            }else{
                snprintf(mapname, BUF_SIZE, "%s/%s", mapPath, ct->mapname);
            }
            if(ct->cfgname[0]=='/'){
                snprintf(cfgname, BUF_SIZE, "%s", ct->cfgname);
            }else{
                snprintf(cfgname, BUF_SIZE, "%s/%s", cfgPath, ct->mapname);
            }
            if(ct->particlename[0]=='/'){
                snprintf(particlename, BUF_SIZE, "%s", ct->cfgname);
            }else{
                snprintf(particlename, BUF_SIZE, "%s/%s", cfgPath, ct->particlename);
            }

            // Let's see if these files exist right now as
            // this will save headaaches later
            //
            if (0 != access(mapname, F_OK))
            {
                fprintf(stderr,"%s : map %s not found\n",__FUNCTION__,mapname);
                errors++;
            }

            if (0 != access(cfgname, F_OK))
            {
                fprintf(stderr,"%s : cfg %s not found\n",__FUNCTION__, cfgname);
                errors++;
            }

            if (0 != access(particlename, F_OK))
            {
                fprintf(stderr,"%s : particles %s not found\n",__FUNCTION__, particlename);
                errors++;
            }

            int mtype = ct->parameter / 10;
            int ftype = ct->parameter % 10;

            if(errors==0){
                fprintf(stderr,"%s : allocating new wtnav TRN\n",__FUNCTION__);
                TerrainNav *trn_new = new TerrainNav(mapname, cfgname, particlename, ftype, mtype, ct->logname);
                if(NULL!=trn_new){
                   fprintf(stderr,"%s : OK replacing wtnav trn[%p] trn->obj[%p] w/ trn_new[%p]\n",__FUNCTION__,self,self->obj,trn_new);

                    delete static_cast<TerrainNav *>(self->obj);
                    self->obj = trn_new;
                }
            }else{
                fprintf(stderr,"%s : errors, aborting TRN init\n",__FUNCTION__);

            }

        }
    }
}

static void s_wposet_to_opose(poseT *dest, pt_cdata_t *src)
{
    if(NULL!=dest && NULL!=src){
        dest->x = src->x;
        dest->y = src->x;
        dest->z = src->x;
        dest->vx = src->vx;
        dest->vy = src->vy;
        dest->vz = src->vz;
        dest->ve = src->ve;
        dest->vw_x = src->vw_x;
        dest->vw_y = src->vw_y;
        dest->vw_z = src->vw_z;
        dest->vn_x = src->vn_x;
        dest->vn_y = src->vn_y;
        dest->vn_z = src->vn_z;
        dest->wx = src->wx;
        dest->wy = src->wy;
        dest->wz = src->wz;
        dest->ax = src->ax;
        dest->ay = src->ay;
        dest->az = src->az;
        dest->phi = src->phi;
        dest->theta = src->theta;
        dest->psi = src->psi;
        dest->psi_berg = src->psi_berg;
        dest->psi_dot_berg = src->psi_dot_berg;
        dest->time = src->time;
        dest->dvlValid = src->dvlValid;
        dest->gpsValid = src->gpsValid;
        dest->bottomLock = src->bottomLock;
        memcpy(dest->covariance,src->covariance,N_COVAR*sizeof(double));
    }
}

static void s_wmeast_to_omeas(measT *dest, mt_cdata_t *src)
{
    if(NULL!=dest && NULL!=src){
        dest->time = src->time;
        dest->dataType = src->dataType;
        dest->phi = src->phi;
        dest->theta = src->theta;
        dest->psi = src->psi;
        dest->x = src->x;
        dest->y = src->x;
        dest->z = src->x;
        dest->numMeas = src->numMeas;
        dest->ping_number = src->ping_number;

        size_t double_sz = dest->numMeas * sizeof(double);
        size_t int_sz = dest->numMeas * sizeof(int);
        size_t bool_sz = dest->numMeas * sizeof(bool);

        dest->covariance = new double[dest->numMeas];
        memcpy(dest->covariance,src->covariance,double_sz);

        dest->ranges = new double[dest->numMeas];
        memcpy(dest->ranges,src->ranges,double_sz);

        dest->crossTrack = new double[dest->numMeas];
        memcpy(dest->crossTrack,src->crossTrack,double_sz);

        dest->alongTrack = new double[dest->numMeas];
        memcpy(dest->alongTrack,src->alongTrack,double_sz);

        dest->altitudes = new double[dest->numMeas];
        memcpy(dest->altitudes,src->altitudes,double_sz);

        dest->alphas = new double[dest->numMeas];
        memcpy(dest->alphas,src->alphas,double_sz);

        dest->measStatus = new bool[dest->numMeas];
        memcpy(dest->measStatus,src->measStatus,bool_sz);

        dest->beamNums = new int[dest->numMeas];
        memcpy(dest->beamNums,src->beamNums,int_sz);

    }
}


wposet_t *wposet_dnew()
{
    wposet_t *m = (wposet_t *)malloc(sizeof(*m));
    if(NULL!=m){
        memset(m,0,sizeof(*m));
        poseT *obj = new poseT();
        m->obj = obj;
    }

    return m;
}

wposet_t *wposet_cnew(poseT *pt)
{
    wposet_t *m  = (wposet_t *)malloc(sizeof(*m));
    if(NULL!=m){
        memset(m,0,sizeof(*m));
        poseT *pnew = new poseT();
        *pnew = *pt;
        m->obj=pnew;
    }

    return m;
}

void wposet_destroy(wposet_t *self)
{
    if (NULL!=self){
        delete static_cast<poseT *>(self->obj);
        free(self);
    }
}

void poset_show(poseT *obj, bool verbose, int indent)
{
    if (NULL != obj) {
        fprintf(stderr,"%*s[self     %10p]\n",indent,(indent>0?" ":""), obj);
        fprintf(stderr,"%*s[time     %10lf]\n",indent,(indent>0?" ":""), obj->time);
        fprintf(stderr,"%*s[dvlVal   %10c]\n",indent,(indent>0?" ":""), obj->dvlValid?'Y':'N');
        fprintf(stderr,"%*s[gpsVal   %10c]\n",indent,(indent>0?" ":""), obj->gpsValid?'Y':'N');
        fprintf(stderr,"%*s[botLock  %10c]\n",indent,(indent>0?" ":""), obj->bottomLock?'Y':'N');
        fprintf(stderr,"%*s[ph,th,ps %10lf,%lf,%lf]\n",indent,(indent>0?" ":""), obj->phi,obj->theta,obj->psi);
        fprintf(stderr,"%*s[xyz      %10lf,%lf,%lf]\n",indent,(indent>0?" ":""), obj->x,obj->y,obj->z);
        fprintf(stderr,"%*s[v*       %10lf,%lf,%lf]\n",indent,(indent>0?" ":""), obj->vx,obj->vy,obj->vz);
        fprintf(stderr,"%*s[vw*      %10lf,%lf,%lf]\n",indent,(indent>0?" ":""), obj->vw_x,obj->vw_y,obj->vw_z);
        fprintf(stderr,"%*s[vn*      %10lf,%lf,%lf]\n",indent,(indent>0?" ":""), obj->vn_x,obj->vn_y,obj->vn_z);
        fprintf(stderr,"%*s[w*       %10lf,%lf,%lf]\n",indent,(indent>0?" ":""), obj->wx,obj->wy,obj->wz);
        fprintf(stderr,"%*s[a*       %10lf,%lf,%lf]\n",indent,(indent>0?" ":""), obj->ax,obj->ay,obj->az);
        fprintf(stderr,"%*s[psib     %10lf]\n",indent,(indent>0?" ":""), obj->psi_berg);
        fprintf(stderr,"%*s[psi.b    %10lf]\n",indent,(indent>0?" ":""), obj->psi_dot_berg);
        fprintf(stderr,"%*s[cov[0:2] %10lf,%lf,%lf]\n",indent,(indent>0?" ":""), obj->covariance[0], obj->covariance[1], obj->covariance[2]);
    }
}
void wposet_show(wposet_t *self, bool verbose, int indent)
{
    if (NULL != self) {
        poseT *obj = static_cast<poseT *>(self->obj);
        poset_show(obj,verbose,indent);
    }
}

int wposet_cdata_to_pose(wposet_t **dest, pt_cdata_t *src)
{
    int retval=-1;
    if(NULL!=dest && NULL!=src){
        wposet_t *pose = (wposet_t *)(*dest);
        if(NULL==pose){
            pose=wposet_dnew();
            *dest=pose;
        }
        if(NULL!=pose){
            poseT *obj = static_cast<poseT *>(pose->obj);
            obj->time=src->time;
            obj->x = src->x;
            obj->y = src->y;
            obj->z = src->z;
            obj->phi=src->phi;
            obj->theta=src->theta;
            obj->psi=src->psi;
            obj->gpsValid=src->gpsValid;
            obj->bottomLock=src->bottomLock;
            obj->dvlValid=src->dvlValid;

            obj->vx=src->vx;
            obj->vy=src->vy;
            obj->vz=src->vz;
            obj->ve=src->ve;
            obj->vw_x=src->vw_x;
            obj->vw_y=src->vw_y;
            obj->vw_z=src->vw_z;
            obj->vn_x=src->vn_x;
            obj->vn_y=src->vn_y;
            obj->vn_z=src->vn_z;
            obj->wx=src->wx;
            obj->wy=src->wy;
            obj->wz=src->wz;
            obj->ax=src->ax;
            obj->ay=src->ay;
            obj->az=src->az;
            obj->psi_berg=src->psi_berg;
            obj->psi_dot_berg=src->psi_dot_berg;
            int i=0;
            for(i=0;i<N_COVAR;i++)
                obj->covariance[i]=src->covariance[i];
          retval=0;

        }
    }
    return retval;
}

int wposet_pose_to_cdata(pt_cdata_t **dest, wposet_t *src)
{
    int retval=-1;
//    fprintf(stderr,"%s:%d dest[%p] src[%p]\n",__FUNCTION__,__LINE__,dest,src);
    if(NULL!=dest && NULL!=src){
        pt_cdata_t *cdata = (pt_cdata_t *)(*dest);
        if(NULL==cdata){
            cdata=(pt_cdata_t *)calloc(1,sizeof(*cdata));
            *dest=cdata;
        }
//        fprintf(stderr,"%s:%d cdata[%p] src[%p]\n",__FUNCTION__,__LINE__,cdata,src);
        if(NULL!=cdata){
            poseT *obj = static_cast<poseT *>(src->obj);
//            fprintf(stderr,"%s:%d cdata[%p] obj[%p]\n",__FUNCTION__,__LINE__,cdata,obj);
            if(NULL!=obj){
            cdata->time=obj->time;
            cdata->x = obj->x;
            cdata->y = obj->y;
            cdata->z = obj->z;
            cdata->phi=obj->phi;
            cdata->theta=obj->theta;
            cdata->psi=obj->psi;
            cdata->gpsValid=obj->gpsValid;
            cdata->bottomLock=obj->bottomLock;
            cdata->dvlValid=obj->dvlValid;

            cdata->vx=obj->vx;
            cdata->vy=obj->vy;
            cdata->vz=obj->vz;
            cdata->ve=obj->ve;
            cdata->vw_x=obj->vw_x;
            cdata->vw_y=obj->vw_y;
            cdata->vw_z=obj->vw_z;
            cdata->vn_x=obj->vn_x;
            cdata->vn_y=obj->vn_y;
            cdata->vn_z=obj->vn_z;
            cdata->wx=obj->wx;
            cdata->wy=obj->wy;
            cdata->wz=obj->wz;
            cdata->ax=obj->ax;
            cdata->ay=obj->ay;
            cdata->az=obj->az;
            cdata->psi_berg=obj->psi_berg;
            cdata->psi_dot_berg=obj->psi_dot_berg;
            int i=0;
            for(i=0;i<N_COVAR;i++)
                cdata->covariance[i]=obj->covariance[i];
            }
            retval=sizeof(*cdata);

        }
    }
    return retval;
}

int wposet_mb1_to_pose(wposet_t **dest, mb1_t *src, long int utmZone)
{
    int retval=-1;
    if(NULL!=dest && NULL!=src){
        wposet_t *pose = (wposet_t *)(*dest);
        if(NULL==pose){
            pose=wposet_dnew();
            *dest=pose;
        }
        if(NULL!=pose){
            poseT *obj = static_cast<poseT *>(pose->obj);
            obj->time = src->ts;
            NavUtils::geoToUtm( Math::degToRad(src->lat),
                               Math::degToRad(src->lon),
                               utmZone, &(obj->x), &(obj->y));
            obj->z = src->depth;
            obj->phi=0.0;
            obj->theta=0.0;
            obj->psi=src->hdg;
            obj->gpsValid=(obj->z<2?true:false);
            obj->bottomLock=true;
            obj->dvlValid=true;
            // TRN can't intialize if vx == 0
            obj->vx = 0.01;
            obj->vy = 0.;
            obj->vz = 0.;
            // wx not required; can use these (how determined?)
            // obj->wx = -3.332e-002;
            // obj->wy = -9.155e-003;
            // obj->wz = -3.076e-002;
            obj->wx = 0.;
            obj->wy = 0.;
            obj->wz = 0.;

            retval=0;
        }

    }
    return retval;
}

int wposet_msg_to_pose(wposet_t **dest, char *src)
{
    int retval=-1;
//    fprintf(stderr,"%s:%d d[%p] s[%p]\n",__FUNCTION__,__LINE__,dest,src);
    if(NULL!=dest && NULL!=src){
        wposet_t *pose = (wposet_t *)(*dest);
        if(NULL==pose){
            pose=wposet_dnew();
            *dest=pose;
        }
//        fprintf(stderr,"%s:%d pose[%p] s[%p]\n",__FUNCTION__,__LINE__,pose,src);
        if(NULL!=pose){
            commsT *ct = new commsT();
            ct->unserialize(src);
            poseT *obj = static_cast<poseT *>(pose->obj);
//            fprintf(stderr,"%s:%d ct[%p] obj[%p] dest[%p]\n",__FUNCTION__,__LINE__,ct,obj,dest);
            *obj = ct->pt;
            delete ct;
            retval=0;
        }
    }
    return retval;
}

int  wposet_serialize(char **pdest, wposet_t *src, int len)
{
    int retval=0;
    if(NULL!=pdest && NULL!=src && len>0){
        char *obuf = *pdest;
        if(NULL==obuf){
            obuf = (char *)calloc(1,sizeof(len));
            *pdest = obuf;
        }
        poseT *obj = static_cast<poseT *>(src->obj);
        retval = obj->serialize(obuf,len);
    }
    return retval;
}

int  wposet_unserialize(wposet_t **pdest, char *src, int len)
{
    int retval=0;
    if(NULL!=pdest && NULL!=src && len>0){
        wposet_t *dest = *pdest;
        if(NULL==dest){
            dest = wposet_dnew();
            *pdest = dest;
        }
        poseT *obj = static_cast<poseT *>(dest->obj);
        retval = obj->unserialize(src,len);
    }
    return retval;
}

wmeast_t *wmeast_dnew()
{
    wmeast_t *m  = (wmeast_t *)malloc(sizeof(*m));
    if(NULL!=m){
        memset(m,0,sizeof(*m));
        measT *obj = new measT();
        m->obj = obj;
    }

    return m;
}

wmeast_t *wmeast_cnew(measT *mt)
{
    wmeast_t *m  = (wmeast_t *)malloc(sizeof(*m));
    if(NULL!=m){
        memset(m,0,sizeof(*m));
        measT *mnew = new measT();
        *mnew = *mt;
        m->obj=mnew;
    }

    return m;
}

wmeast_t *wmeast_new(unsigned int size)
{
    wmeast_t *m = wmeast_dnew();
    if(NULL!=m){
        measT *obj = static_cast<measT *>(m->obj);
        if(NULL!=obj){
            obj->time=0.0;
            obj->dataType   = TRN_SENSOR_MB; //2 denotes MB data
            obj->phi=0;
            obj->theta=0;
            obj->psi=0;
            obj->x=0;
            obj->y=0;
            obj->z=0;
            obj->ping_number= 0;
            obj->numMeas    = size;
            obj->crossTrack = new double[size];
            obj->covariance = new double[size];
            obj->ranges     = new double[size];
            obj->alphas     = new double[size];
            obj->alongTrack = new double[size];
            obj->altitudes  = new double[size];
            obj->measStatus = new bool[size];
            obj->beamNums   = new int[size];
            memset(obj->crossTrack,0,size*sizeof(double));
            memset(obj->covariance,0,size*sizeof(double));
            memset(obj->ranges,0,size*sizeof(double));
            memset(obj->alphas,0,size*sizeof(double));
            memset(obj->alongTrack,0,size*sizeof(double));
            memset(obj->altitudes,0,size*sizeof(double));
            memset(obj->measStatus,0,size*sizeof(bool));
            memset(obj->beamNums,0,size*sizeof(int));
        }
    }
    return m;
}

void wmeast_destroy(wmeast_t *self)
{
    if (NULL!=self){
    	delete static_cast<measT *>(self->obj);
    	free(self);
    }
}
void meast_show(measT *obj, bool verbose, int indent)
{
    if (NULL != obj) {
        fprintf(stderr,"%*s[self     %10p]\n",indent,(indent>0?" ":""), obj);
        fprintf(stderr,"%*s[time     %10lf]\n",indent,(indent>0?" ":""), obj->time);
        fprintf(stderr,"%*s[dataType %10d]\n",indent,(indent>0?" ":""), obj->dataType);
        fprintf(stderr,"%*s[phi      %10lf]\n",indent,(indent>0?" ":""), obj->phi);
        fprintf(stderr,"%*s[theta    %10lf]\n",indent,(indent>0?" ":""), obj->theta);
        fprintf(stderr,"%*s[psi      %10lf]\n",indent,(indent>0?" ":""), obj->psi);
        fprintf(stderr,"%*s[x        %10lf]\n",indent,(indent>0?" ":""), obj->x);
        fprintf(stderr,"%*s[y        %10lf]\n",indent,(indent>0?" ":""), obj->y);
        fprintf(stderr,"%*s[z        %10lf]\n",indent,(indent>0?" ":""), obj->z);
        fprintf(stderr,"%*s[numMeas  %10d]\n",indent,(indent>0?" ":""), obj->numMeas);
        fprintf(stderr,"%*s[ping     %10d]\n",indent,(indent>0?" ":""), obj->ping_number);
    }
}

void wmeast_show(wmeast_t *self, bool verbose, int indent)
{
    if (NULL != self) {
        measT *obj = static_cast<measT *>(self->obj);
        meast_show(obj,verbose,indent);
    }
}

static double vnorm( double v[] )
{
    double vnorm2 = 0.0;
    int i=0;
    for(i=0; i<3; i++) vnorm2 += pow(v[i],2.0);
    return( sqrt( vnorm2 ) );
}

int wmeast_cdata_to_meas(wmeast_t **dest, mt_cdata_t *src)
{
    int retval=-1;
    if(NULL!=dest && NULL!=src){
        wmeast_t *meas = (wmeast_t *)(*dest);
        if(NULL==meas){
            meas=wmeast_dnew();
            *dest=meas;
        }
        if(NULL!=meas){
            measT *obj = static_cast<measT *>(meas->obj);
            obj->time=src->time;
            obj->x = src->x;
            obj->y = src->y;
            obj->z = src->z;
            obj->phi=src->phi;
            obj->theta=src->theta;
            obj->psi=src->psi;
            obj->numMeas=src->numMeas;
            int i=0;
            for(i=0;i<obj->numMeas;i++){
                obj->covariance[i]=src->covariance[i];
                obj->ranges[i]=src->ranges[i];
                obj->crossTrack[i]=src->crossTrack[i];
                obj->alongTrack[i]=src->alongTrack[i];
                obj->altitudes[i]=src->altitudes[i];
                obj->alphas[i]=src->alphas[i];
                obj->measStatus[i]=src->measStatus[i];
                obj->beamNums[i]=src->beamNums[i];
            }
            retval=0;

        }
    }
    return retval;
}

int wmeast_meas_to_cdata(mt_cdata_t **dest, wmeast_t *src)
{
    int retval=-1;
    //    fprintf(stderr,"%s:%d dest[%p] src[%p]\n",__FUNCTION__,__LINE__,dest,src);
    if(NULL!=dest && NULL!=src){
        mt_cdata_t *cdata = (mt_cdata_t *)(*dest);
        if(NULL==cdata){
            measT *obj = static_cast<measT *>(src->obj);
            size_t vsize = obj->numMeas*(6*sizeof(double)+sizeof(bool)+sizeof(int));
            cdata=(mt_cdata_t *)calloc(1,sizeof(*cdata)+vsize);
            *dest=cdata;
        }
        //        fprintf(stderr,"%s:%d cdata[%p] src[%p]\n",__FUNCTION__,__LINE__,cdata,src);
        if(NULL!=cdata){
            measT *obj = static_cast<measT *>(src->obj);
            //            fprintf(stderr,"%s:%d cdata[%p] obj[%p]\n",__FUNCTION__,__LINE__,cdata,obj);
            if(NULL!=obj){
                cdata->time=obj->time;
                cdata->dataType=obj->dataType;
                cdata->ping_number = obj->ping_number;
                cdata->x = obj->x;
                cdata->y = obj->y;
                cdata->z = obj->z;
                cdata->phi=obj->phi;
                cdata->theta=obj->theta;
                cdata->psi=obj->psi;
                cdata->numMeas=obj->numMeas;
                int i=0;
                for(i=0;i<obj->numMeas;i++){
                    cdata->covariance[i]=obj->covariance[i];
                    cdata->ranges[i]=obj->ranges[i];
                    cdata->crossTrack[i]=obj->crossTrack[i];
                    cdata->alongTrack[i]=obj->alongTrack[i];
                    cdata->altitudes[i]=obj->altitudes[i];
                    cdata->alphas[i]=obj->alphas[i];
                    cdata->measStatus[i]=obj->measStatus[i];
                    cdata->beamNums[i]=obj->beamNums[i];
                }
            }
            retval=0;
        }
    }
    return retval;
}

int wmeast_mb1_to_meas(wmeast_t **dest, mb1_t *src, long int utmZone)
{
    int retval=-1;
    if(NULL!=dest && NULL!=src){
        wmeast_t *meas = (wmeast_t *)(*dest);
        if(NULL==meas){
            meas=wmeast_new(src->nbeams);
            *dest=meas;
        }
        if(NULL!=meas){
            int i=0;
            measT *obj = static_cast<measT *>(meas->obj);
            obj->time = src->ts;
            obj->ping_number = src->ping_number;
            obj->dataType=2;
            obj->z=src->depth;
            NavUtils::geoToUtm( Math::degToRad(src->lat),
                               Math::degToRad(src->lon),
                               utmZone, &(obj->x), &(obj->y));

            for(i=0;i<obj->numMeas;i++){
                // TODO: fill in measT from ping...
                obj->beamNums[i] = src->beams[i].beam_num;
                obj->alongTrack[i] = src->beams[i].rhox;
                obj->crossTrack[i] = src->beams[i].rhoy;
                obj->altitudes[i]  = src->beams[i].rhoz;
                double rho[3] = {obj->alongTrack[i], obj->crossTrack[i], obj->altitudes[i]};
                double rhoNorm = vnorm( rho );
                obj->ranges[i] = rhoNorm;
                // [rhoNorm = sqrt(ax^2 + ay^2 + az^2)] (i.e. range magnitude)
                obj->measStatus[i] = rhoNorm>1?true:false;
//                obj->covariance[i] = 0.0;
//                obj->alphas[i]     = 0.0;
            }

            retval=0;
        }

    }
    return retval;
}
int wmeast_msg_to_meas(wmeast_t **dest, char *src)
{
    int retval=-1;
    //    fprintf(stderr,"%s:%d d[%p] s[%p]\n",__FUNCTION__,__LINE__,dest,src);
    if(NULL!=dest && NULL!=src){
        wmeast_t *meas = (wmeast_t *)(*dest);
        if(NULL==meas){
            meas=wmeast_dnew();
            *dest=meas;
        }
        //        fprintf(stderr,"%s:%d pose[%p] s[%p]\n",__FUNCTION__,__LINE__,pose,src);
        if(NULL!=meas){
            commsT *ct = new commsT();
            retval=ct->unserialize(src);
            measT *obj = static_cast<measT *>(meas->obj);
            //            fprintf(stderr,"%s:%d ct[%p] obj[%p] dest[%p]\n",__FUNCTION__,__LINE__,ct,obj,dest);
            *obj = ct->mt;
            delete ct;
//            retval=0;
        }
    }
    return retval;
}


int  wmeast_serialize(char **pdest, wmeast_t *src, int len)
{
    int retval=0;
    if(NULL!=pdest && NULL!=src && len>0){
        char *obuf = *pdest;
        if(NULL==obuf){
            obuf = (char *)calloc(1,sizeof(len));
            *pdest = obuf;
        }
        measT *obj = static_cast<measT *>(src->obj);
        retval = obj->serialize(obuf,len);
    }
    return retval;
}

int  wmeast_unserialize(wmeast_t **pdest, char *src, int len)
{
    int retval=0;
    if(NULL!=pdest && NULL!=src && len>0){
        wmeast_t *dest = *pdest;
        if(NULL==dest){
            dest = wmeast_dnew();
            *pdest = dest;
        }
        measT *obj = static_cast<measT *>(dest->obj);
        retval = obj->unserialize(src,len);
    }
    return retval;
}

int wmeast_get_nmeas(wmeast_t *self)
{
    int retval=-1;
    if(NULL!=self){
        measT *mt = static_cast<measT *>(self->obj);
        retval = mt->numMeas;
    }
    return retval;
}

int32_t trnw_meas_msg(char **dest, wmeast_t *src, int msg_type, int param)
{
    int retval=-1;
    char *msg = (char *)malloc(TRN_MSG_SIZE);
    if(NULL!=msg){
        memset(msg,0,TRN_MSG_SIZE);
        measT *mtsrc = static_cast<measT *>(src->obj);

        commsT *ct = new commsT(msg_type, param, *mtsrc);
        ct->serialize(msg,TRN_MSG_SIZE);
        *dest=msg;
        delete ct;
        retval=TRN_MSG_SIZE;
    }
    return retval;
}

int32_t trnw_pose_msg(char **dest, wposet_t *src, char msg_type)
{
    int retval=-1;
    char *msg = (char *)malloc(TRN_MSG_SIZE);
    if(NULL!=msg){
        memset(msg,0,TRN_MSG_SIZE);
        poseT *ptsrc = static_cast<poseT *>(src->obj);
        commsT ct(msg_type, *ptsrc);
        ct.serialize(msg,TRN_MSG_SIZE);
        *dest=msg;
        retval=TRN_MSG_SIZE;
    }
    return retval;
}

int32_t trnw_init_msg(char **dest, trn_config_t *cfg)
{
    int retval=-1;
    char *msg = (char *)malloc(TRN_MSG_SIZE);
    if(NULL!=msg){
        memset(msg,0,TRN_MSG_SIZE);

        // filter type and map type encoded in single integer
        int param = cfg->map_type*10+cfg->filter_type;
        commsT ct(TRN_INIT, param, cfg->map_file,cfg->cfg_file,cfg->particles_file,cfg->log_dir);

        ct.serialize(msg,TRN_MSG_SIZE);
        *dest=msg;
        retval=TRN_MSG_SIZE;
    }
    return retval;
}

int32_t trnw_vdr_msg(char **dest, char msg_type, int param, float vdr)
{
    int retval=-1;
    char *msg = (char *)malloc(TRN_MSG_SIZE);
    if(NULL!=msg){
        memset(msg,0,TRN_MSG_SIZE);
        commsT ct(msg_type,param,vdr);
        ct.serialize(msg,TRN_MSG_SIZE);
        *dest=msg;
        retval=TRN_MSG_SIZE;
    }
    return retval;
}

int32_t trnw_ptype_msg(char **dest, char msg_type, int param)
{
    int retval=-1;
    char *msg = (char *)malloc(TRN_MSG_SIZE);
    if(NULL!=msg){
        memset(msg,0,TRN_MSG_SIZE);
        commsT ct(msg_type,param);
        ct.serialize(msg,TRN_MSG_SIZE);
        *dest=msg;
        retval=TRN_MSG_SIZE;
    }
    return retval;
}

int32_t trnw_type_msg(char **dest, char msg_type)
{
    int32_t retval=-1;
    char *msg = (char *)malloc(TRN_MSG_SIZE);
    if(NULL!=msg){
        memset(msg,0,TRN_MSG_SIZE);
        commsT ct(msg_type);
        ct.serialize(msg,TRN_MSG_SIZE);
        *dest=msg;
        retval=TRN_MSG_SIZE;
    }
    return retval;
}

int32_t trnw_acknack_msg(char **pdest,char ACK_NACK)
{
    int retval=-1;
    char *msg = (char *)malloc(TRN_MSG_SIZE);
    if(NULL!=msg && NULL!=pdest){
        memset(msg,0,TRN_MSG_SIZE);
        commsT ct(ACK_NACK);
        ct.serialize(msg,TRN_MSG_SIZE);
        *pdest=msg;
        retval=TRN_MSG_SIZE;
    }
    return retval;
}

int32_t trnw_ack_msg(char **pdest)
{
    return trnw_type_msg(pdest,TRN_ACK);
}

int32_t trnw_nack_msg(char **pdest)
{
    return trnw_type_msg(pdest,TRN_NACK);
}

int32_t trnw_triplet_msg(char **dest, char msg_type, d_triplet_t *src)
{
    int retval=-1;
    char *msg = (char *)malloc(TRN_MSG_SIZE);
    if(NULL!=msg){
        memset(msg,0,TRN_MSG_SIZE);
        commsT ct(msg_type,src->x, src->y, src->z);
        ct.serialize(msg,TRN_MSG_SIZE);
        *dest=msg;
        retval=TRN_MSG_SIZE;
    }
    return retval;
}

int32_t trnw_reinit_offset_msg(char **dest, char msg_type, bool lowInfoTransition,
                            double offset_x, double offset_y, double offset_z )
{
    int retval=-1;
    char *msg = (char *)malloc(TRN_MSG_SIZE);
    if(NULL!=msg){
        memset(msg,0,TRN_MSG_SIZE);
        commsT ct(msg_type, (lowInfoTransition ? 1 : 0),offset_x, offset_y, offset_z);
        ct.serialize(msg,TRN_MSG_SIZE);
        *dest=msg;
        retval=TRN_MSG_SIZE;
    }
    return retval;
}

int32_t trnw_reinit_box_msg(char **dest, char msg_type, bool lowInfoTransition,
                            double offset_x, double offset_y, double offset_z,
                            double sdev_x, double sdev_y, double sdev_z )
{
    int retval=-1;
    char *msg = (char *)malloc(TRN_MSG_SIZE);
    if(NULL!=msg){
        memset(msg,0,TRN_MSG_SIZE);
        commsT ct(msg_type, (lowInfoTransition ? 1 : 0),offset_x, offset_y, offset_z, sdev_x, sdev_y, sdev_z);
        ct.serialize(msg,TRN_MSG_SIZE);
        *dest=msg;
        retval=TRN_MSG_SIZE;
    }
    return retval;
}

void trnw_msg_show(char *msg, bool verbose, int indent)
{
    if (NULL != msg) {
        commsT *obj = new commsT();
        obj->unserialize(msg,TRN_MSG_SIZE);
        fprintf(stderr,"%*s[self     %10p]\n",indent,(indent>0?" ":""), obj);
        fprintf(stderr,"%*s[type     %10c]\n",indent,(indent>0?" ":""), obj->msg_type);
        fprintf(stderr,"%*s[param    %10d]\n",indent,(indent>0?" ":""), obj->parameter);
        fprintf(stderr,"%*s[vdr      %10lf]\n",indent,(indent>0?" ":""), obj->vdr);
        fprintf(stderr,"%*s[map      %10s]\n",indent,(indent>0?" ":""), obj->mapname);
        fprintf(stderr,"%*s[cfg      %10s]\n",indent,(indent>0?" ":""), obj->cfgname);
        fprintf(stderr,"%*s[particle %10s]\n",indent,(indent>0?" ":""), obj->particlename);
        fprintf(stderr,"%*s[logdir   %10s]\n",indent,(indent>0?" ":""), obj->logname);
        fprintf(stderr,"%*s[est_nav  %10lf,%lf,%lf]\n",indent,(indent>0?" ":""), obj->est_nav_ofs.x, obj->est_nav_ofs.y, obj->est_nav_ofs.z);
        fprintf(stderr,"%*s[xyz_def  %10lf,%lf,%lf]\n",indent,(indent>0?" ":""), obj->xyz_sdev.x, obj->xyz_sdev.y, obj->xyz_sdev.z);
        if(verbose){
            fprintf(stderr,"%*s[pt       %10p]\n",indent,(indent>0?" ":""), &obj->pt);
            poset_show(&obj->pt,verbose,indent);

            fprintf(stderr,"%*s[mt       %10p]\n",indent,(indent>0?" ":""), &obj->mt);
            meast_show(&obj->mt,verbose,indent);
        }

        delete obj;
    }
}
// End function trnw_msg_show

#define TRNW_TRN_HOST_DFL "localhost"
#define TRNW_TRN_PORT_DFL 27000
#define TRNW_TRN_LOGDIR_DFL "."
#define TRNW_TRN_FILTER_TYPE_DFL TRN_FILT_PARTICLE
#define TRNW_TRN_MAP_TYPE_DFL TRN_MAP_BO

trn_config_t *trncfg_dnew()
{
    trn_config_t *instance = (trn_config_t *)calloc(1,sizeof(*instance));

    if(NULL!=instance){

        instance->trn_host=strdup(TRNW_TRN_HOST_DFL);
        instance->trn_port=TRNW_TRN_PORT_DFL;
        instance->map_file=NULL;
        instance->cfg_file=NULL;
        instance->particles_file=NULL;
        instance->log_dir=strdup(TRNW_TRN_LOGDIR_DFL);
        instance->filter_type=TRNW_TRN_FILTER_TYPE_DFL;
        instance->map_type=TRNW_TRN_MAP_TYPE_DFL;
        instance->sensor_type=TRN_SENSOR_MB;
    }
    return instance;
}

trn_config_t *trncfg_new(char *host,int port,
                         long int utm_zone, int map_type,
                         int sensor_type,
                         int filter_type,
                         int filter_grade,
                         int filter_reinit,
                         int mod_weight,
                         char *map_file,
                         char *cfg_file,
                         char *particles_file,
                         char *log_dir,
                         trnw_oflags_t oflags,
                         double max_northing_cov,
                         double max_northing_err,
                         double max_easting_cov,
                         double max_easting_err
                         )
{
    trn_config_t *instance = (trn_config_t *)calloc(1,sizeof(*instance));

    if(NULL!=instance){
        instance->trn_host=(NULL!=host?strdup(host):NULL);
        instance->trn_port=port;
        instance->map_file=(NULL!=map_file?strdup(map_file):strdup("map.dfl"));
        instance->cfg_file=(NULL!=cfg_file?strdup(cfg_file):strdup("cfg.dfl"));
        instance->particles_file=(NULL!=particles_file?strdup(particles_file):strdup("particles.dfl"));
        instance->log_dir=(NULL!=log_dir?strdup(log_dir):strdup("."));
        instance->sensor_type=sensor_type;
        instance->filter_type=filter_type;
        instance->map_type=map_type;
        instance->utm_zone=utm_zone;
        instance->mod_weight=mod_weight;
        instance->filter_reinit=filter_reinit;
        instance->filter_grade=filter_grade;
        instance->oflags=oflags;
        instance->max_northing_cov = max_northing_cov;
        instance->max_northing_err = max_northing_err;
        instance->max_easting_cov = max_easting_cov;
        instance->max_easting_err = max_easting_err;
   }
    return instance;
}

void trncfg_destroy(trn_config_t **pself)
{
    if(NULL!=pself){

        trn_config_t *self = (trn_config_t *)(*pself);
        if(NULL!=self){

            if(NULL!=self->trn_host)
                free(self->trn_host);
            if(NULL!=self->map_file)
                free(self->map_file);
            if(NULL!=self->particles_file)
                free(self->particles_file);
            if(NULL!=self->cfg_file)
                free(self->cfg_file);
            if(NULL!=self->log_dir)
                free(self->log_dir);

            free(self);
            *pself=NULL;
        }
    }

}
void trncfg_show(trn_config_t *obj, bool verbose, int indent)
{
    if (NULL != obj) {
        fprintf(stderr,"%*s[self      %10p]\n",indent,(indent>0?" ":""), obj);
        fprintf(stderr,"%*s[host      %10s]\n",indent,(indent>0?" ":""), obj->trn_host);
        fprintf(stderr,"%*s[port      %10d]\n",indent,(indent>0?" ":""), obj->trn_port);
        fprintf(stderr,"%*s[utm       %10ld]\n",indent,(indent>0?" ":""), obj->utm_zone);
        fprintf(stderr,"%*s[mtype     %10d]\n",indent,(indent>0?" ":""), obj->map_type);
        fprintf(stderr,"%*s[ftype     %10d]\n",indent,(indent>0?" ":""), obj->filter_type);
        fprintf(stderr,"%*s[map_file  %10s]\n",indent,(indent>0?" ":""), obj->map_file);
        fprintf(stderr,"%*s[cfg_file  %10s]\n",indent,(indent>0?" ":""), obj->cfg_file);
        fprintf(stderr,"%*s[part_file %10s]\n",indent,(indent>0?" ":""), obj->particles_file);
        fprintf(stderr,"%*s[log_dir   %10s]\n",indent,(indent>0?" ":""), obj->log_dir);
        fprintf(stderr,"%*s[maxNcov   %10s%.3lf]\n",indent,(indent>0?" ":""), "",obj->max_northing_cov);
        fprintf(stderr,"%*s[maxNerr   %10s%.3lf]\n",indent,(indent>0?" ":""), "",obj->max_northing_err);
        fprintf(stderr,"%*s[maxEcov   %10s%.3lf]\n",indent,(indent>0?" ":""), "",obj->max_easting_cov);
        fprintf(stderr,"%*s[maxEerr   %10s%.3lf]\n",indent,(indent>0?" ":""), "",obj->max_easting_err);
    }
}

int trnw_utm_to_geo(double northing, double easting, long utmZone,
                       double *lat_deg, double *lon_deg)
{
//    return NavUtils::utmToGeo(northing,easting,utmZone,lat_rad,lon_rad);
    double lat_rad=0.0;
    double lon_rad=0.0;

  int retval=NavUtils::utmToGeo(northing,easting,utmZone,&lat_rad,&lon_rad);
    *lat_deg=Math::radToDeg(lat_rad);
    *lon_deg=Math::radToDeg(lon_rad);
    return retval;
}
// End function trnw_utmToGeo

int trnw_geo_to_utm(double lat_deg, double lon_deg, long int utmZone, double *northing, double *easting)
{
    return  NavUtils::geoToUtm( Math::degToRad(lat_deg),
                               Math::degToRad(lon_deg),
                               utmZone, northing, easting);
}
// End function trnw_utmToGeo

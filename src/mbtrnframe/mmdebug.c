///
/// @file mmdebug.c
/// @authors k. Headley
/// @date 01 jan 2018
 
/// General purpose debug macros
/// run-time configurable per-module debug output
/// compile-time switches to remove from code

/// @sa mconfig.h for application-specific customizations

/////////////////////////
// Terms of use 
/////////////////////////
/*
Copyright Information

Copyright 2000-2018 MBARI
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

#include "mmdebug.h"
#include "mlist.h"

/////////////////////////
// Macros
/////////////////////////

// These macros should only be defined for 
// application main files rather than general C files
/*
/// @def PRODUCT
/// @brief header software product name
#define PRODUCT "MBRT"

/// @def COPYRIGHT
/// @brief header software copyright info
#define COPYRIGHT "Copyright 2002-2013 MBARI Monterey Bay Aquarium Research Institute, all rights reserved."
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

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////


char *mmd_ch_names[MM_CHANNEL_COUNT]={
    "none.mm",
    "trace.mm",
    "debug.mm",
    "warn.mm",
    "err.mm"
};

mmd_module_config_t module_config_table[MM_MODULE_COUNT]={
    {MOD_MFRAME,"mframe",MM_CHANNEL_COUNT,(MM_WARN|MM_ERR),mmd_ch_names},
    {MOD_MERR,"merr",    MM_CHANNEL_COUNT,(MM_WARN|MM_ERR),mmd_ch_names},
    {MOD_MBBUF,"mbbuf",  MM_CHANNEL_COUNT,(MM_WARN|MM_ERR),mmd_ch_names},
    {MOD_MCBUF,"mcbuf",  MM_CHANNEL_COUNT,(MM_WARN|MM_ERR),mmd_ch_names},
    {MOD_MFILE,"mfile",  MM_CHANNEL_COUNT,(MM_WARN|MM_ERR),mmd_ch_names},
    {MOD_MLIST,"mlist",  MM_CHANNEL_COUNT,(MM_WARN|MM_ERR),mmd_ch_names},
    {MOD_MLOG,"mlog",    MM_CHANNEL_COUNT,(MM_WARN|MM_ERR),mmd_ch_names},
    {MOD_MMEM,"mmem",    MM_CHANNEL_COUNT,(MM_WARN|MM_ERR),mmd_ch_names},
    {MOD_MQUEUE,"mqueue",MM_CHANNEL_COUNT,(MM_WARN|MM_ERR),mmd_ch_names},
    {MOD_MSOCK,"msock",  MM_CHANNEL_COUNT,(MM_WARN|MM_ERR),mmd_ch_names},
    {MOD_MSTATS,"mstats",MM_CHANNEL_COUNT,(MM_WARN|MM_ERR),mmd_ch_names},
    {MOD_MTIME,"mtime",  MM_CHANNEL_COUNT,(MM_WARN|MM_ERR),mmd_ch_names},
    {MOD_MTHREAD,"mthread",MM_CHANNEL_COUNT,(MM_WARN|MM_ERR),mmd_ch_names},
};

static mlist_t *mmd_module_list=NULL;
/////////////////////////
// Function Definitions
/////////////////////////

mmd_module_config_t *mmd_module_config_copy(mmd_module_config_t *module)
{
    mmd_module_config_t *instance=NULL;
    if(NULL!=module){
     	instance= (mmd_module_config_t *)malloc(sizeof(mmd_module_config_t));
        if(NULL!=instance){
            memset(instance,0,sizeof(mmd_module_config_t));
            instance->id=module->id;
            instance->channel_count=module->channel_count;
            instance->en_mask=module->en_mask;
            
            if(NULL!=module->name){
                instance->name=strdup(module->name);
            }else{
                instance->name=NULL;
            }
            
            if(NULL!=module->channel_names){
                uint32_t i=0;
                uint32_t cn_size=module->channel_count*sizeof(char *);
                
                instance->channel_names=(char **)malloc(module->channel_count*sizeof(char *));
                memset(instance->channel_names,0,cn_size);
                
                for(i=0;i<module->channel_count;i++){
                    if(NULL!=module->channel_names[i]){
                        instance->channel_names[i]=strdup(module->channel_names[i]);
                    }
                }
            }else{
                instance->channel_names=NULL;
            }
        }
    }
    return instance;
}

void mmd_module_config_destroy(mmd_module_config_t **pself)
{
    if(NULL!=pself){
        mmd_module_config_t *self = (mmd_module_config_t *)(*pself);

        if(NULL!=self){
            if(NULL!=self->name){
                free(self->name);
            }
            if(NULL!=self->channel_names){
                uint32_t i=0;
                for(i=0;i<self->channel_count;i++){
                    if(NULL!=self->channel_names[i]){
                        free(self->channel_names[i]);
                    }
                }
                free(self->channel_names);
            }
            free(self);
            *pself=NULL;
        }
    }
}
void mmd_config_show(mmd_module_config_t *self, bool verbose, uint16_t indent)
{
    if (NULL != self) {
        int wkey = 15;
        int wval = 15;

        fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""), wkey, "self", wval, self);
        fprintf(stderr,"%*s%*s %*u\n",indent,(indent>0?" ":""), wkey, "id", wval, self->id);
        fprintf(stderr,"%*s%*s %*s\n",indent,(indent>0?" ":""), wkey, "name", wval, self->name);
        fprintf(stderr,"%*s%*s %*u\n",indent,(indent>0?" ":""), wkey, "channel_count", wval, self->channel_count);
        fprintf(stderr,"%*s%*s %*s%04x\n",indent,(indent>0?" ":""), wkey, "en_mask", wval-4, "", self->en_mask);
        fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""), wkey, "channel_names", wval, self->channel_names);
        if(verbose){
            uint32_t i=0;
            for(i=0;i<self->channel_count;i++){
                fprintf(stderr,"%*s%*s[%u] %*s %0X\n",indent+2,(indent>0?" ":""), wkey-3, "ch", i, wval, self->channel_names[i], CHMSK(i));
            }
        }
    }
}

void mmd_module_config_free(void *pvoid)
{
    if(NULL!=pvoid){
        mmd_module_config_t *ptype =  (mmd_module_config_t *)pvoid;
        mmd_module_config_destroy(&ptype);
    }
}
/// @fn void mmd_initialize()
/// @brief initialize per module debug settings.
/// @return none
void mmd_initialize()
{
     int i=0;
    if(NULL==mmd_module_list){
        mmd_module_list=mlist_new();
    }

    for(i=0;i<MM_MODULE_COUNT;i++){
        mmd_module_configure(&module_config_table[i]);
    }
    mlist_autofree(mmd_module_list,mmd_module_config_free);
}
// End function mmd_initialize

void mmd_release()
{
	if(NULL!=mmd_module_list){
		mlist_destroy(&mmd_module_list);
    }
}
// End function mmd_release


mmd_module_config_t *s_lookup_module(mmd_module_id_t id)
{
    mmd_module_config_t *module=(mmd_module_config_t *)mlist_first(mmd_module_list);
    while(NULL!=module){
        if(module->id==id){
            break;
        }
        module=(mmd_module_config_t *)mlist_next(mmd_module_list);
    }
    return module;
}

int mmd_module_configure(mmd_module_config_t *module)
{
    int retval=-1;
    if(NULL!=module){
	     mmd_module_config_t *modcpy=NULL;
        if(NULL==mmd_module_list){
            mmd_initialize();
        }

        modcpy=mmd_module_config_copy(module);
        if(NULL!=modcpy){
            mmd_module_config_t *current=s_lookup_module(module->id);
//            mmd_config_show(current,true,5);
            if(NULL!=current){
                mlist_remove(mmd_module_list,current);
            }
//            mmd_config_show(modcpy,true,5);
            mlist_add(mmd_module_list,modcpy);
//            fprintf(stderr,"list len[%lu]\r\n",mlist_size(mmd_module_list));
           retval=0;
        }
        
    }
    
    return retval;
}

//int mmd_channel_configure(mmd_module_id_t id, mmd_channel_id_t ch_id, char *name,  int enable)
//{
//    int retval=-1;
//    return retval;
//}

int mmd_channel_set(mmd_module_id_t id, mmd_en_mask_t mask)
{
    int retval=-1;
    mmd_module_config_t *mod=s_lookup_module(id);
    if(NULL!=mod){
        mod->en_mask = mask;
        retval=0;
    }
    return retval;
}

int mmd_channel_en(mmd_module_id_t id, mmd_en_mask_t mask)
{
    int retval=-1;
    mmd_module_config_t *mod=s_lookup_module(id);
    if(NULL!=mod){
        mod->en_mask |= mask;
        retval=0;
    }
    return retval;
}

int mmd_channel_dis(mmd_module_id_t id, mmd_en_mask_t mask)
{
    int retval=-1;
    mmd_module_config_t *mod=s_lookup_module(id);
    if(NULL!=mod){
        mod->en_mask &= ~(mask);
        retval=0;
    }
    return retval;
}

bool mmd_channel_isset(mmd_module_id_t id, uint32_t mask)
{
    mmd_module_config_t *mod=s_lookup_module(id);
    return ( (NULL!=mod) && ((mod->en_mask&mask)!=0) );
}

mmd_en_mask_t mmd_get_enmask(mmd_module_id_t id, mmd_en_mask_t *dest)
{
     mmd_module_config_t *mod=s_lookup_module(id);
    return ( (NULL!=mod) ? mod->en_mask : 0xFFFFFFFF );

}
// End function mmd_get

char *mmd_module_name(mmd_module_id_t id)
{
    char *retval=NULL;
    mmd_module_config_t *mod=s_lookup_module(id);
    if(NULL!=mod){
        retval=mod->name;
    }
   return retval;
}

char *mmd_channel_name(mmd_module_id_t id, mmd_channel_id_t ch_id)
{
    char *retval=NULL;
    mmd_module_config_t *mod=s_lookup_module(id);
    if(NULL!=mod && NULL!=mod->channel_names && ch_id<mod->channel_count){
        retval=mod->channel_names[ch_id];
    }
    return retval;
}

uint32_t mmd_channel_count(mmd_module_id_t id)
{
    uint32_t retval=-1;
    mmd_module_config_t *mod=s_lookup_module(id);
    if(NULL!=mod){
        retval = mod->channel_count - MM_CHANNEL_COUNT;
    }
    return retval;

}

#ifdef WITH_MMDEBUG_TEST

// Once the module and channel IDs are defined,
// applications may directly use the macros defined in mmdebug.h
// i.e. MMPRINT, MMWRITE, etc. These may be globally globally
// excluded from compilation using -DWITHOUT_MMDEBUG
//
// Some optional steps may be taken to simplify the macros
// (eliminate module ID arg) and define groups for
// inclusion/exclusion from compilation, e.g. -DWITH_MMT_OPTIONAL
// -DWITHOUT_MMT_REQUIRED (see below)

// [option] : define some channel masks for convenience
#define MMD_TEST_M1_MASK (M1C1|M1C2)
#define MMD_TEST_M2_MASK (M2C1|M2C2|M2C3)

// [option] : wrap macros with module ID included (see below)

// [option] : compilation enable/disable (see below)
// - define per-module macro disables, e.g. verbose, production, debug compilation options
// - note ifdef/ifndef WITH_/WITHOUT_ for default behavior
//   i.e. production is defined, verbose and debug not defined

// Typically, the DEBUG group would include
// messages that you only compile in temporarily
#ifdef WITH_MMT_DEBUG
#define MTD_TRACE(ch) MMTRACE(MOD1,ch)
#define MTD_MSG(ch,msg) MMMSG(MOD1,ch,msg)
#define MTD_PRINT(ch,fmt,...) MMPRINT(MOD1,ch,fmt,##__VA_ARGS__)
#define MTD_WRITE(ch,fmt,...) MMWRITE(MOD1,ch,fmt,##__VA_ARGS__)
#else
#define MTD_TRACE(ch)
#define MTD_MSG(ch,msg)
#define MTD_PRINT(ch,fmt,...)
#define MTD_WRITE(ch,fmt,...)
#endif

// Typically, an OPTIONAL group would include
// messages that you may want to compile out
// (or could may disable at run time)
#ifdef WITH_MMT_OPTIONAL
#define MTO_TRACE(ch) MMTRACE(MOD1,ch)
#define MTO_MSG(ch,msg) MMMSG(MOD1,ch,msg)
#define MTO_PRINT(ch,fmt,...) MMPRINT(MOD1,ch,fmt,##__VA_ARGS__)
#define MTO_WRITE(ch,fmt,...) MMWRITE(MOD1,ch,fmt,##__VA_ARGS__)
#else
#define MTO_TRACE(ch)
#define MTO_MSG(ch,msg)
#define MTO_PRINT(ch,fmt,...)
#define MTO_WRITE(ch,fmt,...)
#endif

// Typically, a REQUIRED group would include
// messages you'd never compile out (shown here for illustration)
#ifndef WITHOUT_MMT_REQUIRED
#define MT_MSG(ch,msg) MMMSG(MOD1,ch,msg)
#define MT_PRINT(ch,fmt,...) MMPRINT(MOD1,ch,fmt,##__VA_ARGS__)
#define MT_WRITE(ch,fmt,...) MMWRITE(MOD1,ch,fmt,##__VA_ARGS__)
#define MT_ERR(ch,fmt,...)   MEPRINT(MOD1,ch,fmt,##__VA_ARGS__)
#define MT_WARN(ch,fmt,...)  MWPRINT(MOD1,ch,fmt,##__VA_ARGS__)
#else
#define MT_MSG(ch,msg)
#define MT_PRINT(ch,fmt,...)
#define MT_WRITE(ch,fmt,...)
#define MT_ERR(ch,fmt,...)
#define MT_WARN(ch,fmt,...)
#endif

/// Define module configuration and data in module source
/// for testing, we'll define two modules

/// @enum app_module_ids
/// @brief application module IDs
/// [note : starting above reserved mframe module IDs]
typedef enum{
    MOD1=MM_MODULE_COUNT,
    MOD2,
    APP_MODULE_COUNT
}app_module_ids;

/// @enum mod1_channel_id
/// @brief test module channel IDs
/// [note : starting above reserved mframe channel IDs]
typedef enum{
    ID_M1C1=MM_CHANNEL_COUNT,
    ID_M1C2,
    MOD1_CH_COUNT
}mod1_channel_id;

/// @enum mod1_channel_mask
/// @brief test module channel masks
typedef enum{
    M1C1= (1<<ID_M1C1),
    M1C2= (1<<ID_M1C2)
}mod1_channel_mask;

/// @enum mod2_channel_id
/// @brief test module channel IDs
/// [note : starting above reserved mframe channel IDs]
typedef enum{
    ID_M2C1= MM_CHANNEL_COUNT,
    ID_M2C2,
    ID_M2C3,
    MOD2_CH_COUNT
}mod2_channel_id;

/// @enum mod2_channel_mask
/// @brief test module channel masks
typedef enum{
    M2C1= (1<<ID_M2C1),
    M2C2= (1<<ID_M2C2),
    M2C3= (1<<ID_M2C3)
}mod2_channel_mask;

/// @var char *mmd_test_m1_ch_names[MOD1_CH_COUNT]
/// @brief test module channel names
char *mmd_test_m1_ch_names[MOD1_CH_COUNT]={
    "trace.m1",
    "debug.m1",
    "warn.m1",
    "err.m1",
    "M1C1",
    "M1C2"
};

/// @var char *mmd_test_m2_ch_names[MOD2_CH_COUNT]
/// @brief test module channel names
char *mmd_test_m2_ch_names[MOD2_CH_COUNT]={
    "trace.m2",
    "debug.m2",
    "warn.m2",
    "err.m2",
    "M2C1",
    "M2C2",
    "M3C3"
};

/// @var md_module_config_t mmd_test_app_defaults[]
/// @brief test module configuration defaults
mmd_module_config_t mmd_test_app_defaults[]={
    {MOD1,"Mod-1",MOD1_CH_COUNT,((MM_ERR|MM_WARN)|MMD_TEST_M1_MASK),mmd_test_m1_ch_names},
    
    {MOD2,"Mod-2",MOD2_CH_COUNT,((MM_ERR|MM_WARN)|MMD_TEST_M2_MASK),mmd_test_m2_ch_names}
};

/// @fn int mmd_test()
/// @brief debug unit test. throws assertions on failure
/// @return 0 on success, -1 otherwise
int mmd_test()
{
    int retval=-1;
#ifdef WITH_MMDEBUG
    fprintf(stderr,"\n\ncompiled with -DWITH_MMDEBUG\r\n");
#else
    fprintf(stderr,"\n\ncompiled without -DWITH_MMDEBUG\r\n");
#endif
#ifdef WITH_MMDEBUG_TEST
    fprintf(stderr,"\n\ncompiled with -DWITH_MMDEBUG_TEST\r\n");
#else
    fprintf(stderr,"\n\ncompiled without -DWITH_MMDEBUG_TEST\r\n");
#endif

    // library init
    mmd_initialize();
    
    // configure module channel from static defaults
    // [data is copied, no change to original]
    mmd_module_configure(&mmd_test_app_defaults[0]);
    mmd_module_configure(&mmd_test_app_defaults[1]);
    
    // print messages on enabled channels
    fprintf(stderr,"WARN, ERR, M1[1:2], M2[1:3]] ENABLED\r\n");
    int i=0;
    for(i=0;i<MOD1_CH_COUNT;i++){
        MMPRINT(MOD1,(CHMSK(i)),"m1 ch[%d/%s]",i,mmd_channel_name(MOD1,i));
    }
    for(i=0;i<MOD2_CH_COUNT;i++){
        MMPRINT(MOD2,(CHMSK(i)),"m2 ch[%d/%s]",i,mmd_channel_name(MOD2,i));
    }

    // disable some channels
    fprintf(stderr,"M1C1, M2C1 DISABLED\r\n");
    mmd_channel_dis(MOD1,M1C1);
    mmd_channel_dis(MOD2,M2C1);

    // print messages on enabled channels
    for(i=0;i<MOD1_CH_COUNT;i++){
        MMPRINT(MOD1,(CHMSK(i)),"m1 ch[%d/%s]",i,mmd_channel_name(MOD1,i));
    }
    for(i=0;i<MOD2_CH_COUNT;i++){
        MMPRINT(MOD2,(CHMSK(i)),"m2 ch[%d/%s]",i,mmd_channel_name(MOD2,i));
    }
    
#ifdef WITHOUT_MMT_REQUIRED
    fprintf(stderr,"\n\ncompiled with -DWITHOUT_MMT_REQUIRED\r\n");
#else
    fprintf(stderr,"\n\ncompiled without -DWITHOUT_MMT_REQUIRED\r\n");
#endif

    // required group
    // typical use
    fprintf(stderr,"M1C2 msg, print, write(2) (REQUIRED)\r\n");
    MT_MSG(M1C2,  "m1c2 msg     (req)");
    MT_PRINT(M1C2,"m1c2 print   (req)");
    MT_WRITE(M1C2,"m1c2 write 1 (req) ");
    MT_WRITE(M1C2,"m1c2 write 2 (req)\r\n");

#ifdef WITH_MMT_OPTIONAL
    fprintf(stderr,"\n\ncompiled with -DWITH_MMT_OPTIONAL\r\n");
#else
    fprintf(stderr,"\n\ncompiled without -DWITH_MMT_OPTIONAL\r\n");
#endif

    // optional group
    fprintf(stderr,"M1C2 trace, msg, print, write(2) (OPTIONAL)\r\n");
    MTO_TRACE(M1C2);
    MTO_MSG(M1C2,  "m1c2 msg     (opt)");
    MTO_PRINT(M1C2,"m1c2 print   (opt)");
    MTO_WRITE(M1C2,"m1c2 write 1 (opt) ");
    MTO_WRITE(M1C2,"m1c2 write 2 (opt)\r\n");
    
    // debug group
#ifdef WITH_MMT_DEBUG
    fprintf(stderr,"\n\ncompiled with -DWITH_MMT_DEBUG\r\n");
#else
    fprintf(stderr,"\n\ncompiled without -DWITH_MMT_DEBUG\r\n");
#endif
  fprintf(stderr,"M1C2 trace, msg, print, write(2) (DEBUG)\r\n");
    MTD_TRACE(M1C2);
    MTD_MSG(M1C2,  "m1c2 msg     (debug)");
    MTD_PRINT(M1C2,"m1c2 print   (debug)");
    MTD_WRITE(M1C2,"m1c2 write 1 (debug) ");
    MTD_WRITE(M1C2,"m1c2 write 2 (debug)\r\n");

    fprintf(stderr,"M2C2 warn, err (REQUIRED)\r\n");
    MT_WARN(M2C2,"m2c2 warn (req)");
    MT_ERR(M2C2,"m2c2 err (req)");
    
    fprintf(stderr,"PMPRINT...\r\n");
    PMPRINT(MOD1,MM_ALL,(stderr,"MOD1,MM_ALL %08X\r\n",mmd_get_enmask(MOD1,NULL)));
    PMPRINT(MOD2,MM_ALL,(stderr,"MOD1,MM_ALL %08X\r\n",mmd_get_enmask(MOD1,NULL)));

    retval = 0;
    
    return retval;
}
// End function mmd_test

#endif //WITH_MMDEBUG_TEST


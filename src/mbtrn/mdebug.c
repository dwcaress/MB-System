///
/// @file mdebug.c
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

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
// assert only used in test()
#include <assert.h>

#include "mdebug.h"

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

md_level_t MD_ALL_LEVEL = MDL_ERROR;
md_module_id_t MDI_ALL = 0;

static md_level_t md_module_settings[MD_MAX_MODULES+1] = {0};
static const char *md_module_names[MD_MAX_MODULES+1]   = {0};
static bool md_initialized    = false;
static const char*MD_ALL_NAME = "ALL";

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn void mdb_initialize()
/// @brief initialize per module debug settings.
/// @return none
void mdb_initialize()
{
    for(int i=0;i<=MD_MAX_MODULES;i++){
        md_module_settings[i]=MDL_NONE;
        md_module_names[i]=NULL;
    }
    md_module_settings[MDI_ALL]=MD_ALL_LEVEL;
    md_module_names[MDI_ALL]=MD_ALL_NAME;
    md_initialized=true;
}
// End function mdb_initialize


/// @fn md_level_t mdb_get(md_module_id_t id, md_level_t * dest)
/// @brief get debug setting for module.
/// @param[in] id module ID
/// @param[in] dest optionally store level in data
/// @return module debug level on success, MDL_UNSET otherwise
md_level_t mdb_get(md_module_id_t id, md_level_t *dest)
{
    md_level_t retval=MDL_UNSET;
    if (id <= MD_MAX_MODULES) {
        retval=md_module_settings[id];
        if (NULL != dest) {
            *dest=md_module_settings[id];
        }
    }else{
        // error, out of range
        retval=MDL_UNSET;
    }
    return retval;
   
}
// End function mdb_get


/// @fn int mdb_set(md_module_id_t id, md_level_t level)
/// @brief set module debug level.
/// @param[in] id module ID
/// @param[in] level debug level
/// @return 0 on success, -1 otherwise
int mdb_set(md_module_id_t id, md_level_t level)
{
    int retval=0;
    // don't lazy initialize, since it would
    // overwrite user init done before first set
    
    if (id <= MD_MAX_MODULES) {
        md_module_settings[id]=level;
        if (id==MDI_ALL) {
            MD_ALL_LEVEL=level;
        }
    }else{
        // error, out of range
        retval=-1;
    }
    return retval;
}
// End function mdb_set


/// @fn int mdb_set_name(md_module_id_t id, const char * name)
/// @brief set name for module (channel).
/// @param[in] id module ID
/// @param[in] name module name
/// @return 0 on success, -1 otherwise
int mdb_set_name(md_module_id_t id, const char* name)
{
    int retval=0;
    if (id <= MD_MAX_MODULES) {
        md_module_names[id] = name;
    }else{
    	// error
        retval=-1;
    }
    return retval;
}
// End function mdb_set_name


/// @fn const char * mdb_get_name(md_module_id_t id)
/// @param[in] id module ID
/// @return name string on success, NULL otherwise
const char *mdb_get_name(md_module_id_t id)
{
    const char *retval=NULL;
    if (id <= MD_MAX_MODULES) {
        retval = md_module_names[id];
    }
    return retval;
}
// End function mdb_get_name


/// @fn const char * mdb_level2str(md_level_t level)
/// @brief get string mnemonic for debug level.
/// @param[in] level debug level/
/// @return name string on success, NULL otherwise
const char *mdb_level2str(md_level_t level)
{
    switch ((int)level) {

        case MDL_UNSET:
            return "UNSET";
            break;
        case MDL_NONE:
            return "NONE";
            break;
        case MDL_FATAL:
            return "FATAL";
            break;
        case MDL_ERROR:
            return "ERROR";
            break;
        case MDL_WARN:
            return "WARN";
            break;
        case MDL_INFO:
            return "INFO";
            break;
        case MDL_DEBUG:
            return "DEBUG";
            break;
            
        default:
            break;
    }
    return NULL;
}
// End function mdb_level2str


/// @fn int mdb_test()
/// @brief debug unit test. throws assertions on failure
/// @return 0 on success, -1 otherwise
int mdb_test()
{
    fprintf(stderr,"Debug TRACE [%s]\n",MD_TRACE_S);
    MTRACE();
    fprintf(stderr,"Debug LEVEL [%s]\n",MD_LEVEL_S);
    
    fprintf(stderr,"m*, mv* (DEBUG) macros should NOT follow:\n");
    MDEBUG("test mdebug [%s]\n","MD_DEBUG_LEVEL>=MDL_DEBUG");
    MVDEBUG("test mvdebug [%s]\n","MD_DEBUG_LEVEL>=MDL_DEBUG");
    
    fprintf(stderr,"m*, mv* (INFO) macros should NOT follow:\n");
    MINFO("test minfo [%s]\n","MD_DEBUG_LEVEL>=MDL_INFO");
    MVINFO("test mvinfo [%s]\n","MD_DEBUG_LEVEL>=MDL_INFO");
    
    fprintf(stderr,"m*, mv* (WARN) macros should follow:\n");
    MWARN("test mwarn [%s]\n","MD_DEBUG_LEVEL>=MDL_WARN");
    MVWARN("test mvwarn [%s]\n","MD_DEBUG_LEVEL>=MDL_WARN");
    
    fprintf(stderr,"m*, mv* (ERROR) macros should follow:\n");
    MERROR("test merror [%s]\n","MD_DEBUG_LEVEL>=MDL_ERROR");
    MVERROR("test mverror [%s]\n","MD_DEBUG_LEVEL>=MDL_ERROR");
    
    fprintf(stderr,"m*, mv* (FATAL) macros should follow:\n");
    MFATAL("test mfatal [%s]\n","MD_DEBUG_LEVEL>=MDL_FATAL");
    MVFATAL("test mvfatal [%s]\n","MD_DEBUG_LEVEL>=MDL_FATAL");
    fprintf(stderr,"\n");
    
    md_level_t x;
    int stat=-1;
    
    // test initilization
    // [sets all to MDL_NONE]
    mdb_initialize();

    // test get (initialized value)
    mdb_get(1, &x);
    assert(x==MDL_NONE);
    
    // test out-of-bounds request
    x = mdb_get(MD_MAX_MODULES+1, NULL);
    assert(x==MDL_UNSET);
    
	// test NULL dest request
    x = mdb_get(MD_MAX_MODULES, NULL);
    assert(x==MDL_NONE);

    // test MD_ALL_LEVEL dest request
    x = mdb_get(MDI_ALL, NULL);
    assert(x==MD_ALL_LEVEL);
    

    // test valid set/get
    stat=mdb_set(1,MDL_ERROR);
    assert(stat==0);
    mdb_get(1, &x);
    assert(x==MDL_ERROR);
    
	// test module group macros
    // messages should print (or not) depending on
    // settings for MDI_ALL and modules M and N
    // Macros print more verbose of module and MDI_ALL settings
    // If MDI_ALL==MDL_UNSET, the module setting is used
    int M=1,N=2, X=M;
    mdb_set(MDI_ALL,MDL_ERROR);
    mdb_set_name(1,"MOD1");
    mdb_set_name(2,"MOD2");
    // test set/get name
    assert(strcmp(mdb_get_name(M),"MOD1")==0);
    assert(mdb_get_name(MD_MAX_MODULES)==NULL);
    
    mdb_set(M,MDL_DEBUG);
    fprintf(stderr,"macro tests - %s:[%s] %s:[%s]\n",mdb_get_name(X),mdb_level2str(mdb_get(X,NULL)),mdb_get_name(MDI_ALL), mdb_level2str(mdb_get(MDI_ALL,NULL)));
    MMFATAL(M,"macro FATAL test: [%s]\n", "OK");
    MMERROR(M,"macro ERROR test: [%s]\n", "OK");
    MMWARN(M,"macro WARN test: [%s]\n", "OK");
    MMINFO(M,"macro INFO test: [%s]\n", "OK");
    MMDEBUG(M,"macro DEBUG test: [%s]\n", "OK");

    mdb_set(N,MDL_WARN);
    X=N;
    fprintf(stderr,"macro tests - %s:[%s] %s:[%s]\n",mdb_get_name(X),mdb_level2str(mdb_get(X,NULL)),mdb_get_name(MDI_ALL), mdb_level2str(mdb_get(MDI_ALL,NULL)));
    MMFATAL(N,"macro FATAL test: [%s]\n", "OK");
    MMERROR(N,"macro ERROR test: [%s]\n", "OK");
    MMWARN(N,"macro WARN test: [%s]\n", "OK");
    MMINFO(N,"macro INFO test: [%s]\n", "OK");
    MMDEBUG(N,"macro DEBUG test: [%s]\n", "OK");

    mdb_set(MDI_ALL,MDL_DEBUG);
    mdb_set(M,MDL_ERROR);
    X=M;
    fprintf(stderr,"macro tests - %s:[%s] %s:[%s]\n",mdb_get_name(X),mdb_level2str(mdb_get(X,NULL)),mdb_get_name(MDI_ALL), mdb_level2str(mdb_get(MDI_ALL,NULL)));
    MMFATAL(M,"macro FATAL test: [%s]\n", "OK");
    MMERROR(M,"macro ERROR test: [%s]\n", "OK");
    MMWARN(M,"macro WARN test: [%s]\n", "OK");
    MMINFO(M,"macro INFO test: [%s]\n", "OK");
    MMDEBUG(M,"macro DEBUG test: [%s]\n", "OK");

    mdb_set(MDI_ALL,MDL_UNSET);
    mdb_set(M,MDL_WARN);
    fprintf(stderr,"macro tests - %s:[%s] %s:[%s]\n",mdb_get_name(X),mdb_level2str(mdb_get(X,NULL)),mdb_get_name(MDI_ALL), mdb_level2str(mdb_get(MDI_ALL,NULL)));
    MMFATAL(M,"macro FATAL test: [%s]\n", "OK");
    MMERROR(M,"macro ERROR test: [%s]\n", "OK");
    MMWARN(M,"macro WARN test: [%s]\n", "OK");
    MMINFO(M,"macro INFO test: [%s]\n", "OK");
    MMDEBUG(M,"macro DEBUG test: [%s]\n", "OK");

    mdb_set(N,MDL_INFO);
    X=N;
    fprintf(stderr,"macro tests - %s:[%s] %s:[%s]\n",mdb_get_name(X),mdb_level2str(mdb_get(X,NULL)),mdb_get_name(MDI_ALL), mdb_level2str(mdb_get(MDI_ALL,NULL)));
    MMFATAL(N,"macro FATAL test: [%s]\n", "OK");
    MMERROR(N,"macro ERROR test: [%s]\n", "OK");
    MMWARN(N,"macro WARN test: [%s]\n", "OK");
    MMINFO(N,"macro INFO test: [%s]\n", "OK");
    MMDEBUG(N,"macro DEBUG test: [%s]\n", "OK");

    return 0;
}
// End function mdb_test


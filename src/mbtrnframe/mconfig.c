///
/// @file mconfig.c
/// @authors k. Headley
/// @date 01 jan 2018
 
/// Project module initialization and configuration

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

#include "mconfig.h"

/////////////////////////
// Macros
/////////////////////////

// These macros should only be defined for 
// application main files rather than general C files
/*
/// @def PRODUCT
/// @brief header software product name
#define PRODUCT "MFRAME"

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

/// @var char *r7kr_ch_names[R7KR_CH_COUNT]
/// @brief module channel names
char *r7kr_ch_names[R7KR_CH_COUNT]={
    "trace.mbtrnpp",
    "debug.mbtrnpp",
    "warn.mbtrnpp",
    "err.mbtrnpp",
    "r7kr.v1",
    "r7kr.v2"
};

/// @var char *r7k_ch_names[R7K_CH_COUNT]
/// @brief module channel names
char *r7k_ch_names[R7K_CH_COUNT]={
    "trace.r7k",
    "debug.r7k",
    "warn.r7k",
    "err.r7k",
    "r7k.v1",
    "r7k.v2",
    "r7k.parser",
    "r7k.drfcon"
};

/// @var char *mb1r_ch_names[MB1R_CH_COUNT]
/// @brief module channel names
char *mb1r_ch_names[MB1R_CH_COUNT]={
    "trace.mbtrnpp",
    "debug.mbtrnpp",
    "warn.mbtrnpp",
    "err.mbtrnpp",
    "mb1r.v1",
    "mb1r.v2"
};

/// @var char *mb1_ch_names[MB1_CH_COUNT]
/// @brief module channel names
char *mb1_ch_names[MB1_CH_COUNT]={
    "trace.mb1",
    "debug.mb1",
    "warn.mb1",
    "err.mb1",
    "mb1.v1",
    "mb1.v2",
    "mb1.parser",
    "mb1.drfcon"
};

/// @var char *s7k_ch_names[S7K_CH_COUNT]
/// @brief module channel names
char *s7k_ch_names[S7K_CH_COUNT]={
    "trace.mbtrn",
    "debug.mbtrn",
    "warn.mbtrn",
    "err.mbtrn",
    "s7k.v1",
    "s7k.v2"
};

/// @var char *f7k_ch_names[F7K_CH_COUNT]
/// @brief module channel names
char *f7k_ch_names[F7K_CH_COUNT]={
    "trace.mbtrn",
    "debug.mbtrn",
    "warn.mbtrn",
    "err.mbtrn",
    "f7k.v1",
    "f7k.v2"
};

/// @var char *trnc_ch_names[TRNC_CH_COUNT]
/// @brief module channel names
char *trnc_ch_names[TRNC_CH_COUNT]={
    "trace.trnc",
    "debug.trnc",
    "warn.trnc",
    "err.trnc",
    "trnc.v1",
    "trnc.v2"
};

/// @var char *emu7k_ch_names[EMU7K_CH_COUNT]
/// @brief module channel names
char *emu7k_ch_names[EMU7K_CH_COUNT]={
    "trace.emu7k",
    "debug.emu7k",
    "warn.emu7k",
    "err.emu7k",
    "emu7k.v1",
    "emu7k.v2",
    "emu7k.v3",
    "emu7k.v4",
    "emu7k.v5"
};

/// @var char *tbinx_ch_names[TBINX_CH_COUNT]
/// @brief module channel names
char *tbinx_ch_names[TBINX_CH_COUNT]={
    "trace.tbinx",
    "debug.tbinx",
    "warn.tbinx",
    "err.tbinx",
    "tbinx.v1",
    "tbinx.v2",
    "tbinx.v3",
    "tbinx.v4"
};

/// @var char *mbtrnpp_ch_names[MBTRNPP_CH_COUNT]
/// @brief module channel names
char *mbtrnpp_ch_names[MBTRNPP_CH_COUNT]={
    "trace.mbtrnpp",
    "debug.mbtrnpp",
    "warn.mbtrnpp",
    "err.mbtrnpp",
    "mbtrnpp.v1",
    "mbtrnpp.v2",
    "mbtrnpp.v3",
    "mbtrnpp.v4"
};

/// @var char *mbtnav_ch_names[MBTNAV_CH_COUNT]
/// @brief module channel names
char *mbtnav_ch_names[MBTNAV_CH_COUNT]={
    "trace.mbtnav",
    "debug.mbtnav",
    "warn.mbtnav",
    "err.mbtnav",
    "mbtnav.v1",
    "mbtnav.v2"
};

/// @var char *netif_ch_names[NETIF_CH_COUNT]
/// @brief module channel names
char *netif_ch_names[NETIF_CHAN_COUNT]={
    "trace.netif",
    "debug.netif",
    "warn.netif",
    "err.netif",
    "netif.v1",
    "netif.v2",
    "netif.v3",
    "netif.v4"
};

static mmd_module_config_t mmd_config_defaults[]={
    {MOD_R7KR,"MOD_R7KR",R7KR_CH_COUNT,((MM_ERR|MM_WARN)|R7KR_V1),r7kr_ch_names},
    {MOD_R7K,"MOD_R7K",R7K_CH_COUNT,((MM_ERR|MM_WARN)|R7K_V1),r7k_ch_names},
    {MOD_MB1R,"MOD_MB1R",MB1R_CH_COUNT,((MM_ERR|MM_WARN)|MB1R_V1),mb1r_ch_names},
    {MOD_MB1,"MOD_MB1",MB1_CH_COUNT,((MM_ERR|MM_WARN)|MB1_V1),mb1_ch_names},
    {MOD_S7K,"MOD_S7K",S7K_CH_COUNT,((MM_ERR|MM_WARN)),s7k_ch_names},
    {MOD_F7K,"MOD_F7K",F7K_CH_COUNT,((MM_ERR|MM_WARN)),f7k_ch_names},
    {MOD_TRNC,"MOD_TRNC",TRNC_CH_COUNT,((MM_ERR|MM_WARN)),trnc_ch_names},
    {MOD_EMU7K,"MOD_EMU7K",EMU7K_CH_COUNT,((MM_ERR|MM_WARN)),emu7k_ch_names},
    {MOD_TBINX,"MOD_TBINX",TBINX_CH_COUNT,((MM_ERR|MM_WARN)|TBINX_V1),tbinx_ch_names},
    {MOD_MBTRNPP,"MOD_MBTRNPP",MBTRNPP_CH_COUNT,((MM_ERR|MM_WARN)|MBTRNPP_V1),mbtrnpp_ch_names},
    {MOD_MBTNAV,"MOD_MBTNAV",MBTNAV_CH_COUNT,((MM_ERR|MM_WARN)|MBTNAV_V1),mbtnav_ch_names},
    {MOD_NETIF,"MOD_NETIF",NETIF_CHAN_COUNT,((MM_ERR|MM_WARN)|NETIF_V1),netif_ch_names}
};

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn void mcfg_init()
/// @brief Application specific module configuration.
/// @return none
int mconf_init(void *pargs, void *prtn)
{
	int retval=-1;
	// User code....
	int i=0;
	int app_modules =(APP_MODULE_COUNT-MM_MODULE_COUNT);
	// call mmdebug init (optional)
	mmd_initialize();
	// then configure additional modules
	for(i=0;i<app_modules;i++){
	    int test=mmd_module_configure(&mmd_config_defaults[i]);
        fprintf(stderr,"%s:%d >>> initializing module[id=%02d] - %10s/%08X [%d]\n",__FUNCTION__,__LINE__,mmd_config_defaults[i].id,mmd_config_defaults[i].name,mmd_get_enmask(mmd_config_defaults[i].id,NULL),test);
    }
    fprintf(stderr,"%s:%d >>> MM_WARN  %08X\n",__FUNCTION__,__LINE__,MM_WARN);
    fprintf(stderr,"%s:%d >>> MM_DEBUG %08X\n",__FUNCTION__,__LINE__,MM_DEBUG);
    fprintf(stderr,"%s:%d >>> MM_ERR   %08X\n",__FUNCTION__,__LINE__,MM_ERR);
    fprintf(stderr,"%s:%d >>> MM_NONE  %08X\n",__FUNCTION__,__LINE__,MM_NONE);
    fprintf(stderr,"%s:%d >>> MM_ALL   %08X\n",__FUNCTION__,__LINE__,MM_ALL);

    return retval;
}
// End function mcfg_init



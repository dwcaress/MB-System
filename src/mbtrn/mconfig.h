///
/// @file mconfig.h
/// @authors k. headley
/// @date 06 nov 2012
 
/// Library configuration
/// set debug parameters for modules in this project
 
/// @sa doxygen-examples.c for more examples of Doxygen markup
 

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

// include guard
#ifndef MCONFIG_H
/// @def MCONFIG_H
/// @brief include guard
#define MCONFIG_H

/////////////////////////
// Includes 
/////////////////////////

#include "mframe.h"

// User Includes
#include "mmdebug.h"
//#include "mbtrn.h"
//#include "r7kc.h"
//#include "stream7k.h"
//#include "frames7k.h"

/////////////////////////
// Type Definitions
/////////////////////////

typedef enum{
    MOD_MBTRN=MM_MODULE_COUNT,
    MOD_R7K,
    MOD_S7K,
    MOD_F7K,
    MOD_TRNC,
    MOD_EMU7K,
    MOD_TBINX,
    MOD_MBTRNPP,
    MOD_R7KR,
    APP_MODULE_COUNT
}app_module_ids;

/// @enum mbtrn_channel_id
/// @brief test module channel IDs
/// [note : starting above reserved mframe channel IDs]
typedef enum{
    ID_MBTRN_V1=MM_CHANNEL_COUNT,
    ID_MBTRN_V2,
    MBTRN_CH_COUNT
}mbtrn_channel_id;

/// @enum mbtrn_channel_mask
/// @brief test module channel masks
typedef enum{
    MBTRN_V1= (1<<ID_MBTRN_V1),
    MBTRN_V2= (1<<ID_MBTRN_V2)
}mbtrn_channel_mask;

/// @enum r7kr_channel_id
/// @brief test module channel IDs
/// [note : starting above reserved mframe channel IDs]
typedef enum{
    ID_R7KR_V1=MM_CHANNEL_COUNT,
    ID_R7KR_V2,
    R7KR_CH_COUNT
}r7kr_channel_id;

/// @enum r7kr_channel_mask
/// @brief test module channel masks
typedef enum{
    R7KR_V1= (1<<ID_R7KR_V1),
    R7KR_V2= (1<<ID_R7KR_V2)
}r7kr_channel_mask;

/// @enum r7k_channel_id
/// @brief test module channel IDs
/// [note : starting above reserved mframe channel IDs]
typedef enum{
    ID_R7K_V1=MM_CHANNEL_COUNT,
    ID_R7K_V2,
    ID_R7K_PARSER,
    ID_R7K_DRFCON,
    R7K_CH_COUNT
}r7k_channel_id;

/// @enum r7k_channel_mask
/// @brief test module channel masks
typedef enum{
    R7K_V1= (1<<ID_R7K_V1),
    R7K_V2= (1<<ID_R7K_V2),
    R7K_PARSER= (1<<ID_R7K_PARSER),
    R7K_DRFCON= (1<<ID_R7K_DRFCON)
}r7k_channel_mask;

/// @enum s7k_channel_id
/// @brief test module channel IDs
/// [note : starting above reserved mframe channel IDs]
typedef enum{
    ID_S7K_V1=MM_CHANNEL_COUNT,
    ID_S7K_V2,
    S7K_CH_COUNT
}s7k_channel_id;

/// @enum s7k_channel_mask
/// @brief test module channel masks
typedef enum{
    S7K_V1= (1<<ID_S7K_V1),
    S7K_V2= (1<<ID_S7K_V2)
}s7k_channel_mask;

/// @enum f7k_channel_id
/// @brief test module channel IDs
/// [note : starting above reserved mframe channel IDs]
typedef enum{
    ID_F7K_V1=MM_CHANNEL_COUNT,
    ID_F7K_V2,
    F7K_CH_COUNT
}f7k_channel_id;

/// @enum f7k_channel_mask
/// @brief test module channel masks
typedef enum{
    F7K_V1= (1<<ID_F7K_V1),
    F7K_V2= (1<<ID_F7K_V2)
}f7k_channel_mask;

/// @enum trnc_channel_id
/// @brief test module channel IDs
/// [note : starting above reserved mframe channel IDs]
typedef enum{
    ID_TRNC_V1=MM_CHANNEL_COUNT,
    ID_TRNC_V2,
    TRNC_CH_COUNT
}trnc_channel_id;

/// @enum trnc_channel_mask
/// @brief test module channel masks
typedef enum{
    TRNC_V1= (1<<ID_TRNC_V1),
    TRNC_V2= (1<<ID_TRNC_V2)
}trnc_channel_mask;

/// @enum emu7k_channel_id
/// @brief test module channel IDs
/// [note : starting above reserved mframe channel IDs]
typedef enum{
    ID_EMU7K_V1=MM_CHANNEL_COUNT,
    ID_EMU7K_V2,
    ID_EMU7K_V3,
    ID_EMU7K_V4,
    ID_EMU7K_V5,
    EMU7K_CH_COUNT
}emu7k_channel_id;

/// @enum emu7k_channel_mask
/// @brief test module channel masks
typedef enum{
    EMU7K_V1= (1<<ID_EMU7K_V1),
    EMU7K_V2= (1<<ID_EMU7K_V2),
    EMU7K_V3= (1<<ID_EMU7K_V3),
    EMU7K_V4= (1<<ID_EMU7K_V4),
    EMU7K_V5= (1<<ID_EMU7K_V5)
}emu7k_channel_mask;

/// @enum tbinx_channel_id
/// @brief test module channel IDs
/// [note : starting above reserved mframe channel IDs]
typedef enum{
    ID_TBINX_V1=MM_CHANNEL_COUNT,
    ID_TBINX_V2,
    ID_TBINX_V3,
    ID_TBINX_V4,
    TBINX_CH_COUNT
}tbinx_channel_id;

/// @enum tbinx_channel_mask
/// @brief test module channel masks
typedef enum{
    TBINX_V1= (1<<ID_TBINX_V1),
    TBINX_V2= (1<<ID_TBINX_V2),
    TBINX_V3= (1<<ID_TBINX_V3),
    TBINX_V4= (1<<ID_TBINX_V4)
}tbinx_channel_mask;

/// @enum mbtrnpp_channel_id
/// @brief test module channel IDs
/// [note : starting above reserved mframe channel IDs]
typedef enum{
    ID_MBTRNPP_V1=MM_CHANNEL_COUNT,
    ID_MBTRNPP_V2,
    ID_MBTRNPP_V3,
    ID_MBTRNPP_V4,
    ID_MBTRNPP_V5,
    ID_MBTRNPP_V6,
    MBTRNPP_CH_COUNT
}mbtrnpp_channel_id;

/// @enum mbtrnpp_channel_mask
/// @brief test module channel masks
typedef enum{
    MBTRNPP_V1= (1<<ID_MBTRNPP_V1),
    MBTRNPP_V2= (1<<ID_MBTRNPP_V2),
    MBTRNPP_V3= (1<<ID_MBTRNPP_V3),
    MBTRNPP_V4= (1<<ID_MBTRNPP_V4),
    MBTRNPP_V5= (1<<ID_MBTRNPP_V5),
    MBTRNPP_V6= (1<<ID_MBTRNPP_V6)
}mbtrnpp_channel_mask;

/////////////////////////
// Macros
/////////////////////////

/// @def MTIME_STOPWATCH_EN
/// @brief enable stopwatch macros
//#ifndef MTIME_STOPWATCH_EN
//#define MTIME_STOPWATCH_EN
//#endif

/// @def MST_STATS_EN
/// @brief enable stats macros
#ifndef MST_STATS_EN
#define MST_STATS_EN
#endif

/// @def MBTRNPP_STAT_PERIOD_SEC
#define MBTRNPP_STAT_PERIOD_SEC ((double)20.0)

/////////////////////////
// Exports
/////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

// exported config functions here

/// @fn int mconf_init(void *pargs, void *prtn)
/// @brief app-specific init (user defined in mconfig.c)
/// @param[in]  pargs arguments
/// @param[out] prtn  return value
/// @return 0 on success, -1 otherwise
int mconf_init(void *pargs, void *prtn);

#ifdef __cplusplus
}
#endif

// include guard
#endif

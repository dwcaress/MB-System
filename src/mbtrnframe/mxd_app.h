#ifndef MXD_APP_H
#define MXD_APP_H

#include "mxdebug.h"
#include "mxdebug-common.h"
typedef enum{
    MBTRNPP = MX_APP_RANGE,
    R7KR,
    R7KR_DEBUG,
    R7KR_ERROR,
    R7KC,
    R7KC_DEBUG,
    R7KC_ERROR,
    R7KC_PARSER,
    R7KC_DRFCON,
    MB1R,
    MB1R_DEBUG,
    MB1R_ERROR,
    NETIF,
    TRNC,
    TRNC_DEBUG,
    TRNC_ERROR,
    MB1IO,
    MB1IO_DEBUG,
    MB1IO_ERROR,
    EMU7K,
    EMU7K_DEBUG,
    EMU7K_ERROR,
    FRAMES7K,
    FRAMES7K_DEBUG,
    FRAMES7K_ERROR,
    MBTNAVC,
    MBTNAVC_DEBUG,
    MBTNAVC_ERROR,
    STREAM7K,
    STREAM7K_DEBUG,
    STREAM7K_ERROR,
    TBINX,
    TBINX_DEBUG,
    TBINX_ERROR,
}mod_id_t;

#endif

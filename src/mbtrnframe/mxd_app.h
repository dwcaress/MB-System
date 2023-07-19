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
    F7K,
    NETIF,
    TRNC,
    TRNC_DEBUG,
    TRNC_ERROR
}mod_id_t;

#endif

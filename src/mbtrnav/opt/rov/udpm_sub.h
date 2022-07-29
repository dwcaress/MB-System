///
/// @file udpm_sub.h
/// @authors k. headley
/// @date 2019-06-21

/// UDP multicast subscriber component

/// @sa doxygen-examples.c for more examples of Doxygen markup

/////////////////////////
// Terms of use
/////////////////////////

//Copyright 2002-2019 MBARI
//Monterey Bay Aquarium Research Institute, all rights reserved.
// see LICENSE file

#ifndef UDMP_SUB_H
#define UDMP_SUB_H

/////////////////////////
// Includes
/////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>


/////////////////////////
// Macros

#define UDPMS_HOST_DFL "localhost"
#define UDPMS_GROUP_DFL "239.255.0.16"
#define UDPMS_IF_DFL "unknown"
#define UDPMS_MCAST_PORT_DFL 29000
#define UDPMS_LOCAL_PORT_DFL 7070
#define UDPMS_TTL_DFL 32
#define UDPMS_BIND_DFL false
#define UDPMS_FD_INVALID -1

/////////////////////////
// Type Definitions

typedef struct udpm_sub_s
{
    char *mcast_addr_s;
    char *mcast_if_s;
    char *host_addr_s;
    int mcast_port;
    int local_port;
    struct sockaddr_in _addr;
    struct ip_mreq mreq;
    uint16_t ttl;
    int fd;
    bool connected;
}udpm_sub_t;

typedef unsigned char byte;

/////////////////////////
// Exports

#ifdef __cplusplus
extern "C" {
#endif

udpm_sub_t *udpms_new();
udpm_sub_t *udpms_cnew(const char *maddr, int mport, int ttl);

void udpms_destroy(udpm_sub_t **pself);
void udpms_set_debug(int level);
int udpms_debug();

int udpms_configure(udpm_sub_t *self, const char *maddr, int mport, int ttl);

int udpms_connect(udpm_sub_t *self, bool bind_en, bool bidir_en, bool block_en);
bool udpms_is_connected(udpm_sub_t *self);
int64_t udpms_listen(udpm_sub_t *self, uint8_t *dest, uint32_t len, int32_t to_msec, int flags);

int udpms_disconnect(udpm_sub_t *self);
int udpms_set_blocking(udpm_sub_t *self, bool block_en);

#ifdef __cplusplus
}
#endif

#endif

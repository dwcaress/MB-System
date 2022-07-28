/// @file udpm_sub.c
/// @authors k. headley
/// @date 16apr2022

/// Summary: UDP multicast subscriber component

// ///////////////////////
/// Copyright 2022  Monterey Bay Aquarium Research Institute
/// Distributed under MIT license. See LICENSE file for more information.

// /////////////////
// Includes
#include <sys/socket.h>
#include <netinet/in.h>
#include "udpm_sub.h"

// /////////////////
// Macros
#define OFMT_WMSG 36
#define OFMT_WSTAT 32
#define OFMT_KEY 12
#define OFMT_VAL 16
#define PORTSTR_BYTES 16

#define UDPMS_DBG(...) if(g_debug!=0)fprintf(__VA_ARGS__)
#define UDPMS_NDBG(n,...) if(n<=g_debug)fprintf(__VA_ARGS__)

// /////////////////
// Types

// //////////////////////
// Declarations

int s_get_opt(int fd, int opt_name, void *optval, socklen_t *optlen);
int s_set_opt(int fd, int opt_name, const void *optval, socklen_t optlen);
int s_lget_opt(int fd, int opt_level, int opt_name, void *optval, socklen_t *optlen);
int s_lset_opt(int fd, int opt_level, int opt_name, const void *optval, socklen_t optlen);
int s_bind(int fd, char *host, int port, struct sockaddr_in *addr);

// //////////////////////
// Imports

// //////////////////////
// Module Global Variables

static int wstat=OFMT_WSTAT;
//static int wkey=OFMT_KEY;
//static int wval=OFMT_VAL;
static int g_debug=0;

// //////////////////////
// Function Definitions

int s_get_opt(int fd, int opt_name, void *optval, socklen_t *optlen)
{
    return s_lget_opt(fd, SOL_SOCKET, opt_name, optval, optlen);
}

int s_set_opt(int fd, int opt_name, const void *optval, socklen_t optlen)
{
    return s_lset_opt(fd, SOL_SOCKET, opt_name, optval, optlen);
}

int s_lget_opt(int fd, int opt_level, int opt_name, void *optval, socklen_t *optlen)
{
    return getsockopt(fd, opt_level, opt_name, optval, (socklen_t *)optlen);
}

int s_lset_opt(int fd, int opt_level, int opt_name, const void *optval, socklen_t optlen)
{
    return setsockopt(fd, opt_level, opt_name, optval, (socklen_t)optlen);
}

int s_init_socket(long rto_ms, int *r_fd)
{
    int retval = UDPMS_FD_INVALID;
    int fd = UDPMS_FD_INVALID;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("s_init_socket - cannot create socket");
        return errno;
    }
    UDPMS_DBG(stderr,"s_init_socket - fd acquired[%d]\n", fd);

    struct timeval tv={0,0};
    tv.tv_sec  = rto_ms/1000L;
    tv.tv_usec = 1000L*(rto_ms%1000L);

    if (0 > s_set_opt(fd, SO_RCVTIMEO,
                       (const void **)&tv, sizeof(struct timeval)))
    {
        fprintf(stderr,"setsockopt SO_RCVTIMEO failed: %d\n", errno);
        perror("s_init_socket");
        return errno;
    }
    UDPMS_DBG(stderr,"s_init_socket - SO_RCVTIMEO set to : %ld ms\n", rto_ms);

    // Reuse the socket
    int sockopt = 1;
    if (0 > s_set_opt(fd, SO_REUSEADDR, (const void *)&sockopt, sizeof(int)))
    {
        fprintf(stderr,"setsockopt SO_REUSEADDR failed: %d\n", errno);
        perror("s_init_socket");
        return errno;
    }

#if !defined(__CYGWIN__)
    // Cygwin doesn't define SO_REUSEPORT
    // OSX requires this to reuse socket (linux optional)

    const int so_reuse=1;
    if(s_set_opt(fd, SO_REUSEPORT, &so_reuse, sizeof(so_reuse))==0){
        fprintf(stderr,"%*s %s\n",wstat,"msock_set_opt SO_REUSEPORT","OK");
    }else {
        fprintf(stderr,"%*s %s\n",wstat,"msock_set_opt SO_REUSEPORT","ERR");
        perror("setsockopt SO_REUSEPORT");
        return errno;
    }
#endif // CYGWIN

    struct linger lv = { 0, 0};

    if (0 > setsockopt(fd, SOL_SOCKET, SO_LINGER,
                       (const void **)&lv, sizeof(struct linger)))
    {
        fprintf(stderr,"setsockopt SO_LINGER failed: %d\n", errno);
        perror("setsockopt SO_LINGER");
    }

    if(NULL != r_fd)
        *r_fd = fd;

    retval = 0;

    return retval;
}

int s_bind(int fd, char *host, int port, struct sockaddr_in *addr)
{
    int retval=-1;
    if (fd>0 && NULL != host && NULL != addr) {

        memset((char*)addr, 0, sizeof(struct sockaddr_in));
        addr->sin_family = AF_INET;
        addr->sin_port = htons(port);
        addr->sin_addr.s_addr = inet_addr(host);

        // Bind to the client socket and set socket options
        if ( bind(fd, (struct sockaddr *)addr, sizeof(struct sockaddr_in)) == 0){
            retval = 0;
        } else {
            fprintf(stderr,"ERR - bind failed: fd %d host %s:%d [%d/%s] %s\n", fd, host, port, errno, strerror(errno), (errno==EINVAL?"already bound?":""));
            perror("s_bind");
            return errno;
        }
    }else{
        fprintf(stderr,"%s - invalid arguments\n",__FUNCTION__);
    }
    return retval;
}

udpm_sub_t *udpms_new()
{
    udpm_sub_t *instance = (udpm_sub_t *)malloc(sizeof(udpm_sub_t));
    if(NULL != instance)
    {
        memset(instance,0,sizeof(udpm_sub_t));
        instance->mcast_addr_s = strdup(UDPMS_GROUP_DFL);
        instance->mcast_if_s = strdup(UDPMS_IF_DFL);
        instance->host_addr_s = strdup(UDPMS_HOST_DFL);
        instance->mcast_port = UDPMS_MCAST_PORT_DFL;
        instance->local_port = UDPMS_LOCAL_PORT_DFL;
        instance->ttl = UDPMS_TTL_DFL;
        instance->fd = UDPMS_FD_INVALID;
        instance->connected=false;
    }
    return instance;
}

udpm_sub_t *udpms_cnew(const char *maddr, int mport, int ttl)
{
    udpm_sub_t *instance = udpms_new();
    if(NULL != instance)
    {
        udpms_configure(instance, maddr, mport, ttl);
    }
    return instance;
}

void udpms_destroy(udpm_sub_t **pself)
{
    if(NULL != pself)
    {
        udpm_sub_t *self = (udpm_sub_t *)*pself;
        if(NULL != self)
        {
            close(self->fd);
            free(self->mcast_addr_s);
            free(self->mcast_if_s);
            free(self->host_addr_s);
            free(self);
            *pself = NULL;
        }
    }
}

void udpms_set_debug(int level)
{
    g_debug = level;
}

int udpms_debug()
{
    return g_debug;
}

int udpms_configure(udpm_sub_t *self, const char *maddr, int mport, int ttl)
{
    int rstat = -1;
    if(NULL != self)
    {
        free(self->mcast_addr_s);
        self->mcast_addr_s = (NULL!=maddr ? strdup(maddr) : NULL);
        self->mcast_port = mport;
        self->ttl = ttl;
        self->connected = false;
        close(self->fd);
        self->fd = UDPMS_FD_INVALID;
        rstat = 0;
    }
    return rstat;
}

int udpms_set_blocking(udpm_sub_t *self, bool block_en)
{
    int retval = -1;
    if(NULL != self && self->fd > 0)
    {
        int flags = fcntl(self->fd, F_GETFL, 0);
        if (flags != -1){
            flags = (block_en ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK));
            retval = fcntl(self->fd, F_SETFL, flags);
        }// else error
    }
    return retval;
}

int udpms_connect(udpm_sub_t *self, bool bind_en, bool bidir_en, bool block_en)
{
    int retval = -1;
    if(NULL != self)
    {
        close(self->fd);
        self->fd = UDPMS_FD_INVALID;
        self->connected = false;

        s_init_socket(0, &self->fd);

        udpms_set_blocking(self, false);
        unsigned char so_loop=1;
        const int so_reuse=1;

        if(s_set_opt(self->fd, SO_REUSEADDR, &so_reuse, sizeof(so_reuse))==0){
            fprintf(stderr,"%*s %s\n",wstat,"msock_set_opt SO_REUSEADDR","OK");
        }else {
            fprintf(stderr,"%*s %s\n",wstat,"msock_set_opt SO_REUSEADDR","ERR");
            perror("setsockopt SO_REUSEADDR");
            return retval;
        }

#if !defined(__CYGWIN__)
        // Cygwin doesn't define SO_REUSEPORT
        // OSX requires this to reuse socket (linux optional)
        if(s_set_opt(self->fd, SO_REUSEPORT, &so_reuse, sizeof(so_reuse))==0){
            fprintf(stderr,"%*s %s\n",wstat,"msock_set_opt SO_REUSEPORT","OK");
        }else {
            fprintf(stderr,"%*s %s\n",wstat,"msock_set_opt SO_REUSEPORT","ERR");
            perror("setsockopt SO_REUSEPORT");
            return retval;
        }
#endif // CYGWIN

        if(bidir_en){
            // future expansion (for mcast outbound)
            if(s_lset_opt(self->fd, IPPROTO_IP, IP_MULTICAST_LOOP, &so_loop, sizeof(so_loop))==0) {
                fprintf(stderr,"%*s %s\n",wstat,"msock_lset_opt IP_MULTICAST_LOOP","OK");
            }else {
                fprintf(stderr,"%*s %s\n",wstat,"msock_lset_opt IP_MULTICAST_LOOP","ERR");
                perror("setsockopt IP_MULTICAST_LOOP");
                return retval;
            }
            unsigned char ttl=self->ttl;
            if(s_lset_opt(self->fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl))==0) {
                fprintf(stderr,"%*s %s\n",wstat,"setsockopt IP_MULTICAST_TTL","OK");
            }else {
                fprintf(stderr,"%*s %s\n",wstat,"setsockopt IP_MULTICAST_TTL","ERR");
                perror("setsockopt IP_MULTICAST_TTL");
                return retval;
            }
        }//bidir_en

        // bind to receive address
        if (bind_en){
            // bind the socket to INADDR_ANY (accept mcast on all interfaces)
            if(s_bind(self->fd, self->mcast_addr_s, self->mcast_port, &self->_addr)==0){
                fprintf(stderr,"%*s %s\n",wstat,"bind","OK");
            }else {
                fprintf(stderr,"%*s %s\n",wstat,"bind","ERR");
                perror("s_bind");
                return retval;
            }
        }// bind_en

        udpms_set_blocking(self, block_en);

        self->mreq.imr_multiaddr.s_addr=inet_addr(self->mcast_addr_s);
        self->mreq.imr_interface.s_addr=htonl(INADDR_ANY);
        if (s_lset_opt(self->fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,&self->mreq,sizeof(struct ip_mreq)) == 0){
            fprintf(stderr,"%*s %s\n",wstat,"s_lset_opt IP_ADD_MEMBERSHIP","OK");
            self->connected = true;
            retval = 0;
        }else {
            fprintf(stderr,"%*s %s\n",wstat,"s_lset_opt IP_ADD_MEMBERSHIP","ERR");
            perror("setsockopt IP_ADD_MEMBERSHIP");
            return retval;
        }
    }
    return retval;
}

bool udpms_is_connected(udpm_sub_t *self)
{
    if(NULL != self)
    {
        return self->connected;
    }
    return false;
}

int64_t udpms_listen(udpm_sub_t *self, byte *dest, uint32_t len, int32_t to_msec, int flags)
{
    int64_t retval=0;
    if(NULL != self && self->fd > 0 && NULL != dest && len > 0)
    {
        struct sockaddr *dest_addr = (struct sockaddr *)&self->_addr;
        socklen_t addrlen = sizeof(struct sockaddr_in);

        //        fprintf(stderr,"recvfrom connection[%p] dest_addr[%p] ai_family[%d] addrlen[%d]\n",addr,(addr?addr->ainfo->ai_addr:NULL),(int)(addr?addr->ainfo->ai_family:-1),(int)(addr?addr->ainfo->ai_addrlen:-1));
        if(to_msec>=0){
            struct timeval tv = {0,0};
            tv.tv_sec  = to_msec/1000L;
            tv.tv_usec = 1000L*(to_msec%1000L);
            s_set_opt(self->fd, SO_RCVTIMEO, (const void **)&tv, sizeof(struct timeval));
            udpms_set_blocking(self, false);
        }else{
            udpms_set_blocking(self, true);
        }

        ssize_t test = recvfrom(self->fd, dest, len, flags, dest_addr, &addrlen);

        if(test > 0){
            retval = test;
            // UDPMS_DBG(stderr,"received data connection[%p] dest[%p] ainfo[%p] [%lld]\n",addr,dest_addr,addr->ainfo,retval);
        } else {
            UDPMS_NDBG(4, stderr,"%s: nothing to read ret[%zd] [%d/%s]\n", __func__, test, errno,strerror(errno));
           if(errno != EAGAIN && errno != EWOULDBLOCK){
                UDPMS_DBG(stderr,"%s: recvfrom failed - disconnecting ret[%zd] [%d/%s]\n", __func__, test, errno,strerror(errno));
                udpms_disconnect(self);
            }
        }
    }else{
        fprintf(stderr,"%s - invalid arguments\n",__func__);
    }

    return retval;
}

int udpms_disconnect(udpm_sub_t *self)
{
    int retval = -1;
    if(NULL != self)
    {
        close(self->fd);
        self->connected = false;
        retval = 0;
    }
    return retval;
}

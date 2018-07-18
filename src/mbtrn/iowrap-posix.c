///
/// @file iowrap-posix.c
/// @authors k. Headley
/// @date 01 jan 2018
 
/// MBRTN platform-dependent IO wrappers implementation
/// for *nix/Cygwin

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
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include "iowrap.h"
#include "mdebug.h"
#include "mconfig.h"

/////////////////////////
// Macros
/////////////////////////
#ifndef MSG_NOSIGNAL
# define MSG_NOSIGNAL 0
# ifdef SO_NOSIGPIPE
#  define USE_SO_NOSIGPIPE
# else
#  error "Cannot block SIGPIPE!"
# endif
#endif

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

/////////////////////////
// Function Definitions
/////////////////////////

// socket Allocate a socket descriptor
// bind Associate a socket with an IP address and port number
// listen Tell a socket to listen for incoming connections
// accept Accept an incoming connection on a listening socket
// connect Connect a socket to a server
// close Close a socket descriptor
// setsockopt(), getsockopt() Set various options for a socket
// recv(), recvfrom() Receive data on a socket
// send(), sendto() Send data out over a socket
//  getaddrinfo(), freeaddrinfo(), gai_strerror(), Get information about a host name and/or service and load up a struct sockaddr with the result
// getnameinfo Look up the host name and service name information for a given struct sockaddr->
// gethostname Returns the name of the system
// gethostbyname, gethostbyaddr Get an IP address for a hostname, or vice-versa
// getpeername Return address info about the remote side of the connection
// fcntl Control socket descriptors
// htons(), htonl(), ntohs(), ntohl() Convert multi-byte integer types from host byte order to network byte order
// inet_ntoa(), inet_aton(), inet_addr Convert IP addresses from a dots-and-number string to a struct in_addr and back
// inet_ntop(), inet_pton() Convert IP addresses to human-readable form and back.
// poll Test for events on multiple sockets simultaneously
// select Check if sockets descriptors are ready to read/write
// perror(), strerror() Print an error as a human-readable string

/// @fn iow_thread_t * iow_thread_new()
/// @brief create new thread.
/// @return pointer to thread
iow_thread_t *iow_thread_new()
{
    iow_thread_t *self = (iow_thread_t *)malloc( sizeof(iow_thread_t) );
    if (self) {
        self->status=NULL;
    }
    return self;
}
// End function iow_thread_new


/// @fn void iow_thread_destroy(iow_thread_t ** pself)
/// @brief release thread resources.
/// @param[in] pself pointer to instance pointer (created with iow_thread_new)
/// @return none
void iow_thread_destroy(iow_thread_t **pself)
{
    if (pself) {
        iow_thread_t *self = *pself;
        if (self) {
            free(self);
            *pself=NULL;
        }
    }
}
// End function iow_thread_destroy


/// @fn int iow_thread_start(iow_thread_t * thread, mbtrn_thread_fn func, void * arg)
/// @brief start thread.
/// @param[in] thread thread instance
/// @param[in] func thread entry point function
/// @param[in] arg pointer to thread arguments
/// @return 0 on success, -1 otherwise
int iow_thread_start(iow_thread_t *thread, mbtrn_thread_fn func, void *arg)
{
    int retval=0;
    
    pthread_attr_init(&thread->attr);
    pthread_attr_setdetachstate(&thread->attr, PTHREAD_CREATE_JOINABLE);
    
    if ( pthread_create(&thread->t, &thread->attr, func, arg) != 0)
    {
        fprintf(stderr,"error creating thread.");
        retval=-1;
    }
    return retval;
}
// End function iow_thread_start


/// @fn int iow_thread_join(iow_thread_t * thread)
/// @brief wait for thread to complete.
/// @param[in] thread thread instance
/// @return 0 on success, -1 otherwise
int iow_thread_join(iow_thread_t *thread){
    int retval=0;
    if ( pthread_join ( thread->t, (void **)thread->status ) ) {
        fprintf(stderr,"error joining thread.");
        retval=-1;
    }
    return retval;
}
// End function iow_thread_join


/// @fn int iow_set_blocking(iow_socket_t * s, _Bool enabled)
/// @brief configure socket to block or not block.
/// @param[in] s socket instance
/// @param[in] enabled true for blocking, false for non-blocking
/// @return 0 on success, -1 otherwise
int iow_set_blocking(iow_socket_t *s, bool enabled)
{
    int retval=-1;
    if (NULL != s) {
        int flags = fcntl(s->fd, F_GETFL, 0);
        if (flags != -1){
            flags = enabled ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
            retval = fcntl(s->fd, F_SETFL, flags);
        }// else error
    }// else invalid arg
    
    return retval;
}
// End function iow_set_blocking


/// @fn iow_addr_t * iow_addr_new()
/// @brief create new IP address.
/// should be destroyed with iow_addr_destroy.
/// @return reference to new address
iow_addr_t *iow_addr_new()
{
    iow_addr_t *self = (iow_addr_t *)malloc(sizeof(iow_addr_t));
    if (self) {
        memset(self,0,sizeof(iow_addr_t));
        self->ainfo=NULL;
        self->alist=NULL;
        self->host=NULL;
    }
    return self;
}
// End function iow_addr_new


/// @fn void iow_addr_destroy(iow_addr_t ** pself)
/// @brief release resources for iow_address instance.
/// @param[in] pself pointer to instance
/// @return none
void iow_addr_destroy(iow_addr_t **pself)
{
    if (NULL!=pself) {
        iow_addr_t *self = *pself;
        if (NULL!=self) {
            
            if (NULL!=self->alist) {
                // alist used to hold UDP reslist
                // [i.e. getaddrinfo],
                // ainfo points to list.
                // use freeaddrinfo to free list
                freeaddrinfo(self->alist);
            }else if(NULL!=self->ainfo && NULL==self->alist){
                // alist not used, ainfo holds peer addr
                // info [i.e. recvfrom]
                if (NULL != self->ainfo->ai_addr) {
                    free(self->ainfo->ai_addr);
                }
                free(self->ainfo);
            }
            if (NULL != self->host) {
                free(self->host);
            }
            free(self);
            *pself=NULL;
        }
    }
}
// End function iow_addr_destroy


/// @fn void iow_addr_init(iow_addr_t * self)
/// @brief initialize address instance (zeros members).
/// @param[in] self address instance
/// @return none
void iow_addr_init(iow_addr_t *self)
{
    if (NULL!=self) {
        if (NULL!=self->ainfo) {
            if (NULL != self->ainfo->ai_addr) {
                free(self->ainfo->ai_addr);
            }
            free(self->ainfo);
        }
        self->ainfo = (struct addrinfo *)malloc(sizeof(struct addrinfo));
        memset(self->ainfo,0,sizeof(struct addrinfo));
        self->ainfo->ai_addr = (struct sockaddr *)malloc(IOW_ADDR_LEN);
        memset(self->ainfo->ai_addr,0,IOW_ADDR_LEN);
    }
}
// End function iow_addr_init


/// @fn iow_peer_t * iow_peer_new()
/// @brief create new network peer (e.g. UDP client).
/// caller should release resources using iow_peer_destroy.
/// @return peer instance reference
iow_peer_t *iow_peer_new()
{
    iow_peer_t *self = (iow_peer_t *)malloc(sizeof(iow_peer_t));
    if (self) {
        memset(self,0,sizeof(iow_peer_t));
        self->addr=iow_addr_new();
        iow_addr_init(self->addr);
    }
    return self;
}
// End function iow_peer_new


/// @fn void iow_peer_destroy(iow_peer_t ** pself)
/// @brief release peer resources.
/// @param[in] pself pointer to instance reference.
/// @return none
void iow_peer_destroy(iow_peer_t **pself)
{
    if (NULL!=pself) {
        iow_peer_t *self = *pself;
        if (NULL!=self) {
            iow_addr_destroy(&self->addr);
            free(self);
            *pself=NULL;
        }
    }
}
// End function iow_peer_destroy

/// @fn void iow_peer_free(void * pself)
/// @brief free function (use as autofree function in mlist).
/// @param[in] pself TBD
/// @return none
/// @sa mlist.h
void iow_peer_free(void *pself)
{
    if (NULL!=pself) {
        iow_peer_t *self = (iow_peer_t *)pself;
        if (NULL!=self) {
            iow_addr_destroy(&self->addr);
            free(self);
            self=NULL;
        }
    }
}
// End function iow_peer_free

/// @fn void iow_pstats_show(iow_pstats_t * self, _Bool verbose, uint16_t indent)
/// @brief output reader parameter summary to stderr.
/// @param[in] self stats reference
/// @param[in] verbose provide verbose output
/// @param[in] indent output indentation (spaces)
/// @return none
void iow_pstats_show(iow_pstats_t *self, bool verbose, uint16_t indent)
{
    if (NULL != self) {
        fprintf(stderr,"%*s[self         %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[t_connect    %10ld]\n",indent,(indent>0?" ":""), self->t_connect);
        fprintf(stderr,"%*s[t_disconnect %10ld]\n",indent,(indent>0?" ":""), self->t_disconnect);
        fprintf(stderr,"%*s[tx_count     %10u]\n",indent,(indent>0?" ":""), self->tx_count);
        fprintf(stderr,"%*s[tx_bytes     %10u]\n",indent,(indent>0?" ":""), self->tx_bytes);
        fprintf(stderr,"%*s[rx_count     %10u]\n",indent,(indent>0?" ":""), self->rx_count);
        fprintf(stderr,"%*s[rx_bytes     %10u]\n",indent,(indent>0?" ":""), self->rx_bytes);
        fprintf(stderr,"%*s[hbeats       %10u]\n",indent,(indent>0?" ":""), self->hbeats);
        fprintf(stderr,"%*s[err_count    %10u]\n",indent,(indent>0?" ":""), self->err_count);
    }
}
// End function mbtrn_reader_show

/// @fn iow_socket_t * iow_socket_new(const char * host, int port, iow_socket_type type)
/// @brief create new socket instance.
/// @param[in] host connection IP address
/// @param[in] port connection IP port
/// @param[in] type ST_UDP or ST_TCP
/// @return new socket instance reference.
iow_socket_t *iow_socket_new(const char *host, int port, iow_socket_type type)
{
    iow_socket_t *self = (iow_socket_t *)malloc(sizeof(iow_socket_t));
    if (self) {
        self->fd=-1;
        self->qlen=0;
        self->status=0;
        self->type = type;
        self->addr = iow_addr_new();
        iow_configure(self,host,port,type,0);
    }
    return self;
}
// End function iow_socket_new


/// @fn void iow_socket_destroy(iow_socket_t ** pself)
/// @brief release socket resources.
/// @param[in] pself pointer to instance reference
/// @return none
void iow_socket_destroy(iow_socket_t **pself)
{
    if (pself) {
        iow_socket_t *self = *pself;
        if (self) {
            close(self->fd);
            iow_addr_destroy(&self->addr);
           free(self);
            *pself=NULL;
        }
    }
}
// End function iow_socket_destroy


/// @fn iow_socket_t * iow_wrap_fd(int fd)
/// @brief wrap file descriptor in iow_socket.
/// e.g. when posix functions provide fd that must be passed
/// to iow_ socket functions. Sets socket status to SS_CONNECTED.
/// @param[in] fd file descriptor
/// @return iow_socket instance (caller should destroy with iow_socket_destroy).
iow_socket_t *iow_wrap_fd(int fd)
{
    iow_socket_t *s = iow_socket_new("wrapper",9999,ST_TCP);
    if (NULL != s) {
        s->fd = fd;
        s->status=SS_CONNECTED;
    }
    return s;
}
// End function iow_wrap_fd

/// @fn int iow_configure(iow_socket_t * s, const char * host, int port, iow_socket_type type, uint16_t qlen)
/// @brief configure a socket instance.
/// @param[in] s socket reference
/// @param[in] host connection host
/// @param[in] port connection port
/// @param[in] type ST_UDP, ST_TCP
/// @param[in] qlen number of connections (for servers)
/// @return 0 on success, -1 otherwise.
int iow_configure(iow_socket_t *s, const char *host, int port, iow_socket_type type, uint16_t qlen)
{
    int retval=-1;
    if (NULL != (s->addr->host)) {
        free(s->addr->host);
    }
    if (NULL != host) {
        s->addr->host=strdup(host);
    }
    s->addr->port=port;
    s->qlen=qlen;
    memset(s->addr->portstr,0,PORTSTR_BYTES*sizeof(char));
    sprintf(s->addr->portstr,"%d",port);
    memset(&s->addr->hints,0,sizeof(struct addrinfo));
    s->addr->hints.ai_family=PF_INET;
    MMDEBUG(IOW,"configuring type [%s]\n",(type==ST_TCP ? "SOCK_STREAM" : "SOCK_DGRAM"));
    s->addr->hints.ai_socktype=(type==ST_TCP ? SOCK_STREAM : SOCK_DGRAM);
    s->addr->hints.ai_flags=AI_PASSIVE;
    s->status=SS_CREATED;
    
    int status=0;

    if (NULL != s->addr->alist) {
        // free linked lists, if it exists.
        freeaddrinfo(s->addr->alist);
        s->addr->alist=NULL;
    }
    struct addrinfo *rp=NULL;
    
    if ((status = getaddrinfo(s->addr->host, s->addr->portstr, &s->addr->hints, &rp)) == 0){
        
		// walk the linked list of addrinfo returned by getaddrinfo
        // until we find a socket that succeeds
        s->addr->alist = rp;
        
        for (; rp!=NULL; rp = rp->ai_next) {
//            MMDEBUG(IOW,"rp[%p]\n",rp);
            s->fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (s->fd>0){
                s->status=SS_CONFIGURED;
                s->addr->ainfo = rp;
                MMDEBUG(IOW,"socket created[%d] ainfo[%p] alist[%p]\n",s->fd,s->addr->ainfo,s->addr->alist);
#ifdef USE_SO_NOSIGPIPE
                retval=1;
                if (setsockopt(s->fd, SOL_SOCKET, SO_NOSIGPIPE, (void*)&retval, sizeof(retval)) != 0) {
                    fprintf(stderr, "failed to set SO_NOSIGPIPE on socket [%d/%s]\n", errno,strerror(errno));
                }
#endif
                retval=0;
                break;
            }else {
                s->status=SS_ERROR;
                fprintf(stderr, "socket request failed [%d/%s]\n", errno,strerror(errno));
            }
            close(s->fd);
            s->fd=-1;
            s->status = SS_CONFIGURED;
       }
    }else{
        fprintf(stderr, "getaddrinfo error: %d/%s\n",status, gai_strerror(status));
        s->status=SS_ERROR;
    }
    
    return retval;
}
// End function iow_configure


/// @fn int iow_connect(iow_socket_t * s)
/// @brief connect (to server) socket.
/// @param[in] s socket instance.
/// @return 0 on success, -1 otherwise.
int iow_connect(iow_socket_t *s)
{
    int retval=-1;
    if (NULL != s && NULL != s->addr->ainfo) {
        if (connect(s->fd, s->addr->ainfo->ai_addr, s->addr->ainfo->ai_addrlen)==0) {
            // success
            char buf[ADDRSTR_BYTES]={0};
            MMINFO(IOW,"connect OK [%s]\n",iow_addr2str(s,buf,ADDRSTR_BYTES));
            s->status=SS_CONNECTED;
            retval=0;
        }else{
            MERROR("connect failed for fd[%d] [%d/%s]\n",s->fd,errno,strerror(errno));
        }
    }else{
        MERROR("invalid argument s[%p] ainfo[%p]\n",s,(s?s->addr->ainfo:0));
    }
    return retval;
}
// End function iow_connect


/// @fn int iow_bind(iow_socket_t * s)
/// @brief bind (server) socket to port.
/// @param[in] s socket instance
/// @return 0 on success, -1 otherwise
int iow_bind(iow_socket_t *s)
{
    int retval=0;
    if (NULL != s && s->fd>0 && NULL != s->addr->ainfo) {
        const int optionval = 1;
        setsockopt(s->fd, SOL_SOCKET, SO_REUSEADDR, &optionval, sizeof(optionval));
        s->status = SS_CONFIGURED;
        if(bind(s->fd, s->addr->ainfo->ai_addr, s->addr->ainfo->ai_addrlen) == 0){
            s->status = SS_BOUND;
        }else{
            fprintf(stderr,"bind failed [%d/%s] %s \n",errno,strerror(errno),(errno==EINVAL?"already bound?":""));
        }
    }else{
        fprintf(stderr,"invalid argument\n");
        retval=-1;
    }
    return retval;
}
// End function iow_bind


/// @fn int iow_listen(iow_socket_t * s)
/// @brief listen for connections on (server) socket.
/// @param[in] s socket instance
/// @return 0 on success, -1 otherwise
int iow_listen(iow_socket_t *s)
{
    int retval=-1;
    if (NULL != s && NULL != s->addr->ainfo) {
        const int optionval = 1;
        setsockopt(s->fd, SOL_SOCKET, SO_REUSEADDR, &optionval, sizeof(optionval));
        
        if(s->status==SS_BOUND || bind(s->fd, s->addr->ainfo->ai_addr, s->addr->ainfo->ai_addrlen) == 0){
            s->status = SS_LISTENING;
            if (listen(s->fd, s->qlen)==0) {
                s->status = SS_LISTENOK;
                // success
                char buf[ADDRSTR_BYTES]={0};
                MMINFO(IOW,"%s - listening [%s] queue[%d]\n",__FUNCTION__,iow_addr2str(s,buf,ADDRSTR_BYTES),s->qlen);
                retval=0;
            }else{
                fprintf(stderr,"listen failed [%d/%s]\n",errno,strerror(errno));
            }
        }else{
            fprintf(stderr,"bind failed [%d/%s]\n",errno,strerror(errno));
        }
    }else{
        fprintf(stderr,"invalid argument\n");
    }
    return retval;
}
// End function iow_listen


/// @fn int iow_accept(iow_socket_t * s)
/// @brief accept (client) connection on socket.
/// @param[in] s socket instance
/// @return file descriptor (>0) on success, -1 otherwise
int iow_accept(iow_socket_t *s)
{
    int retval=-1;
    
    if (NULL != s && NULL != s->addr->ainfo){
        struct sockaddr_storage client_addr={0};
        socklen_t addr_size=0;

        s->fd = accept(s->fd, (struct sockaddr *)&client_addr, &addr_size);
        if (s->fd != -1) {
            MMINFO(IOW,"server recieved connection from client on socket [%d]\n",s->fd);
            retval = s->fd;
        }else{
            // accept failed
            MMINFO(IOW,"accept failed [%d/%s]\n",errno,strerror(errno));
        }
    }else{
        fprintf(stderr,"invalid argument\n");
    }
    return retval;
}
// End function iow_accept


/// @fn int64_t iow_send(iow_socket_t *s, byte *buf, uint32_t len)
/// @brief send data via socket.
/// @param[in] s socket instance
/// @param[in] buf data buffer
/// @param[in] len number of bytes to send
/// @return number of bytes sent on success, -1 otherwise
int64_t iow_send(iow_socket_t *s,byte *buf, uint32_t len)
{
    int64_t retval=-1;
    if (NULL != s && NULL != buf && len>0) {
        if (s->status == SS_CONNECTED) {
            if (s->type==ST_TCP) {
                if( (retval = send(s->fd,buf,len,MSG_NOSIGNAL))<=0){
                    MERROR("ERR - send fd[%d] returned %"PRId64" [%d/%s]\n",s->fd,retval,errno,strerror(errno));
                }
            }
        }else{
         // not ready to send;
            MERROR("socket not ready to send\n");
        }
    }else{
        MERROR("invalid arguments\n");
    }
    return retval;
}
// End function iow_send


/// @fn int64_t iow_sendto(iow_socket_t * s, iow_addr_t * peer, byte * buf, uint32_t len)
/// @brief send data via (UDP) socket.
/// @param[in] s socket instance
/// @param[in] peer peer address
/// @param[in] buf data buffer
/// @param[in] len number of bytes to send
/// @return number of bytes sent on success, -1 ohterwise.
int64_t iow_sendto(iow_socket_t *s, iow_addr_t *peer, byte *buf, uint32_t len)
{
    int64_t retval=-1;
    if (NULL != s && NULL != buf && len>0) {
        if (s->status == SS_CONNECTED || s->status==SS_BOUND) {

            if (s->type==ST_UDP ) {
                
                struct sockaddr *dest_addr = (peer==NULL?NULL:(peer->ainfo->ai_addr));
                
                if( (retval = sendto(s->fd,buf,len,0,dest_addr,IOW_ADDR_LEN)) > 0){
//                    MMDEBUG(IOW,"sendto OK [%lld]\n",retval);
                }else{
//                    MERROR("ERR - sendto returned %lld [%d/%s]\n",retval,errno,strerror(errno));
                }
            }else{
                MERROR("invalid arguments (UDP)\n");
            }
        }else{
            // not ready to send;
            MERROR("socket not ready to send\n");
        }
    }else{
        MERROR("invalid arguments\n");
    }
    return retval;
}
// End function iow_sendto


/// @fn int64_t iow_recv(iow_socket_t * s, byte * buf, uint32_t len)
/// @brief receive bytes on socket.
/// @param[in] s socket instance
/// @param[in] buf destination buffer
/// @param[in] len number of bytes to receive
/// @return number of bytes received on success, -1 otherwise
int64_t iow_recv(iow_socket_t *s, byte *buf, uint32_t len)
{
    int64_t retval=-1;
    if (NULL != s && NULL != buf && len>0) {
        retval = recv(s->fd,buf,len,0);
    }else{
        MERROR("invalid arguments\n");
    }
    return retval;
}
// End function iow_recv


/// @fn int64_t iow_recvfrom(iow_socket_t * s, iow_addr_t * peer, byte * buf, uint32_t len)
/// @brief TBD.
/// @param[in] s socket instance
/// @param[in] peer peer address
/// @param[in] buf destination buffer
/// @param[in] len number of bytes to receive
/// @return number of bytes received on success, -1 otherwise
int64_t iow_recvfrom(iow_socket_t *s, iow_addr_t *peer, byte *buf, uint32_t len)
{
    int64_t retval= 0;
    
    if (NULL != s && NULL!=buf && len>0) {

        struct sockaddr *dest_addr=(peer==NULL?NULL:peer->ainfo->ai_addr);
        socklen_t addrlen = (peer==NULL?0:IOW_ADDR_LEN);

        
        if( (retval = recvfrom(s->fd,buf,len,0,dest_addr,&addrlen))>0){
           // MMDEBUG(IOW,"received data peer[%p] dest[%p] ainfo[%p] [%lld]\n",peer,dest_addr,peer->ainfo,retval);
        }else{
//            MMDEBUG(IOW,"recvfrom failed [%d %s]\n",errno,strerror(errno));
        }
    }else{
        MERROR("invalid arguments\n");
    }
    return retval;
}
// End function iow_recvfrom


/// @fn int64_t iow_read_tmout(iow_socket_t * s, byte * buf, uint32_t len, uint32_t timeout_msec)
/// @brief read bytes from socket until length or timeout exceeded.
/// @param[in] s socket instance
/// @param[in] buf data buffer
/// @param[in] len max bytes to receive
/// @param[in] timeout_msec timeout (milliseconds)
/// @return number of bytes received on success, -1 otherwise
int64_t iow_read_tmout(iow_socket_t *s, byte *buf, uint32_t len, uint32_t timeout_msec)
{
    me_errno=ME_OK;
    int64_t retval=0;
    double t_rem=(double)timeout_msec;
    int nbytes=0;
    struct timespec now={0},start={0};

    uint32_t read_total=0;
    uint32_t loops=0;
    if ( (NULL!=s) && s->fd>0 && (NULL!=buf) && len>0) {
        struct timeval tv;
        fd_set read_fds;
        int fdmax;
        int stat=0;

        byte *pbuf=buf;
        memset(buf,0,len);
        
        if (timeout_msec>0) {
            tv.tv_sec = timeout_msec/1000;
            tv.tv_usec = 1000*(timeout_msec%1000);
        }else{
            // use 100 ms default
            tv.tv_sec = 0;
            tv.tv_usec = 250000;
        }
        
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        double start_ns = (double)((1000000000.*start.tv_sec+start.tv_nsec));
        double to_ns    = (double)timeout_msec*1000000.;

        while (read_total<len && t_rem>0 && pbuf<(buf+len) ) {
            loops++;
            FD_ZERO(&read_fds);
            FD_SET(s->fd,&read_fds);
            fdmax = s->fd;
            
            if( (stat=select(fdmax+1, &read_fds, NULL, NULL, &tv)) != -1){
                if (FD_ISSET(s->fd, &read_fds)){
                   // MMINFO(IOW,"readfs [%d/%d] ready to read\n",s->fd,fdmax);
                    
                    if (( nbytes = recv(s->fd, pbuf, (len-read_total), 0)) > 0) {
                        //MMINFO(IOW,"read %d bytes\n",nbytes);
                        read_total+=nbytes;
                        pbuf+=nbytes;
                        retval=read_total;
                    }else{
                        // got error or connection closed by server
                        if (nbytes == 0) {
                            // connection closed
                            fprintf(stderr,"ERR - socket %d closed\n", s->fd);
                            retval = -1;
                            me_errno = ME_ESOCK;
                            break;
                        } else if(nbytes<0) {
                            fprintf(stderr,"ERR - recv failed socket[%d] [%d/%s]\n",s->fd,errno,strerror(errno));
                            retval = -1;
                            me_errno = ME_ERCV;
                            break;
                        }
                    }
                    FD_CLR(s->fd, &read_fds); // remove from master set
                }
            }else{
                MMDEBUG(IOW,"select err [%d/%s]\n",errno,strerror(errno));
                if (errno==EINTR) {
                    // select got signal before timeout expired
                    MMDEBUG(IOW,"EINTR\n");
                }
            }
            
            clock_gettime(CLOCK_MONOTONIC, &now);
            if (timeout_msec>0) {
                double now_ns   = (double)((1000000000.*(now.tv_sec)+now.tv_nsec));


                double x = (to_ns -  (now_ns - start_ns));
                t_rem = (x<t_rem ? x:t_rem);
	
                // don't rely on tv after select returns, since
                // - select may have been interrupted by a signal
                // - it isn't portable to rely on select setting it to the
                //   time not slept.

                tv.tv_sec  = (int32_t)(t_rem/1000000000L);
                tv.tv_usec = (int32_t)(1000*((int32_t)t_rem%1000));
            }else{
                tv.tv_sec = 0;
                tv.tv_usec = 250000;
            }
        }// while
     }// ERR - invalid arg
    
        if (read_total==len) {
            // read complete - OK
            me_errno=ME_OK;
        }else{

            switch (me_errno) {
                case ME_ERCV:
                case ME_ESOCK:
                    // use me_errno
                    break;
                    
                default:
                    if (t_rem<0) {
                        // timed out
                        me_errno=ME_ETMOUT;
                    }else{
                        // incomplete
                        me_errno=ME_EINC;
                    }
            }
        }
//    MMDEBUG(IOW,"t[%.0lf] n[%u/%u] rv[%d] loops[%u]\n",t_rem,read_total,len,retval,loops);
//    if (loops==1) {
//        MMDEBUG(IOW,"start[ %lu : %lu] now[%lu : %lu]\n",start.tv_sec, start.tv_nsec,now.tv_sec,now.tv_nsec);
//    }

    return retval;
}
// End function iow_read_tmout

/// @fn int s_iow2posix_flags(int iflags)
/// @brief convert iow to posix file flags.
/// @param[in] iflags iow flags
/// @return posix file flags value
static int s_iow2posix_flags(int iflags)
{
    int pflags=0;
    
    pflags |= ( (iflags&IOW_RONLY   )!=0 ? O_RDONLY   : 0 );
    pflags |= ( (iflags&IOW_WONLY   )!=0 ? O_WRONLY   : 0 );
    pflags |= ( (iflags&IOW_RDWR    )!=0 ? O_RDWR     : 0 );
    pflags |= ( (iflags&IOW_APPEND  )!=0 ? O_APPEND   : 0 );
    pflags |= ( (iflags&IOW_CREATE  )!=0 ? O_CREAT    : 0 );
    pflags |= ( (iflags&IOW_TRUNC   )!=0 ? O_TRUNC    : 0 );
    pflags |= ( (iflags&IOW_NONBLOCK)!=0 ? O_NONBLOCK : 0 );
    
    return pflags;
}
// End function s_iow2posix_flags


/// @fn mode_t s_iow2posix_mode(iow_mode_t imode)
/// @brief convert iow to posix file (permission) mode flags.
/// @param[in] imode iow file modes
/// @return posix file mode flags
static mode_t s_iow2posix_mode(iow_mode_t imode)
{
    mode_t pmode=0;
    pmode |= ( (imode&IOW_RWXU)!=0 ? S_IRWXU : 0 );
    pmode |= ( (imode&IOW_RU  )!=0 ? S_IRUSR : 0 );
    pmode |= ( (imode&IOW_WU  )!=0 ? S_IWUSR : 0 );
    pmode |= ( (imode&IOW_XU  )!=0 ? S_IXUSR : 0 );
    pmode |= ( (imode&IOW_RWXG)!=0 ? S_IRWXG : 0 );
    pmode |= ( (imode&IOW_RG  )!=0 ? S_IRGRP : 0 );
    pmode |= ( (imode&IOW_WG  )!=0 ? S_IWGRP : 0 );
    pmode |= ( (imode&IOW_XG  )!=0 ? S_IXGRP : 0 );
    pmode |= ( (imode&IOW_RWXO)!=0 ? S_IRWXO : 0 );
    pmode |= ( (imode&IOW_RO  )!=0 ? S_IROTH : 0 );
    pmode |= ( (imode&IOW_WO  )!=0 ? S_IWOTH : 0 );
    pmode |= ( (imode&IOW_XO  )!=0 ? S_IXOTH : 0 );
//    MMDEBUG(IOW,"imode[0x%0X] pmode[0x%0X] S_IRUSR[0x0%X] S_IWUSR[0x%0X] S_IRGRP[0x%0X] S_IWGRP[0x%0X]\n",imode,pmode,S_IRUSR,S_IWUSR,S_IRGRP,S_IWGRP);
//    MMDEBUG(IOW,"S_IRWXU[0x%03X] S_IRUSR[0x%03X] S_IWUSR[0x%03X] S_IXUSR[0x%03X]\n",S_IRWXU,S_IRUSR,S_IWUSR,S_IXUSR);
//    MMDEBUG(IOW,"S_IRWXG[0x%03X] S_IRGRP[0x%03X] S_IWGRP[0x%03X] S_IXGRP[0x%03X]\n",S_IRWXG,S_IRGRP,S_IWGRP,S_IXGRP);
//    MMDEBUG(IOW,"S_IRWXO[0x%03X] S_IROTH[0x%03X] S_IWOTH[0x%03X] S_IXOTH[0x%03X]\n",S_IRWXO,S_IROTH,S_IWOTH,S_IXOTH);
    return pmode;
}
// End function s_iow2posix_mode


/// @fn iow_file_t * iow_file_new(const char * path)
/// @brief new file instance. This is the equivalent of a file descriptor.
/// @param[in] path file path
/// @return new file instance.
iow_file_t *iow_file_new(const char *path)
{
    iow_file_t *self = (iow_file_t *)malloc(sizeof(iow_file_t));
    if (self) {
        memset(self,0,sizeof(iow_file_t));
        self->fd = -1;
        if (NULL!=path) {
            self->path = strdup(path);
        }else{
            self->path=NULL;
        }
    }
    return self;
}
// End function iow_file_new


/// @fn void iow_file_destroy(iow_file_t ** pself)
/// @brief release file resources.
/// @param[in] pself pointer to file reference
/// @return none
void iow_file_destroy(iow_file_t **pself)
{
    if (NULL!=pself) {
        iow_file_t *self = *pself;
        if (NULL!=self) {
            if (NULL!=self->path) {
                free(self->path);
            }
            free(self);
            *pself = NULL;
        }
    }
}
// End function iow_file_destroy

/// @fn void iow_file_show(iow_file_t * self, _Bool verbose, uint16_t indent)
/// @brief output file parameter summary to stderr.
/// @param[in] self file reference
/// @param[in] verbose true for verbose output
/// @param[in] indent indentation (spaces)
/// @return none
void iow_file_show(iow_file_t *self, bool verbose, uint16_t indent)
{
    if (NULL != self) {
        fprintf(stderr,"%*s[self     %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[path     %10s]\n",indent,(indent>0?" ":""), self->path);
        fprintf(stderr,"%*s[fd       %10d]\n",indent,(indent>0?" ":""), self->fd);
        fprintf(stderr,"%*s[flags    %010X]\n",indent,(indent>0?" ":""), self->flags);
        fprintf(stderr,"%*s[mode     %010X]\n",indent,(indent>0?" ":""), self->mode);
    }
}
// End function iow_file_show


/// @fn int iow_open(iow_file_t * self, iow_flags_t flags)
/// @brief open a file.
/// @param[in] self file reference
/// @param[in] flags flag values
/// @return file descriptor (>0) on success, -1 otherwise
int iow_open(iow_file_t *self,iow_flags_t flags)
{
    int retval=-1;
    if (NULL!=self && NULL!=self->path) {
        int pflags=s_iow2posix_flags(flags);
        if ( (retval = open(self->path,pflags))>0){
           self->fd= retval;
            self->flags=pflags;
//            MMDEBUG(IOW,"open setting fd[%d]\n",self->fd);
        }else{
            MERROR("open failed [%d/%s]\n",errno,strerror(errno));
            self->fd=-1;
        }
    }else{
        MERROR("invalid argument\n");
    }
    return retval;
}
// End function iow_open


/// @fn int iow_mopen(iow_file_t * self, iow_flags_t flags, iow_mode_t mode)
/// @brief open file, specify permission modes.
/// @param[in] self file reference
/// @param[in] flags flag values
/// @param[in] mode permission flags
/// @return file descriptor (>0) on success, -1 otherwise
int iow_mopen(iow_file_t *self,iow_flags_t flags,iow_mode_t mode )
{
    int retval=-1;
    if (NULL!=self && NULL!=self->path) {
        int pflags=s_iow2posix_flags(flags);
        mode_t pmode=s_iow2posix_mode(mode);
//        MMDEBUG(IOW,"opening [%s] mode[%x] pmode[%x]\n",self->path,mode,pmode);
        if ( (retval = open(self->path,pflags,pmode) )>0){
            self->fd= retval;
            self->flags=pflags;
            self->mode=pmode;
//            MMDEBUG(IOW,"open setting fd[%d]\n",self->fd);
        }else{
            MERROR("open failed [%d/%s]\n",errno,strerror(errno));
            self->fd=-1;
        }
    }else{
        MERROR("invalid argument\n");
    }
    return retval;
}
// End function iow_mopen


/// @fn int iow_close(iow_file_t * self)
/// @brief close a file.
/// @param[in] self file instance
/// @return 0 on success, -1 otherwise.
int iow_close(iow_file_t *self)
{
    int retval=-1;
    if (NULL!=self && self->fd>0) {
        retval = close(self->fd);
        self->fd = -1;
    }
    return retval;
}
// End function iow_close

/// @fn int iow_rename(iow_file_t * self, const char * path)
/// @brief rename file. closes and reopens file; may change underlying
/// file descriptor/handle.
/// @param[in] self file
/// @param[in] path new name
/// @return new file descriptor/handle (>0) on success, -1 otherwise
int iow_rename(iow_file_t *self,const char *path)
{
    int retval = -1;
    
    if (NULL!=self && NULL!=path) {
        
        if (self->fd>0) {
            iow_close(self);
            self->fd=-1;
        }

        if (NULL!=self->path) {
            free(self->path);
        }
        
//       MMDEBUG(IOW,"renaming to %s\n",path);
        self->path = strdup(path);
        
        if ( (retval = open(self->path,self->flags|O_CREAT,self->mode|S_IWUSR|S_IRUSR) )>0){
            self->fd = retval;
//            MMDEBUG(IOW,"opened %s fd[%d]\n",self->path,self->fd);
            //            MMDEBUG(IOW,"open setting fd[%d]\n",self->fd);
        }else{
            MERROR("open %s failed [%d/%s]\n",self->path,errno,strerror(errno));
            self->fd=-1;
        }

    }else{
        MERROR("invalid arguments\n");
    }
    return retval;
}
// End function iow_rename


/// @fn int64_t iow_seek(iow_file_t *self, uint32_t ofs)
/// @brief move cursor to specified offset
/// @param[in] self file instance
/// @param[in] ofs offset
/// @return number of bytes read >=0 on success, or -1 otherwise.
int64_t iow_seek(iow_file_t *self, uint32_t ofs, iow_whence_t whence)
{
    int64_t retval=-1;
    if (NULL != self) {
        off_t test=0;
        off_t offset=ofs;
        int pwhence=-1;
        switch (whence) {
            case IOW_SET:
                pwhence=SEEK_SET;
                break;
            case IOW_CUR:
                pwhence=SEEK_CUR;
                break;
            case IOW_END:
                pwhence=SEEK_END;
                break;
            default:
                break;
        }
        if( (test=lseek(self->fd, offset, pwhence))>=0){
            retval = (int64_t)test;
        }else{
            MERROR("seek failed [%d/%s]\n",errno,strerror(errno));
        }
    }else{
        MERROR("invalid argument\n");
    }
    return retval;

}
// End function iow_seek

/// @fn int64_t iow_read(iow_file_t * self, byte * dest, uint32_t len)
/// @brief read bytes from file (advances input pointer).
/// @param[in] self file instance
/// @param[in] dest data buffer
/// @param[in] len bytes to read
/// @return number of bytes read >=0 on success, or -1 otherwise.
int64_t iow_read(iow_file_t *self, byte *dest, uint32_t len)
{
    int64_t retval=-1;
    if (NULL != self && NULL!=dest && len!=0) {
        ssize_t test=0;
        if( (test=read(self->fd, dest, len))>0){
            retval = (int64_t)test;
        }else{
            MERROR("read failed [%d/%s]\n",errno,strerror(errno));
        }
    }else{
        MERROR("invalid argument\n");
    }
    return retval;
}
// End function iow_read


/// @fn int64_t iow_write(iow_file_t * self, byte * src, uint32_t len)
/// @brief write bytes to file (advances input pointer.
/// @param[in] self file instance
/// @param[in] dest data buffer
/// @param[in] len bytes to write
/// @return bytes written on success (>=0), -1 otherwise
int64_t iow_write(iow_file_t *self, byte *src, uint32_t len)
{
    int64_t retval=-1;
    if (NULL != self && NULL!=src && len>0) {
        ssize_t test=0;
        if( (test=write(self->fd, src, len))>0){
            retval = (int64_t)test;
        }else{
            MERROR("read failed [%d/%s]\n",errno,strerror(errno));
        }
    }else{
        MERROR("invalid argument\n");
    }
    return retval;
}
// End function iow_write


/// @fn int iow_ftruncate(iow_file_t * self, uint32_t len)
/// @brief truncate file to specified length.
/// @param[in] self file instance
/// @param[in] len len to truncate to
/// @return 0 on success, -1 otherwise
int iow_ftruncate(iow_file_t *self, uint32_t len)
{
    int retval=-1;
    if (NULL != self){
    	retval = ftruncate(self->fd,len);
    }
    return retval;
}
// End function iow_ftruncate


/// @fn int iow_fprintf(iow_file_t * self, char * fmt, ...)
/// @brief formatted print to file.
/// @param[in] self file instance
/// @param[in] fmt print format (e.g. stdio printf)
/// @param[in] args print arguments
/// @return number of bytes output on success, -1 otherwise.
int iow_fprintf(iow_file_t *self, char *fmt, ...)
{
    int retval=-1;
    
    if(NULL != self){
        //get the arguments
        va_list args;
        va_start(args, fmt);
        
        retval=vdprintf(self->fd,fmt,args);

        va_end(args);
    }
    return retval;
}
// End function iow_fprintf


/// @fn int iow_vfprintf(iow_file_t * self, char * fmt, va_list args)
/// @param[in] self file instance
/// @param[in] fmt print format (e.g. stdio printf)
/// @param[in] args print arguments (as va_list - posix, supported on many platforms)
/// @return number of bytes output on success, -1 otherwise.
int iow_vfprintf(iow_file_t *self, char *fmt, va_list args)
{
    int retval=-1;
    
    if(NULL != self && self->fd>0){
        va_list cargs;
        va_copy(cargs,args);
        retval=vdprintf(self->fd,fmt,cargs);
    }else{
        MERROR("invalid argument\n");
    }
    return retval;
}
// End function iow_vfprintf


/// @fn int iow_flush(iow_file_t * self)
/// @brief flush, attempt to sync to disk.
/// calls fsync; may not actually force write to disk until closed
/// @param[in] self file reference
/// @return 0 on success, -1 otherwise
int iow_flush(iow_file_t *self)
{
    int retval=-1;
    if (NULL!=self && self->fd>0) {
        retval=fsync(self->fd);
    }else{
        MERROR("invalid argument\n");
    }
    return retval;
    
}
// End function iow_flush


/// @fn int64_t iow_fsize(iow_file_t * self)
/// @brief file size.
/// @param[in] self file instance
/// @return number of bytes on disk on success, -1 otherwise
int64_t iow_fsize(iow_file_t *self)
{
    int64_t retval=-1;
    struct stat info={0};
    if (NULL!=self) {
        if (stat(self->path,&info)==0) {
            retval=(int64_t)info.st_size;
        }else{
            MERROR("stat failed[%d/%s]\n",errno,strerror(errno));
        }
    }else{
        MERROR("invalid argument\n");
    }
    return retval;
}
// End function iow_fsize


/// @fn time_t iow_mtime(const char * path)
/// @brief return modification time of file (seconds since 1/1/70).
/// @param[in] path file path
/// @return modification time of file on success, -1 otherwise
time_t iow_mtime(const char *path)
{
    time_t retval=-1;
    struct stat info={0};
    if (NULL!=path) {
        if (stat(path,&info)==0) {
            retval=info.st_mtime;
        }else{
            MERROR("stat failed[%d/%s]\n",errno,strerror(errno));
        }
    }else{
        MERROR("invalid argument\n");
    }
    return retval;
}
// End function iow_mtime


// iow mutex API
/// @fn iow_mutex_t * iow_mutex_new()
/// @brief create and initialize new mutex.
/// @return mutex reference on success, or NULL otherwise
iow_mutex_t *iow_mutex_new()
{
    iow_mutex_t *self = (iow_mutex_t *)malloc(sizeof(iow_mutex_t));
    if (self) {
        //self->m = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_init(&self->m, NULL);
    }
    return self;
}
// End function iow_mutex_new

/// @fn void iow_mutex_destroy(iow_mutex_t ** pself)
/// @brief release mutex resources.
/// @param[in] pself pointer to mutex reference
/// @return none
void iow_mutex_destroy(iow_mutex_t **pself)
{
    if (pself) {
        iow_mutex_t *self = *pself;
        if (self) {
            pthread_mutex_destroy(&self->m);
            free(self);
            *pself=NULL;
        }
    }
}
// End function iow_mutex_destroy


/// @fn int iow_mutex_lock(iow_mutex_t * self)
/// @brief lock a mutex.
/// @param[in] self mutex reference
/// @return 0 on success, -1 otherwise
int iow_mutex_lock(iow_mutex_t *self)
{
    int retval=-1;
    if (self) {
        retval=pthread_mutex_lock(&self->m);
    }
    return retval;
}
// End function iow_mutex_lock

/// @fn int iow_mutex_unlock(iow_mutex_t * self)
/// @brief unlock mutex.
/// @param[in] self mutex reference
/// @return 0 on success, -1 otherwise
int iow_mutex_unlock(iow_mutex_t *self)
{
	int retval=-1;
	if (self) {
 	   retval=pthread_mutex_unlock(&self->m);
    }
	return retval;
}
// End function iow_mutex_unlock



#ifdef WITH_TEST

void *iow_test_svr(void *arg)
{
    iow_socket_t *s=(iow_socket_t *)arg;
    char buf[ADDRSTR_BYTES]={0};
    struct timeval tv;
    fd_set master;
    fd_set read_fds;
    int fdmax;
    int stat=0;
    char iobuf[256]; // buffer for client data
    int nbytes;
    struct sockaddr_storage client_addr={0};
    socklen_t addr_size=0;
    
    MMINFO(IOW,"Test server [%s] - starting\n",iow_addr2str(s,buf,ADDRSTR_BYTES));
    iow_listen(s);
    
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    
    FD_ZERO(&read_fds);
    FD_ZERO(&master);
    FD_SET(s->fd,&master);
    fdmax = s->fd;
    bool stop=false;
    
    while (!stop ) {
        read_fds = master;
        MMINFO(IOW,"pending on select\n");
        if( (stat=select(fdmax+1, &read_fds, NULL, NULL, &tv)) != -1){
            int newfd=-1;
            for (int i=s->fd; i<=fdmax; i++) {
                
                if (FD_ISSET(i, &read_fds)){
                    MMINFO(IOW,"readfs [%d/%d] selected\n",i,fdmax);
                    
                    if (i==s->fd) {
                        MMINFO(IOW,"server main listener [%d] got request\n",i);
                        
                        newfd = accept(s->fd, (struct sockaddr *)&client_addr, &addr_size);
                        if (newfd != -1) {
                            MMINFO(IOW,"server recieved connection from client on socket [%d]\n",newfd);
                            FD_SET(newfd,&read_fds);
                            if (newfd>fdmax) {
                                fdmax=newfd;
                            }
                        }else{
                            // accept failed
                            MMINFO(IOW,"accept failed [%d/%s]\n",errno,strerror(errno));
                        }
                    }else{
                        MMINFO(IOW,"server waiting for client data fd[%d]\n",i);
                        if (( nbytes = recv(i, iobuf, sizeof(iobuf), 0)) <= 0) {
                            MMINFO(IOW,"handle client data fd[%d] nbytes[%d]\n",i,nbytes);
                            // got error or connection closed by client
                            if (nbytes == 0) {
                                // connection closed
                                fprintf(stderr,"ERR - socket %d closed\n", i);
                            } else if(nbytes<0) {
                                fprintf(stderr,"ERR - recv failed socket[%d] [%d/%s]\n",i,errno,strerror(errno));
                            }
                        }else{
                            MMINFO(IOW,"server received data on socket [%d] [%s] len[%d]\n",i,iobuf,nbytes);
                            if (strncmp(iobuf,"STOP",4)==0) {
                                stop=true;
                            }
                            send(i,"ACK",strlen("ACK"),0);
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    }
                }else{
                    MMINFO(IOW,"readfs fd[%d/%d] ISSET:%s\n",i,fdmax,(FD_ISSET(i,&read_fds)?"TRUE":"FALSE"));
                }
            }
        }else{
            // select failed
            MMINFO(IOW,"select failed [%d/%s]\n",errno,strerror(errno));
        }
    }
    if (stop) {
        MMINFO(IOW,"Test server - interrupted - stop flag set\n");
        s->status=1;
    }else{
        MMINFO(IOW,"Test server - normal exit\n");
        s->status=0;
    }
    pthread_exit((void *)&s->status);
}

#endif // WITH_TEST

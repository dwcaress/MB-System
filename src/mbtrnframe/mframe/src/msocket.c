///
/// @file msocket.c
/// @authors k. Headley
/// @date 01 jan 2018
 
/// mframe cross-platform socket wrappers implementation
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

#include "mframe.h"
#include "msocket.h"
#include "mtime.h"
#include "merror.h"
#include "medebug.h"

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


#if defined(__QNX__)
#pragma message("WARNING - MSOCKET API not implemented for QNX")

#else
/////////////////////////
// Declarations
/////////////////////////

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////
int g_msocket_debug_level=0;

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn void msock_set_debug(msock_socket_t *s, int level)
/// @brief set module debug level (enable extra module debug output)
/// @param[in] s socket instance
/// @param[in] level debug output level >=0
/// @return none
void msock_set_debug(int level)
{
    g_msocket_debug_level=level;
}

/// @fn int msock_set_blocking(msock_socket_t * s, _Bool enabled)
/// @brief configure socket to block or not block.
/// @param[in] s socket instance
/// @param[in] enabled true for blocking, false for non-blocking
/// @return 0 on success, -1 otherwise
int msock_set_blocking(msock_socket_t *s, bool enabled)
{
    int retval=-1;
    if (NULL != s) {
        int flags = fcntl(s->fd, F_GETFL, 0);
        if (flags != -1){
            flags = (enabled ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK));
            retval = fcntl(s->fd, F_SETFL, flags);
        }// else error
    }// else invalid arg
    
    return retval;
}
// End function msock_set_blocking

/// @fn msock_addr_t * msock_addr_new()
/// @brief create new IP address.
/// should be destroyed with msock_addr_destroy.
/// @return reference to new address
msock_addr_t *msock_addr_new()
{
    msock_addr_t *self = (msock_addr_t *)malloc(sizeof(msock_addr_t));
    if (self) {
        memset(self,0,sizeof(msock_addr_t));
        self->ainfo=NULL;
        self->alist=NULL;
        self->host=NULL;
    }
    return self;
}
// End function msock_addr_new

/// @fn void msock_addr_destroy(msock_addr_t ** pself)
/// @brief release resources for msock_address instance.
/// @param[in] pself pointer to instance
/// @return none
void msock_addr_destroy(msock_addr_t **pself)
{
    if (NULL!=pself) {
        msock_addr_t *self = *pself;
        if (NULL!=self) {
            if (NULL!=self->alist) {
                // alist used to hold UDP reslist
                // [i.e. getaddrinfo],
                // ainfo points to list.
                // use freeaddrinfo to free list
                freeaddrinfo(self->alist);
            }else if(NULL!=self->ainfo && NULL==self->alist){
                // alist not used, ainfo holds connection addr
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
// End function msock_addr_destroy

/// @fn void msock_addr_init(msock_addr_t * self)
/// @brief initialize address instance (zeros members).
/// @param[in] self address instance
/// @return none
void msock_addr_init(msock_addr_t *self)
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
        self->ainfo->ai_addr = (struct sockaddr *)malloc(MSOCK_ADDR_LEN);
        memset(self->ainfo->ai_addr,0,MSOCK_ADDR_LEN);
    }
}
// End function msock_addr_init

/// @fn char *msock_addr2str(msock_socket_t *s, char *dest, size_t len)
/// @brief return address as string
/// @param[in] s socket reference
/// @param[in] dest destination buffer
/// @param[in] len destination size (bytes)
/// @return none
char *msock_addr2str(msock_socket_t *s, char *dest, size_t len)
{
    return NULL;
}
// End function msock_addr_init

/// @fn int msock_connection_addr2str(msock_connection_t *self)
/// @brief convert address to host and service strings
/// @param[in] self socket reference
/// @return >0 on success, -1 otherwise
int msock_connection_addr2str(msock_connection_t *self)
{
    int retval=-1;
    const char *ctest=NULL;
    struct sockaddr_in *psin = NULL;
    
    if (NULL!=self && NULL != self->addr &&
        NULL != self->addr->ainfo &&
        NULL != self->addr->ainfo->ai_addr) {
        
        psin = (struct sockaddr_in *)self->addr->ainfo->ai_addr;
        ctest = inet_ntop(AF_INET, &psin->sin_addr, self->chost, MSOCK_ADDR_LEN);
        
        if (NULL!=ctest) {

            uint16_t port = ntohs(psin->sin_port);
            int svc = port;
            
            snprintf(self->service,NI_MAXSERV,"%d",svc);
            retval=svc;
        }else{
            PEPRINT((stderr,"inet_ntop failed peer [%d %s]\n",errno,strerror(errno)));
        }
    }else{
        PEPRINT((stderr,"invalid arguments self[%p] addr[%p] ainfo[%p]\n",self,
                 (NULL!=self ? self->addr : NULL),
                 ( (NULL!=self && NULL!=self->addr) ? self->addr->ainfo : NULL)));
    }

    return retval;
}
// End function msock_connection_addr2str


/// @fn msock_connection_t * msock_connection_new()
/// @brief create new network connection (e.g. UDP client).
/// caller should release resources using msock_connection_destroy.
/// @return connection instance reference
msock_connection_t *msock_connection_new()
{
    msock_connection_t *self = (msock_connection_t *)malloc(sizeof(msock_connection_t));
    if (self) {
        memset(self,0,sizeof(msock_connection_t));
        self->addr=msock_addr_new();
        msock_addr_init(self->addr);
        self->sock=NULL;
    }
    return self;
}
// End function msock_connection_new

/// @fn void msock_connection_destroy(msock_connection_t ** pself)
/// @brief release connection resources.
/// @param[in] pself pointer to instance reference.
/// @return none
void msock_connection_destroy(msock_connection_t **pself)
{
    if (NULL!=pself) {
        msock_connection_t *self = *pself;
        if (NULL!=self) {
            msock_addr_destroy(&self->addr);
            msock_socket_destroy(&self->sock);
            free(self);
            *pself=NULL;
        }
    }
}
// End function msock_connection_destroy

/// @fn void msock_connection_free(void * pself)
/// @brief free function (use as autofree function in mlist).
/// @param[in] pself TBD
/// @return none
/// @sa mlist.h
void msock_connection_free(void *pself)
{
    if (NULL!=pself) {
        msock_connection_t *self = (msock_connection_t *)pself;
        msock_connection_destroy(&self);
        free(self);
        self=NULL;
    }
}
// End function msock_connection_free

/// @fn void msock_pstats_show(msock_pstats_t * self, _Bool verbose, uint16_t indent)
/// @brief output reader parameter summary to stderr.
/// @param[in] self stats reference
/// @param[in] verbose provide verbose output
/// @param[in] indent output indentation (spaces)
/// @return none
void msock_pstats_show(msock_pstats_t *self, bool verbose, uint16_t indent)
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
// End function msock_pstats_show

/// @fn msock_socket_t * msock_socket_new(const char * host, int port, msock_socket_ctype type)
/// @brief create new socket instance.
/// @param[in] host connection IP address
/// @param[in] port connection IP port
/// @param[in] type ST_UDP or ST_TCP
/// @return new socket instance reference.
msock_socket_t *msock_socket_new(const char *host, int port, msock_socket_ctype type)
{
    msock_socket_t *self = (msock_socket_t *)malloc(sizeof(msock_socket_t));
    if (self) {
        self->fd=-1;
        self->status=0;
        self->type = type;
        self->addr = msock_addr_new();
        msock_configure(self,host,port,type);
    }
    return self;
}
// End function msock_socket_new
msock_socket_t *msock_socket_wnew(const char *host, int port, msock_socket_ctype type)
{
    msock_socket_t *self = (msock_socket_t *)malloc(sizeof(msock_socket_t));
    if (self) {
        self->fd=-1;
        self->status=0;
        self->type = type;
        self->addr = msock_addr_new();
//        msock_configure(self,host,port,type);
    }
    return self;
}
// End function msock_socket_new

/// @fn void msock_socket_destroy(msock_socket_t ** pself)
/// @brief release socket resources.
/// @param[in] pself pointer to instance reference
/// @return none
void msock_socket_destroy(msock_socket_t **pself)
{
    if (pself) {
        msock_socket_t *self = *pself;
        if (self) {
            close(self->fd);
            msock_addr_destroy(&self->addr);
           free(self);
            *pself=NULL;
        }
    }
}
// End function msock_socket_destroy

/// @fn int msock_configure(msock_socket_t * s, const char * host, int port, msock_socket_ctype type)
/// @brief configure a socket instance.
/// @param[in] s socket reference
/// @param[in] host connection host
/// @param[in] port connection port
/// @param[in] type ST_UDP, ST_TCP
/// @return 0 on success, -1 otherwise.
int msock_configure(msock_socket_t *s, const char *host, int port, msock_socket_ctype type)
{
    int retval=-1;
    if (NULL != host) {
        if (NULL != (s->addr->host)) {
            free(s->addr->host);
            s->addr->host=NULL;
        }
        s->addr->host=strdup(host);
    }
    
    s->addr->port=port;

    memset(s->addr->portstr,0,PORTSTR_BYTES*sizeof(char));
    sprintf(s->addr->portstr,"%d",port);
    
    memset(&s->addr->hints,0,sizeof(struct addrinfo));
    PDPRINT((stderr,"configuring type [%s]\n",(type==ST_TCP ? "SOCK_STREAM" : "SOCK_DGRAM")));
    s->addr->hints.ai_family=PF_INET;
    s->addr->hints.ai_socktype=(type==ST_TCP ? SOCK_STREAM : SOCK_DGRAM);
    s->addr->hints.ai_flags=AI_PASSIVE;
    s->addr->hints.ai_protocol=0;
    s->addr->hints.ai_canonname=NULL;
    s->addr->hints.ai_addr=NULL;
    s->addr->hints.ai_next=NULL;
    
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
////            PDPRINT((stderr,"rp[%p]\n",rp));
//            fprintf(stderr,"msock_configure - rp family[%s] type[%s]\n",
//                    (rp->ai_family==AF_INET?"IPv4":"IPv6"),
//                    (rp->ai_socktype==SOCK_STREAM?"TCP":"UDP"));
            
            s->fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (s->fd>0){
                s->addr->ainfo = rp;

//                int sokeepalive=1;
//                setsockopt(s->fd,SOL_SOCKET,SO_KEEPALIVE,&sokeepalive,sizeof(sokeepalive));
//                struct linger solinger={0};
//                setsockopt(s->fd,SOL_SOCKET,SO_LINGER,&solinger,sizeof(solinger));
#if defined(__APPLE__)
                // this is the OSX alternative to the MSG_NOSIGNAL flag for send()
                int so_nosigpipe=1;
                setsockopt(s->fd,SOL_SOCKET,SO_NOSIGPIPE,&so_nosigpipe,sizeof(so_nosigpipe));
#endif
                PDPRINT((stderr,"socket created[%d] ainfo[%p] alist[%p]\n",s->fd,s->addr->ainfo,s->addr->alist));
                retval=0;
                break;
            }else {
                fprintf(stderr, "socket request failed [%d/%s]\n", errno,strerror(errno));
            }
            close(s->fd);
            s->fd=-1;
        }
    }else{
        fprintf(stderr, "getaddrinfo error: %d/%s\n",status, gai_strerror(status));
    }
    
    return retval;
}
// End function msock_configure

/// @fn int msock_connect(msock_socket_t * s)
/// @brief connect (to server) socket.
/// @param[in] s socket instance.
/// @return 0 on success, -1 otherwise.
int msock_connect(msock_socket_t *s)
{
    int retval=-1;
    if ( (NULL != s) && (NULL != s->addr->ainfo)) {
        if (connect(s->fd, s->addr->ainfo->ai_addr, s->addr->ainfo->ai_addrlen)==0) {
            // success
#ifdef WITH_PDEBUG
            char buf[ADDRSTR_BYTES]={0};
            PDPRINT((stderr,"%s - connect OK [%s]\n",__func__,msock_addr2str(s,buf,ADDRSTR_BYTES)));
#endif
            retval=0;
        }else{
            fprintf(stderr,"%s - connect failed fd[%d] [%d/%s]\n",__func__,s->fd,errno,strerror(errno));
        }
    }else{
        fprintf(stderr,"%s - invalid argument s[%p] ainfo[%p]\n",__func__,s,(s?s->addr->ainfo:0));
    }
    return retval;
}
// End function msock_connect

/// @fn int msock_bind(msock_socket_t * s)
/// @brief bind (server) socket to port.
/// @param[in] s socket instance
/// @return 0 on success, -1 otherwise
int msock_bind(msock_socket_t *s)
{
    int retval=-1;
    if (NULL != s && s->fd>0 && NULL != s->addr->ainfo) {

        if(bind(s->fd, s->addr->ainfo->ai_addr, s->addr->ainfo->ai_addrlen) == 0){
            retval=0;
        }else{
            fprintf(stderr,"bind failed fd[%d] [%d/%s] %s \n",s->fd,errno,strerror(errno),(errno==EINVAL?"already bound?":""));
        }
    }else{
        fprintf(stderr,"%s - invalid arguments\n",__FUNCTION__);
    }
    return retval;
}
// End function msock_bind

/// @fn int msock_listen(msock_socket_t *s, int queue)
/// @brief listen for connections on (server) socket.
/// @param[in] s socket instance
/// @param[in] queue queue depth
/// @return 0 on success, -1 otherwise
int msock_listen(msock_socket_t *s, int queue)
{
    int retval=-1;
    if (NULL != s && NULL != s->addr->ainfo) {
        const int optionval = 1;
        setsockopt(s->fd, SOL_SOCKET, SO_REUSEADDR, &optionval, sizeof(optionval));
	//        MSOCK_MAX_QUEUE
        if (listen(s->fd, queue)==0) {
            // success
#ifdef WITH_PDEBUG
            char buf[ADDRSTR_BYTES]={0};
            PDPRINT((stderr,"%s - listening [%s] queue[%d]\n",__FUNCTION__,msock_addr2str(s,buf,ADDRSTR_BYTES),queue));
#endif
            retval=0;
        }else{
            fprintf(stderr,"listen failed fd[%d] [%d/%s]\n",s->fd,errno,strerror(errno));
        }
    }else{
        fprintf(stderr,"%s - invalid arguments\n",__FUNCTION__);
    }
    return retval;
}
// End function msock_listen

/// @fn int msock_accept(msock_socket_t * s, msock_addr_t *addr)
/// @brief accept (client) connection on socket.
/// @param[in] s socket instance
/// @param[out] addr address of connecting peer
/// @return file descriptor (>0) on success, -1 otherwise
int msock_accept(msock_socket_t *s,msock_addr_t *addr)
{
    int retval=-1;
    
    if (NULL != s && NULL != s->addr->ainfo){
        
        struct sockaddr *dest_addr=(addr==NULL?NULL:addr->ainfo->ai_addr);
        socklen_t addrlen = (addr==NULL?0:MSOCK_ADDR_LEN);
 
        int new_fd = accept(s->fd, (struct sockaddr *)dest_addr, &addrlen);
        if (new_fd != -1) {
            PDPRINT((stderr,"accept received connection from client on socket new_fd[%d]\n",new_fd));
            retval = new_fd;
        }else{
            // accept failed
//            PDPRINT((stderr,"accept failed fd[%d] dest_addr[%p] addrlen[%d] [%d/%s]\n",s->fd,dest_addr,addrlen,errno,strerror(errno)));
        }
    }else{
        fprintf(stderr,"%s - invalid arguments\n",__FUNCTION__);
    }
    return retval;
}
// End function msock_accept

/// @fn int64_t msock_send(msock_socket_t *s, byte *buf, uint32_t len)
/// @brief send data via socket.
/// @param[in] s socket instance
/// @param[in] buf data buffer
/// @param[in] len number of bytes to send
/// @return number of bytes sent on success, -1 otherwise
int64_t msock_send(msock_socket_t *s,byte *buf, uint32_t len)
{
    int64_t retval=-1;
    if (NULL != s && NULL != buf && len>0) {
        if (s->type==ST_TCP) {
            //                fprintf(stderr,">>>>>>>>>>>>>>>>> MSOCK_SEND : buf[%p] len[%"PRIu32"]\n",buf,len);
            //                r7k_hex_show(buf,len,16,true,5);
            
#if defined(__APPLE__)
            // must also set socket option SO_NOSIGPIPE
            if( (retval = send(s->fd,buf,len,0))<=0){
                fprintf(stderr,"ERR - send fd[%d] returned %"PRId64" [%d/%s]\n",s->fd,retval,errno,strerror(errno));
            }
#else
            if( (retval = send(s->fd,buf,len,MSG_NOSIGNAL))<=0){
                fprintf(stderr,"ERR - send fd[%d] returned %"PRId64" [%d/%s]\n",s->fd,retval,errno,strerror(errno));
            }
#endif
        }else{
            fprintf(stderr,"%s - invalid arguments (!TCP)\n",__func__);
        }
    }else{
        fprintf(stderr,"%s - invalid arguments\n",__FUNCTION__);
    }
    return retval;
}
// End function msock_send

/// @fn int64_t msock_sendto(msock_socket_t * s, msock_addr_t * addr, byte * buf, uint32_t len, int32_t flags)
/// @brief send data via (UDP) socket.
/// @param[in] s socket instance
/// @param[in] addr address
/// @param[in] buf data buffer
/// @param[in] len number of bytes to send
/// @param[in] flags optional flags
/// @return number of bytes sent on success, -1 otherwise.
int64_t msock_sendto(msock_socket_t *s, msock_addr_t *addr, byte *buf, uint32_t len, int32_t flags)
{
    int64_t retval=-1;
if (NULL != s && NULL != buf && len>0) {
    struct sockaddr *dest_addr = NULL;
    if (NULL!=addr && NULL!=addr->ainfo) {
        dest_addr = addr->ainfo->ai_addr;
    }
    
    if( (retval = sendto(s->fd,buf,len,flags,dest_addr,MSOCK_ADDR_LEN)) > 0){
        //                    PDPRINT((stderr,"sendto OK [%lld]\n",retval));
    }else{
        //                    fprintf(stderr,"ERR - sendto returned %lld [%d/%s]\n",retval,errno,strerror(errno));
    }
}else{
        fprintf(stderr,"%s - invalid arguments\n",__FUNCTION__);
    }
    return retval;
}
// End function msock_sendto

/// @fn int64_t msock_recv(msock_socket_t * s, byte * buf, uint32_t len)
/// @brief receive bytes on socket.
/// @param[in] s socket instance
/// @param[in] buf destination buffer
/// @param[in] len number of bytes to receive
/// @return number of bytes received on success, -1 otherwise
int64_t msock_recv(msock_socket_t *s, byte *buf, uint32_t len, int flags)
{
    int64_t retval=-1;
    if (NULL != s && NULL!= buf && len>0) {
        retval = recv(s->fd,buf,len,flags);
    }else{
        fprintf(stderr,"%s - invalid arguments\n",__FUNCTION__);
    }
    return retval;
}
// End function msock_recv

/// @fn int64_t int64_t msock_recvfrom(msock_socket_t *s, msock_addr_t *addr, byte *buf, uint32_t len, int flags)
/// @brief receive datagram using specified socket.
/// @param[in] s socket instance
/// @param[in] addr address
/// @param[in] buf destination buffer
/// @param[in] len number of bytes to receive
/// @param[in] flags special control flags
/// @return number of bytes received on success, -1 otherwise
int64_t msock_recvfrom(msock_socket_t *s, msock_addr_t *addr, byte *buf, uint32_t len, int flags)
{
    int64_t retval= 0;
    
    if (NULL != s && NULL!=buf && len>0) {
        struct sockaddr *dest_addr=(addr==NULL || addr->ainfo==NULL ? NULL:addr->ainfo->ai_addr);
        socklen_t addrlen = (addr==NULL?0:MSOCK_ADDR_LEN);
        
//        fprintf(stderr,"recvfrom connection[%p] dest_addr[%p] ai_family[%d] addrlen[%d]\n",addr,(addr?addr->ainfo->ai_addr:NULL),(int)(addr?addr->ainfo->ai_family:-1),(int)(addr?addr->ainfo->ai_addrlen:-1));
        
        if( (retval = recvfrom(s->fd,buf,len,flags,dest_addr,&addrlen))>0){
            // PDPRINT((stderr,"received data connection[%p] dest[%p] ainfo[%p] [%lld]\n",addr,dest_addr,addr->ainfo,retval));
        }
        else{
            if(g_msocket_debug_level>0)
            PDPRINT((stderr,"recvfrom failed [%d %s]\n",errno,strerror(errno)));
        }
    }else{
        fprintf(stderr,"%s - invalid arguments\n",__FUNCTION__);
    }
    return retval;
}
// End function msock_recvfrom

/// @fn int64_t msock_read_tmout(msock_socket_t * s, byte * buf, uint32_t len, uint32_t timeout_msec)
/// @brief read bytes from socket until length or timeout exceeded (for non-blocking socket).
/// @param[in] s socket instance
/// @param[in] buf data buffer
/// @param[in] len max bytes to receive
/// @param[in] timeout_msec timeout (milliseconds)
/// @return number of bytes received on success, -1 otherwise
int64_t msock_read_tmout(msock_socket_t *s, byte *buf, uint32_t len, uint32_t timeout_msec)
{
    me_errno=ME_OK;
    int64_t retval=0;
    double t_rem=(double)timeout_msec;
    int64_t read_total=0;
    
    double start_sec   = mtime_dtime();
    double to_sec      = (double)timeout_msec/1000.0;

     if ( (NULL!=s) && s->fd>0 && (NULL!=buf) && len>0) {
         
         bool err_quit=false;
        double elapsed_sec = 0.0;
        byte *pbuf=buf;
        memset(buf,0,len);
         
      
//         fprintf(stderr,"#### read_tmout - fd[%d] buf[%p] len[%"PRIu32"] to[%"PRIu32"]\n",s->fd,buf,len, timeout_msec);

//         fprintf(stderr,"#### read_tmout - buf[%p] len[%"PRIu32"] to[%"PRIu32"]\n",buf,len, timeout_msec);
        while (err_quit==false && read_total<len && elapsed_sec<to_sec && pbuf<(buf+len) ) {

            
            // read from the file/socket
           int64_t nbytes = read(s->fd, pbuf, (len-read_total));

//            fprintf(stderr,"read returned [%"PRId64"/%"PRId64"] [%d/%s]\n",nbytes,(len-read_total),errno,strerror(errno));

            if (nbytes > 0) {
                read_total+=nbytes;
                pbuf+=nbytes;
                retval=read_total;
//                fprintf(stderr,"read_total %"PRId64"\n",read_total);
            }else{
                // got error or connection closed by server
                
                // when server side disconnects
                // read and recv both return 0 or -1 and set one
                // of several error conditions. This ambiguity (that it
                // may return 0) can be resolved via error values.
                // The errors differ depending on how the server socket
                // is shutdown (i.e. test client calls close, not sure what
                // 7k center does, but they generate different errors)
                // Also, the errors extend beyond those explicitly
                // mentioned by the read manpage, though it does say
                // that other errors may be indicated.
                // when the server stops sending (and is connected)
                // FD_ISSET returns 0/false (which is correct).
                //
                // The errors that indicate socket shutdown include
                // EWOULDBLOCK/EAGAIN (these may share the same errno)
                // [errno 11/Resource Temporarily Unavailable]
                // ENOTCONN
                // [errno 107/Transport endpoint is not connected]
                // ECONNREFUSED
                // [errno 111/Connection refused]
                // ECONNRESET
                // [errno 104/Connection reset by peer]
                // ENOENT
                // [errno 2/No such file or directory]
                
                fprintf(stderr,"ERR - read[%"PRId64"] sock[%d] [%d/%s]\n", nbytes, s->fd,errno,strerror(errno));
                
                switch (errno) {
                    case 0:
                        fprintf(stderr,"read 0 (EOF) setting EOF %d\n", s->fd);
                        me_errno=ME_EOF;
                        retval=-1;
                        break;
                    case EWOULDBLOCK:
                        fprintf(stderr,"EAGAIN/EWOULDBLOCK setting socket error %d\n", s->fd);
                        me_errno=ME_ESOCK;
                        retval=-1;
                       break;
                        
                    case ENOTCONN:
                        fprintf(stderr,"ENOTCONN socket %d setting socket error\n", s->fd);
                        me_errno=ME_ESOCK;
                        retval=-1;
                        break;
                    case EINVAL:
                        fprintf(stderr,"EINVAL socket %d setting socket error\n", s->fd);
                        me_errno=ME_ESOCK;
                        retval=-1;
                        break;
                    case EBADF:
                        fprintf(stderr,"EBADF socket %d\n", s->fd);
                        me_errno=ME_ESOCK;
                        retval=-1;
                        break;
                    case EIO:
                        fprintf(stderr,"EIO socket %d\n", s->fd);
                        break;
                    case EFAULT:
                        fprintf(stderr,"EFAULT socket %d\n", s->fd);
                        break;
                    case EINTR:
                        fprintf(stderr,"EINTR socket %d\n", s->fd);
                        break;
                    case ENOENT:
                        fprintf(stderr,"ENOENT socket %d setting socket error\n", s->fd);
                        me_errno=ME_ESOCK;
                        retval=-1;
                        break;
                    case ECONNRESET:
                        fprintf(stderr,"ECONNRESET socket %d setting socket error\n", s->fd);
                        me_errno=ME_ESOCK;
                        retval=-1;
                        break;
                    case ETIMEDOUT:
                        fprintf(stderr,"ETIMEOUT socket %d setting timeout error\n", s->fd);
                        me_errno=ME_ETMOUT;
                        retval=-1;
                        break;
                    default:
                        fprintf(stderr,"read: socket %d unrecognized err [%d/%s] setting socket error\n", s->fd,errno,strerror(errno));
                        me_errno=ME_ESOCK;
                        retval=-1;
                        break;
                }
                if (me_errno==ME_ESOCK || me_errno==ME_EOF || me_errno==ME_ETMOUT) {
                    // bail out if socket error (disconnect)
                    // otherwise, keep going (timeout)
                    err_quit=true;
                    break;
                }
            }
            
            if (timeout_msec>0) {
                // select/read may be interrupted before its timeout
                // Update timeout value and retry for remaining time
                double now_sec = mtime_dtime();
                elapsed_sec   = now_sec-start_sec;
            }
            
        }// while

    }// ERR - invalid arg
    
    if (read_total==len) {
        // read complete - OK
        me_errno=ME_OK;
//        fprintf(stderr,"read filled request - buf:\n");
    }else{
        
        switch (me_errno) {
            case ME_EREAD:
            case ME_ESOCK:
            case ME_EOF:
            case ME_ETMOUT:
                // use me_errno
                break;
                
            default:
                if (t_rem<0) {
                    // timed out
                    me_errno=ME_ETMOUT;
                }
        }
    }
//    fprintf(stderr,"#### read_tmout ret[%"PRId64"/%"PRIu32"] time[%.5lf/%.5lf]\n",retval,len,elapsed_sec,to_sec);

    return retval;
}
// End function msock_read_tmout

/// @fn int msock_close(msock_socket_t *self)
/// @brief close socket
/// @param[in] self socket instance reference
/// @return 0 on success, -1 otherwise.
int msock_close(msock_socket_t *self)
{
    int retval=-1;
    if(NULL!=self)
    retval = close(self->fd);
    return retval;
}
// End function msock_close

/// @fn msock_socket_t * msock_wrap_fd(int fd)
/// @brief wrap file descriptor in msock_socket.
/// e.g. when posix functions provide fd that must be passed
/// to iow_ socket functions. Sets socket status to SS_CONNECTED.
/// @param[in] fd file descriptor
/// @return iow_socket instance (caller should destroy with iow_socket_destroy).
msock_socket_t *msock_wrap_fd(int fd)
{
    msock_socket_t *s = msock_socket_wnew("wrapper",9999,ST_TCP);
    if (NULL != s) {
        s->fd = fd;
        s->status=SS_CONNECTED;
    }
    return s;
}
// End function msock_wrap_fd


int msock_get_opt(msock_socket_t *self, int opt_name, void *optval, socklen_t *optlen)
{
    return msock_lget_opt(self, SOL_SOCKET, opt_name, optval, optlen);
}

int msock_set_opt(msock_socket_t *self, int opt_name, const void *optval, socklen_t optlen)
{
	    return msock_lset_opt(self, SOL_SOCKET, opt_name, optval, optlen);
}

int msock_lget_opt(msock_socket_t *self, int opt_level, int opt_name, void *optval, socklen_t *optlen)
{
    return getsockopt(self->fd, opt_level, opt_name, optval, (socklen_t *)optlen);
}

int msock_lset_opt(msock_socket_t *self, int opt_level, int opt_name, const void *optval, socklen_t optlen)
{
        return setsockopt(self->fd, opt_level, opt_name, optval, (socklen_t)optlen);
}

#ifdef WITH_MSOCKET_TEST
/// @fn int32_t msock_test()
/// @brief test socket API
/// @return 0 on success, -1 otherwise
int msock_test()
{
    int retval=-1;
    
    msock_socket_t *svr = msock_socket_new("localhost",9999,ST_TCP);
    msock_socket_t *cli = msock_socket_new("localhost",9999,ST_TCP);
    
    if (NULL!=svr && NULL!=cli) {
        msock_set_blocking(svr,true);
        msock_set_blocking(cli,true);
        
        if( (msock_bind(svr)==0)){
            fprintf(stderr,"svr bound\n");
            if( (msock_listen(svr,1)==0) ){
                fprintf(stderr,"svr listening\n");
                if(msock_connect(cli)==0){
                    fprintf(stderr,"cli connected\n");
                     if ( (svr->fd = msock_accept(svr,NULL))>0) {
                        fprintf(stderr,"svr accepted\n");
                        if(msock_send(cli,(byte *)"REQ",4)==4){
                            fprintf(stderr,"cli REQ sent\n");
                            byte smsg[16]={0};
                            int32_t brx=0;
                           if( (brx=msock_recv(svr,smsg,4,0))==4 && strcmp((const char *)smsg,"REQ")==0){
                                fprintf(stderr,"svr REQ received\n");
                                if(msock_send(svr,(byte *)"ACK",4)==4){
                                    fprintf(stderr,"svr ACK sent\n");
                                    byte cmsg[16]={0};
                                    if(msock_recv(cli,cmsg,4,0)==4 && strcmp((const char *)cmsg,"ACK")==0){
                                        fprintf(stderr,"cli ACK received\n");
                                            fprintf(stderr,"OK\n");
                                            retval=0;
                                    }else{
                                        fprintf(stderr,"cli rcv failed [%d/%s]\n",errno,strerror(errno));
                                    }
                                }else{
                                    fprintf(stderr,"svr send failed [%d/%s]\n",errno,strerror(errno));
                                }
                            }else{
                                fprintf(stderr,"svr rcv failed smsg[%s] brx[%d] [%d/%s]\n",smsg,brx,errno,strerror(errno));
                            }
                        }else{
                            fprintf(stderr,"cli send failed [%d/%s]\n",errno,strerror(errno));
                        }
                    }else{
                        fprintf(stderr,"svr accept failed [%d/%s]\n",errno,strerror(errno));
                    }
                }else{
                    fprintf(stderr,"connect failed [%d/%s]\n",errno,strerror(errno));
                }
            }else{
                fprintf(stderr,"listen failed [%d/%s]\n",errno,strerror(errno));
            }
        }else{
            fprintf(stderr,"bind failed [%d/%s]\n",errno,strerror(errno));
        }
    }
    msock_socket_destroy(&svr);
    msock_socket_destroy(&cli);
    return retval;
}
// End function msock_test


#endif // WITH_MSOCKET_TEST
#endif // ! __QNX__

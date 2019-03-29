///
/// @file mbtrn-server.c
/// @authors k. Headley
/// @date 01 jan 2018
 
/// Test server for mbtrn
/// Reads MB data from a file and writes
/// it to a socket (e.g. emulates reson 7k center source)

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

// TODO: clean up server porting
#if defined(__unix__) || defined(__APPLE__)
#include <sys/poll.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#else
#	include <winsock2.h>
#	include <WS2tcpip.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mbtrn-server.h"
#include "mdebug.h"


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

/// @fn mbtrn_server_t * mbtrn_server_new(iow_socket_t * s, iow_file_t * mb_data)
/// @brief create new mbtrn test server - emulate reson 7k center (not fully implemented).
/// @param[in] s socket reference
/// @param[in] mb_data reson data file (optional)
/// @return new server reference
mbtrn_server_t *mbtrn_server_new(iow_socket_t *s, iow_file_t *mb_data)
{
    mbtrn_server_t *self = (mbtrn_server_t *)malloc(sizeof(mbtrn_server_t));
    if (self) {
        self->in_file = mb_data;
        self->auto_free = true;
        self->sock_if=s;
        self->stop=false;
        self->t=iow_thread_new();
    }
    return self;
}
// End function mbtrn_server_new


/// @fn void mbtrn_server_destroy(mbtrn_server_t ** pself)
/// @brief release server resources.
/// @param[in] pself pointer to instance reference
/// @return none
void mbtrn_server_destroy(mbtrn_server_t **pself)
{
    if (pself) {
        mbtrn_server_t *self = *(pself);
        if (self) {
            if (self->auto_free) {
                iow_socket_destroy(&self->sock_if);
                iow_file_destroy(&self->in_file);
                iow_thread_destroy(&self->t);
            }
            free(self);
        }
        *pself =  NULL;
    }
}
// End function mbtrn_server_destroy


// POSIX server
/// @def REQ_TEST_REQ
/// @brief protocol request data
#define REQ_TEST_REQ "REQ"
/// @def REQ_SERVER_STOP
/// @brief protocol server stop
#define REQ_SERVER_STOP "STOP"
/// @def REQ_SERVER_SUB
/// @brief protocol subscribe
#define REQ_SERVER_SUB "SUB"
/// @typedef enum server_req_id server_req_id
/// @brief TBD
typedef enum {REQ=1,SUB,STOP}server_req_id;

/// @fn int s_server_handle_request(mbtrn_server_t * svr, char * req, int client_fd)
/// @brief handle client request.
/// @param[in] svr server reference
/// @param[in] req request string
/// @param[in] client_fd client file descriptor/handle
/// @return 0 on success, -1 otherwise
static  int s_server_handle_request(mbtrn_server_t *svr, char *req, int client_fd)
{
    int retval=0;
    if (strncmp(req,"STOP",4)==0) {
        MDEBUG("STOP received\n");
        send(client_fd,"ACK",strlen("ACK"),0);
        svr->stop=true;
    }else if(strncmp(req,"REQ",3)==0){
        MDEBUG("REQ received\n");
        send(client_fd,"ACK",strlen("ACK"),0);
    }else if(strncmp(req,"SUB",3)==0){
        MDEBUG("SUB received\n");
    }else if( (*(uint16_t *)req)==5 ){
        MDEBUG("7K message received\n");
        r7k_msg_t *msg = r7k_msg_new(sizeof(r7k_rth_7501_ack_t));
        r7k_rth_7501_ack_t *prth = (r7k_rth_7501_ack_t *)(msg->data);
        prth->ticket = 1;
        //byte x[16]="ABCDEF0123456789";
        memcpy(prth->tracking_number, "ABCDEF0123456789",strlen("ABCDEF0123456789"));
        msg->drf->size           = R7K_MSG_DRF_SIZE(msg);
        msg->drf->record_type_id = R7K_RT_REMCON_ACK;
        msg->drf->device_id      = R7K_DEVID_7KCENTER;
        msg->nf->tx_id       = r7k_txid();
        msg->nf->seq_number  = 0;
        msg->nf->packet_size = R7K_MSG_NF_PACKET_SIZE(msg);
        msg->nf->total_size  = R7K_MSG_NF_TOTAL_SIZE(msg);
 
        r7k_msg_set_checksum(msg);
        MDEBUG("sending SUB ACK:\n");
        r7k_msg_show(msg,true,3);
        iow_socket_t *s = iow_wrap_fd(client_fd);
        r7k_msg_send(s,msg);
        r7k_msg_destroy(&msg);
        iow_socket_destroy(&s);
    }else{
        MDEBUG("ERR - invalid request [%s]\n",req);
        retval=-1;
    }
    return retval;
}
// End function s_server_handle_request


/// @fn void * s_server_main(void * arg)
/// @brief test server thread function.
/// @param[in] arg void pointer to service reference
/// @return 0 on success, -1 otherwise
static void *s_server_main(void *arg)
{
    mbtrn_server_t *svr = (mbtrn_server_t *)arg;
    
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
    bool stop_req=false;

    iow_socket_t  *s = svr->sock_if;
    if ( (NULL!=svr) && (NULL!=s)) {
       
        MINFO("mbtrn server [%s] - starting\n",iow_addr2str(s,buf,ADDRSTR_BYTES));
        iow_listen(s);
        
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        
        FD_ZERO(&read_fds);
        FD_ZERO(&master);
        FD_SET(s->fd,&master);
        fdmax = s->fd;
       while (!svr->stop && !stop_req) {
           read_fds = master;
          // MINFO("pending on select\n");
           if( (stat=select(fdmax+1, &read_fds, NULL, NULL, &tv)) != -1){
               int newfd=-1;
               for (int i=s->fd; i<=fdmax; i++) {
                   
                   if (FD_ISSET(i, &read_fds)){
                      // MINFO("readfs [%d/%d] selected\n",i,fdmax);
                       
                       if (i==s->fd) {
                           MINFO("server main listener [%d] got request\n",i);
                           
                           newfd = accept(s->fd, (struct sockaddr *)&client_addr, &addr_size);
                           if (newfd != -1) {
                               MINFO("server recieved connection from client on socket [%d]\n",newfd);
                               FD_SET(newfd,&read_fds);
                               if (newfd>fdmax) {
                                   fdmax=newfd;
                               }
                           }else{
                               // accept failed
                               MINFO("accept failed [%d/%s]\n",errno,strerror(errno));
                           }
                       }else{
                           MINFO("server waiting for client data fd[%d]\n",i);
                           if (( nbytes = recv(i, iobuf, sizeof iobuf, 0)) <= 0) {
                               MINFO("handle client data fd[%d] nbytes[%d]\n",i,nbytes);
                               // got error or connection closed by client
                               if (nbytes == 0) {
                                   // connection closed
                                   fprintf(stderr,"ERR - socket %d hung up\n", i);
                               } else if(nbytes<0) {
                                   fprintf(stderr,"ERR - recv failed socket[%d] [%d/%s]\n",i,errno,strerror(errno));
                               }
                           }else{
                               MINFO("server received request on socket [%d] [%s] len[%d]\n",i,iobuf,nbytes);
                               s_server_handle_request(svr,iobuf,i);
                           }
                           close(i); // bye!
                           FD_CLR(i, &master); // remove from master set
                       }
                   }else{
//                       MINFO("readfs fd[%d/%d] ISSET:%s\n",i,fdmax,(FD_ISSET(i,&read_fds)?"TRUE":"FALSE"));
                   }
               }
           }else{
               // select failed
               MINFO("select failed [%d/%s]\n",errno,strerror(errno));
           }
       }
    }
    if (stop_req) {
        MINFO("Test server - interrupted - stop flag set\n");
        s->status=1;
    }else{
        MINFO("Test server - normal exit\n");
        s->status=0;
    }
    pthread_exit((void *)&s->status);

//    if (NULL != svr) {
//        while (!svr->stop) {
//            sleep(1);
//        }
//        MDEBUG("stop flag set - exiting\n");
//    }
    return (void *)svr->t->status;
}
// End function s_server_main


/// @fn int mbtrn_server_start(mbtrn_server_t * self)
/// @brief start test server in a thread.
/// @param[in] self server reference
/// @return 0 on success, -1 otherwise
int mbtrn_server_start(mbtrn_server_t *self)
{
    int retval=0;
    if (self) {
        self->stop=false;
        if(iow_thread_start(self->t, s_server_main, (void *)self)!=0){
            retval=-1;
        }else{
            
            sleep(1);
        }
    }
    return retval;
}
// End function mbtrn_server_start


/// @fn int mbtrn_server_stop(mbtrn_server_t * self)
/// @brief stop server thread.
/// @param[in] self server reference
/// @return 0 on success, -1 otherwise
int mbtrn_server_stop(mbtrn_server_t *self)
{
    int retval=-1;
    if (NULL==self) {
        self->stop=true;
        iow_thread_join(self->t);
        retval=0;
    }
    return retval;
}
// End function mbtrn_server_stop




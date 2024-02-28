///
/// @file mmcpub.c
/// @authors k. headley
/// @date 2019-06-21
/// antony courtney 25/11/94

/// UDP Multicast publisher (uses mframe)

/// @sa doxygen-examples.c for more examples of Doxygen markup


/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information

 Copyright 2002-2019 MBARI
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// for basename
#include <libgen.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include "mframe.h"
#include "msocket.h"

/////////////////////////
// Macros
/////////////////////////
#define DFL_GROUP "239.255.0.16"
#define DFL_MCAST_PORT 29000
#define DFL_LOCAL_PORT 7070
#define DFL_TTL 32

#define MAX_DATA_BYTES 1024
#define MSGBUFSIZE MAX_DATA_BYTES
#define OFMT_WMSG 36
#define OFMT_WSTAT 32
#define OFMT_KEY 12
#define OFMT_VAL 16

/////////////////////////
// Declarations
/////////////////////////

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////

bool g_mcast_interrupt=false;

static void s_termination_handler (int signum)
{
    switch (signum) {
#if ! defined(__WIN32) && ! defined(__WIN64)
        case SIGHUP:
#endif
        case SIGINT:
        case SIGTERM:
            fprintf(stderr,"\nsig received[%d]\n",signum);
            g_mcast_interrupt=true;
            break;
        default:
            fprintf(stderr,"\ns_termination_handler: sig not handled[%d]\n",signum);
            break;
    }
}

int main(int argc, char *argv[])
{
    const char *mcast_addr_s=DFL_GROUP;
    const char *mcast_if_s=NULL;
    const char *host_addr_s=NULL;
    int mcast_port=DFL_MCAST_PORT;
    u_char so_ttl=DFL_TTL;
    u_char so_loop=1;
    struct ip_mreq mreq;

    int opt_c=0;
    int ittl=0;
    bool bind_en=false;
    bool bidir_en=true;
    bool xout_en=false;
    int delay=2;
    char *message="MCPUB";

    int msg_n=0;
    int wstat=OFMT_WSTAT;
    int wkey=OFMT_KEY;
    int wval=OFMT_VAL;

    char *lcm = NULL;

    // configure signal handling
    // for main thread
    struct sigaction saStruct;
    sigemptyset(&saStruct.sa_mask);
    saStruct.sa_flags = 0;
    saStruct.sa_handler = s_termination_handler;
    sigaction(SIGINT, &saStruct, NULL);

    while((opt_c = getopt(argc, argv, "a:bd:hi:m:lLp:t:ux")) != -1){
        switch(opt_c) {
            case 'a':
                mcast_addr_s = optarg;
                break;
            case 'b':
                bind_en = true;
                break;
            case 'd':
                delay = atoi(optarg);
                break;
            case 'i':
                host_addr_s = optarg;
                break;
            case 'm':
                mcast_if_s=optarg;
                break;
            case 'l':
                so_loop = 0;
                break;
            case 'p':
                mcast_port = atoi(optarg);
                break;
            case 't':
                ittl=atoi(optarg);
                so_ttl=(unsigned char)(ittl & 0xff);
                break;
            case 'u':
                bidir_en = false;
                break;
            case 'x':
                xout_en = true;
                break;
            case 'L':
                if(NULL!=lcm)free(lcm);
                lcm = strdup("LC02");
                break;
            case 'h':
                fprintf(stderr,"\n");
                fprintf(stderr,"Usage: %s [options] [-h]\n",basename(argv[0]));
                fprintf(stderr,"\n");
                fprintf(stderr,"-a <addr>: mcast group address\n");
                fprintf(stderr,"-p <port>: mcast port\n");
                fprintf(stderr,"-m <addr>: mcast interface address\n");
                fprintf(stderr,"-t <ttl> : mcast ttl\n");
                fprintf(stderr,"-i <addr>: host IP address\n");
                fprintf(stderr,"-l       : disable mcast loopback\n");
                fprintf(stderr,"-b       : enable bind\n");
                fprintf(stderr,"-u       : unidirectional (mcast pub->sub only)\n");
                fprintf(stderr,"-x       : enable hex out\n");
                fprintf(stderr,"-L       : LCM compatible message (not fully compliant)\n");
                fprintf(stderr,"-h : print this help message\n");
                fprintf(stderr,"\n");
                exit(0);
                break;
            default:
                break;
        }
    }

    // show config
    fprintf(stderr,"%*s %*s\n",wkey,"host_addr",wval,(NULL!=host_addr_s?host_addr_s:""));
    fprintf(stderr,"%*s %*s\n",wkey,"mcast_addr",wval,mcast_addr_s);
    fprintf(stderr,"%*s %*d\n",wkey,"mcast_port",wval,mcast_port);
    fprintf(stderr,"%*s %*s\n",wkey,"mcast_if",wval,(NULL!=mcast_if_s?mcast_if_s:""));
    fprintf(stderr,"%*s %*d\n",wkey,"so_ttl",wval,so_ttl);
    fprintf(stderr,"%*s %*c\n",wkey,"so_loop",wval,so_loop!=0?'Y':'N');
    fprintf(stderr,"%*s %*c\n",wkey,"bind_en",wval,bind_en?'Y':'N');
    fprintf(stderr,"%*s %*c\n",wkey,"bidir_en",wval,bidir_en?'N':'Y');
    fprintf(stderr,"%*s %*c\n",wkey,"xout",wval,xout_en?'Y':'N');
    fprintf(stderr,"%*s %*s\n",wkey,"message",wval,message);
    fprintf(stderr,"%*s %*d\n",wkey,"PID",wval,getpid());
    fprintf(stderr,"%*s %*s\n",wkey,"LCM",wval,lcm);
    fprintf(stderr,"\n");

    if(NULL!=host_addr_s){
        fprintf(stderr,"%*s %s\n",wstat,"local addr",host_addr_s);
    }else{
        fprintf(stderr,"%*s %s\n",wstat,"local addr","INADDR_ANY");
    }

    msock_socket_t *pub = NULL;
    if( (pub = msock_socket_new(mcast_addr_s,mcast_port,ST_UDPM))!=NULL){
        fprintf(stderr,"%*s %s\n",wstat,"socket","OK");
    }else {
        fprintf(stderr,"%*s %s\n",wstat,"socket","ERR");
        perror("socket");
        exit(1);
    }

    msock_set_blocking(pub,false);

    const int so_reuse = 1;

    // enable multiple clients on same host
    if(msock_set_opt(pub, SO_REUSEADDR, &so_reuse, sizeof(so_reuse))==0){
        fprintf(stderr,"%*s %s\n",wstat,"msock_set_opt SO_REUSEADDR","OK");
    }else {
        fprintf(stderr,"%*s %s\n",wstat,"msock_set_opt SO_REUSEADDR","ERR");
        perror("setsockopt SO_REUSEADDR");
        exit(1);
    }

#if !defined(__CYGWIN__)
    // Cygwin doesn't define SO_REUSEPORT
    // OSX requires this to reuse socket (linux optional)
    if (msock_set_opt(pub, SO_REUSEPORT, &so_reuse, sizeof(so_reuse)) ==0){
        fprintf(stderr,"%*s %s\n",wstat,"msock_set_opt SO_REUSEPORT","OK");
    }else {
        fprintf(stderr,"%*s %s\n",wstat,"msock_set_opt SO_REUSEPORT","ERR");
        perror("setsockopt SO_REUSEPORT");
        exit(1);
    }
#endif // CYGWIN

    if(NULL!=mcast_if_s){
        memset(&mreq,0,sizeof(mreq));

        if(strcmp(mcast_if_s,"INADDR_ANY")==0){
            mreq.imr_multiaddr.s_addr=htonl(INADDR_ANY);
        }else{
            mreq.imr_interface.s_addr=inet_addr(mcast_if_s);
        }

        if(msock_lset_opt(pub, IPPROTO_IP, IP_MULTICAST_IF, &mreq, sizeof(mreq))==0){
            fprintf(stderr,"%*s %s\n",wstat,"msock_lset_opt IP_MULTICAST_IF","OK");
        }else {
            fprintf(stderr,"%*s %s\n",wstat,"msock_lset_opt IP_MULTICAST_IF","ERR");
            perror("setsockopt IP_MULTICAST_IF");
        }
    }

    // bind to receive address
    if (bind_en){
        // bind the socket to INADDR_ANY (accept mcast on all interfaces)
        if(msock_bind(pub)==0){
            fprintf(stderr,"%*s %s\n",wstat,"bind","OK");
        }else {
            fprintf(stderr,"%*s %s\n",wstat,"bind","ERR");
            perror("bind");
            exit(1);
        }
    }// bind_en

    // future expansion (for mcast outbound)
    if(msock_lset_opt(pub, IPPROTO_IP, IP_MULTICAST_LOOP, &so_loop, sizeof(so_loop))==0) {
        fprintf(stderr,"%*s %s\n",wstat,"msock_lset_opt IP_MULTICAST_LOOP","OK");
    }else {
        fprintf(stderr,"%*s %s\n",wstat,"msock_lset_opt IP_MULTICAST_LOOP","ERR");
        perror("setsockopt IP_MULTICAST_LOOP");
    }

    if(msock_lset_opt(pub, IPPROTO_IP, IP_MULTICAST_TTL, &so_ttl, sizeof(so_ttl))==0) {
        fprintf(stderr,"%*s %s\n",wstat,"msock_lset_opt IP_MULTICAST_TTL","OK");
    }else {
        fprintf(stderr,"%*s %s\n",wstat,"msock_lset_opt IP_MULTICAST_TTL","ERR");
        perror("setsockopt IP_MULTICAST_TTL");
    }

    // use setsockopt() to request that the kernel join a multicast group
    mreq.imr_multiaddr.s_addr=inet_addr(mcast_addr_s);
    mreq.imr_interface.s_addr=htonl(INADDR_ANY);
    if (msock_lset_opt(pub, IPPROTO_IP, IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) == 0){
        fprintf(stderr,"%*s %s\n",wstat,"msock_lset_opt IP_ADD_MEMBERSHIP","OK");
    }else {
        fprintf(stderr,"%*s %s\n",wstat,"msock_lset_opt IP_ADD_MEMBERSHIP","ERR");
        perror("setsockopt IP_ADD_MEMBERSHIP");
        exit(1);
    }

    // set up an address struct sub client rx/tx
    msock_addr_t xaddr,*pxaddr=&xaddr;
    struct addrinfo xaddr_ai;
    struct sockaddr xaddr_in;
    pxaddr->ainfo=&xaddr_ai;
    pxaddr->ainfo->ai_addr=&xaddr_in;
    struct sockaddr_in *psa = (struct sockaddr_in *)pub->addr->ainfo->ai_addr;
    struct sockaddr_in *pxa = (struct sockaddr_in *)pxaddr->ainfo->ai_addr;

    // enter main loop
    while (!g_mcast_interrupt) {
        int wmsg=OFMT_WMSG;
        int64_t tx_bytes=0;
        char txbuf[MSGBUFSIZE];
        // prepare mcast message and address
        memset(txbuf,0,MAX_DATA_BYTES);
        size_t tx_len=0;
        if(NULL == lcm){
            snprintf(txbuf, MAX_DATA_BYTES, "MSG mid[%3d]", msg_n++);
            tx_len=strlen(txbuf)+1;
        }else{
            typedef struct lcm_hdr_s{
                byte magic[4];
                uint32_t seq;
                // char channel[]
            }lcm_hdr_t;

            lcm_hdr_t *hdr = (lcm_hdr_t *)txbuf;
            hdr->magic[0] = 'L';
            hdr->magic[1] = 'C';
            hdr->magic[2] = '0';
            hdr->magic[3] = '2';
            hdr->seq = msg_n;

            // channel pointer
            char *pchannel = txbuf + sizeof(lcm_hdr_t);
            const char *channel="MSG";
            // pointer to message length
            uint32_t *plen = (uint32_t *)((byte *)pchannel + strlen(channel)+1);
            // message data pointer
            char *pdata = pchannel + strlen(channel)+1+sizeof(uint32_t);

            // write channel
            size_t wlen = strlen(channel)+1;
            snprintf(pchannel, wlen, "%s",channel);

            // write data (msg ID)
            wlen = MAX_DATA_BYTES - (pdata - txbuf);
            *plen = snprintf(pdata, wlen, "mid[%3d]",msg_n++)+1;
            tx_len = sizeof(lcm_hdr_t) + strlen(channel) + strlen(pdata) + 2 + sizeof(uint32_t);

            // hexdump message
            fprintf(stderr, "msg bytes\n");
            for(int i=0, col=0;i<tx_len;i++){
                fprintf(stderr, "%02x ",txbuf[i]);
                if(col>0 && (col%7)==0) {
                    fprintf(stderr, "\n");
                    col=0;
                }else{col++;}
            }
            fprintf(stderr, "\n");
        }
        // Send mcast message to SUB clients
        if( (tx_bytes=msock_sendto(pub, pub->addr, (byte *)txbuf, tx_len, 0))>0){
            fprintf(stderr,"PUB - mtx msg[%-*s] len[%7zu] dest[%s : %hu]\n",
                    wmsg, txbuf,tx_len,
                    inet_ntoa(psa->sin_addr),
                    htons(psa->sin_port));
        }else{
            perror("sendto");
        }

        if(bidir_en){
            // check for messages from RX hosts (don't block)
            char rxbuf[MSGBUFSIZE];

            // read socket and capture sender address
            memset(pxa,0,sizeof(struct sockaddr));
            // zero message buffer
            memset(rxbuf,0,MSGBUFSIZE);

            // read/respond to pending SUB unicast messages
            bool sub_loop=true;
            while (sub_loop){
                int64_t rx_bytes=0;

                if((rx_bytes=msock_recvfrom(pub,pxaddr,(byte *)rxbuf,MSGBUFSIZE,0)) >0){
                    fprintf(stderr,"PUB - urx msg[%-*s] len[%7"PRId64"]  src[%s : %hu]\n",wmsg,rxbuf,rx_bytes,
                            inet_ntoa(pxa->sin_addr),
                            htons(pxa->sin_port));

                    // generate ACK message
                    memset(txbuf,0,MSGBUFSIZE);
                    char *pids=strstr(rxbuf,"mid");
                    char *cids=strstr(rxbuf,"cid");
                    int mid = -1;
                    int cid = -1;
                    if(NULL!= cids){
                        sscanf(cids,"cid[%d]",&cid);
                    }
                    if( pids!=NULL){
                        sscanf(pids,"mid[%d",&mid);
                    }
                    snprintf(txbuf, MSGBUFSIZE, "ACK mid[%d] cid[%d] pid[%d] ",mid,cid,getpid());
                    tx_len = strlen(txbuf)+1;

                    tx_bytes=0;
                    // send ACK to SUB client (using recvfrom addr)
                    if ( (tx_bytes=msock_sendto(pub, pxaddr, (byte *)txbuf, tx_len, 0)) >= 0){

                        fprintf(stderr,"PUB - utx msg[%-*s] len[%3zu/%-3"PRId64"] dest[%s : %hu]\n",
                                wmsg, txbuf, tx_len,tx_bytes,
                                inet_ntoa(pxa->sin_addr),
                                htons(pxa->sin_port));
                    }else{
                        perror("sendto");
                    }
                }else{
                    // error or no data: exit loop
                    if(errno!=EAGAIN){
                        perror("recvfrom");
                    }
                    sub_loop=false;
                    break;
                }
            }// while pending SUB messages
        }// bidir en

        if(delay>0){
            sleep(delay);
        }
    }
    fprintf(stderr,"destroying socket\n");
    msock_socket_destroy(&pub);
    return 0;
}



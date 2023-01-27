///
/// @file mcpub.c
/// @authors k. headley
/// @date 2019-06-21
/// antony courtney 25/11/94

/// UDP Multicast publisher

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

/////////////////////////
// Macros
/////////////////////////
#define DFL_GROUP "239.255.0.16"
#define DFL_MCAST_PORT 29000
#define DFL_LOCAL_PORT 7070
#define DFL_TTL 32
#define TRACE() fprintf(stderr,"%s:%d\n",__func__,__LINE__)

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

static int s_set_blocking(int fd, bool enable)
{
    int retval=-1;
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags != -1){
        if(enable){
            flags &= ~O_NONBLOCK;
        }else{
            flags |= O_NONBLOCK;
        }
        fcntl(fd, F_SETFL, flags);
        retval=0;
    }// else error
    return retval;
}

int main(int argc, char *argv[])
{
    struct sockaddr_in local_addr;
    struct sockaddr_in mcast_addr;
    struct sockaddr_in x_addr;
    socklen_t addrlen;
    const char *mcast_addr_s=DFL_GROUP;
    const char *mcast_if_s=NULL;
    const char *host_addr_s=NULL;
    int mcast_port=DFL_MCAST_PORT;
    u_char so_ttl=DFL_TTL;
    u_int so_reuse=1;
    u_char so_loop=1;
    struct ip_mreq mreq;

    int fd;
    int opt_c=0;
    int ittl=0;
    bool bind_en=false;
    bool bidir_en=true;
    bool xout_en=false;
    int delay=1;
    char *message="MCPUB";

    char rxbuf[MSGBUFSIZE];
    char txbuf[MSGBUFSIZE];
    int nbytes=0;
    int msg_n=0;
    int wmsg=OFMT_WMSG;
    int wstat=OFMT_WSTAT;
    int wkey=OFMT_KEY;
    int wval=OFMT_VAL;

    // configure signal handling
    // for main thread
    struct sigaction saStruct;
    sigemptyset(&saStruct.sa_mask);
    saStruct.sa_flags = 0;
    saStruct.sa_handler = s_termination_handler;
    sigaction(SIGINT, &saStruct, NULL);

    while((opt_c = getopt(argc, argv, "a:bd:hi:m:lp:t:ux")) != -1){
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
                fprintf(stderr,"-h : print this help message\n");
                fprintf(stderr,"\n");
                exit(0);
                break;
            default:
                break;
        }
    }

    // show config
    fprintf(stderr,"%*s %*s\n",wkey,"host_addr",wval,host_addr_s);
    fprintf(stderr,"%*s %*s\n",wkey,"mcast_addr",wval,mcast_addr_s);
    fprintf(stderr,"%*s %*d\n",wkey,"mcast_port",wval,mcast_port);
    fprintf(stderr,"%*s %*s\n",wkey,"mcast_if",wval,mcast_if_s);
    fprintf(stderr,"%*s %*d\n",wkey,"so_ttl",wval,so_ttl);
    fprintf(stderr,"%*s %*c\n",wkey,"so_loop",wval,so_loop!=0?'Y':'N');
    fprintf(stderr,"%*s %*c\n",wkey,"bind_en",wval,bind_en?'Y':'N');
    fprintf(stderr,"%*s %*c\n",wkey,"bidir_en",wval,bidir_en?'N':'Y');
    fprintf(stderr,"%*s %*c\n",wkey,"xout",wval,xout_en?'Y':'N');
    fprintf(stderr,"%*s %*s\n",wkey,"message",wval,message);
    fprintf(stderr,"%*s %*d\n",wkey,"PID",wval,getpid());
    fprintf(stderr,"\n");

    // set up destination address
    memset(&local_addr,0,sizeof(local_addr));
    local_addr.sin_family=AF_INET;
    local_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    if(NULL!=host_addr_s){
        local_addr.sin_addr.s_addr=inet_addr(host_addr_s);
        fprintf(stderr,"%*s %s\n",wstat,"local addr",host_addr_s);
    }else{
        local_addr.sin_addr.s_addr=htonl(INADDR_ANY);
        fprintf(stderr,"%*s %s\n",wstat,"local addr","INADDR_ANY");
    }
    local_addr.sin_port=htons(mcast_port);

    // set up mcast address
    memset(&mcast_addr,0,sizeof(mcast_addr));
    mcast_addr.sin_family=AF_INET;
    mcast_addr.sin_addr.s_addr=inet_addr(mcast_addr_s);
    mcast_addr.sin_port=htons(mcast_port);

    // create UDP socket
    if ((fd=socket(AF_INET,SOCK_DGRAM,0))>0){
        fprintf(stderr,"%*s %s\n",wstat,"socket","OK");
    }else {
        fprintf(stderr,"%*s %s\n",wstat,"socket","ERR");
        perror("socket");
        exit(1);
    }

    // allow multiple sockets to use the same PORT number
#if !defined(__CYGWIN__)
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &so_reuse, sizeof(so_reuse)) ==0){
        fprintf(stderr,"%*s %s\n",wstat,"setsockopt SO_REUSEPORT","OK");
    }else {
        fprintf(stderr,"%*s %s\n",wstat,"setsockopt SO_REUSEPORT","ERR");
        perror("setsockopt SO_REUSEPORT");
        exit(1);
    }
#endif
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &so_reuse, sizeof(so_reuse))==0){
        fprintf(stderr,"%*s %s\n",wstat,"setsockopt SO_REUSEADDR","OK");
    }else {
        fprintf(stderr,"%*s %s\n",wstat,"setsockopt SO_REUSEADDR","ERR");
        perror("setsockopt SO_REUSEADDR");
        exit(1);
    }


    if(NULL!=mcast_if_s){
        memset(&mreq,0,sizeof(mreq));
        if(strcmp(mcast_if_s,"INADDR_ANY")==0){
            mreq.imr_multiaddr.s_addr=htonl(INADDR_ANY);
        }else{
            mreq.imr_interface.s_addr=inet_addr(mcast_if_s);
        }

        if (setsockopt (fd, IPPROTO_IP, IP_MULTICAST_IF, &mreq, sizeof(mreq))==0){
            fprintf(stderr,"%*s %s\n",wstat,"setsockopt IP_MULTICAST_IF","OK");
        }else {
            fprintf(stderr,"%*s %s\n",wstat,"setsockopt IP_MULTICAST_IF","ERR");
            perror("setsockopt IP_MULTICAST_IF");
        }
    }

    // bind to receive address
    if (bind_en){
        if( bind(fd,(struct sockaddr *) &local_addr,sizeof(local_addr))==0){
            fprintf(stderr,"%*s %s\n",wstat,"bind","OK");
        }else {
            fprintf(stderr,"%*s %s\n",wstat,"bind","ERR");
            perror("bind");
            exit(1);
        }
    }

    // loop : multicast loopback
    // enable a copy of mcast messages to go to sender host
    // enable for PUB host, optional for SUB host
    if (setsockopt (fd, IPPROTO_IP, IP_MULTICAST_LOOP, &so_loop, sizeof(so_loop))==0){
        fprintf(stderr,"%*s %s\n",wstat,"setsockopt IP_MULTICAST_LOOP","OK");
    }else {
        fprintf(stderr,"%*s %s\n",wstat,"setsockopt IP_MULTICAST_LOOP","ERR");
        perror("setsockopt IP_MULTICAST_LOOP");
        exit(1);
    }

    if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &so_ttl, sizeof(so_ttl))==0){
        fprintf(stderr,"%*s %s\n",wstat,"setsockopt IP_MULTICAST_TTL","OK");
    }else {
        fprintf(stderr,"%*s %s\n",wstat,"setsockopt IP_MULTICAST_TTL","ERR");
        perror("setsockopt IP_MULTICAST_TTL");
        exit(1);
    }

    // use setsockopt() to request that the kernel join a multicast group
    mreq.imr_multiaddr.s_addr=inet_addr(mcast_addr_s);
    mreq.imr_interface.s_addr=htonl(INADDR_ANY);

    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) ==0){
        fprintf(stderr,"%*s %s\n",wstat,"setsockopt IP_ADD_MEMBERSHIP","OK");
    }else {
        fprintf(stderr,"%*s %s\n",wstat,"setsockopt IP_ADD_MEMBERSHIP","ERR");
        perror("setsockopt IP_ADD_MEMBERSHIP");
        exit(1);
    }
    fprintf(stderr,"\n");

    s_set_blocking(fd,false);

    // enter main loop
    while (!g_mcast_interrupt) {

        // prepare mcast message and address
        memset(txbuf,0,MAX_DATA_BYTES);
        snprintf(txbuf, MAX_DATA_BYTES, "MSG mid[%d]",msg_n++);
        size_t tx_len=strlen(txbuf)+1;

        addrlen=sizeof(mcast_addr);

        fprintf(stderr,"PUB - mtx msg[%-*s] len[%7lu] dest[%s : %hu]\n",
                wmsg, txbuf,tx_len,
                inet_ntoa(mcast_addr.sin_addr),
                htons(mcast_addr.sin_port));

        // Send mcast message to SUB clients
        if (sendto(fd, txbuf, tx_len, 0, (struct sockaddr *) &mcast_addr,
                   addrlen) < 0) {
            perror("sendto");
        }

        if(bidir_en){
            // check for messages from RX hosts (don't block)

            // read socket and capture sender address
            memset(&x_addr,0,sizeof(x_addr));
            addrlen=sizeof(x_addr);
            // zero message buffer
            memset(rxbuf,0,MSGBUFSIZE);

            // read/respond to pending SUB unicast messages
            bool sub_loop=true;
            while (sub_loop){

                if((nbytes=recvfrom(fd,rxbuf, MSGBUFSIZE, 0,
                                    (struct sockaddr *) &x_addr, &addrlen)) >0){

                    fprintf(stderr,"PUB - urx msg[%-*s] len[%7d]  src[%s : %hu]\n",wmsg,rxbuf,nbytes,
                            inet_ntoa(x_addr.sin_addr),
                            htons(x_addr.sin_port));

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
                    size_t tx_len = strlen(txbuf)+1;

                    ssize_t tx_bytes=0;
                    // send ACK to SUB client (using recvfrom addr)
                    if ( (tx_bytes=sendto(fd,txbuf, tx_len, 0,(struct sockaddr *) &x_addr,
                                          addrlen)) >= 0){

                        fprintf(stderr,"PUB - utx msg[%-*s] len[%3lu/%-3lu] dest[%s : %hu]\n",
                                wmsg, txbuf, tx_len,tx_bytes,
                                inet_ntoa(x_addr.sin_addr),
                                htons(x_addr.sin_port));
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
    fprintf(stderr,"closing socket [%d]\n",fd);
    close(fd);
    return 0;
}




// listener.c -- joins a multicast group and echoes all data it receives from the group to its stderr...
// Antony Courtney,    25/11/94
// Modified by: Frédéric Bastien (25/03/04)
// to compile without warning and work correctly

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h> // for basename
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include "mframe.h"
#include "msocket.h"
#include "medebug.h"

#define DFL_GROUP "239.255.76.67"
#define DFL_MCAST_PORT 7667
#define DFL_LOCAL_PORT 7070
#define DFL_TTL 32

#define MAX_DATA_BYTES 1024
#define MSGBUFSIZE MAX_DATA_BYTES
#define OFMT_WMSG 36
#define OFMT_WSTAT 32
#define OFMT_KEY 12
#define OFMT_VAL 16

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

static int s_msg_xshow(char *msg, int len)
{
    int i=0;
    for(i=0;i<len;i++){
        if(i!=0 && i%16==0)fprintf(stderr,"\n");
        if(i%16==0)fprintf(stderr,"%08d",i);
        fprintf(stderr," %02X",(unsigned char)msg[i]);
    }
    fprintf(stderr,"\n");
    return i;
}

int main(int argc, char *argv[])
{

    const char *mcast_addr_s=DFL_GROUP;
    const char *mcast_if_s=NULL;
    const char *host_addr_s=NULL;
    int mcast_port=DFL_MCAST_PORT;
    u_char ttl=DFL_TTL;
    u_char loop=0;
    struct ip_mreq mreq;

    int opt_c=0;
    int ittl=0;
    bool bind_en=true;
    bool bidir_en=true;
    bool xout_en=false;
    bool aout_en=true;

    int cycles=-1;
    int cycle_count=0;

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

    while((opt_c = getopt(argc, argv, "a:bhi:lm:n:o:p:t:u")) != -1){
        switch(opt_c) {
            case 'a':
                mcast_addr_s = optarg;
                break;
            case 'b':
                bind_en = false;
                break;
            case 'i':
                host_addr_s = optarg;
                break;
            case 'l':
                loop = 1;
                break;
            case 'm':
                mcast_if_s=optarg;
                break;
            case 'n':
                cycle_count = atoi(optarg);
                cycles=cycle_count;
                break;
            case 'p':
                mcast_port = atoi(optarg);
                break;
            case 't':
                ittl=atoi(optarg);
                ttl=(unsigned char)(ittl & 0xff);
                break;
            case 'u':
                bidir_en = false;
                break;
            case 'o':
                if(strstr(optarg,"x+")!=NULL)
                    xout_en = true;
                if(strstr(optarg,"a+")!=NULL)
                    aout_en = true;
                if(strstr(optarg,"x-")!=NULL)
                    xout_en = false;
                if(strstr(optarg,"a-")!=NULL)
                    aout_en = false;
                break;
            case 'h':
                fprintf(stderr,"\n");
                fprintf(stderr,"Usage: %s [options] [-h]\n",basename(argv[0]));
                fprintf(stderr,"\n");
                fprintf(stderr,"-a <addr>: mcast group address\n");
                fprintf(stderr,"-p <port>: mcast port\n");
                fprintf(stderr,"-m <addr>: mcast interface address\n");
                fprintf(stderr,"-t <ttl> : mccast ttl\n");
                fprintf(stderr,"-i <addr>: host IP address\n");
                fprintf(stderr,"-l       : enable mcast loopback\n");
                fprintf(stderr,"-b       : disable bind\n");
                fprintf(stderr,"-u       : unidirectional (mcast pub->sub only)\n");
                fprintf(stderr,"-o <fmt> : output where fmt is x+,x-: hex a+,a-: ascii\n");
                fprintf(stderr,"-n <int> : cycles\n");
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
    fprintf(stderr,"%*s %*d\n",wkey,"ttl",wval,ttl);
    fprintf(stderr,"%*s %*c\n",wkey,"bind",wval,bind_en?'Y':'N');
    fprintf(stderr,"%*s %*c\n",wkey,"loopback",wval,loop!=0?'Y':'N');
    fprintf(stderr,"%*s %*c\n",wkey,"unidir",wval,bidir_en?'N':'Y');
    fprintf(stderr,"%*s %*c\n",wkey,"xout",wval,xout_en?'Y':'N');
    fprintf(stderr,"%*s %*c\n",wkey,"aout",wval,aout_en?'Y':'N');
    fprintf(stderr,"%*s %*d\n",wkey,"cycles",wval,cycles);
    fprintf(stderr,"%*s %*d\n",wkey,"PID",wval,getpid());
    fprintf(stderr,"\n");

    msock_socket_t *sub =NULL;

    if(NULL!=host_addr_s){
        fprintf(stderr,"%*s %s\n",wstat,"local addr",host_addr_s);
    }else{
        fprintf(stderr,"%*s %s\n",wstat,"local addr","INADDR_ANY");
    }

    // create socket using INADDR_ANY
    if( (sub = msock_socket_new("0.0.0.0",mcast_port,ST_UDPM))!=NULL){
        fprintf(stderr,"%*s %s\n",wstat,"socket","OK");
    }else {
        fprintf(stderr,"%*s %s\n",wstat,"socket","ERR");
        perror("socket");
        exit(1);
    }

    msock_set_blocking(sub,false);

    const int so_reuse = 1;

    // enable multiple clients on same host
    if(msock_set_opt(sub, SO_REUSEADDR, &so_reuse, sizeof(so_reuse))==0){
        fprintf(stderr,"%*s %s\n",wstat,"msock_set_opt SO_REUSEADDR","OK");
    }else {
        fprintf(stderr,"%*s %s\n",wstat,"msock_set_opt SO_REUSEADDR","ERR");
        perror("setsockopt SO_REUSEADDR");
        exit(1);
    }

#if !defined(__CYGWIN__)
    // Cygwin doesn't define SO_REUSEPORT
    // OSX requires this to reuse socket (linux optional)
    if(msock_set_opt(sub, SO_REUSEPORT, &so_reuse, sizeof(so_reuse))==0){
        fprintf(stderr,"%*s %s\n",wstat,"msock_set_opt SO_REUSEPORT","OK");
    }else {
        fprintf(stderr,"%*s %s\n",wstat,"msock_set_opt SO_REUSEPORT","ERR");
        perror("setsockopt SO_REUSEPORT");
        exit(1);
    }
#endif // CYGWIN


    if(bidir_en){
        // future expansion (for mcast outbound)
        unsigned char mcast_loop=1;
        if(msock_lset_opt(sub, IPPROTO_IP, IP_MULTICAST_LOOP, &mcast_loop, sizeof(mcast_loop))==0) {
            fprintf(stderr,"%*s %s\n",wstat,"msock_lset_opt IP_MULTICAST_LOOP","OK");
        }else {
            fprintf(stderr,"%*s %s\n",wstat,"msock_lset_opt IP_MULTICAST_LOOP","ERR");
            perror("setsockopt IP_MULTICAST_LOOP");
            exit(1);
        }

        if(msock_lset_opt(sub, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl))==0) {
            fprintf(stderr,"%*s %s\n",wstat,"setsockopt IP_MULTICAST_TTL","OK");
        }else {
            fprintf(stderr,"%*s %s\n",wstat,"setsockopt IP_MULTICAST_TTL","ERR");
            perror("setsockopt IP_MULTICAST_TTL");
            exit(1);
        }
    }//bidir_en


    // bind to receive address
    if (bind_en){
        // bind the socket to INADDR_ANY (accept mcast on all interfaces)
        if(msock_bind(sub)==0){
            fprintf(stderr,"%*s %s\n",wstat,"bind","OK");
        }else {
            fprintf(stderr,"%*s %s\n",wstat,"bind","ERR");
            perror("bind");
            exit(1);
        }
    }// bind_en


    // use setsockopt() to request that the kernel join a multicast group
    mreq.imr_multiaddr.s_addr=inet_addr(mcast_addr_s);
    mreq.imr_interface.s_addr=htonl(INADDR_ANY);
    if (msock_lset_opt(sub, IPPROTO_IP, IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) == 0){
        fprintf(stderr,"%*s %s\n",wstat,"msock_lset_opt IP_ADD_MEMBERSHIP","OK");
    }else {
        fprintf(stderr,"%*s %s\n",wstat,"msock_lset_opt IP_ADD_MEMBERSHIP","ERR");
        perror("setsockopt IP_ADD_MEMBERSHIP");
        exit(1);
    }

    // enter main loop
    while (!g_mcast_interrupt) {
        int wmsg=OFMT_WMSG;
        int64_t tx_bytes=0;
        char rxbuf[MSGBUFSIZE];

        memset(rxbuf,0,MSGBUFSIZE);

        bool rxok=false;
        struct sockaddr_in *psa = (struct sockaddr_in *)sub->addr->ainfo->ai_addr;

        // read message from PUB host
        if ((tx_bytes=msock_recvfrom(sub,sub->addr,(byte *)rxbuf,MSGBUFSIZE,0)) >0){

            rxok=true;
            // format and print message
            // summary
            fprintf(stderr,"SUB - mrx msg[%-*s] len[%7"PRId64"]  src[%s : %hu]\n",
                    wmsg, (aout_en?rxbuf:""), tx_bytes,
                    inet_ntoa(psa->sin_addr),
                    htons(psa->sin_port));

            // hex bytes
            if(xout_en)
                s_msg_xshow(rxbuf,tx_bytes);
        }else{
            if(errno!=EAGAIN)
                perror("recvfrom");
        }

        // respond to MSG packets (mcast pub)
        if(rxok && bidir_en && strstr(rxbuf,"MSG")!=NULL){
            char txbuf[MSGBUFSIZE];

            // generate a PNG message
            memset(txbuf,0,MSGBUFSIZE);
            int mid = -1;
            char *pid=NULL;
            if( (pid=strstr(rxbuf,"mid["))!=NULL){
                sscanf(pid,"mid[%d",&mid);
            }
            sprintf(txbuf,"PNG mid[%d] cid[%d] ",mid,getpid());
            size_t tx_len = strlen(txbuf)+1;


            // send PNG message to PUB host (use recvfrom addr)
            if( (tx_bytes=msock_sendto(sub, sub->addr, (byte *)txbuf, tx_len, 0))>0){

                fprintf(stderr,"SUB - utx msg[%-*s] len[%3zu/%-3"PRId64"] dest[%s : %hu]\n",
                        wmsg, txbuf, tx_len,tx_bytes,
                        inet_ntoa(psa->sin_addr),
                        htons(psa->sin_port));

                // clear/init recvfrom address
                memset(rxbuf,0,MSGBUFSIZE);
                int64_t rx_bytes=0;

                // attempt to read ACK
                if ((rx_bytes=msock_recvfrom(sub,sub->addr,(byte *)rxbuf,MSGBUFSIZE,0)) > 0){

                    fprintf(stderr,"SUB - urx msg[%-*s] len[%7"PRId64"]  src[%s : %hu]\n",
                            wmsg, rxbuf, rx_bytes,
                            inet_ntoa(psa->sin_addr),
                            htons(psa->sin_port));
                }
            }else{
                fprintf(stderr,"sendto failed[%d/%s]\n",errno, strerror(errno));
            }
        }// if PUB message

        if(cycles>0){
            if(--cycle_count==0){
                fprintf(stderr,"Exiting after [%d] cycles\n",cycles);
                break;
            }
        }
    }

    fprintf(stderr,"destroying socket\n");
    msock_socket_destroy(&sub);
    return 0;
}


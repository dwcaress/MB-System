/// @file udpms_test.c
/// @authors k. headley
/// @date 16apr2022

/// Summary: Test for UDP multicast subscriber component
/// compile using
/// gcc -o udpms-test -I. -I../../trnw -I../../terrain-nav  -L../../build -DWITH_TRNU udpms_test.c -ludpm_sub.o

// ///////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute
// Distributed under MIT license. See LICENSE file for more information.

// /////////////////
// Includes
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "udpm_sub.h"

#ifdef WITH_TRNU
#include <math.h>
#include "trn_msg.h"
#endif

// /////////////////
// Macros

#define TRACE() fprintf(stderr, "%s:%d \n", __func__, __LINE__)

// /////////////////
// Types
typedef struct app_cfg_s{
    char *mhost;
    int mport;
    int ttl;
}app_cfg_t;

// //////////////////////
// Declarations
static void s_termination_handler (int signum);

// //////////////////////
// Module Global Variables

static bool g_mcast_interrupt = false;
void s_show_hex(byte *src, int64_t len);
void s_parse_args(int argc, char **argv, app_cfg_t *cfg);
void s_show_help();

#ifdef WITH_TRNU
static int s_trnu_str(trnu_pub_t *update, char *dest, int len, int indent);
#endif

// //////////////////////
// Function Definitions

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

void s_show_hex(byte *src, int64_t len)
{
    if(NULL != src && len>0)
    {
        int col=0;

        for(int i=0; i<len; i++)
        {
            if(col==0)
                fprintf(stderr,"%08X : ",i);

            fprintf(stderr,"%02X ",src[i]);

            if(col>0 && (col%15)==0){
                fprintf(stderr,"\n");
                col=0;
            }else{
                col++;
            }
        }
        fprintf(stderr,"\n");
    }
}

#ifdef WITH_TRNU
static int s_trnu_str(trnu_pub_t *update, char *dest, int len, int indent)
{
    int retval=0;
    if(NULL!=update && NULL!=dest && len>0){
        int wkey=15;
        int wval=15;
        int rem=len;
        char *dp=dest;
        int wbytes=snprintf(dp,rem,"%*s %*s  %*p\n",indent,(indent>0?" ":""), wkey,"addr",wval,update);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*.3lf\n",indent,(indent>0?" ":""), wkey,"mb1_time",wval,update->mb1_time);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*.3lf\n",indent,(indent>0?" ":""), wkey,"update_time",wval,update->update_time);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*.3lf\n",indent,(indent>0?" ":""), wkey,"reinit_time",wval,update->reinit_time);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*s%08X\n",indent,(indent>0?" ":""), wkey,"sync",(wval-8)," ",update->sync);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"reinit_count",wval,update->reinit_count);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*.3lf\n",indent,(indent>0?" ":""), wkey,"reinit_tlast",wval,update->reinit_tlast);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"filter_state",wval,update->filter_state);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"success",wval,update->success);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*hd\n",indent,(indent>0?" ":""), wkey,"is_converged",wval,update->is_converged);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*hd\n",indent,(indent>0?" ":""), wkey,"is_valid",wval,update->is_valid);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"mb1_cycle",wval,update->mb1_cycle);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"ping_number",wval,update->ping_number);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"n_con_seq",wval,update->n_con_seq);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"n_con_tot",wval,update->n_con_tot);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"n_uncon_seq",wval,update->n_uncon_seq);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"n_uncon_tot",wval,update->n_uncon_tot);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s\n",indent,(indent>0?" ":""), wkey,"estimates:");
        rem-=(wbytes-1);
        dp+=wbytes;
        int i=0;
        for(i=0;i<5;i++){
            trnu_estimate_t *est = &update->est[i];
            const char *est_labels[5]={"pt", "mle", "mmse", "offset", "last_good"};
            //            const char *cov_labels[4]={"pt.covx", "pt.covy","pt.covz","pt.covxy"};
            wbytes=snprintf(dp,rem,"%*s %*s[%d]   %.3lf,%s,%.3lf,%.3lf,%.3lf,%.3lf,%.3lf,%.3lf,%.3lf\n",indent,(indent>0?" ":""),wkey-3," ",i,
                            est->time,est_labels[i],
                            est->x,est->y,est->z,
                            est->cov[0],est->cov[1],est->cov[2],est->cov[3]);

            rem-=(wbytes-1);
            dp+=wbytes;
        }

        wbytes=snprintf(dp,rem,"%*s %*s\n",indent,(indent>0?" ":""), wkey,"Bias Estimates:");
        rem-=(wbytes-1);
        dp+=wbytes;
        trnu_estimate_t *ept = &update->est[TRNU_EST_PT];
        //        trnu_estimate_t *emle = &update->est[TRNU_EST_MLE];
        trnu_estimate_t *emmse = &update->est[TRNU_EST_MMSE];
        trnu_estimate_t *offset = &update->est[TRNU_EST_OFFSET];
        trnu_estimate_t *last_good = &update->est[TRNU_EST_LAST_GOOD];
        wbytes=snprintf(dp,rem,"%*s %*s %.3lf,%.3lf,%.3lf\n",indent,(indent>0?" ":""), wkey," OFFSET:",offset->x,offset->y,offset->z);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s %.3lf,%.3lf,%.3lf\n",indent,(indent>0?" ":""), wkey," LAST:",last_good->x,last_good->y,last_good->z);
        rem-=(wbytes-1);
        dp+=wbytes;
        //       wbytes=snprintf(dp,rem,"%*s %*s %.3lf,%.3lf,%.3lf\n",indent,(indent>0?" ":""), wkey," MLE:",(emle->x-ept->x),(emle->y-ept->y),(emle->z-ept->z));
        //        rem-=(wbytes-1);
        //        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s %.3lf,%.3lf,%.3lf\n",indent,(indent>0?" ":""), wkey,"MMSE:",(emmse->x-ept->x),(emmse->y-ept->y),(emmse->z-ept->z));
        rem-=(wbytes-1);
        dp+=wbytes;
        snprintf(dp,rem,"%*s %*s %.3lf,%.3lf,%.3lf\n",indent,(indent>0?" ":""), wkey," COV:",sqrt(emmse->cov[0]),sqrt(emmse->cov[1]),sqrt(emmse->cov[2]));

        retval=strlen(dest)+1;
    }
    return retval;
}
#endif

void s_show_help()
{
    char help_message[] = "\n Test udpm_sub\n";

    char use_message[] = "\n use : udpms-test [options]\n"
    " options:\n"
    "  -a s : multicast group IP address\n"
    "  -p i : multcast group port\n"
    "  -t i : multicast ttl\n"
    "  -h   : show help\n\n";
    fprintf(stderr, "%s", help_message);
    fprintf(stderr, "%s", use_message);
}

void s_parse_args(int argc, char **argv, app_cfg_t *cfg)
{
    if(argc>1 && NULL!=cfg)
    {
        for(int i=1;i<argc;i++)
        {
            if(strcmp(argv[i], "-a")==0 && (i+1)<argc){
                free(cfg->mhost);
                cfg->mhost = strdup(argv[i+1]);
                i++;
            } else if (strcmp(argv[i], "-p")==0 && (i+1)<argc) {
                cfg->mport = atoi(argv[i+1]);
                i++;
            } else if (strcmp(argv[i], "-t")==0 && (i+1)<argc) {
                cfg->ttl = atoi(argv[i+1]);
                i++;
            } else {
                s_show_help();
                exit(0);
            }
        }
    }

    fprintf(stderr,"using:\n");
    fprintf(stderr,"host : %s\n",cfg->mhost);
    fprintf(stderr,"port : %d\n",cfg->mport);
    fprintf(stderr,"ttl  : %d\n",cfg->ttl);
}

int main(int argc, char **argv)
{
    // configure signal handling
    // for main thread
    struct sigaction saStruct;
    sigemptyset(&saStruct.sa_mask);
    saStruct.sa_flags = 0;
    saStruct.sa_handler = s_termination_handler;
    sigaction(SIGINT, &saStruct, NULL);

    app_cfg_t cfg_s = {
        strdup(UDPMS_GROUP_DFL),
        7667,
        UDPMS_TTL_DFL
    }, *cfg = &cfg_s;

    s_parse_args(argc, argv, cfg);

    udpm_sub_t *sub = udpms_cnew(cfg->mhost, cfg->mport, cfg->ttl);

    while(!g_mcast_interrupt)
    {
        if(!udpms_is_connected(sub)){
            if(udpms_connect(sub, true, false, false)!=0){
                sleep(5);
            }
        }else{
            byte iobuf[512]={0};
            memset(iobuf, 0, 512);
            int64_t test = udpms_listen(sub, iobuf, 512, 3000, 0);
            if(test>0){
                s_show_hex(iobuf, 512);
#ifdef WITH_TRNU
                trnu_pub_t *trnu = (trnu_pub_t *)iobuf;
                char sbuf[4096]={0};
                s_trnu_str(trnu, sbuf, 4096, 3);
                fprintf(stderr,"\ntrnu:\n%s\n", sbuf);
#endif
            }
        }
    }

    udpms_destroy(&sub);
    free(cfg->mhost);
    return 0;
}

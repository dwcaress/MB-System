///
/// @file mtime.c
/// @authors k. Headley
/// @date 01 jan 2018
 
/// mframe cross-platform time wrappers implementation
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
#include "mtime.h"

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

double mtime_dtime()
{
    double retval=0.0;

#if defined(__linux__) || defined(__CYGWIN__) || defined(__QNX__)
    struct timespec now={0};
    clock_gettime(MTIME_DTIME_CLOCK, &now);
    retval=((double)now.tv_sec+((double)now.tv_nsec/(double)1.0e9));
#elif defined(__MACH__) || defined(__APPLE__)
    struct timespec now={0};
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), MTIME_DTIME_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    now.tv_sec = mts.tv_sec;
    now.tv_nsec = mts.tv_nsec;
    retval=((double)now.tv_sec+((double)now.tv_nsec/(double)1.0e9));
#else
    fprintf(stderr,"mtime_dtime - not implemented\n");
#endif

    return retval;
}
// End function mtime_dtime

double mtime_etime()
{
    double retval=0.0;

    struct timespec ts;
    memset(&ts,0,sizeof(struct timespec));

#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), MTIME_ETIME_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    ts.tv_sec = mts.tv_sec;
    ts.tv_nsec = mts.tv_nsec;
#else
    clock_gettime(MTIME_ETIME_CLOCK, &ts);
#endif

    retval=ts.tv_sec+(ts.tv_nsec/1.e9);

    return retval;
}
// End function mtime_etime

double mtime_mdtime(double mod)
{
    double retval = 0.0;
    double now = mtime_dtime();

    if (mod>0.0) {
        retval = fmod(now,mod);
    }else{
        retval =  now;
    }
  
    return retval;
}
// End function mtime_mdtime

void mtime_delay_ns(uint32_t nsec)
{

    long lnsec = nsec;
    struct timespec delay;
    struct timespec rem;
    delay.tv_sec= (lnsec/1000000000);
    delay.tv_nsec=(lnsec%1000000000);

    memset(&rem,0,sizeof(struct timespec));
//    fprintf(stderr,"%s:%d - s[%ld] ns[%ld]\r\n",__FUNCTION__,__LINE__,delay.tv_sec,delay.tv_nsec);
    while ( nanosleep(&delay,&rem)!=0) {
        memcpy(&delay,&rem,sizeof(struct timespec));
        memset(&rem,0,sizeof(struct timespec));
    }
}
// End function mtime_delay_nsec

void mtime_delay_ms(uint32_t msec)
{

    uint32_t sec = msec/1000;
    uint32_t nsec = (msec%1000)*1000000;
//    fprintf(stderr,"%s:%d - s[%"PRIu32"] ns[%"PRIu32"]\r\n",__FUNCTION__,__LINE__,
//            sec,nsec);
	uint32_t i=0;
    for(i=0;i<sec;i++)mtime_delay_ns(1e9);
    if(nsec>0)
    mtime_delay_ns(nsec);
}
// End function mtime_delay_ms

void mtime_alloc_splits(mtime_stopwatch_t *self, unsigned int n)
{

    if(NULL!=self ){
        if(n==0){
            if(NULL!=self->split){
                free(self->split);
            }
            self->split=NULL;
            self->nsplits=0;
                
        }else{
            self->split=(double *)realloc(self->split,n*sizeof(double));
            if(NULL!=self->split){
                self->nsplits=n;
                mtime_clr_splits(self);
            }else{
                self->nsplits=0;
            }

        }
    }
}
// End function mtime_alloc_splits

void mtime_clr_splits(mtime_stopwatch_t *self)
{
    if(NULL!=self && NULL!=self->split && self->nsplits>0)
        memset(self->split,0,self->nsplits*sizeof(double));
}
// End function mtime_clr_splits

void mtime_sw_new(mtime_stopwatch_t **pself, unsigned int nsplits)
{
    if(NULL!=pself){
        uint32_t alloc_size=sizeof(mtime_stopwatch_t);
        
        mtime_stopwatch_t *instance = (mtime_stopwatch_t *)malloc(alloc_size);
        
        if(NULL!=instance){
            memset(instance,0,alloc_size);
            instance->split=NULL;
            instance->nsplits=0;
            if(nsplits>0){
                instance->split = (double *)malloc(nsplits*sizeof(double));
                instance->nsplits=nsplits;
            }
        }
        mtime_stopwatch_t *self = *pself;
        if(NULL!=self){
            mtime_sw_destroy(pself);
        }
        *pself = instance;
    }
}
// End function mtime_sw_new

void mtime_sw_destroy(mtime_stopwatch_t **pself)
{
    if(NULL!=pself){
        mtime_stopwatch_t *self = *pself;
        if(NULL!=self){
            if(NULL!=self->split)
                free(self->split);
            free(self);
            *pself=NULL;
        }
    }
}
// End function mtime_sw_destroy

int mtime_clock_getres(int clock_id, struct timespec *res)
{
    unsigned long retval = -1;
    
#if defined(__MACH__)
    //kern_return_t          kr;
    mach_msg_type_number_t count;
    clock_serv_t           cclock;
    natural_t              attribute[4];
    
    // could use kr to get, check return value
    host_get_clock_service(mach_host_self(), MTIME_DTIME_CLOCK,(clock_serv_t *)&cclock);
    
    count = sizeof(attribute)/sizeof(natural_t);
    // could use kr to get, check return value
    clock_get_attributes(cclock, CLOCK_GET_TIME_RES,(clock_attr_t)attribute, &count);
    res->tv_nsec=attribute[0];
    mach_port_deallocate(mach_task_self(), cclock);
    retval=0;
    
    // this is how to get time of day (not relevant here)
    //    mach_timespec_t        timespec;
    //    struct timeval         t;
    //    kr = clock_get_time(cclock, &timespec);
    //    gettimeofday(&t, NULL);
    
    
#else
    if(clock_getres(clock_id,res)==0){
        retval = 0;
    }
#endif
    return retval;
}
// End function mtime_clock_getres

int mtime_clock_setres(int clock_id, struct timespec *res)
{
    unsigned long retval = -1;
    
#if defined(__MACH__) || defined(__linux__)
// not supported on OS X
#else
    if(clock_setres(clock_id,res)==0){
        // maybe supported on linux
        // is supported on QNX4
        retval = 0;
    }
#endif
    return retval;
}
// End function mtime_clock_setres

#ifdef WITH_MTIME_TEST
/// @fn int32_t mtime_test()
/// @brief test mtime API
/// @return 0 on success, -1 otherwise
int mtime_test(int argc, char **argv)
{
    int retval=-1;
    
    mtime_stopwatch_t *swatch=NULL;
    int loop_count=20;
    int lc=0,oc=0,sc=0;
    int test=0;
    int op_count=10000;
    unsigned int clk_res=500000;
    
    // parse command line
    for(lc=0;lc<argc;lc++){
        if(strcmp(argv[lc],"-h")==0){
            fprintf(stderr," use: %s [options] \n",argv[0]);
            fprintf(stderr,"   -l n : number of loops\n");
            fprintf(stderr,"   -o n : number of times to do operation\n");
            fprintf(stderr,"   -r n : clock resolution (nsec, QNX only)\n");
            fprintf(stderr," \n");
            exit(0);
        }
        if(strcmp(argv[lc],"-l")==0 && (lc+1)<argc){
            sscanf(argv[lc+1],"%d",&loop_count);
            lc++;
            continue;
        }
        if(strcmp(argv[lc],"-o")==0 && (lc+1)<argc){
            if(sscanf(argv[lc+1],"%d",&op_count)==1){
                lc++;
                continue;
            }
        }
        if(strcmp(argv[lc],"-r")==0 && (lc+1)<argc){
            if(sscanf(argv[lc+1],"%u",&clk_res)==1){
                lc++;
                continue;
            }
        }
    }

    // allocate a stopwatch, with <loop_count> split times to make loop measurements
    // release when done using MTIME_SW_DESTROY to prevent memory leaks
    MTIME_SW_NEW(&swatch,loop_count);
    
    fprintf(stderr,"loop_count[%d]\n",loop_count);
    fprintf(stderr,"op_count[%d]\n",op_count);
    fprintf(stderr,"clk_res    [%u]\n",clk_res);
    
    // get clock resolution
    test = MTIME_SW_GETRES(swatch);
    fprintf(stderr,"clock getres[%ld] [%d, %d/%s]\n",MTIME_SW_RES(swatch),test,errno,strerror(errno));
#if defined(__QNX__)
    if(MTIME_DTIME_CLOCK==CLOCK_REALTIME)fprintf(stderr,"clock is CLOCK_REALTIME\n");
    // set clock resolution (not supported on all platforms)
    swatch->res.tv_nsec=clk_res;
    MTIME_SW_SETRES(swatch,clk_res);
    fprintf(stderr,"clock setres[%ld] [%d, %d/%s]\n",MTIME_SW_RES(swatch),test,errno,strerror(errno));
#endif

    sc=0;
    
    // get start time
    MTIME_SW_START(swatch,mtime_dtime());

    // do stuff...e.g. measure how long it takes to
    // run mtime_dtime <op_count> times
    for(lc=0;lc<loop_count;lc++){
        // time how long it takes to call a function a number of times
        for(oc=0;oc<op_count;oc++){
            mtime_dtime();
        }
        // mark split time for each loop
        MTIME_SW_SET_SPLIT(swatch,sc,mtime_dtime());
        sc++;
    }
    
    // get stop time
    MTIME_SW_STOP(swatch,mtime_dtime());
    // save elapsed time (stop-start)
    MTIME_SW_EL_SAVE(swatch);
    
    // Note the macros outside this block compile out if MTIME_STOPWATCH_EN
    // is not defined; this block has logic, IO, etc.
    // could wrap using #ifdef MTIME_STOPWATCH_EN
    // or use the stopwatch reference for minor impact and less clutter
    if(NULL!=swatch){
        
        // process split times, get avg, etc.
        fprintf(stderr," lap      split         tlap     tmin     tmax    sum\n");
        fprintf(stderr,"         [n/n+1]\n");
        double tsum=0.0, tmin=100.0, tmax=-100.0;
        for(lc=0;lc<loop_count-1;lc++){
            double tlap=MTIME_SW_GET_SPLIT(swatch,lc,lc+1);
            tsum+=tlap;
            if(tlap>tmax)tmax=tlap;
            if(tlap<tmin)tmin=tlap;
            if(NULL!=swatch->split){
                fprintf(stderr,"%02d-%02d  %.4lf/%.4lf  %+.4lf  %+.4lf  %+.4lf %+.4lf\n",lc,lc+1,swatch->split[lc]-swatch->start, swatch->split[lc+1]-swatch->start,tlap,tmin,tmax,tsum);
                
            }else{
                fprintf(stderr,"%02d-%02d  %.4lf/%.4lf  %+.4lf  %+.4lf  %+.4lf  %+.4lf\n",lc,lc+1,0., 0.,tlap,tmin,tmax,tsum);
            }
        }

        fprintf(stderr,"start[%.4lf] stop[%.4lf] start-stop[%.4lf]\n",swatch->start,swatch->stop,(swatch->stop-swatch->start));
        fprintf(stderr,"SW elapsed[%.4lf]\n",MTIME_SW_ELAPSED(swatch));
        fprintf(stderr,"lc[%d] oc[%d] tmin[%e] tmax[%e] sum[%e] avg[%.4e/%.4e]\n",loop_count,op_count,tmin,tmax,tsum,(tsum/loop_count),((tsum/loop_count)/op_count));
        
        retval=0;
    }
    
    // release stopwatch resources
    MTIME_SW_DESTROY(&swatch);

    double et_now = mtime_etime();
    time_t tt_now = time(NULL);
    fprintf(stderr,"etime[%0.3lf] ttnow[%ld]\n",et_now,tt_now);
    return retval;
}
#endif // WITH_MTIME_TEST

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

/// @fn double mtime_dtime()
/// @brief get system time as a double
/// @return system time as double, with usec precision
///  if supported by platform
double mtime_dtime()
{
    double retval=0.0;
    struct timespec now={0};

#if defined(__linux__) || defined(__CYGWIN__)
    clock_gettime(CLOCK_MONOTONIC, &now); //CLOCK_REALTIME, CLOCK_MONOTONIC_RAW, CLOCK_PROCESS_CPUTIME_ID
    retval=((double)now.tv_sec+((double)now.tv_nsec/(double)1.0e9));
#elif defined(__MACH__)
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
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

/// @fn double mtime_mdtime(double mod)
/// @brief get system time as a double
/// @return system time as double, with usec precision
///  if supported by platform
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

/// @fn void mtime_delay_ns(uint32_t nsec)
/// @brief delay for specied period
/// @param[in] nsec delay period (nsec)
/// @return none
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
/// @fn void mtime_delay_ms(uint32_t msec)
/// @brief delay for specied period
/// @param[in] msec delay period (msec)
/// @return none
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

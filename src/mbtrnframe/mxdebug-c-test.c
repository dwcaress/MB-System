///
/// @file mxdebug-c-test.c
/// @authors k. Headley
/// @date 15 jul 2023

/// Unit test wrapper for mxdebug C API

/// To compile test build mframe using
/// make clean WITH_MXDEBUG_TEST=1 all
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

#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include "mxdebug.h"

/*
 options:
 -DWITHOUT_MXDEBUG
 -DWITHOUT_PTHREAD_MUTEX

 g++ -c -g -O0  -DMXDEBUG_C_API -I. -I/usr/local/include -I/usr/include mxdebug.cpp -o mxdebug.o
 gcc -g -O0  -I. -I/usr/local/include -I/usr/include  mxdc-test.c mlist.c -o mxdc-test mxdebug.o
*/

#ifdef WITH_MXDEBUG_TEST

typedef enum{
    MFOO = MX_APP_RANGE,
    MBAR,
    MBAZ,
    MQUUX,
    MXXX
}mod_id_t;

static void init_debug()
{
    mxd_setModule(MFOO, 1, false, "foo");
    mxd_setModule(MBAR, 1, false, "bar");
}

static void test_fn()
{
    MX_MPRINT(MFOO, "%s:%d\n", __func__, __LINE__);
    mxd_setModule(MBAZ, 2, false, "baz");
}

static void *worker_fn(void *arg)
{
    int id = *((int *)(arg));
    char name[32]={0};
    snprintf(name, 32, "worker.%03d", id);

    fprintf(stderr, "%s worker name %s id %d\n", __func__, name, id);

    mxd_setModule(id, 2, false, name);

    for(int i = 0; i < 5; i++)
    {
        MX_MPRINT(id, "%s:%d MPRINT(%03d) name: %s level: %d\n", __func__, __LINE__, id, name, mxd_level(id));
        MX_LPRINT(id, 1, "%s:%d LPRINT(%s, 1)\n", __func__, __LINE__, name);
        MX_LPRINT(id, 2, "%s:%d LPRINT(%s, 2)\n", __func__, __LINE__, name);
        MX_LPRINT(id, 3, "%s:%d LPRINT(%s, 3)\n", __func__, __LINE__, name);
        MX_MBPRINT(id, (i%2 == 0), "%s:%d MPRINT(%s) %c\n", __func__, __LINE__, name, MXD_BOOL2CH((i%2 == 0)));
    }

    mxd_fshow(stderr, 0);

    fprintf(stderr, "%s - calling mxd_removeModule id %d\n", __func__, id);
    mxd_removeModule(id);

    return NULL;
}

static int mxdebug_c_test(int argc, char **argv)
{
    int retval = 0;

    init_debug();

    MX_MPRINT(MFOO, "%s:%d MFOO MPRINT\n", __func__, __LINE__);

    test_fn();

    MX_MPRINT(MBAR, "%s:%d MBAZ level: %d\n", __func__, __LINE__, mxd_level(MBAZ));
    MX_LPRINT(MBAZ, 1, "%s:%d LPRINT(MBAZ, 1)\n", __func__, __LINE__);
    MX_LPRINT(MBAZ, 2, "%s:%d LPRINT(MBAZ, 2)\n", __func__, __LINE__);
    MX_LPRINT(MBAZ, 3, "%s:%d LPRINT(MBAZ, 3)\n", __func__, __LINE__);
    MX_MPRINT(MXXX, "%s:%d MPRINT(MXXX)\n", __func__, __LINE__);
    MX_PRINT("%s:%d PRINT\n", __func__, __LINE__);

    MX_BPRINT( true ,"%s:%d BPRINT(true)\n", __func__, __LINE__);
    MX_BPRINT( false ,"%s:%d BPRINT(false)\n", __func__, __LINE__);
    MX_MBPRINT(MBAZ, true ,"%s:%d MBPRINT(MBAZ, true)\n", __func__, __LINE__);
    MX_MBPRINT(MBAZ, false ,"%s:%d MBPRINT(MBAZ, false)\n", __func__, __LINE__);

    MX_DEBUG("%s:%d MX_DEBUG\n", __func__, __LINE__);
    MX_INFO("%s:%d MX_INFO\n", __func__, __LINE__);
    MX_WARN("%s:%d MX_WARN\n", __func__, __LINE__);
    MX_ERROR("%s:%d MX_ERROR\n", __func__, __LINE__);

    mxd_show();
    mxd_removeModule(MFOO);
    mxd_setName(MBAZ, "mDude");
    mxd_show();
    mxd_removeModule(MBAZ);
    mxd_show();
    mxd_setModule(MQUUX, 1, false, "quux");
    mxd_fshow(NULL, 5);
    fprintf(stderr, "test hasID(MQUUX) %c\n", MXD_BOOL2CH(mxd_hasID(MQUUX)));
    fprintf(stderr, "test hasID(MBAZ) %c\n", MXD_BOOL2CH(mxd_hasID(MBAZ)));
    fprintf(stderr, "releasing...\n");
    mxd_release();
    mxd_fshow(NULL, 5);

    int NTHREADS = 10;
    if(argc > 1)
        sscanf(argv[1], "%d", &NTHREADS);

    if(NTHREADS > 0){
        pthread_t tid[NTHREADS];
        int id[NTHREADS];

        fprintf(stderr, "\n\n----- Starting Thread Test N=%d -----\n",NTHREADS);
        fprintf(stderr, "size[%d]\n", mxd_size());

        for(int i=0; i < NTHREADS; i++){
            id[i] = i+5;
            fprintf(stderr, "+++ starting thread[worker.%d]\n", id[i]);
            pthread_create(&(tid[i]),
                           NULL,
                           &worker_fn, (void *)&id[i]);
        }
        for(int i=0; i < NTHREADS; i++){
            fprintf(stderr, "--- joining thread[worker.%d]\n", i+5);
            while(pthread_join(tid[i], NULL) != 0){
                struct timespec ts = {0,50000};
                nanosleep(&ts, NULL);
            }
        }
    }

    fprintf(stderr, "*** final\n");
    mxd_show();
    fprintf(stderr, "destroying...\n");
    mxd_destroy();
    mxd_show();
    fprintf(stderr, "done\n");
    return retval;
}
#endif

int main(int argc, char **argv)
{
    int retval=-1;
#ifdef WITH_MXDEBUG_TEST
    mxdebug_c_test(argc, argv);
#else
    fprintf(stderr,"mxdebug_c_test not implemented - compile using -DWITH_MXDEBUG_TEST (make WITH_MXDEBUG_TEST=1 ...)\r\n");
#endif
    return retval;
}

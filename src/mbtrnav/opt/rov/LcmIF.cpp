///
/// @file LcmIF.cpp
/// @authors k. headley
/// @date 2022-03-17

/// LCM wrapper for TRN

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
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <lcm/lcm-cpp.hpp>
#include "LcmIF.h"

/////////////////////////
// Macros
/////////////////////////

/////////////////////////
// Declarations
/////////////////////////

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////


///////////////////////////
// Function Definitions
///////////////////////////

LcmIF::LcmIF(std::string lcm_url)
{
    _lcm = new lcm::LCM(lcm_url);
    _running = false;
    _stop_worker = false;
}

LcmIF::~LcmIF()
{

    delete _lcm;
}

bool LcmIF::initialize()
{
    // make sure lcm initialized OK
    if( !_lcm->good() )
    {
        TRACE();
        return false;
    }

    TRACE();

    return true;
}


int add_sub(libtrnav::lcm_subscriber &subscriber)
{
    TRACE();
    return -1;
}

int LcmIF::start()
{
    TRACE();
    if(!_running){
        _stop_worker = false;
        _worker = std::thread(&LcmIF::worker_fn, this);
        _running = true;
    }

    return -1;
}

int LcmIF::stop()
{
    TRACE();
    if(_running){
        _stop_worker = true;
        TRACE();
        _worker.join();
        TRACE();
//        delete _worker;
    }
    return -1;
}

int LcmIF::worker_fn()
{
    TRACE();
    while(!_stop_worker){
        int rc = _lcm->handleTimeout(200);

        if ( rc == 0 ) {
            _handlerTimeout = true;
        } else {
            _handlerTimeout = false;
        }

        if ( rc < 0 ) {
            fprintf(stderr,"handler thread error.\n");
        }
    }
    TRACE();
    return 0;
}

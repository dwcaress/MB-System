///
/// @file LcmIF.h
/// @authors k. headley
/// @date 2022-03-17
/// LCM wrapper for TRN
///  Requires:
///   C++11
///   liblcm
///   libpthreads
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

// Always do this
#ifndef LCMIF_H
#define LCMIF_H

/////////////////////////
// Includes
/////////////////////////
#include <thread>
#include <stdint.h>
#include <stdbool.h>
#include <lcm/lcm-cpp.hpp>
#include "LcmSub.hpp"


/////////////////////////
// Macros
/////////////////////////

#define TRACE() fprintf(stderr,"%s:%d\n",__func__, __LINE__)


/////////////////////////
// Type Definitions
/////////////////////////

class LcmIF
{
public:
    typedef void (* handler_fn)();
    LcmIF(std::string lcm_url="");
    ~LcmIF();

    bool initialize();
    int add_sub(libtrnav::LcmSub &subscriber);
    int start();
    int stop();
    bool handler_timed_out() { return _handlerTimeout; }

    // clear handler time-outs
    void clear_handler_timeouts() { _handlerTimeout = false; }

protected:
private:
    int worker_fn();
    std::thread _worker;
    lcm::LCM *_lcm;
    bool _running;
    bool _stop_worker;
    bool _handlerTimeout;
};

// include guard
#endif

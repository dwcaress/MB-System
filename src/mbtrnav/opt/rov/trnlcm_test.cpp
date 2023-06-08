/// @file trnlcm_test.cpp
/// @authors k. headley
/// @date 21mar2022

/// Summary: test code for trn_lcm_input

// ///////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute
// Distributed under MIT license. See LICENSE file for more information.

// /////////////////
// Includes
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
// initalizers for header-only modules
// must be defined before any framework headers are included
#define INIT_PCF_LOG
#define INIT_TRN_LCM_INPUT
#include "lcm_interface.hpp"
#include "lcm_pcf/signal_t.hpp"
#include "trn_lcm_input.hpp"

// /////////////////
// Macros

// /////////////////
// Types

// /////////////////
// Module variables
static int g_signal=0;
static bool g_interrupt=false;

// /////////////////
// Declarations

// /////////////////
// Function Definitions


/// @fn void termination_handler (int signum)
/// @brief termination signal handler.
/// @param[in] signum signal number
/// @return none
static void s_termination_handler (int signum)
{
    switch (signum) {
        case SIGINT:
        case SIGHUP:
        case SIGTERM:
            fprintf(stderr,"INFO - sig received[%d]\n",signum);
            g_interrupt=true;
            g_signal=signum;
            break;
        default:
            fprintf(stderr,"ERR - s_termination_handler: sig not handled[%d]\n",signum);
            break;
    }
}

int main(int argc, char **argv)
{
    struct sigaction saStruct;
    sigemptyset(&saStruct.sa_mask);
    saStruct.sa_flags = 0;
    saStruct.sa_handler = s_termination_handler;
    sigaction(SIGINT, &saStruct, NULL);

    pcf::lcm_interface lcm("");

    lcm.initialize();

    lcm_pcf::signal_t signalMsg={0};
    lcm_pcf::string_t stringMsg;
//    double ts = 0.0;
    std::string msg;

    trn::trn_lcm_input signalSub("RAW_SIGNAL", 10);
    trn::trn_lcm_input stringSub("STRING_MSG", 10);
    pcf::lcm_publisher signalPub("RAW_SIGNAL");
    pcf::lcm_publisher stringPub("STRING_MSG");

    lcm.add_subscriber(signalSub);
    lcm.add_subscriber(stringSub);
    lcm.add_publisher(signalPub);
    lcm.add_publisher(stringPub);

    lcm.start();
    fprintf(stderr,"%s:%d\n",__func__, __LINE__);

    while(!g_interrupt){
        sleep(2);
        signalPub.publish(signalMsg);
        signalMsg.signal+=1.0;

        stringMsg.val = "Hello from stringPub! - " +
        std::to_string(signalPub.get_sequence());
        // publish the string
        stringPub.publish(stringMsg);
    }
    fprintf(stderr,"%s:%d\n",__func__, __LINE__);
    lcm.stop();

    fprintf(stderr,"%s:%d %s\n",__func__, __LINE__, pcf::get_iso8601_timestamp().c_str());

    return 0;
}

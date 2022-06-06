//////////////////////////////////////////////////////////////////////////////
// Copyright 2021  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef LCM_INTERFACE_HPP  // include guard
#define LCM_INTERFACE_HPP

#include <thread>
#include <string>
#include <iostream>

#include <lcm/lcm-cpp.hpp>

#include "pcf_log.hpp"
#include "lcm_pcf_msg.hpp"
#include "lcm_publisher.hpp"
#include "lcm_subscriber.hpp"

namespace pcf
{
// Class: lcm_interface
// An LCM interface class, see <LCM Publish/Subscribe Example> for usage
class lcm_interface : public log
{
public:

    //////////////////////////////////////////////////////////////////
    // Constructor: lcm_interface
    // create a lcm_interface object
    //
    // Parameters:
    // name - the object name used by <log>
    lcm_interface(const std::string& name = "UNKNOWN")
    {
        set_object_name(name);
        set_class_name("pcf::lcm_interface");
        mStopHandlerThread = true;

        mHandlerTimeout = false;
    }

    //////////////////////////////////////////////////////////////////
    // Function: initialize
    // initialize lcm_interface object
    //
    // Returns:
    // true if the lcm_interface object was successfully initialized
    //
    // false if the lcm_interface object initialization failed
    bool initialize()
    {
        // make sure lcm initialized OK
        if( !mLcm.good() )
        {
            err_msg("failed to initialize");
            return false;
        }

        info_msg("sucessfully initialized");

        return true;
    }

    //////////////////////////////////////////////////////////////////
    // Function: add_publisher
    // add a <lcm_publisher> object to the lcm_interface
    // (note: it is the programmer's responsibility to ensure that the
    // lcm_interface object does not outlive lcm_publisher object
    // referenced)
    void add_publisher(lcm_publisher& publisher)
    {
        info_msg("adding publisher: " + publisher.get_object_name());

        publisher.set_lcm_instance(mLcm);
    }

    //////////////////////////////////////////////////////////////////
    // Function: add_subscriber
    // add a <lcm_subscriber> object to the lcm_interface
    // (note: it is the programmer's responsibility to ensure that the
    // lcm_interface object does not outlive lcm_subscriber object
    // referenced)
    void add_subscriber(lcm_subscriber& subscriber)
    {
        // TODO: save the lcm::Subscription pointer returned by mLcm.subscribe(...) so
        // you can unsubscribe or change the subscription
        // https://lcm-proj.github.io/classlcm_1_1LCM.html#ac9b48c70992c4d0c36d782208ca4dd93

        info_msg("adding subscriber: " + subscriber.get_object_name());

        mLcm.subscribe(subscriber.get_channel_name(),
                       &lcm_subscriber::handle_msg,
                       &subscriber);
    }

    //////////////////////////////////////////////////////////////////
    // Function: get_lcm_instance
    // return a pointer to the underlying LCM instance, see
    // <LCM: https://lcm-proj.github.io/> documentation for details
    lcm_t* get_lcm_instance()
    {
        if( mLcm.good() )
            return mLcm.getUnderlyingLCM();

        return nullptr;
    }

    //////////////////////////////////////////////////////////////////
    // Function: start
    // start the handler thread for LCM messages
    void start()
    {
        // start the handler thread if it's not running
        if ( mStopHandlerThread )
        {
            mStopHandlerThread = false;
            mHandlerThread = std::thread(&lcm_interface::server, this);
        }

        return;
    }

    //////////////////////////////////////////////////////////////////
    // Function: stop
    // stop the handler thread for LCM messages
    void stop()
    {
        mStopHandlerThread = true;

        // wait for thread to exit
        mHandlerThread.join();

        return;
    }

    // check for handler time-outs
    bool handler_timed_out() { return mHandlerTimeout; }

    // clear handler time-outs
    void clear_handler_timeouts() { mHandlerTimeout = false; }

private:

    //message handling thread
    void server()
    {
        while( !mStopHandlerThread )
        {
            // handle lcm messages
            int rc = mLcm.handleTimeout(200);

            if ( rc == 0 )
                mHandlerTimeout = true;
            else
                mHandlerTimeout = false;

            if ( rc < 0 )
                err_msg("handler thread error.");
        }

        info_msg("message handling thread exited");
    }

private:

    // LCM vars
    lcm::LCM mLcm;
    // handler thread
    std::thread mHandlerThread;
    // handler thread stop signal
    bool mStopHandlerThread;
    // timeout event flag
    bool mHandlerTimeout;
};

}

#endif


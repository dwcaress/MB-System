//////////////////////////////////////////////////////////////////////////////
// Copyright 2021  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef LCM_PCF_MSG_HPP  // include guard
#define LCM_PCF_MSG_HPP

#include "pcf_utils.hpp"

namespace pcf
{
    // Class: lcm_pcf_msg
    // A PCF LCM message object
    template <typename T>
    class lcm_pcf_msg
    {
        public:

            lcm_pcf_msg()
            {
                mSequence = 0;
                mTimeStamp = 0.0;
            }

            //////////////////////////////////////////////////////////////////
            // Function: get_age
            // return the number of seconds elapsed since the message was
            // received.  The age will be reported as the seconds since the
            // epoch if no message has been received.
            double get_age() { return (pcf::get_timestamp() - mTimeStamp); }

            //////////////////////////////////////////////////////////////////
            // Function: get_sequence
            // return the number of messages that have been received by the
            // associated <lcm_subscriber> object.
            int64_t get_sequence() { return mSequence; }

            //////////////////////////////////////////////////////////////////
            // Variable: msg
            // LCM message object storage
            T msg;

        private:

            int64_t mSequence;
            double mTimeStamp;

            // allow lcm_subscriber_t to access private data
            friend class lcm_subscriber;
    };
}

#endif

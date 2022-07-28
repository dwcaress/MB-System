//////////////////////////////////////////////////////////////////////////////
// Copyright 2021  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef LCM_PUBLISHER_HPP  // include guard
#define LCM_PUBLISHER_HPP

#include <string>
#include <iostream>
#include <lcm/lcm-cpp.hpp>

#include "pcf_log.hpp"

namespace pcf
{
    // Class: lcm_publisher
    // An LCM publisher class, see <LCM Publish/Subscribe Example> for usage
    class lcm_publisher : public log
    {
        public:

            //////////////////////////////////////////////////////////////////
            // Constructor: lcm_publisher
            // create a lcm_publisher object
            //
            // Parameters:
            // name - the object name used by <log>, the object name
            // will also be used as the channel name unless another channel
            // name is set.
            lcm_publisher(const std::string& name = "UNKNOWN")
            {
                set_object_name(name);
                set_class_name("pcf::lcm_publisher");

                mDataLen = 0;
                mData = nullptr;
                mTxSequence = 0;
                mLcm_ptr = nullptr;
                mChannelName = name;
            }

            ~lcm_publisher()
            {
                if ( mData != nullptr )
                    delete[] mData;
            }

            //////////////////////////////////////////////////////////////////
            // Function: get_channel_name
            // return the name of the object as a std::string
            std::string get_channel_name() { return mChannelName; }

            //////////////////////////////////////////////////////////////////
            // Function: set_channel_name
            //
            // Parameters:
            // name - the channel name
            void set_channel_name(const std::string& name)
            {
                mChannelName = name;
            }

            //////////////////////////////////////////////////////////////////
            // Function: get_sequence
            // return the objects internal transmit sequence number for
            // LCM messages sent as an
            // <int64_t: https://en.cppreference.com/w/cpp/header/cstdint>
            int64_t get_sequence()
            {
                return mTxSequence;
            }

            //////////////////////////////////////////////////////////////////
            // Function: publish
            // publish an LCM message
            //
            // Parameters:
            // msg - a reference to an LCM message object
            //
            // Returns:
            // true - if the message was sucessfully published
            // false - if on failure to publish the message
            template<class T>
            bool publish(const T& msg)
            {
                // get the encoded size of the message
                int len = msg.getEncodedSize();

                // check the encoded size of the message
                if ( len > mDataLen )
                {
                    allocate_msg_buffer(len);

                    info_msg("resized encoding buffer to " +
                             std::to_string(len) + " bytes");
                }

                //encode the msg into a static buffer
                if ( msg.encode(mData, 0, len) < 0 )
                {
                    err_msg("failed to encode message into buffer");
                    return false;
                }

                // check the publisher instance
                if ( mLcm_ptr == nullptr )
                {
                    err_msg("no LCM instance, message not sent");
                    return false;
                }

                // publish the message
                if ( mLcm_ptr->publish(mChannelName, mData, mDataLen) < 0 )
                {
                    err_msg("LCM publish failure");
                    return false;
                }

                // increment on succesful send
                ++mTxSequence;

                return true;
            }

        private:

            void allocate_msg_buffer(int len)
            {
                // delete the old data buffer if it exists
                if ( mData != nullptr )
                    delete[] mData;

                // update the buffer length
                mDataLen = len;

                // allocate the storage for the data buffer
                mData = new uint8_t[mDataLen];
            }

            void set_lcm_instance(lcm::LCM& lcm) { mLcm_ptr = &lcm; }

        private:

            // LCM message buffer
            int mDataLen;
            uint8_t* mData;

            // tx stats for message
            int64_t mTxSequence;

            // LCM vars
            lcm::LCM* mLcm_ptr;
            std::string mChannelName;

            // for access to lcm_publisher::set_lcm_instance
            friend class lcm_interface;
    };
}

#endif

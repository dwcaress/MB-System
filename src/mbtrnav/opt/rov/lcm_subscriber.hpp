//////////////////////////////////////////////////////////////////////////////
// Copyright 2021  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef LCM_SUBSCRIBER_HPP  // include guard
#define LCM_SUBSCRIBER_HPP

#include <mutex>
#include <string>
#include <cstring>
#include <cstdint>

#include <lcm/lcm-cpp.hpp>

#include "pcf_log.hpp"
#include "semaphore.hpp"
#include "pcf_utils.hpp"
#include "lcm_pcf_msg.hpp"

// supress compiler warning
#define UNUSED(x) (void)(x)

namespace pcf
{
    // Class: lcm_subscriber
    // An LCM subscriber class, see <LCM Publish/Subscribe Example> for usage
    class lcm_subscriber : public log
    {
        public:

            //////////////////////////////////////////////////////////////////
            // Constructor: lcm_subscriber
            // create a lcm_subscriber object
            //
            // Parameters:
            // name - the object name used by <log>, the object name
            // will also be used as the channel name unless another channel
            // name is set.
            lcm_subscriber(const std::string& name = "UNKNOWN")
            {
                set_object_name(name);
                set_class_name("pcf::lcm_subscriber");

                mRxBufferUsec = 0;
                mRxBufferSize = 0;

                mData = nullptr;
                mDataLen = 0;

                mRxSequence = 0;

                mSemaphore = nullptr;

                mChannelName = name;
            }
            virtual ~lcm_subscriber() {}

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
            // return the objects internal receive sequence number for
            // LCM messages received as an
            // <int64_t: https://en.cppreference.com/w/cpp/header/cstdint>
            int64_t get_sequence()
            {
                // lock the mutex for this transaction
                std::lock_guard<std::mutex> lock(mDataMutex);

                return mRxSequence;
            }

            //////////////////////////////////////////////////////////////////
            // Function: get_timestamp
            // return an <epoch: https://en.wikipedia.org/wiki/Unix_time>
            // time-stamp in seconds of the most recently received LCM message
            // as a double
            double get_timestamp()
            {
                // lock the mutex for this transaction
                std::lock_guard<std::mutex> lock(mDataMutex);

                double ts = mRxBufferUsec;

                ts /= 1000000.0;

                return ts;
            }

            //////////////////////////////////////////////////////////////////
            // Function: get_message_age
            // return the age of the most recently received LCM message in
            // seconds as a double
            double get_message_age()
            {
                return (pcf::get_timestamp() - get_timestamp());
            }

            //////////////////////////////////////////////////////////////////
            // Function: set_semaphore
            // register a <semaphore> object with the lcm_subscriber object.
            // The semaphore object will be signaled when a new LCM message
            // arrives.  (note: it is the programmer's responsibility to
            // ensure that the lcm_subscriber object does not outlive
            // semaphore object referenced)
            void set_semaphore(semaphore& sem) { mSemaphore = &sem; }
// TODO: consider a container of semaphores that the handler could post.
// This would allow multiple threads to wait for data from the same channel

            //////////////////////////////////////////////////////////////////
            // Function: clear_semaphore
            // deregister the <semaphore> object from the lcm_subscriber
            // object.
            void clear_semaphore() { mSemaphore = nullptr; }

            //////////////////////////////////////////////////////////////////
            // Function: get_lcm_msg
            // get the most recent LCM message object
            //
            // Parameters:
            // msg - a reference to an LCM message object
            //
            // Returns:
            // The number of bytes copied to the LCM message object, or less
            // than zero if an error occured.
            template<class T>
            int get_lcm_msg(T& msg)
            {
                // lock the mutex for this transaction
                std::lock_guard<std::mutex> lock(mDataMutex);

                // return number of bytes decoded or -1 for error
                return msg.decode(mData, 0, mDataLen);
            }

            //////////////////////////////////////////////////////////////////
            // Function: get_pcf_msg
            // get the most recent PCF message object
            //
            // Parameters:
            // p_msg - a reference to an <lcm_pcf_msg> object
            //
            // Returns:
            // The number of bytes copied to the <lcm_pcf_msg> object, or less
            // than zero if an error occured.
            template<class T>
            int get_pcf_msg(T& p_msg)
            {
                // lock the mutex for this transaction
                std::lock_guard<std::mutex> lock(mDataMutex);

                p_msg.mSequence = mRxSequence;
                p_msg.mTimeStamp = (double)mRxBufferUsec;
                p_msg.mTimeStamp /= 1000000.0;

                // return number of bytes decoded or -1 for error
                return p_msg.msg.decode(mData, 0, mDataLen);
            }

        protected:

            //////////////////////////////////////////////////////////////////
            // Function: process_msg
            // derived classes can override this function to perform additional
            // message processing.  See <LCM Derived Subscriber Example> for
            // usage
        virtual void process_msg() { info_msg("processing message"); return; }

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

            void handle_msg(const lcm::ReceiveBuffer* rbuf,
                            const std::string& chan)
            {
                // chan is required by lcm::LCM::subscribe(...)
                UNUSED(chan);

                mDataMutex.lock();

                ++mRxSequence;
                mRxBufferUsec = rbuf->recv_utime;
                mRxBufferSize = rbuf->data_size;

                if ( mDataLen < rbuf->data_size)
                    allocate_msg_buffer(rbuf->data_size);

                std::memcpy(mData, rbuf->data, rbuf->data_size);

                // subclasses can perform additional process to message
                process_msg();

                mDataMutex.unlock();

                if ( mSemaphore != nullptr )
                    mSemaphore->post();

            }

        protected:

            std::string mChannelName;

            // lcm::ReceiveBuffer time-stamp and size
            int64_t mRxBufferUsec;
            uint32_t mRxBufferSize;

            // storage for lcm::ReceiveBuffer::data
            uint8_t* mData;
            uint32_t mDataLen;

            // rx stats for message
            int64_t mRxSequence;

            // mutex and semaphore
            std::mutex mDataMutex;
            semaphore* mSemaphore;

            // for access to lcm_subscriber::handle_msg
        friend class lcm_interface;
    };
}

#endif

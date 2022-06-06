//////////////////////////////////////////////////////////////////////////////
// Copyright 2021  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef SEMAPHORE_HPP
#define SEMAPHORE_HPP

#include <mutex>
#include <limits>
#include <chrono>
#include <condition_variable>

namespace pcf
{
// Class: semaphore
// A counting semaphore class
class semaphore
{
public:
    //////////////////////////////////////////////////////////////////
    // Constructor: semaphore
    // creates a counting semaphore object
    //
    // Parameters:
    // count - initial count value, default 0

    semaphore(unsigned int count = 0) { mCount = count; }
    semaphore(const semaphore &other):mCount(other.mCount){};
    
    //////////////////////////////////////////////////////////////////
    // Function: wait
    // wait for a semaphore count > 0

    inline void wait()
    {
        std::unique_lock<std::mutex> lock(mMutex);

        // wait for the condition
        mCondVar.wait(lock, [&]{ return mCount > 0; });

        --mCount;
    }

    //////////////////////////////////////////////////////////////////
    // Function: wait
    // wait for a semaphore count > 0 or until the specificed wait
    // time has expired
    //
    // Parameters:
    // ms - time to wait in milliseconds
    //
    // Returns:
    // true for a count > 0
    //
    // false if the time expired waiting for the count

    inline bool wait(int ms)
    {
        // set the timeout in milliseconds
        std::chrono::milliseconds to(ms);

        std::unique_lock<std::mutex> lock(mMutex);

        // wait for the condition or timeout
        if ( mCondVar.wait_for(lock, to, [&]{ return mCount > 0; }) )
        {
            // decrement the condition variable and return true
            --mCount;
            return true;
        }

        // return false on timeout
        return false;
    }

    //////////////////////////////////////////////////////////////////
    // Function: post
    // increment the semaphore count by 1

    inline void post()
    {
        std::unique_lock<std::mutex> lock(mMutex);

        if ( mCount < std::numeric_limits<unsigned int>::max() )
            ++mCount;

        mCondVar.notify_one();
    }

    //////////////////////////////////////////////////////////////////
    // Function: get_count
    // get the current semaphore count value
    //
    // Returns:
    // current count value of semaphore object

    inline unsigned int get_count()
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return mCount;
    }

    //////////////////////////////////////////////////////////////////
    // Function: clear_count
    // set the semaphore count value to 0
    //

    inline void clear_count()
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mCount = 0;
    }

private:
    unsigned int mCount;
    std::mutex mMutex;
    std::condition_variable mCondVar;

};
}

#endif

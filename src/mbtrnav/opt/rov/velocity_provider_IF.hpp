
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef VELOCITY_PROVIDER_IF_HPP  // include guard
#define VELOCITY_PROVIDER_IF_HPP

#include "flag_utils.hpp"

namespace trn {

typedef enum{
    VF_VALID=0x1,
    VF_BLOCK=0x2,
    VF_RLOCK=0x4
}vel_flag_bits_t;
typedef uint32_t vel_flags_t;

class vel_info
{
public:
    vel_info()
    : mTimeUsec(0), mFlags(0), mVxMSec(0), mVyMSec(0), mVzMSec(0)
    {
    }

    vel_info(double time_ems, double vx, double vy, double vz, vel_flags_t flags)
    : mTimeUsec(time_ems), mFlags(flags), mVxMSec(vx), mVyMSec(vy), mVzMSec(vz)
    {
    }

    vel_info(const vel_info &other)
    : mTimeUsec(other.mTimeUsec), mFlags(other.mFlags), mVxMSec(other.mVxMSec), mVyMSec(other.mVyMSec), mVzMSec(other.mVzMSec)
    {
    }

    ~vel_info()
    {
    }

    double time_usec()
    {
        return mTimeUsec;
    }

    flag_var<uint32_t> &flags()
    {
        return mFlags;
    }

    double vx_ms()
    {
        return mVxMSec;
    }

    double vy_ms()
    {
        return mVyMSec;
    }

    double vz_ms()
    {
        return mVzMSec;
    }

    const char *velstr()
    {
        mStrBuf.clear();
        std::ostringstream ss;
        ss << std::dec << std::setfill(' ') << std::fixed << std::setprecision(3);
        ss << time_usec() << ",";
        ss << "x" << std::hex << std::setfill('0') << std::setw(8);
        ss << mFlags.get() << ",";
        ss << std::dec << std::setfill(' ') << std::fixed << std::setprecision(3);
        ss << mVxMSec << ",";
        ss << mVyMSec << ",";
        ss << mVzMSec;

        mStrBuf = ss.str();
        return mStrBuf.c_str();
    }

protected:
    double mTimeUsec;
    flag_var<uint32_t> mFlags;
    double mVxMSec;
    double mVyMSec;
    double mVzMSec;
    std::string mStrBuf;
};

// Navigation provider interface API
// (time, lat, lon, depth)
class velocity_provider_IF 
{
public:
    virtual vel_info *vel_inst() = 0;
    virtual vel_info *vel_filt()
    {
        return nullptr;
    }
};

}

#endif

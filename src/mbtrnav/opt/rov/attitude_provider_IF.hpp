
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef ATT_PROVIDER_IF_HPP  // include guard
#define ATT_PROVIDER_IF_HPP

#include <tuple>
#include "flag_utils.hpp"

namespace trn {
// pitch<0>, roll<1>, heading<2> (radians)
using att_tup = std::tuple<double, double, double>;

typedef enum{
    AF_VALID=0x1,
    AF_INVERT_PITCH=0x2,
    AF_INVERT_ROLL=0x4,
}att_flag_bits_t;

typedef uint32_t att_flags_t;

typedef enum{
    PA_RADIANS=0,
    PA_DEGREES
}att_angle_units_t;

class att_info
{
public:
    att_info()
    : mTimeUsec(0.), mAttitude(0,0,0), mFlags()
    {
    }

    att_info(double time_ems, const att_tup att, const flag_var<nav_flags_t> &flags)
    : mTimeUsec(time_ems), mAttitude(0,0,0), mFlags(flags)
    {
    }

    att_info(double time_ems, double pitch, double roll, double heading, const flag_var<att_flags_t> &flags)
    : mTimeUsec(time_ems), mAttitude(pitch,roll,heading), mFlags(flags)
    {
    }

    att_info(const att_info &other)
    : mTimeUsec(other.mTimeUsec), mAttitude(other.mAttitude), mFlags(other.mFlags)
    {
    }

    ~att_info()
    {
    }

    bool valid()
    {
        return mFlags.is_set(AF_VALID);
    }

    double time_usec()
    {
        return mTimeUsec;
    }

    double pitch(att_angle_units_t u=PA_RADIANS)
    {
        double angle = (u==PA_DEGREES ? std::get<0>(mAttitude)*180./M_PI : std::get<0>(mAttitude));
        return (mFlags.is_set(AF_INVERT_PITCH) ? (angle * -1.) : angle);
    }

    double roll(att_angle_units_t u=PA_RADIANS)
    {
        double angle = (u==PA_DEGREES ? std::get<1>(mAttitude)*180./M_PI : std::get<1>(mAttitude));
        return (mFlags.is_set(AF_INVERT_ROLL) ? (angle * -1.) : angle);
    }

    double heading(att_angle_units_t u=PA_RADIANS)
    {
        double angle = (u==PA_DEGREES ? std::get<2>(mAttitude)*180./M_PI : std::get<2>(mAttitude));
        return angle;
    }

    flag_var<att_flags_t> &flags()
    {
        return mFlags;
    }

    const char *attstr()
    {
        mStrBuf.clear();
        std::ostringstream ss;
        ss << std::dec << std::setfill(' ') << std::fixed << std::setprecision(3);
        ss << time_usec() << ",";
        ss << "x" << std::hex << std::setfill('0') << std::setw(8);
        ss << mFlags.get() << ",";
        ss << std::dec << std::setfill(' ') << std::fixed << std::setprecision(5);
        ss << pitch() << ",";
        ss << roll() << ",";
        ss << heading();

        mStrBuf = ss.str();
        return mStrBuf.c_str();//mStrBuf;
    }

protected:
    double mTimeUsec;
    att_tup mAttitude;
    flag_var<nav_flags_t>mFlags;
    std::string mStrBuf;
 };

// Navigation provider interface API
// (time, lat, lon, depth)
class attitude_provider_IF
{
public:
    virtual att_info *att_inst() = 0;
    virtual att_info *att_filt()
    {
        return nullptr;
    }
};

}

#endif

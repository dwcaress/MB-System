
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef NAVIGATION_PROVIDER_IF_HPP  // include guard
#define NAVIGATION_PROVIDER_IF_HPP

#include <tuple>
#include "flag_utils.hpp"

namespace trn {

typedef enum{
    NF_POS_VALID=0x1,
    NF_DEPTH_VALID=0x2,
    NF_HAS_LOCK=0x4
}nav_flag_bits_t;
typedef uint32_t nav_flags_t;

class nav_info
{
public:

    nav_info()
    : mTimeUsec(0), mLat(0), mLon(0), mDepth(0), mFlags(0)
    {
    }
    
    nav_info(double time_ems, double lat, double lon, double depth, const flag_var<nav_flags_t> &flags)
    : mTimeUsec(time_ems), mLat(lat), mLon(lon), mDepth(depth), mFlags(flags)
    {
    }

    nav_info(const nav_info &other)
    : mTimeUsec(other.mTimeUsec), mLat(other.mLat), mLon(other.mLon), mDepth(other.mDepth), mFlags(other.mFlags)
    {
    }

    ~nav_info()
    {
    }

    bool pos_valid()
    {
        return mFlags.is_set(NF_POS_VALID);
    }

    bool depth_valid()
    {
        return mFlags.is_set(NF_DEPTH_VALID);
    }

    bool has_lock()
    {
        return mFlags.is_set(NF_HAS_LOCK);
    }

    double time_usec()
    {
        return mTimeUsec;
    }

    flag_var<nav_flags_t> &flags()
    {
        return mFlags;
    }

    void lat_lon(double &r_lat, double &r_lon)
    {
        r_lat = mLat;
        r_lon = mLon;
    }

    double lat()
    {
        return mLat;
    }

    double lon()
    {
        return mLon;
    }

    double depth()
    {
        return mDepth;
    }

    const char *navstr()
    {
        mStrBuf.clear();
        std::ostringstream ss;
        ss << std::dec << std::setfill(' ') << std::fixed << std::setprecision(3);
        ss << time_usec() << ",";
        ss << "x" << std::hex << std::setfill('0') << std::setw(8);
        ss << mFlags.get() << ",";
        ss << std::dec << std::setfill(' ') << std::fixed << std::setprecision(5);
        ss << mLat << ",";
        ss << mLon << ",";
        ss << std::setprecision(3);
        ss << mDepth;

        mStrBuf = ss.str();
        return mStrBuf.c_str();//mStrBuf;
    }

protected:
    double mTimeUsec;
    double mLat;
    double mLon;
    double mDepth;
    flag_var<nav_flags_t>mFlags;
    std::string mStrBuf;
 };

// Navigation provider interface API
// (time, lat, lon, depth)
class navigation_provider_IF
{
public:
    virtual nav_info *nav_inst() = 0;
    virtual nav_info *nav_filt()
    {
        return nullptr;
    }
};

}

#endif

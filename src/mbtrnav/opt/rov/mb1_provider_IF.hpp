
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef MB1_PROVIDER_IF_HPP  // include guard
#define MB1_PROVIDER_IF_HPP

#include "flag_utils.hpp"

namespace trn {

// beam number, range
using mb1_beam_tup = std::tuple<uint32_t, double, double, double>;

// use bath enums for input types and flags
typedef trn::bath_flag_bits_t mb1_flag_bits_t;
typedef trn::bath_input_type_t mb1_input_type_t;
typedef uint32_t mb1_flags_t;

class mb1_info
{
public:
    mb1_info()
    : mTimeUsec(0)
    , mFlags(0)
    , mTs(0.)
    , mPingNumber(0)
    , mLat(0.)
    , mLon(0.)
    , mDepth(0.)
    , mHeading(0.)
    , mNBeams(0)
    , mBeamList()
    {
    }

    mb1_info(double time_ems, uint32_t ping_number, double ts, double lat, double lon, double heading, double depth,  int32_t nbeams, const std::list<mb1_beam_tup> &beams, mb1_flags_t flags)
    : mTimeUsec(time_ems)
    , mFlags(flags)
    , mTs(ts)
    , mPingNumber(ping_number)
    , mLat(lat)
    , mLon(lon)
    , mDepth(depth)
    , mHeading(heading)
    , mNBeams(nbeams)
    , mBeamList(beams)
    {
    }

    mb1_info(const mb1_info &other)
    : mTimeUsec(other.mTimeUsec)
    , mFlags(other.mFlags)
    , mTs(other.mTs)
    , mPingNumber(other.mPingNumber)
    , mLat(other.mLat)
    , mLon(other.mLon)
    , mDepth(other.mDepth)
    , mHeading(other.mHeading)
    , mNBeams(other.mNBeams)
    , mBeamList(other.mBeamList)
    {
    }

    ~mb1_info()
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

    double ts()
    {
        return mTs;
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

    double heading()
    {
        return mHeading;
    }

    int32_t ping_number()
    {
        return mPingNumber;
    }

    int32_t nbeams()
    {
        return mNBeams;
    }

    size_t beam_count()
    {
        return mBeamList.size();
    }

    std::list<mb1_beam_tup> &beams_raw()
    {
        return mBeamList;
    }

    const char *mb1str()
    {
        mStrBuf.clear();
        std::ostringstream ss;
        ss << std::dec << std::setfill(' ') << std::fixed << std::setprecision(3);
        ss << time_usec() << ",";
        ss << ts() << ",";
        ss << "x" << std::hex << std::setfill('0') << std::setw(8);
        ss << mFlags.get() << ",";
        ss << std::dec << std::setfill(' ');
        ss << ping_number() << ",";
        ss << lat() << ",";
        ss << lon() << ",";
        ss << heading() << ",";
        ss << depth() << ",";
        ss << nbeams() << ",";
        ss << beam_count() << ",";

        std::list<trn::mb1_beam_tup>::iterator it;

        for(it=mBeamList.begin(); it!=mBeamList.end(); )
        {
            trn::mb1_beam_tup bt = static_cast<trn::mb1_beam_tup> (*it);
            ss <<  std::get<0>(bt) << ":" << std::get<1>(bt);
            it++;
            if(it != mBeamList.end())
                ss << ",";
        }

        mStrBuf = ss.str();
        return mStrBuf.c_str();
    }

protected:
    double mTimeUsec;
    flag_var<uint32_t> mFlags;
    double mTs;
    int32_t mPingNumber;
    double mLat;
    double mLon;
    double mDepth;
    double mHeading;
    int32_t mNBeams;
    std::list<mb1_beam_tup> mBeamList;
    std::string mStrBuf;
};

// Navigation provider interface API
// (time, lat, lon, depth)
class mb1_provider_IF
{
public:
    virtual int mb1_input_type() = 0;
    virtual void set_mb1_input_type(int t) = 0;
    virtual mb1_info *mb1_inst() = 0;
    virtual mb1_info *mb1_filt()
    {
        return nullptr;
    }
protected:
};

}

#endif

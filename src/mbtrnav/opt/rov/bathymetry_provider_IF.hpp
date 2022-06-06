
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef BATHYMETRY_PROVIDER_IF_HPP  // include guard
#define BATHYMETRY_PROVIDER_IF_HPP

#include "flag_utils.hpp"

namespace trn {

// beam number, range
using beam_tup = std::tuple<uint16_t, double>;

typedef enum{
    BF_VALID=0x1,
    BF_BLOCK=0x2,
    BF_RLOCK=0x4,
    BF_FRAME=0xFF0
}bath_flag_bits_t;
typedef uint32_t bath_flags_t;

typedef enum{
    BT_DVL=1,
    BT_MULTIBEAM,
    BT_PENCIL,
    BT_HOMER,
    BT_DELTAT
}bath_input_type_t;

class bath_info
{
public:
    bath_info()
    : mTimeUsec(0), mFlags(0), mPingNumber(0), mBeamList()
    {
    }

    bath_info(double time_ems, uint32_t ping_number, const std::list<beam_tup> &beams, bath_flags_t flags)
    : mTimeUsec(time_ems), mFlags(flags), mPingNumber(ping_number), mBeamList(beams)
    {
    }

    bath_info(const bath_info &other)
    : mTimeUsec(other.mTimeUsec), mFlags(other.mFlags), mPingNumber(other.mPingNumber),  mBeamList(other.mBeamList)
    {
    }

    ~bath_info()
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

    uint32_t ping_number()
    {
        return mPingNumber;
    }


    size_t beam_count()
    {
        return mBeamList.size();
    }

    std::list<beam_tup> &beams_raw()
    {
        return mBeamList;
    }

    const char *bathstr()
    {
        mStrBuf.clear();
        std::ostringstream ss;
        ss << std::dec << std::setfill(' ') << std::fixed << std::setprecision(3);
        ss << time_usec() << ",";
        ss << "x" << std::hex << std::setfill('0') << std::setw(8);
        ss << mFlags.get() << ",";
        ss << std::dec << std::setfill(' ');
        ss << ping_number() << ",";
        ss << beam_count() << ",";

        std::list<trn::beam_tup>::iterator it;
        int k=0;
        for(it=mBeamList.begin(); it!=mBeamList.end(); k++)
        {
            trn::beam_tup bt = static_cast<trn::beam_tup> (*it);
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
    uint32_t mPingNumber;
    std::list<beam_tup> mBeamList;
    std::string mStrBuf;
};

// Navigation provider interface API
// (time, lat, lon, depth)
class bathymetry_provider_IF
{
public:
    virtual int bath_input_type() = 0;
    virtual void set_bath_input_type(int t) = 0;
    virtual bath_info *bath_inst() = 0;
    virtual bath_info *bath_filt()
    {
        return nullptr;
    }
protected:
};

}

#endif

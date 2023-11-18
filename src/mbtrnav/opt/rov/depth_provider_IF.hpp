
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef DEPTH_PROVIDER_IF_HPP  // include guard
#define DEPTH_PROVIDER_IF_HPP

#include <tuple>
#include "flag_utils.hpp"

namespace trn {

typedef enum{
    DF_DEPTH_VALID=0x2,
}depth_flag_bits_t;

typedef uint32_t depth_flags_t;

class depth_info
{
public:

    depth_info()
    : mTimeUsec(0), mDepth(0), mPressure(0), mFlags(0)
    {
    }
    
    depth_info(double time_ems, double depth, double pressure, const flag_var<depth_flags_t> &flags)
    : mTimeUsec(time_ems)
    , mDepth(depth)
    , mPressure(pressure)
    , mFlags(flags)
    {
    }

    depth_info(const depth_info &other)
    : mTimeUsec(other.mTimeUsec)
    , mDepth(other.mDepth)
    , mPressure(other.mPressure)
    , mFlags(other.mFlags)
    {
    }

    ~depth_info()
    {
    }

    bool depth_valid()
    {
        return mFlags.is_set(NF_DEPTH_VALID);
    }

    double time_usec()
    {
        return mTimeUsec;
    }

    flag_var<depth_flags_t> &flags()
    {
        return mFlags;
    }

    void depth_pressure(double &r_depth_m, double &r_pressure_dbar)
    {
        r_depth_m = depth_m();
        r_pressure_dbar = mPressure;
    }

    double pressure_dbar()
    {
        return mPressure;
    }


    double pressure_to_depth_m(double lat_rad)
    {
        // Sea-Bird uses the formula in UNESCO Technical Papers in Marine Science No. 44.
        // This is an empirical formula that takes compressibility (that is, density)
        // into account. An ocean water column at 0 °C (t = 0) and 35 PSU (s = 35) is assumed.
        // The gravity variation with latitude and pressure is computed as:
        //    g (m/sec2) = 9.780318 * [ 1.0 + ( 5.2788x10-3 + 2.36x10-5 * x) * x ] + 1.092x10-6 * p
        // where
        //    x = [sin (latitude / 57.29578) ] 2 p = pressure (decibars)
        // Then, depth is calculated from pressure:
        //    depth (meters) = [(((-1.82x10 -15 * p + 2.279x10 -10 ) * p - 2.2512x10 -5 ) * p + 9.72659) * p] / g
        //    where
        //    p = pressure (decibars) g = gravity (m/sec2)
        double x = pow(sin(lat_rad), 2.);
        double g = 9.780318 * ( 1.0 + ( 5.2788e-3 + 2.36e-5 * x) * x ) + 1.092e-6 * mPressure;
        double depth = ((((-1.82e-15 * mPressure + 2.279e-10 ) * mPressure - 2.2512e-5 ) * mPressure + 9.72659) * mPressure) / g;
        return depth;

    }

//    double pressure_to_depth_m(double lat)
//    {
//        // Peter M. Saunders
//        // Print Publication: 01 Apr 1981
//        // DOI: https://doi.org/10.1175/1520-0485(1981)011<0573:PCOPTD>2.0.CO;2
//        // Page(s): 573–574
//
//        // z = (1-c1) * p − c2 * p^2
//        // p is in decibars and z in meters
//        // c1 = (5.92 + 5.25 sin^2(lat)) × 10^−3,
//        // c2 = 2.21 × 10−6
//        double c1 = (5.92 + 5.25 * sin(lat) * sin(lat)) * 1e-3;
//        double c2 = 2.21e-6;
//        double depth = (1 - c1) * mPressure - c2 * pow(mPressure, 2.);
//
//        return depth;
//    }

    double depth_m()
    {
        return mDepth;
    }

    const char *depthstr(double lat=0.)
    {
        mStrBuf.clear();
        std::ostringstream ss;
        ss << std::dec << std::setfill(' ') << std::fixed << std::setprecision(3);
        ss << time_usec() << ",";
        ss << "x" << std::hex << std::setfill('0') << std::setw(8);
        ss << mFlags.get() << ",";
        ss << std::dec << std::setfill(' ') << std::fixed << std::setprecision(3);
        ss << mDepth << ",";
        ss << pressure_to_depth_m(lat) << ",";
        ss << mPressure;

        mStrBuf = ss.str();
        return mStrBuf.c_str();
    }

protected:
    double mTimeUsec;
    double mDepth;
    double mPressure;
    flag_var<depth_flags_t>mFlags;
    std::string mStrBuf;
 };

// Depth provider interface API
// (time, pressure, depth)
class depth_provider_IF
{
public:
    virtual depth_info *depth_inst() = 0;
    virtual depth_info *depth_filt()
    {
        return nullptr;
    }

    virtual bool provides_depth() = 0;

};

}

#endif

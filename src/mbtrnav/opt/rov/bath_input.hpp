
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef BATH_INPUT_HPP  // include guard
#define BATH_INPUT_HPP

#include "flag_utils.hpp"
#include "bathymetry_provider_IF.hpp"
#include "trn_lcm_input.hpp"

namespace trn {

class bath_input : virtual public trn_lcm_input, public bathymetry_provider_IF
{
public:

    bath_input()
    :mBathInst(),mBathInputType(-1)
    {}
    ~bath_input()
    {}

    virtual bath_info *bath_inst() override
    {
        bath_info *inf = nullptr;
        mDataInstMutex.lock();
        inf = new bath_info(mBathInst);
        mDataInstMutex.unlock();
        return inf;
    }

    virtual bool provides_bath() override
    {
        return true;
    }

    virtual int bath_input_type() override
    {
        return mBathInputType;
    }

    virtual void set_bath_input_type(int t) override
    {
        mBathInputType=t;
    }

protected:
    // instantaneous (latest) bathymetry
    bath_info mBathInst;
    std::mutex mDataInstMutex;
    int mBathInputType;

private:

};

}

#endif

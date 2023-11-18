
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef MB1_INPUT_HPP  // include guard
#define MB1_INPUT_HPP

#include "flag_utils.hpp"
#include "mb1_provider_IF.hpp"
#include "trn_lcm_input.hpp"

namespace trn {

class mb1_input : virtual public trn_lcm_input, public mb1_provider_IF
{
public:

    mb1_input()
    :mMB1Inst(),mMB1InputType(-1)
    {}
    ~mb1_input()
    {}

    virtual mb1_info *mb1_inst() override
    {
        mb1_info *inf = nullptr;
        mDataInstMutex.lock();
        inf = new mb1_info(mMB1Inst);
        mDataInstMutex.unlock();
        return inf;
    }

    virtual bool provides_mb1() override
    {
        return true;
    }

    virtual int mb1_input_type() override
    {
        return mMB1InputType;
    }

    virtual void set_mb1_input_type(int t) override
    {
        mMB1InputType=t;
    }

protected:
    // instantaneous (latest) bathymetry, nav, heading
    mb1_info mMB1Inst;
    std::mutex mDataInstMutex;
    int mMB1InputType;

private:

};

}

#endif

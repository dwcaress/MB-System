
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef VELOCITY_INPUT_HPP  // include guard
#define VELOCITY_INPUT_HPP

#include "flag_utils.hpp"
#include "velocity_provider_IF.hpp"
#include "trn_lcm_input.hpp"

namespace trn {

class vel_input : virtual public trn_lcm_input, public velocity_provider_IF
{
public:

    //    vel_input(const std::string& name = "UNKNOWN", uint32_t depth=0)
    //    :trn_lcm_input(name, depth), mVelInst()
    //    {}
    vel_input()
    :mVelInst()
    {}
    ~vel_input()
    {}
    
    virtual vel_info *vel_inst() override
    {
        vel_info *inf =  nullptr;
        mDataInstMutex.lock();
        inf = new vel_info(mVelInst);
        mDataInstMutex.unlock();
        return inf;
    }

    virtual bool provides_vel() override {return true;}

protected:
    // instantaneous (latest) velocity
    vel_info mVelInst;
    std::mutex mDataInstMutex;

private:

};

}

#endif

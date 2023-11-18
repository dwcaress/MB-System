
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef DEPTH_INPUT_HPP  // include guard
#define DEPTH_INPUT_HPP

#include "flag_utils.hpp"
#include "depth_provider_IF.hpp"
#include "trn_lcm_input.hpp"

namespace trn {

class depth_input : virtual public trn_lcm_input, public depth_provider_IF
{
public:

    depth_input()
    :mDepthInst()
    {}
    ~depth_input()
    {}

    virtual depth_info *depth_inst() override
    {
        depth_info *inf = nullptr;
        mDataInstMutex.lock();
        inf = new depth_info(mDepthInst);
        mDataInstMutex.unlock();
        return inf;
    }

    virtual bool provides_depth() override
    {
        return true;
    }

protected:
    // instantaneous (latest) depth
    depth_info mDepthInst;
    std::mutex mDataInstMutex;

private:

};

}

#endif


//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef ATT_INPUT_HPP  // include guard
#define ATT_INPUT_HPP

#include "flag_utils.hpp"
#include "attitude_provider_IF.hpp"
#include "trn_lcm_input.hpp"

namespace trn {

class att_input : virtual public trn_lcm_input, public attitude_provider_IF
{
public:

    att_input()
    :mAttInst()
    {}
    ~att_input()
    {}

    virtual att_info *att_inst() override
    {
        att_info *inf = nullptr;
        mDataInstMutex.lock();
        inf = new att_info(mAttInst);
        inf->set_flags(mFlags);
        mDataInstMutex.unlock();
        return inf;
    }
    
    virtual bool provides_att() override {return true;}

    void set_flags(const flag_var<att_flags_t> &flags)
    {
        mFlags = flags;
    }

    flag_var<att_flags_t> &flags()
    {
        return mFlags;
    }

protected:
    // instantaneous (latest) attitude
    att_info mAttInst;
    std::mutex mDataInstMutex;
    flag_var<nav_flags_t>mFlags;

private:

};

}

#endif

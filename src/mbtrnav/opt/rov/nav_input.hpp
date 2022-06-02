
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef NAV_INPUT_HPP  // include guard
#define NAV_INPUT_HPP

#include "flag_utils.hpp"
#include "navigation_provider_IF.hpp"
#include "trn_lcm_input.hpp"

namespace trn {

class nav_input : virtual public trn_lcm_input, public navigation_provider_IF
{
public:

    nav_input()
    :mNavInst()
    {}
    
    ~nav_input()
    {}

    virtual nav_info *nav_inst() override
    {
        nav_info *inf =  nullptr;
        mDataInstMutex.lock();
        inf = new nav_info(mNavInst);
        mDataInstMutex.unlock();
        return inf;
    }

    virtual bool provides_nav() override {return true;}

protected:
    // instantaneous (latest) nav
    nav_info mNavInst;
    std::mutex mDataInstMutex;

private:

};

}

#endif

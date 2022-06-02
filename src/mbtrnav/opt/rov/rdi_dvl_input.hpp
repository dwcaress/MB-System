
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef RDI_DVL_INPUT_HPP  // include guard
#define RDI_DVL_INPUT_HPP

#include <tuple>
#include <queue>
#include <list>

#include "pcf_utils.hpp"
#include "lcm_subscriber.hpp"
#include "lcm_pcf/string_t.hpp"
#include "data_container.hpp"

namespace trn
{
class rdi_dvl_input : public trn_lcm_input
{
public:
    rdi_dvl_input(const std::string& name = "UNKNOWN", uint32_t depth=0)
    :trn_lcm_input(name, depth), mFoo(-1)
    {

    }

    ~rdi_dvl_input(){
        std::cerr << __func__ << ":" << __LINE__ << " rdi_dvl_input DTOR" <<"\n";

    }

    int foo(){return mFoo;};
    virtual void show(int wkey=15, int wval=28) override
    {
        trn_lcm_input::show(wkey, wval);
        std::cerr << std::setw(wkey) << "foo" << std::setw(wval) << mFoo << "\n";
    }
protected:
private:
    int mFoo;
};
}

#endif

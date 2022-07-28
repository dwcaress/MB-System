//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef RAW_SIGNAL_INPUT_HPP  // include guard
#define RAW_SIGNAL_INPUT_HPP

#include <tuple>
#include <queue>
#include <list>

#include "pcf_utils.hpp"
#include "lcm_subscriber.hpp"
#include "lcm_pcf/string_t.hpp"
#include "data_container.hpp"

namespace trn
{

class raw_signal_input : public trn_lcm_input
{
public:
    raw_signal_input(const std::string& name = "UNKNOWN", uint32_t depth=0)
    :trn_lcm_input(name, depth), mBar(-1)
    {

    }
    ~raw_signal_input(){
        std::cerr << __func__ << ":" << __LINE__ << " raw_signal_input DTOR" <<"\n";

    }
    int bar(){return mBar;};
    int set_bar(int x){
        int retval=mBar;
        mBar++;
        return retval;
    };
    virtual void show(int wkey=15, int wval=28) override
    {
        trn_lcm_input::show(wkey, wval);
        std::cerr << std::setw(wkey) << "bar" << std::setw(wval) << mBar << "\n";
    }
protected:
private:
    int mBar;
};

}

#endif

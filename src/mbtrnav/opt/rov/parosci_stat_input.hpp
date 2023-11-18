
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef PAROSCI_STAT_INPUT_HPP  // include guard
#define PAROSCI_STAT_INPUT_HPP

#include <tuple>
#include <queue>
#include <list>

#include "pcf_utils.hpp"
#include "lcm_subscriber.hpp"
#include "lcm_pcf/string_t.hpp"
#include "data_container.hpp"
#include "gss/parosci_stat_t.hpp"
#include "depth_input.hpp"

namespace trn
{
class parosci_stat_input : public depth_input
{
public:


    parosci_stat_input(const std::string& name = "UNKNOWN", uint32_t depth=0)
    :trn_lcm_input(name, depth)
    , mUnits(std::string("unspecified"))
    {
        mDelegateNotify = false;
    }

    ~parosci_stat_input()
    {
    }

    void process_msg() override
    {
        // invoke base class to buffer data
        trn::trn_lcm_input::process_msg();

        mDataListMutex.lock();

        // get the data container
        if(!mDataList.empty()){
            data_container &dcon = mDataList.front();

            gss::parosci_stat_t msg;
            msg.decode((void *)dcon.data_bytes(), 0, dcon.data_len());

            double time = msg.unix_time * 1.e6;
            dcon.set_data_time(time);

            depth_input::mDataInstMutex.lock();

            // set (Instantaneous) pressure/depth values
            double depth = 0;
            // assumes pressure in dbar
            // TODO: convert units
            double pressure = msg.pressure;
            mUnits = std::string(msg.units);

            // TODO: check doubles[] parameters? (not documented...)
            depth_flags_t flags = NF_DEPTH_VALID;

            mDepthInst = depth_info(time, depth, pressure, flags);

            depth_input::mDataInstMutex.unlock();

#ifdef WITH_SHOW_DCON
            std::cerr << __func__ << ":" << std::dec << __LINE__ << " Updated DATA_TIME\n";
            dcon.show(false);
#endif
        }
        mDataListMutex.unlock();

        // Note: mDelegateNotify is initialized by the CTOR.
        // mDelegateNotify should be appropriately set and
        // observed by sub-classes to implement the intended
        // behavior, i.e. defer notification until processing
        // is complete
        if(!mDelegateNotify){
//            std::cerr << __func__ << ":" << std::dec << __LINE__ << " NOTIFY SEM\n";
            notify_sem_list();
        }
    }

    virtual void tostream(std::ostream &os, int wkey=15, int wval=28) override
    {
        trn_lcm_input::tostream(os, wkey, wval);
        os << std::setw(wkey) << "depth" << std::setw(wval) << mDepthInst.depth_m() << " (lat=0)\n";
        os << std::setw(wkey) << "pressure" << std::setw(wval) << mDepthInst.pressure_dbar() << "\n";
        os << std::setw(wkey) << "flags" << std::setw(wval-8) << "x";
        os << std::setw(8) << std::hex << std::setfill('0') << mDepthInst.flags().get() << "\n";
        os << std::setfill(' ');
        os << std::setw(wkey) << "pressure units" << std::setw(wval) << mUnits << "\n";
    }

    virtual void show(int wkey=15, int wval=28) override
    {
        tostream(std::cerr, wkey, wval);
    }

protected:

private:
    std::string mUnits;
};
}

#endif

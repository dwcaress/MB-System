
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef NAV_SOLUTION_INPUT_HPP  // include guard
#define NAV_SOLUTION_INPUT_HPP

#include <tuple>
#include <queue>
#include <list>

#include "pcf_utils.hpp"
#include "lcm_subscriber.hpp"
#include "lcm_pcf/string_t.hpp"
#include "data_container.hpp"
#include "gss/nav_solution_t.hpp"
#include "nav_input.hpp"
#include "att_input.hpp"
#include "trn_debug.hpp"

namespace trn
{
class nav_solution_input : public nav_input, public att_input
{
public:

    nav_solution_input(const std::string& name = "UNKNOWN", uint32_t depth=0)
    :trn_lcm_input(name, depth)
    {
        mDelegateNotify = true;
    }

    ~nav_solution_input()
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

            gss::nav_solution_t ns;
            ns.decode((void *)dcon.data_bytes(), 0, dcon.data_len());

            double time = ns.unix_time * 1000000UL;
            dcon.set_data_time(time);

            nav_input::mDataInstMutex.lock();
            // set (Instantaneous) nav values
            double lat = ns.absolute_position[1];
            double lon = ns.absolute_position[0];
            double depth = ns.depth;
            nav_flags_t nflags = 0;
            nflags |= (ns.relative_position_ok ? NF_POS_VALID : 0);
            nflags |= (ns.depth_ok ? NF_DEPTH_VALID : 0);
            mNavInst = nav_info(time, lat, lon, depth, nflags);

            // att 0:phi 1:theta 2:psi
            double roll = ns.attitude[0] * M_PI/180.;
            double pitch = ns.attitude[1] * M_PI/180.;
            double heading = ns.attitude[2] * M_PI/180.;

            att_flags_t aflags = 0;
            aflags |= (ns.attitude_ok ? AF_VALID : 0);
            mAttInst = att_info(time, pitch, roll, heading, aflags);
            nav_input::mDataInstMutex.unlock();

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
        if(mDelegateNotify){
            TRN_NDPRINT(5, "NAV_SOL::%s:%d  NOTIFY SEM\n", __func__, __LINE__);

            notify_sem_list();
        }
    }

    virtual void tostream(std::ostream &os, int wkey=15, int wval=28) override
    {
        trn_lcm_input::tostream(os, wkey, wval);
        os << std::setw(wkey) << "lat" << std::setw(wval) << mNavInst.lat() << "\n";
        os << std::setw(wkey) << "lon" << std::setw(wval) << mNavInst.lon() << "\n";
        os << std::setw(wkey) << "depth" << std::setw(wval) << mNavInst.depth() << "\n";
        os << std::setw(wkey) << "nflags" << std::setw(wval-8) << "x";
        os << std::setw(8) << std::hex << std::setfill('0') << mNavInst.flags().get() << "\n";
        os << std::setfill(' ');
        os << std::setw(wkey) << "pitch" << std::setw(wval) << mAttInst.pitch() << "\n";
        os << std::setw(wkey) << "roll" << std::setw(wval) << mAttInst.roll() << "\n";
        os << std::setw(wkey) << "heading" << std::setw(wval) << mAttInst.heading() << "\n";
        os << std::setw(wkey) << "aflags" << std::setw(wval-8) << "x";
        os << std::setw(8) << std::hex << std::setfill('0') << mAttInst.flags().get() << "\n";
    }

    virtual void show(int wkey=15, int wval=28) override
    {
        tostream(std::cerr, wkey, wval);
    }

protected:

private:
};
}

#endif

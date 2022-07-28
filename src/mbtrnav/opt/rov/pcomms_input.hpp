
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef PCOMMS_INPUT_HPP  // include guard
#define PCOMMS_INPUT_HPP

#include <tuple>
#include <queue>
#include <list>

#include "pcf_utils.hpp"
#include "lcm_subscriber.hpp"
#include "lcm_pcf/string_t.hpp"
#include "data_container.hpp"
#include "gss/pcomms_t.hpp"
#include "nav_input.hpp"
#include "att_input.hpp"

namespace trn
{
class pcomms_input : public nav_input, public att_input
{
public:

    pcomms_input(const std::string& name = "UNKNOWN", uint32_t depth=0)
    :trn_lcm_input(name, depth)
    {
        mDelegateNotify = false;
    }

    ~pcomms_input()
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

            gss::pcomms_t msg;
            msg.decode((void *)dcon.data_bytes(), 0, dcon.data_len());

            double time = msg.time_unix_sec * 1000000UL;
            dcon.set_data_time(time);

            nav_input::mDataInstMutex.lock();
            int r_err=0;
            // set (Instantaneous) nav values
            double lat = msg_tool<gss::pcomms_t>::get_analog(msg, "latitude", r_err);
            double lon = msg_tool<gss::pcomms_t>::get_analog(msg, "longitude", r_err);
            double depth = msg_tool<gss::pcomms_t>::get_analog(msg, "depth", r_err) ;
            double pitch = msg_tool<gss::pcomms_t>::get_analog(msg, "pitch", r_err) * M_PI/180.;
            double roll = msg_tool<gss::pcomms_t>::get_analog(msg, "roll", r_err) * M_PI/180.;
            double heading = msg_tool<gss::pcomms_t>::get_analog(msg, "heading", r_err) * M_PI/180.;

            nav_flags_t nflags = 0;
            bool pos_valid = msg_tool<gss::pcomms_t>::get_digital(msg, "pos_status", r_err);
//            if(r_err!=0)
//                std::cerr << "WARN - pos_status not found [" << r_err << "]/n";
//            else
//                std::cerr << "WARN - pos_status found [" << pos_valid << "," << r_err << "]/n";

            nflags |= (pos_valid ? NF_POS_VALID : 0);
            nflags |= NF_DEPTH_VALID;

            mNavInst = nav_info(time, lat, lon, depth, nflags);

            att_flags_t aflags = 0;
            bool att_valid = msg_tool<gss::pcomms_t>::get_digital(msg, "orientation_status", r_err);
            aflags |= (att_valid ? AF_VALID : 0);
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
        if(!mDelegateNotify){
//            std::cerr << __func__ << ":" << std::dec << __LINE__ << " NOTIFY SEM\n";
            notify_sem_list();
        }
    }

    virtual void show(int wkey=15, int wval=28) override
    {
        trn_lcm_input::show(wkey, wval);
        std::cerr << std::setw(wkey) << "lat" << std::setw(wval) << mNavInst.lat() << "\n";
        std::cerr << std::setw(wkey) << "lon" << std::setw(wval) << mNavInst.lon() << "\n";
        std::cerr << std::setw(wkey) << "depth" << std::setw(wval) << mNavInst.depth() << "\n";
        std::cerr << std::setw(wkey) << "nflags" << std::setw(wval-8) << "x";
        std::cerr << std::setw(8) << std::hex << std::setfill('0') << mNavInst.flags().get() << "\n";
        std::cerr << std::setfill(' ');
        std::cerr << std::setw(wkey) << "pitch" << std::setw(wval) << mAttInst.pitch() << "\n";
        std::cerr << std::setw(wkey) << "roll" << std::setw(wval) << mAttInst.roll() << "\n";
        std::cerr << std::setw(wkey) << "heading" << std::setw(wval) << mAttInst.heading() << "\n";
        std::cerr << std::setw(wkey) << "aflags" << std::setw(wval-8) << "x";
        std::cerr << std::setw(8) << std::hex << std::setfill('0') << mAttInst.flags().get() << "\n";
    }

protected:

private:
};
}

#endif

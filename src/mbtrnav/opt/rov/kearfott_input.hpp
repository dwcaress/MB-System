
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef KEARFOTT_INPUT_HPP  // include guard
#define KEARFOTT_INPUT_HPP

#include <tuple>
#include <queue>
#include <list>

#include "pcf_utils.hpp"
#include "lcm_subscriber.hpp"
#include "lcm_pcf/string_t.hpp"
#include "data_container.hpp"
#include "oi/kearfott_t.hpp"
#include "nav_input.hpp"
#include "att_input.hpp"

namespace trn
{
class kearfott_input : public nav_input, public att_input
{
public:

    typedef enum
    {
        DLOOP_OPEN=0x80,
        GPS_PROC=0x40,
        GPS_REJ=0x20,
        DOP_PROC=0x08,
        DOP_REJ=0x04,
        ZUPT_PROC=0x02,
        DVLH_VALID=0x01
    }monitor_flag_t;

    kearfott_input(const std::string& name = "UNKNOWN", uint32_t depth=0)
    :trn_lcm_input(name, depth)
    {
        mDelegateNotify = false;
    }

    ~kearfott_input()
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

            oi::kearfott_t msg;
            msg.decode((void *)dcon.data_bytes(), 0, dcon.data_len());

            double time = msg.time_unix_sec * 1000000UL;
            dcon.set_data_time(time);

            nav_input::mDataInstMutex.lock();

            // set (Instantaneous) nav values
            double lat = msg.latitude_rad * 180./M_PI;
            double lon = msg.longitude_rad * 180./M_PI;
            double depth = msg.depth_m;
            double pitch = msg.pitch_rad;
            double roll = msg.roll_rad;
            double heading = msg.heading_rad;

            // TODO: check doubles[] parameters? (not documented...)
            nav_flags_t nflags = 0;
            bool pos_valid = ((msg.monitor&GPS_REJ)==0);

            nflags |= ( pos_valid ? NF_POS_VALID : 0);
            nflags |= ( (msg.monitor&DVLH_VALID)==1 ? NF_DEPTH_VALID : 0);

            mNavInst = nav_info(time, lat, lon, depth, nflags);

            att_flags_t aflags = 0;
            bool att_valid = true;
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

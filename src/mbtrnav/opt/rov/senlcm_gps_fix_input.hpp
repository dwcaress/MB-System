
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef SENLCM_GPS_FIX_INPUT_HPP  // include guard
#define SENLCM_GPS_FIX_INPUT_HPP

#include <tuple>
#include <queue>
#include <list>

#include "pcf_utils.hpp"
#include "lcm_subscriber.hpp"
#include "lcm_pcf/string_t.hpp"
#include "data_container.hpp"
#include "senlcm/gps_fix_t.hpp"
#include "nav_input.hpp"
#include "att_input.hpp"
#include "vel_input.hpp"

namespace trn
{
class senlcm_gps_fix_input : public nav_input
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

    senlcm_gps_fix_input(const std::string& name = "UNKNOWN", uint32_t depth=0)
    :trn_lcm_input(name, depth)
    {
        mDelegateNotify = false;
    }

    ~senlcm_gps_fix_input()
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

            senlcm::gps_fix_t msg;
            msg.decode((void *)dcon.data_bytes(), 0, dcon.data_len());

            double time = msg.header.timestamp;
            dcon.set_data_time(time);

            nav_input::mDataInstMutex.lock();

            // set (Instantaneous) nav values (decimal degrees)
            double lat = msg.latitude;
            double lon = msg.longitud;
            double depth = msg.altitude;

            // TODO: check doubles[] parameters? (not documented...)
            nav_flags_t nflags = 0;
            bool pos_valid = true;

            nflags |= ( pos_valid ? NF_POS_VALID : 0);
            nflags |= NF_DEPTH_VALID;

            mNavInst = nav_info(time, lat, lon, depth, nflags);

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

    virtual void tostream(std::ostream &os, int wkey=15, int wval=28) override
    {
        trn_lcm_input::tostream(os, wkey, wval);
        os << std::setw(wkey) << "lat" << std::setw(wval) << mNavInst.lat() << "\n";
        os << std::setw(wkey) << "lon" << std::setw(wval) << mNavInst.lon() << "\n";
        os << std::setw(wkey) << "depth" << std::setw(wval) << mNavInst.depth() << "\n";
        os << std::setw(wkey) << "nflags" << std::setw(wval-8) << "x";
        os << std::setw(8) << std::hex << std::setfill('0') << mNavInst.flags().get() << "\n";
        os << std::setfill(' ');
//        os << std::setw(wkey) << "pitch" << std::setw(wval) << mAttInst.pitch() << "\n";
//        os << std::setw(wkey) << "roll" << std::setw(wval) << mAttInst.roll() << "\n";
//        os << std::setw(wkey) << "heading" << std::setw(wval) << mAttInst.heading() << "\n";
//        os << std::setw(wkey) << "aflags" << std::setw(wval-8) << "x";
//        os << std::setw(8) << std::hex << std::setfill('0') << mAttInst.flags().get() << "\n";
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

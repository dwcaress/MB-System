
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef X_MB1_INPUT_HPP  // include guard
#define X_MB1_INPUT_HPP
#include <iostream>

#include "pcf_utils.hpp"
#include "lcm_subscriber.hpp"
#include "lcm_pcf/string_t.hpp"
#include "data_container.hpp"
#include "mb1_input.hpp"
#include "flag_utils.hpp"
#include "trn_debug.hpp"

namespace trn
{

class x_mb1_input : public mb1_input
{
public:
    x_mb1_input(const std::string& name = "UNKNOWN", uint32_t depth=0)
    :trn_lcm_input(name, depth)
    {
        mDelegateNotify = true;
    }

    ~x_mb1_input()
    {
    }

    void process_msg() override
    {
        // invoke base class to buffer data
        trn::trn_lcm_input::process_msg();

        mb1_input::mDataListMutex.lock();

        // get the data container
        if(!mDataList.empty()){
            data_container &dcon = mDataList.front();

            trn_mb1_t msg;
            msg.decode((void *)dcon.data_bytes(), 0, dcon.data_len());
            double time = msg.ts * 1000000UL;
            dcon.set_data_time(time);

            mb1_input::mDataInstMutex.lock();

            // set (Instantaneous) bathymetry values
            mb1_flags_t bflags = 0;
            bflags |= BF_BLOCK;
            bflags |= BF_RLOCK;
            bflags |= BF_VALID;

            // set (Instantaneous) nav values
            double ts = msg.ts;
            double lat = msg.lat;
            double lon = msg.lon;
            double heading = msg.hdg;
            double depth = msg.depth;
            int32_t ping_number = msg.ping_number;
            int32_t nbeams = msg.nbeams;

            std::list<mb1_beam_tup> beams;
            int i=0;
            for(i=0; i < msg.nbeams; i++){
                uint32_t beam_n = msg.beams[i].beam_num;
                double  rhox = msg.beams[i].rhox;
                double rhoy = msg.beams[i].rhoy;
                double rhoz = msg.beams[i].rhoz;
                beams.emplace_back(beam_n, rhox, rhoy, rhoz);
            }
            mMB1Inst = mb1_info(time, ping_number, ts, lat, lon, heading, depth, nbeams, beams, bflags);


            mb1_input::mDataInstMutex.unlock();

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
            TRN_NDPRINT(6, "TRN_MB1::%s:%d  NOTIFY SEM\n", __func__, __LINE__);
            notify_sem_list();
        }
    }

    virtual void tostream(std::ostream &os, int wkey=15, int wval=28) override
    {
        trn_lcm_input::tostream(os, wkey, wval);
        os << std::setw(wkey) << std::setfill(' ') << "TimeUsec";
        os << std::setw(wval) << mMB1Inst.time_usec() << "\n";
        os << std::setw(wkey) << std::setfill(' ') << "ts";
        os << std::setw(wval) << mMB1Inst.ts() << "\n";
        os << std::setw(wkey) << "bflags" << std::setw(wval-8) << "x";
        os << std::setw(8) << std::hex << std::setfill('0') << mMB1Inst.flags().get() << "\n";

        os << std::setfill(' ') << std::dec;
        os << std::setw(wkey) << std::setfill(' ') << "ping_number";
        os << std::setw(wval) << mMB1Inst.ping_number() << "\n";
        os << std::setw(wkey) << "lat" << std::setw(wval) << mMB1Inst.lat() << "\n";
        os << std::setw(wkey) << "lon" << std::setw(wval) << mMB1Inst.lon() << "\n";
        os << std::setw(wkey) << "depth" << std::setw(wval) << mMB1Inst.depth() << "\n";
        os << std::setw(wkey) << "heading" << std::setw(wval) << mMB1Inst.heading() << "\n";
        os << std::setw(wkey) << "bflags" << std::setw(wval-8) << "x";
        os << std::setw(8) << std::hex << std::setfill('0') << mMB1Inst.flags().get() << "\n";
        os << std::setw(wkey) << std::setfill(' ') << "nbeams";
        os << std::setw(wval) << mMB1Inst.nbeams() << "\n";
        os << std::setw(wkey) << std::setfill(' ') << "beam_count";
        std::list<mb1_beam_tup> blist = mMB1Inst.beams_raw();
        os << std::setw(wval) << blist.size() << "\n";
        os << std::setw(wkey) << "beams\n";
        std::list<mb1_beam_tup>::iterator it;
        for(it=blist.begin(); it != blist.end() ; it++){
            os << std::setw(wkey) << "[" << std::get<0>(*it) << "," << std::get<1>(*it) << "," << std::get<2>(*it) << "," << std::get<3>(*it) << "]\n";
        }
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

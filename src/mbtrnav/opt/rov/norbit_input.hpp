
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef NORBIT_INPUT_HPP  // include guard
#define NORBIT_INPUT_HPP
#include <iostream>

#include "pcf_utils.hpp"
#include "lcm_subscriber.hpp"
#include "lcm_pcf/string_t.hpp"
#include "data_container.hpp"
#include "bath_input.hpp"
#include "geolcm/point_t.hpp"
#include "stdlcm/header_t.hpp"
#include "senlcm/multibeam_euclidean_t.hpp"
#include "flag_utils.hpp"
#include "trn_debug.hpp"

namespace trn
{

class norbit_input : public bath_input
{
public:
    norbit_input(const std::string& name = "UNKNOWN", uint32_t depth=0)
    :trn_lcm_input(name, depth)
    {
        mDelegateNotify = true;
    }

    ~norbit_input()
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

            senlcm::multibeam_euclidean_t msg;
            msg.decode((void *)dcon.data_bytes(), 0, dcon.data_len());
            // msg.header.timestamp uses epoch microseconds
            double time = msg.header.timestamp;
            dcon.set_data_time(time);

            bath_input::mDataInstMutex.lock();
            // set (Instantaneous) bathymetry values
            bath_flags_t bflags = 0;
            bflags |= BF_BLOCK;
            bflags |= BF_RLOCK;
            // TODO: translate per-beam flags to bflags
            bflags |= BF_VALID; //(msg.valid > 0 ? BF_VALID : 0);

            // this is not the actual sonar ping number
            uint32_t ping_number = msg.header.sequence;

//            std::cerr << std::hex;
//            std::cerr << "  bflags : " << bflags << "\n";
//            std::cerr << std::dec;
//            std::cerr << "  norbit.lock_btm : " << (norbit.lock_btm?1:0) << "\n";
//            std::cerr << "  norbit.lock_ref : " << (norbit.lock_ref?1:0) << "\n";

            std::list<beam_tup> beams;
            int i = 0;
            int ll = 0;
            int ul = msg.n_beams;
            int inc = 1;

#if 0
            // get 120 center beams
            ll = (msg.n_beams >= 120 ? msg.n_beams/2-60 : 0);
            ul = (msg.n_beams >= 120 ? msg.n_beams/2+60 : msg.n_beams);
#endif

#if 0
            // decimate evenly to 120 beams
            ll = 0;
            ul = msg.n_beams;
            inc = msg.n_beams > 120 ? msg.n_beams/120 : 1;

#endif
            for(i = ll; i < ul; i+=inc){
                // TODO: use message intensity and flags
                double v[3] = {msg.range[i].x, msg.range[i].y, msg.range[i].z};
                double range = trnx_utils::vnorm(v);
                beams.emplace_back(i, range);
//                std::cerr << " norbit range[" << i << "]: " << range << std::endl;
            }

            mBathInst = bath_info(time, ping_number, beams, bflags);

            bath_input::mDataInstMutex.unlock();

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
            TRN_NDPRINT(6, "NORBIT::%s:%d  NOTIFY SEM\n", __func__, __LINE__);
            notify_sem_list();
        }
    }

    virtual void tostream(std::ostream &os, int wkey=15, int wval=28) override
    {
        trn_lcm_input::tostream(os, wkey, wval);
        os << std::setw(wkey) << std::setfill(' ') << "TimeUsec";

        os << std::setw(wval) << mBathInst.time_usec() << "\n";
        os << std::setw(wkey) << "bflags" << std::setw(wval-8) << "x";
        os << std::setw(8) << std::hex << std::setfill('0') << mBathInst.flags().get() << "\n";

        os << std::setfill(' ') << std::dec;
        os << std::setw(wkey) << std::setfill(' ') << "ping_number";
        os << std::setw(wval) << mBathInst.ping_number() << "\n";
        std::list<beam_tup> blist = mBathInst.beams_raw();
        os << std::setw(wkey) << std::setfill(' ') << "beam_count";
        os << std::setw(wval) << blist.size() << "\n";
        os << std::setw(wkey) << "beams\n";
        std::list<beam_tup>::iterator it;
        for(it=blist.begin(); it != blist.end() ; it++){
            os << std::setw(wkey) << "[" << std::get<0>(*it) << "," << std::get<1>(*it) << "]\n";
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

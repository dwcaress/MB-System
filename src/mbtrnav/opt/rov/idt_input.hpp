
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef IDT_INPUT_HPP  // include guard
#define IDT_INPUT_HPP
#include <iostream>

#include "pcf_utils.hpp"
#include "lcm_subscriber.hpp"
#include "lcm_pcf/string_t.hpp"
#include "data_container.hpp"
#include "bath_input.hpp"
#include "oi/idt_t.hpp"
#include "oi/lcm_header_t.hpp"
#include "flag_utils.hpp"
#include "trn_debug.hpp"

namespace trn
{

class idt_input : public bath_input
{
public:
    idt_input(const std::string& name = "UNKNOWN", uint32_t depth=0)
    :trn_lcm_input(name, depth)
    {
        mDelegateNotify = true;
    }

    ~idt_input()
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

            oi::idt_t idt;
            idt.decode((void *)dcon.data_bytes(), 0, dcon.data_len());
            double time = idt.ping_time * 1000000UL;
            dcon.set_data_time(time);

            bath_input::mDataInstMutex.lock();
            // set (Instantaneous) bathymetry values
            bath_flags_t bflags = 0;
            bflags |= BF_BLOCK;
            bflags |= BF_RLOCK;
            bflags |= (idt.valid>0 ? BF_VALID : 0);

            uint32_t ping_number = idt.ping_number;
//            std::cerr << std::hex;
//            std::cerr << "  bflags : " << bflags << "\n";
//            std::cerr << std::dec;
//            std::cerr << "  idt.lock_btm : " << (idt.lock_btm?1:0) << "\n";
//            std::cerr << "  idt.lock_ref : " << (idt.lock_ref?1:0) << "\n";

            std::list<beam_tup> beams;
            int i=0;
            for(i=0; i<idt.nbeams; i++){
                beams.emplace_back(i,idt.range[i]);
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
            TRN_NDPRINT(5, "IDT::%s:%d  NOTIFY SEM\n", __func__, __LINE__);
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

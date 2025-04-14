
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef RPH_ANGRATE_INPUT_HPP  // include guard
#define RPH_ANGRATE_INPUT_HPP

#include <tuple>
#include <queue>
#include <list>

#include "pcf_utils.hpp"
#include "lcm_subscriber.hpp"
#include "lcm_pcf/string_t.hpp"
#include "data_container.hpp"
#include "senlcm/rph_angrate_t.hpp"
#include "att_input.hpp"

namespace trn
{
class rph_angrate_input : public att_input
{
public:

    rph_angrate_input(const std::string& name = "UNKNOWN", uint32_t depth=0)
    :trn_lcm_input(name, depth)
    {
        mDelegateNotify = false;
    }

    ~rph_angrate_input()
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

            senlcm::rph_angrate_t msg;
            msg.decode((void *)dcon.data_bytes(), 0, dcon.data_len());

            // TODO: this time may not be epoch sec
            // header.timestamp is in microseconds (no multiply needed)
            double time = msg.header.timestamp;
            dcon.set_data_time(time);

            att_input::mDataInstMutex.lock();

            // set (Instantaneous) attitude values
            // angles in radians, -PI::PI
            // roll: +STBD pitch:+UP heading:+STBD/WEST
            // rph_angrate: 0:roll 1:pitch 2:heading
            // heading: 0: North PI/2: West -PI/2: East +/-PI: South
            double roll = msg.rph[0];
            double pitch = msg.rph[1];
            double heading = msg.rph[2];
            if(heading < 0 && heading >= -M_PI) {
                heading += 2. * M_PI;
            }


            // TODO: check status byte (undocumented...)
            att_flags_t aflags = 0;
            bool att_valid = true;
            aflags |= (att_valid ? AF_VALID : 0);
            mAttInst = att_info(time, pitch, roll, heading, aflags);
            att_input::mDataInstMutex.unlock();

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

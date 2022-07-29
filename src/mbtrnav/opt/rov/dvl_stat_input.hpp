
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef DVL_STAT_INPUT_HPP  // include guard
#define DVL_STAT_INPUT_HPP

#include "pcf_utils.hpp"
#include "lcm_subscriber.hpp"
#include "lcm_pcf/string_t.hpp"
#include "data_container.hpp"
#include "bath_input.hpp"
#include "vel_input.hpp"
#include "gss/dvl_stat_t.hpp"
#include "flag_utils.hpp"
#include "trn_debug.hpp"

namespace trn
{

class dvl_stat_input : public  bath_input, public vel_input //velocity_provider_IF
{
public:
    dvl_stat_input(const std::string& name = "UNKNOWN", uint32_t depth=0)
    :trn_lcm_input(name, depth),  mPingNumber(0)
    {
        mDelegateNotify = true;
    }

    ~dvl_stat_input()
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

            gss::dvl_stat_t dvl;
            dvl.decode((void *)dcon.data_bytes(), 0, dcon.data_len());
            double time = dvl.time_unix_sec * 1000000UL;
            dcon.set_data_time(time);

            bath_input::mDataInstMutex.lock();
           // set (Instantaneous) bathymetry values
            bath_flags_t bflags = 0;
            bflags |= (dvl.lock_btm ? BF_BLOCK : 0);
            bflags |= (dvl.lock_ref ? BF_RLOCK : 0);
//            bflags |= (dvl.lock_btm && dvl.lock_ref ? BF_VALID : 0);
            bflags |= (dvl.lock_btm ? BF_VALID : 0);

//            std::cerr << std::hex;
//            std::cerr << "  bflags : " << bflags << "\n";
//            std::cerr << std::dec;
//            std::cerr << "  dvl.lock_btm : " << (dvl.lock_btm?1:0) << "\n";
//            std::cerr << "  dvl.lock_ref : " << (dvl.lock_ref?1:0) << "\n";

            std::list<beam_tup> beams;
            beams.emplace_back(1,dvl.altitude_btm_beam1_m);
            beams.emplace_back(2,dvl.altitude_btm_beam2_m);
            beams.emplace_back(3,dvl.altitude_btm_beam3_m);
            beams.emplace_back(4,dvl.altitude_btm_beam4_m);
            mBathInst = bath_info(time, mPingNumber++, beams, bflags);

            double vx = dvl.velocity_btm_x_m_sec;
            double vy = dvl.velocity_btm_y_m_sec;
            double vz = dvl.velocity_btm_z_m_sec;
            vel_flags_t vflags = 0;

            vflags |= (dvl.lock_btm ? VF_BLOCK : 0);
            vflags |= (dvl.lock_ref ? VF_RLOCK : 0);
//            vflags |= (dvl.lock_btm && dvl.lock_ref ? VF_VALID : 0);
            vflags |= (dvl.lock_btm ? VF_VALID : 0);
            mVelInst = vel_info(time, vx, vy, vz, vflags);

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
            TRN_NDPRINT(5, "DVL_STAT::%s:%d  NOTIFY SEM\n", __func__, __LINE__);
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
        std::list<beam_tup> blist = mBathInst.beams_raw();
        os << std::setw(wkey) << std::setfill(' ') << "ping_number";
        os << std::setw(wval) << mBathInst.ping_number() << "\n";
        os << std::setw(wkey) << std::setfill(' ') << "beam_count";
        os << std::setw(wval) << blist.size() << "\n";
        os << std::setw(wkey) << "beams\n";
        std::list<beam_tup>::iterator it;
        for(it=blist.begin(); it != blist.end() ; it++){
            os << std::setw(wkey) << "[" << std::get<0>(*it) << "," << std::get<1>(*it) << "]\n";
        }

        os << std::setw(wkey) << "vflags" << std::setw(wval-8) << "x";
        os << std::setw(8) << std::hex << std::setfill('0') << mVelInst.flags().get() << "\n";

        os << std::dec << std::setfill(' ') ;
        os << std::fixed << std::setprecision(3);
        os << std::setw(wkey) << "vx";
        os << std::setw(wval) << mVelInst.vx_ms() << "\n";
        os << std::setw(wkey) << "vy";
        os << std::setw(wval) << mVelInst.vy_ms() << "\n";
        os << std::setw(wkey) << "vz";
        os << std::setw(wval) << mVelInst.vz_ms() << "\n";
    }

    virtual void show(int wkey=15, int wval=28) override
    {
        tostream(std::cerr, wkey, wval);
    }

protected:
private:
    uint32_t mPingNumber;
};
}

#endif

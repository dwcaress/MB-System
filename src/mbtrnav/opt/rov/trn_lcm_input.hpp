//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef TRN_LCM_INPUT_HPP  // include guard
#define TRN_LCM_INPUT_HPP

#include <tuple>
#include <queue>
#include <list>
#include <iostream>

#include "pcf_utils.hpp"
#include "lcm_subscriber.hpp"
#include "lcm_pcf/string_t.hpp"
#include "data_container.hpp"
#include "gss/analog_t.hpp"
#include "gss/digital_t.hpp"
#include "trn_debug.hpp"

namespace trn
{
using sem_item = std::tuple<const std::string, pcf::semaphore*>;

// msg_tool enables access to analog_t and digital_t in gss LCM messages
template<typename T>
class msg_tool
{
public:
    static double get_analog(T msg, const char *key, int &r_err);
    static bool get_digital(T msg, const char *key, int &r_err);
};

template <typename T>
double msg_tool<T>::get_analog(T msg, const char *key, int &r_err)
{
    std::vector<gss::analog_t>::iterator it;
    for(it = msg.analogs.begin(); it!=msg.analogs.end(); it++)
    {
        gss::analog_t at = static_cast<gss::analog_t> (*it);
        if(at.name.compare(key)==0){
            r_err = 0;
            return at.value;
        }
    }
    r_err = -1;
    return 0.;
}

template <typename T>
bool msg_tool<T>::get_digital(T msg, const char *key, int &r_err)
{
    std::vector<gss::digital_t>::iterator it;
    for(it = msg.digitals.begin(); it!=msg.digitals.end(); it++)
    {
        gss::digital_t dt = static_cast<gss::digital_t> (*it);
        if(dt.name.compare(key)==0){
            r_err = 0;
            return dt.value;
        }
    }
    r_err = -1;
    return false;
}

// trn_lcm_input is the base class for TRN input streams
// trn_lcm_input automatically buffers input messages and records
// the data origin timestamp. It provides methods for notifying
// (via semaphore) processes when a message arrives. Notfication
// may be (and typically is) delegated to a subclass.
class trn_lcm_input : public pcf::lcm_subscriber
{
public:

    trn_lcm_input(const std::string& name = "UNKNOWN", uint32_t depth=0) :
    pcf::lcm_subscriber(name), mDelegateNotify(false), mListDepth(depth)
    {
    }

    ~trn_lcm_input()
    {
        clear_data_list();

        while(mSemList.size() > 0){
            sem_item stup = mSemList.front();
            std::string name = std::get<0>(stup);
            pcf::semaphore *sem = std::get<1>(stup);
            delete sem;
            mSemList.pop_front();
        }
    }

    int list_depth() { return mListDepth; }
    int list_count() { return mDataList.size(); }
    bool is_empty() { return (mDataList.size()==0); }

    // trim data list to be <= mDataListLen; delete oldest first
    int ntrim_data_list(uint32_t size)
    {
        std::lock_guard<std::mutex> lock(mDataListMutex);
        while(!mDataList.empty() && (mDataList.size() > size)){
            mDataList.pop_back();
        }
        return mDataList.size();
    }

    int trim_data_list() {return ntrim_data_list(mListDepth);}
    int clear_data_list() {return ntrim_data_list(0);}

    int add_sem(const std::string &channel, int count=0)
    {
        int16_t retval = -1;
        pcf::semaphore *sem = new pcf::semaphore(count);
        if(sem != nullptr){
            mSemList.emplace_front(std::string(channel), sem);
            retval = 0;
        }
        return retval;
    }

    int remove_sem(const std::string &channel)
    {
        int retval = -1;
        if(!mSemList.empty()){
            std::list<sem_item>::iterator it;

            for(it=mSemList.begin(); it!=mSemList.end() ; it++) {
                const std::string key = std::get<0>(*it);
                pcf::semaphore *x = std::get<1>(*it);
                if(key.compare(channel)==0){
                    mSemList.erase(it);
                    delete x;
                    std::cerr << "deleted sem channel[" << channel.c_str() << "]\n";
                    retval = 0;
                }
            }
        }
        return retval;
    }

    pcf::semaphore *get_sem(const std::string &channel)
    {
        if(!mSemList.empty()){
            std::list<sem_item>::iterator it;

            for(it=mSemList.begin(); it!=mSemList.end() ; it++) {
                const std::string key = std::get<0>(*it);
                if(key.compare(channel)==0){
                    pcf::semaphore *sem = std::get<1>(*it);
                    return sem;
                }
            }
        }
        return nullptr;
    }

    bool test_sem(const std::string &channel, int to_msec)
    {
        if(!mSemList.empty()){
            std::list<sem_item>::iterator it;

            for(it=mSemList.begin(); it!=mSemList.end() ; it++) {
                const std::string key = std::get<0>(*it);
                if(key.compare(channel)==0){
                    pcf::semaphore *sem = std::get<1>(*it);
                    return sem->wait(to_msec);
                }
            }
        }
        return false;
    }

    void clear_sem_list()
    {
        if(!mSemList.empty()){
            std::list<sem_item>::iterator it;

            for(it=mSemList.begin(); it!=mSemList.end() ; it++) {
                pcf::semaphore *sem = std::get<1>(*it);
                delete sem;
            }
        }
    }

    void notify_sem_list()
    {
        if(!mSemList.empty()){
            std::list<sem_item>::iterator it;

            for(it=mSemList.begin(); it!=mSemList.end() ; it++) {
                pcf::semaphore *sem = std::get<1>(*it);
                sem->post();
            }
        }
    }

    data_container &get(const uint32_t elem)
    {
        if(!mDataList.empty()){
            std::list<data_container>::iterator it;

            uint32_t n=0;
            for(it=mDataList.begin(); it!=mDataList.end() ; it++,n++) {
                if(n==elem){
                    std::cerr << "n:" << n << " e:" << elem << "\n";
                    return *it;
                }
            }
        }
        std::cerr << __func__ << ":" << std::dec << __LINE__ << "\n";

        return trn_lcm_input::TRN_ENODCON;
    }

    virtual void tostream(std::ostream &os, int wkey=15, int wval=28)
    {
        os << std::dec << std::setfill(' ');
        os << std::setw(wkey) << "channel" << std::setw(wval) << get_channel_name() << "\n";
        os << std::setw(wkey) << "list depth" << std::setw(wval) << list_depth() << "\n";
        os << std::setw(wkey) << "list count" << std::setw(wval) << list_count() << "\n";
    }

    virtual std::string tostring(int wkey=15, int wval=18)
    {
        std::ostringstream ss;
        tostream(ss, wkey, wval);
        return ss.str();
    }

    virtual void show(int wkey=15, int wval=28)
    {
        tostream(std::cerr);
    }

    virtual bool provides_att(){return false;}
    virtual bool provides_bath(){return false;}
    virtual bool provides_nav(){return false;}
    virtual bool provides_vel(){return false;}
protected:

    void buffer_data()
    {
        // lock data container list
        // use lock/unlock instead of std::lock_guard,
        // since trim() also locks mDataList
        mDataListMutex.lock();
        mDataList.emplace_front(mChannelName, mDataLen, mData, mRxBufferUsec, 0, mRxSequence);
        mDataListMutex.unlock();

        trim_data_list();
//        std::cerr << __func__ << ":" << __LINE__ << " ";
//        std::cerr << "list len["<< std::dec << list_count() << "]" << "\n";
    }

    void process_msg() override
    {
        // call base class like this
        // pcf::lcm_subscriber::process_msg();

        // add data (just collected by pcf::lcm_subscriber::handle_msg)
        // trims buffer to mListDepth
        buffer_data();

        // get a reference (w/o copy) to added element
#ifdef WITH_SHOW_DCON
        if(!mDataList.empty()){
            data_container &dcon = mDataList.front();
            // get a reference (w/ copy) to added element
            // data_container dcon = mDataList.front();
            dcon.show();
        }
#endif
        // Note: mDelegateNotify is initialized by the CTOR.
        // mDelegateNotify should be appropriately set and
        // observed by sub-classes to implement the intended
        // behavior, i.e. defer notification until processing
        // is complete
        if(!mDelegateNotify){
            TRN_NDPRINT(3, "TRN_LCM::%s:%d  NOTIFY SEM\n", __func__, __LINE__);
            notify_sem_list();
        }
    }

    // optionally delegate sem_notify to subclass
    bool mDelegateNotify;
    // data container list
    std::list<trn::data_container> mDataList;
    // semphore list (notiyy on message)
    std::list<sem_item> mSemList;
    // data list depth limit
    uint32_t mListDepth;
    // data list mutex
    std::mutex mDataListMutex;
    // data container error reference value
    static trn::data_container TRN_ENODCON;

private:


};

// Define: INIT_TRN_LCM_INPUT
// This must be defined once and only once per process.  The macro must be
// defined before the first class or derived class from the Precision Control
// Framework is included.
#ifdef INIT_TRN_LCM_INPUT
trn::data_container trn::trn_lcm_input::TRN_ENODCON;
#endif
}

#endif

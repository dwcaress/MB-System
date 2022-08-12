//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef DATA_CONTAINER_HPP  // include guard
#define DATA_CONTAINER_HPP

namespace trn
{
class data_container
{
public:
    data_container()
    : mArrTimeUsec(0), mDataTimeUsec(0), mSequence(0), mDataLen(0)
    {
        // copy name
        mChannelName = new std::string("DC_ERR");
        // copy data
        mData = nullptr;
    }
    data_container(const std::string& channel, uint32_t data_len, uint8_t *data, uint64_t atime, uint64_t dtime, int64_t seq)
    :mArrTimeUsec(atime), mDataTimeUsec(dtime), mSequence(seq), mDataLen(data_len)
    {
//        std::cerr << "data_container CTOR (fields)\n";
        // copy name
        mChannelName = new std::string(channel);
        // copy data
        mData = new uint8_t[mDataLen];
        std::memcpy(mData, data, data_len);
    }

    data_container(const std::string &channel, const lcm::ReceiveBuffer *rbuf, int64_t seq=0, int64_t dtime=0)
    :mArrTimeUsec(rbuf->recv_utime), mDataTimeUsec(dtime), mSequence(seq), mDataLen(rbuf->data_size)
    {
//        std::cerr << "data_container CTOR (ReceiveBuffer)\n";
        // copy name
        mChannelName = new std::string(channel);
        // copy data
        mData = new uint8_t[rbuf->data_size];
        std::memcpy(mData, rbuf->data, rbuf->data_size);
    }

    data_container(const data_container &other)
    :mArrTimeUsec(other.mArrTimeUsec), mDataTimeUsec(other.mDataTimeUsec), mSequence(other.mSequence), mDataLen(other.mDataLen)
    {
//        std::cerr << "data_container CTOR (copy)\n";
        // copy name
        mChannelName = new std::string(other.mChannelName->c_str());
        // copy data
        mData = new uint8_t[other.mDataLen];
        std::memcpy(mData, other.mData, other.mDataLen);
    }

    data_container(data_container &&other)
    :mChannelName(other.mChannelName), mArrTimeUsec(other.mArrTimeUsec), mDataTimeUsec(other.mDataTimeUsec), mSequence(other.mSequence), mDataLen(other.mDataLen),  mData(other.mData)
    {
//        std::cerr << "data_container MTOR\n";
    }

    ~data_container()
    {
        if(mChannelName)
            delete mChannelName;
        if(mData)
            delete mData;
    }

    uint32_t data_len(){ return mDataLen; }
    int64_t sequence(){ return mSequence; }
    int64_t arr_time(){ return mArrTimeUsec; }
    int64_t data_time(){ return mDataTimeUsec; }
    uint8_t *data_bytes(){ return mData; }
    void set_data_time(int64_t time)
    {
        mDataMutex.lock();
        mDataTimeUsec = time;
        mDataMutex.unlock();
    }
    void show(bool show_hex=true, int wkey=15, int wval=18)
    {

        mDataMutex.lock();
        std::cerr << std::dec << std::setfill(' ');
        std::cerr << std::setw(wkey) << "channel" << std::setw(wval) << mChannelName->c_str() << '\n';
        std::cerr << std::setw(wkey) << "atime" << std::setw(wval) << mArrTimeUsec << '\n';
        std::cerr << std::setw(wkey) << "dtime" << std::setw(wval) << mDataTimeUsec << '\n';
        std::cerr << std::setw(wkey) << "seq" << std::setw(wval) << mSequence << '\n';
        std::cerr << std::setw(wkey) << "len" << std::setw(wval) << mDataLen << '\n';
        std::cerr << std::setw(wkey) << "data" << '\n';

        bool phdr=true;
        if(show_hex){
            for(uint32_t i=0;i<mDataLen;i++){
                if(phdr){
                    std::cerr << std::setw(wkey-8) << std::setfill(' ') << " ";
                    std::cerr << std::hex << std::setw(8) << std::setfill('0') << i << ' ';
                    phdr=false;
                }
                std::cerr << std::hex << std::setw(2) << std::setfill('0') << (int)mData[i] << ' ';
                if(((i+1)%16)==0){
                    std::cerr << '\n';
                    phdr = true;
                }
            }
        }
        std::cerr << "\n\n";
        mDataMutex.unlock();
    }
protected:
private:
    // TODO: consider hashing channel name for determinacy, efficiency
    // name (LCM channel)
    std::string *mChannelName;
    // LCM arrival time
    int64_t mArrTimeUsec;
    // Data origin time
    int64_t mDataTimeUsec;
    // sequence number
    int64_t mSequence;
    // data payload length (bytes)
    uint32_t mDataLen;
    // data bytes
    uint8_t* mData;
    // data access mutex
    std::mutex mDataMutex;
};

}

#endif

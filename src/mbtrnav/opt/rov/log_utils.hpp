
/// @file log_utils.hpp
/// @authors k. headley
/// @date 27 mar 2022

/// Summary: logging/output utilities

// ///////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute
// Distributed under MIT license. See LICENSE file for more information.

// /////////////////
// Includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sstream>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <tuple>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <sys/stat.h>

#include "flag_utils.hpp"
#include "trn_debug.hpp"

#ifndef LOG_UTILS_HPP
#define LOG_UTILS_HPP

// /////////////////
// Macros

// Macros enable compiling out logging completely
#ifdef WITHOUT_LULOGGING

#define LU_PDEBUG(p,...)
#define LU_PNDEBUG(p,n,...)
#define LU_PVERBOSE(p,...)
#define LU_PEVENT(p,...)
#define LU_PINFO(p,...)
#define LU_PWARN(p,...)
#define LU_PERROR(p,...)
#define LU_ULOG(p,...)
#define LU_PLOG(p,...)
#define LU_BLOG(p,...)

#define LUP_PDEBUG(p,...)
#define LUP_PNDEBUG(p,n,...)
#define LUP_PVERBOSE(p,...)
#define LUP_PEVENT(p,...)
#define LUP_PINFO(p,...)
#define LUP_PWARN(p,...)
#define LUP_PERROR(p,...)
#define LUP_ULOG(p,...)
#define LUP_PLOG(p,...)
#define LUP_BLOG(p,...)

#else

#define LU_PDEBUG(p,...) p.pdebug(__VA_ARGS__)
#define LU_PNDEBUG(p,n,...) p.pndebug(n,__VA_ARGS__)
#define LU_PVERBOSE(p,...) p.pverbose(__VA_ARGS__)
#define LU_PEVENT(p,...) p.pevent(__VA_ARGS__)
#define LU_PINFO(p,...) p.pinfo(__VA_ARGS__)
#define LU_PWARN(p,...) p.pwarn(__VA_ARGS__)
#define LU_PERROR(p,...) p.perror(__VA_ARGS__)
#define LU_ULOG(p,...) p.ulog(__VA_ARGS__)
#define LU_PLOG(p,...) p.plog(__VA_ARGS__)
#define LU_BLOG(p,...) p.blog(__VA_ARGS__)

#define LUP_PDEBUG(p,...) p->pdebug(__VA_ARGS__)
#define LUP_PNDEBUG(p,n,...) p->pndebug(n,__VA_ARGS__)
#define LUP_PVERBOSE(p,...) p->pverbose(__VA_ARGS__)
#define LUP_PEVENT(p,...) p->pevent(__VA_ARGS__)
#define LUP_PINFO(p,...) p->pinfo(__VA_ARGS__)
#define LUP_PWARN(p,...) p->pwarn(__VA_ARGS__)
#define LUP_PERROR(p,...) p->perror(__VA_ARGS__)
#define LUP_ULOG(p,...) p->ulog(__VA_ARGS__)
#define LUP_PLOG(p,...) p->plog(__VA_ARGS__)
#define LUP_BLOG(p,...) p->blog(__VA_ARGS__)

#endif

// /////////////////
// Types

namespace logu {

using file_item = std::tuple<std::FILE*, bool>;
using dmap_item = std::tuple<uint32_t, std::vector<std::string>>;

typedef uint8_t byte;

typedef enum
{
    LL_NONE=0,
    LL_ERR,
    LL_WARN,
    LL_EVENT,
    LL_INFO,
    LL_VERBOSE,
    LL_DEBUG,
    LL_DFL
}log_level_bits_t;
typedef uint32_t log_level_t;

typedef enum
{
    DF_NONE=0x0,
    DF_SOUT=0x1,
    DF_SERR=0x2,
    DF_FILE=0x4,
    DF_ALL=0x7,
    DF_INVALID=0x10
}log_stream_sel_bits_t;
typedef uint32_t stream_sel_t;

typedef enum{
    LF_TIME_ISO8601=0x1,
    LF_TIME_POSIX_S=0x2,
    LF_TIME_POSIX_MS=0x4,
    LF_LVL_SHORT=0x10,
    LF_LVL_LONG=0x20,
    LF_SEP_COMMA=0x40,
    LF_SEP_SPACE=0x80,
    LF_SEP_TAB=0x100,
    LF_SEP_DASH=0x200,
    LF_SEP_SEMI=0x400,
    LF_SEP_USR=0x800,
    LF_DEL_UNIX=0x1000,
    LF_DEL_CRLF=0x2000,
    LF_DEL_USR=0x4000,
    LF_BIN_RAW=0x8000,
    LF_BIN_HEX=0x10000,
    LF_CHANNEL=0x20000,
    LF_TIME_BITS=0x00007,
    LF_LVL_BITS=0x00030,
    LF_SEP_BITS=0x000FC,
    LF_DEL_BITS=0x07000,
    LF_BIN_BITS=0x18000
}record_format_bits_t;
typedef uint32_t record_format_t;

typedef std::vector<std::string> keys_t;

class log_profile
{
public:
    log_profile()
    :mLevel(logu::LL_DFL), mDestKeys({"stderr"}),mFmtFlags()
    {}
    log_profile(log_level_t level, keys_t keys, flag_var<uint32_t> fflags)
    :mLevel(level), mDestKeys(keys),mFmtFlags(fflags)
    {}
    log_profile(const log_profile &other)
    :mLevel(other.mLevel), mDestKeys(other.mDestKeys),mFmtFlags(other.mFmtFlags)
    {}
    ~log_profile()
    {}

    flag_var<uint32_t> fflags(){return mFmtFlags;}
    log_level_t level(){return mLevel;}

    bool dest_en(const std::string &key)
    {
        keys_t::iterator it;
        for(it=mDestKeys.begin(); it != mDestKeys.end(); it++){
            std::string dstr = static_cast<std::string>(*it);
            if( dstr.compare(key) == 0)
                return true;

        }
        return false;
    }
    keys_t::iterator dest_begin(){return mDestKeys.begin();}
    keys_t::iterator dest_end(){return mDestKeys.end();}
private:
    log_level_t mLevel;
    keys_t mDestKeys;
    flag_var<uint32_t> mFmtFlags;
};


class utils
{
public:
    static std::string sep_str(flag_var<uint32_t> &fmt_flags, const char *usr=nullptr)
    {
        // add the milliseconds to the string
        std::ostringstream ss;

        if(fmt_flags&LF_SEP_COMMA){
            ss << ",";
        } else  if(fmt_flags&LF_SEP_SPACE){
            ss << " ";
        } else  if(fmt_flags&LF_SEP_TAB){
            ss << "\t";
        } else  if(fmt_flags&LF_SEP_DASH){
            ss << "-";
        } else  if(fmt_flags&LF_SEP_SEMI){
            ss << ";";
        } else  if(fmt_flags&LF_SEP_USR){
            ss << (usr != nullptr ? usr : ",");
        }  else  {
            ss << ",";
        }

        return ss.str();
    }

    static std::string del_str(flag_var<uint32_t> &fmt_flags)
    {
        // add the milliseconds to the string
        std::ostringstream ss;

        if(fmt_flags.is_set(LF_DEL_CRLF)){
            ss << "\r\n";
        } else  {
            ss << "\n";
        }

        return ss.str();
    }

    static std::string level_str(const log_level_t level, flag_var<uint32_t> &fflags)
    {
        // add the milliseconds to the string
        std::ostringstream ss;

        if(level == LL_ERR){
            if(fflags.is_set(LF_LVL_LONG)){
                ss << "ERR";
            } else {
                ss << "e";
            }
        } else  if(level == LL_WARN){
            if(fflags.is_set(LF_LVL_LONG)){
                ss << "WARN";
            } else {
                ss << "w";
            }
        } else  if(level == LL_INFO){
            if(fflags.is_set(LF_LVL_LONG)){
                ss << "INFO";
            } else {
                ss << "i";
            }
        } else  if(level == LL_VERBOSE){
            if(fflags.is_set(LF_LVL_LONG)){
                ss << "VERB";
            } else {
                ss << "v";
            }

        } else  if(level == LL_DEBUG){
            if(fflags.is_set(LF_LVL_LONG)){
                ss << "DEBUG";
            } else {
                ss << "d";
            }

        }  else  if(level == LL_EVENT){
            if(fflags.is_set(LF_LVL_LONG)){
                ss << "EVENT";
            } else {
                ss << "*";
            }

        } else  {
            ss << "-";
        }

        return ss.str();
    }

    static double dtime()
    {
        auto ts_now = std::chrono::system_clock::now();
        std::chrono::duration<double> epoch_time = ts_now.time_since_epoch();
        double ts = epoch_time.count();
        return ts;
    }
    static std::string time_str(flag_var<uint32_t> &fmt_flags)
    {

        auto ts_now = std::chrono::system_clock::now();
        std::chrono::duration<double> epoch_time = ts_now.time_since_epoch();
        double ts = epoch_time.count();

        std::time_t tt_now = static_cast<time_t>(ts);
        double ipart = 0.;

        // get the milliseconds from the ts
        int ms = (int)(1000.0 * std::modf(ts, &ipart));

        // create a formatted time stamp string
        char buf[64] = {0};

        // add the milliseconds to the string
        std::ostringstream ss;

        if(fmt_flags.is_set(LF_TIME_ISO8601)){
            std::strftime(buf, 64, "%Y-%m-%dT%H:%M:%S", std::gmtime(&tt_now));
            ss << std::string(buf);
            ss << "." << std::setfill('0') << std::setw(3) << ms << "Z";

        } else if(fmt_flags.is_set(LF_TIME_POSIX_S)){
            ss << std::dec << std::fixed << std::setprecision(0) << ipart ;
        } else if(fmt_flags.is_set(LF_TIME_POSIX_MS)){
            ss << std::dec << std::fixed << std::setprecision(3) << ts;
        } else {
            ss << "";
        }

        return ss.str();
    }

    static bool dir_exists(const std::string &dir_path, bool create)
    {
        struct stat buffer;
        if(stat(dir_path.c_str(), &buffer) == 0)
            return true;
        if(create && mkdir(dir_path.c_str(), (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) == 0)
            return true;
        return false;
    }

    static int open_file(FILE **dest, const std::string &path, std::string &r_path, bool force=true, const char *mode="a+")
    {
        *dest = nullptr;

        char *dcopy = strdup(path.c_str());
        char *fcopy = strdup(path.c_str());
        std::string dstr(dirname(dcopy));
        std::string fstr(basename(fcopy));
        free(dcopy);
        free(fcopy);

        if(logu::utils::dir_exists(dstr, true) == false){
            std::string fpath = "./" + fstr;
            TRN_DPRINT("%s:%d - could not create [%s]; changing path [%s]\n", __func__, __LINE__, path.c_str(), fpath.c_str());
            *dest = std::fopen(fpath.c_str(), mode);
            r_path = fpath;
        }else{
            *dest = std::fopen(path.c_str(), mode);
            r_path = path;
        }
        return (*dest != nullptr ? 0 : -1);
    }
};

class logger
{
public:
    logger()
    : mProfileMap(), mFileMap(), mLevel(0)
    {
        //mFileMap.emplace_hint(mFileMap.begin(), std::string("stderr"),stderr);
        //mFileMap.emplace_hint(mFileMap.end(), std::string("stdout"),stdout);
        add_file("stderr", stderr,true);
        add_file("stdout", stdout,true);
    }

    ~logger()
    {
    }

    void set_profile(log_level_t level, keys_t keys, flag_var<uint32_t> fflags)
    {
        mProfileMap[level] = log_profile(level, keys, fflags);
    }

    int add_file(const std::string &key, FILE *file, bool enable=false)
    {
        int retval = -1;

        if(file!=stdout && file!=stderr){
            std::map<std::string, file_item>::iterator it;
            it = mFileMap.find(key);

            if(it == mFileMap.end()){
                file_item fi = std::make_tuple(file, enable);
                mFileMap.emplace(std::string(key), fi);
                retval = 0;
            }// already exists
        }// already contains stdout, stderr
        return retval;
    }

    FILE *add_file(const std::string &key, const std::string &path, const std::string &mode="w+", bool enable=false)
    {
        FILE *retval = nullptr;

        std::map<std::string, file_item>::iterator it;

        it = mFileMap.find(key);

        if(it == mFileMap.end()){

            FILE *file=nullptr;
            std::string r_path;
            logu::utils::open_file(&file, path.c_str(), r_path, true, mode.c_str() );


            if(file != nullptr){
                file_item fi = std::make_tuple(file, enable);
                mFileMap.emplace(std::string(key), fi);
                retval = file;
            }
        }// already exists
        return retval;
    }

    FILE *lookup_file(const std::string &key)
    {
        FILE *retval = nullptr;

        std::map<std::string, file_item>::iterator it;

        it = mFileMap.find(key);

        if(it != mFileMap.end()){
            file_item fi = it->second;
            FILE *fp = std::get<0>(fi);
            return fp;
        }// already exists
        return retval;
    }

    void enable_stdout()
    {
        std::list<std::string>::iterator it;
    }

    log_profile profile(log_level_t level)
    {
        std::map<log_level_t,log_profile>::iterator it;

        for(it=mProfileMap.begin(); it!=mProfileMap.end(); it++){

            log_profile prof = static_cast<log_profile>(it->second);

            if(  prof.level() == level ){
               return prof;
            }
        }

        // return add default profile (creates if it doesn't exist)
        return mProfileMap[logu::LL_DFL];
    }

    int level()
    {
        return mLevel;
    }

    void set_level(int level)
    {
        mLevel = level;
    }

    int pdebug(const char *fmt, ...)
    {
        int retval = 0;

        va_list va;
        va_start(va,fmt);
        vdprint(LL_DEBUG, fmt, va);
        va_end(va);
        return retval;
    }

    int pndebug(int n, const char *fmt, ...)
    {
        int retval = 0;
        if(n <= mLevel){
            va_list va;
            va_start(va,fmt);
            vdprint(LL_DEBUG, fmt, va);
            va_end(va);
        }

        return retval;
    }

    int pverbose(const char *fmt, ...)
    {
        int retval = 0;

        va_list va;
        va_start(va,fmt);
        vdprint(LL_VERBOSE, fmt, va);
        va_end(va);
        return retval;
    }

    int pevent(const char *fmt, ...)
    {
        int retval = 0;

        va_list va;
        va_start(va,fmt);
        vdprint(LL_EVENT, fmt, va);
        va_end(va);
        return retval;
    }

    int pinfo(const char *fmt, ...)
    {
        int retval = 0;

        va_list va;
        va_start(va,fmt);
        vdprint(LL_INFO, fmt, va);
        va_end(va);
        return retval;
    }

    int pwarn(const char *fmt, ...)
    {
        int retval = 0;

        va_list va;
        va_start(va,fmt);
        vdprint(LL_WARN, fmt, va);
        va_end(va);
        return retval;
    }

    int perror(const char *fmt, ...)
    {
        int retval = 0;

        va_list va;
        va_start(va,fmt);
        vdprint(LL_ERR, fmt, va);
        va_end(va);
        return retval;
    }

    int ulog(const std::string &key, const char *fmt, ...)
    {
        int retval = 0;
        FILE *fp = lookup_file(key);
        if(nullptr != fp){
            va_list va;
            va_start(va,fmt);
            retval = vfprintf(fp, fmt, va);
            va_end(va);
        }
        return retval;
    }

    int plog(logu::log_profile prof, const char *fmt, ...)
    {
        int retval = 0;
        va_list va;
        va_start(va,fmt);
        retval = vpprint(prof, fmt, va);
        va_end(va);
        return retval;
    }

    int blog(const std::string &key, uint8_t *src, size_t len)
    {
        int retval = 0;
        FILE *fp = lookup_file(key);
        if(nullptr != fp){
            retval = fwrite(src, len, 1, fp);
        }
        return retval;
    }


protected:

    static int vrprint(FILE *fp, log_level_t level, flag_var<uint32_t> &fflags, const char *fmt, va_list va)
    {
        int retval = -1;
        // TODO: log size management (segmentation, rotation)
        if(NULL != fp){
            std::string sep = utils::sep_str(fflags);
            if(fflags.any_set(logu::LF_TIME_BITS)){
                retval += fprintf(fp,"%s%s", utils::time_str(fflags).c_str(), sep.c_str());
            }
            if(fflags.any_set(logu::LF_LVL_BITS)){
                retval += fprintf(fp,"%s%s", utils::level_str(level,fflags).c_str(), sep.c_str());
            }
            retval +=  vfprintf(fp, fmt, va);
            retval += fprintf(fp,"%s", utils::del_str(fflags).c_str());
        }
        return retval;
    }

    int vdprint(log_level_t level, const char *fmt, va_list va)
    {
        int retval = 0;
        log_profile prof = profile(level);
        flag_var<uint32_t> fflags = prof.fflags();

        keys_t::iterator it;
        for(it = prof.dest_begin(); it != prof.dest_end(); it++){
            std::string key = static_cast<std::string>(*it);
            std::map<std::string,file_item>::iterator it = mFileMap.find(key);
            if( it != mFileMap.end())
            {
                file_item fi = it->second;
                FILE *fp = std::get<0>(fi);
                if(fp != nullptr){
                    va_list vac;
                    va_copy(vac, va);
                    retval = vrprint(fp, level, fflags, fmt, vac);
                    va_end(vac);
                }
            }

        }
        return retval;
    }

    int vpprint(logu::log_profile prof, const char *fmt, va_list va)
    {
        int retval = 0;
        flag_var<uint32_t> fflags = prof.fflags();

        keys_t::iterator it;
        for(it = prof.dest_begin(); it != prof.dest_end(); it++){
            std::string key = static_cast<std::string>(*it);
            std::map<std::string,file_item>::iterator it = mFileMap.find(key);
            if( it != mFileMap.end())
            {
                file_item fi = it->second;
                FILE *fp = std::get<0>(fi);
                if(fp != nullptr){
                    va_list vac;
                    va_copy(vac, va);
                    retval = vrprint(fp, prof.level(), fflags, fmt, vac);
                    va_end(vac);
                }
            }

        }
        return retval;
    }

private:
    std::map<log_level_t, log_profile> mProfileMap;
    std::map<std::string, file_item> mFileMap;
    int mLevel;
};

/// log attribute flags
/// LF_NOLIMIT no-limit value
/// LF_MONO    monolithic log (single segment)
/// LF_DIS     disable log output
/// LF_OVWR    enable segment overwrite (rotatation)
/// LF_SEG    segment log
/// LF_LIMLEN  limit segments by length
/// LF_LIMTIME limit segments by time
/// LF_SESSION file name includes session
//typedef enum{
//    LF_NOLIMIT=0x40,
//    LF_MONO=0x20,
//    LF_DIS=0x1,
//    LF_OVWR=0x2,
//    LF_SEG=0x4,
//    LF_LIMLEN=0x8,
//    LF_LIMTIME=0x10,
//    LF_SESSION=0x20
//} log_attr_bits_t;
//typedef uint32_t log_attr_t;

}

#endif

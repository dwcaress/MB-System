//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef TRN_DEBUG_HPP  // include guard
#define TRN_DEBUG_HPP

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <stdarg.h>

// Module debug output macros
// Applications may set the debug level for this module using
//   trn_debug::instance()->set_level(n)
//
// the default level is zero.
//
//   TRN_DPRINT(...)     : output to stderr for any level != 0
//   TRN_NDPRINT(n,...)  : output to stderr for any level > n
//   TRN_DFPRINT(f,...)  : output to file for any level != 0
//   TRN_NDFPRINT(n,...) : output to file for any level > n
//   TRN_VPRINT(n,...)   : output to file if verbose is enabled
//   TRN_TRACE()         : output function:line for debugging
//
// Comment out the macro body to compile out debug calls:
//   #define TRN_DPRINT(...) //if(trn::...
// see also trn_debug
#define TRN_DPRINT(...) trn_debug::get()->dprint(__VA_ARGS__)
#define TRN_NDPRINT(n,...) trn_debug::get()->ndprint(n,__VA_ARGS__)
#define TRN_DFPRINT(fp,...) trn_debug::get()->dfprint(fp,__VA_ARGS__)
#define TRN_NDFPRINT(n,fp,...) trn_debug::get()->ndfprint(n,fp,__VA_ARGS__)
#define TRN_VPRINT(...) trn_debug::get()->vprint(__VA_ARGS__)
#define TRN_TRACE() fprintf(stderr,"%s:%d\n",__func__,__LINE__)


// module level debug output singleton
// use with TRN* debug macros
// applications may set the module debug
// output level using
//   trn_debug:::instance()->set_level(n)
// to release trn_debug resources, applications should call
//   trn_debug::instance(true)
class trn_debug
{
public:
    static trn_debug *get(bool release=false)
    {
        static trn_debug *mInstance = nullptr;

        if(release){
            if(mInstance != nullptr)
                delete mInstance;
            mInstance = nullptr;
        }else{
            if(mInstance == nullptr)
                mInstance = new trn_debug();
        }
        return mInstance;
    }

    void set_debug(int level)
    {
        mLevel = level;
    }

    int debug()
    {
        return mLevel;
    }

    void set_verbose(bool enable_verbose)
    {
        mVerbose = enable_verbose;
    }

    bool verbose()
    {
        return mVerbose;
    }

    // debug output methods
    // recommended to use macros, enabling debug
    // messages to be omitted from compilation
    int dprint(const char *fmt, ...)
    {
        int retval = 0;
        va_list va1;
        va_start(va1,fmt);
        retval = trn_debug::af_vfprintf(stderr, mLevel!=0, fmt, va1);
        va_end(va1);
        return retval;
    }

    int dfprint(FILE *fp, const char *fmt, ...)
    {
        int retval = 0;
        va_list va1;
        va_start(va1,fmt);
        retval = trn_debug::af_vfprintf(fp, mLevel!=0, fmt, va1);
        va_end(va1);
        return retval;
    }

    int ndprint(int n, const char *fmt, ...)
    {
        int retval = 0;
        if(n<=mLevel){
            va_list va1;
            va_start(va1,fmt);
            retval = trn_debug::af_vfprintf(stderr, n<=mLevel, fmt, va1);
            va_end(va1);
        }
        return retval;
    }

    int ndfprint(int n, FILE *fp, const char *fmt, ...)
    {
        int retval = 0;
        if(n<=mLevel){
            va_list va1;
            va_start(va1,fmt);
            retval = trn_debug::af_vfprintf(fp, n<=mLevel, fmt, va1);
            va_end(va1);
        }
        return retval;
    }

    int vprint(const char *fmt, ...)
    {
        int retval = 0;
        va_list va1;
        va_start(va1,fmt);
        retval = trn_debug::af_vfprintf(stderr, mVerbose, fmt, va1);
        va_end(va1);
        return retval;
    }

protected:
    static int af_vfprintf(FILE *fp, bool en_cond, const char *fmt, va_list va)
    {
        int retval = 0;
        if(en_cond && NULL!=fp){
            retval = vfprintf(fp, fmt, va);
        }
        return retval;
    }
    static int af_fprintf(FILE *fp, bool en_cond, const char *fmt, ...)
    {
        int retval = 0;
        if(en_cond && NULL!=fp){
            va_list va1;
            va_start(va1,fmt);
            retval = vfprintf(fp, fmt, va1);
            va_end(va1);
        }
        return retval;

    }

private:
    trn_debug()
    :mLevel(0), mVerbose(false)
    {
    }
    ~trn_debug()
    {}
    int mLevel;
    bool mVerbose;
};

#endif // include guard

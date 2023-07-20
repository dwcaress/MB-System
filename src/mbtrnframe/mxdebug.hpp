///
/// @file mxdebug.hpp
/// @authors k. Headley
/// @date 06 jul 2023

/// debug output macros
///  - supports per-module output
///  - conditional output: always, module exists, output level comparison, boolean expression
///  - thread-safe with  pthread mutex support (-DWITHOUT_PTHREAD_MUTEX to disable)
///  - C, C++ APIs
///
/// Compile C++ test
/// gcc -c -g -O0 -I. -I/usr/local/include -I/usr/include mlist.c
/// g++ -g -O0 -std=c++11 -I. -I/usr/local/include -I/usr/include mxdebug.cpp  mxdcpp-test.cpp  -o mxdcpp-test -L/usr/lib -lstdc++ mlist.o
///
/// Compile C test
/// g++ -c -g -O0  -DMXDEBUG_C_API -I. -I/usr/local/include -I/usr/include mxdebug.cpp -o mxdebug.o
/// gcc -g -O0  -I. -I/usr/local/include -I/usr/include  mxdc-test.c mlist.c -o mxdc-test mxdebug.o
///
/// Compile options:
/// -DWITHOUT_MXDEBUG disable MXDBUG macros
/// -DWITHOUT_PTHREAD_MUTEX no mutex (maybe faster for single threaded use)
/// -DMXDEBUG_C_API required for C API
/// -DWITH_FLOCK enable stderr file locking (flockfile, not enabled by default, unneeded?)
/// -DWITH_MXD_DEBUG enable MXDebug internal debug info

///
/// Use
/// setModule(id, "name", level)
//// pre-defined info, warn,debug, error channel IDs: MXINFO, MXDEBUG, MXWARN, MXERROR
// Reconfigurable, channel IDs may be used with other macros
///  MX_INFO(fmt, ...)
///  MX_WARN(fmt, ...)
///  MX_DEBUG(fmt, ...)
///  MX_ERROR(fmt, ...)
/// always
///  MX_PRINT(fmt, ...)
///  MX_MSG(fmt, ...)
/// if module defined
///  MX_DPRINT(mid, fmt, ...)
///  MX_DMSG(mid, fmt, ...)
/// if module level != 0
///  MX_MPRINT(mid, fmt, ...)
///  MX_MMSG(mid, fmt, ...)
/// if module level <= N
///  MX_LPRINT(mid, N, fmt, ...)
///  MX_LMSG(mid, N, fmt, ...)
/// if B is true
///  MX_BPRINT(B, fmt, ...)
///  MX_BMSG(B, fmt, ...)
/// if module defined and B is true
///  MX_MBPRINT(mid, B, fmt, ...)
///  MX_MBMSG(mid, B, fmt, ...)
/// trace
///  MX_TRACE()
///  MX_MTRACE(m)

/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information

 Copyright 2000-2018 MBARI
 Monterey Bay Aquarium Research Institute, all rights reserved.

 Terms of Use

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version. You can access the GPLv3 license at
 http://www.gnu.org/licenses/gpl-3.0.html

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details
 (http://www.gnu.org/licenses/gpl-3.0.html)

 MBARI provides the documentation and software code "as is", with no warranty,
 express or implied, as to the software, title, non-infringement of third party
 rights, merchantability, or fitness for any particular purpose, the accuracy of
 the code, or the performance or results which you may obtain from its use. You
 assume the entire risk associated with use of the code, and you agree to be
 responsible for the entire cost of repair or servicing of the program with
 which you are using the code.

 In no event shall MBARI be liable for any damages, whether general, special,
 incidental or consequential damages, arising out of your use of the software,
 including, but not limited to, the loss or corruption of your data or damages
 of any kind resulting from use of the software, any prohibited use, or your
 inability to use the software. You agree to defend, indemnify and hold harmless
 MBARI and its officers, directors, and employees against any claim, loss,
 liability or expense, including attorneys' fees, resulting from loss of or
 damage to property or the injury to or death of any person arising out of the
 use of the software.

 The MBARI software is provided without obligation on the part of the
 Monterey Bay Aquarium Research Institute to assist in its use, correction,
 modification, or enhancement.

 MBARI assumes no responsibility or liability for any third party and/or
 commercial software required for the database or applications. Licensee agrees
 to obtain and maintain valid licenses for any additional third party software
 required.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <pthread.h>

#include "mxdebug-common.h"
#include "mlist.h"


#ifndef MXDEBUG_HPP
#define MXDEBUG_HPP

#ifdef __cplusplus
extern "C" {
#endif

// C++ output API
#ifndef WITHOUT_MXDEBUG

// pre-defined info, warn,debug, error channel IDs
// Reconfigurable, channel IDs may be used with other macros
#define MX_INFO(fmt, ...) {MXDebug::mdprint(MXINFO, fmt, __VA_ARGS__);}
#define MX_WARN(fmt, ...) {MXDebug::mdprint(MXWARN, fmt, __VA_ARGS__);}
#define MX_DEBUG(fmt, ...) {MXDebug::mdprint(MXDEBUG, fmt, __VA_ARGS__);}
// TODO: make error print w/ func, line
#define MX_ERROR(fmt, ...) {MXDebug::mdprint(MXERROR, fmt, __VA_ARGS__);}

#define MX_INFO_MSG(fmt, ...) {MXDebug::mdprint(MXINFO, fmt);}
#define MX_WARN_MSG(fmt, ...) {MXDebug::mdprint(MXWARN, fmt);}
#define MX_DEBUG_MSG(fmt, ...) {MXDebug::mdprint(MXDEBUG, fmt);}
#define MX_ERROR_MSG(fmt, ...) {MXDebug::mdprint(MXERROR, fmt);}

// always
#define MX_PRINT(fmt, ...) {MXDebug::dprint(fmt, __VA_ARGS__);}
#define MX_MSG(fmt, ...) {MXDebug::dprint(fmt);}
// if module defined
#define MX_DPRINT(mid, fmt, ...) {MXDebug::mdprint(mid, fmt, __VA_ARGS__);}
#define MX_DMSG(mid, fmt, ...) {MXDebug::dprint(mid, fmt);}
// if module level != 0
#define MX_MPRINT(mid, fmt, ...) {MXDebug::nzprint(mid, fmt, __VA_ARGS__);}
#define MX_MMSG(mid, fmt, ...) {MXDebug::nzprint(mid, fmt);}
// if module level <= N
#define MX_LPRINT(mid, N, fmt, ...) {MXDebug::geprint(mid, N, fmt, __VA_ARGS__);}
#define MX_LMSG(mid, N, fmt, ...) {MXDebug::geprint(mid, N, fmt);}
// if B is true
#define MX_BPRINT(B, fmt, ...) {if(B)MXDebug::dprint(fmt, __VA_ARGS__);}
#define MX_BMSG(B, fmt, ...) {if(B)MXDebug::dprint(fmt);}
// if module defined and B is true
#define MX_MBPRINT(mid, B, fmt, ...) {if(B)MXDebug::mdprint(mid, fmt, __VA_ARGS__);}
#define MX_MBMSG(mid, B, fmt, ...) {if(B)MXDebug::mdprint(mid, fmt);}
// trace
//#define MX_TRACE()  {std::cerr << __func__ << ":" << __LINE__ << std::endl;}
//#define MX_MTRACE(m)  {std::cerr << "## "#m": " << __func__ << ":" << __LINE__ << std::endl;}
#define MX_TRACE()  {fprintf(stderr, "%s:%d\n", __func__, __LINE__);}
//#define MX_MTRACE(m)  {fprintf(stderr, "%s:%d %s\n", __func__, __LINE__, m);}
//// a hack for iostreams
//#define MX_DCERR(sfmt) if(MXDebug::debug()){ std::cerr << sfmt; }
//#define MX_MCERR(m, sfmt) if(MXDebug::debug()){ std::cerr << std::endl << m << sfmt; }
//#define MX_CCERR(c, sfmt) if(c){ std::cerr << sfmt; }
//#define MX_DOSTREAM(os, sfmt) if(MXDebug::debug()){ os << sfmt; }
//#define MX_COSTREAM(os, c, sfmt) if(c){ os << sfmt; }

#else
// pre-defined info, warn,debug, error channels (reconfigurable)
#define MX_INFO(fmt, ...)
#define MX_WARN(fmt, ...)
#define MX_DEBUG(fmt, ...)
#define MX_ERROR(fmt, ...)

// always
#define MX_PRINT(fmt, ...)
#define MX_MSG(fmt, ...)
// if module defined
#define MX_DPRINT(mid, fmt, ...)
#define MX_DMSG(mid, fmt, ...)

// if module level != 0
#define MX_MPRINT(mid, fmt, ...)
#define MX_MMSG(mid, fmt, ...)
// if module level <= N
#define MX_LPRINT(mid, N, fmt, ...)
#define MX_LMSG(mid, N, fmt, ...)
// if B is true
#define MX_BPRINT(B, fmt, ...)
#define MX_BMSG(B, fmt, ...)
// if module defined and B is true
#define MX_MBPRINT(mid, B, fmt, ...)
#define MX_MBMSG(mid, B, fmt, ...)
// trace
#define MX_TRACE()
#define MX_MTRACE(m)

// a hack for iostreams
//#define MX_DCERR(sfmt)
//#define MX_CCERR(c, sfmt)
//#define MX_MCERR(m, sfmt)
//#define MX_DOSTREAM(os, sfmt)
//#define MX_COSTREAM(os,c, sfmt)

#endif

#ifdef WITH_MXD_DEBUG
#define MXD_PRINT(fmt, ...) {fprintf(stderr, fmt, __VA_ARGS__);}
#define MXD_MSG(fmt) {fprintf(stderr, fmt);}
#else
#define MXD_PRINT(fmt, ...)
#define MXD_MSG(fmt)
#endif

class MXDebug
{
private:

    MXDebug()
    {
        MXD_PRINT("%s:%d ++++++++ CTOR ++++++++\n", __func__, __LINE__);
    }
    ~MXDebug()
    {
        MXD_PRINT("%s:%d ++++++++ DTOR ++++++++\n", __func__, __LINE__);
    }

    static int init()
    {
        if(m_mlist == NULL){
            m_mlist = mlist_new();
            mlist_autofree(m_mlist, module_destroy);
        }else{
            // empty the module list
            mlist_autofree(m_mlist, module_destroy);
            mlist_purge(m_mlist);
        }

        // set default modules
        mx_module_t *mods[] = {
            module_new(MXINFO,  0, false, "mx.info"),
            module_new(MXDEBUG, 0, false, "mx.debug"),
            module_new(MXWARN,  0, false, "mx.warn"),
            module_new(MXERROR, 0, false, "mx.err"),
            NULL
        };
        for(mx_module_t **pmod = &mods[0]; *pmod != NULL; pmod++){
            mlist_add(m_mlist, *pmod);
        }
        return 0;
    }

    static mx_module_t *lookupModule(int id)
    {
        MUTEX_LOCK(&m_list_mutex);
        mx_module_t *mod = (mx_module_t *)mlist_first(m_mlist);
        while(mod != NULL)
        {
            if(mod->id == id){
                // module found; return
                break;
            }
            mod = (mx_module_t *)mlist_next(m_mlist);
        }
        MUTEX_UNLOCK(&m_list_mutex);
        return mod;
    }

    static mx_module_t *module_new(int id, int level=0, bool suspend=false, const char *name=NULL)
    {
        mx_module_t *instance = (mx_module_t *)malloc(sizeof(mx_module_t));
        if(instance != NULL){
            memset(instance, 0, sizeof(mx_module_t));
            instance->id = id;
            instance->name = (name == NULL ? NULL : strdup(name));
            instance->level = level;
            instance->suspend = suspend;
        }
        return instance;
    }

    static void module_destroy(void *pself)
    {
        if(pself != NULL){
            mx_module_t *self = (mx_module_t *)pself;
            if(self != NULL){
                free(self->name);
                free(self);
            }
        }
    }

    static int module_vcmp(void *a, void *b)
    {
        mx_module_t *pa = (mx_module_t *)a;
        mx_module_t *pb = (mx_module_t *)b;
        if(pa->id == pb->id)
            return 0;
        if(pa->id > pb->id)
            return 1;
        return -1;
    }

public:

    static void setModule(int id, int level, bool suspend=false, const char *name = NULL)
    {
        MXD_PRINT("%s: add module %s id:%d level:%d suspend:%c\n", __func__, name, id, level, MXD_BOOL2CH(suspend));

        if(mlist_size(m_mlist)==0)
            init();

        mx_module_t *mod = lookupModule(id);

        MUTEX_LOCK(&m_list_mutex);

        if(mod != NULL) {
            // exists, set name, level
            if(name != NULL){
                free(mod->name);
                mod->name = strdup(name);
            }
            mod->level = level;
            mod->suspend = suspend;
        } else {
            // doesn't exist, add it
            mx_module_t *mod = module_new(id, level, suspend, name);
            mlist_add(m_mlist, mod);
            MXD_PRINT("%s: added module %s id:%d level:%d suspend:%c\n", __func__, mod->name, mod->id, mod->level, MXD_BOOL2CH(mod->suspend));
        }

        MUTEX_UNLOCK(&m_list_mutex);
    }

    static void removeModule(int id)
    {
        mx_module_t *mod = lookupModule(id);

        MUTEX_LOCK(&m_list_mutex);

        if(mod != NULL) {
            mlist_remove(m_mlist, mod);
        }

        MUTEX_UNLOCK(&m_list_mutex);
    }

    static void suspend(int id, bool suspend)
    {
        mx_module_t *mod = lookupModule(id);

        MUTEX_LOCK(&m_list_mutex);

        if(mod != NULL) {
            mod->suspend = suspend;
        }

        MUTEX_UNLOCK(&m_list_mutex);
    }

    static void nSuspend(int *idSet, int len, bool suspend)
    {
        MUTEX_LOCK(&m_list_mutex);

        mx_module_t *next_mod = (mx_module_t *)mlist_first(m_mlist);

        while(next_mod != NULL){

            if(idSet == NULL){
                // suspend all
                next_mod->suspend = suspend;
            } else {
                // suspend set
                int *next_id = idSet;
                for(int i = 0; i < len; i++, next_id++) {
                    if(*next_id == next_mod->id){
                        next_mod->suspend = suspend;
                    }
                }
            }

            next_mod = (mx_module_t *)mlist_next(m_mlist);
        }

        MUTEX_UNLOCK(&m_list_mutex);
    }

    static bool suspended(int id)
    {
        mx_module_t *mod = lookupModule(id);

        if(mod != NULL) {
            return mod->suspend;
        }
        return false;
    }

    static void release()
    {
        MUTEX_LOCK(&m_list_mutex);
        mlist_autofree(m_mlist, module_destroy);
        mlist_purge(m_mlist);
        MUTEX_UNLOCK(&m_list_mutex);
    }

    static void destroy()
    {
        release();

        MUTEX_UNLOCK(&m_list_mutex);
        MUTEX_DESTROY(&m_list_mutex);

        MUTEX_UNLOCK(&m_write_mutex);
        MUTEX_DESTROY(&m_write_mutex);

        mlist_autofree(m_mlist, module_destroy);
        mlist_destroy(&m_mlist);
        m_mlist = NULL;
    }

    static void show(FILE *fp = stderr, int indent = 0)
    {
        if(m_mlist != NULL) {
            int wkey = 15;
            int wval = 15;
            MUTEX_LOCK(&m_list_mutex);

            if(fp == NULL){
                fp = stderr;
            }
            fprintf(fp, "%*s%*s %*lu\n", indent, "", wkey, "m_size", wval, mlist_size(m_mlist));
            mx_module_t *mod = (mx_module_t *)mlist_first(m_mlist);
            int i=0;
            while(mod != NULL){
                fprintf(fp, "%*s%*d %*s id[%+04d] level[%+04d] suspended[%c]\n", indent, "", wkey, i++, wval, (mod->name == NULL? "NULL" : mod->name), mod->id, mod->level, MXD_BOOL2CH(mod->suspend));
                mod = (mx_module_t *)mlist_next(m_mlist);

            }
            MUTEX_UNLOCK(&m_list_mutex);
        }
    }

    static char *name(int id)
    {
        mx_module_t *mod = lookupModule(id);

        if(mod != NULL) {
            return mod->name;
        }
        return NULL;
    }

    static void setName(int id, const char *name)
    {

        mx_module_t *mod = lookupModule(id);

        MUTEX_LOCK(&m_list_mutex);

        if(mod != NULL && name != NULL) {
            free(mod->name);
            mod->name = strdup(name);
        }

        MUTEX_UNLOCK(&m_list_mutex);
    }


    static int level(int id)
    {
        mx_module_t *mod = lookupModule(id);
        
        if(mod != NULL) {
            return mod->level;
        }
        return 0;
    }

    static void setLevel(int id, int level)
    {

        mx_module_t *mod = lookupModule(id);

        if(mod !=NULL){
            MUTEX_LOCK(&m_list_mutex);

            mod->level = level;

            MUTEX_UNLOCK(&m_list_mutex);
        }
    }

    static bool testModule(int id, int level)
    {
        bool retval = false;

        mx_module_t *mod = lookupModule(id);

        if(mod != NULL){
            MUTEX_LOCK(&m_list_mutex);

            // return true if module exists, level <= module leval and not suspended
            if(mod->level != 0 && mod->level >= level && !mod->suspend)
                retval = true;

            MUTEX_UNLOCK(&m_list_mutex);
        }
        return retval;
    }

    static bool nTestModule(int *idSet, int len, int level)
    {
        bool retval = false;

        if(idSet != NULL){

            for(int i = 0; i < len; i++) {
                if(testModule(idSet[i], level)){
                    retval = true;
                    break;
                }
            }

        }
        return retval;
    }

    static int size()
    {
        return  mlist_size(m_mlist);
    }

    static bool hasID(int id)
    {
        mx_module_t *mod = lookupModule(id);

        if(mod != NULL) {
            return true;
        }
        return false;
    }

    static void  autoNewline(bool enable)
    {
        m_auto_newline = enable;
    }

    static mx_module_t *save(int id)
    {
        mx_module_t *mod = lookupModule(id);
        mx_module_t *dest = NULL;
        if(mod != NULL) {
            dest = (mx_module_t *)malloc(sizeof(mx_module_t));
            if(dest != NULL){
                MUTEX_LOCK(&m_list_mutex);
                dest->id = mod->id;
                dest->level = mod->level;
                dest->suspend = mod->suspend;
                dest->name = mod->name == NULL ? NULL : strdup(mod->name);
                MUTEX_UNLOCK(&m_list_mutex);
            }
        }
        return dest;
    }

    static void restore(int id, mx_module_t *src)
    {
        mx_module_t *mod = lookupModule(id);

        if(mod != NULL && src != NULL) {
            MUTEX_LOCK(&m_list_mutex);
            mod->id = src->id;
            mod->level = src->level;
            mod->suspend = src->suspend;
            free(mod->name);
            mod->name = src->name == NULL ? NULL : strdup(src->name);
            MUTEX_UNLOCK(&m_list_mutex);
            free(src->name);
            free(src);
        }
    }

    static void dprint(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);

        MUTEX_LOCK(&m_write_mutex);
        FILE_LOCK(stderr);

        vfprintf(stderr, fmt, args);
        if( m_auto_newline)
            fprintf(stderr, "\n");

        FILE_UNLOCK(stderr);
        MUTEX_UNLOCK(&m_write_mutex);

        va_end(args);
    }

    static void vdprint(const char *fmt, va_list args)
    {
        MUTEX_LOCK(&m_write_mutex);
        FILE_LOCK(stderr);

        vfprintf(stderr, fmt, args);
        if( m_auto_newline)
            fprintf(stderr, "\n");

        FILE_UNLOCK(stderr);
        MUTEX_UNLOCK(&m_write_mutex);
    }

    static void mdprint(int id, const char *fmt, ...)
    {

        mx_module_t *mod = lookupModule(id);

        if(mod != NULL &&
           !mod->suspend)
        {
            va_list args;
            va_start(args, fmt);

            MUTEX_LOCK(&m_write_mutex);
            FILE_LOCK(stderr);

            if(mod->name != NULL &&
               strlen(mod->name) > 0) {
                fprintf(stderr, "%s ", mod->name);
            }
            vfprintf(stderr, fmt, args);
            if( m_auto_newline)
                fprintf(stderr, "\n");

            FILE_UNLOCK(stderr);
            MUTEX_UNLOCK(&m_write_mutex);

            va_end(args);
        }
    }

    static void vmdprint(int id, const char *fmt, va_list args)
    {
        mx_module_t *mod = lookupModule(id);

        if(mod != NULL &&
           !mod->suspend)
        {
            MUTEX_LOCK(&m_write_mutex);
            FILE_LOCK(stderr);

            if(mod->name != NULL &&
               strlen(mod->name) > 0) {
                fprintf(stderr, "%s ", mod->name);
            }
            vfprintf(stderr, fmt, args);
            if( m_auto_newline)
                fprintf(stderr, "\n");

            FILE_UNLOCK(stderr);
            MUTEX_UNLOCK(&m_write_mutex);
        }
    }

    static void geprint(int id, int lvl, const char *fmt, ...)
    {

        mx_module_t *mod = lookupModule(id);

        if(mod != NULL  &&
           !mod->suspend &&
           mod->level >= lvl)
        {

            va_list args;
            va_start(args, fmt);

            MUTEX_LOCK(&m_write_mutex);
            FILE_LOCK(stderr);

            fprintf(stderr, "%s ", mod->name);
            vfprintf(stderr, fmt, args);
            if( m_auto_newline)
                fprintf(stderr, "\n");

            FILE_UNLOCK(stderr);
            MUTEX_UNLOCK(&m_write_mutex);

            va_end(args);
        }
    }

    static void vgeprint(int id, int lvl, const char *fmt, va_list args)
    {
        mx_module_t *mod = lookupModule(id);

        if(mod != NULL &&
           !mod->suspend &&
           mod->level >= lvl)
        {

            MUTEX_LOCK(&m_write_mutex);
            FILE_LOCK(stderr);

            fprintf(stderr, "%s ", mod->name);
            vfprintf(stderr, fmt, args);
            if( m_auto_newline)
                fprintf(stderr, "\n");

            FILE_UNLOCK(stderr);
            MUTEX_UNLOCK(&m_write_mutex);
        }
    }

    static void nzprint(int id, const char *fmt, ...)
    {
        mx_module_t *mod = lookupModule(id);

        if(mod != NULL &&
           !mod->suspend
           && mod->level != 0)
        {

            va_list args;
            va_start(args, fmt);

            MUTEX_LOCK(&m_write_mutex);
            FILE_LOCK(stderr);

            fprintf(stderr, "%s ", mod->name);
            vfprintf(stderr, fmt, args);
            if( m_auto_newline)
                fprintf(stderr, "\n");

            FILE_UNLOCK(stderr);
            MUTEX_UNLOCK(&m_write_mutex);

            va_end(args);
        }
    }

    static void vnzprint(int id, const char *fmt, va_list args)
    {
        mx_module_t *mod = lookupModule(id);

        if(mod != NULL &&
           !mod->suspend
           && mod->level != 0)
        {
            MUTEX_LOCK(&m_write_mutex);
            FILE_LOCK(stderr);

            fprintf(stderr, "%s ", mod->name);
            vfprintf(stderr, fmt, args);
            if( m_auto_newline)
                fprintf(stderr, "\n");

            FILE_UNLOCK(stderr);
            MUTEX_UNLOCK(&m_write_mutex);
        }
    }

protected:
    static mlist_t *m_mlist;
    static pthread_mutex_t m_list_mutex;
    static pthread_mutex_t m_write_mutex;
    static bool m_auto_newline;
};

// C API wrapper functions

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#ifdef MXDEBUG_C_API
EXTERNC void mxd_setModule(int id, int level, int suspend, const char *name)
{
    MXDebug::setModule( id,  level, suspend, name);
}

EXTERNC void mxd_removeModule(int id)
{
    MXDebug::removeModule(id);
}

EXTERNC void mxd_autoNewline(bool enable)
{
    MXDebug::autoNewline(enable);
}

EXTERNC void mxd_release()
{
    MXDebug::release();
}

EXTERNC void mxd_destroy()
{
    MXDebug::destroy();
}

EXTERNC void mxd_show()
{
    MXDebug::show(NULL, 0);
}

EXTERNC void mxd_fshow(FILE *fp, int indent)
{
    MXDebug::show(fp, indent);
}

EXTERNC char *mxd_name(int id)
{
    return MXDebug::name(id);
}

EXTERNC void mxd_setName(int id, const char *name)
{
    MXDebug::setName(id, name);
}

EXTERNC int mxd_level(int id)
{
    return MXDebug::level(id);
}

EXTERNC void mxd_setLevel(int id, int level)
{
    MXDebug::setLevel(id, level);
}

EXTERNC bool mxd_testModule(int id, int level)
{
    return MXDebug::testModule(id, level);
}

EXTERNC bool mxd_nTestModule(int *id, int len, int level)
{
    return MXDebug::nTestModule(id, len, level);
}

EXTERNC int mxd_size()
{
    return MXDebug::size();
}

EXTERNC bool mxd_hasID(int id)
{
    return MXDebug::hasID(id);
}

EXTERNC void mxd_suspend(int id, bool suspend)
{
    return MXDebug::suspend(id, suspend);
}

EXTERNC void mxd_nSuspend(int *idSet, int len, bool suspend)
{
    return MXDebug::nSuspend(idSet, len, suspend);
}

EXTERNC int mxd_suspended(int id)
{
    return MXDebug::suspended(id);
}

EXTERNC mx_module_t *mxd_save(int id)
{
    return MXDebug::save(id);
}

EXTERNC void mxd_restore(int id, mx_module_t *src)
{
    return MXDebug::restore(id, src);
}

EXTERNC void mxd_dprint(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    MXDebug::vdprint(fmt, args);
    va_end(args);
}

EXTERNC void mxd_mdprint(int id, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    MXDebug::vmdprint(id, fmt, args);
    va_end(args);
}

EXTERNC void mxd_geprint(int id, int lvl, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    MXDebug::vgeprint(id, lvl, fmt, args);
    va_end(args);
}

EXTERNC void mxd_nzprint(int id, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    MXDebug::vnzprint( id, fmt, args);
    va_end(args);
}
#endif

#ifdef __cplusplus
}
#endif

#endif


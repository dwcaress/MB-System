#include <stdio.h>
#include <stdarg.h>

// trn_log.h
//
// trn_log implements logging capability for Terrain Aided Navigation module.
//
// Features:
//		- output to log file, stdout and/or stderr
//		- configurable by module
//		- user-defined logging groups
//		- global log group
//		- replaces myOutput (selectable using macro)
//
// The primary logging method
//
//		void logs(int strmask, const char* format, ...)
//
// provides printf style logging to one or more output
// streams where
//		strmask : enum to select output streams
//		format  : printf format string
//		...     : printf variable arguments
//
// Basic use is simple: call logs as you would printf, providing
// an OR of TLStream outputs:
//
//		logs(TL_LOG|TL_SERR, "this is message[%d]",4);
//
// There strmask may also be spefied using the TL_OMASK macro.
// The TL_OMASK macro applies logging configurations that apply to
// a module (logging group):
//
//		TL_OMASK(<TLModuleID>,<default stream(s)>)
//
//	e.g.
// 		logs(TL_OMASK(TL_TRN_SERVER, TL_SERR),"this is message[%d]",5);
//
// indicates that the log message should go to stderr, unless the
// the TL_TRN_SERVER module configuration overrides that.
//
// Module logging configurations are specified using tl_mconfig
//
//		void tl_mconfig(TLModuleID id, TLStreams s_en, TLStreams s_di)
// where
//		id   : TLModuleID module ID enum value
//		s_en : TLStreams enum stream enable
//		s_di : TLStreams enum stream disable
//
// If no streams are disabled, the enabled streams are added to (OR'd with)
// the dfl argument in TL_OMASK.
// A stream constant TL_NC is provided to specify that the enable or disable
// streams configuration should not be changed.
//
// TL_OMASK settings affect all log calls from the specified module
// that use the using TL_OMASK macro to set the mask:
//
//		logs(TL_OMASK(<TLModuleID>,<default stream(s)>) , <fmt>, ...);
//
// By default, modules are configured to enable log output (only).
//
// In the event of a conflict, disable overrides enable.
// Defaults are set in trn_log.cpp
//
// Module IDs represent logical control groups; they need not
// be confined to a single module.
// Logging controls may be extended to form logical groups
// with common logging controls.
//
// To add another module/group:
//		- add a constant to TL_ModuleID enum in trn_log.h
//		- add an entry to tl_module_config[] trn_log.cpp
//
// IMPORTANT:
// The TLModuleID values are used to index the tl_module_config array;
// It's critical that new entries be appended to these lists. New TL_ModuleID
// values should be inserted before TL_N_MODULES to prevent indexing errors.
// Changes must be reflected in BOTH TLModuleID and tl_module_config to
// prevent configuration errors.
//
// Line endings
// It is OK to terminate messages with or without newlines.
// Messages to the log file will be automatically terminated with newlines '\n'.
// If a message already ends in newline, no newline is added.
// Where newlines are included, messages in general should terminate with a
// single newline. Messages should use Unix line ends '\n' rather than DOS
// line ends "\r\n".
//
// Ring Buffer
//
// The trn_server log file name is supplied by the client. It is possible that
// logging calls may be made before the log file exists. Until the file
// is created, trn_log writes to stderr and also to a ring buffer.
// When the log file is created, the content of the ring buffer is written
// to the beginning of the log file. The ring buffer is not used after the
// log file is created.
// If the ring buffer fills before the log file is created, a message is sent
// to stderr and the ring buffer is overwritten.
// The ring buffer size may be configured using
// TL_RING_BYTES in trn_log.h.

// Examples:
//
// tl_mconfig
// To enable console output for a module, use
//
//      tl_mconfig(<TLModuleID>,TL_SERR,TL_NC)
// or
//      tl_mconfig(<TLModuleID>,TL_SOUT,TL_NC)
//
// To enable console output for a module and disable logging, use
//
//      tl_mconfig(<TLModuleID>,TL_SERR,TL_SOUT|TL_LOG)
// or
//      tl_mconfig(<TLModuleID>,TL_SOUT,TL_SERR|TL_LOG)
//
// logs
//	Send output ONLY to module level enabled streams:
//	(unless module enabled = module disabled):
//
// 		logs(TL_OMASK(<TLModuleID>,S_NONE),"foo");
//
//	Send output to logfile AND module level enabled streams
//	(unless log output is disabled):
//
// 		logs(TL_OMASK(<TLModuleID>,S_LOG),"foo");
//
// See also test_trn_log.cpp

#ifndef TRN_LOG_H
#define TRN_LOG_H

// ring buffer size for
// messages logged before
// the log file is created
#define TL_RING_BYTES 4096

// Enum for different log message streams:
// TL_NONE  : don't log
// TL_SOUT  : stdout (buffered)
// TL_SERR  : stderr (unbuffered)
// TL_LOG   : trn log file
// TL_BOTH  : trn log file and stderr
// TL_BOTHB : trn log file and stdout
typedef enum{
    TL_NC=0xFF,
    TL_NONE=0x00,
    TL_SOUT=0x01,
    TL_SERR=0x02,
    TL_LOG=0x04,
    TL_BOTH=0x06,
    TL_BOTHB=0x05,
    TL_ALL=0x07
} TLStreams;

// Global module IDs
// !!! IMPORTANT !!!
// Global module logging settings are indexed using TLModuleID:
// Any changes must be reflected in BOTH
// - tl_modules (in trn_log.c)
// - TLModuleID (in trn_log.h)
typedef enum{
    TL_TRN_SERVER,
    TL_STRUCT_DEFS,
    TL_TERRAIN_NAV,
    TL_TERRAIN_NAV_AID_LOG,
    TL_TNAV_CONFIG,
    TL_TNAV_PARTICLE_FILTER,
    TL_TNAV_FILTER,
    TL_TNAV_POINT_MASS_FILTER,
    TL_TNAV_SIGMA_POINT_FILTER,
    TL_TNAV_EXT_KALMAN_FILTER,
    TL_MATRIX_ARRAY_CALCS,
    TL_TERRAIN_MAP,
    TL_TERRAIN_MAP_DEM,
    TL_TEST_TRN_LOG,
    TL_TNAV_BANK_FILTER,
    TL_N_MODULES
}TLModuleID;

struct TLModule_s{
    TLStreams g_en;
    TLStreams g_di;
};
typedef struct TLModule_s TLModule;

extern TLModule *tl_modules;


#define TL_OMASK(mid,dfl) ( (dfl | tl_modules[mid].g_en) & (~tl_modules[mid].g_di))
//#define TL_OMASK(mid,dfl) ( (dfl | tl_modules[mid].g_en) & (~tl_modules[mid].g_di))

// The LOGM macro is provided for compatibility
// w/ QNX (e.g. structDefs.cpp functions are called in QNX).
// Note that QNX 4.25 does not support variadic macros.
// LOGM may be defined to configure logging as needed for non-QNX contexts.
// It's recommended for use only where there are cross-platfrom dependencies.
#ifdef _QNX
// Included for compatibility w/ QNX
// Calls myOutput function output (which writes to stdout)
#define LOGM output
#else
// configure as needed for non-QNX
#define LOGM(fmt, ...) logs(TL_OMASK(TL_BOTH,TL_LOG),fmt, ##__VA_ARGS__ )
#endif

// logs sends output to one or more streams
// specified by ORing TLStreams enum values
void logs(int strmask, const char* format, ...) ;

// create a new log file (should only be called by trn_server)
void tl_new_logfile(const char* directory);

void tl_mconfig(TLModuleID id, TLStreams s_en, TLStreams s_di);
void tl_release();

#endif

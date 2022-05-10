//////////////////////////////////////////////////////////////////////////////
// Copyright 2021  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef PCF_LOG_HPP  // include guard
#define PCF_LOG_HPP

#include <mutex>
#include <string>
#include <sstream>
#include <iostream>
#include <functional>
#include "pcf_utils.hpp"

namespace pcf
{
    // Class: log_level
    // Logging levels for the pcf::log object
    enum class log_level
    {
        // Enum: log_level
        //
        // pcf::log_level::OFF - no messages logged
        // pcf::log_level::DEBUG - all messages logged
        // pcf::log_level::INFO - all messages logged except DEBUG
        // pcf::log_level::WARN - all messages logged except DEBUG and INFO
        // pcf::log_level::ERR - only ERR messages logged
        OFF   = 0,
        DEBUG = 1,
        INFO  = 2,
        WARN  = 3,
        ERR   = 4
    };

    // Class: log
    // A logging class for the Precision Control Framework,
    // see <Using the pcf::log class> for usage
    class log
    {
        public:

            //////////////////////////////////////////////////////////////////
            // Constructor: log
            // create a PCF logging object
            //
            // Parameters:
            // class_name - the name of the class to be logged
            // object_name - the name of the object to be logged
            log(const std::string& class_name = "UNKNOWN_CLASS",
                const std::string& object_name = "UNKNOWN_OBJECT")
            {
                mClassName = class_name;
                mObjectName = object_name;

                mObjectStatus = true;
            }

            //////////////////////////////////////////////////////////////////
            // Function: get_class_name
            // return the name of the class as a std::string
            std::string get_class_name() { return mClassName; }

            //////////////////////////////////////////////////////////////////
            // Function: set_class_name
            //
            // Parameters:
            // name - the class name
            void set_class_name(const std::string& name)
            {
                mClassName = name;
            }

            //////////////////////////////////////////////////////////////////
            // Function: get_object_name
            // return the name of the object as a std::string
            std::string get_object_name() { return mObjectName; }

            //////////////////////////////////////////////////////////////////
            // Function: set_object_name
            //
            // Parameters:
            // name - the object name
            void set_object_name(const std::string& name)
            {
                mObjectName = name;
            }

            //////////////////////////////////////////////////////////////////
            // Function: get_full_name
            // return the combined class and object name as a std::string
            std::string get_full_name()
            {
                return mClassName + "[" + mObjectName + "]";
            }

            //////////////////////////////////////////////////////////////////
            // Function: object_status_on
            // Enable object status messages to std::cout.  This can be
            // used in conjunction with object_satus_off() to temporarily
            // disable an objects status messages to std::cout regardless
            // of the objects <log_level> setting.
            void object_status_on() { mObjectStatus = true; }

            //////////////////////////////////////////////////////////////////
            // Function: object_status_off
            // Enable object status messages to std::cout.  This can be
            // used in conjunction with object_satus_on() to temporarily
            // disable an objects status messages to std::cout regardless
            // of the objects <log_level> setting.
            void object_status_off() { mObjectStatus = false; }

            //////////////////////////////////////////////////////////////////
            // Function: is_object_status_on
            // return true if show status is enabled, otherwise false
            bool is_object_status_on() { return mObjectStatus; }

            //////////////////////////////////////////////////////////////////
            // Function: get_status_level
            // return the <log_level> for status messages
            static pcf::log_level get_status_level() { return mStatusLevel; }

            //////////////////////////////////////////////////////////////////
            // Function: get_logger_level
            // return the <log_level> for logger messages
            static pcf::log_level get_logger_level() { return mLoggerLevel; }

            //////////////////////////////////////////////////////////////////
            // Function: set_status_level
            // Set the <log_level> for status messages sent to std::cout.
            // This is a global setting that applies to all threads using a
            // pcf::log oject.
            static void set_status_level(pcf::log_level l)
            {
                mStatusLevel = l;
            }

            //////////////////////////////////////////////////////////////////
            // Function: set_logger_level
            // Set the <log_level> for logger messages sent to the logger
            // callback if it is registered (see <set_logger_function>).
            // This is a global setting that applies to all threads using
            // a pcf::log oject.
            static void set_logger_level(pcf::log_level l)
            {
                mLoggerLevel = l;
            }

            //////////////////////////////////////////////////////////////////
            // Function: debug_msg
            // Log a DEBUG <log_level> message
            //
            // Parameters:
            // msg - the log message
            // nl - optional new-line flag
            void debug_msg(const std::string& msg, bool nl = true)
            {
                return status_msg(log_level::DEBUG, msg, nl);
            }

            //////////////////////////////////////////////////////////////////
            // Function: info_msg
            // Log an INFO <log_level> message
            //
            // Parameters:
            // msg - the log message
            // nl - optional new-line flag
            void info_msg(const std::string& msg, bool nl = true)
            {
                return status_msg(log_level::INFO, msg, nl);
            }

            //////////////////////////////////////////////////////////////////
            // Function: warn_msg
            // Log a WARN <log_level> message
            //
            // Parameters:
            // msg - the log message
            // nl - optional new-line flag
            void warn_msg(const std::string& msg, bool nl = true)
            {
                return status_msg(log_level::WARN, msg, nl);
            }

            //////////////////////////////////////////////////////////////////
            // Function: err_msg
            // Log an ERR <log_level> message
            //
            // Parameters:
            // msg - the log message
            // nl - optional new-line flag
            void err_msg(const std::string& msg, bool nl = true)
            {
                return status_msg(log_level::ERR, msg, nl);
            }

            //////////////////////////////////////////////////////////////////
            // Function: set_logger_function
            // Register a logging callback function with a function signature
            // of (void)(const std::string& ).  This is a global setting that
            // applies to all threads using a pcf::log object.  (note: it's the
            // programmer's responsibility to ensure the logger callback can
            // handle the volume of logged messages in a timely fashion.)
            static void set_logger_function(void (*fp)(const std::string&))
            {
                mLoggerFunc = fp;
            }

            //////////////////////////////////////////////////////////////////
            // Function: clear_logger_function
            // Deregister the logging callback function.
            static void clear_logger_function() { mLoggerFunc = nullptr; }

            // debug for testing function pointer
            // void* get_logger_func_address() { return (void*)mLoggerFunc; }

        private:

            void status_msg(log_level sl,
                            const std::string& msg,
                            bool nl = true)
            {
                if ( is_status_disabled() && is_logging_disabled() )
                    return;

                // if you've gotten here form the status message
                std::stringstream ss;

                ss << status_prefix(sl) << msg;

                // show message based on mObjectStatus and mStatusLevel
                if ( mObjectStatus && (sl >= mStatusLevel) )
                {
                    // send string to standard out
                    std::cout << ss.str();

                    // optional newline
                    if ( nl ) std::cout << std::endl;
                }

                // log message based on mLoggerFunc and mLoggerLevel
                if ( (mLoggerFunc != nullptr) && (sl >= mLoggerLevel) )
                {
                    // grab the logger mutex and log the message
                    std::lock_guard<std::mutex> lock(mLoggerMutex);
                    (*mLoggerFunc)(ss.str());
                }

                return;
            }

            std::string status_prefix(pcf::log_level sl)
            {
                std::stringstream ss;

                switch( sl )
                {
                    case log_level::DEBUG:  ss << "DEBUG, "; break;
                    case log_level::INFO:   ss << "INFO, "; break;
                    case log_level::WARN:   ss << "WARN, "; break;
                    case log_level::ERR:    ss << "ERR, "; break;
                    case log_level::OFF:    return "";
                }

                ss << get_iso8601_timestamp() << ", ";
                ss << mClassName << "[" << mObjectName << "], ";

                return ss.str();
            }

            // global state of status reporting
            bool is_status_disabled()
            {
                return (mStatusLevel == log_level::OFF);
            }

            // global state of logging
            bool is_logging_disabled()
            {
                return ( (mLoggerFunc == nullptr) ||
                         (mLoggerLevel == log_level::OFF) );
            }

        private:

            // object id
            std::string mClassName;
            std::string mObjectName;

            // temporary disable at object level
            bool mObjectStatus;

            // global logger mutex for thread safe log writes
            static std::mutex mLoggerMutex;

            // global status/logger levels and logger function
            static pcf::log_level mStatusLevel;
            static pcf::log_level mLoggerLevel;
            static void (*mLoggerFunc)(const std::string&);
    };

//////////////////////////////////////////////////////////////////////////////
// Define: INIT_PCF_LOG
// This must be defined once and only once per process.  The macro must be
// defined before the first class or derived class from the Precision Control
// Framework is included.
#ifdef INIT_PCF_LOG
    //////////////////////////////////////////////////////////////////////////
    // declaration for static logger mutex

    std::mutex pcf::log::mLoggerMutex;

    //////////////////////////////////////////////////////////////////////////
    // initializers for static member variables

    void (*pcf::log::mLoggerFunc)(const std::string&){ nullptr };
    pcf::log_level pcf::log::mStatusLevel{ pcf::log_level::INFO };
    pcf::log_level pcf::log::mLoggerLevel{ pcf::log_level::DEBUG };
#endif

}

#endif


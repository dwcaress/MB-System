//////////////////////////////////////////////////////////////////////////////
// Copyright 2021  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef TRNROV_UTILS_HPP  // include guard
#define TRNROV_UTILS_HPP

#include <cmath>
#include <ctime>
#include <chrono>
#include <thread>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <iostream>

#include <unistd.h>

namespace pcf
{
    //////////////////////////////////////////////////////////////////
    // Function: get_timestamp
    // return an <epoch: https://en.wikipedia.org/wiki/Unix_time>
    // time-stamp in seconds
    inline double get_timestamp()
    {
        auto ts = std::chrono::system_clock::now();
        std::chrono::duration<double> epoch_time = ts.time_since_epoch();

        return epoch_time.count();
    }

    //////////////////////////////////////////////////////////////////
    // Function: timestamp_to_iso8601
    // convert an <epoch: https://en.wikipedia.org/wiki/Unix_time>
    // time-stamp to an <ISO8601: https://en.wikipedia.org/wiki/ISO_8601>
    // string
    inline std::string timestamp_to_iso8601(double ts)
    {
        char buf[64];
        std::time_t t = static_cast<time_t>(ts);
        double ipart;

        // get the milliseconds from the ts
        int ms = (int)(1000.0 * std::modf(ts, &ipart));

        // create a formatted time stamp string
        std::strftime(buf, 64, "%Y-%m-%dT%H:%M:%S", std::gmtime(&t));

        // add the milliseconds to the string
        std::stringstream ss;

        ss << std::string(buf);
        ss << "." << std::setfill('0') << std::setw(3) << ms << "Z";

        return ss.str();
    }

    //////////////////////////////////////////////////////////////////
    // Function: get_iso8601_timestamp
    // return an <epoch: https://en.wikipedia.org/wiki/Unix_time>
    // time-stamp string using the
    // <ISO8601: https://en.wikipedia.org/wiki/ISO_8601> format
    inline std::string get_iso8601_timestamp()
    {
        return timestamp_to_iso8601(get_timestamp());
    }

    //////////////////////////////////////////////////////////////////
    // Function: get_timestamp_string
    // return a file name friendly time stamp string
    inline std::string get_timestamp_string()
    {
        // grab the time
        auto now = std::chrono::system_clock::now();

        // convert the time to a time_t type
        std::time_t now_t = std::chrono::system_clock::to_time_t(now);

        // create a buffer with YYYY-MM-DD_HH.MM.SS.SSS format
        // note: not using std::put_time as it was broken until GCC 5.0,
        // best to use strftime for portability
        char ts_buff[100];
        std::strftime(ts_buff, sizeof(ts_buff), "%Y-%m-%d_%H.%M.%S",
                      std::localtime(&now_t));

        // get the fractional seconds
        auto ts = now.time_since_epoch();
        auto ts_ms = std::chrono::duration_cast<std::chrono::milliseconds>(ts);
        std::size_t fractional_seconds = ts_ms.count() % 1000;

        // create a formatted time stamp string
        std::stringstream ss;

        ss << std::string(ts_buff) << "."
           << std::setfill('0') << std::setw(3) << fractional_seconds;

        return ss.str();
    }

    //////////////////////////////////////////////////////////////////
    // Function: sleep_ms
    // suspend the calling thread for ms milliseconds
    inline void sleep_ms(long ms)
    {
        return std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    //////////////////////////////////////////////////////////////////
    // Function: get_process_id
    // return a process ID string in the following form
    // <host-name>:<process-name>(<PID>)
    inline std::string get_process_id()
    {
        int pid = -1;
        char host_name[256] = "UNKNOWN";
        std::string app_name = "UNKNOWN";

        // get pid
        pid = getpid();

        // get the host name
        gethostname(host_name, 256);

        // get program name
#if defined(_MACOS) || defined(__APPLE__)
        app_name = std::string(getprogname());
#endif

#if defined(__LINUX__) || defined(__linux__)
        // create file input stream
        std::ifstream infile;

        // open /proc/[pid]/comm (since Linux 2.6.33 - Feb 24th, 2010)
        infile.open("/proc/" + std::to_string(pid) + "/comm");

        // read the process name
        if ( infile.is_open() )
            infile >> app_name;

        // close the file
        infile.close();
#endif

#if !defined(_MACOS) && !defined(__LINUX__) && !defined(__linux__) && !defined(__APPLE__)
    #pragma GCC warning "Unknown OS type"
#endif

        // return publisher ID
        return std::string(host_name) + ":" +
               app_name + "(" + std::to_string(pid) + ")";

    }
};

#endif


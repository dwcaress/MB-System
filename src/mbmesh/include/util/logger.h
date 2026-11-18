#pragma once

/*
 * Logging for mbmesh, with the same line format as mbgrd2gltf's logger:
 *
 *   INFO 2026-10-19 11:24:03.073 main.cpp main():188 message text
 *
 * that is, level, local date and time to the millisecond, source file, function
 * and line of the call, then the message. Use the LOG_* macros so the file,
 * function and line are filled in automatically.
 *
 * Differences from mbgrd2gltf's logger, on purpose:
 *   - Every level goes to stderr, because mbmesh writes data, such as the
 *     --info report, to stdout.
 *   - Output is serialized with a mutex, because chunks are reconstructed on
 *     several threads at once.
 *   - Capture, for the processing log in the HTML page, keeps every message up to
 *     DEBUG whatever the console level is, so the page documents the whole run
 *     even without -V. It is capped, and says how many messages it left out.
 *
 * It is header-only, using C++17 inline variables.
 */

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace mbmesh {

enum class LogLevel { OFF, ERROR, WARN, INFO, DEBUG };

class Logger {
public:
    static void set_level(LogLevel level) {
        current_level() = level;
    }

    static bool should_log(LogLevel level) {
        return level <= current_level();
    }

    // Keep a copy of every message, at any level up to DEBUG, from now on. At most
    // maximum_messages are kept; the rest are counted and left out.
    static void start_capture(std::size_t maximum_messages = 20000) {
        std::lock_guard<std::mutex> lock(output_mutex());
        capture_state().enabled = true;
        capture_state().maximum = maximum_messages;
        capture_state().messages.clear();
        capture_state().omitted = 0;
    }

    static std::vector<std::string> captured_messages() {
        std::lock_guard<std::mutex> lock(output_mutex());
        return capture_state().messages;
    }

    // How many messages did not fit under the cap.
    static std::size_t captured_omitted() {
        std::lock_guard<std::mutex> lock(output_mutex());
        return capture_state().omitted;
    }

    template <typename... Args>
    static void log(LogLevel level, const char *file, const char *func, int line, Args &&...args) {
        const bool to_console = should_log(level);
        bool to_capture = false;
        {
            std::lock_guard<std::mutex> lock(output_mutex());
            to_capture = capture_state().enabled && level != LogLevel::OFF;
        }
        if (!to_console && !to_capture) {
            return;
        }

        std::ostringstream message;
        build_message(message, std::forward<Args>(args)...);

        std::ostringstream full;
        full << level_to_string(level) << ' ' << timestamp() << ' ' << extract_filename(file) << ' ' << func << "():" << line
             << ' ' << message.str();

        // One lock and one write per message, so lines from different threads
        // never interleave.
        std::lock_guard<std::mutex> lock(output_mutex());
        if (to_console) {
            std::cerr << full.str() << std::endl;
        }
        if (to_capture && capture_state().enabled) {
            if (capture_state().messages.size() < capture_state().maximum) {
                capture_state().messages.push_back(full.str());
            } else {
                capture_state().omitted++;
            }
        }
    }

    template <typename... Args>
    static void debug(const char *file, const char *func, int line, Args &&...args) {
        log(LogLevel::DEBUG, file, func, line, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void info(const char *file, const char *func, int line, Args &&...args) {
        log(LogLevel::INFO, file, func, line, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void warn(const char *file, const char *func, int line, Args &&...args) {
        log(LogLevel::WARN, file, func, line, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void error(const char *file, const char *func, int line, Args &&...args) {
        log(LogLevel::ERROR, file, func, line, std::forward<Args>(args)...);
    }

private:
    static LogLevel &current_level() {
        static LogLevel level = LogLevel::OFF;
        return level;
    }

    struct CaptureState {
        bool enabled = false;
        std::size_t maximum = 0;
        std::size_t omitted = 0;
        std::vector<std::string> messages;
    };

    static CaptureState &capture_state() {
        static CaptureState state;
        return state;
    }

    static std::mutex &output_mutex() {
        static std::mutex mutex;
        return mutex;
    }

    static const char *level_to_string(LogLevel level) {
        switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARN:
            return "WARN";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "OFF";
        }
    }

    static std::string timestamp() {
        const auto now = std::chrono::system_clock::now();
        const std::time_t time = std::chrono::system_clock::to_time_t(now);
        const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::tm local_time;
#ifdef _WIN32
        localtime_s(&local_time, &time);
#else
        localtime_r(&time, &local_time);
#endif

        std::ostringstream stream;
        stream << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << milliseconds.count();
        return stream.str();
    }

    static std::string extract_filename(const char *path) {
        const std::string text(path);
        const std::size_t position = text.find_last_of("/\\");
        return position == std::string::npos ? text : text.substr(position + 1);
    }

    static void build_message(std::ostringstream &) {}

    template <typename T, typename... Args>
    static void build_message(std::ostringstream &stream, T &&first, Args &&...rest) {
        stream << first;
        if constexpr (sizeof...(rest) > 0) {
            stream << ' ';
            build_message(stream, std::forward<Args>(rest)...);
        }
    }
};

// Capture the file, function and line of the call site automatically.
#define LOG_DEBUG(...) ::mbmesh::Logger::debug(__FILE__, __func__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...) ::mbmesh::Logger::info(__FILE__, __func__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) ::mbmesh::Logger::warn(__FILE__, __func__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) ::mbmesh::Logger::error(__FILE__, __func__, __LINE__, __VA_ARGS__)

} // namespace mbmesh

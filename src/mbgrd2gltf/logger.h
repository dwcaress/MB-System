/*--------------------------------------------------------------------
 *    The MB-system:	logger.h	1/5/2026
 *
 *    Copyright (c) 2026 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <unistd.h>

namespace mbgrd2gltf {

enum class LogLevel {
  OFF,
  ERROR,
  WARN,
  INFO,
  DEBUG
};

class Logger {
private:
  static LogLevel current_level;

  static const char* level_to_string(LogLevel level) {
    switch (level) {
      case LogLevel::DEBUG: return "DEBUG";
      case LogLevel::INFO: return "INFO";
      case LogLevel::WARN: return "WARN";
      case LogLevel::ERROR: return "ERROR";
      default: return "OFF";
    }
  }

  static std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()) % 1000;
    
    std::tm tm_buf;
    localtime_r(&time, &tm_buf);
    
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
  }

  static std::string extract_filename(const char* path) {
    std::string p(path);
    size_t pos = p.find_last_of("/\\");
    return (pos == std::string::npos) ? p : p.substr(pos + 1);
  }

public:
  static void set_level(LogLevel level) {
    current_level = level;
  }

  static bool should_log(LogLevel level) {
    return level <= current_level;
  }

  template<typename... Args>
  static void log(LogLevel level, const char* file, const char* func, int line, Args&&... args) {
    if (!should_log(level)) return;

    std::ostringstream msg;
    build_message(msg, std::forward<Args>(args)...);

    std::ostream& out = (level == LogLevel::INFO) ? std::cout : std::cerr;
    out << level_to_string(level) << " "
        << get_timestamp() << " "
        << extract_filename(file) << " "
        << func << "():" << line << " "
        << "[" << getpid() << "] "
        << msg.str() << std::endl;
  }

  template<typename... Args>
  static void debug(const char* file, const char* func, int line, Args&&... args) {
    log(LogLevel::DEBUG, file, func, line, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void info(const char* file, const char* func, int line, Args&&... args) {
    log(LogLevel::INFO, file, func, line, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void warn(const char* file, const char* func, int line, Args&&... args) {
    log(LogLevel::WARN, file, func, line, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void error(const char* file, const char* func, int line, Args&&... args) {
    log(LogLevel::ERROR, file, func, line, std::forward<Args>(args)...);
  }

private:
  static void build_message(std::ostringstream&) {}

  template<typename T, typename... Args>
  static void build_message(std::ostringstream& oss, T&& first, Args&&... rest) {
    oss << first;
    if constexpr (sizeof...(rest) > 0) {
      oss << " ";
      build_message(oss, std::forward<Args>(rest)...);
    }
  }
};

// Convenience macros to automatically capture file, function, and line
#define LOG_DEBUG(...) Logger::debug(__FILE__, __func__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...) Logger::info(__FILE__, __func__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) Logger::warn(__FILE__, __func__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) Logger::error(__FILE__, __func__, __LINE__, __VA_ARGS__)

} // namespace mbgrd2gltf

#endif // LOGGER_H

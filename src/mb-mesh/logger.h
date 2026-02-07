/*--------------------------------------------------------------------
 *    The MB-system:  logger.h  2/6/2026
 *
 *    Copyright (c) 2026 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

#ifndef MB_MESH_LOGGER_H
#define MB_MESH_LOGGER_H

#include <string>
#include <iostream>

namespace mbmesh {

enum class LogLevel {
  DEBUG,
  INFO,
  WARNING,
  ERROR
};

class Logger {
public:
  static void set_level(LogLevel level) {
    current_level_ = level;
  }

  static void log(LogLevel level, const std::string& message) {
    if (level >= current_level_) {
      std::cout << get_level_string(level) << ": " << message << std::endl;
    }
  }

private:
  static LogLevel current_level_;

  static std::string get_level_string(LogLevel level) {
    switch (level) {
      case LogLevel::DEBUG: return "DEBUG";
      case LogLevel::INFO: return "INFO";
      case LogLevel::WARNING: return "WARNING";
      case LogLevel::ERROR: return "ERROR";
      default: return "UNKNOWN";
    }
  }
};

}  // namespace mbmesh

#define LOG_DEBUG(msg) mbmesh::Logger::log(mbmesh::LogLevel::DEBUG, msg)
#define LOG_INFO(msg) mbmesh::Logger::log(mbmesh::LogLevel::INFO, msg)
#define LOG_WARNING(msg) mbmesh::Logger::log(mbmesh::LogLevel::WARNING, msg)
#define LOG_ERROR(msg) mbmesh::Logger::log(mbmesh::LogLevel::ERROR, msg)

#endif  // MB_MESH_LOGGER_H

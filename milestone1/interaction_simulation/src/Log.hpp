// Copyright 2026 Paralelinux-2
#ifndef LOG_HPP
#define LOG_HPP

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string_view>
#include <string>

/// @brief Logger for generating log files.
class Log final {
public:
  enum class Category {
    ERROR,
    INFO,
  };

  /// @brief Convert Category enum to string. 
  inline static std::string categoryToString(Log::Category c) {
    switch (c) {
      case Category::ERROR: return "ERROR";
      case Category::INFO:  return "INFO";
      default:              return "NULL";
    }
  }

  /// @brief Get the global log object.
  static Log& getInstance();

  /// @brief Open file and allow writing.
  void start(std::string_view filename = Log::DEFAULT_LOG_FILENAME);

  /// @brief Close file and disallow writing.
  void stop();

  /// @brief Write to log file.
  /// @throws std::logic_error if called before Log::start() or after
  ///   Log::stop().
  void write(Log::Category category, std::string_view content);

private:
  Log() = default;
  ~Log() = default;

  /// @brief Disable move semantics
  Log(Log&&) = delete;
  Log& operator=(Log&&) = delete;

  /// @brief Disable copy semantics
  Log(const Log&) = delete;
  Log& operator=(const Log&) = delete;

  /// @brief Helper method to calculate an available path for the file. 
  std::filesystem::path calculatePath(std::string_view filename) const;
  
  inline static std::ofstream file;
  inline static std::mutex mtx;
  inline static const char* DEFAULT_LOG_FILENAME = "interaction-simulation";
  inline static const char* DEFAULT_LOG_DIR = "logs";
  inline static const char* DEFAULT_LOG_FILE_EXTENSION = ".log";
};

#endif  // LOG_HPP

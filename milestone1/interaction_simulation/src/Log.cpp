// Copyright 2026 Paralelinux-2
#include "Log.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <stdexcept>
#include <string>

Log& Log::getInstance() {
  static Log log;
  return log;
}

void Log::start(std::string_view filename) {
  std::lock_guard lock(this->mtx);
  if (this->file.is_open()) {
    this->file.close();
  }

  // Create log directory if its not yet created
  std::filesystem::create_directory(Log::DEFAULT_LOG_DIR);
  const auto path = this->calculatePath(filename);

  // Try to open the log file
  this->file.open(path, std::ios::app);
  if (!this->file) {
    throw std::runtime_error("Could not open Log file: "
      + std::string(filename));
  }
}

std::filesystem::path Log::calculatePath(std::string_view filename) const {
  std::filesystem::path path;
  size_t iterationNumber{};
  const std::string dir = Log::DEFAULT_LOG_DIR;
  const std::string fileExtension = Log::DEFAULT_LOG_FILE_EXTENSION;
  do {
    // Looks like this: logs/filename-#.log
    path = dir + "/" + filename.data() + '-' + std::to_string(iterationNumber++)
      + fileExtension;
  } while (std::filesystem::exists(path));
  return path;
}

void Log::stop() {
  std::lock_guard lock(this->mtx);
  if (this->file.is_open()) {
    this->file.close();
  }
}

void Log::write(Log::Category category, std::string_view content) {
  std::lock_guard lock(this->mtx);
  if (!this->file.is_open()) {
    throw std::logic_error("Cannot write into an unopened file.");
  }
  const auto now = std::chrono::system_clock::now();
  const std::time_t timestamp = std::chrono::system_clock::to_time_t(now);
  std::string formatedTime = std::ctime(&timestamp);
  formatedTime.pop_back();
  this->file << formatedTime << " - [" << Log::categoryToString(category)
    << "]: " << content << std::endl;
  // Example output: Sat mar 21 17:10:55 2026 - [INFO]: Client sent packet.
}

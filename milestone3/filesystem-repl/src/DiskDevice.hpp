// Copyright (C) 2026 Jason Montenegro-Navarro <jasonmontenegro49@gmail.com>
#pragma once
#include <cstdint>
#include <expected>
#include <fstream>
#include <string>

#include "EzTypes.hpp"

namespace ez {

/// @brief Disk file holder which reads and writes to and from it
class DiskDevice {
 public:
  DiskDevice() = default;
  ~DiskDevice();

  DiskDevice(const DiskDevice&) = delete;
  DiskDevice& operator=(const DiskDevice&) = delete;
  DiskDevice(DiskDevice&&) noexcept = default;
  DiskDevice& operator=(DiskDevice&&) noexcept = default;

  std::expected<void, EzError> open(const std::string& path);
  std::expected<void, EzError> create(const std::string& path,
                                      uint32_t totalBlocks);
  void close();
  bool isOpen();
  /// @brief Reads exactly blocks of 256 bytes
  std::expected<void, EzError> readBlock(uint32_t index, void* buffer);
  /// @brief Writes exactly blocks of 256 bytes
  std::expected<void, EzError> writeBlock(uint32_t index, const void* buffer);

  /// @brief Reads and constructs object T directly from disk
  template <typename T>
  std::expected<T, EzError> readStruct(uint32_t index) {
    static_assert(sizeof(T) == ez::BLOCK_SIZE,
                  "T must be exactly BLOCK_SIZE bytes");
    T obj{};
    const auto result = this->readBlock(index, &obj);
    if (!result.has_value()) {
      return std::unexpected(result.error());
    }
    return {obj};
  }
  /// @brief Writes object T directly to disk
  template <typename T>
  std::expected<void, EzError> writeStruct(uint32_t index, const T& obj) {
    static_assert(sizeof(T) == ez::BLOCK_SIZE,
                  "T must be exactly BLOCK_SIZE bytes");
    return this->writeBlock(index, &obj);
  }

 private:
  std::fstream file;
  std::string diskPath;
};

}  // namespace ez

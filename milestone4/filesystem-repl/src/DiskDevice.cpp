// Copyright (C) 2026 Jason Montenegro-Navarro <jasonmontenegro49@gmail.com>
#include "DiskDevice.hpp"

#include <cstdint>
#include <expected>
#include <fstream>
#include <string>

#include "EzTypes.hpp"

namespace ez {

DiskDevice::~DiskDevice() { this->close(); }

std::expected<void, EzError> DiskDevice::open(const std::string& path) {
  this->diskPath = path;
  this->file.open(path, std::ios::in | std::ios::out | std::ios::binary);
  if (!this->isOpen()) {
    return std::unexpected(EzError::IO_ERROR);
  }
  return {};
}

std::expected<void, EzError> DiskDevice::create(const std::string& path,
                                                uint32_t totalBlocks) {
  this->diskPath = path;
  this->file.open(
      path, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
  if (!this->isOpen()) {
    return std::unexpected(EzError::IO_ERROR);
  }
  this->file.seekp(totalBlocks * ez::BLOCK_SIZE - 1);
  this->file.put(0);
  if (!this->file.good()) {
    return std::unexpected(EzError::IO_ERROR);
  }
  this->file.flush();
  return {};
}

void DiskDevice::close() {
  if (this->isOpen()) {
    this->file.flush();
    this->file.close();
  }
}

bool DiskDevice::isOpen() { return this->file.is_open(); }

std::expected<void, EzError> DiskDevice::readBlock(uint32_t index,
                                                   void* buffer) {
  this->file.clear();
  if (!this->isOpen()) {
    return std::unexpected(EzError::NOT_OPEN);
  }
  this->file.seekg(static_cast<std::streamoff>(index * ez::BLOCK_SIZE));
  if (!this->file.good()) {
    return std::unexpected(EzError::IO_ERROR);
  }
  this->file.read(reinterpret_cast<char*>(buffer), ez::BLOCK_SIZE);
  if (!this->file.good()) {
    return std::unexpected(EzError::IO_ERROR);
  }
  return {};
}

std::expected<void, EzError> DiskDevice::writeBlock(uint32_t index,
                                                    const void* buffer) {
  this->file.clear();
  if (!this->isOpen()) {
    return std::unexpected(EzError::NOT_OPEN);
  }
  this->file.seekp(static_cast<std::streamoff>(index * ez::BLOCK_SIZE));
  if (!this->file.good()) {
    return std::unexpected(EzError::IO_ERROR);
  }
  this->file.write(reinterpret_cast<const char*>(buffer), ez::BLOCK_SIZE);
  if (!this->file.good()) {
    return std::unexpected(EzError::IO_ERROR);
  }
  return {};
}

}  // namespace ez

// Copyright (C) 2026 Jason Montenegro-Navarro <jasonmontenegro49@gmail.com>
#include "BitmapManager.hpp"

#include <array>
#include <cstdint>
#include <expected>

#include "DiskDevice.hpp"
#include "EzTypes.hpp"

namespace ez {

void BitmapManager::init(DiskDevice* device, uint32_t bitmapStart,
                         uint32_t bitmapBlockCount, uint32_t totalBlocks) {
  this->device = device;
  this->bitmapStart = bitmapStart;
  this->bitmapBlockCount = bitmapBlockCount;
  this->totalBlocks = totalBlocks;
}

std::expected<void, EzError> BitmapManager::set(uint32_t index, bool state) {
  // it's finding the exact spot in disk
  const uint32_t byteOffset = index / 8ul;
  const uint32_t bitOffset = index % 8ul;
  const uint32_t bitmapBlock =
      this->bitmapStart + (byteOffset / ez::BLOCK_SIZE);
  const uint32_t byteInBlock = byteOffset % ez::BLOCK_SIZE;
  std::array<uint8_t, ez::BLOCK_SIZE> buffer;
  const auto result = this->device->readBlock(bitmapBlock, buffer.data());
  if (!result.has_value()) {
    return result;
  }
  // it's setting the bit exactly in the extracted block and writing it back
  if (state) {
    buffer[byteInBlock] |= (1u << bitOffset);
  } else {
    buffer[byteInBlock] &= ~(1u << bitOffset);
  }
  const auto write = this->device->writeBlock(bitmapBlock, buffer.data());
  if (!write.has_value()) {
    return std::unexpected(write.error());
  }
  return {};
}

std::expected<bool, EzError> BitmapManager::get(uint32_t index) {
  // it's finding the exact spot in disk
  const uint32_t byteOffset = index / 8ul;
  const uint32_t bitOffset = index % 8ul;
  const uint32_t bitmapBlock =
      this->bitmapStart + (byteOffset / ez::BLOCK_SIZE);
  const uint32_t byteInBlock = byteOffset % ez::BLOCK_SIZE;
  std::array<uint8_t, ez::BLOCK_SIZE> buffer;
  const auto result = this->device->readBlock(bitmapBlock, buffer.data());
  if (!result.has_value()) {
    return std::unexpected(result.error());
  }
  // checks the stored value in the exact bit in the extracted block
  const bool used = (buffer[byteInBlock] >> bitOffset) & 1u;
  return {used};
}

std::expected<uint32_t, EzError> BitmapManager::allocateBlock(
    uint32_t startSearch) {
  // traverses the bitmap blocks looking for a spot in the bits
  for (uint32_t bmapIndex{}; bmapIndex < this->bitmapBlockCount; ++bmapIndex) {
    // read 256 byte bmap block
    std::array<uint8_t, ez::BLOCK_SIZE> bmapBlock;
    const auto result = this->device->readBlock(this->bitmapStart + bmapIndex,
                                                bmapBlock.data());
    if (!result.has_value()) {
      return std::unexpected(result.error());
    }
    const uint32_t baseBitBlock = bmapIndex * ez::BLOCK_SIZE * 8;
    // traverse block, if a bit block is not used, set to one to claim
    for (uint32_t byteIndex{}; byteIndex < ez::BLOCK_SIZE; ++byteIndex) {
      // The whole byte is used
      if (bmapBlock[byteIndex] == 0xFFu) {
        continue;
      }
      for (uint32_t bitIndex{}; bitIndex < 8; ++bitIndex) {
        if (!((bmapBlock[byteIndex] >> bitIndex) & 1u)) {
          // check block index limits
          const uint32_t bitBlockIndex =
              baseBitBlock + byteIndex * 8u + bitIndex;
          if (bitBlockIndex >= this->totalBlocks) {
            return std::unexpected(EzError::NO_SPACE);
          }
          if (bitBlockIndex < startSearch) {
            continue;
          }
          // claim bit block
          bmapBlock[byteIndex] |= (1u << bitIndex);
          const auto write = this->device->writeBlock(
              this->bitmapStart + bmapIndex, bmapBlock.data());
          if (!write.has_value()) {
            return std::unexpected(write.error());
          }
          return {bitBlockIndex};
        }
      }
    }
  }
  return std::unexpected(EzError::NO_SPACE);
}

std::expected<void, EzError> BitmapManager::freeBlock(uint32_t index) {
  return set(index, false);
}

std::expected<void, EzError> BitmapManager::setRange(uint32_t startIndex,
                                                     uint32_t count,
                                                     bool state) {
  for (uint32_t index{startIndex}; index < startIndex + count; ++index) {
    const auto result = this->set(index, state);
    if (!result.has_value()) {
      return result;
    }
  }
  return {};
}

}  // namespace ez

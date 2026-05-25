// Copyright (C) 2026 Jason Montenegro-Navarro <jasonmontenegro49@gmail.com>
#pragma once
#include <cstdint>
#include <expected>

#include "EzTypes.hpp"

namespace ez {

class DiskDevice;

/// @brief Manages the bitmap in the disk container directly
/// @note Bitmap section always starts in block 1
class BitmapManager {
 public:
  BitmapManager() = default;
  /// @brief Initializes bitmap disk structure
  void init(DiskDevice* device, uint32_t bitmapStart, uint32_t bitmapBlockCount,
            uint32_t totalBlocks);
  /// @brief Sets a bit exactly in disk
  /// @note index is a bit index
  std::expected<void, EzError> set(uint32_t index, bool state);
  /// @note startIndex is a bit index
  std::expected<void, EzError> setRange(uint32_t startIndex, uint32_t count,
                                        bool state);
  /// @brief Gets a bit exactly in disk
  /// @note index is a bit index
  std::expected<bool, EzError> get(uint32_t index);
  /// @brief Allocates a new bitmap block
  /// @note index is a bit index
  std::expected<uint32_t, EzError> allocateBlock(uint32_t startSearch = 0);
  /// @brief Frees a whole bitmap block
  /// @note index is a bit index
  std::expected<void, EzError> freeBlock(uint32_t index);

 private:
  DiskDevice* device{};
  uint32_t bitmapStart{1};
  uint32_t bitmapBlockCount{};
  uint32_t totalBlocks{};
};

}  // namespace ez

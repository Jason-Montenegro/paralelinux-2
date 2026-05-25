// Copyright (C) 2026 Jason Montenegro-Navarro <jasonmontenegro49@gmail.com>
#pragma once
#include <concepts>
#include <cstdint>
#include <expected>

#include "DiskDevice.hpp"
#include "EzTypes.hpp"

namespace ez {
class BitmapManager;
}

namespace ez {

struct BlockSlot {
  uint32_t& ref;            // system wide relative block index
  uint32_t fileBlockIndex;  // inode relative block index
};

class InodeManager {
 public:
  InodeManager() = default;
  void init(DiskDevice* device, BitmapManager* bmap, uint32_t inodeTableStart,
            uint32_t totalInodes, uint32_t dataStart);
  std::expected<Inode, EzError> read(uint32_t inodeId);
  std::expected<void, EzError> write(const Inode& inode);
  std::expected<uint32_t, EzError> nextInodeId();
  std::expected<Inode, EzError> create(InodeType type);
  std::expected<void, EzError> free(uint32_t inodeId);
  /// @brief Returns a data block index for a newly create data block
  std::expected<uint32_t, EzError> allocateDataBlock();
  /// @brief Frees a system wide block index
  std::expected<void, EzError> freeDataBlock(uint32_t blockIndex);
  /// @brief For an inode relative block index, returns the system wide block
  ///        index
  std::expected<uint32_t, EzError> resolveDataBlock(Inode& inode,
                                                    uint32_t blockIndex);
  /// @brief For an inode, it frees all its containing system wide block indexes
  std::expected<void, EzError> freeAllDataBlocks(Inode& inode);
  /// @brief For a system wide block index, it allocates into inode and returns
  ///        the relative index inside the inode containers
  std::expected<uint32_t, EzError> allocateDataBlockAt(Inode& inode,
                                                       uint32_t blockIndex);

 private:
  template <std::invocable<BlockSlot> Visitor>
  std::expected<bool, EzError> walkInodeBlocks(Inode& inode, Visitor visitor);
  std::expected<uint32_t, EzError> allocateIndirectWith(uint32_t firstPtr);
  std::expected<bool, EzError> appendToIndirect(uint32_t indirectBlock,
                                                uint32_t newPtr);
  std::expected<uint32_t, EzError> allocateInSingleIndirect(Inode& inode,
                                                            uint32_t data);
  std::expected<uint32_t, EzError> allocateInDoubleIndirect(Inode& inode,
                                                            uint32_t data);
  std::expected<uint32_t, EzError> allocateInTripleIndirect(Inode& inode,
                                                            uint32_t data);

 private:
  DiskDevice* device{};
  BitmapManager* bmap{};
  uint32_t inodeTableStart{};
  uint32_t totalInodes{};
  uint32_t dataStart{};
};

/// @brief Traverses the inode blocks to perform a visitor action on the data
///        chunk
/// @note On visitor function, returning true will keepWalking and false will
///       stop it from walking the chunk blocks
template <std::invocable<BlockSlot> Visitor>
std::expected<bool, EzError> InodeManager::walkInodeBlocks(Inode& inode,
                                                           Visitor visitor) {
  uint32_t fileBlockIndex{};
  bool keepWalking = true;  // this is for early exiting
  // operates on the first indirection block level
  const auto walkL1 = [&](uint32_t l1Block) -> std::expected<void, EzError> {
    const auto l1 = this->device->readStruct<IndirectBlock>(l1Block);
    if (!l1.has_value()) {
      return std::unexpected(l1.error());
    }

    for (auto ptr : l1->ptrs) {
      if (ptr == ez::NULL_BLOCK) {
        return {};
      }
      keepWalking = visitor(
          ez::BlockSlot{.ref = ptr, .fileBlockIndex = fileBlockIndex++});
      if (!keepWalking) {
        return {};
      }
    }
    return {};
  };
  // operates on the second indirection block level
  const auto walkL2 = [&](uint32_t l2Block) -> std::expected<void, EzError> {
    const auto l2 = this->device->readStruct<IndirectBlock>(l2Block);
    if (!l2.has_value()) {
      return std::unexpected(l2.error());
    }

    for (auto ptr : l2->ptrs) {
      if (ptr == ez::NULL_BLOCK) {
        return {};
      }
      const auto result = walkL1(ptr);
      if (!result.has_value()) {
        return std::unexpected(result.error());
      }
      if (!keepWalking) {
        return {};
      }
    }
    return {};
  };
  // operates on the third indirection block level
  const auto walkL3 = [&](uint32_t l3Block) -> std::expected<void, EzError> {
    const auto l3 = this->device->readStruct<IndirectBlock>(l3Block);
    if (!l3.has_value()) {
      return std::unexpected(l3.error());
    }

    for (auto ptr : l3->ptrs) {
      if (ptr == ez::NULL_BLOCK) {
        return {};
      }
      const auto result = walkL2(ptr);
      if (!result.has_value()) {
        return std::unexpected(result.error());
      }
      if (!keepWalking) {
        return {};
      }
    }
    return {};
  };

  // applying visitor function to direct blocks
  for (uint32_t index{}; index < ez::INODE_BLOCK_CONTAINER_SIZE; ++index) {
    if (inode.directBlocks[index] == ez::NULL_BLOCK) {
      return {true};
    }
    keepWalking = visitor(BlockSlot{.ref = inode.directBlocks[index],
                                    .fileBlockIndex = fileBlockIndex++});
    if (!keepWalking) {
      return {false};
    }
  }
  // applying visitor function to indirect blocks
  for (uint32_t index{}; index < ez::INODE_BLOCK_CONTAINER_SIZE; ++index) {
    if (inode.indirectBlocks[index] == ez::NULL_BLOCK) {
      return {true};
    }
    const auto result = walkL1(inode.indirectBlocks[index]);
    if (!result.has_value()) {
      return std::unexpected(result.error());
    }
    if (!keepWalking) {
      return {false};
    }
  }
  // applying visitor function to double indirect blocks
  for (uint32_t index{}; index < ez::INODE_BLOCK_CONTAINER_SIZE; ++index) {
    if (inode.doubleIndirectBlocks[index] == ez::NULL_BLOCK) {
      return {true};
    }
    const auto result = walkL2(inode.doubleIndirectBlocks[index]);
    if (!result.has_value()) {
      return std::unexpected(result.error());
    }
    if (!keepWalking) {
      return {false};
    }
  }
  // applying visitor function to triple indirect blocks
  for (uint32_t index{}; index < ez::INODE_BLOCK_CONTAINER_SIZE; ++index) {
    if (inode.tripleIndirectBlocks[index] == ez::NULL_BLOCK) {
      return {true};
    }
    const auto result = walkL3(inode.tripleIndirectBlocks[index]);
    if (!result.has_value()) {
      return std::unexpected(result.error());
    }
    if (!keepWalking) {
      return {false};
    }
  }

  return {true};
}

}  // namespace ez

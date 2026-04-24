// Copyright (C) 2026 Jason Montenegro-Navarro <jasonmontenegro49@gmail.com>
#include "InodeManager.hpp"

#include <array>
#include <cstdint>
#include <ctime>
#include <expected>

#include "BitmapManager.hpp"
#include "DiskDevice.hpp"
#include "EzTypes.hpp"

namespace ez {

void InodeManager::init(DiskDevice* device, BitmapManager* bmap,
                        uint32_t inodeTableStart, uint32_t totalInodes,
                        uint32_t dataStart) {
  this->device = device;
  this->bmap = bmap;
  this->inodeTableStart = inodeTableStart;
  this->totalInodes = totalInodes;
  this->dataStart = dataStart;
}

std::expected<Inode, EzError> InodeManager::read(uint32_t inodeId) {
  if (inodeId >= this->totalInodes) {
    return std::unexpected(EzError::NOT_FOUND);
  }
  return this->device->readStruct<Inode>(this->inodeTableStart + inodeId);
}

std::expected<void, EzError> InodeManager::write(const Inode& inode) {
  if (inode.inodeId >= this->totalInodes) {
    return std::unexpected(EzError::INVALID_ARGUMENT);
  }
  return this->device->writeStruct<Inode>(this->inodeTableStart + inode.inodeId,
                                          inode);
}

std::expected<uint32_t, EzError> InodeManager::nextInodeId() {
  for (uint32_t id{}; id < this->totalInodes; ++id) {
    const auto result =
        this->device->readStruct<Inode>(this->inodeTableStart + id);
    if (!result.has_value()) {
      return std::unexpected(result.error());
    }
    if (result->inodeType == InodeType::FREE) {
      return {id};
    }
  }
  return std::unexpected(EzError::NO_INODES);
}

std::expected<Inode, EzError> InodeManager::create(InodeType type) {
  const auto nextId = this->nextInodeId();
  if (!nextId.has_value()) {
    return std::unexpected(nextId.error());
  }

  Inode inode{.inodeType = type,
              .inodeId = nextId.value(),
              .totalSize = 0,
              .createdTimestamp = static_cast<uint32_t>(std::time(nullptr))};

  const auto write =
      this->device->writeStruct(this->inodeTableStart + inode.inodeId, inode);
  if (!write.has_value()) {
    return std::unexpected(write.error());
  }
  const auto bmapSet =
      this->bmap->set(this->inodeTableStart + inode.inodeId, true);
  if (!bmapSet.has_value()) {
    return std::unexpected(bmapSet.error());
  }
  return {inode};
}

std::expected<void, EzError> InodeManager::free(uint32_t inodeId) {
  Inode empty{};
  const auto write =
      this->device->writeStruct(this->inodeTableStart + inodeId, empty);
  if (!write.has_value()) {
    return write;
  }
  return this->bmap->set(this->inodeTableStart + inodeId, false);
}

std::expected<uint32_t, EzError> InodeManager::allocateDataBlock() {
  const auto blockIndex = this->bmap->allocateBlock(this->dataStart);
  if (!blockIndex.has_value()) {
    return blockIndex;
  }
  std::array<uint8_t, ez::BLOCK_SIZE> empty{};
  const auto write = this->device->writeBlock(blockIndex.value(), empty.data());
  if (!write.has_value()) {
    this->bmap->freeBlock(blockIndex.value());
    return std::unexpected(write.error());
  }
  return blockIndex;
}

std::expected<void, EzError> InodeManager::freeDataBlock(uint32_t blockIndex) {
  return this->bmap->freeBlock(blockIndex);
}

std::expected<uint32_t, EzError> InodeManager::resolveDataBlock(
    Inode& inode, uint32_t blockIndex) {
  uint32_t foundBlock = ez::NULL_BLOCK;

  const auto result = this->walkInodeBlocks(inode, [&](BlockSlot slot) {
    if (slot.fileBlockIndex != blockIndex) {
      return true;
    }
    foundBlock = slot.ref;
    return false;
  });

  if (!result.has_value()) {
    return std::unexpected(result.error());
  }
  return {foundBlock};
}

std::expected<void, EzError> InodeManager::freeAllDataBlocks(Inode& inode) {
  const auto result = this->walkInodeBlocks(inode, [&](BlockSlot slot) {
    this->bmap->freeBlock(slot.ref);
    return true;
  });

  if (!result.has_value()) {
    return std::unexpected(result.error());
  }
  for (size_t index{}; index < 4 * 15; ++index) {
    if (index < 15) {
      inode.directBlocks[index % 15] = ez::NULL_BLOCK;
    } else if (index < 30) {
      inode.indirectBlocks[index % 15] = ez::NULL_BLOCK;
    } else if (index < 45) {
      inode.doubleIndirectBlocks[index % 15] = ez::NULL_BLOCK;
    } else {
      inode.tripleIndirectBlocks[index % 15] = ez::NULL_BLOCK;
    }
  }
  inode.totalSize = 0;
  return {};
}

std::expected<bool, EzError> InodeManager::appendToIndirect(
    uint32_t indirectBlock, uint32_t newPtr) {
  const auto read = this->device->readStruct<IndirectBlock>(indirectBlock);
  if (!read.has_value()) {
    return std::unexpected(read.error());
  }

  IndirectBlock block = read.value();
  for (uint32_t slotIndex{}; slotIndex < PTRS_PER_INODE_INDEX_BLOCK;
       ++slotIndex) {
    if (block.ptrs[slotIndex] != NULL_BLOCK) {
      continue;
    }
    block.ptrs[slotIndex] = newPtr;
    const auto write = this->device->writeStruct(indirectBlock, block);
    if (!write.has_value()) {
      return std::unexpected(write.error());
    }
    return {true};
  }
  return {false};
}

std::expected<uint32_t, EzError> InodeManager::allocateIndirectWith(
    uint32_t firstPtr) {
  const auto blockId = this->allocateDataBlock();
  if (!blockId.has_value()) {
    return std::unexpected(blockId.error());
  }

  IndirectBlock block{};
  block.ptrs[0] = firstPtr;
  const auto write = this->device->writeStruct(blockId.value(), block);
  if (!write.has_value()) {
    this->freeDataBlock(blockId.value());
    return std::unexpected(write.error());
  }
  return blockId;
}

std::expected<uint32_t, EzError> InodeManager::allocateInSingleIndirect(
    Inode& inode, uint32_t data) {
  for (uint32_t inodeSlot{}; inodeSlot < INODE_BLOCK_CONTAINER_SIZE;
       ++inodeSlot) {
    // creates the indirection block up to first layer
    if (inode.indirectBlocks[inodeSlot] == NULL_BLOCK) {
      const auto l1 = this->allocateIndirectWith(data);
      if (!l1.has_value()) {
        return std::unexpected(l1.error());
      }
      inode.indirectBlocks[inodeSlot] = l1.value();
      return {data};
    }
    const auto appended =
        this->appendToIndirect(inode.indirectBlocks[inodeSlot], data);
    if (!appended.has_value()) {
      return std::unexpected(appended.error());
    }
    if (appended.value()) {
      return {data};
    }
  }
  return std::unexpected(EzError::NO_SPACE);
}

std::expected<uint32_t, EzError> InodeManager::allocateInDoubleIndirect(
    Inode& inode, uint32_t data) {
  for (uint32_t inodeSlot{}; inodeSlot < INODE_BLOCK_CONTAINER_SIZE;
       ++inodeSlot) {
    // creates the indirection blocks up to second layer
    if (inode.doubleIndirectBlocks[inodeSlot] == NULL_BLOCK) {
      const auto l1 = this->allocateIndirectWith(data);
      if (!l1.has_value()) {
        return std::unexpected(l1.error());
      }
      const auto l2 = this->allocateIndirectWith(l1.value());
      if (!l2.has_value()) {
        this->freeDataBlock(l1.value());
        return std::unexpected(l2.error());
      }
      inode.doubleIndirectBlocks[inodeSlot] = l2.value();
      return {data};
    }
    const auto l2Read = this->device->readStruct<IndirectBlock>(
        inode.doubleIndirectBlocks[inodeSlot]);
    if (!l2Read.has_value()) {
      return std::unexpected(l2Read.error());
    }
    // append to existing l2 block
    IndirectBlock l2 = l2Read.value();
    for (uint32_t l2Slot{}; l2Slot < PTRS_PER_INODE_INDEX_BLOCK; ++l2Slot) {
      if (l2.ptrs[l2Slot] == NULL_BLOCK) {
        const auto l1 = this->allocateIndirectWith(data);
        if (!l1.has_value()) {
          return std::unexpected(l1.error());
        }
        l2.ptrs[l2Slot] = l1.value();
        const auto write = this->device->writeStruct(
            inode.doubleIndirectBlocks[inodeSlot], l2);
        if (!write.has_value()) {
          this->freeDataBlock(l1.value());
          return std::unexpected(write.error());
        }
        return {data};
      }
      const auto appended = this->appendToIndirect(l2.ptrs[l2Slot], data);
      if (!appended.has_value()) {
        return std::unexpected(appended.error());
      }
      if (appended.value()) {
        return {data};
      }
    }
  }
  return std::unexpected(EzError::NO_SPACE);
}

std::expected<uint32_t, EzError> InodeManager::allocateInTripleIndirect(
    Inode& inode, uint32_t data) {
  for (uint32_t inodeSlot{}; inodeSlot < INODE_BLOCK_CONTAINER_SIZE;
       ++inodeSlot) {
    // creates the indirection blocks for all layers
    if (inode.tripleIndirectBlocks[inodeSlot] == NULL_BLOCK) {
      const auto l1 = this->allocateIndirectWith(data);
      if (!l1.has_value()) {
        return std::unexpected(l1.error());
      }
      const auto l2 = this->allocateIndirectWith(l1.value());
      if (!l2.has_value()) {
        this->freeDataBlock(l1.value());
        return std::unexpected(l2.error());
      }
      const auto l3 = this->allocateIndirectWith(l2.value());
      if (!l3.has_value()) {
        this->freeDataBlock(l2.value());
        this->freeDataBlock(l1.value());
        return std::unexpected(l3.error());
      }
      inode.tripleIndirectBlocks[inodeSlot] = l3.value();
      return {data};
    }
    const auto l3Read = this->device->readStruct<IndirectBlock>(
        inode.tripleIndirectBlocks[inodeSlot]);
    if (!l3Read.has_value()) {
      return std::unexpected(l3Read.error());
    }
    // append to l3 block
    IndirectBlock l3 = l3Read.value();
    for (uint32_t l3Slot{}; l3Slot < PTRS_PER_INODE_INDEX_BLOCK; ++l3Slot) {
      if (l3.ptrs[l3Slot] == NULL_BLOCK) {
        const auto l1 = this->allocateIndirectWith(data);
        if (!l1.has_value()) {
          return std::unexpected(l1.error());
        }
        const auto l2 = this->allocateIndirectWith(l1.value());
        if (!l2.has_value()) {
          this->freeDataBlock(l1.value());
          return std::unexpected(l2.error());
        }
        l3.ptrs[l3Slot] = l2.value();
        const auto write = this->device->writeStruct(
            inode.tripleIndirectBlocks[inodeSlot], l3);
        if (!write.has_value()) {
          this->freeDataBlock(l2.value());
          this->freeDataBlock(l1.value());
          return std::unexpected(write.error());
        }
        return {data};
      }
      const auto l2Read =
          this->device->readStruct<IndirectBlock>(l3.ptrs[l3Slot]);
      if (!l2Read.has_value()) {
        return std::unexpected(l2Read.error());
      }
      // append to l2 block
      IndirectBlock l2 = l2Read.value();
      for (uint32_t l2Slot{}; l2Slot < PTRS_PER_INODE_INDEX_BLOCK; ++l2Slot) {
        if (l2.ptrs[l2Slot] == NULL_BLOCK) {
          const auto l1 = this->allocateIndirectWith(data);
          if (!l1.has_value()) {
            return std::unexpected(l1.error());
          }
          l2.ptrs[l2Slot] = l1.value();
          const auto write = this->device->writeStruct(l3.ptrs[l3Slot], l2);
          if (!write.has_value()) {
            this->freeDataBlock(l1.value());
            return std::unexpected(write.error());
          }
          return {data};
        }
        const auto appended = this->appendToIndirect(l2.ptrs[l2Slot], data);
        if (!appended.has_value()) {
          return std::unexpected(appended.error());
        }
        if (appended.value()) {
          return {data};
        }
      }
    }
  }
  return std::unexpected(EzError::NO_SPACE);
}

std::expected<uint32_t, EzError> InodeManager::allocateDataBlockAt(
    Inode& inode, uint32_t blockIndex) {
  const uint32_t currentBlockCount = inode.totalSize / ez::BLOCK_SIZE;
  if (currentBlockCount >= ez::MAX_INODE_BLOCKS) {
    return std::unexpected(EzError::NO_SPACE);
  }
  for (uint32_t directSlot{}; directSlot < ez::INODE_BLOCK_CONTAINER_SIZE;
       ++directSlot) {
    if (inode.directBlocks[directSlot] != NULL_BLOCK) {
      continue;
    }
    inode.directBlocks[directSlot] = blockIndex;
    return {blockIndex};
  }

  const auto single = this->allocateInSingleIndirect(inode, blockIndex);
  if (single.has_value() || single.error() != EzError::NO_SPACE) {
    if (!single.has_value()) {
      return std::unexpected(single.error());
    }
    return single;
  }

  const auto doubleIndirect = this->allocateInDoubleIndirect(inode, blockIndex);
  if (doubleIndirect.has_value() ||
      doubleIndirect.error() != EzError::NO_SPACE) {
    if (!doubleIndirect.has_value()) {
      return std::unexpected(doubleIndirect.error());
    }
    return doubleIndirect;
  }

  const auto tripleIndirect = this->allocateInTripleIndirect(inode, blockIndex);
  if (!tripleIndirect.has_value()) {
    return std::unexpected(tripleIndirect.error());
  }
  return tripleIndirect;
}

}  // namespace ez

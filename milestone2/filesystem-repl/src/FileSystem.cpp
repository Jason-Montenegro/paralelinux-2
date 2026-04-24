// Copyright (C) 2026 Jason Montenegro-Navarro <jasonmontenegro49@gmail.com>
#include "FileSystem.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <expected>
#include <string>
#include <utility>

#include "EzTypes.hpp"
#include "StringUtils.hpp"

/// Macro to simplify <expected> API with non-void expected types
/// creates a varName variable on success
/// returns std::unexpected(error)
#define ASSIGN_OR_RETURN(varName, expression)                         \
  auto&& resVarName##varName = (expression);                          \
  if (!resVarName##varName.has_value())                               \
    return std::unexpected<ez::EzError>(resVarName##varName.error()); \
  auto varName = std::move(resVarName##varName.value());

/// Macro to simplify <expected> API with non-void expected types
/// creates a varName variable on success
/// uses error handler
#define ASSIGN_OR_ELSE(varName, expression, onError) \
  auto&& resVarName##varName = (expression);         \
  if (!resVarName##varName.has_value())              \
    return onError(resVarName##varName.error());     \
  auto varName = std::move(resVarName##varName.value());

/// Macro to simplify <expected> API with void expected types
/// returns std::unexpected(error)
#define EXPECT_OR_RETURN(expression)                         \
  {                                                          \
    auto&& res##res = (expression);                          \
    if (!res##res.has_value())                               \
      return std::unexpected<ez::EzError>(res##res.error()); \
  }

/// Macro to simplify <expected> API with void expected types with error handler
#define EXPECT_OR_ELSE(expression, onError)                      \
  {                                                              \
    auto&& res##res = (expression);                              \
    if (!res##res.has_value()) return onError(res##res.error()); \
  }

namespace ez {

std::expected<void, EzError> FileSystem::mount(const std::string& path) {
  if (this->isMounted()) {
    this->unmount();
  }
  EXPECT_OR_RETURN(this->device.open(path))
  ASSIGN_OR_RETURN(sb, this->device.readStruct<SuperBlock>(0))
  if (sb.fileSystemIdentifier != ez::FS_IDENTIFIER) {
    return std::unexpected(EzError::CORRUPT);
  }
  if (sb.blockSize != ez::BLOCK_SIZE) {
    return std::unexpected(EzError::CORRUPT);
  }
  this->superBlock = sb;
  const uint32_t bitmapBlocks = this->superBlock.inodeTableStart - 1;
  this->bmap.init(&this->device, 1, bitmapBlocks, this->superBlock.totalBlocks);
  this->inodeManager.init(
      &this->device, &this->bmap, this->superBlock.inodeTableStart,
      this->superBlock.totalInodes, this->superBlock.dataStart);
  this->isFsMounted = true;
  return {};
}

bool FileSystem::isMounted() const { return this->isFsMounted; }

void FileSystem::unmount() {
  if (!this->isMounted()) {
    return;
  }
  this->flushSuperBlock();
  this->device.close();
  this->isFsMounted = false;
}

std::expected<void, EzError> FileSystem::format(const std::string& path,
                                                size_t sizeBytes) {
  if (this->isMounted()) {
    this->unmount();
  }

  const auto totalBlocks = static_cast<uint32_t>(sizeBytes / ez::BLOCK_SIZE);
  if (totalBlocks < ez::MIN_BLOCKS) {
    return std::unexpected(EzError::INVALID_ARGUMENT);
  }
  // calculate disk layout
  const auto bitmapBlocks = static_cast<uint32_t>(
      std::ceil(static_cast<double>(totalBlocks) / (ez::BLOCK_SIZE * 8)));
  const uint32_t overhead = 1 + bitmapBlocks;
  const uint32_t remaining =
      (totalBlocks > overhead) ? totalBlocks - overhead : 0;
  uint32_t inodeCount = std::min(std::max(1u, remaining / 9), ez::MAX_INODES);
  const uint32_t dataStart = overhead + inodeCount;
  const uint32_t dataBlocks =
      (totalBlocks > dataStart) ? totalBlocks - dataStart : 0;
  EXPECT_OR_RETURN(this->device.create(path, totalBlocks));
  // build disk structures
  this->superBlock = SuperBlock{.fileSystemIdentifier = ez::FS_IDENTIFIER,
                                .totalBlocks = totalBlocks,
                                .blockSize = ez::BLOCK_SIZE,
                                .inodeTableStart = 1 + bitmapBlocks,
                                .totalInodes = inodeCount,
                                .dataStart = dataStart,
                                .blocksAvailable = dataBlocks,
                                .inodesAvailable = inodeCount - 1};
  EXPECT_OR_RETURN(this->device.writeStruct<SuperBlock>(0, this->superBlock))
  this->bmap.init(&this->device, 1, bitmapBlocks, totalBlocks);
  this->inodeManager.init(&this->device, &this->bmap,
                          this->superBlock.inodeTableStart, inodeCount,
                          dataStart);
  this->bmap.setRange(0, dataStart, true);
  // create root directory
  Inode root{.inodeType = InodeType::DIRECTORY,
             .inodeId = ez::ROOT_INODE_ID,
             .totalSize = 0,
             .createdTimestamp = static_cast<uint32_t>(std::time(nullptr))};
  EXPECT_OR_RETURN(this->inodeManager.write(root))
  EXPECT_OR_RETURN(this->addDirectoryEntry(root, ".", ez::ROOT_INODE_ID))
  EXPECT_OR_RETURN(this->inodeManager.write(root))
  this->isFsMounted = true;
  return {};
}

std::expected<void, EzError> FileSystem::addDirectoryEntry(
    Inode& dir, const std::string& name, uint32_t inodeId) {
  // find an existing block in inode and add entry
  // integer ceiling formula used
  const uint32_t totalBlocks =
      (dir.totalSize + ez::BLOCK_SIZE - 1) / ez::BLOCK_SIZE;
  for (uint32_t blockIndex{}; blockIndex < totalBlocks; ++blockIndex) {
    // find a valid directory block
    const auto blockId = this->inodeManager.resolveDataBlock(dir, blockIndex);
    if (!blockId.has_value() || blockId.value() == ez::NULL_BLOCK) {
      // this case will never happen unless the inode has no blocks or it
      // contains sparse blocks at some point, but I don't think that will ever
      // happen
      continue;
    }
    ASSIGN_OR_RETURN(dirBlock,
                     this->device.readStruct<DirBlock>(blockId.value()))
    // find a spot in dirBlock to store new entry
    for (auto& entry : dirBlock.entries) {
      if (entry.entryInodeId != ez::NULL_BLOCK) {
        continue;
      }
      std::memset(entry.entryName, 0, sizeof(entry.entryName));
      std::strncpy(entry.entryName, name.c_str(), ez::DIR_ENTRY_NAME_MAX);
      entry.entryInodeId = inodeId;
      EXPECT_OR_RETURN(this->device.writeStruct(blockId.value(), dirBlock))
      return {};
    }
  }

  ASSIGN_OR_RETURN(newBlock, this->inodeManager.allocateDataBlock())
  // add new block to inode
  EXPECT_OR_ELSE(this->inodeManager.allocateDataBlockAt(dir, newBlock),
                 [&](const auto& err) {
                   this->inodeManager.freeDataBlock(newBlock);
                   return std::unexpected(err);
                 })
  dir.totalSize += ez::BLOCK_SIZE;
  // append new entry to dirBlock
  DirBlock dirBlock{};
  std::strncpy(dirBlock.entries[0].entryName, name.c_str(),
               ez::DIR_ENTRY_NAME_MAX);
  dirBlock.entries[0].entryInodeId = inodeId;
  EXPECT_OR_RETURN(this->device.writeStruct(newBlock, dirBlock))
  return {};
}

std::expected<FileStat, EzError> FileSystem::stat(const std::string& path) {
  if (!this->isMounted()) {
    return std::unexpected(EzError::NOT_OPEN);
  }
  ASSIGN_OR_RETURN(inodeResult, this->resolvePath(path))
  return {FileStat{.inodeId = inodeResult.inodeId,
                   .type = inodeResult.inodeType,
                   .size = inodeResult.totalSize,
                   .createdTimestamp = inodeResult.createdTimestamp}};
}

std::expected<Inode, EzError> FileSystem::resolvePath(const std::string& path) {
  ASSIGN_OR_RETURN(currentInode, this->inodeManager.read(ez::ROOT_INODE_ID))
  // traverses from the root down the path looking for the inode
  for (const auto& part : utils::tokenizePath(path)) {
    if (part == ".") {
      // cwd
      continue;
    }
    if (part == "..") {
      // redirects to previous path
      ASSIGN_OR_RETURN(parentId, this->lookupInDir(currentInode, ".."))
      ASSIGN_OR_RETURN(parentInode, this->inodeManager.read(parentId))
      currentInode = parentInode;
      continue;
    }
    if (currentInode.inodeType != InodeType::DIRECTORY) {
      return std::unexpected(EzError::NOT_DIRECTORY);
    }
    // moves forward in the path
    ASSIGN_OR_RETURN(childId, this->lookupInDir(currentInode, part))
    ASSIGN_OR_RETURN(childInode, this->inodeManager.read(childId))
    currentInode = childInode;
  }
  return {currentInode};
}

std::expected<uint32_t, EzError> FileSystem::lookupInDir(
    const Inode& dir, const std::string& name) {
  // integer ceiling formula
  const uint32_t totalBlocks =
      (dir.totalSize + ez::BLOCK_SIZE - 1) / ez::BLOCK_SIZE;

  for (uint32_t blockIndex{}; blockIndex < totalBlocks; ++blockIndex) {
    const auto blockId = this->inodeManager.resolveDataBlock(
        const_cast<Inode&>(dir), blockIndex);
    if (!blockId.has_value() || blockId.value() == ez::NULL_BLOCK) {
      continue;
    }
    const auto dirBlock = this->device.readStruct<DirBlock>(blockId.value());
    if (!dirBlock.has_value()) {
      continue;
    }
    // parse through the entries in the dirBlock
    for (const auto& entry : dirBlock->entries) {
      if (entry.entryInodeId != ez::NULL_BLOCK &&
          std::string(entry.entryName) == name) {
        return {entry.entryInodeId};
      }
    }
  }
  return std::unexpected(EzError::NOT_FOUND);
}

std::expected<void, EzError> FileSystem::mkdir(const std::string& path) {
  if (!this->isMounted()) {
    return std::unexpected(EzError::NOT_OPEN);
  }
  const auto [parentPath, name] = utils::splitPath(path);
  if (name.empty()) {
    return std::unexpected(EzError::INVALID_PATH);
  }
  if (name.size() > ez::DIR_ENTRY_NAME_MAX) {
    return std::unexpected(EzError::NAME_TOO_LONG);
  }
  ASSIGN_OR_RETURN(parentInode, this->resolvePath(parentPath))
  if (parentInode.inodeType != InodeType::DIRECTORY) {
    return std::unexpected(EzError::NOT_DIRECTORY);
  }
  if (this->lookupInDir(parentInode, name).has_value()) {
    return std::unexpected(EzError::ALREADY_EXISTS);
  }
  ASSIGN_OR_RETURN(newDirInode, this->inodeManager.create(InodeType::DIRECTORY))
  EXPECT_OR_ELSE(this->addDirectoryEntry(newDirInode, std::string("."),
                                         newDirInode.inodeId),
                 [&](const auto& err) {
                   this->inodeManager.free(newDirInode.inodeId);
                   return std::unexpected(err);
                 })
  EXPECT_OR_ELSE(this->addDirectoryEntry(newDirInode, std::string(".."),
                                         parentInode.inodeId),
                 [&](const auto& err) {
                   this->inodeManager.free(newDirInode.inodeId);
                   return std::unexpected(err);
                 })
  EXPECT_OR_RETURN(this->inodeManager.write(newDirInode))
  EXPECT_OR_ELSE(
      this->addDirectoryEntry(parentInode, name, newDirInode.inodeId),
      [&](const auto& err) {
        this->inodeManager.free(newDirInode.inodeId);
        return std::unexpected(err);
      })
  EXPECT_OR_RETURN(this->inodeManager.write(parentInode))
  this->updateSuperBlock(-1, 0);
  return {};
}

void FileSystem::updateSuperBlock(int32_t inodeDelta, int32_t blockDelta) {
  this->superBlock.inodesAvailable = static_cast<uint32_t>(
      static_cast<int32_t>(this->superBlock.inodesAvailable) + inodeDelta);
  this->superBlock.blocksAvailable = static_cast<uint32_t>(
      static_cast<int32_t>(this->superBlock.blocksAvailable) + blockDelta);
  this->flushSuperBlock();
}

void FileSystem::flushSuperBlock() {
  this->device.writeStruct<SuperBlock>(0, this->superBlock);
}

std::expected<void, EzError> FileSystem::rmdir(const std::string& path) {
  if (!this->isMounted()) {
    return std::unexpected(EzError::NOT_OPEN);
  }
  const auto [parentPath, name] = utils::splitPath(path);
  if (name.empty() || name == "." || name == "..") {
    return std::unexpected(EzError::INVALID_ARGUMENT);
  }
  ASSIGN_OR_RETURN(targetInode, this->resolvePath(path))
  if (targetInode.inodeType != InodeType::DIRECTORY) {
    return std::unexpected(EzError::NOT_DIRECTORY);
  }
  if (targetInode.inodeId == ez::ROOT_INODE_ID) {
    return std::unexpected(EzError::PERMISSION_DENIED);
  }
  ASSIGN_OR_RETURN(entries, this->listDirInode(targetInode))
  for (const auto& entry : entries) {
    if (entry.name != "." && entry.name != "..") {
      return std::unexpected(EzError::NOT_EMPTY);
    }
  }
  EXPECT_OR_RETURN(this->inodeManager.freeAllDataBlocks(targetInode))
  EXPECT_OR_RETURN(this->inodeManager.free(targetInode.inodeId))
  ASSIGN_OR_RETURN(parentInode, this->resolvePath(parentPath))
  this->removeDirectoryEntry(parentInode, name);
  this->inodeManager.write(parentInode);
  this->updateSuperBlock(+1, 0);
  return {};
}

std::expected<void, EzError> FileSystem::removeDirectoryEntry(
    Inode& dir, const std::string& name) {
  // find an existing block in inode and add entry
  // integer ceiling formula
  const uint32_t totalBlocks =
      (dir.totalSize + ez::BLOCK_SIZE - 1) / ez::BLOCK_SIZE;

  for (uint32_t blockIndex{}; blockIndex < totalBlocks; ++blockIndex) {
    // find a valid directory block
    const auto blockId = this->inodeManager.resolveDataBlock(dir, blockIndex);
    if (!blockId.has_value() || blockId.value() == ez::NULL_BLOCK) {
      // this case will never happen unless the inode has no blocks or it
      // contains sparse blocks at some point, but I don't think that will ever
      // happen
      continue;
    }
    const auto readResult = this->device.readStruct<DirBlock>(blockId.value());
    if (!readResult.has_value()) {
      continue;
    }
    // search for correct entry in dirBlock
    DirBlock dirBlock = readResult.value();
    for (auto& entry : dirBlock.entries) {
      if (entry.entryInodeId == ez::NULL_BLOCK) {
        continue;
      }
      if (std::string(entry.entryName) != name) {
        continue;
      }
      // remove block from inode
      entry.entryInodeId = 0;
      std::memset(entry.entryName, 0, sizeof(entry.entryName));
      EXPECT_OR_RETURN(this->device.writeStruct(blockId.value(), dirBlock))
      return {};
    }
  }
  return std::unexpected(EzError::NOT_FOUND);
}

std::expected<void, EzError> FileSystem::writeToFile(const std::string& path,
                                                     const std::string& text) {
  if (!this->isMounted()) {
    return std::unexpected(EzError::NOT_OPEN);
  }
  const auto [parentPath, name] = utils::splitPath(path);
  if (name.empty()) {
    return std::unexpected(EzError::INVALID_PATH);
  }
  if (name.size() > ez::DIR_ENTRY_NAME_MAX) {
    return std::unexpected(EzError::NAME_TOO_LONG);
  }
  ASSIGN_OR_RETURN(parentInode, this->resolvePath(parentPath))
  if (parentInode.inodeType != InodeType::DIRECTORY) {
    return std::unexpected(EzError::NOT_DIRECTORY);
  }
  const auto existingResult = this->lookupInDir(parentInode, name);
  const bool isNew = !existingResult.has_value();
  // resolve the file inode: if it's newly created or found
  Inode fileInode{};
  if (isNew) {
    ASSIGN_OR_RETURN(temp, this->inodeManager.create(InodeType::FILE))
    fileInode = temp;
    EXPECT_OR_ELSE(
        this->addDirectoryEntry(parentInode, name, fileInode.inodeId),
        [&](const auto& err) {
          this->inodeManager.free(temp.inodeId);
          return std::unexpected(err);
        })
    EXPECT_OR_RETURN(this->inodeManager.write(parentInode))
  } else {
    ASSIGN_OR_RETURN(temp, this->inodeManager.read(existingResult.value()))
    fileInode = temp;
    if (fileInode.inodeType == InodeType::DIRECTORY) {
      return std::unexpected(EzError::IS_DIRECTORY);
    }
    // integer ceiling formula
    const uint32_t oldBlocks =
        (fileInode.totalSize + ez::BLOCK_SIZE - 1) / ez::BLOCK_SIZE;
    EXPECT_OR_RETURN(this->inodeManager.freeAllDataBlocks(fileInode))
    EXPECT_OR_RETURN(this->inodeManager.write(fileInode))
    this->updateSuperBlock(0, static_cast<int32_t>(oldBlocks));
  }

  uint32_t bytesRemaining = static_cast<uint32_t>(text.size());
  uint32_t blockIndex{};
  const auto* ptr = reinterpret_cast<const uint8_t*>(text.data());
  // write data to file data block
  while (bytesRemaining > 0) {
    const uint32_t toWrite =
        std::min(bytesRemaining, static_cast<uint32_t>(ez::BLOCK_SIZE));
    // create new data block
    ASSIGN_OR_ELSE(newBlock, this->inodeManager.allocateDataBlock(),
                   [&](const auto& err) {
                     fileInode.totalSize = blockIndex * ez::BLOCK_SIZE;
                     this->inodeManager.write(fileInode);
                     return std::unexpected(err);
                   })
    // append newly created data block
    EXPECT_OR_ELSE(this->inodeManager.allocateDataBlockAt(fileInode, newBlock),
                   [&](const auto& err) {
                     this->inodeManager.freeDataBlock(newBlock);
                     fileInode.totalSize = blockIndex * ez::BLOCK_SIZE;
                     this->inodeManager.write(fileInode);
                     return std::unexpected(err);
                   })
    // write data to newly created data block
    std::array<uint8_t, ez::BLOCK_SIZE> buffer{};
    std::memcpy(buffer.data(), ptr, toWrite);
    this->device.writeBlock(newBlock, buffer.data());
    ptr += toWrite;
    bytesRemaining -= toWrite;
    ++blockIndex;
  }

  fileInode.totalSize = static_cast<uint32_t>(text.size());
  this->inodeManager.write(fileInode);
  this->updateSuperBlock(isNew ? -1 : 0, -static_cast<int32_t>(blockIndex));
  return {};
}

std::expected<std::vector<std::byte>, EzError> FileSystem::readFromFile(
    const std::string& path) {
  if (!this->isMounted()) {
    return std::unexpected(EzError::NOT_OPEN);
  }
  ASSIGN_OR_RETURN(fileInode, this->resolvePath(path))
  if (fileInode.inodeType == InodeType::DIRECTORY) {
    return std::unexpected(EzError::IS_DIRECTORY);
  }

  std::vector<std::byte> result;
  result.reserve(fileInode.totalSize);
  uint32_t bytesRemaining = fileInode.totalSize;
  uint32_t blockIndex{};
  // read data from file data blocks
  while (bytesRemaining > 0) {
    const auto blockId =
        this->inodeManager.resolveDataBlock(fileInode, blockIndex);
    if (!blockId.has_value() || blockId.value() == ez::NULL_BLOCK) {
      return std::unexpected(EzError::CORRUPT);
    }
    std::array<uint8_t, ez::BLOCK_SIZE> buffer{};
    EXPECT_OR_RETURN(this->device.readBlock(blockId.value(), buffer.data()))
    // read data directly from block
    const uint32_t toRead =
        std::min(bytesRemaining, static_cast<uint32_t>(ez::BLOCK_SIZE));
    const auto* bytePtr = reinterpret_cast<const std::byte*>(buffer.data());
    result.insert(result.end(), bytePtr, bytePtr + toRead);
    bytesRemaining -= toRead;
    ++blockIndex;
  }
  return {result};
}

std::expected<std::vector<DirEntryInfo>, EzError> FileSystem::listEntries(
    const std::string& path) {
  if (!this->isMounted()) {
    return std::unexpected(EzError::NOT_OPEN);
  }
  ASSIGN_OR_RETURN(dirInode, this->resolvePath(path))
  if (dirInode.inodeType != InodeType::DIRECTORY) {
    return std::unexpected(EzError::NOT_DIRECTORY);
  }
  ASSIGN_OR_RETURN(entries, this->listDirInode(dirInode))
  std::vector<DirEntryInfo> result;
  for (const auto& entry : entries) {
    ASSIGN_OR_RETURN(inode, this->inodeManager.read(entry.inodeId))
    result.push_back(DirEntryInfo{
        .name = entry.name, .inodeId = entry.inodeId, .type = inode.inodeType});
  }
  return {result};
}

std::expected<std::vector<DirEntryInfo>, EzError> FileSystem::listDirInode(
    const Inode& dir) {
  std::vector<DirEntryInfo> result;
  // integer ceiling formula
  const uint32_t totalBlocks =
      (dir.totalSize + ez::BLOCK_SIZE - 1) / ez::BLOCK_SIZE;

  for (uint32_t blockIndex{}; blockIndex < totalBlocks; ++blockIndex) {
    // find a valid directory block
    const auto blockId = this->inodeManager.resolveDataBlock(
        const_cast<Inode&>(dir), blockIndex);
    if (!blockId.has_value() || blockId.value() == ez::NULL_BLOCK) {
      continue;
    }
    const auto dirBlock = this->device.readStruct<DirBlock>(blockId.value());
    if (!dirBlock.has_value()) {
      continue;
    }
    // list all entries
    for (const auto& entry : dirBlock->entries) {
      if (entry.entryInodeId == ez::NULL_BLOCK) {
        continue;
      }
      // entries don't know their type when stored in a directory, it must
      // be resolved elsewhere by querying the type with the inode id
      result.push_back(DirEntryInfo{.name = std::string(entry.entryName),
                                    .inodeId = entry.entryInodeId,
                                    .type = InodeType::FILE});
    }
  }
  return {result};
}

std::expected<void, EzError> FileSystem::removeFile(const std::string& path) {
  if (!this->isMounted()) {
    return std::unexpected(EzError::NOT_OPEN);
  }
  const auto [parentPath, name] = utils::splitPath(path);
  ASSIGN_OR_RETURN(fileInode, this->resolvePath(path))
  if (fileInode.inodeType == InodeType::DIRECTORY) {
    return std::unexpected(EzError::IS_DIRECTORY);
  }
  // integer ceiling formula
  const uint32_t blocksUsed =
      (fileInode.totalSize + ez::BLOCK_SIZE - 1) / ez::BLOCK_SIZE;
  this->inodeManager.freeAllDataBlocks(fileInode);
  this->inodeManager.free(fileInode.inodeId);

  ASSIGN_OR_RETURN(parentInode, this->resolvePath(parentPath))
  this->removeDirectoryEntry(parentInode, name);
  this->inodeManager.write(parentInode);
  this->updateSuperBlock(+1, static_cast<int32_t>(blocksUsed));
  return {};
}

}  // namespace ez

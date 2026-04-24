// Copyright (C) 2026 Jason Montenegro-Navarro <jasonmontenegro49@gmail.com>
#pragma once
#include <cstdint>
#include <string>

namespace ez {

constexpr uint8_t FS_IDENTIFIER = 0xC4;
constexpr uint16_t BLOCK_SIZE = 256;
constexpr uint32_t MIN_BLOCKS = 4;
constexpr uint32_t MAX_INODES = 536'870'912;
constexpr uint32_t MAX_BITMAP_BLOCKS = 2'097'152;

constexpr uint32_t NULL_BLOCK = 0x0;
constexpr uint32_t ROOT_INODE_ID = 0x0;

constexpr uint32_t INODE_BLOCK_CONTAINER_SIZE = 15;
constexpr uint32_t PTRS_PER_INODE_INDEX_BLOCK = 64;  // 256 / 4
constexpr uint32_t MAX_INODE_BLOCKS =
    ez::INODE_BLOCK_CONTAINER_SIZE *
    (1 + ez::PTRS_PER_INODE_INDEX_BLOCK +
     ez::PTRS_PER_INODE_INDEX_BLOCK * ez::PTRS_PER_INODE_INDEX_BLOCK +
     ez::PTRS_PER_INODE_INDEX_BLOCK * ez::PTRS_PER_INODE_INDEX_BLOCK *
         ez::PTRS_PER_INODE_INDEX_BLOCK);  // 3.994.575

constexpr uint32_t DIR_ENTRY_SIZE = 24;
constexpr uint32_t DIR_ENTRY_NAME_MAX = 19;
constexpr uint32_t DIR_ENTRIES_PER_BLOCK = 10;
constexpr uint32_t DIR_BLOCK_PADDING = 16;

enum class InodeType : uint8_t {
  FREE = 0x0,
  FILE = 0x1,
  DIRECTORY = 0x2,
};

// all structures are packed to be 256 to read and write to disk directly
#pragma pack(push, 1)

struct SuperBlock {
  uint8_t fileSystemIdentifier{FS_IDENTIFIER};
  uint32_t totalBlocks{MIN_BLOCKS};
  uint16_t blockSize{BLOCK_SIZE};
  uint32_t inodeTableStart{};
  uint32_t totalInodes{};
  uint32_t dataStart{};
  uint32_t blocksAvailable{MIN_BLOCKS};
  uint32_t inodesAvailable{};
  uint8_t padding[229];
};

struct Inode {
  InodeType inodeType{InodeType::FILE};
  uint32_t inodeId{};
  uint32_t totalSize{};
  uint32_t createdTimestamp{};
  uint32_t directBlocks[INODE_BLOCK_CONTAINER_SIZE]{};
  uint32_t indirectBlocks[INODE_BLOCK_CONTAINER_SIZE]{};
  uint32_t doubleIndirectBlocks[INODE_BLOCK_CONTAINER_SIZE]{};
  uint32_t tripleIndirectBlocks[INODE_BLOCK_CONTAINER_SIZE]{};
  uint8_t padding[3]{};
};

struct DirEntry {
  char entryName[20]{};     // 19 chars + null terminator
  uint32_t entryInodeId{};  // 0x0 = empty
};

struct DirBlock {
  DirEntry entries[DIR_ENTRIES_PER_BLOCK]{};
  uint8_t padding[DIR_BLOCK_PADDING]{};
};

struct IndirectBlock {
  uint32_t ptrs[PTRS_PER_INODE_INDEX_BLOCK]{};
};

#pragma pack(pop)

enum class EzError : uint8_t {
  OK = 0,
  NOT_FOUND,
  ALREADY_EXISTS,
  NO_SPACE,
  NO_INODES,
  IS_DIRECTORY,
  NOT_DIRECTORY,
  NOT_EMPTY,
  NAME_TOO_LONG,
  INVALID_PATH,
  IO_ERROR,
  CORRUPT,
  PERMISSION_DENIED,
  INVALID_ARGUMENT,
  NOT_OPEN,
};

inline const char* errorToStr(EzError e) {
  switch (e) {
    case EzError::OK:
      return "OK";
    case EzError::NOT_FOUND:
      return "No such file or directory";
    case EzError::ALREADY_EXISTS:
      return "File already exists";
    case EzError::NO_SPACE:
      return "No space left on device";
    case EzError::NO_INODES:
      return "No inodes available";
    case EzError::IS_DIRECTORY:
      return "Is a directory";
    case EzError::NOT_DIRECTORY:
      return "Not a directory";
    case EzError::NOT_EMPTY:
      return "Directory not empty";
    case EzError::NAME_TOO_LONG:
      return "Name too long";
    case EzError::INVALID_PATH:
      return "Invalid path";
    case EzError::IO_ERROR:
      return "I/O error";
    case EzError::CORRUPT:
      return "Filesystem corrupt";
    case EzError::PERMISSION_DENIED:
      return "Permission denied";
    case EzError::INVALID_ARGUMENT:
      return "Invalid argument";
    case EzError::NOT_OPEN:
      return "Filesystem not open";
    default:
      return "Unknown error";
  }
}

struct FileStat {
  uint32_t inodeId{};
  InodeType type{InodeType::FREE};
  uint32_t size{};
  uint32_t createdTimestamp{};

  bool isFile() const { return type == InodeType::FILE; }
  bool isDir() const { return type == InodeType::DIRECTORY; }
};

struct DirEntryInfo {
  std::string name{};
  uint32_t inodeId{};
  InodeType type{};

  bool isFile() const { return type == InodeType::FILE; }
  bool isDir() const { return type == InodeType::DIRECTORY; }
};

struct FsInfo {
  uint64_t totalBlocks{};
  uint64_t blocksAvailable{};
  uint64_t totalInodes{};
  uint64_t inodesAvailable{};
  uint64_t blockSizeBytes{};
  uint64_t totalBytes{};
  uint64_t usedBytes{};
  uint64_t freeBytes{};
};

};  // namespace ez

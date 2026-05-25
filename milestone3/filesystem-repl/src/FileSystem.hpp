// Copyright (C) 2026 Jason Montenegro-Navarro <jasonmontenegro49@gmail.com>
#pragma once
#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <vector>

#include "BitmapManager.hpp"
#include "DiskDevice.hpp"
#include "EzTypes.hpp"
#include "InodeManager.hpp"

namespace ez {

class FileSystem {
 public:
  FileSystem() = default;
  ~FileSystem() { this->unmount(); }
  FileSystem(const FileSystem&) = delete;
  FileSystem& operator=(const FileSystem&) = delete;
  FileSystem(FileSystem&&) noexcept = default;
  FileSystem& operator=(FileSystem&&) noexcept = default;

  /// @brief Mounts disk partition to file system
  std::expected<void, EzError> mount(const std::string& path);
  void unmount();
  bool isMounted() const;
  /// @brief Formats a newly created disk partition
  std::expected<void, EzError> format(const std::string& path,
                                      size_t sizeBytes);
  /// @brief Returns directory entry information
  std::expected<FileStat, EzError> stat(const std::string& path);
  /// @brief Creates a directory
  std::expected<void, EzError> mkdir(const std::string& path);
  /// @brief Removes a directory
  std::expected<void, EzError> rmdir(const std::string& path);
  /// @brief Writes to a file
  std::expected<void, EzError> writeToFile(const std::string& path,
                                           const std::string& text);
  /// @brief Reads from a file
  std::expected<std::vector<std::byte>, EzError> readFromFile(
      const std::string& path);
  /// @brief Lists all entries from a directory
  std::expected<std::vector<DirEntryInfo>, EzError> listEntries(
      const std::string& path);
  /// @brief Removes a file
  std::expected<void, EzError> removeFile(const std::string& path);

 private:
  std::expected<void, EzError> addDirectoryEntry(Inode& dir,
                                                 const std::string& name,
                                                 uint32_t inodeId);
  /// @brief Searches for an inode in the give path
  std::expected<Inode, EzError> resolvePath(const std::string& path);
  /// @brief Looks for the inodeId of a name inside a directory inode
  std::expected<uint32_t, EzError> lookupInDir(const Inode& dir,
                                               const std::string& name);
  /// @brief Both deltas can reduce or increment the counters for each field
  /// @note updateSuperBlock(-1. 0)  reduces one inode
  /// @note updateSuperBlock(0. -1)  reduces one block
  /// @note updateSuperBlock(1. 0)  increments one inode
  /// @note updateSuperBlock(0. 1)  increments one block
  void updateSuperBlock(int32_t inodeDelta, int32_t blockDelta);
  void flushSuperBlock();
  std::expected<void, EzError> removeDirectoryEntry(Inode& dir,
                                                    const std::string& name);
  std::expected<std::vector<DirEntryInfo>, EzError> listDirInode(
      const Inode& dir);

 private:
  DiskDevice device;
  BitmapManager bmap;
  InodeManager inodeManager;
  SuperBlock superBlock{};
  bool isFsMounted{false};
};

}  // namespace ez

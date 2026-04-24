// Copyright (C) 2026 Jason Montenegro-Navarro <jasonmontenegro49@gmail.com>
#pragma once
#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#include "FileSystem.hpp"

namespace ez {
enum class EzError : uint8_t;
}

class Repl {
 public:
  explicit Repl(int argc, std::span<char*> argv);
  ~Repl();
  void run();

 private:
  struct ReplContext {
    ez::FileSystem& fs;
    std::string& cwd;
  };

  static void printReplStartup();
  void printPrompt() const;
  static void printError(const char* msg, ez::EzError e);
  static std::string formatSize(uint64_t bytes);
  static std::string resolvePath(const std::string& cwd,
                                 const std::string& path);
  static void printHelp();
  using Handler =
      std::function<void(ReplContext&, const std::vector<std::string>&)>;
  static std::unordered_map<std::string, Handler> commandDispatcher;
  ez::FileSystem fs;
  std::string cwd{"/"};
};

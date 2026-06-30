// Copyright (C) 2026 Jason Montenegro-Navarro <jasonmontenegro49@gmail.com>
#include "Repl.hpp"

#include <algorithm>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>

#include "EzTypes.hpp"
#include "FileSystem.hpp"
#include "StringUtils.hpp"

std::string Repl::formatSize(uint64_t bytes) {
  if (bytes < 1024) {
    return std::to_string(bytes) + " B";
  }
  if (bytes < 1024ull * 1024ull) {
    return std::to_string(bytes / 1024) + " KB";
  }
  if (bytes < 1024ull * 1024ull * 1024ull) {
    return std::to_string(bytes / (1024ull * 1024ull)) + " MB";
  }
  return std::to_string(bytes / (1024ull * 1024ull * 1024ull)) + " GB";
}

std::string Repl::resolvePath(const std::string& cwd, const std::string& path) {
  if (path.empty()) {
    return cwd;
  }
  if (path[0] == '/') {
    return path;
  }
  // Handle "." and ".."
  const std::string base = (cwd == "/") ? "" : cwd;
  const std::string full = base + '/' + path;
  // Tokenize path into names
  std::vector<std::string> tokens;
  std::istringstream ss(full);
  std::string name;
  while (std::getline(ss, name, '/')) {
    if (name.empty() || name == ".") {
      continue;
    }
    if (name == "..") {
      if (!tokens.empty()) {
        tokens.pop_back();
      }
    } else {
      tokens.push_back(name);
    }
  }
  if (tokens.empty()) {
    return "/";
  }
  std::string result;
  for (auto& p : tokens) {
    result += std::string("/") + p;
  }
  return result;
}

void Repl::printHelp() {
  const auto row = [](const char* cmd, const char* description) {
    std::cout << "  " << std::left << std::setw(30) << cmd << description
              << "\n";
  };

  std::cout << "\n  ez-filesystem REPL commands\n";
  std::cout << "  ─────────────────────────────────────────\n";
  row("format <file> <size_bytes>", "Create a new filesystem container");
  row("mount <file>", "Mount an existing container");
  row("unmount", "Unmount current container");
  row("ls <path>", "List directory");
  row("cd <path>", "Change directory");
  row("pwd", "Print working directory");
  row("mkdir <path>", "Create directory");
  row("rmdir <path>", "Remove empty directory");
  row("touch <path>", "Create empty file");
  row("write <path> <text...>", "Write text to file (overwrites)");
  row("cat <path>", "Print file contents");
  row("rm <path>", "Delete file");
  row("help", "Show this help");
  row("exit", "Exit the REPL");
  std::cout << "\n";
}

std::unordered_map<std::string, Repl::Handler> Repl::commandDispatcher{
    {"format",
     [](ReplContext& ctx, const auto& tokens) {
       if (tokens.size() < 3) {
         std::cout << "  usage: format <file> <size_bytes>\n";
         return;
       }
       const uint64_t size = std::stoull(tokens[2]);
       const auto result = ctx.fs.format(tokens[1], size);
       if (result.has_value()) {
         std::cout << "  formatted " << tokens[1] << " to "
                   << Repl::formatSize(size) << '\n';
       } else {
         Repl::printError("format", result.error());
       }
       ctx.cwd = '/';
     }},
    {"mount",
     [](ReplContext& ctx, const auto& tokens) {
       if (tokens.size() < 2) {
         std::cout << "  usage: usage: mount <file>\n";
         return;
       }
       const auto result = ctx.fs.mount(tokens[1]);
       if (result.has_value()) {
         std::cout << "  mounted " << tokens[1] << '\n';
       } else {
         Repl::printError("mount", result.error());
       }
       ctx.cwd = '/';
     }},
    {"unmount",
     [](ReplContext& ctx, const auto& tokens) {
       (void)tokens;
       ctx.fs.unmount();
       ctx.cwd = '/';
       std::cout << "  unmounted\n";
     }},
    {"pwd",
     [](ReplContext& ctx, const auto& tokens) {
       (void)tokens;
       std::cout << "  " << ctx.cwd << '\n';
     }},
    {"cd",
     [](ReplContext& ctx, const auto& tokens) {
       const std::string target =
           (tokens.size() < 2) ? "/" : Repl::resolvePath(ctx.cwd, tokens[1]);
       const auto result = ctx.fs.stat(target);
       if (!result.has_value()) {
         Repl::printError("cd", result.error());
       } else if (!result->isDir()) {
         std::cerr << "  path is not a directory" << '\n';
       } else {
         ctx.cwd = target;
       }
     }},
    {"ls",
     [](ReplContext& ctx, const auto& tokens) {
       const std::string target = (tokens.size() < 2)
                                      ? ctx.cwd
                                      : Repl::resolvePath(ctx.cwd, tokens[1]);
       auto result = ctx.fs.listEntries(target);
       if (!result.has_value()) {
         Repl::printError("ls", result.error());
         return;
       }
       std::sort(result->begin(), result->end(), [](auto& a, auto& b) {
         if (a.isDir() != b.isDir()) {
           return a.isDir() > b.isDir();
         }
         return a.name < b.name;
       });
       for (auto& e : result.value()) {
         if (e.name == "." || e.name == "..") {
           continue;
         }
         if (e.isDir()) {
           std::cout << "  dir:  " << e.name << '/' << '\n';
         } else {
           std::cout << "  file: " << e.name << '\n';
         }
       }
     }},
    {"mkdir",
     [](ReplContext& ctx, const auto& tokens) {
       if (tokens.size() < 2) {
         std::cerr << "  usage: mkdir <path>\n";
         return;
       }
       const auto result = ctx.fs.mkdir(Repl::resolvePath(ctx.cwd, tokens[1]));
       if (!result.has_value()) {
         Repl::printError("mkdir", result.error());
       }
     }},
    {"rmdir",
     [](ReplContext& ctx, const auto& tokens) {
       if (tokens.size() < 2) {
         std::cerr << "  usage: rmdir <path>\n";
         return;
       }
       const auto result = ctx.fs.rmdir(Repl::resolvePath(ctx.cwd, tokens[1]));
       if (!result.has_value()) {
         Repl::printError("rmdir", result.error());
       }
     }},
    {"touch",
     [](ReplContext& ctx, const auto& tokens) {
       if (tokens.size() < 2) {
         std::cerr << "  usage: touch <path>\n";
         return;
       }
       const auto result =
           ctx.fs.writeToFile(Repl::resolvePath(ctx.cwd, tokens[1]), "");
       if (!result.has_value()) {
         Repl::printError("touch", result.error());
       }
     }},
    {"write",
     [](ReplContext& ctx, const auto& tokens) {
       if (tokens.size() < 3) {
         std::cerr << "  usage: write <path> <text...>\n";
       }
       std::string text;
       for (auto it{tokens.begin() + 2}; it != tokens.end(); ++it) {
         text += *it;
         if (std::distance(it, tokens.end()) >= 2) {
           text += ' ';
         }
       }
       const auto result =
           ctx.fs.writeToFile(Repl::resolvePath(ctx.cwd, tokens[1]), text);
       if (!result.has_value()) {
         Repl::printError("write", result.error());
       }
     }},
    {"cat",
     [](ReplContext& ctx, const auto& tokens) {
       if (tokens.size() < 2) {
         std::cerr << "  usage: cat <path>\n";
       }
       const auto result =
           ctx.fs.readFromFile(Repl::resolvePath(ctx.cwd, tokens[1]));
       if (!result.has_value()) {
         Repl::printError("cat", result.error());
         return;
       }
       std::string output;
       output.reserve(result->size());
       std::transform(result->begin(), result->end(),
                      std::back_inserter(output),
                      [](std::byte b) { return static_cast<char>(b); });
       std::cout << output << '\n';
     }},
    {"rm", [](ReplContext& ctx, const auto& tokens) {
       if (tokens.size() < 2) {
         std::cerr << "  usage: rm <path>\n";
         return;
       }
       const auto result =
           ctx.fs.removeFile(Repl::resolvePath(ctx.cwd, tokens[1]));
       if (!result.has_value()) {
         Repl::printError("rm", result.error());
       }
     }}};

Repl::Repl(int argc, std::span<char*> argv) {
  if (argc == 2) {
    const auto result = this->fs.mount(argv[1]);
    if (result.has_value()) {
      std::cerr << "  mounted " << argv[1] << '\n';
    } else {
      std::cerr << "  could not mount " << ez::errorToStr(result.error())
                << '\n';
    }
  }
}

Repl::~Repl() { this->fs.unmount(); }

void Repl::run() {
  Repl::printReplStartup();
  std::string line;
  Repl::ReplContext ctx(this->fs, this->cwd);
  while (true) {
    this->printPrompt();
    if (!std::getline(std::cin, line)) {
      break;
    }

    auto tokens = utils::tokenize(line);
    if (tokens.empty()) {
      continue;
    }
    const std::string& command = tokens[0];
    if (command == "exit") {
      break;
    }
    if (command == "help") {
      Repl::printHelp();
      continue;
    }

    try {
      this->commandDispatcher.at(command)(ctx, tokens);
    } catch (const std::out_of_range& e) {
      std::cerr << "  unknown command '" << command
                << "'. input 'help' for command options list\n";
    }
  }
}

void Repl::printReplStartup() {
  std::cout << "┌──────────────────────────────────┐\n"
            << "│        ez-filesystem REPL        │\n"
            << "└──────────────────────────────────┘\n"
            << "input 'help' for command options list\n";
}

void Repl::printPrompt() const { std::cout << "ez: " << this->cwd << " $ "; }

void Repl::printError(const char* msg, ez::EzError e) {
  std::cerr << "  error: " << msg << ": " << ez::errorToStr(e) << '\n';
}

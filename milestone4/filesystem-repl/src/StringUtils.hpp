// Copyright (C) 2026 Jason Montenegro-Navarro <jasonmontenegro49@gmail.com>
#pragma once
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace utils {
/// @brief Tokenizes a line with whitespace as delimiter between tokens
inline std::vector<std::string> tokenize(const std::string& line) {
  std::vector<std::string> tokens;
  std::stringstream ss(line);
  std::string token;
  while (ss >> token) {
    tokens.push_back(token);
  }
  return tokens;
}

/// @brief Tokenizes path
inline std::vector<std::string> tokenizePath(const std::string& path) {
  std::vector<std::string> parts;
  std::stringstream ss(path);
  std::string token;
  while (std::getline(ss, token, '/')) {
    if (!token.empty()) {
      parts.push_back(token);
    }
  }
  return parts;
}

/// @brief Splits path between parent and basename
inline std::pair<std::string, std::string> splitPath(const std::string& path) {
  if (path == "/" || path.empty()) {
    return {"/", ""};
  }
  const auto pos = path.rfind('/');
  std::string parent = (pos == 0) ? "/" : path.substr(0, pos);
  return {parent, path.substr(pos + 1)};
}

}  // namespace utils

// Copyright (C) 2026 Jason Montenegro-Navarro <jasonmontenegro49@gmail.com>
#pragma once
#include <string>
#include <unordered_map>

class HtmlTemplateLoader {
 public:
  static HtmlTemplateLoader& instance();
  std::string load(const std::string& filename);

 private:
  HtmlTemplateLoader() = default;
  /// @brief Cache for templates
  std::unordered_map<std::string, std::string> templates;
};

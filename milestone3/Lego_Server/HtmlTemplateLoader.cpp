// Copyright (C) 2026 Jason Montenegro-Navarro <jasonmontenegro49@gmail.com>
#include "HtmlTemplateLoader.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

HtmlTemplateLoader& HtmlTemplateLoader::instance() {
  static HtmlTemplateLoader loader;
  return loader;
}

std::string HtmlTemplateLoader::load(const std::string& filename) {
  if (this->templates.find(filename) != this->templates.end()) {
    return this->templates.at(filename);
  }
  // read template and cache it
  std::fstream file("pages/" + filename);
  if (!file.is_open()) {
    throw std::runtime_error(filename + " could not be found in templates");
  }
  std::ostringstream os;
  os << file.rdbuf();
  this->templates[filename] = os.str();
  return this->templates.at(filename);
}

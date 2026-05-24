// Copyright (C) 2026 Jason Montenegro-Navarro <jasonmontenegro49@gmail.com>
#include "HttpRequest.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

HttpRequest::HttpRequest(std::vector<std::byte> res) {
  std::istringstream ss(
      std::string(reinterpret_cast<char*>(res.data()), res.size()));

  // parse request line
  std::string firstLine;
  std::getline(ss, firstLine);
  std::istringstream lineStream(firstLine);
  lineStream >> this->reqMethod >> this->reqPath >> this->reqVersion;
  if (!this->reqVersion.empty() && this->reqVersion.back() == '\r') {
    this->reqVersion.pop_back();
  }
  if (this->reqPath.back() == '/') {
    this->reqPath.pop_back();
  }
  // parse headers
  std::string line;
  while (std::getline(ss, line) && line != "\r" && !line.empty()) {
    const auto colon = line.find(':');
    if (colon != std::string::npos) {
      const auto key = line.substr(0, colon);
      auto val = line.substr(colon + 2);
      if (!val.empty() && val.back() == '\r') {
        val.pop_back();
      }
      this->headers.emplace(key, val);
    }
  }

  // parse body
  std::string body;
  while (std::getline(ss, line)) {
    body += line + "\n";
  }
  std::transform(
      body.cbegin(), body.cend(), std::back_inserter(this->reqBody),
      [](auto& c) { return static_cast<std::byte>(static_cast<uint8_t>(c)); });
}

const std::string& HttpRequest::method() const { return this->reqMethod; }

const std::string& HttpRequest::path() const { return this->reqPath; }

const std::string& HttpRequest::version() const { return this->reqVersion; }

size_t HttpRequest::contentLength() const {
  const auto val = this->header("Content-Length");
  if (val.empty()) {
    return 0;
  }
  return static_cast<size_t>(std::stoul(val));
}

std::string HttpRequest::contentType() const {
  for (const auto& [key, val] : this->headers) {
    if (key == "Content-Type") {
      return val;
    }
  }
  return "";
}

std::string HttpRequest::header(const std::string& name) const {
  for (const auto& [key, val] : this->headers) {
    if (key == name) {
      return val;
    }
  }
  return "";
}

const std::vector<std::byte>& HttpRequest::body() const {
  return this->reqBody;
}

HttpRequest& HttpRequest::setMethod(const std::string& method) {
  this->reqMethod = method;
  return *this;
}

HttpRequest& HttpRequest::setPath(const std::string& path) {
  this->reqPath = path;
  return *this;
}

HttpRequest& HttpRequest::setVersion(const std::string& version) {
  this->reqVersion = version;
  return *this;
}

HttpRequest& HttpRequest::setHeader(const std::string& header,
                                    const std::string& content) {
  this->headers[header] = content;
  return *this;
}

HttpRequest& HttpRequest::setBody(const std::string& content,
                                  const std::string& contentType) {
  this->headers["Content-Type"] = contentType;
  this->headers["Content-Length"] = std::to_string(content.size());
  this->reqBody.resize(content.size());
  std::transform(
      content.cbegin(), content.cend(), this->reqBody.begin(),
      [](const auto& c) { return std::byte{static_cast<uint8_t>(c)}; });
  return *this;
}

std::vector<std::byte> HttpRequest::serialize() const {
  std::ostringstream ss;
  // first line
  ss << this->reqMethod << " " << this->reqPath << " " << this->reqVersion
     << "\r\n";

  // headers
  for (const auto& [key, val] : this->headers) {
    ss << key << ": " << val << "\r\n";
  }
  ss << "\r\n";

  // body
  const std::string head = ss.str();
  std::vector<std::byte> result;
  result.reserve(head.size() + this->reqBody.size());

  std::transform(head.cbegin(), head.cend(), std::back_inserter(result),
                 [](char c) { return std::byte{static_cast<uint8_t>(c)}; });
  result.insert(result.end(), this->reqBody.begin(), this->reqBody.end());
  return result;
}

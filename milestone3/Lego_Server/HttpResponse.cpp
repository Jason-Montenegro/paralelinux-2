// Copyright (C) 2026 Jason Montenegro-Navarro <jasonmontenegro49@gmail.com>
#include "HttpResponse.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

HttpResponse::HttpResponse(std::vector<std::byte> res) {
  std::string content;
  std::transform(
      res.cbegin(), res.cend(), std::back_inserter(content),
      [](const auto& b) { return static_cast<char>(static_cast<uint8_t>(b)); });
  std::istringstream ss(content);

  ss >> this->resVersion >> this->resCode >> this->resMessage;
  std::string line;
  std::getline(ss, line);

  while (std::getline(ss, line) && !line.empty() && line != "\r") {
    const auto colon = line.find(':');
    if (colon != std::string::npos) {
      const auto header = line.substr(0, colon);
      auto value = line.substr(colon + 2);
      if (!value.empty() && value.back() == '\r') {
        value.pop_back();
      }
      this->headers[header] = value;
    }
  }
  const std::string remaining(std::istreambuf_iterator<char>(ss), {});
  std::transform(remaining.cbegin(), remaining.cend(),
                 std::back_inserter(this->resBody),
                 [](char c) { return std::byte{static_cast<uint8_t>(c)}; });
}

HttpResponse::HttpResponse() : resMessage("OK"), resCode(200) {}

HttpResponse& HttpResponse::setStatus(size_t code, const std::string& message) {
  this->resCode = code;
  this->resMessage = message;
  return *this;
}

HttpResponse& HttpResponse::setHeader(const std::string& name,
                                      const std::string& value) {
  this->headers[name] = value;
  return *this;
}

HttpResponse& HttpResponse::setBody(const std::vector<std::byte>& body,
                                    const std::string& contentType) {
  this->resBody = body;
  this->setHeader("Content-Type", contentType);
  this->setHeader("Content-Length", std::to_string(resBody.size()));
  return *this;
}

HttpResponse& HttpResponse::setBody(const std::string& body,
                                    const std::string& contentType) {
  this->resBody.resize(body.size());
  std::transform(body.cbegin(), body.cend(), this->resBody.begin(),
                 [](char c) { return std::byte(c); });
  this->setHeader("Content-Type", contentType);
  this->setHeader("Content-Length", std::to_string(this->resBody.size()));
  return *this;
}

HttpResponse& HttpResponse::setVersion(const std::string& version) {
  this->resVersion = version;
  return *this;
}

int HttpResponse::statusCode() const { return this->resCode; }

const std::string& HttpResponse::statusMessage() const {
  return this->resMessage;
}

std::string HttpResponse::header(const std::string& name) const {
  if (this->headers.find(name) != this->headers.end()) {
    return this->headers.at(name);
  }
  return "";
}

const std::string& HttpResponse::version() const { return this->resVersion; }

size_t HttpResponse::contentLength() const {
  try {
    auto lengthStr = this->headers.at("Content-Length");
    return std::strtoul(lengthStr.c_str(), nullptr, 10);
  } catch (...) {
    return 0;
  }
}

const std::vector<std::byte>& HttpResponse::body() const { return resBody; }

std::vector<std::byte> HttpResponse::serialize() const {
  auto head = this->resVersion + " " + std::to_string(this->resCode) + " " +
              this->resMessage + "\r\n";
  for (const auto& [header, content] : this->headers) {
    head += header + ": " + content + "\r\n";
  }
  head += "\r\n";

  std::vector<std::byte> result;
  result.reserve(head.size() + this->resBody.size());
  std::transform(
      head.cbegin(), head.cend(), std::back_inserter(result),
      [](const auto& c) { return std::byte{static_cast<uint8_t>(c)}; });
  result.insert(result.end(), this->resBody.cbegin(), this->resBody.cend());
  return result;
}

HttpResponse HttpResponse::ok(const std::string& httpVersion) {
  return HttpResponse().setStatus(200, "OK").setVersion(httpVersion);
}

HttpResponse HttpResponse::notFound(const std::string& httpVersion) {
  return HttpResponse()
      .setStatus(404, "Not Found")
      .setBody("<h1>Not Found</h1>", "text/html")
      .setVersion(httpVersion);
}

HttpResponse HttpResponse::badRequest(const std::string& httpVersion) {
  return HttpResponse()
      .setStatus(400, "Bad Request")
      .setBody("<h1>Bad Request</h1>", "text/html")
      .setVersion(httpVersion);
}

HttpResponse HttpResponse::internalError(const std::string& httpVersion) {
  return HttpResponse()
      .setStatus(500, "Internal Server Error")
      .setBody("<h1>Internal Server Error</h1>", "text/html")
      .setVersion(httpVersion);
}

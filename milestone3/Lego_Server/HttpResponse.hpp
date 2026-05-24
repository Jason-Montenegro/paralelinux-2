// Copyright (C) 2026 Jason Montenegro-Navarro <jasonmontenegro49@gmail.com>
#pragma once
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

/// @brief Http response
/// @note The class is a skeleton, and will hold anything you pass into it
class HttpResponse {
 public:
  explicit HttpResponse(std::vector<std::byte> res);
  HttpResponse();
  HttpResponse& setStatus(size_t code, const std::string& message);
  HttpResponse& setHeader(const std::string& name, const std::string& value);
  HttpResponse& setBody(const std::vector<std::byte>& body,
                        const std::string& contentType);
  HttpResponse& setBody(const std::string& body,
                        const std::string& contentType);
  HttpResponse& setVersion(const std::string& version);
  int statusCode() const;
  const std::string& statusMessage() const;
  std::string header(const std::string& name) const;
  const std::vector<std::byte>& body() const;
  std::vector<std::byte> serialize() const;
  const std::string& version() const;
  size_t contentLength() const;
  // Common response factories
  static HttpResponse ok(const std::string& httpVersion);
  static HttpResponse notFound(const std::string& httpVersion);
  static HttpResponse badRequest(const std::string& httpVersion);
  static HttpResponse internalError(const std::string& httpVersion);

 private:
  std::unordered_map<std::string, std::string> headers;
  std::vector<std::byte> resBody;
  std::string resMessage;
  std::string resVersion;
  size_t resCode;
};
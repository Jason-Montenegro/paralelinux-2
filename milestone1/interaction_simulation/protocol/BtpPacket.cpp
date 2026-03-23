// Copyright 2026 Jason Montenegro <jasonmontenegro49@gmail.com>
#include "BtpPacket.hpp"

#include <arpa/inet.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>
#include <string_view>
#include <string>
#include <vector>

#include "BtpTypes.hpp"

std::vector<std::byte> BtpPacket::serializePacket(const BtpPacket& packet) {
  std::vector<std::byte> result(BtpPacket::TOTAL_HEADERS_LENGTH
    + packet.content.size(), std::byte{});
  BtpPacket::serializeIpType(packet, result);
  BtpPacket::serializeSourceIp(packet, result);
  BtpPacket::serializeDestinationIp(packet, result);
  BtpPacket::serializePacketType(packet, result);
  BtpPacket::serializeContentType(packet, result);
  BtpPacket::serializeContentLength(packet, result);
  BtpPacket::serializeContent(packet, result);
  return result;
}

void BtpPacket::serializeIpType(const BtpPacket& packet,
  std::vector<std::byte>& result) {
  assert(result.size() >= BtpPacket::TOTAL_HEADERS_LENGTH);
  result[BtpPacket::IP_TYPE_OFFSET] = std::byte{
    static_cast<uint8_t>(packet.ipType)};
}

void BtpPacket::serializeSourceIp(const BtpPacket& packet,
  std::vector<std::byte>& result) {
  assert(result.size() >= BtpPacket::TOTAL_HEADERS_LENGTH);
  int status = 1;
  if (packet.ipType == BtpTypes::IpType::IPv4) {
    // IPv4 addresses pad the rest of the IP space with 0
    status = ::inet_pton(AF_INET, packet.sourceIp.c_str(),
      result.data() + BtpPacket::SOURCE_IP_OFFSET);
  } else {
    status = ::inet_pton(AF_INET6, packet.sourceIp.c_str(),
      result.data() + BtpPacket::SOURCE_IP_OFFSET);
  }
  if (status == 0) {
    throw std::runtime_error("Invalid format for IP address while parsing"
      " BTP packet");
  }
  if (status == -1) {
    throw std::runtime_error("Invalid address family on parsed IP address"
      " from BTP packet");
  }
}

void BtpPacket::serializeDestinationIp(const BtpPacket& packet,
  std::vector<std::byte>& result) {
  assert(result.size() >= BtpPacket::TOTAL_HEADERS_LENGTH);
  int status = 1;
  if (packet.ipType == BtpTypes::IpType::IPv4) {
    // IPv4 addresses pad the rest of the IP space with 0
    status = ::inet_pton(AF_INET, packet.destinationIp.c_str(),
      result.data() + BtpPacket::DESTINATION_IP_OFFSET);
  } else {
    status = ::inet_pton(AF_INET6, packet.destinationIp.c_str(),
      result.data() + BtpPacket::DESTINATION_IP_OFFSET);
  }
  if (status == 0) {
    throw std::runtime_error("Invalid format for IP address while parsing"
      " BTP packet");
  }
  if (status == -1) {
    throw std::runtime_error("Invalid address family on parsed IP address"
      " from BTP packet");
  }
}

void BtpPacket::serializePacketType(const BtpPacket& packet,
  std::vector<std::byte>& result) {
  assert(result.size() >= BtpPacket::TOTAL_HEADERS_LENGTH);
  result[BtpPacket::PACKET_TYPE_OFFSET] = std::byte{
    static_cast<uint8_t>(packet.packetType)};
}

void BtpPacket::serializeContentType(const BtpPacket& packet,
  std::vector<std::byte>& result) {
  assert(
    result.size() >= BtpPacket::TOTAL_HEADERS_LENGTH + packet.content.size());
  result[BtpPacket::CONTENT_TYPE_OFFSET] = std::byte{
    static_cast<uint8_t>(packet.contentType)};
}

void BtpPacket::serializeContentLength(const BtpPacket& packet,
  std::vector<std::byte>& result) {
  assert(result.size() >= BtpPacket::TOTAL_HEADERS_LENGTH);
  const uint32_t contentLength = static_cast<uint32_t>(packet.content.size());
  for (size_t index{}; index < BtpPacket::CONTENT_LENGTH_SIZE; ++index) {
    // Serializes bytes one by one
    result[BtpPacket::CONTENT_LENGTH_OFFSET + index] = std::byte{
      static_cast<uint8_t>(contentLength >> (24 - index * 8))};
  }
}

void BtpPacket::serializeContent(const BtpPacket& packet,
  std::vector<std::byte>& result) {
  assert(result.size() >= packet.content.size());
  std::copy(packet.content.cbegin(), packet.content.cend(),
    result.begin() + BtpPacket::TOTAL_HEADERS_LENGTH);
}

BtpTypes::IpType BtpPacket::getIpType() const {
  return this->ipType;
}

const std::string& BtpPacket::getSourceIp() const {
  return this->sourceIp;
}

const std::string& BtpPacket::getDestinationIp() const {
  return this->destinationIp;
}

BtpTypes::BtpPacketType BtpPacket::getBtpPacketType() const {
  return this->packetType;
}

BtpTypes::BtpContentType BtpPacket::getBtpContentType() const {
  return this->contentType;
}

uint32_t BtpPacket::getContentLength() const {
  return static_cast<uint32_t>(this->content.size());
}

const std::vector<std::byte>& BtpPacket::getContent() const {
  return this->content;
}

BtpTypes::IpType BtpPacket::parseIpType(std::span<const std::byte> headers) {
  BtpPacket::checkHeaderSize(headers);  // Throws exception on incorrect sizing
  // Refer to the protocol documentation for specific indices on packet members
  const auto networkData = headers.subspan(BtpPacket::IP_TYPE_OFFSET, 1);
  return static_cast<BtpTypes::IpType>(std::to_integer<uint8_t>(
    networkData[0]));
}

void BtpPacket::checkHeaderSize(std::span<const std::byte> headers) {
  if (headers.size() < BtpPacket::TOTAL_HEADERS_LENGTH) {
    throw std::runtime_error("BtpPacket header is of a smaller size than"
      " expected.");
  }
}

std::string BtpPacket::parseSourceIp(std::span<const std::byte> headers,
  BtpTypes::IpType ipType) {
  BtpPacket::checkHeaderSize(headers);  // Throws exception on incorrect sizing
  // Refer to the protocol documentation for specific indices on packet members
  const auto networkData = headers.subspan(BtpPacket::SOURCE_IP_OFFSET,
    BtpPacket::IP_MEMBER_SIZE);
  std::array<char, INET6_ADDRSTRLEN> ipString{};
  if (ipType == BtpTypes::IpType::IPv4) {
    if (::inet_ntop(AF_INET, networkData.data(), ipString.data(),
      INET_ADDRSTRLEN) != nullptr) {
      return std::string(ipString.data());
    }
  } else {
    if (::inet_ntop(AF_INET6, networkData.data(), ipString.data(),
      INET6_ADDRSTRLEN) != nullptr) {
      return std::string(ipString.data());
    }
  }
  throw std::runtime_error("Failed to parse IP address from BTP packet.");
}

std::string BtpPacket::parseDestinationIp(std::span<const std::byte> headers,
  BtpTypes::IpType ipType) {
  BtpPacket::checkHeaderSize(headers);  // Throws exception on incorrect sizing
  // Refer to the protocol documentation for specific indices on packet members
  const auto networkData = headers.subspan(BtpPacket::DESTINATION_IP_OFFSET,
    BtpPacket::IP_MEMBER_SIZE);
  std::array<char, INET6_ADDRSTRLEN> ipString{};
  if (ipType == BtpTypes::IpType::IPv4) {
    if (::inet_ntop(AF_INET, networkData.data(), ipString.data(),
      INET_ADDRSTRLEN) != nullptr) {
      return std::string(ipString.data());
    }
  } else {
    if (::inet_ntop(AF_INET6, networkData.data(), ipString.data(),
      INET6_ADDRSTRLEN) != nullptr) {
      return std::string(ipString.data());
    }
  }
  throw std::runtime_error("Failed to parse IP address from BTP packet.");
}

BtpTypes::BtpPacketType BtpPacket::parseBtpPacketType(std::span<const std::byte>
  headers) {
  BtpPacket::checkHeaderSize(headers);  // Throws exception on incorrect sizing
  // Refer to the protocol documentation for specific indices on packet members
  const auto networkData = headers.subspan(BtpPacket::PACKET_TYPE_OFFSET, 1);
  return static_cast<BtpTypes::BtpPacketType>(std::to_integer<uint8_t>(
    networkData[0]));
}

BtpTypes::BtpContentType BtpPacket::parseBtpContentType(std::span<const
  std::byte> headers) {
  BtpPacket::checkHeaderSize(headers);  // Throws exception on incorrect sizing
  // Refer to the protocol documentation for specific indices on packet members
  const auto networkData = headers.subspan(BtpPacket::CONTENT_TYPE_OFFSET, 1);
  return static_cast<BtpTypes::BtpContentType>(std::to_integer<uint8_t>(
    networkData[0]));
}

uint32_t BtpPacket::parseContentLength(std::span<const std::byte> headers) {
  BtpPacket::checkHeaderSize(headers);  // Throws exception on incorrect sizing
  // Refer to the protocol documentation for specific indices on packet members
  const auto networkData = headers.subspan(BtpPacket::CONTENT_LENGTH_OFFSET,
    BtpPacket::CONTENT_LENGTH_SIZE);
  uint32_t result{};
  for (size_t index{}; index < BtpPacket::CONTENT_LENGTH_SIZE; ++index) {
    // Extracts bytes one by one
    result |= static_cast<uint32_t>(
      std::to_integer<uint8_t>(networkData[index]) << (24 - index * 8));
  }
  return result;
}

std::ostream& operator<<(std::ostream& os, const BtpPacket& packet) {
  return os
    << "=== Btp packet ===\n"
    << "ip-type: " <<((packet.ipType == BtpTypes::IpType::IPv4)
                          ? "IPv4" : "IPv6") << '\n'
    << "source-ip: " << packet.sourceIp << '\n'
    << "destination-ip: " << packet.destinationIp << '\n'
    << "packet-type: " << BtpTypes::btpPacketTypeToString(
                            packet.packetType) << '\n'
    << "content-type: " << BtpTypes::btpPacketContentTypeToString(
                            packet.contentType) << '\n'
    << "content-length: " << packet.content.size() << '\n'
    << "content: " << BtpTypes::contentToMessage(packet.content);
}

BtpPacket BtpPacket::Builder::build() {
  return BtpPacket(
    std::move(this->content),
    this->sourceIp,
    this->destinationIp,
    this->ipType,
    this->packetType,
    this->contentType
  );
}

BtpPacket::Builder& BtpPacket::Builder::setIpType(BtpTypes::IpType type) {
  this->ipType = type;
  return *this;
}

BtpPacket::Builder& BtpPacket::Builder::setSourceIp(std::string ip) {
  this->sourceIp = std::move(ip);
  return *this;
}

BtpPacket::Builder& BtpPacket::Builder::setDestinationIp(std::string ip) {
  this->destinationIp = std::move(ip);
  return *this;
}

BtpPacket::Builder& BtpPacket::Builder::setBtpPacketType(BtpTypes::BtpPacketType
  type) {
  this->packetType = type;
  return *this;
}

BtpPacket::Builder& BtpPacket::Builder::setBtpContentType(
  BtpTypes::BtpContentType type) {
  this->contentType = type;
  return *this;
}

BtpPacket::Builder& BtpPacket::Builder::setContent(const std::vector<std::byte>&
  contentBuffer) {
  this->content = contentBuffer;
  return *this;
}

BtpPacket::Builder& BtpPacket::Builder::setContent(std::vector<std::byte>&&
  contentBuffer) {
  this->content = std::move(contentBuffer);
  return *this;
}

BtpPacket::Builder BtpPacket::builder() {
  return BtpPacket::Builder();
}

BtpPacket::BtpPacket(
  std::vector<std::byte>&& content,
  std::string_view sourceIp, 
  std::string_view destinationIp, 
  BtpTypes::IpType ipType,
  BtpTypes::BtpPacketType packetType,
  BtpTypes::BtpContentType contentType
) :
  content(std::move(content)),
  sourceIp(sourceIp),
  destinationIp(destinationIp),
  ipType(ipType),
  packetType(packetType),
  contentType(contentType)
{ }

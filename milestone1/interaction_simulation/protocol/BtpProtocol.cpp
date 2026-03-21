// Copyright 2026 Jason Montenegro <jasonmontenegro49@gmail.com>
#include "BtpProtocol.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "BtpPacket.hpp"
#include "ITransport.hpp"

BtpProtocol::BtpProtocol(ITransport& transport) : transport(transport) { }

void BtpProtocol::send(const BtpPacket &packet) {
  this->transport.send(BtpPacket::serializePacket(packet));
}

BtpPacket BtpProtocol::receive() {
  // Receive the headers first
  const auto headers = this->transport.receive(BtpPacket::TOTAL_HEADERS_LENGTH);
  BtpTypes::IpType ipType = BtpPacket::parseIpType(headers);
  const uint32_t contentLength = BtpPacket::parseContentLength(headers);
  // Set headers into builder
  auto builder = BtpPacket::builder()
    .setIpType(ipType)
    .setSourceIp(BtpPacket::parseSourceIp(headers, ipType))
    .setDestinationIp(BtpPacket::parseDestinationIp(headers, ipType))
    .setBtpPacketType(BtpPacket::parseBtpPacketType(headers))
    .setBtpContentType(BtpPacket::parseBtpContentType(headers));

  // Parse the content, set it to builder and build packet
  std::vector<std::byte> content;
  content.reserve(contentLength);
  size_t remainingBytes = contentLength;
  const size_t readChunkSize = 256;
  while (remainingBytes > 0) {
    const size_t chunkSize = std::min(remainingBytes, readChunkSize);
    const auto chunk = transport.receive(chunkSize);
    content.insert(content.cend(), chunk.cbegin(), chunk.cend());
    remainingBytes -= chunkSize;
  }
  return builder.setContent(std::move(content)).build();
}

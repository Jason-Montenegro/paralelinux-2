// Copyright 2026 Jason Montenegro <jasonmontenegro49@gmail.com>
#ifndef BTP_PACKET_HPP
#define BTP_PACKET_HPP

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <span>
#include <string>
#include <vector>

#include "BtpTypes.hpp"

class BtpProtocol;

/// @brief BtpPacket builder object for using BtpProtocol strategy.
class BtpPacket {
public:
  /// @brief Fixed value for handling the packet.
  static constexpr size_t TOTAL_HEADERS_LENGTH = 39;

  /// @brief Serializes packet to byte stream.
  /// @throws If any parsed IP is invalid
  /// @throws If content-length and actual content.size() is different.
  static std::vector<std::byte> serializePacket(const BtpPacket& packet);

  /// @brief Returns the IP type.
  BtpTypes::IpType getIpType() const;

  /// @brief Returns the source IP address.
  const std::string& getSourceIp() const;
  
  /// @brief Returns the destination IP address.
  const std::string& getDestinationIp() const;
  
  /// @brief Returns the BTP packet type.
  BtpTypes::BtpPacketType getBtpPacketType() const;
  
  /// @brief Returns the BTP content type.
  BtpTypes::BtpContentType getBtpContentType() const;
  
  /// @brief Returns the content length in bytes.
  uint32_t getContentLength() const;
  
  /// @brief Returns the packet content.
  const std::vector<std::byte>& getContent() const;

  /// @brief Parses IP type from headers.
  /// @throws std::runtime_error if headers have insufficient sizing.
  static BtpTypes::IpType parseIpType(std::span<const std::byte> headers);

  /// @brief Parses source IP from headers.
  /// @throws std::runtime_error if headers have insufficient sizing.
  /// @throws std::runtime_error when headers contain an unrecognizeable IP
  ///   address.
  static std::string parseSourceIp(std::span<const std::byte> headers,
    BtpTypes::IpType ipType);

  /// @brief Parses destination IP from headers.
  /// @throws std::runtime_error if headers have insufficient sizing.
  /// @throws std::runtime_error when headers contain an unrecognizeable IP
  ///   address.
  static std::string parseDestinationIp(std::span<const std::byte> headers,
    BtpTypes::IpType ipType);
  
  /// @brief Parses BTP packet type. 
  /// @throws std::runtime_error if headers have insufficient sizing.
  static BtpTypes::BtpPacketType parseBtpPacketType(std::span<const std::byte>
    headers);
  
  /// @brief Parses BTP content type. 
  /// @throws std::runtime_error if headers have insufficient sizing.
  static BtpTypes::BtpContentType parseBtpContentType(std::span<const std::byte>
    headers);
  
  /// @brief Parses content length. 
  /// @throws std::runtime_error if headers have insufficient sizing.
  static uint32_t parseContentLength(std::span<const std::byte> headers);

  /// @brief Printing overload.
  friend std::ostream& operator<<(std::ostream& os, const BtpPacket& packet);

  class Builder {
  public:
    /// @brief Construct default state builder.
    Builder() = default;

    /// @brief Constructs BtpPacket.
    /// @warning Calling this method invalidates the builder. Do not use it
    ///   if it was already called before.
    BtpPacket build();

    /// @brief Sets the IP type. 
    BtpPacket::Builder& setIpType(BtpTypes::IpType type);

    /// @brief Sets the source IP address.
    BtpPacket::Builder& setSourceIp(std::string ip);

    /// @brief Sets the destination IP address.
    BtpPacket::Builder& setDestinationIp(std::string ip);
    
    /// @brief Sets the BTP packet type.
    BtpPacket::Builder& setBtpPacketType(BtpTypes::BtpPacketType type);
    
    /// @brief Sets the BTP content type.
    BtpPacket::Builder& setBtpContentType(BtpTypes::BtpContentType type);
    
    /// @brief Sets the packet content.
    BtpPacket::Builder& setContent(const std::vector<std::byte>& contentBuffer);
    /// @brief Sets the packet content.
    BtpPacket::Builder& setContent(std::vector<std::byte>&& contentBuffer);

  private:
    std::vector<std::byte> content;
    std::string sourceIp;
    std::string destinationIp;
    BtpTypes::IpType ipType = BtpTypes::IpType::IPv4;
    BtpTypes::BtpPacketType packetType = BtpTypes::BtpPacketType::REQUEST;
    BtpTypes::BtpContentType contentType = BtpTypes::BtpContentType::TEXT_PLAIN;
  };

  friend BtpPacket::Builder;

  /// @brief Get a builder to build BtpPackets.
  static BtpPacket::Builder builder();

  /// @brief Copy ctor allows BtpPackets to have their own content, not share.
  BtpPacket(const BtpPacket& other) = default;
  BtpPacket& operator=(const BtpPacket& other) = default;
  /// @brief Move operations
  BtpPacket(BtpPacket&& other) noexcept = default;
  BtpPacket& operator=(BtpPacket&& other) noexcept = default;

private:
  explicit BtpPacket(
    std::vector<std::byte>&& content,
    std::string_view sourceIp, 
    std::string_view destinationIp, 
    BtpTypes::IpType ipType,
    BtpTypes::BtpPacketType packetType,
    BtpTypes::BtpContentType contentType
  );

  /// @brief Packet contents
  std::vector<std::byte> content;
  std::string sourceIp;
  std::string destinationIp;
  BtpTypes::IpType ipType = BtpTypes::IpType::IPv4;
  BtpTypes::BtpPacketType packetType = BtpTypes::BtpPacketType::REQUEST;
  BtpTypes::BtpContentType contentType = BtpTypes::BtpContentType::TEXT_PLAIN;

private:
  /// @brief Fixed values for manipulating the packet.
  static constexpr size_t IP_MEMBER_SIZE        = 16;
  static constexpr size_t IPV4_BYTE_SIZE        = 4;
  static constexpr size_t CONTENT_LENGTH_SIZE   = sizeof(uint32_t);

  /// @brief Packet offsets to manipulate the packet.
  static constexpr size_t IP_TYPE_OFFSET        = 0;
  static constexpr size_t SOURCE_IP_OFFSET      =
    BtpPacket::IP_TYPE_OFFSET + 1;
  static constexpr size_t DESTINATION_IP_OFFSET =
    BtpPacket::IP_MEMBER_SIZE + BtpPacket::SOURCE_IP_OFFSET;
  static constexpr size_t PACKET_TYPE_OFFSET    =
    BtpPacket::IP_MEMBER_SIZE + BtpPacket::DESTINATION_IP_OFFSET;
  static constexpr size_t CONTENT_TYPE_OFFSET   =
    BtpPacket::PACKET_TYPE_OFFSET + 1;
  static constexpr size_t CONTENT_LENGTH_OFFSET =
    BtpPacket::CONTENT_TYPE_OFFSET + 1;

  /// @brief Checks if the headers argument greater or equal to
  ///   BtpPacket::TOTAL_HEADERS_LENGTH.
  /// @throws std::runtime_error if headers argument is of a different size.
  static void checkHeaderSize(std::span<const std::byte> headers);

  /// @brief Serializes the IP type into the correct spot in the buffer.
  /// @throws std::runtime_error when IP address has invalid format or family.
  static void serializeIpType(const BtpPacket& packet,
    std::vector<std::byte>& result);

  /// @brief Serializes the source IP into the correct spot in the buffer.
  /// @throws std::runtime_error when IP address has invalid format or family.
  static void serializeSourceIp(const BtpPacket& packet,
    std::vector<std::byte>& result);

  /// @brief Serializes the destination IP into the correct spot in the buffer.
  /// @throws std::runtime_error when IP address has invalid format or family.
  static void serializeDestinationIp(const BtpPacket& packet,
    std::vector<std::byte>& result);

  /// @brief Serializes the packet type into the correct spot in the buffer.
  static void serializePacketType(const BtpPacket& packet,
    std::vector<std::byte>& result);

  /// @brief Serializes the content type into the correct spot in the buffer.
  static void serializeContentType(const BtpPacket& packet,
    std::vector<std::byte>& result);

  /// @brief Serializes the content length into the correct spot in the buffer.
  static void serializeContentLength(const BtpPacket& packet,
    std::vector<std::byte>& result);

  /// @brief Serializes the content into the correct spot in the buffer.
  static void serializeContent(const BtpPacket& packet,
    std::vector<std::byte>& result);
};

#endif  // BTP_PACKET_HPP

// Copyright 2026 Jason Montenegro <jasonmontenegro49@gmail.com>
#ifndef BTP_TYPES_HPP
#define BTP_TYPES_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <string>
#include <vector>

namespace BtpTypes {
  enum class IpType : uint8_t {
    IPv4 = 0x00,
    IPv6 = 0x01
  };

  enum class BtpPacketType : uint8_t {
    STOP        = 0x00,
    REQUEST     = 0x01,
    RESPONSE    = 0x02,
    UNREACHABLE = 0x03,
  };
  
  enum class BtpContentType : uint8_t {
    TEXT_PLAIN         = 0x01,  // text/plain
    TEXT_HTML_ALL_LEGO = 0x02,  // text/html/all-lego
    TEXT_ALL_LEGO      = 0x03,  // text/all-lego
    TEXT_LEGO          = 0x04,  // text/lego
    TEXT_ALL_FIGURES   = 0x05,  // text/all-figures
    TEXT_FIGURE        = 0x06,  // text/figure
  };

  /// @brief Convert BtpPacketType to a usable string.
  inline std::string btpPacketTypeToString(BtpPacketType type) {
    switch (type) {
    case BtpPacketType::REQUEST:     return "REQUEST";
    case BtpPacketType::RESPONSE:    return "RESPONSE";
    case BtpPacketType::STOP:        return "STOP";
    case BtpPacketType::UNREACHABLE: return "UNREACHABLE";
    default:                         return "NULL";
    }
  }

  /// @brief Convert BtpPacketContentType to a usable string.
  inline std::string btpPacketContentTypeToString(BtpContentType type) {
    switch (type) {
    case BtpContentType::TEXT_PLAIN:         return "text/plain";
    case BtpContentType::TEXT_HTML_ALL_LEGO: return "text/html/all-lego";
    case BtpContentType::TEXT_ALL_LEGO:      return "text/all-lego";
    case BtpContentType::TEXT_LEGO:          return "text/lego";
    case BtpContentType::TEXT_ALL_FIGURES:   return "all-figures";
    case BtpContentType::TEXT_FIGURE:        return "text/figure";
    default:                                 return "NULL";
    }
  }

  /// @brief Convert a string message to BtpContent.
  inline std::vector<std::byte> messageToBtpContent(std::string_view message) {
    std::vector<std::byte> buffer(message.length(), std::byte{});
    std::transform(message.cbegin(), message.cend(), buffer.begin(),
      [](char character) { return std::byte(static_cast<uint8_t>(character));
    });
    return buffer;
  }

  /// @brief Convert BtpContent to a usable string.
  inline std::string contentToMessage(std::span<const std::byte> content) {
    std::string message(content.size(), '\0');
    std::transform(content.begin(), content.end(), message.begin(),
      [](std::byte byte) {
        return static_cast<char>(std::to_integer<uint8_t>(byte));
    });
    return message;
  }
}

#endif  // BTP_TYPES_HPP

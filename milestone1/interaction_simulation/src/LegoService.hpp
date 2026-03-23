// Copyright 2026 Paralelinux-2
#ifndef LEGO_SERVICE_HPP
#define LEGO_SERVICE_HPP

#include <string>

#include "BtpPacket.hpp"
#include "BtpTypes.hpp"
#include "LegoRepository.hpp"

class LegoService {
public:
  /// @brief Creates response Btppacket.
  BtpPacket handle(const BtpPacket& request);

private:
  /// @brief Helper function for packet building.
  static BtpPacket buildResponse(const BtpPacket& request,
    BtpTypes::BtpContentType contentType, std::string&& content);

  LegoRepository legoRepository;
};

#endif  // LEGO_SERVICE_HPP

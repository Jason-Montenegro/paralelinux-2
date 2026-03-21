// Copyright 2026 Jason Montenegro <jasonmontenegro49@gmail.com>
#ifndef BTP_PROTOCOL_HPP
#define BTP_PROTOCOL_HPP

#include "BtpPacket.hpp"
#include "ITransport.hpp"

/// @brief BTP strategy object for transporting BTP packets.
class BtpProtocol {
public:
  BtpProtocol(ITransport& transport);

  /// @brief Sends BtpPackets through transport's send() method.
  void send(const BtpPacket& packet);

  /// @brief Receives a BtpPacket through transport's receive() method
  BtpPacket receive();

private:
  ITransport& transport;
};

#endif  // BTP_PROTOCOL_HPP

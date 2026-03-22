// Copyright 2026 Paralelinux-2
#ifndef CLIENT_CONSOLE_HANDLER_HPP
#define CLIENT_CONSOLE_HANDLER_HPP

#include <mutex>
#include <optional>
#include <string>
#include <tuple>

#include "BtpPacket.hpp"
#include "BtpTypes.hpp"

class ClientConsoleHandler {
public:
  ClientConsoleHandler(std::mutex& cout);

  /// @brief Prompts the user in the console for input to create a BtpPacket.
  std::optional<BtpPacket> promptBtpPacket(BtpTypes::IpType ipType,
    std::string_view sourceIp, std::string_view destinationIp);

  /// @brief Notifies an error message to the console.
  void notifyError(std::string_view errorMessage);

  /// @brief Shows a BtpPacket in the console.
  void showBtpPacket(const BtpPacket& packet);

private:
  /// @brief Helper function to print UI.
  void printRequestTypePrompt();
  
  /// @brief Helper function to print UI.
  void printContentPrompt(std::string_view inputOptionId) const;

  using IpAddressTuple = std::tuple<BtpTypes::IpType, std::string_view,
    std::string_view>;
  
  /// @brief Helper function to create a request BtpPacket.
  static BtpPacket createRequestPacket(const IpAddressTuple& ipTuple,
    BtpTypes::BtpContentType contentType);
    
    /// @brief Helper function to create a stop BtpPacket.
  static BtpPacket createStopPacket(const IpAddressTuple& ipTuple);

  static const std::array<std::pair<std::string, BtpTypes::BtpContentType>, 5>
    inputOptions;
  std::mutex& mtx;
};

#endif  // CLIENT_CONSOLE_HANDLER_HPP

// Copyright 2026 Paralelinux-2
#include "ClientConsoleHandler.hpp"

#include <array>
#include <iostream>
#include <mutex>
#include <optional>
#include <string>
#include <tuple>
#include <utility>

#include "BtpPacket.hpp"
#include "BtpTypes.hpp"

// ANSI escape codes for coloring output
#define ESC "\x1B"
#define END ESC"[0m"
#define BOLD ESC"[1m"
// When using colors, the macros don't transform to the number so it needs two
// more macros to do it. That's why NUM_TO_CSTR and COLOR_TO_NUM exist.
#define NUM_TO_CSTR(x) #x
#define COLOR_TO_NUM(x) NUM_TO_CSTR(x)
#define COLOR(c) ESC "[38;5;" COLOR_TO_NUM(c) "m"
#define BACKGROUND(c) ESC "[48;5;" COLOR_TO_NUM(c) "m"
#define RED 196
#define DARK_RED 160
#define BLK 0
#define WHITE 231
#define GRAY 251
#define YELLOW 230
#define ORANGE 208
#define LIGHT_ORANGE 215

const std::array<std::pair<std::string, BtpTypes::BtpContentType>, 5>
  ClientConsoleHandler::inputOptions{{
  {"PLAIN", BtpTypes::BtpContentType::TEXT_PLAIN},
  {"ALL_LEGO", BtpTypes::BtpContentType::TEXT_ALL_LEGO},
  {"LEGO", BtpTypes::BtpContentType::TEXT_LEGO},
  {"ALL_FIGURES", BtpTypes::BtpContentType::TEXT_ALL_FIGURES},
  {"FIGURE", BtpTypes::BtpContentType::TEXT_FIGURE},
}};

ClientConsoleHandler::ClientConsoleHandler(std::mutex& cout) : mtx(cout) { }

std::optional<BtpPacket> ClientConsoleHandler::promptBtpPacket(
  BtpTypes::IpType ipType, std::string_view sourceIp, std::string_view
  destinationIp) {
  std::lock_guard lock(this->mtx);
  this->printRequestTypePrompt();
  std::string input;
  std::getline(std::cin, input);
  if (input == "STOP") {
    return ClientConsoleHandler::createStopPacket(
      {ipType, sourceIp, destinationIp});
  }
  for (const auto& option : this->inputOptions) {
    if (input == option.first) {
      // Pass the handler string identifier
      this->printContentPrompt(option.first);
      // Pass the IP addresses so the handler can build the packet
      return ClientConsoleHandler::createRequestPacket(
        {ipType, sourceIp, destinationIp}, option.second);
    }
  }
  return std::nullopt;
}

void ClientConsoleHandler::printRequestTypePrompt() {
  static std::array<std::array<const char*, 2>, 5> arr{{
    {"PLAIN", "REQUEST text/plain"},
    {"ALL_LEGO", "REQUEST text/all-lego"},
    {"LEGO", "REQUEST text/lego"},
    {"ALL_FIGURES", "REQUEST text/all-figures"},
    {"FIGURE", "REQUEST text/figure"}
  }};
  // All numbers are ANSI color codes
  ::printf(BOLD "%s" END "\n", "Tipos de peticiones");
  for (const auto& line : arr) {
    ::printf(
      "\t" COLOR(ORANGE) "\"" END COLOR(YELLOW) "%s" END COLOR(ORANGE) "\"" END
      BOLD "%28s" END "\n", line[0], line[1]
    );
  }
  ::printf(
    BOLD "%s" END
    COLOR(DARK_RED) "\"" END BOLD COLOR(RED) "%s" END COLOR(DARK_RED) "\"" END
    BOLD "." END "\n", "Para terminar la simulación, ingrese: ", "STOP"
  );
  ::printf(BOLD "%s" END "\n", "Seleccione algún tipo de petición.");
  ::printf(BOLD "%s: " END, "Cliente");
}

void ClientConsoleHandler::printContentPrompt(std::string_view inputOptionId)
  const {
  printf(BOLD "%s" END "\n", "Ingrese el contenido de su paquete REQUEST.");
  printf(COLOR(LIGHT_ORANGE) "[%s]: " END, inputOptionId.data());
}

BtpPacket ClientConsoleHandler::createRequestPacket(const IpAddressTuple&
  ipTuple, BtpTypes::BtpContentType contentType) {
  std::string contentInput;
  std::getline(std::cin, contentInput);
  return BtpPacket::builder()
    .setIpType(std::get<BtpTypes::IpType>(ipTuple))
    .setSourceIp(std::string(std::get<1>(ipTuple)))
    .setDestinationIp(std::string(std::get<2>(ipTuple)))
    .setBtpPacketType(BtpTypes::BtpPacketType::REQUEST)
    .setBtpContentType(contentType)
    .setContent(BtpTypes::messageToBtpContent(contentInput))
    .build();
}

BtpPacket ClientConsoleHandler::createStopPacket(const IpAddressTuple&
  ipTuple) {
  // Content type and content attributes are irrelevant because STOP packet
  // type is for stopping a simulation.
  return BtpPacket::builder()
    .setIpType(std::get<BtpTypes::IpType>(ipTuple))
    .setSourceIp(std::string(std::get<1>(ipTuple)))
    .setDestinationIp(std::string(std::get<2>(ipTuple)))
    .setBtpPacketType(BtpTypes::BtpPacketType::STOP)
    .setBtpContentType(BtpTypes::BtpContentType::TEXT_PLAIN)
    .setContent({})
    .build();
}

void ClientConsoleHandler::notifyError(std::string_view errorMessage) {
  std::lock_guard lock(this->mtx);
  std::cout << "\n" BOLD  BACKGROUND(WHITE) " " COLOR(RED) << "[ERROR]: " END
    COLOR(GRAY) " " << errorMessage << " " END "\n" << std::endl;
}

void ClientConsoleHandler::showBtpPacket(const BtpPacket& packet) {
  std::lock_guard lock(this->mtx);
  std::cout << "\n" BOLD BACKGROUND(WHITE) COLOR(BLK)
    << " Enviaste el siguiente paquete BTP " << END "\n" BOLD << packet << END
    "\n" << std::endl;
}

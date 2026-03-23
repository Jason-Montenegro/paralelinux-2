// Copyright 2026 Paralelinux-2
#include "Client.hpp"

#include <condition_variable>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

#include "BtpPacket.hpp"
#include "BtpTypes.hpp"
#include "ClientConsoleHandler.hpp"
#include "ControlTuple.hpp"
#include "Log.hpp"
#include "ThreadSafeQueue.hpp"

Client::Client(ThreadSafeQueue<BtpPacket>& clientToIntermediary,
  ThreadSafeQueue<BtpPacket>& intermediaryToClient)
  : clientToIntermediary(clientToIntermediary),
    intermediaryToClient(intermediaryToClient) { }

void Client::run(ControlTuple intermediaryControl, std::mutex& coutMtx) {
  this->waitOnIntermediary(intermediaryControl);
  // Log must be already started from the program entry point
  Log& log = Log::getInstance();
  ClientConsoleHandler consoleHandler(coutMtx);

  while (true) {
    try {
      std::optional<BtpPacket> clientRequest = consoleHandler.promptBtpPacket(
        BtpTypes::IpType::IPv4, "127.0.0.1", "127.0.0.1");

      if (!clientRequest) {
        log.write(Log::Category::ERROR, "User provided wrong input at prompt.");
        consoleHandler.notifyError("No existe ninguna petición de ese tipo. "
          "Ingrese una opción de la lista existente.");
        continue;
      }

      const bool exitSimulation =
        clientRequest->getBtpPacketType() == BtpTypes::BtpPacketType::STOP;

      consoleHandler.showBtpPacket(*clientRequest);
      this->clientToIntermediary.push(std::move(*clientRequest));
      log.write(Log::Category::INFO, "Client sent a request packet to "
        "intermediary.");
      
      // This condition is only for simulation programs
      if (exitSimulation) {
        log.write(Log::Category::INFO, "Client thread is going offline.");
        break;
      }
    } catch (const std::runtime_error& err) {
      log.write(Log::Category::ERROR, "Client could not create BtpPacket "
        "request: " + std::string(err.what()));
    }

    // Wait for the response packet
    const BtpPacket intermediaryResponse =
      this->intermediaryToClient.wait_and_pop();
    log.write(Log::Category::INFO, "Client received intermediary response.");
    consoleHandler.showBtpPacket(intermediaryResponse);
  }
}

void Client::waitOnIntermediary(ControlTuple intermediaryControl) const {
  // Waits for Intermediary instance to be ready
  std::unique_lock lock(std::get<std::mutex&>(intermediaryControl));
  std::get<std::condition_variable&>(intermediaryControl).wait(lock,
    [&intermediaryControl] { return std::get<bool&>(intermediaryControl);
  });
}

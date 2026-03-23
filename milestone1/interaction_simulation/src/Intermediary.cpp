// Copyright 2026 Paralelinux-2
#include "Intermediary.hpp"

#include <condition_variable>
#include <mutex>
#include <tuple>
#include <utility>

#include "BtpPacket.hpp"
#include "ControlTuple.hpp"
#include "Log.hpp"
#include "ThreadSafeQueue.hpp"

Intermediary::Intermediary(ThreadSafeQueue<BtpPacket>& clientToIntermediary,
  ThreadSafeQueue<BtpPacket>& intermediaryToClient,
  ThreadSafeQueue<BtpPacket>& intermediaryToServer,
  ThreadSafeQueue<BtpPacket>& serverToIntermediary)
  : clientToIntermediary(clientToIntermediary),
    intermediaryToClient(intermediaryToClient),
    intermediaryToServer(intermediaryToServer),
    serverToIntermediary(serverToIntermediary) { }

void Intermediary::run(ControlTuple intermediaryControl,
  ControlTuple serverControl) {
  this->waitOnServer(serverControl);
  this->signalToClient(intermediaryControl);
  // Log must be already started from the program entry point
  Log& log = Log::getInstance();

  while (true) {
    BtpPacket clientRequest = this->clientToIntermediary.wait_and_pop();
    log.write(Log::Category::INFO, "Intermediary received client request "
      "packet.");
    log.write(Log::Category::INFO, "Intermediary redirecting packet to "
      "server.");
    const bool exitSimulation =
      clientRequest.getBtpPacketType() == BtpTypes::BtpPacketType::STOP;
    this->intermediaryToServer.push(std::move(clientRequest));

    // This condition is only for simulation programs.
    if (exitSimulation) {
      log.write(Log::Category::INFO, "Intermediary thread is going offline.");
      break;
    }

    // Wait for the response packet
    BtpPacket serverResponse = this->serverToIntermediary.wait_and_pop();
    log.write(Log::Category::INFO, "Intermediary received server request "
      "packet.");
    log.write(Log::Category::INFO, "Intermediary redirecting packet to "
      "client.");
    this->intermediaryToClient.push(std::move(serverResponse));
  }
}

void Intermediary::waitOnServer(ControlTuple serverControl) const {
  // Wait for server to be ready
  std::unique_lock lock(std::get<std::mutex&>(serverControl));
  std::get<std::condition_variable&>(serverControl).wait(lock,
    [&serverControl] { return std::get<bool&>(serverControl); });
}

void Intermediary::signalToClient(ControlTuple intermediaryControl) const {
  // Signal client that intermediary is ready
  std::lock_guard lock(std::get<std::mutex&>(intermediaryControl));
  std::get<bool&>(intermediaryControl) = true;
  std::get<std::condition_variable&>(intermediaryControl).notify_one();
}

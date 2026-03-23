// Copyright 2026 Paralelinux-2
#include "Server.hpp"

#include <condition_variable>
#include <mutex>
#include <tuple>

#include "BtpPacket.hpp"
#include "BtpTypes.hpp"
#include "ControlTuple.hpp"
#include "Log.hpp"
#include "ThreadSafeQueue.hpp"

Server::Server(ThreadSafeQueue<BtpPacket>& intermediaryToServer,
  ThreadSafeQueue<BtpPacket>& serverToIntermediary)
  : intermediaryToServer(intermediaryToServer),
    serverToIntermediary(serverToIntermediary) { }

void Server::run(ControlTuple serverControl) {
  this->signalToIntermediary(serverControl);
  // Log must be already started from the program entry point
  Log& log = Log::getInstance();

  while (true) {
    const BtpPacket request = this->intermediaryToServer.wait_and_pop();
    log.write(Log::Category::INFO, "Server received intermediary request "
      "packet.");

    if (request.getBtpPacketType() == BtpTypes::BtpPacketType::STOP) {
      log.write(Log::Category::INFO, "Server thread is going offline.");
      break;
    }

    BtpPacket response = this->legoService.handle(request);
    log.write(Log::Category::INFO, "Server redirecting packet to "
      "intermediary.");
    this->serverToIntermediary.push(std::move(response));
  }
}

void Server::signalToIntermediary(ControlTuple serverControl) const {
  // Signal intermediary that server is ready
  std::lock_guard lock(std::get<std::mutex&>(serverControl));
  std::get<bool&>(serverControl) = true;
  std::get<std::condition_variable&>(serverControl).notify_one();
}

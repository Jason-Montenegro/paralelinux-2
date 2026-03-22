// Copyright 2026 Paralelinux-2
#include "Client.hpp"

#include <condition_variable>
#include <mutex>
#include <tuple>

#include "BtpPacket.hpp"
#include "ControlTuple.hpp"
#include "ThreadSafeQueue.hpp"

Client::Client(ThreadSafeQueue<BtpPacket>& clientToIntermediary,
  ThreadSafeQueue<BtpPacket>& intermediaryToClient)
  : clientToIntermediary(clientToIntermediary),
    intermediaryToClient(intermediaryToClient) { }

void Client::run(ControlTuple intermediaryControl, std::mutex& coutMtx) {
  this->waitOnIntermediary(intermediaryControl);

  while (true) {
    // TODO: pending client interaction
    break;
  }
}

void Client::waitOnIntermediary(ControlTuple intermediaryControl) const {
  // Waits for Intermediary instance to be ready
  std::unique_lock lock(std::get<std::mutex&>(intermediaryControl));
  std::get<std::condition_variable&>(intermediaryControl).wait(lock,
    [&intermediaryControl] { return std::get<bool&>(intermediaryControl);
  });
}

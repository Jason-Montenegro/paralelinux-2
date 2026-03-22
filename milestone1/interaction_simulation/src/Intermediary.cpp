// Copyright 2026 Paralelinux-2
#include "Intermediary.hpp"

#include <condition_variable>
#include <mutex>
#include <tuple>

#include "BtpPacket.hpp"
#include "ControlTuple.hpp"
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
  ControlTuple serverControl, std::mutex& coutMtx) {
  this->waitOnServer(serverControl);
  this->signalToClient(intermediaryControl);

  while (true) {
    // TODO: pending intermediary interaction
    break;
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

// Copyright 2026 Paralelinux-2
#ifndef INTERMEDIARY_HPP
#define INTERMEDIARY_HPP

#include <mutex>

#include "BtpPacket.hpp"
#include "ControlTuple.hpp"
#include "ThreadSafeQueue.hpp"

class Intermediary {
public:
  Intermediary(ThreadSafeQueue<BtpPacket>& clientToIntermediary,
    ThreadSafeQueue<BtpPacket>& intermediaryToClient,
    ThreadSafeQueue<BtpPacket>& intermediaryToServer,
    ThreadSafeQueue<BtpPacket>& serverToIntermediary);

  /// @brief Run the intermediary simulation.
  void run(ControlTuple intermediaryControl, ControlTuple serverControl,
    std::mutex& coutMtx);

private:
  /// @brief Concurrency control helper.
  void waitOnServer(ControlTuple serverControl) const;
  
  /// @brief Concurrency control helper.
  void signalToClient(ControlTuple intermediaryControl) const;

  ThreadSafeQueue<BtpPacket>& clientToIntermediary;
  ThreadSafeQueue<BtpPacket>& intermediaryToClient;
  ThreadSafeQueue<BtpPacket>& intermediaryToServer;
  ThreadSafeQueue<BtpPacket>& serverToIntermediary;
};

#endif  // INTERMEDIARY_HPP

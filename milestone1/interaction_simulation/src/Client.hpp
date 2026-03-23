// Copyright 2026 Paralelinux-2
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <mutex>

#include "BtpPacket.hpp"
#include "ControlTuple.hpp"
#include "ThreadSafeQueue.hpp"

class Client {
public:
  Client(ThreadSafeQueue<BtpPacket>& clientToIntermediary,
    ThreadSafeQueue<BtpPacket>& intermediaryToClient);

  /// @brief Run the client simulation.
  void run(ControlTuple intermediaryControl, std::mutex& coutMtx);

private:
  /// @brief Concurrency control helper.
  void waitOnIntermediary(ControlTuple intermediaryControl) const;

  ThreadSafeQueue<BtpPacket>& clientToIntermediary;
  ThreadSafeQueue<BtpPacket>& intermediaryToClient;
};

#endif  // CLIENT_HPP

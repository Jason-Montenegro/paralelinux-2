// Copyright 2026 Paralelinux-2
#ifndef SERVER_HPP
#define SERVER_HPP

#include "BtpPacket.hpp"
#include "ControlTuple.hpp"
#include "LegoService.hpp"
#include "ThreadSafeQueue.hpp"

class Server {
public:
  Server(ThreadSafeQueue<BtpPacket>& intermediaryToServer,
    ThreadSafeQueue<BtpPacket>& serverToIntermediary);

  /// @brief Runs the server's simulation.
  void run(ControlTuple serverControl);

private:
  /// @brief Concurrency control helper.
  void signalToIntermediary(ControlTuple serverControl) const;

  ThreadSafeQueue<BtpPacket>& intermediaryToServer;
  ThreadSafeQueue<BtpPacket>& serverToIntermediary;
  LegoService legoService;
};

#endif  // SERVER_HPP

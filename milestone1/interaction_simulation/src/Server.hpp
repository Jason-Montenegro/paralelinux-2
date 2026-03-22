// Copyright 2026 Paralelinux-2
#ifndef SERVER_HPP
#define SERVER_HPP

#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "BtpPacket.hpp"
#include "ControlTuple.hpp"
#include "ThreadSafeQueue.hpp"

class Server {
public:
  Server(ThreadSafeQueue<BtpPacket>& intermediaryToServer,
    ThreadSafeQueue<BtpPacket>& serverToIntermediary);

  /// @brief Runs the server's simulation.
  void run(ControlTuple serverControl, std::mutex& coutMtx);

private:
  /// @brief Concurrency control helper.
  void signalToIntermediary(ControlTuple serverControl) const;

  ThreadSafeQueue<BtpPacket>& intermediaryToServer;
  ThreadSafeQueue<BtpPacket>& serverToIntermediary;

  /// @brief Associates an animal name to parts of lego pieces required.
  static const std::unordered_map
    <std::string, std::vector<std::vector<std::pair<size_t, std::string>>>>
    legoFigureRequirements;
  std::unordered_map<std::string, size_t> legoInventory;
};

#endif  // SERVER_HPP

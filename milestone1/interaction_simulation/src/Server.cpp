// Copyright 2026 Paralelinux-2
#include "Server.hpp"

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "BtpPacket.hpp"
#include "ControlTuple.hpp"
#include "ThreadSafeQueue.hpp"

const std::unordered_map
  <std::string, std::vector<std::vector<std::pair<size_t, std::string>>>>
  Server::legoFigureRequirements{
  {
    {
      "fish", {
        {  // Part 1
          {3, "brick 2x4 yellow"},
          {1, "brick 2x3 yellow"},
          {2, "brick 1x1 yellow eye"},
        },
        {  // Part 2
          {2, "brick 2x2 translucent blue"},
          {4, "brick 2x2 translucent red"},
        },
      }
    },
    {
      "duck", {
        {  // Part 1
          {1, "brick 2x2 slope orange"},
          {2, "brick 2x3 yellow"},
          {2, "brick 2x2 yellow"},
        },
        {  // Part 2
          {2, "brick 1x1 yellow eye"},
          {1, "brick 2x1 yellow"},
          {1, "brick 2x4 slope orange"},
        },
      }
    }
  }
};

Server::Server(ThreadSafeQueue<BtpPacket>& intermediaryToServer,
  ThreadSafeQueue<BtpPacket>& serverToIntermediary)
  : intermediaryToServer(intermediaryToServer),
    serverToIntermediary(serverToIntermediary) {
  // Copy the lego figure requirements as lego inventory
  for (const auto& figure : Server::legoFigureRequirements) {
    for (const auto& part : figure.second) {
      for (const auto& entry : part) {
        this->legoInventory[entry.second] = entry.first;
      }
    }
  }
}

void Server::run(ControlTuple serverControl, std::mutex& coutMtx) {
  this->signalToIntermediary(serverControl);

  while (true) {
    // TODO: pending server interaction
    break;
  }
}

void Server::signalToIntermediary(ControlTuple serverControl) const {
  // Signal intermediary that server is ready
  std::lock_guard lock(std::get<std::mutex&>(serverControl));
  std::get<bool&>(serverControl) = true;
  std::get<std::condition_variable&>(serverControl).notify_one();
}

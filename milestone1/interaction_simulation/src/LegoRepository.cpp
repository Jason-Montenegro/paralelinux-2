// Copyright 2026 Paralelinux-2
#include "LegoRepository.hpp"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

const std::unordered_map
  <std::string, std::vector<std::vector<std::pair<size_t, std::string>>>>
  LegoRepository::legoFigureRequirements{
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
    },
    {
      "amogus", {
        {  // Part 1
          {4, "brick 1x2 red"},
          {2, "brick 4x6 red"},
          {2, "brick 2x4 red"},
        },
        {  // Part 2
          {6, "brick 2x3 light blue"},
        },
      }
    },
  }
};

LegoRepository::LegoRepository() {
  // Initializes the inventory with the base lego figure requirements
  for (const auto& figure : LegoRepository::legoFigureRequirements) {
    for (const auto& part : figure.second) {
      for (const auto& entry : part) {
        this->legoInventory[entry.second] += entry.first;
      }
    }
  }
}

std::string LegoRepository::getAllLego() const {
  // Fetches all legos stored in the inventory
  std::string content;
  size_t remainingEntries = this->legoInventory.size();
  for (const auto& legoPiece : this->legoInventory) {
    content += "\"quantity\":" + std::to_string(legoPiece.second)
      + ",\"description\":\"" + legoPiece.first + "\"";
    --remainingEntries;
    if (remainingEntries > 0) {
      content += ",";
    }
  }
  return content;
}

std::string LegoRepository::getLego(std::string description) const {
  // Fetches the current inventory of a specific lego piece
  std::string content =
    "\"quantity\":0,\"description\":\"unknown description\"";
  for (const auto& legoPiece : this->legoInventory) {
    if (legoPiece.first == description) {
      content = "\"quantity\":" + std::to_string(legoPiece.second)
        + ",\"description\":\"" + legoPiece.first + "\"";
      break;
    }
  }
  return content;
}

std::string LegoRepository::getAllFigures() const {
  // Fetches the name of all name&part combinations
  std::string content;
  for (const auto& legoFigure : LegoRepository::legoFigureRequirements) {
    const size_t totalParts = legoFigure.second.size();
    size_t remainingParts = totalParts;

    for (size_t part{1}; part <= totalParts; ++part) {
      content += "\"" + legoFigure.first + "&" + std::to_string(part)
        + "\"";

      if (--remainingParts > 0) {
          content += ",";
        }
      }
    content += ",";
  }
  content.pop_back();
  return content;
}

std::string LegoRepository::getFigure(std::string name, size_t part)
  const {
  if (!LegoRepository::legoFigureRequirements.contains(name)
    || LegoRepository::legoFigureRequirements.at(name).size() < part
    || part == 0) {
    return "\"quantity\":0,\"description\":\"unknown figure\"";
  }
  // Fetches the pieces needed to build a name&part set
  size_t totalEntries = LegoRepository::legoFigureRequirements.at(
    name)[part - 1].size();
  std::string content;
  for (const auto& piece : LegoRepository::legoFigureRequirements.at(
    name)[part - 1]) {
    content += "\"quantity\":" + std::to_string(piece.first)
      + ",\"description\":\"" + piece.second + "\"";

    if (--totalEntries > 0) {
      content += ",";
    }
  }
  return content;
}

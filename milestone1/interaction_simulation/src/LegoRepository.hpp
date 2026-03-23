// Copyright 2026 Paralelinux-2
#ifndef LEGO_REPOSITORY_HPP
#define LEGO_REPOSITORY_HPP

#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class LegoRepository {
public:
  LegoRepository();

  /// @brief Returns all stored lego pieces in inventory.
  std::string getAllLego() const;

  /// @brief Finds a specific lego in inventory.
  std::string getLego(std::string description) const;

  /// @brief Returns all figures that can be made.
  std::string getAllFigures() const;

  /// @brief Returns all needed pieces for a figure&part.
  std::string getFigure(std::string name, size_t part) const;

private:
  /// @brief Associates a Lego figure to its required parts.
  static const std::unordered_map
    <std::string, std::vector<std::vector<std::pair<size_t, std::string>>>>
    legoFigureRequirements;
  /// @brief Current lego pieces inventory
  std::unordered_map<std::string, size_t> legoInventory;
};

#endif  // LEGO_REPOSITORY_HPP

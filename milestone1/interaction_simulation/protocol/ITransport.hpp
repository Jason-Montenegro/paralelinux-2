// Copyright 2026 Jason Montenegro <jasonmontenegro49@gmail.com>
#ifndef ITRANSPORT_HPP
#define ITRANSPORT_HPP

#include <cstddef>
#include <span>
#include <vector>

/**
 * Interface for transportation.
 * Defines the contract for transportations.
 * Concrete implementations are responsible for the underlying
 * transport mechanism (TCP, UDP, in-memory, etc.).
 */
class ITransport {
public:
  virtual ~ITransport() = default;

  /// @brief Sends data to destination.
  virtual void send(std::span<const std::byte> data) = 0;

  /// @brief Receives data from source.
  virtual std::vector<std::byte> receive(size_t bytes) = 0;
};

#endif  // ITRANSPORT_HPP

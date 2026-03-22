// Copyright 2026 Paralelinux-2
#ifndef CONTROL_TUPLE_HPP
#define CONTROL_TUPLE_HPP

#include <condition_variable>
#include <mutex>
#include <tuple>

using ControlTuple = std::tuple<std::mutex&, std::condition_variable&, bool&>;

#endif  // CONTROL_TUPLE_HPP

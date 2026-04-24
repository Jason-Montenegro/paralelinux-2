// Copyright (C) 2026 Jason Montenegro-Navarro <jasonmontenegro49@gmail.com>
#include <span>

#include "Repl.hpp"

int main(int argc, char* argv[]) {
  Repl repl(argc, std::span<char*>(argv, argc));
  repl.run();
}

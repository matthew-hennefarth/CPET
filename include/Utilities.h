// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef UTILITIES_H
#define UTILITIES_H

/* C++ STL HEADER FILES */
#include <fstream>
#include <functional>
#include <random>
#include <sstream>
#include <string>
#include <vector>

/* CPET HEADER FILES */
#include "Exceptions.h"

[[nodiscard]] inline std::unique_ptr<std::mt19937>&
randomNumberGenerator() noexcept {
  static thread_local std::unique_ptr<std::mt19937> generator = nullptr;
  if (generator == nullptr) {
    generator = std::make_unique<std::mt19937>(std::random_device()());
  }
  return generator;
}

void forEachLineIn(const std::string& file,
                   const std::function<void(const std::string&)>& func);

std::vector<std::string> split(std::string_view str, char delim);

[[nodiscard]] bool isDouble(std::string str) noexcept;

template <class InputIt, class UnaryPredicate>
InputIt find_if_ex(InputIt first, InputIt last, UnaryPredicate p) {
  if (auto loc = std::find_if(first, last, p); loc != last) {
    return loc;
  }
  throw cpet::value_not_found("Could not find element in container");
}

#endif  // UTILITIES_H

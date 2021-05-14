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
#include <memory>

#include <iostream>

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

[[nodiscard]] inline std::string lstrip(std::string_view str,
                                        std::string_view escape = " \t") {
  auto strBegin = str.find_first_not_of(escape);
  if (strBegin > str.size()) {
    return std::string{};
  }
  return str.substr(strBegin).data();
}

[[nodiscard]] inline std::string rstrip(std::string_view str,
                                        std::string_view escape = " \t") {
  auto strEnd = str.find_last_not_of(escape);
  if (strEnd == std::string::npos) {
    return str.data();
  }
  return {str.data(), strEnd + 1};
}

[[nodiscard]] inline std::string removeAfter(std::string_view str,
                                             std::string_view escape = " \t") {
  auto strEnd = str.find_first_of(escape);
  if (strEnd == std::string::npos) {
    return str.data();
  }
  return std::string{str.data(), strEnd};
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

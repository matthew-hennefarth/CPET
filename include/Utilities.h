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

/* EXTERNAL LIBRARY HEADER FILES */
#include "spdlog/spdlog.h"

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

[[nodiscard]] bool isDouble(const std::string& str) noexcept;

template <class T>
void filter(std::vector<T>& list, const T& remove = T()) noexcept(true) {
  for (typename std::vector<T>::size_type i = 0; i < list.size(); i++) {
    if (list[i] == remove) {
      list.erase(list.begin() + static_cast<long>(i));
      i--;
    }
  }
}

template <class InputIt, class UnaryPredicate>
InputIt find_if_ex(InputIt first, InputIt last, UnaryPredicate p) {
  if (auto loc = std::find_if(first, last, p); loc != last) {
    return loc;
  }
  throw cpet::value_not_found("Could not find element in container");
}

#endif  // UTILITIES_H

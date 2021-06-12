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

/* CPET HEADER FILES */
#include "Exceptions.h"
namespace cpet {
namespace util {

[[nodiscard]] inline std::unique_ptr<std::mt19937>&
randomNumberGenerator() noexcept {
  static thread_local std::unique_ptr<std::mt19937> generator = nullptr;
  if (generator == nullptr) {
    generator = std::make_unique<std::mt19937>(std::random_device()());
  }
  return generator;
}

[[nodiscard]] inline std::string lstrip(const std::string_view str,
                                        const std::string_view escape = " \t") {
  auto strBegin = str.find_first_not_of(escape);
  if (strBegin > str.size()) {
    return std::string{};
  }
  return str.substr(strBegin).data();
}

[[nodiscard]] inline std::string rstrip(const std::string_view str,
                                        const std::string_view escape = " \t") {
  auto strEnd = str.find_last_not_of(escape);
  if (strEnd == std::string::npos) {
    return str.data();
  }
  return {str.data(), strEnd + 1};
}

[[nodiscard]] inline std::string removeAfter(
    const std::string_view str, const std::string_view escape = " \t") {
  auto strEnd = str.find_first_of(escape);
  if (strEnd == std::string::npos) {
    return str.data();
  }
  return std::string{str.data(), strEnd};
}

[[nodiscard]] constexpr bool startswith(const std::string_view str,
                                        const std::string_view str2) noexcept {
  return str.rfind(str2, 0) != std::string::npos;
}

void forEachLineIn(const std::string& file,
                   const std::function<void(const std::string&)>& func);

std::vector<std::string> split(const std::string_view str, char delim);

[[nodiscard]] bool isDouble(std::string str) noexcept;

template <class InputIt, class UnaryPredicate>
InputIt find_if_ex(const InputIt first, const InputIt last,
                   const UnaryPredicate p) {
  if (auto loc = std::find_if(first, last, p); loc != last) {
    return loc;
  }
  throw cpet::value_not_found("Could not find element in container");
}

constexpr unsigned int countSetBits(unsigned int n) {
  unsigned int count = 0;
  while (n != 0) {
    count += n & 1;
    n >>= 1;
  }
  return count;
}

inline std::string tolower(const std::string_view str) {
  std::string result;
  result.reserve(str.size());
  std::transform(str.begin(), str.end(), std::back_inserter(result),
                 [](const char& c) { return std::tolower(c); });
  return result;
}
}  // namespace util
}  // namespace cpet
#endif  // UTILITIES_H

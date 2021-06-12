// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

/* C++ STL HEADER FILES */
#include <iostream>
#include <stdexcept>
#include <string>

namespace cpet {

class exception : public std::runtime_error {
 public:
  template <typename S1, typename = typename std::enable_if<
                             std::is_convertible_v<S1, std::string>>>
  explicit exception(S1&& what_arg)
      : std::runtime_error(std::forward<S1>(what_arg)) {}
};

class value_error : public cpet::exception {
 public:
  template <typename S1, typename = typename std::enable_if<
                             std::is_convertible_v<S1, std::string>>>
  explicit value_error(S1&& what_arg)
      : cpet::exception(std::forward<S1>(what_arg)) {}
};

class value_not_found : public cpet::exception {
 public:
  template <typename S1, typename = typename std::enable_if<
                             std::is_convertible_v<S1, std::string>>>
  explicit value_not_found(S1&& what_arg)
      : cpet::exception(std::forward<S1>(what_arg)) {}
};

class io_error : public cpet::exception {
 public:
  template <typename S1, typename = typename std::enable_if<
                             std::is_convertible_v<S1, std::string>>>
  explicit io_error(S1&& what_arg)
      : cpet::exception(std::forward<S1>(what_arg)) {}
};

class invalid_option : public cpet::exception {
 public:
  template <typename S1, typename = typename std::enable_if<
                             std::is_convertible_v<S1, std::string>>>
  explicit invalid_option(S1&& what_arg)
      : cpet::exception(std::forward<S1>(what_arg)) {}
};
}  // namespace cpet

#endif  // EXCEPTIONS_H

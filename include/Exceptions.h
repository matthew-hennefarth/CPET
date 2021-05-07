// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

/* C++ STL HEADER FILES */
#include <iostream>
#include <stdexcept>
#include <string>

namespace cpet {

class exception : std::runtime_error {
 public:
  explicit exception(const std::string& what_arg)
      : std::runtime_error(what_arg) {}
  explicit exception(const char* what_arg) : std::runtime_error(what_arg) {}
};

class value_error : cpet::exception {
 public:
  explicit value_error(const std::string& what_arg)
      : cpet::exception(what_arg) {}
  explicit value_error(const char* what_arg) : cpet::exception(what_arg) {}
};

class value_not_found : cpet::exception {
 public:
  explicit value_not_found(const std::string& what_arg)
      : cpet::exception(what_arg) {}
  explicit value_not_found(const char* what_arg) : cpet::exception(what_arg) {}
};

class io_error : cpet::exception {
 public:
  explicit io_error(const std::string& what_arg) : cpet::exception(what_arg) {}
  explicit io_error(const char* what_arg) : cpet::exception(what_arg) {}
};

}  // namespace cpet

#endif  // EXCEPTIONS_H

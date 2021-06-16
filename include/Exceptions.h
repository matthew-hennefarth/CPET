// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

/* C++ STL HEADER FILES */
#include <stdexcept>
#include <string>
#include <utility>

namespace cpet {

class exception : public std::runtime_error {
 public:
  template <typename StringType,
            typename = std::enable_if_t<
                std::is_convertible_v<StringType, std::string>>,
            typename = std::enable_if_t<!std::is_array_v<StringType>>>
  explicit exception(StringType&& what_arg)
      : std::runtime_error(std::forward<StringType>(what_arg)) {
  }  // NOLINT(hicpp-no-array-decay,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
};

class value_error : public cpet::exception {
 public:
  template <typename StringType,
            typename = std::enable_if_t<
                std::is_convertible_v<StringType, std::string>>,
            typename = std::enable_if_t<!std::is_array_v<StringType>>>
  explicit value_error(StringType&& what_arg)
      : cpet::exception(std::forward<StringType>(what_arg)) {}
};

class value_not_found : public cpet::exception {
 public:
  template <typename StringType,
            typename = std::enable_if_t<
                std::is_convertible_v<StringType, std::string>>,
            typename = std::enable_if_t<!std::is_array_v<StringType>>>
  explicit value_not_found(StringType&& what_arg)
      : cpet::exception(std::forward<StringType>(what_arg)) {}
};

class io_error : public cpet::exception {
 public:
  template <typename StringType,
            typename = std::enable_if_t<
                std::is_convertible_v<StringType, std::string>>,
            typename = std::enable_if_t<!std::is_array_v<StringType>>>
  explicit io_error(StringType&& what_arg)
      : cpet::exception(std::forward<StringType>(what_arg)) {}
};

class invalid_option : public cpet::exception {
 public:
  template <typename StringType,
            typename = std::enable_if_t<
                std::is_convertible_v<StringType, std::string>>,
            typename = std::enable_if_t<!std::is_array_v<StringType>>>
  explicit invalid_option(StringType&& what_arg)
      : cpet::exception(std::forward<StringType>(what_arg)) {}
};
}  // namespace cpet

#endif  // EXCEPTIONS_H

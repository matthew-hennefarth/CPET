// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

/* C++ STL HEADER FILES */
#if __cplusplus <= 199711L
  #include <algorithm>
#else
  #include <numeric>
#endif
#include <sstream>

/* CPET HEADER FILES */
#include "Utilities.h"

namespace cpet {
namespace util {
void forEachLineIn(const std::string& file,
                   const std::function<void(const std::string&)>& func) {
  std::fstream inFile(file);
  std::string line;
  if (inFile.is_open()) {
    while (std::getline(inFile, line)) {
      func(line);
    }
  } else {
    throw cpet::io_error("Could not open file " + file);
  }
}

std::vector<std::string> split(const std::string_view str, char delim) {
  std::vector<std::string> result;

  std::string::size_type start = 0;
  for (size_t i = 0; i < str.size(); i++) {
    if (str[i] == delim && start != i) {
      result.emplace_back(str.substr(start, i - start));
      start = i + 1;
    } else if (str[i] == delim && start == i) {
      start++;
    }
  }

  result.emplace_back(str.substr(start, str.size()));
  result.erase(remove(result.begin(), result.end(), ""), result.end());
  return result;
}

bool isDouble(std::string str) noexcept {
  double result{0};
  /* This removes the trailing whitespace */
  str.erase(str.find_last_not_of(" \t\n\r\f\v") + 1);
  auto i = std::istringstream(str);

  i >> result;
  return !i.fail() && i.eof();
}
}  // namespace util
}  // namespace cpet

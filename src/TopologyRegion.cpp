// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "TopologyRegion.h"

/* C++ STL HEADER FILES */
#include <utility>

/* CPET HEADER FILES */
#include "Exceptions.h"
#include "Utilities.h"

namespace cpet{
TopologyRegion cpet::TopologyRegion::fromSimple(
    const std::vector<std::string>& options) {
  constexpr size_t MIN_PARSE_TOKENS = 3;
  if (options.size() < MIN_PARSE_TOKENS) {
    throw cpet::invalid_option(
        "Invalid Option: topology expects at least 3 parameters");
  }
  if (!util::isDouble(options[0])) {
    throw cpet::invalid_option(
        "Invalid Option: number of samples should be numeric");
  }
  int samples = std::stoi(options[0]);
  if (samples < 0) {
    throw cpet::invalid_option(
        "Invalid Option: topology samples should be non-negative");
  }
  std::unique_ptr<Volume> vol = Volume::generateVolume(
      std::vector<std::string>{options.begin() + 1, options.end()});
  return {std::move(vol), samples, DEFAULT_STEP_SIZE};
}
}

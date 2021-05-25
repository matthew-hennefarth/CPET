// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "Volume.h"

/* C++ STL HEADER FILES */
#include <unordered_map>
#include <functional>

/* CPET HEADER FILES */
#include "Box.h"
#include "Exceptions.h"
#include "Utilities.h"

std::unique_ptr<Volume> makeBox(const std::vector<std::string>& options) {
  constexpr int BOX_PARAMETERS = 3;

  if (options.size() != BOX_PARAMETERS) {
    throw cpet::invalid_option(
        "Invalid Option: Box requires 3 values: h, w, l");
  }
  if (!std::all_of(options.begin(), options.end(), isDouble)) {
    throw cpet::invalid_option(
        "Invalid Option: Box requires 3 doubles, received other");
  }

  return std::make_unique<Box>(std::array<double, BOX_PARAMETERS>{
      std::stod(options[0]), std::stod(options[1]), std::stod(options[2])});
}

std::unique_ptr<Volume> Volume::generateVolume(
    std::vector<std::string> options) {
  static const std::unordered_map<
      std::string,
      std::function<std::unique_ptr<Volume>(const std::vector<std::string>&)>>
      volumeHash = {{"box", &makeBox}};

  if (options.empty()) {
    throw cpet::invalid_option("Invalid Option: no options to generate volume");
  }
  const auto key = options[0];
  const auto func = volumeHash.find(key);
  if (func == volumeHash.end()) {
    throw cpet::invalid_option("Invalid Option: Unsupported volume specified " +
                               key);
  }
  return func->second(
      std::vector<std::string>(options.begin() + 1, options.end()));
}

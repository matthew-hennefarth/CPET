#include "Volume.h"

#include <unordered_map>
#include <functional>

#include "Box.h"
#include "Exceptions.h"
#include "Utilities.h"

std::unique_ptr<Volume> makeBox(const std::vector<std::string>& options) {
  if (options.size() != 3) {
    throw cpet::invalid_option(
        "Invalid Option: Box requires 3 values: h, w, l");
  }
  for (const auto& dim : options) {
    if (!isDouble(dim)) {
      throw cpet::invalid_option(
          "Invalid Option: Box requires 3 doubles, received other: " + dim);
    }
  }
  return std::make_unique<Box>(std::array<double, 3>{
      std::stod(options[1]), std::stod(options[2]), std::stod(options[3])});
}

const std::unordered_map<
    std::string,
    std::function<std::unique_ptr<Volume>(const std::vector<std::string>&)>>
    volumeHash = {{"box", &makeBox}};

std::unique_ptr<Volume> Volume::generateVolume(
    std::vector<std::string> options) {
  if (options.empty()) {
    throw cpet::invalid_option("Invalid Option: no options to generate volume");
  }
  auto key = options[0];
  const auto func = volumeHash.find(key);
  if (func == volumeHash.end()) {
    throw cpet::invalid_option("Invalid Option: Unsupported volume specified " +
                               key);
  }
  return func->second(
      std::vector<std::string>(options.begin() + 1, options.end()));
}

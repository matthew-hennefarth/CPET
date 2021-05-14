// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "Option.h"

#include <array>

/* CPET HEADER FILES */
#include "Box.h"
#include "Utilities.h"

constexpr char BLOCK_START_IDENTIFIER = '%';
constexpr const char* BLOCK_END_SEQUENCE = "end";

Option::Option(const std::string& optionFile) {
  loadOptionsDataFromFile_(optionFile);
  parseSimpleOptions_();
  parseBlockOptions_();
}

void Option::loadOptionsDataFromFile_(const std::string& optionFile) {
  bool inBlock = false;
  std::vector<std::string> blocktmp;
  std::string currentBlockKey{};
  int line_number = 0;

  SPDLOG_DEBUG("Reading in options from {}", optionFile);
  forEachLineIn(optionFile, [&, this](const std::string& orig_line) {
    ++line_number;
    SPDLOG_DEBUG("{}...{}", line_number, orig_line);
    auto line = removeAfter(lstrip(orig_line), "#");
    if (line.empty()) {
      return;
    }

    if (line[0] == BLOCK_START_IDENTIFIER) {
      if (inBlock) {
        SPDLOG_ERROR("Error on line number {} in {}", line_number, optionFile);
        SPDLOG_ERROR("{}...{}", line_number, orig_line);
        throw cpet::invalid_option(
            "Invalid Option: Cannot specify block within a block");
      }
      inBlock = true;

      line = lstrip(line, "% \t");

      currentBlockKey = removeAfter(line);
      if (currentBlockKey.empty()) {
        SPDLOG_ERROR("Error on line number {} in {}", line_number, optionFile);
        SPDLOG_ERROR("{}...{}", line_number, orig_line);
        throw cpet::invalid_option("Invalid Option: No key specified");
      }

      line = line.substr(currentBlockKey.size());
      line = lstrip(line);

      if (!line.empty()) {
        blocktmp.emplace_back(line);
      }

    } else if (inBlock &&
               (line.find(BLOCK_END_SEQUENCE) == std::string::npos)) {
      blocktmp.emplace_back(line);
    } else if (inBlock) {
      line = line.substr(0, line.find(BLOCK_END_SEQUENCE));
      if (!line.empty()) {
        blocktmp.emplace_back(line);
      }
      this->blockOptions_.emplace(currentBlockKey, blocktmp);
      // RESET
      inBlock = false;
      currentBlockKey.clear();
      blocktmp.clear();
    } else {
      this->simpleOptions_.emplace_back(line);
    }
  });

  if (inBlock) {
    throw cpet::invalid_option(
        "Invalid Option: Block section not terminated with \'end\'");
  }
#if SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_DEBUG
  SPDLOG_DEBUG("Options parsed into");
  SPDLOG_DEBUG("Simple:\n");
  for (const auto& s : simpleOptions_) {
    SPDLOG_DEBUG(s);
  }
  SPDLOG_DEBUG("\nBlock:\n");
  for (const auto& [key, value] : blockOptions_) {
    SPDLOG_DEBUG("key: {}", key);
    for (const auto& s : value) {
      SPDLOG_DEBUG(s);
    }
  }
#endif
}

void Option::parseSimpleOptions_() {
  SPDLOG_DEBUG("Parsing simple options");
  for (const auto& line : simpleOptions_) {
    if (line.substr(0, 5) == "align") {
      std::vector<std::string> info = split(line.substr(5), ' ');
      if (info.empty() || info.size() == 2) {
        throw cpet::invalid_option(
            "Invalid Option: align expects 1 or 3 identifiers");
      }
      centerID = info[0];
      if (info.size() > 1) {
        direction1ID = info[1];
        direction2ID = info[2];
      }

    } else if (line.substr(0, 8) == "topology") {
      std::vector<std::string> info = split(line.substr(8), ' ');
      if (info.size() < 5) {
        throw cpet::invalid_option(
            "Invalid Option: topology expects 5 parameters");
      }
      if (info[0] == "box") {
        std::array<double, 3> dims = {std::stod(info[1]), std::stod(info[2]),
                                      std::stod(info[3])};
        int samples = std::stoi(info[4]);
        for (const auto& d : dims) {
          if (d <= 0) {
            throw cpet::invalid_option(
                "Invalid Option: box dimensions should be positive valued");
          }
        }
        if (samples < 0) {
          throw cpet::invalid_option(
              "Invalid Option: topology samples should be non-negative");
        }
        calculateEFieldTopology.emplace_back(std::make_unique<Box>(dims),
                                             std::stoi(info[4]));
      } else {
        throw cpet::invalid_option("Invalid Option: unknown volume");
      }
    } else if (line.substr(0, 5) == "field") {
      std::vector<std::string> info = split(line.substr(5), ' ');
      calculateEFieldPoints.emplace_back(info[0]);
    } else if (line.substr(0, 4) == "plot3d") {
      std::vector<std::string> info = split(line.substr(4), ' ');
      if (info[0] == "box") {
        std::array<double, 3> dims = {std::stod(info[1]), std::stod(info[2]),
                                      std::stod(info[3])};
        calculateEFieldVolumes.emplace_back(
            std::make_unique<Box>(dims),
            std::array<int, 3>{std::stoi(info[4]), std::stoi(info[5]),
                               std::stoi(info[6])});
      }
    }

  }
}
void Option::parseBlockOptions_() {
  SPDLOG_DEBUG("Parsing block options");
  for (const auto& [key, lines] : blockOptions_) {
  }
}


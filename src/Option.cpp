// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "Option.h"

#include <array>

/* CPET HEADER FILES */
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
  int line_number = 0; /* Only need this for printing error to user */

  SPDLOG_DEBUG("Reading in options from {}", optionFile);
  forEachLineIn(optionFile, [&, this](const std::string& orig_line) {
    ++line_number;
    SPDLOG_DEBUG("{}...{}", line_number, orig_line);
    auto line = removeAfter(lstrip(orig_line), "#");
    if (line.empty()) {
      return;
    }

    if (line[0] == BLOCK_START_IDENTIFIER) {
      /* Start of a new block section */
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
      /* We are in a block section, but have not reached the end */
      blocktmp.emplace_back(line);
    } else if (inBlock) {
      /* We are in a block section, but there is an end sequence */
      line = line.substr(0, line.find(BLOCK_END_SEQUENCE));
      if (!line.empty()) {
        blocktmp.emplace_back(line);
      }

      this->blockOptions_.emplace(currentBlockKey, blocktmp);
      /* Reset Temporary Holders */
      inBlock = false;
      currentBlockKey.clear();
      blocktmp.clear();
    } else {
      /* Just a simple input line */
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
    std::vector<std::string> info = split(line, ' ');

    if (info.empty()) {
      continue;
    }

    std::string key = info.at(0);
    if (parseSimpleOptionsMap_.find(key) == parseSimpleOptionsMap_.end()) {
      SPDLOG_WARN("Unknown key in simple options {}", key);
    } else {
      info.erase(info.begin()); /* pops first element */
      (this->*(parseSimpleOptionsMap_[key]))(info);
    }
  }
}
void Option::parseBlockOptions_() {
  SPDLOG_DEBUG("Parsing block options");
//  for (const auto& [key, lines] : blockOptions_) {
//  }
}


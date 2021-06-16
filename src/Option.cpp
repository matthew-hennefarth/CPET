// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "Option.h"

/* C++ STL HEADER FILES */
#include <array>

/* CPET HEADER FILES */
#include "Utilities.h"

namespace cpet {

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

  SPDLOG_DEBUG("Reading in options from {}", optionFile);
  util::forEachLineIn(optionFile, [&, this, line_number = 0](
                                      const std::string& orig_line) mutable {
    ++line_number;
    SPDLOG_DEBUG("{}...{}", line_number, orig_line);
    auto line = util::removeAfter(util::lstrip(orig_line), "#");
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
      line = util::lstrip(line, "% \t");
      currentBlockKey = util::removeAfter(line);

      if (currentBlockKey.empty()) {
        SPDLOG_ERROR("Error on line number {} in {}", line_number, optionFile);
        SPDLOG_ERROR("{}...{}", line_number, orig_line);
        throw cpet::invalid_option("Invalid Option: No key specified");
      }

      line = line.substr(currentBlockKey.size());
      line = util::lstrip(line);

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

      blockOptions_.emplace_back(std::make_pair(currentBlockKey, blocktmp));

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
  for (const auto& str : simpleOptions_) {
    SPDLOG_DEBUG(str);
  }
  SPDLOG_DEBUG("\nBlock:\n");
  for (const auto& [key, value] : blockOptions_) {
    SPDLOG_DEBUG("key: {}", key);
    for (const auto& str : value) {
      SPDLOG_DEBUG(str);
    }
  }
#endif
}

void Option::parseSimpleOptions_() {
  const std::unordered_map<std::string,
                           void (Option::*)(const std::vector<std::string>&)>
      parseSimpleOptionsMap = {
          {ALIGN_KEY, &Option::parseAlignSimple_},
          {TOPOLOGY_KEY, &Option::parseTopologySimple_},
          {FIELD_KEY, &Option::parseFieldSimple_},
          {PLOT_3D_KEY, &Option::parsePlot3dSimple_},
          {COORDINATE_START_INDEX_KEY, &Option::parseCoordinateStartSimple_},
          {COORDINATE_SKIP_INDEX_KEY, &Option::parseCoordinateSkipSimple_}};

  SPDLOG_DEBUG("Parsing simple options");

  std::for_each(simpleOptions_.begin(), simpleOptions_.end(),
                [this, &parseSimpleOptionsMap](const auto& line) {
                  const std::vector<std::string> info = util::split(line, ' ');
                  if (info.empty()) {
                    return;
                  }

                  const auto key = util::tolower(info.at(0));
                  const std::vector<std::string> key_options{info.begin() + 1,
                                                             info.end()};

                  if (const auto func = parseSimpleOptionsMap.find(key);
                      func == parseSimpleOptionsMap.end()) {
                    SPDLOG_WARN("Unknown key in simple options {}", key);
                  } else {
                    (this->*(func->second))(key_options);
                  }
                });
}
void Option::parseBlockOptions_() {
  SPDLOG_DEBUG("Parsing block options");
  const std::unordered_map<std::string,
                           void (Option::*)(const std::vector<std::string>&)>
      parseBlockOptionsMap = {{PLOT_3D_KEY, &Option::parsePlot3dBlock_},
                              {FIELD_KEY, &Option::parseFieldBlock_},
                              {TOPOLOGY_KEY, &Option::parseTopologyBlock_}};

  for (const auto& [key, lines] : blockOptions_) {
    if (const auto func = parseBlockOptionsMap.find(util::tolower(key));
        func == parseBlockOptionsMap.end()) {
      SPDLOG_WARN("Unknown block key: {}", key);
    } else {
      (this->*(func->second))(lines);
    }
  }
}
}  // namespace cpet

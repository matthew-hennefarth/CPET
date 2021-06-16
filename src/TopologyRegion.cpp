// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "TopologyRegion.h"

/* C++ STL HEADER FILES */
#include <utility>

/* EXTERNAL LIBRARY HEADER FILES */
#include <spdlog/spdlog.h>

/* CPET HEADER FILES */
#include "Exceptions.h"
#include "Utilities.h"
#include "System.h"
#include "Instrumentation.h"

namespace cpet {

void TopologyRegion::computeTopologyWith(const std::vector<System>& systems,
                                         int numberOfThreads) const {
  SPDLOG_INFO("======[Sampling topology]======");
  SPDLOG_INFO("[Volume ]   ==>> {}", volume_->description());
  SPDLOG_INFO("[Npoints]   ==>> {}", numberOfSamples_);
  SPDLOG_INFO("[Threads]   ==>> {}", numberOfThreads);
  SPDLOG_INFO("[STEP SIZE] ==>> {}", stepSize_);

  int index = 0;
  for (const auto& system : systems) {
    SPDLOG_INFO("=~=~=~=~[Trajectory {}]=~=~=~=~", index);
    std::vector<PathSample> results;
    {
      Timer t;
      results = system.electricFieldTopologyIn(numberOfThreads, *this);
    }

    if (sampleOutput_) {
      writeSampleOutput_(results, index);
    }

    ++index;
  }
}

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
TopologyRegion TopologyRegion::fromBlock(
    const std::vector<std::string>& options) {
  std::unique_ptr<Volume> vol{nullptr};                   // Required
  std::optional<int> samples{std::nullopt};               // Required
  std::optional<std::string> sampleOutput{std::nullopt};  // Optional
  double stepsize = DEFAULT_STEP_SIZE;                    // Optional

  constexpr const char* VOLUME_KEY = "volume";
  constexpr const char* SAMPLES_KEY = "samples";
  constexpr const char* STEP_SIZE_KEY = "stepsize";
  constexpr const char* SAMPLE_OUTPUT_KEY = "sampleoutput";

  for (const auto& line : options) {
    const auto tokens = util::split(line, ' ');
    if (tokens.size() < 2) {
      continue;
    }

    const auto key = util::tolower(*tokens.begin());
    const std::vector<std::string> key_options{tokens.begin() + 1,
                                               tokens.end()};

    if (key == VOLUME_KEY) {
      vol = Volume::generateVolume(key_options);
    } else if (key == SAMPLES_KEY) {
      if (!util::isDouble(*key_options.begin())) {
        throw cpet::invalid_option(
            "Invalid Option: topology requires samples to be numeric type");
      }
      samples = std::stoi(*key_options.begin());
    } else if (key == STEP_SIZE_KEY) {
      if (!util::isDouble(*key_options.begin())) {
        throw cpet::invalid_option(
            "Invalid Option: topology requires step size to be numeric type");
      }
      stepsize = std::stod(*key_options.begin());
    } else if (key == SAMPLE_OUTPUT_KEY) {
      sampleOutput = *key_options.begin();
    } else {
      SPDLOG_WARN("Unknown key specified in block topology: {}", key);
    }
  }

  if (!samples) {
    throw cpet::invalid_option(
        "Invalid Option: Number of samples not specified for topology "
        "sampling");
  }
  if (vol == nullptr) {
    throw cpet::invalid_option(
        "Invalid Option: No volume specified for topology sampling");
  }

  TopologyRegion result{std::move(vol), *samples, stepsize};

  if (sampleOutput) {
    result.sampleOutput(*sampleOutput);
  }

  return result;
}

void TopologyRegion::writeSampleOutput_(const std::vector<PathSample>& data,
                                        int index) const {
  if (!sampleOutput_) {
    return;
  }
  SPDLOG_DEBUG("Writing topology results");
  const std::string file =
      *sampleOutput_ + '_' + std::to_string(index) + ".top";

  std::ofstream outFile(file, std::ios::out);
  if (outFile.is_open()) {
    outFile << '#' << details() << '\n';
    /* TODO add options writing to this file...*/
    std::for_each(data.begin(), data.end(),
                  [&outFile](const auto& line) { outFile << line << '\n'; });
    outFile << std::flush;
  } else {
    SPDLOG_ERROR("Could not open file {}", file);
    throw cpet::io_error("Could not open file " + file);
  }
}

}  // namespace cpet

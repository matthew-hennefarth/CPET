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

  std::vector<std::vector<PathSample>> sampleResults;

  if (!analysisOnly()){
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
        results = system.electricFieldTopologyIn(numberOfThreads, *volume_, stepSize_, numberOfSamples_);
      }

      if (sampleOutput_) {
        writeSampleOutput_(results, index);
      }
      sampleResults.emplace_back(results);
      ++index;
    }
  }

  /* Check to see if we need to be here */
  if (computeMatrix()) {
    if (sampleInput_) {
      //sampleResults = loadSampleData_();
      /* Load the data from disk */
    }
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
  TopologyRegion result{std::move(vol), samples, DEFAULT_STEP_SIZE};
  result.sampleOutput("topology_sample");
  return result;
}
TopologyRegion TopologyRegion::fromBlock(
    const std::vector<std::string>& options) {
  bool analysisOnly{false};                      // Determines what is required

  std::unique_ptr<Volume> vol{nullptr};
  std::optional<int> samples{std::nullopt};
  std::optional<std::string> sampleOutput{std::nullopt};
  double stepsize = DEFAULT_STEP_SIZE;
  std::optional<std::string> sampleInput{std::nullopt};
  std::optional<std::array<int, 2>> bins{std::nullopt};


  constexpr const char* VOLUME_KEY = "volume";
  constexpr const char* SAMPLES_KEY = "samples";
  constexpr const char* STEP_SIZE_KEY = "stepsize";
  constexpr const char* SAMPLE_OUTPUT_KEY = "sampleoutput";
  constexpr const char* SAMPLE_INPUT_KEY = "sampleinput";
  constexpr const char* BINS_KEY = "bins";

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
    } else if (key == SAMPLE_INPUT_KEY) {
      sampleInput = *key_options.begin();
      analysisOnly = true;
    } else if (key == BINS_KEY) {
        if (key_options.size() < 2) {
          if (!util::isDouble(*key_options.begin())){
            throw cpet::invalid_option("Invalid Option: topology requires bin to be numeric");
          }
          bins = {std::stoi(*key_options.begin()), std::stoi(*key_options.begin())};
        } else {
          if (!util::isDouble(*key_options.begin()) || !util::isDouble(*(key_options.begin() + 1))) {
            throw cpet::invalid_option(
                "Invalid Option: topology requires bin to be numeric");
          }
          bins = {std::stoi(*key_options.begin()), std::stoi(*(key_options.begin()+1))};
        }
    } else {
      SPDLOG_WARN("Unknown key specified in block topology: {}", key);
    }
  }

  TopologyRegion result;
  if (!analysisOnly) {
    if (!samples) {
      throw cpet::invalid_option(
          "Invalid Option: Number of samples not specified for topology "
          "sampling");
    }
    result.numberOfSamples_ = *samples;
    if (vol == nullptr) {
      throw cpet::invalid_option(
          "Invalid Option: No volume specified for topology sampling");
    }
    result.volume_ = std::move(vol);
    result.stepSize_ = stepsize
        ;
    if (sampleOutput) {
      result.sampleOutput(*sampleOutput);
    }
  }

  if (sampleInput) {
    result.sampleInput(*sampleInput);
    if (!bins) {
      throw cpet::invalid_option("Invalid Option: sampleInput specified but no bins specified!");
    }
  }
  result.bins_ = bins;

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

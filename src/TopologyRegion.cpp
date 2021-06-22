// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "TopologyRegion.h"

/* C++ STL HEADER FILES */
#include <utility>
#include <filesystem>

/* EXTERNAL LIBRARY HEADER FILES */
#include <spdlog/spdlog.h>
#include <matplot/matplot.h>

/* CPET HEADER FILES */
#include "Exceptions.h"
#include "Utilities.h"
#include "System.h"
#include "Instrumentation.h"
#include "Histogram2D.h"

namespace cpet {

void TopologyRegion::computeTopologyWith(const std::vector<System>& systems,
                                         int numberOfThreads) const {
  std::vector<std::vector<PathSample>> sampleResults;

  if (!analysisOnly()) {
    assert(volume_ != nullptr);
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
        results = system.electricFieldTopologyIn(numberOfThreads, *volume_,
                                                 stepSize_, numberOfSamples_);
      }

      if (sampleOutput_) {
        writeSampleOutput_(results, index);
      }
      sampleResults.emplace_back(std::move(results));
      ++index;
    }
  }

  if (computeMatrix()) {
    assert(static_cast<bool>(bins_));
    SPDLOG_INFO("==[Computing Distance Matrix]==");
    if (sampleInput_) {
      sampleResults = loadSampleData_();
    }
    const auto histograms = constructHistograms_(sampleResults);
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
  bool analysisOnly{false};  // Determines what is required

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
        if (!util::isDouble(*key_options.begin())) {
          throw cpet::invalid_option(
              "Invalid Option: topology requires bin to be numeric");
        }
        bins = {std::stoi(*key_options.begin()),
                std::stoi(*key_options.begin())};
      } else {
        if (!util::isDouble(*key_options.begin()) ||
            !util::isDouble(*(key_options.begin() + 1))) {
          throw cpet::invalid_option(
              "Invalid Option: topology requires bin to be numeric");
        }
        bins = {std::stoi(*key_options.begin()),
                std::stoi(*(key_options.begin() + 1))};
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
    result.stepSize_ = stepsize;
    if (sampleOutput) {
      result.sampleOutput(*sampleOutput);
    }
  }

  if (sampleInput) {
    result.sampleInput(*sampleInput);
    if (!bins) {
      throw cpet::invalid_option(
          "Invalid Option: sampleInput specified but no bins specified!");
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
std::vector<std::vector<PathSample>> TopologyRegion::loadSampleData_() const {
  SPDLOG_INFO("Loading in already-sampled data");
  std::vector<std::vector<PathSample>> data;
  auto nextFileName = [&, index = 0]() mutable {
    return *sampleInput_ + '_' + std::to_string(index++) + ".top";
  };
  std::string filename;
  while (std::filesystem::exists(filename = nextFileName())) {
    SPDLOG_DEBUG("Loading in data from file {}", filename);
    std::vector<PathSample> tmpData;
    util::forEachLineIn(filename, [&,
                                   linenumber = 0](const auto& line) mutable {
      if (!line.empty() && !util::startswith(line, "#")) {
        const auto tokens = util::split(line, ',');
        if (tokens.size() != 2) {
          SPDLOG_WARN(
              "topology data file {} contains invalid number of entries on "
              "line {}",
              filename, linenumber);
        } else if (std::all_of(tokens.begin(), tokens.end(), util::isDouble)) {
          tmpData.emplace_back(
              PathSample{std::stod(tokens.at(0)), std::stod(tokens.at(1))});
        } else {
          SPDLOG_WARN(
              "topology data file {} has non-numeric types in data section in "
              "line {}",
              filename, linenumber);
        }
      }
      ++linenumber;
    });
    data.emplace_back(std::move(tmpData));
  }
  SPDLOG_INFO("Loaded in {} topology sample files", data.size());
  return data;
}
std::vector<std::vector<double>> TopologyRegion::constructHistograms_(
    const std::vector<std::vector<PathSample>>& sampleData) const {
  std::vector<std::vector<double>> histograms;

  const auto sampleDataFlatten = util::flatten(sampleData);

  const auto [xmin, xmax] = std::minmax_element(sampleDataFlatten.begin(), sampleDataFlatten.end(), [](const auto& ps1, const auto& ps2){
    return ps1.distance < ps2.distance;
  });
  const auto [ymin, ymax] = std::minmax_element(sampleDataFlatten.begin(), sampleDataFlatten.end(), [](const auto& ps1, const auto& ps2){
    return ps1.curvature < ps2.curvature;
  });

  const std::array xlim = {round(xmin->distance*1000.0)/1000.0, round(xmax->distance*1000.0)/1000.0};
  std::array ylim = {round(ymin->distance*1000.0)/1000.0, round(ymax->distance*1000.0)/1000.0};

  SPDLOG_INFO("====[Computing  Histograms]====");
  SPDLOG_INFO("[Bins] ==>> {} x {}", (*bins_)[0], (*bins_)[1]);
  SPDLOG_INFO("[XLim] ==>> [{}, {}]", xlim[0], xlim[1]);
  SPDLOG_INFO("[YLim] ==>> [{}, {}]", ylim[0], ylim[1]);

  for (const auto& samples : sampleData) {
    std::vector<double> curvatures;  // y
    std::vector<double> distances;   // x

    std::for_each(samples.begin(), samples.end(),
                  [&](const PathSample& sample) {
                    curvatures.emplace_back(sample.curvature);
                    distances.emplace_back(sample.distance);
                  });

    const auto temp_histo =
        histo::construct2DHistogram(distances, curvatures, *bins_, xlim, ylim);
    histograms.emplace_back(histo::normalize(util::flatten(temp_histo)));
  }

  return histograms;
}

}  // namespace cpet

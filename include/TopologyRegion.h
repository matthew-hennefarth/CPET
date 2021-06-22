// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef TOPOLOGYREGION_H
#define TOPOLOGYREGION_H

/* C++ STL HEADER FILES */
#include <memory>
#include <string>
#include <optional>
#include <utility>
#include <vector>

/* CPET HEADER FILES */
#include "Volume.h"
#include "PathSample.h"

namespace cpet {

constexpr double DEFAULT_STEP_SIZE = 0.001;

class System;

class TopologyRegion {
 public:
  TopologyRegion() = default;
  inline TopologyRegion(std::unique_ptr<Volume> vol, const int samples,
                        const double stepSize) noexcept
      : volume_(std::move(vol)),
        numberOfSamples_(samples),
        stepSize_(stepSize) {}

  [[nodiscard]] inline std::string details() const noexcept {
    return "Samples: " + std::to_string(numberOfSamples_) +
           "; Volume: " + volume_->description();
  }

  void computeTopologyWith(const std::vector<System>& systems,
                           int numberOfThreads) const;

  [[nodiscard]] constexpr bool computeMatrix() const noexcept {
    return static_cast<bool>(bins_);
  }

  [[nodiscard]] constexpr bool analysisOnly() const noexcept {
    return static_cast<bool>(sampleInput_);
  }

  [[nodiscard]] constexpr int numberOfSamples() const noexcept {
    return numberOfSamples_;
  }

  [[nodiscard]] constexpr double stepSize() const noexcept { return stepSize_; }

  inline void sampleOutput(const std::string& str) noexcept {
    if (!str.empty()) {
      sampleOutput_ = str;
    }
  }

  [[nodiscard]] constexpr const std::optional<std::string>& sampleOutput()
      const noexcept {
    return sampleOutput_;
  }

  inline void sampleInput(const std::string& str) noexcept {
    if (!str.empty()) {
      sampleInput_ = str;
    }
  }

  [[nodiscard]] constexpr const std::optional<std::string>& sampleInput()
      const noexcept {
    return sampleInput_;
  }

  [[nodiscard]] constexpr const std::optional<std::array<int, 2>>& bins()
      const noexcept {
    return bins_;
  }

  [[nodiscard]] static TopologyRegion fromSimple(
      const std::vector<std::string>& options);

  [[nodiscard]] static TopologyRegion fromBlock(
      const std::vector<std::string>& options);

 private:
  std::unique_ptr<Volume> volume_{nullptr};
  int numberOfSamples_;
  double stepSize_{DEFAULT_STEP_SIZE};
  std::optional<std::string> sampleOutput_{std::nullopt};
  std::optional<std::string> sampleInput_{std::nullopt};
  std::optional<std::string> matrixOutput_{std::nullopt};
  std::optional<std::array<int, 2>> bins_{std::nullopt};

  void writeSampleOutput_(const std::vector<PathSample>& data, int index) const;

  void writeMatrixOutput_(const std::vector<std::vector<double>>& matrix) const;

  [[nodiscard]] std::vector<std::vector<PathSample>> loadSampleData_() const;

  [[nodiscard]] std::vector<std::vector<double>> constructHistograms_(
      const std::vector<std::vector<PathSample>>& sampleData) const;

  [[nodiscard]] static std::vector<std::vector<double>> constructMatrix_(
      const std::vector<std::vector<double>>& histograms);
};
}  // namespace cpet
#endif  // TOPOLOGYREGION_H

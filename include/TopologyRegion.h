// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef TOPOLOGYREGION_H
#define TOPOLOGYREGION_H

/* C++ STL HEADER FILES */
#include <memory>
#include <string>
#include <optional>

/* CPET HEADER FILES */
#include "Volume.h"
#include "PathSample.h"

namespace cpet {

constexpr double DEFAULT_STEP_SIZE = 0.001;

class System;

class TopologyRegion {
 public:
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

  [[nodiscard]] inline const Volume& volume() const noexcept {
    return *volume_;
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

  [[nodiscard]] static TopologyRegion fromSimple(
      const std::vector<std::string>& options);

  [[nodiscard]] static TopologyRegion fromBlock(
      const std::vector<std::string>& options);

 private:
  std::unique_ptr<Volume> volume_;
  int numberOfSamples_;
  double stepSize_{DEFAULT_STEP_SIZE};
  std::optional<std::string> sampleOutput_{std::nullopt};

  void writeSampleOutput_(const std::vector<PathSample>& data, int index) const;
};
}  // namespace cpet
#endif  // TOPOLOGYREGION_H

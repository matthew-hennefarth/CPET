// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef EFIELDVOLUME_H
#define EFIELDVOLUME_H

/* C++ STL HEADER FILES */
#include <array>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <optional>
#include <unordered_map>

#include <Eigen/Dense>

/* CPET HEADER FILES */
#include "Volume.h"

class System;

class EFieldVolume {
  public:
  EFieldVolume(std::unique_ptr<Volume> vol, std::array<int, 3> density,
               bool plot = false) noexcept
      : volume_(std::move(vol)), sampleDensity_(density), showPlot_(plot) {
    points_ = volume_->partition(sampleDensity_);
  }

  [[nodiscard]] inline std::string name() const noexcept {
    std::string result = volume_->type() + '_';

    for (size_t i = 0; i < sampleDensity_.size() - 1; i++) {
      result += std::to_string(sampleDensity_.at(i)) + '-';
    }
    result += std::to_string(sampleDensity_.at(sampleDensity_.size() - 1));

    return result;
  }

  [[nodiscard]] inline std::string details() const noexcept {
    return ("Sample Density: " + std::to_string(sampleDensity_[0]) + ' ' +
            std::to_string(sampleDensity_[1]) + ' ' +
            std::to_string(sampleDensity_[2]) +
            "; Volume: " + volume_->description());
  }

  void plot(const std::vector<Eigen::Vector3d>& electricField) const;

  void writeVolumeResults(
      const std::vector<System>& systems,
      const std::vector<std::vector<Eigen::Vector3d>>& results) const;

  [[nodiscard]] inline const Volume& volume() const noexcept {
    return *volume_;
  }

  [[nodiscard]] constexpr const std::array<int, 3>& sampleDensity()
      const noexcept {
    return sampleDensity_;
  }

  [[nodiscard]] constexpr const std::vector<Eigen::Vector3d>& points()
      const noexcept {
    return points_;
  }

  [[nodiscard]] constexpr bool showPlot() const noexcept { return showPlot_; }

  [[nodiscard]] constexpr const std::optional<std::string>& output()
      const noexcept {
    return output_;
  }

  inline void output(const std::string& outputFile) {
    if (!outputFile.empty()) {
      output_ = outputFile;
    }
  }

  public:
  [[nodiscard]] static EFieldVolume fromSimple(
      const std::vector<std::string>& options);

  [[nodiscard]] static EFieldVolume fromBlock(
      const std::vector<std::string>& options);

 private:
  std::unique_ptr<Volume> volume_;

  std::array<int, 3> sampleDensity_;

  std::vector<Eigen::Vector3d> points_;

  bool showPlot_{false};

  std::optional<std::string> output_{std::nullopt};
};
#endif  // EFIELDVOLUME_H

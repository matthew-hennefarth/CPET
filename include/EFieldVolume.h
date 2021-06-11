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
#include <numeric>
#include <cassert>

#include <Eigen/Dense>

/* CPET HEADER FILES */
#include "Volume.h"

namespace cpet {

class System;

class EFieldVolume {
 public:
  EFieldVolume(std::unique_ptr<Volume> vol, std::array<int, 3> density,
               bool plot = false,
               std::optional<std::string> output = std::nullopt) noexcept
      : volume_(std::move(vol)),
        sampleDensity_(density),
        showPlot_(plot),
        output_(std::move(output)) {
    points_ = volume_->partition(sampleDensity_);
  }

  [[nodiscard]] inline std::string name() const {
    assert(!sampleDensity_.empty());

    constexpr auto append_density = [](const std::string& res,
                                       const int density) -> std::string {
      return res + std::to_string(density) + '-';
    };

    auto result =
        std::accumulate(sampleDensity_.begin(), sampleDensity_.end() - 1,
                        volume_->type() + '_', append_density);

    result += std::to_string(*sampleDensity_.rbegin());

    return result;
  }

  [[nodiscard]] inline std::string details() const noexcept {
    return ("Sample Density: " + std::to_string(sampleDensity_[0]) + ' ' +
            std::to_string(sampleDensity_[1]) + ' ' +
            std::to_string(sampleDensity_[2]) +
            "; Volume: " + volume_->description());
  }

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

  template <typename S1>
  inline void output(S1&& outputFile) {
    if (!outputFile.empty()) {
      output_ = std::forward<S1>(outputFile);
    }
  }

  [[nodiscard]] static EFieldVolume fromSimple(
      const std::vector<std::string>& options);

  [[nodiscard]] static EFieldVolume fromBlock(
      const std::vector<std::string>& options);

  void computeVolumeWith(const std::vector<System>& systems) const;

 private:
  std::unique_ptr<Volume> volume_;
  std::array<int, 3> sampleDensity_;
  std::vector<Eigen::Vector3d> points_;
  bool showPlot_{false};
  std::optional<std::string> output_{std::nullopt};

  void plot_(const std::vector<Eigen::Vector3d>& electricField) const;

  void writeOutput_(
      const std::vector<System>& systems,
      const std::vector<std::vector<Eigen::Vector3d>>& results) const;

};
}  // namespace cpet
#endif  // EFIELDVOLUME_H

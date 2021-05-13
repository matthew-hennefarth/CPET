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

#include <Eigen/Dense>

/* CPET HEADER FILES */
#include "Volume.h"

struct EFieldVolume {
  std::unique_ptr<Volume> volume;

  std::array<int, 3> sampleDensity;

  std::vector<Eigen::Vector3d> points;

  bool showPlot{true};

  EFieldVolume(std::unique_ptr<Volume> vol, std::array<int, 3> density,
               bool plot = true) noexcept
      : volume(std::move(vol)), sampleDensity(density), showPlot(plot) {
    points = volume->partition(sampleDensity);
  }

  [[nodiscard]] inline std::string name() const noexcept {
    std::string result = volume->type() + '_';

    for (size_t i = 0; i < sampleDensity.size() - 1; i++) {
      result += std::to_string(sampleDensity.at(i)) + '-';
    }
    result += std::to_string(sampleDensity.at(sampleDensity.size() - 1));

    return result;
  }

  [[nodiscard]] inline std::string details() const noexcept {
    return ("Sample Density: " + std::to_string(sampleDensity[0]) + ' ' +
            std::to_string(sampleDensity[1]) + ' ' +
            std::to_string(sampleDensity[2]) +
            "; Volume: " + volume->description());
  }

  void plot(const std::vector<Eigen::Vector3d>& electricField) const; 

};
#endif  // EFIELDVOLUME_H

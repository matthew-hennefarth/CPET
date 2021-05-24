// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef BOX_H
#define BOX_H

/* C++ STL HEADER FILES */
#include <random>
#include <string>
#include <vector>

/* EXTERNAL LIBRARY HEADER FILES */
#include <Eigen/Dense>
#include <spdlog/spdlog.h>

/* CPET HEADER FILES */
#include "Exceptions.h"
#include "Utilities.h"
#include "Volume.h"

class Box : public Volume {
 public:
  explicit inline Box(const std::array<double, 3>& sides) : sides_(sides) {
    for (const auto& val : sides_) {
      if (val < 0) {
        SPDLOG_ERROR("Invalid value for box side length {}", val);
        throw cpet::value_error("Invalid value for box side length");
      }
    }
  }

  [[nodiscard]] inline const double& maxDim() const noexcept override {
    return *std::max_element(sides_.begin(), sides_.end());
  }

  [[nodiscard]] inline double diagonal() const noexcept {
    const double diag =
        std::inner_product(sides_.begin(), sides_.end(), sides_.begin(), 0.0);

    return 2 * sqrt(diag);
  }

  [[nodiscard]] inline bool isInside(
      const Eigen::Vector3d& position) const override {
    for (size_t i = 0; i < 3; i++) {
      if (abs(position[static_cast<long>(i)]) >= sides_.at(i)) {
        return false;
      }
    }
    return true;
  }

  [[nodiscard]] inline Eigen::Vector3d randomPoint() const noexcept override {
    Eigen::Vector3d result;

    std::array<std::uniform_real_distribution<double>, 3> distribution;
    initializeDistributions_(distribution);

    int i = 0;
    for (auto& dis : distribution) {
      result[i] = dis(*(randomNumberGenerator()));
      i++;
    }
    return result;
  }

  [[nodiscard]] inline std::string description() const noexcept override {
    std::string result = "Box:";
    for (const auto& dim : sides_) {
      result += (" " + std::to_string(dim));
    }
    return result;
  }

  [[nodiscard]] inline int randomDistance(
      double stepSize) const noexcept override {
    std::uniform_int_distribution<int> distribution(
        1, static_cast<int>(diagonal() / stepSize));
    return distribution(*randomNumberGenerator());
  }

  [[nodiscard]] inline std::string type() const noexcept override {
    return "box";
  }

  [[nodiscard]] inline std::vector<Eigen::Vector3d> partition(
      const std::array<int, 3>& density) const noexcept override {
    /* Prevents division by zero later */
    for (const auto& dens : density) {
      /* Replace by approx */
      if (dens == 0.0) {
        return {};
      }
    }
    double x{0};
    double y{0};
    double z{0};

    std::vector<Eigen::Vector3d> result;

    result.reserve(
        static_cast<size_t>(abs(density[0] * density[1] * density[2])));

    x = -1 * sides_[0];
    while (x <= sides_[0]) {
      y = -1 * sides_[1];
      while (y <= sides_[1]) {
        z = -1 * sides_[2];
        while (z <= sides_[2]) {
          result.emplace_back(x, y, z);
          z += static_cast<double>(static_cast<float>(sides_[2] / density[2]));
        }
        y += (sides_[1] / density[1]);
      }
      x += (sides_[0] / density[0]);
    }
    return result;
  }

 private:
  inline void initializeDistributions_(
      std::array<std::uniform_real_distribution<double>, 3>& distribution)
      const noexcept {
    for (size_t i = 0; i < distribution.size(); i++) {
      distribution.at(i) = std::uniform_real_distribution<double>(
          -1 * sides_.at(i), sides_.at(i));
    }
  }

  std::array<double, 3> sides_;
};

#endif  // BOX_H

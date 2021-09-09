// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef BOX_H
#define BOX_H

/* C++ STL HEADER FILES */
#include <random>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>

/* EXTERNAL LIBRARY HEADER FILES */
#include <Eigen/Dense>
#include <spdlog/spdlog.h>

/* CPET HEADER FILES */
#include "Exceptions.h"
#include "Utilities.h"
#include "Volume.h"
namespace cpet {
class Box : public Volume {
 public:
  explicit inline Box(const std::array<double, 3>& sides) : sides_(sides) {
    constexpr auto is_less_than_zero = [](const double side) -> bool {
      return side < 0.0;
    };
    if (const auto& location =
            std::find_if(sides_.begin(), sides_.end(), is_less_than_zero);
        location != sides_.end()) {
      SPDLOG_ERROR("Invalid value for box side length {}", *location);
      throw cpet::value_error("Invalid value for box side length");
    }
  }

  inline Box(const std::array<double, 3>& sides, const Eigen::Vector3d& center) : sides_(sides) {
    constexpr auto is_less_than_zero = [](const double side) -> bool {
      return side < 0.0;
    };
    if (const auto& location =
          std::find_if(sides_.begin(), sides_.end(), is_less_than_zero);
        location != sides_.end()) {
      SPDLOG_ERROR("Invalid value for box side length {}", *location);
      throw cpet::value_error("Invalid value for box side length");
    }
    center_ = center;
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
    const Eigen::Vector3d displaced = position - center_;
    for (size_t i = 0; i < 3; i++) {
      if (abs(displaced[static_cast<long>(i)]) >= sides_.at(i)) {
        return false;
      }
    }
    return true;
  }

  [[nodiscard]] inline Eigen::Vector3d randomPoint() const noexcept override {
    std::array<std::uniform_real_distribution<double>, 3> distribution;
    initializeDistributions_(distribution);
    constexpr auto getRandomNumber =
        [](std::uniform_real_distribution<double>& dis) -> double {
      return dis(*(util::randomNumberGenerator()));
    };

    Eigen::Vector3d result;
    std::transform(distribution.begin(), distribution.end(), result.begin(),
                   getRandomNumber);
    return result + center_;
  }

  [[nodiscard]] inline std::string description() const noexcept override {
    constexpr auto append_string = [](const std::string& sum,
                                      const double dim) {
      return sum + ' ' + std::to_string(dim);
    };

    return std::accumulate(sides_.begin(), sides_.end(), std::string("Box:"),
                           append_string);
  }

  [[nodiscard]] inline int randomDistance(
      double stepSize) const noexcept override {
    std::uniform_int_distribution<int> distribution(
        1, static_cast<int>(diagonal() / stepSize));
    return distribution(*util::randomNumberGenerator());
  }

  [[nodiscard]] inline std::string type() const noexcept override {
    return "box";
  }

  [[nodiscard]] inline std::vector<Eigen::Vector3d> partition(
      const std::array<int, 3>& density) const noexcept override {
    /* Prevents division by zero later */
    constexpr auto is_zero = [](const int dens) -> bool { return dens == 0.0; };
    if (std::any_of(density.begin(), density.end(), is_zero)) {
      return {};
    }

    std::vector<Eigen::Vector3d> result;
    result.reserve(
        static_cast<size_t>(abs(density[0] * density[1] * density[2])));

    double x = -1 * sides_[0];
    while (x <= sides_[0]) {
      double y = -1 * sides_[1];
      while (y <= sides_[1]) {
        double z = -1 * sides_[2];
        while (z <= sides_[2]) {
          result.emplace_back(x + center_[0], y + center_[1], z + center_[2]);
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
}  // namespace cpet
#endif  // BOX_H

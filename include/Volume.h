// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef VOLUME_H
#define VOLUME_H

/* C++ STL HEADER FILES */
#include <string>
#include <vector>

/* EXTERNAL LIBRARY HEADER FILES */
#include <Eigen/Dense>

class Volume {
 public:
  Volume() = default;

  Volume(const Volume &) = default;

  Volume(Volume &&) = default;

  Volume &operator=(const Volume &) = default;

  Volume &operator=(Volume &&) = default;

  virtual ~Volume() = default;

  [[nodiscard]] virtual bool isInside(
      const Eigen::Vector3d &position) const = 0;

  [[nodiscard]] virtual const double &maxDim() const noexcept = 0;

  [[nodiscard]] virtual Eigen::Vector3d randomPoint() const = 0;

  [[nodiscard]] virtual std::string description() const noexcept(true) = 0;

  [[nodiscard]] virtual int randomDistance(double stepSize) const noexcept = 0;

  [[nodiscard]] virtual std::string type() const noexcept = 0;

  [[nodiscard]] virtual std::vector<Eigen::Vector3d> partition(
      const std::array<int, 3> &density) const noexcept = 0;
};

#endif  // VOLUME_H

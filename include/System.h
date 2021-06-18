// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef SYSTEM_H
#define SYSTEM_H

/* C++ STL HEADER FILES */
#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

/* EXTERNAL LIBRARY HEADER FILES */
#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>
#include <Eigen/Dense>

/* CPET HEADER FILES */
#include "Option.h"
#include "PointCharge.h"
#include "TopologyRegion.h"
#include "Utilities.h"
#include "Volume.h"
#include "Frame.h"
#include "PathSample.h"

namespace cpet {

class System {
 public:
  System(Frame frame, const Option& options);

  [[nodiscard]] Eigen::Vector3d electricFieldAt(
      const Eigen::Vector3d& position) const;

  [[nodiscard]] std::vector<PathSample> electricFieldTopologyIn(
      int numOfThreads, const Volume& volume, const double stepsize,
      const int numberOfSamples) const;

  inline void transformToUserSpace() {
    translateSystemToCenter_();
    transformToUserBasis_();
  }

  [[nodiscard]] inline Eigen::Vector3d transformToUserSpace(
      const Eigen::Vector3d& vec) const noexcept {
    const Eigen::Matrix3d inverse = basisMatrix_.inverse();
    return inverse * (vec - center_);
  }

  inline void printCenterAndBasis() const noexcept {
    SPDLOG_INFO("[center] ==>> {}", this->center_.transpose());
    SPDLOG_INFO("[User Basis]");
    SPDLOG_INFO(this->basisMatrix_.transpose());
  }

  [[nodiscard]] Eigen::Vector3d center() const noexcept { return center_; }

  [[nodiscard]] Eigen::Matrix3d basisMatrix() const noexcept {
    return basisMatrix_;
  }

  [[nodiscard]] std::vector<Eigen::Vector3d> computeElectricFieldIn(
      const EFieldVolume& volume) const noexcept;

  [[nodiscard]] constexpr const Frame& frame() const noexcept { return frame_; }

 private:
  static inline void constructOrthonormalBasis_(
      std::array<Eigen::Vector3d, 3>& basis) noexcept {
    SPDLOG_DEBUG("Constructing orthonormal basis...");
    basis[2] = basis[0].cross(basis[1]);
    basis[1] = basis[2].cross(basis[0]);

    basis[2] = basis[2] / basis[2].norm();
    basis[1] = basis[1] / basis[1].norm();
  }

  [[nodiscard]] double curvatureAt_(const Eigen::Vector3d& alpha_0,
                                    double stepSize) const noexcept;

  [[nodiscard]] PathSample sampleElectricFieldTopologyIn_(
      const Volume& region, double stepSize) const noexcept;

  inline void forEachPointCharge_(
      const std::function<void(PointCharge&)>& func) {
    std::for_each(pointCharges_.begin(), pointCharges_.end(), func);
    std::for_each(frame_.begin(), frame_.end(), func);
  }

  inline void translateSystemTo_(const Eigen::Vector3d& position) {
    forEachPointCharge_(
        [&position](PointCharge& pc) { pc.coordinate -= position; });
  }

  inline void translateSystemToCenter_() {
    SPDLOG_DEBUG("Translating to the center");
    SPDLOG_DEBUG("[center] ==>> {}", center_.transpose());
    translateSystemTo_(center_);
  }

  [[maybe_unused]] inline void translateSystemToOrigin_() {
    SPDLOG_DEBUG("Translating to the Origin");
    translateSystemTo_(-1 * (center_));
  }

  inline void transformToUserBasis_() {
    SPDLOG_DEBUG("Translating to user basis");

    Eigen::Matrix3d inverse = basisMatrix_.inverse();
    SPDLOG_DEBUG("[User Basis]");
    SPDLOG_DEBUG(inverse);
    forEachPointCharge_([&inverse](PointCharge& pc) {
      pc.coordinate = inverse * pc.coordinate;
    });
  }

  [[maybe_unused]] inline void transformToDefaultBasis_() {
    SPDLOG_DEBUG("Translating to default basis");
    forEachPointCharge_([this](PointCharge& pc) {
      pc.coordinate = basisMatrix_ * pc.coordinate;
    });
  }

  [[nodiscard]] inline Eigen::Vector3d nextPoint_(
      const Eigen::Vector3d& pos, const double stepSize) const noexcept {
    Eigen::Vector3d f = electricFieldAt(pos);
    f /= f.norm();
    return (pos + stepSize * f);
  }

  Frame frame_;
  std::vector<PointCharge> pointCharges_;
  Eigen::Vector3d center_;
  Eigen::Matrix3d basisMatrix_;
};
}  // namespace cpet
#endif  // SYSTEM_H

#ifndef SYSTEM_H
#define SYSTEM_H

/* C++ STL HEADER FILES */
#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

/* EXTERNAL LIBRARY HEADER FILES */
#include "Eigen/Dense"
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"

/* CPET HEADER FILES */
#include "Option.h"
#include "PointCharge.h"
#include "TopologyRegion.h"
#include "Utilities.h"
#include "Volume.h"

// TODO maybe make this an option?
#define STEP_SIZE 0.001

struct PathSample {
  double distance;
  double curvature;

  friend std::ostream& operator<<(std::ostream& os, const PathSample& ps) {
    os << ps.distance << ',' << ps.curvature;
    return os;
  }
};

class System {
 public:
  System(std::vector<PointCharge> pc, const Option& options);

  [[nodiscard]] Eigen::Vector3d electricFieldAt(
      const Eigen::Vector3d& position) const;

  [[nodiscard]] std::vector<PathSample> electricFieldTopologyIn(
      int numOfThreads, const TopologyRegion& topologicalRegion);

  inline void transformToUserSpace() noexcept {
    translateSystemToCenter_();
    transformToUserBasis_();
  }

  [[nodiscard]] Eigen::Vector3d center() const {return center_;}
  [[nodiscard]] Eigen::Matrix3d basisMatrix() const {return basisMatrix_;}

 private:
  static inline void constructOrthonormalBasis_(
      std::array<Eigen::Vector3d, 3>& basis) noexcept {
    SPDLOG_DEBUG("Constructing orthonormal basis...");
    basis[2] = basis[0].cross(basis[1]);
    basis[1] = basis[2].cross(basis[0]);

    basis[2] = basis[2] / basis[2].norm();
    basis[1] = basis[1] / basis[1].norm();
  }

  [[nodiscard]] double curvatureAt_(
      const Eigen::Vector3d& alpha_0) const noexcept;

  PathSample sampleElectricFieldTopologyIn_(
      const Volume& region) const noexcept;

  inline void forEachPointCharge_(
      const std::function<void(PointCharge&)>& func) {
    std::for_each(begin(pointCharges_), end(pointCharges_), func);
  }

  inline void translateSystemTo_(const Eigen::Vector3d& position) noexcept {
    forEachPointCharge_(
        [&position](PointCharge& pc) { pc.coordinate -= position; });
  }

  inline void translateSystemToCenter_() noexcept {
    SPDLOG_DEBUG("Translating to the center");
    SPDLOG_INFO("[center] ==>> {}", center_.transpose());
    translateSystemTo_(center_);
  }

  inline void translateSystemToOrigin_() noexcept {
    SPDLOG_DEBUG("Translating to the Origin");
    translateSystemTo_(-1 * (center_));
  }

  inline void transformToUserBasis_() noexcept {
    SPDLOG_DEBUG("Translating to user basis");

    Eigen::Matrix3d inverse = basisMatrix_.inverse();
    SPDLOG_INFO("[User Basis]");
    SPDLOG_INFO(inverse);
    forEachPointCharge_([&inverse](PointCharge& pc) {
      pc.coordinate = inverse * pc.coordinate;
    });
  }

  inline void transformToDefaultBasis_() noexcept {
    SPDLOG_DEBUG("Translating to default basis");
    forEachPointCharge_([this](PointCharge& pc) {
      pc.coordinate = basisMatrix_ * pc.coordinate;
    });
  }

  [[nodiscard]] inline Eigen::Vector3d nextPoint_(
      const Eigen::Vector3d& pos) const noexcept {
    /* Runge Kutta Order 4 */
    Eigen::Vector3d u1 = STEP_SIZE * electricFieldAt(pos);
    Eigen::Vector3d u2 = STEP_SIZE * electricFieldAt(pos + (0.5 * u1));
    Eigen::Vector3d u3 = STEP_SIZE * electricFieldAt(pos + 2 * u2 - u1);
    return (pos + (1.0 / 6.0) * (u1 + 4 * u2 + u3));
  }

  std::vector<PointCharge> pointCharges_;
  Eigen::Vector3d center_;
  Eigen::Matrix3d basisMatrix_;
};

#endif  // SYSTEM_H

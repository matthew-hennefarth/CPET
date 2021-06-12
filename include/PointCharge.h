// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef POINTCHARGE_H
#define POINTCHARGE_H

#include <utility>
#include <vector>

/* EXTERNAL LIBRARY HEADER FILES */
#include <Eigen/Dense>

/* CPET HEADER FILES */
#include "AtomID.h"
#include "Utilities.h"
namespace cpet {

struct PointCharge {
  Eigen::Vector3d coordinate;

  double charge;

  AtomID id;

  inline PointCharge(Eigen::Vector3d coord, double q, AtomID aid) noexcept
      : coordinate(std::move(coord)), charge(q), id(std::move(aid)) {}

  [[nodiscard]] inline bool operator==(const PointCharge& pc) const {
    return (coordinate == pc.coordinate) && (charge == pc.charge) &&
           (id == pc.id);
  }
};
}  // namespace cpet
#endif  // POINTCHARGE_H

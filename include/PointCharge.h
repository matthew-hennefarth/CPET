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

  PointCharge(PointCharge&&) = default;

  PointCharge(const PointCharge&) = default;

  inline PointCharge& operator=(const PointCharge&) noexcept = default;

  inline PointCharge& operator=(PointCharge&&) noexcept = default;

  [[nodiscard]] inline bool operator==(const PointCharge& pc) const {
    return (coordinate == pc.coordinate) && (charge == pc.charge) &&
           (id == pc.id);
  }

  ~PointCharge() = default;

  [[nodiscard]] static inline auto find(
      const std::vector<PointCharge>& pointCharges, const AtomID& id)
      -> decltype(pointCharges.begin()) {
    return util::find_if_ex(begin(pointCharges), end(pointCharges),
                            [&id](const auto& pc) { return pc.id == id; });
  }
} __attribute__((aligned(128)));
}  // namespace cpet
#endif  // POINTCHARGE_H

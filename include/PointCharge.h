#ifndef POINTCHARGE_H
#define POINTCHARGE_H

/* EXTERNAL LIBRARY HEADER FILES */
#include "Eigen/Dense"

/* CPET HEADER FILES */
#include "AtomID.h"
#include "Utilities.h"

struct PointCharge {
    Eigen::Vector3d coordinate;

    double charge;

    AtomID id;

    inline PointCharge(const Eigen::Vector3d& coord, double q, AtomID  aid) noexcept
        : coordinate(coord), charge(q), id(std::move(aid)) {}

    inline PointCharge(PointCharge&&) = default;

    inline PointCharge(const PointCharge&) = default;

    inline PointCharge& operator=(const PointCharge&) noexcept = default;

    inline PointCharge& operator=(PointCharge&&) noexcept = default;

    [[nodiscard]] static inline auto find(const std::vector<PointCharge>& pointCharges,
                                          const AtomID& id) -> decltype (pointCharges.begin()){
        return find_if_ex(begin(pointCharges), end(pointCharges),
                          [&id](const auto& pc){
                              return pc.id == id;
                          });
    }
};

#endif //POINTCHARGE_H

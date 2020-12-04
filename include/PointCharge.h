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

    PointCharge(const Eigen::Vector3d &coord, double q, AtomID  aid)
        : coordinate(coord), charge(q), id(std::move(aid)) {}

    PointCharge(PointCharge&&) = default;

    PointCharge(const PointCharge&) = default;

    inline PointCharge& operator=(const PointCharge&) = default;

    inline PointCharge& operator=(PointCharge&&) = default;

    [[nodiscard]] static inline auto find(const std::vector<PointCharge>& pointCharges,
                                          const AtomID& id) -> decltype (pointCharges.begin()){
        return find_if_ex(begin(pointCharges), end(pointCharges),
                          [&id](const auto& pc){
                              return pc.id == id;
                          });
    }
};

#endif //POINTCHARGE_H

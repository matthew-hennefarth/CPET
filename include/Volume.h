//
// Created by Matthew Hennefarth on 11/12/20.
//

#ifndef VOLUME_H
#define VOLUME_H

#include "Eigen/Dense"

class Volume {
    public:
        Volume() = default;

        Volume(const Volume &) = default;

        Volume& operator=(const Volume&) = default;

        virtual ~Volume() = default;

        [[nodiscard]] virtual bool isInside(const Eigen::Vector3d &position) const noexcept(true) = 0;

        [[nodiscard]] virtual const double &maxDim() const noexcept = 0;

        [[nodiscard]] virtual Eigen::Vector3d randomPoint() const = 0;

        [[nodiscard]] virtual std::string_view description() const noexcept(true) = 0;

        [[nodiscard]] virtual int randomDistance(double stepSize) const noexcept = 0;

        [[nodiscard]] virtual std::string type() const noexcept = 0;

};

#endif //VOLUME_H

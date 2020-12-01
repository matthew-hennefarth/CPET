//
// Created by Matthew Hennefarth on 11/12/20.
//

#ifndef VOLUME_H
#define VOLUME_H

#include <random>

#include "Eigen/Dense"

class Volume {
    public:
        Volume() : _gen(std::random_device()()) {};

        Volume(const Volume &) = default;

        Volume& operator=(const Volume&) = default;

        virtual ~Volume() = default;

        [[nodiscard]] virtual bool isInside(const Eigen::Vector3d &position) const noexcept(true) = 0;

        [[nodiscard]] virtual const double &maxDim() const noexcept(true) = 0;

        [[nodiscard]] virtual Eigen::Vector3d randomPoint() const = 0;

        [[nodiscard]] inline std::mt19937 &randomNumberGenerator() const { return _gen; }

        [[nodiscard]] virtual std::string_view description() const noexcept(true) = 0;

    protected:
        virtual constexpr void _initializeDistributions() = 0;

        mutable std::array<std::uniform_real_distribution<double>, 3> _distributions;

    private:
        mutable std::mt19937 _gen;
};

#endif //VOLUME_H

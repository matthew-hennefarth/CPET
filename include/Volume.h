//
// Created by Matthew Hennefarth on 11/12/20.
//

#ifndef VOLUME_H
#define VOLUME_H

#include <random>

#include "Eigen/Dense"

class Volume{
    public:
        Volume() : _rd(), _gen(_rd()){};
        virtual ~Volume() = default;

        [[nodiscard]] virtual bool isInside(const Eigen::Vector3d& position) const = 0;
        [[nodiscard]] virtual const double& maxDim() const noexcept(true) = 0;
        [[nodiscard]] virtual Eigen::Vector3d randomPoint() = 0;
        [[nodiscard]] inline std::mt19937& randomNumberGenerator() {
            return _gen;
        }

    protected:
        virtual constexpr void _initializeDistributions() = 0;

        std::array<std::uniform_real_distribution<double>, 3> _distributions;

    private:
        std::random_device _rd;
        std::mt19937 _gen;
};

#endif //VOLUME_H

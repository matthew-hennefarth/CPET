//
// Created by Matthew Hennefarth on 11/12/20.
//

#ifndef BOX_H
#define BOX_H

#include <random>

#include "Eigen/Dense"
#include "Volume.h"

class Box : public Volume{
    public:
        inline Box(const std::array<double, 3>& sides) : _sides(sides){
            for(const auto& val : _sides){
                assert(val >0);
            }

            _initializeDistributions();
        }

        ~Box() override = default;

        [[nodiscard]] constexpr const double& maxDim() const noexcept(true) override {
            return *std::max_element(_sides.begin(), _sides.end());
        }

        [[nodiscard]] inline bool isInside(const Eigen::Vector3d& position) const noexcept(true) override {
            for(size_t i = 0; i < 3; i++){
                if (abs(position[static_cast<long>(i)]) >= _sides[i])
                    return false;
            }
            return true;
        }

        [[nodiscard]] inline Eigen::Vector3d randomPoint() const override{
            Eigen::Vector3d result;

            int i = 0;
            for(auto& dis : _distributions){
                result[i] = dis(randomNumberGenerator());
                i++;
            }
            return result;
        }

        [[nodiscard]] inline std::string_view description() const noexcept(true) override {
            std::string result = "Box:";
            for(const auto& dim : _sides){
                result += (" " + std::to_string(dim));
            }
            return result;
        }

    private:
        constexpr void _initializeDistributions() noexcept(true) override{
            for(std::array<std::uniform_real_distribution<double>, 3>::size_type i = 0; i < _distributions.size(); i++){
                _distributions[i] = std::uniform_real_distribution<double>(-1*_sides[i], _sides[i]);
            }
        }

        std::array<double, 3> _sides;

};

#endif //BOX_H
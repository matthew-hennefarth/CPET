//
// Created by Matthew Hennefarth on 11/12/20.
//

#ifndef BOX_H
#define BOX_H

#include <random>

#include "Eigen/Dense"

#include "Volume.h"
#include "Utilities.h"

class Box : public Volume{
    public:
        inline Box(const std::array<double, 3>& sides) : _sides(sides){
            for(const auto& val : _sides){
                assert(val >0);
            }
        }

        ~Box() override = default;

        [[nodiscard]] constexpr const double& maxDim() const noexcept override {
            return *std::max_element(_sides.begin(), _sides.end());
        }

        [[nodiscard]] inline bool isInside(const Eigen::Vector3d& position) const noexcept override {
            for(size_t i = 0; i < 3; i++){
                if (abs(position[static_cast<long>(i)]) >= _sides[i])
                    return false;
            }
            return true;
        }

        [[nodiscard]] inline Eigen::Vector3d randomPoint() const noexcept override{
            Eigen::Vector3d result;

            std::array<std::uniform_real_distribution<double>, 3> distribution;
            _initializeDistributions(distribution);

            int i = 0;
            for(auto& dis : distribution){
                result[i] = dis(*(randomNumberGenerator()));
                i++;
            }
            return result;
        }

        [[nodiscard]] inline std::string description() const noexcept override {
            std::string result = "Box:";
            for(const auto& dim : _sides){
                result += (" " + std::to_string(dim));
            }
            return result;
        }

        [[nodiscard]] inline int randomDistance(double stepSize) const noexcept override{
            std::uniform_int_distribution<int> distribution(1, static_cast<int>(maxDim() / stepSize));
            return distribution(*randomNumberGenerator());
        }

        [[nodiscard]] inline std::string type() const noexcept override{
            return "box";
        }

    private:
        constexpr void _initializeDistributions(std::array<std::uniform_real_distribution<double>, 3>& distribution) const noexcept{
            for(size_t i = 0; i < distribution.size(); i++){
                distribution[i] = std::uniform_real_distribution<double>(-1*_sides[i], _sides[i]);
            }
        }

        std::array<double, 3> _sides;

};

#endif //BOX_H
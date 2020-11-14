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
        Box(const std::array<double, 3>& sides);
        ~Box() override = default;
        [[nodiscard]] inline bool isInside(const Eigen::Vector3d& position) const override{
            for(size_t i = 0; i < 3; i++){
                if (abs(position[static_cast<long>(i)]) >= _sides[i])
                    return false;
            }
            return true;
        }

        [[nodiscard]] inline Eigen::Vector3d randomPoint() override{
            Eigen::Vector3d result;

            int i = 0;
            for(auto& dis : _distributions){
                result[i] = dis(randomNumberGenerator());
                i++;
            }
            return result;
        }

        inline void _initializeDistributions() override{
            for(std::array<std::uniform_real_distribution<double>, 3>::size_type i = 0; i < _distributions.size(); i++){
                _distributions[i] = std::uniform_real_distribution<double>(-1*_sides[i], _sides[i]);
            }
        }

    private:
        std::array<double, 3> _sides;

};

Box::Box (const std::array<double, 3>& sides) : _sides(sides){
    for(const auto& val : _sides){
        assert(val >0);
    }

    _initializeDistributions();
}



#endif //BOX_H

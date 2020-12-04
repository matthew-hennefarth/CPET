#ifndef BOX_H
#define BOX_H

/* C++ STL HEADER FILES */
#include <random>

/* EXTERNAL LIBRARY HEADER FILES */
#include "Eigen/Dense"

/* CPET HEADER FILES */
#include "Volume.h"
#include "Utilities.h"
#include "Exceptions.h"

class Box : public Volume {
    public:
        explicit inline Box(const std::array<double, 3>& sides) : sides_(sides) {
            for (const auto& val : sides_) {
                if (val < 0){
                    throw cpet::value_error("Invalid value: " + std::to_string(val));
                }
            }
        }

        [[nodiscard]] inline const double& maxDim() const noexcept override {
            return *std::max_element(sides_.begin(), sides_.end());
        }

        [[nodiscard]] inline bool isInside(const Eigen::Vector3d& position) const noexcept override {
            for (size_t i = 0; i < 3; i++){
                if (abs(position[static_cast<long>(i)]) >= sides_[i]) {
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]] inline Eigen::Vector3d randomPoint() const noexcept override {
            Eigen::Vector3d result;

            std::array<std::uniform_real_distribution<double>, 3> distribution;
            initializeDistributions_(distribution);

            int i = 0;
            for(auto& dis : distribution){
                result[i] = dis(*(randomNumberGenerator()));
                i++;
            }
            return result;
        }

        [[nodiscard]] inline std::string description() const noexcept override {
            std::string result = "Box:";
            for(const auto& dim : sides_){
                result += (" " + std::to_string(dim));
            }
            return result;
        }

        [[nodiscard]] inline int randomDistance(double stepSize) const noexcept override {
            std::uniform_int_distribution<int> distribution(1, static_cast<int>(maxDim() / stepSize));
            return distribution(*randomNumberGenerator());
        }

        [[nodiscard]] inline std::string type() const noexcept override {
            return "box";
        }

    private:
        inline void initializeDistributions_(std::array<std::uniform_real_distribution<double>, 3>& distribution) const noexcept {
            for(size_t i = 0; i < distribution.size(); i++){
                distribution[i] = std::uniform_real_distribution<double>(-1*sides_[i], sides_[i]);
            }
        }

        std::array<double, 3> sides_;

};

#endif //BOX_H
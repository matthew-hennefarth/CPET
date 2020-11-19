#ifndef SYSTEM_H
#define SYSTEM_H

/*
 * C++ STL HEADER FILES
 */
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <iostream>
#include <filesystem>
#include <random>

/*
 * EXTERNAL LIBRARY HEADER FILES
 */
#include "spdlog/spdlog.h"
#include "Eigen/Dense"

/*
 * CPET HEADER FILES
 */
#include "Volume.h"
#include "Box.h"
#include "Utilities.h"

#define PERM_SPACE 0.0055263495
#define PI 3.1415926535
#define STEP_SIZE 0.001

struct PointCharge{
    Eigen::Vector3d coordinate;
    double charge;

    PointCharge(const Eigen::Vector3d& coord, const double& q): coordinate(coord), charge(q){}
};

struct PathSample{
    double distance;
    double curvature;
};

class System{
    public:
        System(const std::string_view &proteinFile, const std::string_view &optionsFile);
        ~System() = default;

        [[nodiscard]] Eigen::Vector3d electricField (const Eigen::Vector3d& position) const;
        void calculateTopology(const size_t& procs);

    private:
        void _loadPDB();
        void _loadOptions(const std::string_view& optionsFile);
        // TODO change the name of this
        void _b (std::vector<PathSample>& shared_array, const std::vector<size_t>& values) noexcept(true) {
            for (const auto& i : values){
                _sample(shared_array, i);
            }
        };

        void _sample(std::vector<PathSample>& output, size_t i) noexcept(true);

        inline void _applyToPC(const std::function<void(PointCharge&)>& func) {
            for (auto& pc: _pointCharges){
                func(pc);
            }
        }

        inline void _translate(const Eigen::Vector3d& position){
            _applyToPC([position] (PointCharge& pc){ pc.coordinate -= position; });
        }

        inline void _translateToCenter() {
            SPDLOG_INFO("Translating to the center");
            _translate(_center);
        }

        inline void _translateToOrigin(){
            SPDLOG_INFO("Translating to the Origin");
            _translate(-1*(_center));
        }

        inline void  _toUserBasis(){
            SPDLOG_INFO("Translating to user basis");
            Eigen::Matrix3d inverse = _basisMatrix.inverse();
            _applyToPC([inverse] (PointCharge& pc){ pc.coordinate = inverse*pc.coordinate; });
        }

        inline void _toDefaultBasis() {
            SPDLOG_INFO("Translating to default basis");
            _applyToPC([this] (PointCharge& pc){ pc.coordinate = _basisMatrix*pc.coordinate; });
        }

        [[nodiscard]] inline Eigen::Vector3d _next(const Eigen::Vector3d& pos) const {
            // Runge Kutta Order 4
            Eigen::Vector3d u1 = STEP_SIZE*electricField(pos);
            Eigen::Vector3d u2 = STEP_SIZE*electricField(pos+(0.5*u1));
            Eigen::Vector3d u3 = STEP_SIZE*electricField(pos+2*u2-u1);
            //return (pos + 0.5*(u1+u2));
            return (pos + (1.0/6.0)*(u1+4*u2+u3));
        }

        [[nodiscard]] inline double _curvature(const Eigen::Vector3d& alpha_0) const {
            Eigen::Vector3d alpha_prime = electricField(alpha_0);
            Eigen::Vector3d alpha_1 = _next(alpha_0);

            // Measures how much "time" we spent going forward
            // delta alpha/delta t = E, in the limit of delta t -> 0
            // then we have delta alpha/E = delta t
            double delta_t = (alpha_1 - alpha_0).norm() / alpha_prime.norm();

            // Simple directional derivative of the electric field in that direction
            Eigen::Vector3d alpha_prime_prime = (electricField(alpha_1) - alpha_prime)/delta_t;

            double alpha_prime_norm = alpha_prime.norm();

            return (alpha_prime.cross(alpha_prime_prime)).norm() / (alpha_prime_norm*alpha_prime_norm*alpha_prime_norm);

        }

        std::vector<PointCharge> _pointCharges;
        Eigen::Vector3d _center;
        Eigen::Matrix3d _basisMatrix;
        std::shared_ptr<Volume> _region;
        std::string _name;
        std::mt19937 _gen;
        size_t _numberOfSamples{};
        std::uniform_int_distribution<int> _distribution;
        std::mutex _mutex;

};
#endif //SYSTEM_H

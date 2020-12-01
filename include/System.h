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
#include <algorithm>

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

// TODO maybe make this an option?
#define STEP_SIZE 0.001

struct PointCharge {
    Eigen::Vector3d coordinate;
    double charge;

    PointCharge(const Eigen::Vector3d &coord, const double &q) : coordinate(coord), charge(q) {}
};

struct PathSample {
    double distance;
    double curvature;
};

class System {
    public:
        System(const std::string_view &proteinFile, const std::string_view &optionsFile);

        ~System() = default;

        [[nodiscard]] Eigen::Vector3d electricField(const Eigen::Vector3d &position) const noexcept(true);

        std::vector<PathSample> calculateTopology(const size_t &procs);

    private:
        void _loadPDB(const std::string_view&);

        void _loadOptions(const std::string_view&);

        [[nodiscard]] double _curvature(const Eigen::Vector3d& alpha_0) const noexcept(true);

        PathSample _sample() noexcept(true);

        inline void _forEachPC(const std::function<void(PointCharge &)> &func) {
            std::for_each(begin(_pointCharges), end(_pointCharges), func);
        }

        inline void _translate(const Eigen::Vector3d &position) noexcept(true) {
            _forEachPC([&position](PointCharge &pc) { pc.coordinate -= position; });
        }

        inline void _translateToCenter() noexcept(true) {
            SPDLOG_DEBUG("Translating to the center");
            _translate(_center);
        }

        inline void _translateToOrigin() noexcept(true) {
            SPDLOG_DEBUG("Translating to the Origin");
            _translate(-1 * (_center));
        }

        inline void _toUserBasis() {
            SPDLOG_INFO("Translating to user basis");
            Eigen::Matrix3d inverse = _basisMatrix.inverse();
            //TODO Check if the inverse works..ie, isInvertible
            _forEachPC([&inverse](PointCharge &pc) { pc.coordinate = inverse * pc.coordinate; });
        }

        inline void _toDefaultBasis() noexcept(true) {
            SPDLOG_INFO("Translating to default basis");
            _forEachPC([this](PointCharge &pc) { pc.coordinate = _basisMatrix * pc.coordinate; });
        }

        [[nodiscard]] inline Eigen::Vector3d _next(const Eigen::Vector3d &pos) const noexcept(true) {
            // Runge Kutta Order 4
            Eigen::Vector3d u1 = STEP_SIZE * electricField(pos);
            Eigen::Vector3d u2 = STEP_SIZE * electricField(pos + (0.5 * u1));
            Eigen::Vector3d u3 = STEP_SIZE * electricField(pos + 2 * u2 - u1);
            //return (pos + 0.5*(u1+u2));
            return (pos + (1.0 / 6.0) * (u1 + 4 * u2 + u3));
        }

        [[nodiscard]] inline int _randomDistance(){
            static thread_local std::unique_ptr<std::mt19937> generator = nullptr;
            if (generator == nullptr){
                generator = std::make_unique<std::mt19937>(std::random_device()());
            }
            std::uniform_int_distribution<int> distribution(1, static_cast<int>(_region->maxDim() / STEP_SIZE));
            return distribution(*generator);
        }

        std::vector<PointCharge> _pointCharges;
        Eigen::Vector3d _center;
        Eigen::Matrix3d _basisMatrix;
        std::unique_ptr<Volume> _region;
        std::mutex _mutex_volume;
        size_t _numberOfSamples{};
};

#endif //SYSTEM_H

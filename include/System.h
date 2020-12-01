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

    friend std::ostream& operator<<(std::ostream& os, const PathSample& ps)
    {
        os << ps.distance << ',' << ps.curvature;
        return os;
    }
};

class System {
    public:
        System(const std::string_view &proteinFile, const std::string_view &optionsFile);

        ~System() = default;

        [[nodiscard]] Eigen::Vector3d electricField(const Eigen::Vector3d &position) const noexcept;

        std::vector<PathSample> calculateTopology(const size_t &procs);

    private:
        void _loadPDB(const std::string_view&);

        void _loadOptions(const std::string_view&);

        [[nodiscard]] double _curvature(const Eigen::Vector3d& alpha_0) const noexcept;

        PathSample _sample() const noexcept;

        inline void _forEachPC(const std::function<void(PointCharge &)> &func) {
            std::for_each(begin(_pointCharges), end(_pointCharges), func);
        }

        inline void _translate(const Eigen::Vector3d &position) noexcept {
            _forEachPC([&position](PointCharge &pc) { pc.coordinate -= position; });
        }

        inline void _translateToCenter() noexcept {
            SPDLOG_DEBUG("Translating to the center");
            _translate(_center);
        }

        inline void _translateToOrigin() noexcept {
            SPDLOG_DEBUG("Translating to the Origin");
            _translate(-1 * (_center));
        }

        inline void _toUserBasis() {
            SPDLOG_INFO("Translating to user basis");
            Eigen::Matrix3d inverse = _basisMatrix.inverse();
            //TODO Check if the inverse works..ie, isInvertible
            _forEachPC([&inverse](PointCharge &pc) { pc.coordinate = inverse * pc.coordinate; });
        }

        inline void _toDefaultBasis() noexcept {
            SPDLOG_INFO("Translating to default basis");
            _forEachPC([this](PointCharge &pc) { pc.coordinate = _basisMatrix * pc.coordinate; });
        }

        [[nodiscard]] inline Eigen::Vector3d _next(const Eigen::Vector3d &pos) const noexcept {
            /* Runge Kutta Order 4 */
            Eigen::Vector3d u1 = STEP_SIZE * electricField(pos);
            Eigen::Vector3d u2 = STEP_SIZE * electricField(pos + (0.5 * u1));
            Eigen::Vector3d u3 = STEP_SIZE * electricField(pos + 2 * u2 - u1);
            return (pos + (1.0 / 6.0) * (u1 + 4 * u2 + u3));
        }

        [[nodiscard]] inline int _randomDistance() const noexcept {
            std::uniform_int_distribution<int> distribution(1, static_cast<int>(_region->maxDim() / STEP_SIZE));
            return distribution(*randomNumberGenerator());
        }

        std::vector<PointCharge> _pointCharges;
        Eigen::Vector3d _center;
        Eigen::Matrix3d _basisMatrix;
        std::unique_ptr<Volume> _region;
        size_t _numberOfSamples{};
};

#endif //SYSTEM_H

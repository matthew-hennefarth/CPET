//
// Created by Matthew Hennefarth on 11/12/20.
//

#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <filesystem>

#include "spdlog/spdlog.h"
#include "Eigen/Dense"

#define PERM_SPACE 0.0055263495
#define PI 3.1415926535
#define STEP_SIZE 0.01

#include "Volume.h"
#include "Box.h"
#include "Utilities.h"

struct PointCharge{
    Eigen::Vector3d coordinate;
    double charge;
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
        void calculateTopology(const int& nIter);

    private:
        void _loadPDB(const std::string_view& proteinFile);
        void _loadOptions(const std::string_view& optionsFile);

        [[nodiscard]] inline PathSample _sample(){
            Eigen::Vector3d initialPosition = _region->randomPoint();
            Eigen::Vector3d finalPosition = initialPosition;
            int steps = 0, maxSteps = 10;
            while(_region->isInside(finalPosition) && steps < maxSteps){
                finalPosition = _next(finalPosition);
                steps++;
            }

            return {(finalPosition - initialPosition).norm(),
                    (_curvature(finalPosition) + _curvature(initialPosition))/2.0};

        }

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

        [[nodiscard]] Eigen::Vector3d _next(const Eigen::Vector3d& pos) const {
            // Runge Kutta Order 4
            Eigen::Vector3d u1 = STEP_SIZE*electricField(pos);
            Eigen::Vector3d u2 = STEP_SIZE*electricField(pos+(0.5*u1));
            Eigen::Vector3d u3 = STEP_SIZE*electricField(pos+2*u2-u1);
            return (pos + (1.0/6.0)*(u1+4*u2+u3));
        }

        [[nodiscard]] double _curvature(const Eigen::Vector3d& alpha_0) const {
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
        std::unique_ptr<Volume> _region;
};

System::System(const std::string_view &proteinFile, const std::string_view &optionsFile)
    : _pointCharges(), _center(), _basisMatrix(Eigen::Matrix3d::Identity()), _region(nullptr){
    _loadPDB(proteinFile);
    _loadOptions(optionsFile);

    if(_region == nullptr){
        throw std::exception();
    }

    _translateToCenter();
    _toUserBasis();
    calculateTopology(10);
}

Eigen::Vector3d System::electricField (const Eigen::Vector3d &position) const {
    Eigen::Vector3d result(0,0,0);

    for (const auto &pc : _pointCharges) {
        Eigen::Vector3d d = (position - pc.coordinate);
        double dNorm = d.norm();
        result += ((pc.charge * d) / (dNorm * dNorm * dNorm));
    }
    result /= (1.0 / (4.0 * PI * PERM_SPACE));
    return result;
}

void System::calculateTopology (const int &nIter) {

    SPDLOG_INFO("Calculating Topology");
    for(int i = 0; i < nIter; i++){
        auto sampleData = _sample();
        SPDLOG_INFO("Distance {}, Curvature {}", sampleData.distance, sampleData.curvature);
    }
}

void System::_loadPDB(const std::string_view& proteinFile){
    SPDLOG_INFO("Loading in the PDB file");

    std::uintmax_t fileSize = std::filesystem::file_size(proteinFile);
    _pointCharges.reserve(fileSize/69);

    extractFromFile(proteinFile, [this] (const std::string &line) {
        if(line.substr(0, 4) == "ATOM" || line.substr(0, 6) == "HETATM"){
            PointCharge pc = {{std::stod(line.substr(31, 8)),
                               std::stod(line.substr(39,8)),
                               std::stod(line.substr(47, 8))},
                              std::stod(line.substr(55,8))};
            this->_pointCharges.emplace_back(pc);
        }
    });
}

void System::_loadOptions(const std::string_view& optionsFile){
    SPDLOG_INFO("Loading in the options file");

    extractFromFile(optionsFile, [this] (const std::string &line){
        if (line.substr(0,6) == "center"){
            std::vector<std::string> info = split(line.substr(6), ' ');
            this->_center = {stod(info[0]), stod(info[1]), stod(info[2])};
        }
        else if (line.substr(0,2) == "v1"){
            std::vector<std::string> info = split(line.substr(2), ' ');
            Eigen::Vector3d v1(stod(info[0]), stod(info[1]), stod(info[2]));
            this->_basisMatrix.block(0,0,3,1) = v1;
        }
        else if (line.substr(0,2) == "v2"){
            std::vector<std::string> info = split(line.substr(2), ' ');
            Eigen::Vector3d v2(stod(info[0]), stod(info[1]), stod(info[2]));
            this->_basisMatrix.block(0,1,3,1) = v2;
        }
        else if(line.substr(0,6) == "volume"){
            std::vector<std::string> info = split(line.substr(6), ' ');
            if (info[0] == "box"){
                std::array<double, 3> dims = {std::stod(info[1]),std::stod(info[2]),std::stod(info[3])};
                this->_region = std::make_unique<Box>(dims);
            }
        }
    });

    _basisMatrix.block(0,2,3,1) = _basisMatrix.col(0).cross(_basisMatrix.col(1));
}

#endif //SYSTEM_H

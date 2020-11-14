//
// Created by Matthew Hennefarth on 11/12/20.
//

#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

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

class System{
    public:
        System(const std::string_view &proteinFile, const std::string_view &optionsFile);
        ~System() = default;

        [[nodiscard]] Eigen::Vector3d electricField (const Eigen::Vector3d& position) const;
        void calculateTopology(const int& nIter);

    private:
        void _loadPDB(const std::string_view& proteinFile);
        void _loadOptions(const std::string_view& optionsFile);

        inline void _applyToPC(const std::function<void(PointCharge&)>& func) {
            for (auto& pc: *(_pointCharges)){
                func(pc);
            }
        }

        inline void _translate(const Eigen::Vector3d& position){

            static const Eigen::Vector3d* staticCastPtr = &position;
            _applyToPC([] (PointCharge& pc){ pc.coordinate -= *(staticCastPtr); });
        }

        inline void _translateToCenter() {
            SPDLOG_INFO("Translating to the center");
            _translate(*(_center));
        }

        inline void _translateToOrigin(){
            SPDLOG_INFO("Translating to the Origin");
            _translate(-1*(*(_center)));
        }

        inline void  _toUserBasis(){
            SPDLOG_INFO("Translating to user basis");
            static Eigen::Matrix3d inverse = _basisMatrix->inverse();
            _applyToPC([] (PointCharge& pc){ pc.coordinate = inverse*pc.coordinate; });
        }

        inline void _toDefaultBasis() {
            SPDLOG_INFO("Translating to default basis");
            _applyToPC([this] (PointCharge& pc){ pc.coordinate = *(_basisMatrix)*pc.coordinate; });
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

        std::unique_ptr<std::vector<PointCharge>> _pointCharges;
        std::unique_ptr<Eigen::Vector3d> _center;
        std::unique_ptr<Eigen::Matrix3d> _basisMatrix;
        std::unique_ptr<Volume> _region;

};

System::System(const std::string_view &proteinFile, const std::string_view &optionsFile)
    : _pointCharges(nullptr), _center(nullptr), _basisMatrix(nullptr), _region(nullptr){
    _loadPDB(proteinFile);
    _loadOptions(optionsFile);
    _translateToCenter();
    _toUserBasis();
    calculateTopology(10);
}

Eigen::Vector3d System::electricField (const Eigen::Vector3d &position) const {
    Eigen::Vector3d result(0,0,0);

    for (const auto &pc : *(_pointCharges)) {
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
        auto initialPosition = _region->randomPoint();
        Eigen::Vector3d finalPosition = initialPosition;
        int steps = 0;
        while(_region->isInside(finalPosition) && steps < 10){
            finalPosition = _next(finalPosition);
            steps++;
        }

        double distance = (finalPosition - initialPosition).norm();
        double meanCurvature = (_curvature(finalPosition) + _curvature(initialPosition))/2.0;

        SPDLOG_INFO("Distance {}, Curvature {}", distance, meanCurvature);
    }
}

void System::_loadPDB(const std::string_view& proteinFile){
    SPDLOG_INFO("Loading in the PDB file");

    static std::unique_ptr<std::vector<PointCharge>> temp = std::make_unique<std::vector<PointCharge>>();
    extractFromFile(proteinFile, [] (const std::string &line) {
        if(line.substr(0, 4) == "ATOM" || line.substr(0, 6) == "HETATM"){
            PointCharge pc = {{std::stod(line.substr(31, 8)),
                               std::stod(line.substr(39,8)),
                               std::stod(line.substr(47, 8))},
                              std::stod(line.substr(55,8))};
            temp->emplace_back(pc);
        }
    });
    _pointCharges = std::move(temp);
}

void System::_loadOptions(const std::string_view& optionsFile){
    SPDLOG_INFO("Loading in the options file");

    static std::unique_ptr<Eigen::Vector3d> center = std::make_unique<Eigen::Vector3d>(0,0,0);
    static std::unique_ptr<Eigen::Matrix3d> basisMatrix = std::make_unique<Eigen::Matrix3d>();
    *(basisMatrix) = basisMatrix->Identity();

    static std::unique_ptr<Volume> region = nullptr;

    extractFromFile(optionsFile, [] (const std::string &line){
        if (line.substr(0,6) == "center"){
            auto info = split(line.substr(6), ' ');
            *(center) = {stod(info[0]), stod(info[1]), stod(info[2])};
        }
        else if (line.substr(0,2) == "v1"){
            auto info = split(line.substr(2), ' ');
            Eigen::Vector3d v1(stod(info[0]), stod(info[1]), stod(info[2]));
            basisMatrix->block(0,0,3,1) = v1;
        }
        else if (line.substr(0,2) == "v2"){
            auto info = split(line.substr(2), ' ');
            Eigen::Vector3d v2(stod(info[0]), stod(info[1]), stod(info[2]));
            basisMatrix->block(0,1,3,1) = v2;
        }
        else if(line.substr(0,6) == "volume"){
            auto info = split(line.substr(6), ' ');
            if (info[0] == "box"){
                std::array<double, 3> dims = {std::stod(info[1]),std::stod(info[2]),std::stod(info[3])};
                region = std::make_unique<Box>(dims);
            }
        }
    });

    _center = std::move(center);
    _basisMatrix = std::move(basisMatrix);
    _basisMatrix->block(0,2,3,1) = _basisMatrix->col(0).cross(_basisMatrix->col(1));
    _region = std::move(region);
}

#endif //SYSTEM_H

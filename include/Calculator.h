//
// Created by Matthew Hennefarth on 12/1/20.
//

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <string>
#include <vector>

#include "Eigen/Dense"

#include "Option.h"
#include "System.h"
#include "TopologyRegion.h"

class Calculator{
    public:
        Calculator(std::string proteinFile, const std::string& optionFile, std::string chargesFile="", size_t procs=1);

        void compute();

    private:
        std::string _proteinFile;
        Option _option;
        std::string _chargeFile;
        size_t _procs;

        void _fixCharges(std::vector<std::vector<PointCharge>>& trajectory) const;

        void _computeTopology(const std::vector<std::vector<PointCharge>>& pointChargeTrajectory) const;
        void _computeEField(const std::vector<std::vector<PointCharge>>& pointChargeTrajectory) const;

        std::vector<std::vector<PointCharge>> _loadPDB() const;

        std::vector<double> _loadCharges() const;

        void _writeTopology(const std::vector<PathSample>& data, const TopologyRegion& region, size_t i) const;

        void _writeEField(const std::vector<std::vector<Eigen::Vector3d>>& results) const;

};

#endif //CALCULATOR_H

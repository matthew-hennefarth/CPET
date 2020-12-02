//
// Created by Matthew Hennefarth on 12/1/20.
//

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <string>
#include <vector>

#include "Option.h"
#include "System.h"
#include "TopologyRegion.h"

class Calculator{
    public:
        Calculator(std::string proteinFile, std::string optionFile, std::string chargesFile="", size_t procs=1);

        void compute();

    private:
        std::string _proteinFile;
        Option _option;
        std::string _chargeFile;
        size_t _procs;

        void _fixCharges(std::vector<std::vector<PointCharge>>& trajectory) const;

        std::vector<std::vector<PointCharge>> _loadPDB() const;

        std::vector<double> _loadCharges() const;

        void writeTopology(const std::vector<PathSample>& data, const TopologyRegion& region) const;

};

#endif //CALCULATOR_H

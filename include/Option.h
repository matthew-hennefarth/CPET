#ifndef OPTION_H
#define OPTION_H

#include <vector>
#include <string>

#include "Eigen/Dense"

#include "TopologyRegion.h"
#include "AtomID.h"

class Option{
    public:
        Option(const std::string& optionFile);

        AtomID centerID{AtomID::origin};
        AtomID direction1ID{AtomID::e1};
        AtomID direction2ID{AtomID::e2};
        std::vector<std::string> calculateEFieldPoints;
        std::vector<TopologyRegion> calculateEFieldTopology;

    private:
        void _loadOptions(const std::string& optionFile);

};

#endif //OPTION_H

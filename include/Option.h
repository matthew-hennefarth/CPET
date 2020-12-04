#ifndef OPTION_H
#define OPTION_H

/* C++ STL HEADER FILES */
#include <vector>
#include <string>

/* EXTERNAL LIBRARY HEADER FILES */
#include "Eigen/Dense"

/* CPET HEADER FILES */
#include "TopologyRegion.h"
#include "AtomID.h"

class Option{
    public:
        explicit Option(const std::string& optionFile);

        AtomID centerID{AtomID::Constants::origin};

        AtomID direction1ID{AtomID::Constants::e1};

        AtomID direction2ID{AtomID::Constants::e2};

        std::vector<std::string> calculateEFieldPoints;

        std::vector<TopologyRegion> calculateEFieldTopology;

    private:
        void loadOptions_(const std::string& optionFile);

};

#endif //OPTION_H

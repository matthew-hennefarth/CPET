#include "Option.h"
#include "Utilities.h"
#include "Box.h"

Option::Option(const std::string& optionFile){
    _loadOptions(optionFile);
}

void Option::_loadOptions(const std::string& optionFile){
    forEachLineIn(optionFile, [this](const std::string& line){
        if (line.substr(0, 5) == "align") {
            std::vector<std::string> info = split(line.substr(5), ' ');
            centerID = info[0];
            if (info.size() > 1){
                direction1ID = info[1];
                direction2ID = info[2];
            }

        }

        else if (line.substr(0, 8) == "topology") {
            std::vector<std::string> info = split(line.substr(8), ' ');
            if (info[0] == "box") {
                std::array<double, 3> dims = {std::stod(info[1]), std::stod(info[2]), std::stod(info[3])};

                calculateEFieldTopology.emplace_back(std::make_unique<Box>(dims), std::stoi(info[4]));
            }
        }

        else if (line.substr(0, 5) == "field") {
            std::vector<std::string> info = split(line.substr(5), ' ');
            calculateEFieldPoints.emplace_back(info[0]);
        }
    });
}
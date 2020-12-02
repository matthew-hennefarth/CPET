//
// Created by Matthew Hennefarth on 12/1/20.
//

#include <algorithm>

#include "spdlog/spdlog.h"

#include "AtomID.h"

const std::string AtomID::origin = "-1:-1:-1";

const std::string AtomID::e1 = "-1:-1:-2";

const std::string AtomID::e2 = "-1:-1:-2";

AtomID AtomID::generateID(const std::string& pdbLine){
    std::string result =  pdbLine.substr(21,2) + ":"
                        + pdbLine.substr(22,4) + ":"
                        + pdbLine.substr(12,4);

    result.erase(remove(begin(result), end(result), ' '), end(result));
    return AtomID(result);
}

#include "spdlog/spdlog.h"

#include "Calculator.h"
#include "Utilities.h"
#include "System.h"
#include "Instrumentation.h"

Calculator::Calculator(std::string proteinFile, std::string optionFile, std::string chargesFile, size_t procs)
 : _proteinFile(std::move(proteinFile)), _option(std::move(optionFile)), _chargeFile(std::move(chargesFile)), _procs(procs){}

 void Calculator::compute(){
     std::vector<std::vector<PointCharge>> pointChargeTrajectory = _loadPDB();

     if(!_chargeFile.empty()){
         _fixCharges(pointChargeTrajectory);
     }

     for(const auto& structure: pointChargeTrajectory){
         System s(structure, _option);
         s.transformToUserSpace();

         for(const auto& region : _option.calculateEFieldTopology){
             Timer t;
             auto results = s.calculateTopology(_procs, region);
             writeTopology(results, region);
         }
     }

}

std::vector<std::vector<PointCharge>> Calculator::_loadPDB() const{

    std::vector<std::vector<PointCharge>> pointChargeTrajectory;
    std::vector<PointCharge> tmpHolder;
    forEachLineIn(_proteinFile, [&pointChargeTrajectory, &tmpHolder](const std::string& line){
        if (line.rfind("ENDMDL", 0) == 0){
            pointChargeTrajectory.push_back(tmpHolder);
            tmpHolder.clear();
        }
        else if(line.rfind("ATOM", 0) == 0 || line.rfind("HETATM", 0) == 0){
            tmpHolder.emplace_back(Eigen::Vector3d({std::stod(line.substr(31, 8)),
                                                                  std::stod(line.substr(39, 8)),
                                                                  std::stod(line.substr(47, 8))}),
                                   std::stod(line.substr(55, 8)),
                                   AtomID::generateID(line));
        }
    });
    if (!tmpHolder.empty()){
        pointChargeTrajectory.push_back(tmpHolder);
    }

    return pointChargeTrajectory;
}

std::vector<double> Calculator::_loadCharges() const{
    std::vector<double> realCharges;
    forEachLineIn(_chargeFile, [&realCharges](const std::string& line){
       auto charges = split(line, ' ');
       for(const auto& charge : charges){
           realCharges.emplace_back(std::stod(charge));
       }
    });
    return realCharges;
}

void Calculator::_fixCharges(std::vector<std::vector<PointCharge>>& trajectory) const {
    auto realCharges = _loadCharges();

    for(auto& structure : trajectory){
        if(structure.size() != realCharges.size()){
            throw std::exception();
        }

        for(size_t i = 0; i < structure.size(); i++){
            structure[i].charge = realCharges[i];
        }
    }
}

void Calculator::writeTopology(const std::vector<PathSample>& data, const TopologyRegion& region) const {
    std::string file = _proteinFile + "." + region.volume->type() + ".top";
    std::ofstream outFile(file, std::ios::out);
    if(outFile.is_open()){
        for(const auto& line : data){
            outFile << line << '\n';
        }
        outFile << std::flush;
    }
    else{
        SPDLOG_ERROR("Could not open file {}", file);
    }
}
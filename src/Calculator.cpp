#include <fstream>

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

#include "Calculator.h"
#include "Utilities.h"
#include "System.h"
#include "Instrumentation.h"

Calculator::Calculator(std::string proteinFile, const std::string& optionFile, std::string chargesFile, size_t procs)
 : _proteinFile(std::move(proteinFile)), _option(optionFile), _chargeFile(std::move(chargesFile)), _procs(procs){}

 void Calculator::compute(){
     std::vector<std::vector<PointCharge>> pointChargeTrajectory = _loadPDB();

     if(!_chargeFile.empty()){
         _fixCharges(pointChargeTrajectory);
     }

     _computeTopology(pointChargeTrajectory);
     _computeEField(pointChargeTrajectory);
}

void Calculator::_computeTopology(const std::vector<std::vector<PointCharge>>& pointChargeTrajectory) const{
    for(size_t i = 0; i < pointChargeTrajectory.size(); i++){
        SPDLOG_INFO("=~=~=~=~[Trajectory {}]=~=~=~=~", i);
        System sys(pointChargeTrajectory[i], _option);
        sys.transformToUserSpace();

        for(const auto& region : _option.calculateEFieldTopology){
            SPDLOG_INFO("======[Sampling topology]======");
            SPDLOG_INFO("[Volume ] ==>> {}", region.volume->description());
            SPDLOG_INFO("[Npoints] ==>> {}", region.numberOfSamples);
            SPDLOG_INFO("[Threads] ==>> {}", _procs);
            std::vector<PathSample> results;
            {
                Timer t;
                results = sys.calculateTopology(_procs, region);
            }
            _writeTopology(results, region, i);
        }
    }
}

void Calculator::_computeEField(const std::vector<std::vector<PointCharge>>& pointChargeTrajectory) const{
    std::vector<std::vector<Eigen::Vector3d>> results;
    for(const auto& point: _option.calculateEFieldPoints){
        SPDLOG_INFO("=~=~=~=~[Field at {}]=~=~=~=~", point);
        std::vector<Eigen::Vector3d> fieldAtPoint;
        for(const auto& trajectory : pointChargeTrajectory){
            System sys(trajectory, _option);
            sys.transformToUserSpace();

            Eigen::Vector3d location;
            if(point.find(':') != std::string::npos){
                /* TODO Can reuse this...seems to be a function!!!!*/
                AtomID pcID;
                pcID.setID(point);
                auto pointPC = find_if(begin(trajectory), end(trajectory),
                                        [&pcID](const auto& pc){ return pc.id == pcID; });

                if (pointPC != end(trajectory)){
                    location = pointPC->coordinate;
                }
                else{
                    throw std::exception();
                }
            }
            else{
                auto point_split = split(point, ',');
                location = {std::stod(point_split[0]), std::stod(point_split[1]), std::stod(point_split[2])};
            }
            Eigen::Vector3d field = sys.electricField(location);
            SPDLOG_INFO("Field: {}", field.transpose());
            fieldAtPoint.emplace_back(sys.electricField(field));
        }
        results.push_back(fieldAtPoint);

    }

    _writeEField(results);

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
           try{
               realCharges.emplace_back(std::stod(charge));
           }
           catch(const std::invalid_argument&){
               continue;
           }
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

void Calculator::_writeTopology(const std::vector<PathSample>& data, const TopologyRegion& region, size_t i) const {
    std::string file = _proteinFile + '_' + std::to_string(i) + '_' + region.volume->type() + ".top";
    std::ofstream outFile(file, std::ios::out);
    if(outFile.is_open()){
        outFile << '#' << _proteinFile << ' ' << i << '\n';
        outFile << '#' << region.details() << '\n';
        /* TODO add options writing to this file...*/
        for(const auto& line : data){
            outFile << line << '\n';
        }
        outFile << std::flush;
    }
    else{
        SPDLOG_ERROR("Could not open file {}", file);
    }
}

void Calculator::_writeEField(const std::vector<std::vector<Eigen::Vector3d>> &results) const {
    std::string file = _proteinFile + ".field";
    std::ofstream outFile(file, std::ios::out);
    if(outFile.is_open()){
        outFile << '#' << _proteinFile << '\n';
        for(size_t i = 0; i < results.size(); i++){
            outFile << '#' << _option.calculateEFieldPoints[i] << '\n';
            for(const Eigen::Vector3d& field : results[i]){
                outFile << field.transpose() << '\n';
            }
        }
        outFile << std::flush;
    }
    else{
        SPDLOG_ERROR("Could not open file {}", file);
    }
}
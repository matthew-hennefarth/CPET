//
// Created by Matthew Hennefarth on 11/18/20.
//

#include <thread>

#include "System.h"

System::System(const std::string_view &proteinFile, const std::string_view &optionsFile)
    :  _pointCharges(), _center(), _basisMatrix(Eigen::Matrix3d::Identity()), _region(nullptr),
    _name(proteinFile), _gen(std::random_device()()), _numberOfSamples(0){

    _loadPDB();

    if(_pointCharges.empty()){
        throw std::exception();
    }

    _loadOptions(optionsFile);

    if(_region == nullptr){
        throw std::exception();
    }

    _distribution = std::uniform_int_distribution<int>(1, static_cast<int>(_region->maxDim() / STEP_SIZE));

    _translateToCenter();
    _toUserBasis();
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

void System::calculateTopology (const size_t& procs) {

    SPDLOG_INFO("Calculating Topology");
    //auto* const shared_array = new PathSample[_numberOfSamples];
    std::vector<PathSample> shared_array(_numberOfSamples);

    if (procs == 1){
        std::vector<size_t> values(_numberOfSamples);

        for(size_t i = 0; i < _numberOfSamples; i++){
            values[i] = i;
        }

        _b(shared_array, values);

    }
    else{
        // TODO Make this a function in Utilities
        std::vector<size_t> elementsToCompute(procs,
                                              _numberOfSamples / procs);
        for(size_t i = 0; i < _numberOfSamples%procs; i++){
            elementsToCompute[i]++;
        }

        std::vector<std::vector<size_t>> chunks(procs);
        size_t index = 0;
        for(size_t i = 0 ; i < procs; i++){
            for(size_t j = 0; j < elementsToCompute[i]; j++, index++){
                chunks[i].push_back(index);
            }
        }
        //Now chunks contains the indexes for each proc to use

        SPDLOG_INFO("Initializing threads");
        std::vector<std::thread> threads;
        threads.reserve(procs);

        for(size_t i = 0; i < procs; i++){
            threads.emplace_back(std::thread(&System::_b, this, std::ref(shared_array), std::ref(chunks[i])));
        }
        for(auto& t : threads){
            t.join();
        }

    }

    for(size_t i = 0; i < _numberOfSamples; i++){
        SPDLOG_INFO("{}, {}", shared_array[i].distance, shared_array[i].curvature);
    }

}

void System::_loadPDB(){
    SPDLOG_INFO("Loading in the PDB file");

    std::uintmax_t fileSize = std::filesystem::file_size(_name);
    _pointCharges.reserve(fileSize/69);

    extractFromFile(_name, [this] (const std::string &line) {
        if(line.substr(0, 4) == "ATOM" || line.substr(0, 6) == "HETATM"){
            this->_pointCharges.emplace_back(Eigen::Vector3d({std::stod(line.substr(31, 8)),
                                                              std::stod(line.substr(39,8)),
                                                              std::stod(line.substr(47, 8))}),
                                             std::stod(line.substr(55,8))
            );
        }
    });
}

void System::_loadOptions(const std::string_view& optionsFile){
    SPDLOG_INFO("Loading in the options file");

    // TODO somehow place some checks here...
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
        else if(line.substr(0,6) == "sample"){
            std::vector<std::string> info = split(line.substr(6), ' ');
            this->_numberOfSamples = static_cast<size_t>(std::stoi(info[0]));
        }
    });

    _basisMatrix.block(0,2,3,1) = _basisMatrix.col(0).cross(_basisMatrix.col(1));
}

void System::_sample(std::vector<PathSample>& output, size_t i) noexcept(true) {
    _mutex.lock();
    Eigen::Vector3d initialPosition = _region->randomPoint();
    const int maxSteps = _distribution(_gen);
    _mutex.unlock();

    Eigen::Vector3d finalPosition = initialPosition;

    int steps = 0;

    while(_region->isInside(finalPosition) && steps < maxSteps){
        finalPosition = _next(finalPosition);
        steps++;
    }

    output[i] = {(finalPosition - initialPosition).norm(),
    (_curvature(finalPosition) + _curvature(initialPosition))/2.0};

}


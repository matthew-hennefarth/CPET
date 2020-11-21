//
// Created by Matthew Hennefarth on 11/18/20.
//

#include <thread>

#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/fmt/ostr.h"

#include "System.h"

System::System(const std::string_view &proteinFile, const std::string_view &optionsFile)
    :  _pointCharges(), _center(), _basisMatrix(Eigen::Matrix3d::Identity()), _region(nullptr),
    _name(proteinFile), _gen(std::random_device()()), _numberOfSamples(0){

    _loadOptions(optionsFile);
    if(_region == nullptr){
        throw std::exception();
    }

    _loadPDB();
    if(_pointCharges.empty()){
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

    SPDLOG_INFO("======[Sampling topology]======");
    SPDLOG_INFO("[Npoints] ==>> {}", _numberOfSamples);
    SPDLOG_INFO("[Threads] ==>> {}", procs);
    std::vector<PathSample> shared_array(_numberOfSamples);

    std::shared_ptr<spdlog::logger> thread_logger;

    if(! (thread_logger = spdlog::get("Thread"))){
        thread_logger = spdlog::stdout_logger_mt("Thread");
    }

    if (procs == 1){
        std::vector<size_t> values(_numberOfSamples);
        std::iota(std::begin(values), std::end(values), 0);
        _sampleLoop(shared_array, values);
    }
    else{
#if SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_DEBUG
        thread_logger->set_pattern("[Thread: %t] [%l] %v");
#else
        thread_logger->set_pattern("[Thread: %t] ==>> %v");
#endif

        auto chunks = chunkIndex(procs, _numberOfSamples);

        SPDLOG_INFO("====[Initializing threads]====");
        std::vector<std::thread> workers;
        workers.reserve(procs);

        for(size_t i = 0; i < procs; i++){
            workers.emplace_back(std::thread(&System::_sampleLoop, this, std::ref(shared_array), std::ref(chunks[i])));
        }
        for(auto& thread : workers){
            thread.join();
        }

    }
//    for(size_t i = 0; i < _numberOfSamples; i++){
//        SPDLOG_INFO("{}, {}", shared_array[i].distance, shared_array[i].curvature);
//    }
}

void System::_loadPDB(){
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

    SPDLOG_INFO("Loaded in {} point charges from file {}", _pointCharges.size(), _name);
}

void System::_loadOptions(const std::string_view& optionsFile){
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

    SPDLOG_INFO("=====[Options | {}]=====", optionsFile);
    SPDLOG_INFO("[V1]     ==>> [ {} ]", _basisMatrix.block(0,0,3,1).transpose());
    SPDLOG_INFO("[V2]     ==>> [ {} ]", _basisMatrix.block(0,1,3,1).transpose());
    SPDLOG_INFO("[V3]     ==>> [ {} ]", _basisMatrix.block(0,2,3,1).transpose());
    SPDLOG_INFO("[Center] ==>> [ {} ]", _center.transpose());
    SPDLOG_INFO("[Region] ==>> {}", _region->description());
}

void System::_sample(std::vector<PathSample>& output, size_t i) noexcept(true) {
    _mutex.lock();
    Eigen::Vector3d initialPosition = _region->randomPoint();
    const int maxSteps = _distribution(_gen);
    _mutex.unlock();

    Eigen::Vector3d finalPosition = initialPosition;

    int steps = 0;

    while(_region->isInside(finalPosition) && ++steps < maxSteps){
        finalPosition = _next(finalPosition);
    }

    output[i] = {(finalPosition - initialPosition).norm(),
    (_curvature(finalPosition) + _curvature(initialPosition))/2.0};

}


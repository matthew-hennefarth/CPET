/*
 * C++ STL HEADER FILES
 */
#include <thread>
#include <cmath>
#include <array>

/*
 * EXTERNAL LIBRARY HEADER FILES
 */
#include "spdlog/sinks/stdout_sinks.h"
#include "cs_libguarded/cs_plain_guarded.h"

/*
 * CPET HEADER FILES
 */
#include "System.h"
#include "Instrumentation.h"
#include "RAIIThread.h"

#define PERM_SPACE 0.0055263495

System::System(std::vector<PointCharge> pc, const Option& options) : _pointCharges(std::move(pc)){

    /* TODO can I make each of these functions? i bet I can...*/
    if (options.centerID != AtomID::origin){
        auto centerPC = find_if(begin(_pointCharges), end(_pointCharges),
                          [&options](const auto& pc){ return (pc.id == options.centerID); });
        if (centerPC != end(_pointCharges)){
            _center = centerPC->coordinate;
        }
        else{
            throw std::exception();
        }
    }

    std::array<Eigen::Vector3d, 3> basis;
    if(options.direction1ID != AtomID::e1){
        auto basis1PC = find_if(begin(_pointCharges), end(_pointCharges),
                           [&options](const auto& pc){ return pc.id == options.direction1ID; });

        if (basis1PC != end(_pointCharges)){
            basis[0] = basis1PC->coordinate;
        }
        else{
            throw std::exception();
        }
    }
    else{
        basis[0] = {1.0,0.0,0.0};
    }

    if(options.direction2ID != AtomID::e2){
        auto basis2PC = find_if(begin(_pointCharges), end(_pointCharges),
                                [&options](const auto& pc){ return pc.id == options.direction2ID; });

        if (basis2PC != end(_pointCharges)){
            basis[1] = basis2PC->coordinate;
        }
        else{
            throw std::exception();
        }
    }
    else{
        basis[1] = {0.0, 1.0, 0.0};
    }

    basis[2] = basis[0].cross(basis[1]);
    basis[1] = basis[2].cross(basis[0]);

    for(size_t i = 0; i < basis.size(); i++){
        _basisMatrix.block(0, static_cast<Eigen::Index>(i), 3, 1) = basis[i];
    }

    _pointCharges.erase(remove_if(begin(_pointCharges), end(_pointCharges),
                                  [](const auto& pc){ return pc.charge == 0.0; }),
                        end(_pointCharges));

}

Eigen::Vector3d System::electricField(const Eigen::Vector3d &position) const noexcept(true) {
    Eigen::Vector3d result(0, 0, 0);

    for (const auto &pc : _pointCharges) {
        Eigen::Vector3d d = (position - pc.coordinate);
        double dNorm = d.norm();
        result += ((pc.charge * d) / (dNorm * dNorm * dNorm));
    }
    result /= (1.0 / (4.0 * M_PI * PERM_SPACE));
    return result;
}

std::vector<PathSample> System::calculateTopology(const size_t &procs, const TopologyRegion& topologicalRegion) {
    std::vector<PathSample> sampleResults;
    sampleResults.reserve(topologicalRegion.numberOfSamples);

    std::shared_ptr<spdlog::logger> thread_logger;
    if (!(thread_logger = spdlog::get("Thread"))) {
        thread_logger = spdlog::stdout_logger_mt("Thread");
    }

    if (procs == 1) {
        size_t samples = topologicalRegion.numberOfSamples;
        while(samples-- > 0)
            sampleResults.emplace_back(_sample(topologicalRegion.volume));

        SPDLOG_INFO("{} Points calculated", topologicalRegion.numberOfSamples);
    }
    else {

#if SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_DEBUG
        thread_logger->set_pattern("[Thread: %t] [%l] %v");
#else
        thread_logger->set_pattern("[Thread: %t] ==>> %v");
#endif

        /* Initialize thread-safe data types */
        std::atomic_int samples = static_cast<int>(topologicalRegion.numberOfSamples);
        libguarded::plain_guarded<std::vector<PathSample>> shared_vector;
        {
            auto sharedVectorHandle = shared_vector.lock();
            sharedVectorHandle->reserve(topologicalRegion.numberOfSamples);
        }

        SPDLOG_INFO("====[Initializing threads]====");
        {
            std::vector<RAIIThread> workers;
            workers.reserve(procs);

            for (size_t i = 0; i < procs; i++) {
                workers.emplace_back([&samples, &shared_vector, &topologicalRegion, this]() {
                    auto thread_logger = spdlog::get("Thread");
                    thread_logger->info("Starting");
                    int completed = 0;
                    while (samples-- > 0) {
                        thread_logger->debug("Computing sample {}", samples + 1);
                        auto s = _sample(topologicalRegion.volume);
                        {
                            auto vector_handler = shared_vector.lock();
                            vector_handler->push_back(s);
                        }
                        completed++;
                    }
                    thread_logger->info("{} Points calculated", completed);
                });
            }
        }

        sampleResults = *(shared_vector.lock());
    }
    return sampleResults;
}

PathSample System::_sample(const std::unique_ptr<Volume>& region) const noexcept(true) {
    /* This is not thread-safe, however, implementation is thread-safe */
    Eigen::Vector3d initialPosition = region->randomPoint();
    /* This is not thread-safe, however, implementation is thread-safe */
    const int maxSteps = region->randomDistance(STEP_SIZE);

    Eigen::Vector3d finalPosition = initialPosition;
    int steps = 0;

    while (region->isInside(finalPosition) && ++steps < maxSteps)
        finalPosition = _next(finalPosition);

    return {(finalPosition - initialPosition).norm(),
                 (_curvature(finalPosition) + _curvature(initialPosition)) / 2.0};

}

double System::_curvature(const Eigen::Vector3d &alpha_0) const noexcept(true) {
    Eigen::Vector3d alpha_prime = electricField(alpha_0);
    Eigen::Vector3d alpha_1 = _next(alpha_0);

    /* Measures how much "time" we spent going forward */
    /* delta alpha/delta t = E, in the limit of delta t -> 0 */
    /* then we have delta alpha/E = delta t */
    double delta_t = (alpha_1 - alpha_0).norm() / alpha_prime.norm();

    /* Simple directional derivative of the electric field in that direction */
    Eigen::Vector3d alpha_prime_prime = (electricField(alpha_1) - alpha_prime) / delta_t;

    double alpha_prime_norm = alpha_prime.norm();

    return (alpha_prime.cross(alpha_prime_prime)).norm() / (alpha_prime_norm * alpha_prime_norm * alpha_prime_norm);
}


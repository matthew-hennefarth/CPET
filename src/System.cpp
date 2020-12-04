/* C++ STL HEADER FILES */
#include <cmath>
#include <array>

/* EXTERNAL LIBRARY HEADER FILES */
#include "spdlog/sinks/stdout_sinks.h"
#include "cs_libguarded/cs_plain_guarded.h"

/* CPET HEADER FILES */
#include "System.h"
#include "Instrumentation.h"
#include "RAIIThread.h"

#define PERM_SPACE 0.0055263495

System::System(std::vector<PointCharge> pc, const Option& options) : pointCharges_(std::move(pc)){

    if (options.centerID != AtomID::Constants::origin){
        center_ = PointCharge::find(pointCharges_, options.centerID)->coordinate;
    }

    std::array<Eigen::Vector3d, 3> basis;
    if(options.direction1ID != AtomID::Constants::e1){
        basis[0] = PointCharge::find(pointCharges_, options.direction1ID)->coordinate;
    }
    else{
        basis[0] = {1.0,0.0,0.0};
    }

    if(options.direction2ID != AtomID::Constants::e2){
        basis[1] = PointCharge::find(pointCharges_, options.direction2ID)->coordinate;
    }
    else{
        basis[1] = {0.0, 1.0, 0.0};
    }

    constructOrthonormalBasis_(basis);

    for(size_t i = 0; i < basis.size(); i++){
        basisMatrix_.block(0, static_cast<Eigen::Index>(i), 3, 1) = basis[i];
    }

    pointCharges_.erase(remove_if(begin(pointCharges_), end(pointCharges_),
                                        [](const auto& pc){
                                            return pc.charge == 0.0;
                                        }),
                        end(pointCharges_));

}

Eigen::Vector3d System::electricFieldAt(const Eigen::Vector3d &position) const noexcept {
    Eigen::Vector3d result(0, 0, 0);

    Eigen::Vector3d d;
    double dNorm;
    for (const auto &pc : pointCharges_) {
        d = (position - pc.coordinate);
        dNorm = d.norm();
        result += ((pc.charge * d) / (dNorm * dNorm * dNorm));
    }
    result /= (1.0 / (4.0 * M_PI * PERM_SPACE));
    return result;
}

std::vector<PathSample> System::electricFieldTopologyIn(int numOfThreads, const TopologyRegion& topologicalRegion) {
    std::vector<PathSample> sampleResults;
    sampleResults.reserve(static_cast<size_t>(topologicalRegion.numberOfSamples));

    std::shared_ptr<spdlog::logger> thread_logger;
    if (!(thread_logger = spdlog::get("Thread"))) {
        thread_logger = spdlog::stdout_logger_mt("Thread");
    }

    if (numOfThreads == 1) {
        int samples = topologicalRegion.numberOfSamples;
        while(samples-- > 0)
            sampleResults.emplace_back(sampleElectricFieldTopologyIn_(*topologicalRegion.volume));

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
            sharedVectorHandle->reserve(static_cast<size_t>(topologicalRegion.numberOfSamples));
        }

        SPDLOG_INFO("====[Initializing threads]====");
        {
            std::vector<RAIIThread> workers;
            workers.reserve(static_cast<size_t>(numOfThreads));

            for (int i = 0; i < numOfThreads; i++) {
                workers.emplace_back([&samples, &shared_vector, &topologicalRegion, this]() {
                    auto thread_logger = spdlog::get("Thread");
                    thread_logger->info("Starting");
                    int completed = 0;
                    while (samples-- > 0) {
                        thread_logger->debug("Computing sample {}", samples + 1);
                        auto s = sampleElectricFieldTopologyIn_(*topologicalRegion.volume);
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

PathSample System::sampleElectricFieldTopologyIn_(const Volume& region) const noexcept(true) {
    /* This is not thread-safe, however, implementation is thread-safe */
    Eigen::Vector3d initialPosition = region.randomPoint();
    /* This is not thread-safe, however, implementation is thread-safe */
    const int maxSteps = region.randomDistance(STEP_SIZE);

    Eigen::Vector3d finalPosition = initialPosition;
    int steps = 0;

    while (region.isInside(finalPosition) && ++steps < maxSteps)
        finalPosition = nextPoint_(finalPosition);

    return {(finalPosition - initialPosition).norm(),
                 (curvatureAt_(finalPosition) + curvatureAt_(initialPosition)) / 2.0};

}

double System::curvatureAt_(const Eigen::Vector3d &alpha_0) const noexcept {
    Eigen::Vector3d alpha_prime = electricFieldAt(alpha_0);
    Eigen::Vector3d alpha_1 = nextPoint_(alpha_0);

    /* Measures how much "time" we spent going forward */
    /* delta alpha/delta t = E, in the limit of delta t -> 0 */
    /* then we have delta alpha/E = delta t */
    double delta_t = (alpha_1 - alpha_0).norm() / alpha_prime.norm();

    /* Simple directional derivative of the electric field in that direction */
    Eigen::Vector3d alpha_prime_prime = (electricFieldAt(alpha_1) - alpha_prime) / delta_t;

    double alpha_prime_norm = alpha_prime.norm();

    return (alpha_prime.cross(alpha_prime_prime)).norm() / (alpha_prime_norm * alpha_prime_norm * alpha_prime_norm);
}


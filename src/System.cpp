// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

/* C++ STL HEADER FILES */
#include <array>
#include <cmath>

/* EXTERNAL LIBRARY HEADER FILES */
#include "cs_libguarded/cs_plain_guarded.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/stdout_sinks.h"

/* CPET HEADER FILES */
#include "Instrumentation.h"
#include "RAIIThread.h"
#include "System.h"

#define PERM_SPACE 0.0055263495

System::System(std::vector<PointCharge> pc, const Option& options)
    : pointCharges_(std::move(pc)) {
  if (options.centerID.position()) {
    center_ = *(options.centerID.position());
  } else {
    center_ = PointCharge::find(pointCharges_, options.centerID)->coordinate;
  }

  std::array<Eigen::Vector3d, 3> basis;

  if (options.direction1ID.position()) {
    if (options.direction1ID.isConstant()) {
      SPDLOG_DEBUG("Using constant direction for direction 1");
      basis[0] = *(options.direction1ID.position());
    } else {
      SPDLOG_DEBUG("Using user defined vector for direction 1");
      basis[0] = *(options.direction1ID.position()) - center_;
    }
  } else {
    basis[0] =
        PointCharge::find(pointCharges_, options.direction1ID)->coordinate -
        center_;
  }
  SPDLOG_DEBUG("Basis[0] is {}", basis[0].transpose());
  basis[0] = basis[0] / basis[0].norm();
  SPDLOG_DEBUG("Normalized, basis[0] is {}", basis[0].transpose());

  if (options.direction2ID.position()) {
    if (options.direction2ID.isConstant()) {
      SPDLOG_DEBUG("Using constant direction for direction 2");
      basis[1] = *(options.direction2ID.position());
    } else {
      SPDLOG_DEBUG("Using user defined vector for direction 2");
      basis[1] = *(options.direction2ID.position()) - center_;
    }
  } else {
    basis[1] =
        PointCharge::find(pointCharges_, options.direction2ID)->coordinate -
        center_;
  }
  SPDLOG_DEBUG("Basis[1] is {}", basis[1].transpose());
  basis[1] = basis[1] / basis[1].norm();
  SPDLOG_DEBUG("Normalized, basis[1] is {}", basis[1].transpose());

  constructOrthonormalBasis_(basis);

  SPDLOG_DEBUG("Final Basis Vectors:");
#if SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_DEBUG
  for (const auto& b : basis) {
    SPDLOG_DEBUG("{}", b.transpose());
  }
#endif

  SPDLOG_DEBUG("Constructing basis matrix...");
  for (size_t i = 0; i < basis.size(); i++) {
    basisMatrix_.block(0, static_cast<Eigen::Index>(i), 3, 1) = basis[i];
  }
  if (basisMatrix_.determinant() == 0) {
    SPDLOG_ERROR("Basis is not linearly independent");
    throw cpet::value_error("Basis is not linearly independent");
  }

  SPDLOG_DEBUG("Removing point charges with charge of 0...");
  pointCharges_.erase(
      remove_if(begin(pointCharges_), end(pointCharges_),
                [](const auto& pc) { return pc.charge == 0.0; }),
      end(pointCharges_));
}

Eigen::Vector3d System::electricFieldAt(const Eigen::Vector3d& position) const {
  SPDLOG_DEBUG("Computing field at {}...", position.transpose());
  Eigen::Vector3d result(0, 0, 0);

  Eigen::Vector3d d;
  double dNorm;
  for (const auto& pc : pointCharges_) {
    d = (position - pc.coordinate);
    dNorm = d.norm();
    result += ((pc.charge * d) / (dNorm * dNorm * dNorm));
  }
  result *= (1.0 / (4.0 * M_PI * PERM_SPACE));
  SPDLOG_DEBUG("Field is {}", result.transpose());
  return result;
}

std::vector<PathSample> System::electricFieldTopologyIn(
    int numOfThreads, const TopologyRegion& topologicalRegion) {
  std::vector<PathSample> sampleResults;
  sampleResults.reserve(static_cast<size_t>(topologicalRegion.numberOfSamples));

  std::shared_ptr<spdlog::logger> thread_logger;
  if (!(thread_logger = spdlog::get("Thread"))) {
    thread_logger = spdlog::stdout_logger_mt("Thread");
  }

  if (numOfThreads == 1) {
    SPDLOG_DEBUG("Single thread...");
    int samples = topologicalRegion.numberOfSamples;
    while (samples-- > 0)
      sampleResults.emplace_back(
          sampleElectricFieldTopologyIn_(*topologicalRegion.volume));

    SPDLOG_INFO("{} Points calculated", topologicalRegion.numberOfSamples);
  } else {
    SPDLOG_DEBUG("Multi-threads: {}", numOfThreads);
#if SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_DEBUG
    thread_logger->set_pattern("[Thread: %t] [%l] %v");
#else
    thread_logger->set_pattern("[Thread: %t] ==>> %v");
#endif

    /* Initialize thread-safe data types */
    SPDLOG_DEBUG("Initializing data structures...");
    std::atomic_int samples =
        static_cast<int>(topologicalRegion.numberOfSamples);
    libguarded::plain_guarded<std::vector<PathSample>> shared_vector;
    {
      auto sharedVectorHandle = shared_vector.lock();
      sharedVectorHandle->reserve(
          static_cast<size_t>(topologicalRegion.numberOfSamples));
    }

    SPDLOG_INFO("====[Initializing threads]====");
    {
      std::vector<RAIIThread> workers;
      workers.reserve(static_cast<size_t>(numOfThreads));

      for (int i = 0; i < numOfThreads; i++) {
        workers.emplace_back([&samples, &shared_vector, &topologicalRegion,
                              this]() {
          auto thread_logger = spdlog::get("Thread");
          thread_logger->info("Spinning up...");
          int completed = 0;
          while (samples-- > 0) {
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

PathSample System::sampleElectricFieldTopologyIn_(const Volume& region) const
    noexcept(true) {
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

double System::curvatureAt_(const Eigen::Vector3d& alpha_0) const noexcept {
  SPDLOG_DEBUG("Calculating curvature of field at {}", alpha_0.transpose());
  Eigen::Vector3d alpha_prime = electricFieldAt(alpha_0);
  Eigen::Vector3d alpha_1 = nextPoint_(alpha_0);

  /* Measures how much "time" we spent going forward */
  /* delta alpha/delta t = E, in the limit of delta t -> 0 */
  /* then we have delta alpha/E = delta t */
  double delta_t = (alpha_1 - alpha_0).norm() / alpha_prime.norm();

  /* Simple directional derivative of the electric field in that direction */
  Eigen::Vector3d alpha_prime_prime =
      (electricFieldAt(alpha_1) - alpha_prime) / delta_t;

  double alpha_prime_norm = alpha_prime.norm();

  return (alpha_prime.cross(alpha_prime_prime)).norm() /
         (alpha_prime_norm * alpha_prime_norm * alpha_prime_norm);
}


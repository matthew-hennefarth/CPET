// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

/* C++ STL HEADER FILES */
#include <array>
#include <cmath>

/* EXTERNAL LIBRARY HEADER FILES */
#include <cs_plain_guarded.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_sinks.h>

/* CPET HEADER FILES */
#include "Instrumentation.h"
#include "RAIIThread.h"
#include "System.h"

namespace cpet {

System::System(Frame frame, const Option& options)
    : frame_(std::move(frame)), pointCharges_(frame_.begin(), frame_.end()) {
  if (options.centerID().position()) {
    center_ = *(options.centerID().position());
  } else {
    center_ = frame_.find(options.centerID())->coordinate;
  }

  std::array<Eigen::Vector3d, 3> basis;

  if (options.direction1ID().position()) {
    if (options.direction1ID().isConstant()) {
      SPDLOG_DEBUG("Using constant direction for direction 1");
      basis[0] = *(options.direction1ID().position());
    } else {
      SPDLOG_DEBUG("Using user defined vector for direction 1");
      basis[0] = *(options.direction1ID().position()) - center_;
    }
  } else {
    basis[0] = frame_.find(options.direction1ID())->coordinate - center_;
  }
  SPDLOG_DEBUG("Basis[0] is {}", basis[0].transpose());
  basis[0] = basis[0] / basis[0].norm();
  SPDLOG_DEBUG("Normalized, basis[0] is {}", basis[0].transpose());

  if (options.direction2ID().position()) {
    if (options.direction2ID().isConstant()) {
      SPDLOG_DEBUG("Using constant direction for direction 2");
      basis[1] = *(options.direction2ID().position());
    } else {
      SPDLOG_DEBUG("Using user defined vector for direction 2");
      basis[1] = *(options.direction2ID().position()) - center_;
    }
  } else {
    basis[1] = frame_.find(options.direction2ID())->coordinate - center_;
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
    basisMatrix_.block(0, static_cast<Eigen::Index>(i), 3, 1) = basis.at(i);
  }
  if (basisMatrix_.determinant() == 0) {
    SPDLOG_ERROR("Basis is not linearly independent");
    throw cpet::value_error("Basis is not linearly independent");
  }

  SPDLOG_DEBUG("Removing point charges with charge of 0...");
  pointCharges_.erase(remove_if(begin(pointCharges_), end(pointCharges_),
                                [](const auto& p) { return p.charge == 0.0; }),
                      end(pointCharges_));
}

Eigen::Vector3d System::electricFieldAt(const Eigen::Vector3d& position) const {
  Eigen::Vector3d result(0, 0, 0);
  constexpr double PERM_SPACE = 0.0055263495;
  constexpr double TO_V_PER_ANG = (1.0 / (4.0 * M_PI * PERM_SPACE));

  /* If we can speed this up, we should!! */
  for (const auto& pc : pointCharges_) {
    const Eigen::Vector3d d = (position - pc.coordinate);
    const auto dNorm = d.norm();
    result += ((pc.charge * d) / (dNorm * dNorm * dNorm));
  }
  result *= TO_V_PER_ANG;
  return result;
}

std::vector<PathSample> System::electricFieldTopologyIn(
    int numOfThreads, const Volume& volume, const double stepsize,
    const int numberOfSamples) const {
  std::vector<PathSample> sampleResults;
  sampleResults.reserve(static_cast<size_t>(numberOfSamples));

  std::shared_ptr<spdlog::logger> thread_logger;
  if (!(thread_logger = spdlog::get("Thread"))) {
    thread_logger = spdlog::stdout_logger_mt("Thread");
  }

  if (numOfThreads == 1) {
    SPDLOG_DEBUG("Single thread...");
    int samples = numberOfSamples;
    while (samples-- > 0) {
      sampleResults.emplace_back(
          sampleElectricFieldTopologyIn_(volume, stepsize));
    }
    SPDLOG_INFO("{} Points calculated", numberOfSamples);
  } else {
    SPDLOG_DEBUG("Multi-threads: {}", numOfThreads);
#if SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_DEBUG
    thread_logger->set_pattern("[Thread: %t] [%l] %v");
#else
    thread_logger->set_pattern("[Thread: %t] ==>> %v");
#endif

    /* Initialize thread-safe data types */
    SPDLOG_DEBUG("Initializing data structures...");
    std::atomic_int samples = numberOfSamples;
    libguarded::plain_guarded<std::vector<PathSample>> shared_vector;
    {
      auto sharedVectorHandle = shared_vector.lock();
      sharedVectorHandle->reserve(static_cast<size_t>(numberOfSamples));
    }

    SPDLOG_INFO("====[Initializing threads]====");
    {
      const auto thread_work = [&, this]() {
        auto this_thread_logger = spdlog::get("Thread");
        this_thread_logger->info("Spinning up...");
        int completed = 0;
        while (samples-- > 0) {
          auto s = sampleElectricFieldTopologyIn_(volume, stepsize);
          {
            auto vector_handler = shared_vector.lock();
            vector_handler->push_back(s);
          }
          completed++;
        }
        this_thread_logger->info("{} Points calculated", completed);
      };

      std::vector<util::RAIIThread> workers;
      workers.reserve(static_cast<size_t>(numOfThreads));
      for (int i = 0; i < numOfThreads; i++) {
        workers.emplace_back(thread_work);
      }
    }
    SPDLOG_DEBUG("Gathering results from shared vector");
    sampleResults = *(shared_vector.lock());
  }
  return sampleResults;
}

PathSample System::sampleElectricFieldTopologyIn_(const Volume& region,
                                                  const double stepSize) const
    noexcept(true) {
  /* This is not thread-safe, however, implementation is thread-safe */
  const Eigen::Vector3d initialPosition = region.randomPoint();
  /* This is not thread-safe, however, implementation is thread-safe */
  const int maxSteps = region.randomDistance(stepSize);

  Eigen::Vector3d finalPosition = initialPosition;
  SPDLOG_DEBUG("Initial position {}", initialPosition.transpose());
  int steps = 0;

  while (region.isInside(finalPosition) && ++steps < maxSteps) {
    finalPosition = nextPoint_(finalPosition, stepSize);
    SPDLOG_DEBUG("Updated position: {}", finalPosition.transpose());
  }

  SPDLOG_DEBUG("Initial position {}", initialPosition.transpose());
  SPDLOG_DEBUG("Final position: {}", finalPosition.transpose());
  SPDLOG_DEBUG("Num of steps: {}", steps);
  SPDLOG_DEBUG("Distance between end and start: {}",
               (finalPosition - initialPosition).norm());

  return {(finalPosition - initialPosition).norm(),
          (curvatureAt_(finalPosition, stepSize) +
           curvatureAt_(initialPosition, stepSize)) /
              2.0};
}

double System::curvatureAt_(const Eigen::Vector3d& alpha_0,
                            const double stepSize) const noexcept {
  SPDLOG_DEBUG("Calculating curvature of field at {}", alpha_0.transpose());

  Eigen::Vector3d alpha_1 = nextPoint_(alpha_0, stepSize);
  Eigen::Vector3d alpha_2 = nextPoint_(alpha_1, stepSize);

  Eigen::Vector3d alpha_0_prime = alpha_1 - alpha_0;
  Eigen::Vector3d alpha_1_prime = alpha_2 - alpha_1;

  Eigen::Vector3d alpha_0_prime_prime = alpha_1_prime - alpha_0_prime;

  double alpha_prime_norm = alpha_0_prime.norm();

  return (alpha_0_prime.cross(alpha_0_prime_prime)).norm() /
         (alpha_prime_norm * alpha_prime_norm * alpha_prime_norm);
}
std::vector<Eigen::Vector3d> System::computeElectricFieldIn(
    const EFieldVolume& volume) const noexcept {
  const auto compute_volume_in_system =
      [this](const Eigen::Vector3d& position) -> Eigen::Vector3d {
    return this->electricFieldAt(position);
  };
  std::vector<Eigen::Vector3d> results;
  results.reserve(volume.points().size());
  std::transform(volume.points().begin(), volume.points().end(),
                 std::back_inserter(results), compute_volume_in_system);
  return results;
}
}  // namespace cpet

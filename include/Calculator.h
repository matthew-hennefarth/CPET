// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef CALCULATOR_H
#define CALCULATOR_H

/* C++ STL HEADER FILES */
#include <string>
#include <vector>
#include <algorithm>

/* EXTERNAL LIBRARY HEADER FILES */
#include <Eigen/Dense>

/* CPET HEADER FILES */
#include "Option.h"
#include "PointCharge.h"
#include "System.h"
#include "TopologyRegion.h"
namespace cpet {

class Calculator {
 public:
  Calculator(std::string proteinFile, const std::string& optionFile,
             std::string chargesFile = "", int nThreads = 1);

  void compute();

  inline void setOutputFilePrefix(const std::string& prefix) {
    outputPrefix_ = prefix;
  }

 private:
  std::string proteinFile_;

  std::string outputPrefix_;

  Option option_;

  std::string chargeFile_;

  int numberOfThreads_;

  std::vector<std::vector<PointCharge>> pointChargeTrajectory_;

  std::vector<System> systems_;

  void fixCharges_();

  void computeTopology_() const;

  void computeEField_() const;

  void computeVolume_() const;

  void loadPointChargeTrajectory_();

  inline void createSystems_() {
    if (!systems_.empty()) {
      systems_.clear();
    }

    const auto make_system =
        [this](const std::vector<PointCharge>& trajectory) -> System {
      return System{trajectory, this->option_};
    };

    systems_.reserve(pointChargeTrajectory_.size());
    std::transform(pointChargeTrajectory_.begin(), pointChargeTrajectory_.end(),
                   std::back_inserter(systems_), make_system);
  }

  inline void transformSystems_() {
    std::for_each(systems_.begin(), systems_.end(),
                  [](auto& system) { system.transformToUserSpace(); });
  }

  [[nodiscard]] std::vector<double> loadChargesFile_() const;

  void writeTopologyResults_(const std::vector<PathSample>& data,
                             const TopologyRegion& region, int i) const;

  void writeEFieldResults_(
      const std::vector<std::vector<Eigen::Vector3d>>& results) const;
};
}  // namespace cpet
#endif  // CALCULATOR_H

// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef OPTION_H
#define OPTION_H

/* C++ STL HEADER FILES */
#include <string>
#include <vector>

/* EXTERNAL LIBRARY HEADER FILES */
#include <Eigen/Dense>

/* CPET HEADER FILES */
#include "AtomID.h"
#include "EFieldVolume.h"
#include "TopologyRegion.h"

class Option {
 public:
  Option() = default;

  explicit Option(const std::string& optionFile);

  AtomID centerID{AtomID::Constants::origin};

  AtomID direction1ID{AtomID::Constants::e1};

  AtomID direction2ID{AtomID::Constants::e2};

  std::vector<AtomID> calculateEFieldPoints;

  std::vector<TopologyRegion> calculateEFieldTopology;

  std::vector<EFieldVolume> calculateEFieldVolumes;

 private:
  void loadOptionsFromFile_(const std::string& optionFile);
};

#endif  // OPTION_H

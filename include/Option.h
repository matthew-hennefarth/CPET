// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef OPTION_H
#define OPTION_H

/* C++ STL HEADER FILES */
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

/* EXTERNAL LIBRARY HEADER FILES */
#include <Eigen/Dense>

/* CPET HEADER FILES */
#include "AtomID.h"
#include "EFieldVolume.h"
#include "TopologyRegion.h"
#include "Box.h"

constexpr const char* ALIGN_KEY = "align";
constexpr const char* TOPOLOGY_KEY = "topology";
constexpr const char* FIELD_KEY = "field";
constexpr const char* PLOT_3D_KEY = "plot3d";

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
  std::vector<std::string> simpleOptions_;
  std::unordered_map<std::string, std::vector<std::string>> blockOptions_;

  void loadOptionsDataFromFile_(const std::string& optionFile);
  void parseSimpleOptions_();
  void parseBlockOptions_();

  inline void parseAlignSimple_(const std::vector<std::string>& options) {
    if (options.size() != 1 && options.size() < 3) {
      throw cpet::invalid_option(
          "Invalid Option: align expects 1 or 3 identifiers");
    }
    centerID = options.at(0);
    if (options.size() > 1) {
      direction1ID = options.at(1);
      direction2ID = options.at(2);
    }
  }

  inline void parseTopologySimple_(const std::vector<std::string>& options) {
    if (options.size() < 5) {
      throw cpet::invalid_option(
          "Invalid Option: topology expects 5 parameters");
    }
    if (options.at(0) == "box") {
      std::array<double, 3> dims = {
          std::stod(options[1]), std::stod(options[2]), std::stod(options[3])};
      int samples = std::stoi(options[4]);
      for (const auto& d : dims) {
        if (d <= 0) {
          throw cpet::invalid_option(
              "Invalid Option: box dimensions should be positive valued");
        }
      }
      if (samples < 0) {
        throw cpet::invalid_option(
            "Invalid Option: topology samples should be non-negative");
      }
      calculateEFieldTopology.emplace_back(std::make_unique<Box>(dims),
                                           samples);
    } else {
      throw cpet::invalid_option("Invalid Option: unknown volume");
    }
  }
  inline void parseFieldSimple_(const std::vector<std::string>& options) {
    for (const auto& location : options) {
      calculateEFieldPoints.emplace_back(location);
    }
  }

  void parsePlot3dSimple_(const std::vector<std::string>& options) {
    bool plot = false;
    if (options.empty()) {
      throw cpet::invalid_option(
          "Invalid Option: plot3d expects at least 7 options");
    }
    std::vector<std::string> regionOptions;

    if (options[0] == "show") {
      plot = true;
      regionOptions =
          std::vector<std::string>(options.begin() + 1, options.end());
    } else {
      regionOptions = options;
    }
    if (regionOptions.size() < 7) {
      throw cpet::invalid_option(
          "Invalid Option: not enough options specified for plot3d, expected "
          "8");
    }
    if (regionOptions[0] == "box") {
      std::array<double, 3> dims = {std::stod(regionOptions[1]),
                                    std::stod(regionOptions[2]),
                                    std::stod(regionOptions[3])};
      calculateEFieldVolumes.emplace_back(
          std::make_unique<Box>(dims),
          std::array<int, 3>{std::stoi(regionOptions[4]),
                             std::stoi(regionOptions[5]),
                             std::stoi(regionOptions[6])});
    } else {
      throw cpet::invalid_option(
          "Invalid Option: Unknown volume specified for plot3d");
    }
  }

  std::unordered_map<std::string,
                     void (Option::*)(const std::vector<std::string>&)>
      parseSimpleOptionsMap_ = {{ALIGN_KEY, &Option::parseAlignSimple_},
                                {TOPOLOGY_KEY, &Option::parseTopologySimple_},
                                {FIELD_KEY, &Option::parseFieldSimple_},
                                {PLOT_3D_KEY, &Option::parsePlot3dSimple_}};
};

#endif  // OPTION_H

// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "FieldLocations.h"

/* EXTERNAL LIBRARY HEADER FILES */
#include <Eigen/Dense>

/* CPET HEADER FILES */
#include "System.h"

namespace cpet {

FieldLocations FieldLocations::fromSimple(
    const std::vector<std::string>& options) {
  FieldLocations fl;
  constexpr auto create_location = [](const std::string& location) -> AtomID {
    return AtomID{location};
  };
  std::transform(options.begin(), options.end(),
                 std::back_inserter(fl.locations_), create_location);
  return fl;
}

FieldLocations FieldLocations::fromBlock(
    const std::vector<std::string>& options) {
  constexpr const char* PLOT_KEY = "plot";
  constexpr const char* LOCATIONS_KEY = "locations";
  constexpr const char* OUTPUT_KEY = "output";

  FieldLocations fl;
  for (const auto& line : options) {
    const auto tokens = util::split(line, ' ');
    if (tokens.size() < 2) {
      continue;
    }

    const auto key = tokens[0];
    const std::vector<std::string> key_options{tokens.begin() + 1,
                                               tokens.end()};

    if (key == LOCATIONS_KEY) {
      constexpr auto create_location =
          [](const std::string& location) -> AtomID {
        return AtomID{location};
      };

      std::transform(key_options.begin(), key_options.end(),
                     std::back_inserter(fl.locations_), create_location);
    } else if (key == PLOT_KEY) {
      fl.plotStyle_ = decodePlotStyle_(key_options);
    } else if (key == OUTPUT_KEY) {
      fl.output_ = *key_options.begin();
    }
  }
  return fl;
}
void FieldLocations::computeEFieldsWith(
    const std::vector<System>& systems,
    const std::vector<std::vector<PointCharge>>& pointChargeTrajectory) const {
  if (systems.size() != pointChargeTrajectory.size()) {
    throw cpet::value_error(
        "Wrong number of systems to number of point charges structures");
  }

  std::vector<std::vector<Eigen::Vector3d>> results;
  for (const auto& point : locations_) {
    SPDLOG_INFO("=~=~=~=~[Field at {}]=~=~=~=~", point.ID());
    std::vector<Eigen::Vector3d> fieldTrajectoryAtPoint;

    for (size_t i = 0; i < systems.size(); i++) {
      Eigen::Vector3d location;

      if (point.position()) {
        location = *(point.position());
      } else {
        location =
            PointCharge::find(pointChargeTrajectory[i], point)->coordinate;
        location = systems[i].transformToUserSpace(location);
      }

      // systems[i].printCenterAndBasis();

      Eigen::Vector3d field = systems[i].electricFieldAt(location);
      SPDLOG_INFO("{} [{}]", field.transpose(), field.norm());
      fieldTrajectoryAtPoint.emplace_back(field);
    }
    results.push_back(fieldTrajectoryAtPoint);
  }
  if (output_) {
    writeOutput_(results);
  }
  if (showPlots()) {
    // plot_(results);
  }
}
void FieldLocations::writeOutput_(
    const std::vector<std::vector<Eigen::Vector3d>>& results) const {
  if (!output_) {
    return;
  }

  std::ofstream outFile(*output_, std::ios::out);
  if (outFile.is_open()) {
    for (size_t i = 0; i < results.size(); i++) {
      outFile << '#' << locations_[i].ID() << '\n';
      for (const Eigen::Vector3d& field : results[i]) {
        outFile << field.transpose() << '\n';
      }
    }
    outFile << std::flush;
  } else {
    SPDLOG_ERROR("Could not open file {}", *output_);
    throw cpet::io_error("Could not open file " + *output_);
  }
}

}  // namespace cpet
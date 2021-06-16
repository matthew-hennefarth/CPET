// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef OPTION_H
#define OPTION_H

/* C++ STL HEADER FILES */
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <utility>

/* EXTERNAL LIBRARY HEADER FILES */
#include <Eigen/Dense>

/* CPET HEADER FILES */
#include "AtomID.h"
#include "EFieldVolume.h"
#include "TopologyRegion.h"
#include "Box.h"
#include "FieldLocations.h"

namespace cpet {

constexpr const char* ALIGN_KEY = "align";
constexpr const char* TOPOLOGY_KEY = "topology";
constexpr const char* FIELD_KEY = "field";
constexpr const char* PLOT_3D_KEY = "plot3d";
constexpr const char* COORDINATE_START_INDEX_KEY = "coordinatesstart";
constexpr const char* COORDINATE_SKIP_INDEX_KEY = "coordinatesskip";

class Option {
 public:
  Option() = default;
  explicit Option(const std::string& optionFile);

  [[nodiscard]] constexpr const std::vector<FieldLocations>&
  calculateFieldLocations() const noexcept {
    return calculateFieldLocations_;
  }

  inline void addFieldLocations(const FieldLocations& fl) {
    calculateFieldLocations_.emplace_back(fl);
  }

  [[nodiscard]] constexpr const std::vector<EFieldVolume>&
  calculateEFieldVolumes() const noexcept {
    return calculateEFieldVolumes_;
  }

  [[nodiscard]] constexpr const std::vector<TopologyRegion>&
  calculateEFieldTopology() const noexcept {
    return calculateEFieldTopology_;
  }

  [[nodiscard]] constexpr const AtomID& centerID() const noexcept {
    return centerID_;
  }

  template <typename S1>
  constexpr void centerID(S1&& atomid) {
    centerID_ = std::forward<S1>(atomid);
  }

  [[nodiscard]] constexpr const AtomID& direction1ID() const noexcept {
    return direction1ID_;
  }

  [[nodiscard]] constexpr const AtomID& direction2ID() const noexcept {
    return direction2ID_;
  }

  [[nodiscard]] constexpr const int& coordinatesStartIndex() const noexcept {
    return coordinatesStartIndex_;
  }

  [[nodiscard]] constexpr const int& coordinatesStepSize() const noexcept {
    return coordinatesStepSize_;
  }

 private:
  std::vector<FieldLocations> calculateFieldLocations_;
  std::vector<EFieldVolume> calculateEFieldVolumes_;
  std::vector<TopologyRegion> calculateEFieldTopology_;
  AtomID centerID_{AtomID::Constants::origin};
  AtomID direction1ID_{AtomID::Constants::e1};
  AtomID direction2ID_{AtomID::Constants::e2};
  int coordinatesStartIndex_{0};
  int coordinatesStepSize_{1};
  std::vector<std::string> simpleOptions_;
  std::vector<std::pair<std::string, std::vector<std::string>>> blockOptions_;

  void loadOptionsDataFromFile_(const std::string& optionFile);

  void parseSimpleOptions_();

  void parseBlockOptions_();

  inline void parseAlignSimple_(const std::vector<std::string>& options) {
    if (options.size() != 1 && options.size() < 3) {
      throw cpet::invalid_option(
          "Invalid Option: align expects 1 or 3 identifiers");
    }
    centerID_ = options.at(0);
    if (options.size() > 1) {
      direction1ID_ = options.at(1);
      direction2ID_ = options.at(2);
    }
  }

  inline void parseCoordinateStartSimple_(const std::vector<std::string>& options) {
    if (options.empty()){
      SPDLOG_WARN("No parameters specified for coordinateStart!");
      return;
    }
    if (!util::isDouble(*options.begin())) {
      throw cpet::invalid_option("Invalid Option: coordinateStart option should be numeric");
    }
    coordinatesStartIndex_ = std::stoi(*options.begin());
    if (coordinatesStartIndex_ < 0) {
      throw cpet::invalid_option("Invalid Option: coordinateStart option should be >= 0");
    }
  }

  inline void parseCoordinateSkipSimple_(const std::vector<std::string>& options) {
    if (options.empty()){
      SPDLOG_WARN("No parameters specified for coordinateSkip!");
      return;
    }
    if (!util::isDouble(*options.begin())) {
      throw cpet::invalid_option("Invalid Option: coordinateSkip option should be numeric");
    }
    coordinatesStepSize_ = std::stoi(*options.begin());
    if (coordinatesStepSize_ <= 0) {
      throw cpet::invalid_option("Invalid Option: coordinateSkip option should be > 0");
    }
  }


  inline void parseTopologySimple_(const std::vector<std::string>& options) {
    calculateEFieldTopology_.emplace_back(TopologyRegion::fromSimple(options));
  }

  inline void parseTopologyBlock_(const std::vector<std::string>& options) {
    calculateEFieldTopology_.emplace_back(TopologyRegion::fromBlock(options));
  }

  inline void parseFieldSimple_(const std::vector<std::string>& options) {
    calculateFieldLocations_.emplace_back(FieldLocations::fromSimple(options));
  }

  inline void parseFieldBlock_(const std::vector<std::string>& options) {
    calculateFieldLocations_.emplace_back(FieldLocations::fromBlock(options));
  }

  inline void parsePlot3dSimple_(const std::vector<std::string>& options) {
    calculateEFieldVolumes_.emplace_back(EFieldVolume::fromSimple(options));
  }

  inline void parsePlot3dBlock_(const std::vector<std::string>& blockOptions) {
    calculateEFieldVolumes_.emplace_back(EFieldVolume::fromBlock(blockOptions));
  }
};
}  // namespace cpet
#endif  // OPTION_H

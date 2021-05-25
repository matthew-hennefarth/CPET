// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

/* C++ STL HEADER FILES */
#include <fstream>

/* EXTERNAL LIBRARY HEADER FILES */
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/spdlog.h"

/* CPET HEADER FILES */
#include "Calculator.h"
#include "Exceptions.h"
#include "Instrumentation.h"
#include "System.h"
#include "Utilities.h"

constexpr int PDB_XCOORD_START = 31;
constexpr int PDB_YCOORD_START = 39;
constexpr int PDB_ZCOORD_START = 47;
constexpr int PDB_CHARGE_START = 55;
constexpr int PDB_COORD_WIDTH = 8;
constexpr int PDB_CHARGE_WIDTH = 8;

Calculator::Calculator(std::string proteinFile, const std::string& optionFile,
                       std::string chargesFile, int nThreads)
    : proteinFile_(std::move(proteinFile)),
      outputPrefix_(proteinFile_),
      option_(optionFile),
      chargeFile_(std::move(chargesFile)),
      numberOfThreads_(nThreads) {
  loadPointChargeTrajectory_();
}

void Calculator::compute() {
  if (!chargeFile_.empty()) {
    fixCharges_();
  }

  createSystems_();
  transformSystems_();

  if (!option_.calculateEFieldTopology.empty()) {
    computeTopology_();
  }
  if (!option_.calculateEFieldPoints.empty()) {
    computeEField_();
  }
  if (!option_.calculateEFieldVolumes.empty()) {
    computeVolume_();
  }
}

void Calculator::computeTopology_() const {
  for (size_t i = 0; i < systems_.size(); i++) {
    SPDLOG_INFO("=~=~=~=~[Trajectory {}]=~=~=~=~", i);
    const System&  sys = systems_.at(i);
    sys.printCenterAndBasis();

    for (const auto& region : option_.calculateEFieldTopology) {
      SPDLOG_INFO("======[Sampling topology]======");
      SPDLOG_INFO("[Volume ]   ==>> {}", region.volume->description());
      SPDLOG_INFO("[Npoints]   ==>> {}", region.numberOfSamples);
      SPDLOG_INFO("[Threads]   ==>> {}", numberOfThreads_);
      SPDLOG_INFO("[STEP SIZE] ==>> {}", STEP_SIZE);
      std::vector<PathSample> results;
      {
        Timer t;
        results = sys.electricFieldTopologyIn(numberOfThreads_, region);
      }
      writeTopologyResults_(results, region, static_cast<int>(i));
    }
  }
}

void Calculator::computeEField_() const {
  std::vector<std::vector<Eigen::Vector3d>> results;
  for (const auto& point : option_.calculateEFieldPoints) {
    SPDLOG_INFO("=~=~=~=~[Field at {}]=~=~=~=~", point.ID());
    std::vector<Eigen::Vector3d> fieldTrajectoryAtPoint;

    for (size_t i = 0; i < systems_.size(); i++) {
      Eigen::Vector3d location;

      if (point.position()) {
        location = *(point.position());
      } else {
        location =
            PointCharge::find(pointChargeTrajectory_[i], point)->coordinate;
        location = systems_[i].transformToUserSpace(location);
      }

      systems_[i].printCenterAndBasis();

      Eigen::Vector3d field = systems_[i].electricFieldAt(location);
      SPDLOG_INFO("Field: {} [{}]", field.transpose(), field.norm());
      fieldTrajectoryAtPoint.emplace_back(field);
    }
    results.push_back(fieldTrajectoryAtPoint);
  }
  writeEFieldResults_(results);
}

void Calculator::computeVolume_() const {
  for (const auto& volume : option_.calculateEFieldVolumes) {
    std::vector<std::vector<Eigen::Vector3d>> volumeResults;
    volumeResults.reserve(systems_.size());

    for (const auto& system : systems_) {
      system.printCenterAndBasis();
      std::vector<Eigen::Vector3d> tmpSystemResults;
      tmpSystemResults.reserve(volume.points().size());

      for (const auto& position : volume.points()) {
        tmpSystemResults.push_back(system.electricFieldAt(position));
      }

      volumeResults.push_back(tmpSystemResults);
      if (volume.showPlot()) {
        volume.plot(tmpSystemResults);
      }
    }
    if (volume.output()) {
      volume.writeVolumeResults(systems_, volumeResults);
    }
  }
}

void Calculator::loadPointChargeTrajectory_() {
  SPDLOG_DEBUG("Loading point charge trajectory from {} ...", proteinFile_);
  std::vector<PointCharge> tmpHolder;
  forEachLineIn(proteinFile_, [this, &tmpHolder](const std::string& line) {
    if (line.rfind("ENDMDL", 0) == 0) {
      pointChargeTrajectory_.push_back(tmpHolder);
      tmpHolder.clear();
    } else if (line.rfind("ATOM", 0) == 0 || line.rfind("HETATM", 0) == 0) {
      tmpHolder.emplace_back(
          Eigen::Vector3d(
              {std::stod(line.substr(PDB_XCOORD_START, PDB_COORD_WIDTH)),
               std::stod(line.substr(PDB_YCOORD_START, PDB_COORD_WIDTH)),
               std::stod(line.substr(PDB_ZCOORD_START, PDB_COORD_WIDTH))}),
          std::stod(line.substr(PDB_CHARGE_START, PDB_CHARGE_WIDTH)),
          AtomID::generateID(line));
    }
  });
  if (!tmpHolder.empty()) {
    pointChargeTrajectory_.push_back(tmpHolder);
  }
}

std::vector<double> Calculator::loadChargesFile_() const {
  SPDLOG_DEBUG("Loading charges from external file {} ...", chargeFile_);
  std::vector<double> realCharges;
  forEachLineIn(chargeFile_, [&realCharges](const std::string& line) {
    if (line.rfind("ATOM", 0) == 0 || line.rfind("HETATM", 0) == 0) {
      realCharges.emplace_back(
          std::stod(line.substr(PDB_CHARGE_START, PDB_CHARGE_WIDTH)));
    }
  });
  return realCharges;
}

void Calculator::fixCharges_() {
  SPDLOG_DEBUG("Fixing charges in structure file with real charges...");
  auto realCharges = loadChargesFile_();

  for (auto& structure : pointChargeTrajectory_) {
    if (structure.size() != realCharges.size()) {
      SPDLOG_ERROR("Structure size: {}, number of charges: {}",
                   structure.size(), realCharges.size());
      throw cpet::value_error(
          "Inconsistent number of point charges in trajectory and in charge "
          "file");
    }

    for (size_t i = 0; i < structure.size(); i++) {
      structure[i].charge = realCharges[i];
    }
  }
}

void Calculator::writeTopologyResults_(const std::vector<PathSample>& data,
                                       const TopologyRegion& region,
                                       int i) const {
  SPDLOG_DEBUG("Writing topology results...");
  std::string file = outputPrefix_ + '_' + std::to_string(i) + '_' +
                     region.volume->type() + ".top";
  std::ofstream outFile(file, std::ios::out);
  if (outFile.is_open()) {
    outFile << '#' << proteinFile_ << ' ' << i << '\n';
    outFile << '#' << region.details() << '\n';
    /* TODO add options writing to this file...*/
    for (const auto& line : data) {
      outFile << line << '\n';
    }
    outFile << std::flush;
  } else {
    SPDLOG_ERROR("Could not open file {}", file);
    throw cpet::io_error("Could not open file " + file);
  }
}

void Calculator::writeEFieldResults_(
    const std::vector<std::vector<Eigen::Vector3d>>& results) const {
  std::string file = outputPrefix_ + ".field";
  std::ofstream outFile(file, std::ios::out);
  if (outFile.is_open()) {
    outFile << '#' << proteinFile_ << '\n';
    for (size_t i = 0; i < results.size(); i++) {
      outFile << '#' << option_.calculateEFieldPoints[i].ID() << '\n';
      for (const Eigen::Vector3d& field : results[i]) {
        outFile << field.transpose() << '\n';
      }
    }
    outFile << std::flush;
  } else {
    SPDLOG_ERROR("Could not open file {}", file);
    throw cpet::io_error("Could not open file " + file);
  }
}


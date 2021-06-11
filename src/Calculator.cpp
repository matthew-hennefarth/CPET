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

namespace cpet {

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

  if (!option_.calculateEFieldTopology().empty()) {
    computeTopology_();
  }
  if (!option_.calculateFieldLocations().empty()) {
    computeEField_();
  }
  if (!option_.calculateEFieldVolumes().empty()) {
    computeVolume_();
  }
}

void Calculator::computeTopology_() const {
  const auto print_header = [&](const TopologyRegion& region) {
    SPDLOG_INFO("======[Sampling topology]======");
    SPDLOG_INFO("[Volume ]   ==>> {}", region.volume->description());
    SPDLOG_INFO("[Npoints]   ==>> {}", region.numberOfSamples);
    SPDLOG_INFO("[Threads]   ==>> {}", numberOfThreads_);
    SPDLOG_INFO("[STEP SIZE] ==>> {}", STEP_SIZE);
  };

  const auto compute_topology = [this](const System& system,
                                       const TopologyRegion& region,
                                       const int index) {
    std::vector<PathSample> results;
    {
      Timer t;
      results = system.electricFieldTopologyIn(numberOfThreads_, region);
    }
    writeTopologyResults_(results, region, static_cast<int>(index));
  };

  std::for_each(systems_.begin(), systems_.end(),
                [&, index = 0](const auto& system) mutable {
                  SPDLOG_INFO("=~=~=~=~[Trajectory {}]=~=~=~=~", index);
                  system.printCenterAndBasis();
                  std::for_each(option_.calculateEFieldTopology().begin(),
                                option_.calculateEFieldTopology().end(),
                                [&](const auto& region) {
                                  print_header(region);
                                  compute_topology(system, region, index);
                                });
                  ++index;
                });
}

void Calculator::computeEField_() const {
  for (const auto& fieldLocations : option_.calculateFieldLocations()) {
    fieldLocations.computeEFieldsWith(systems_);
  }
}

void Calculator::computeVolume_() const {
  std::for_each(
      option_.calculateEFieldVolumes().begin(),
      option_.calculateEFieldVolumes().end(), [this](const auto& volume) {
        std::vector<std::vector<Eigen::Vector3d>> volumeResults;
        volumeResults.reserve(systems_.size());
        const auto compute_volume_data = [&volume](const System& system) {
          system.printCenterAndBasis();
          auto tmpSystemResults = system.computeElectricFieldIn(volume);
          if (volume.showPlot()) {
            volume.plot(tmpSystemResults);
          }
          return tmpSystemResults;
        };
        std::transform(systems_.begin(), systems_.end(),
                       std::back_inserter(volumeResults), compute_volume_data);
        if (volume.output()) {
          volume.writeVolumeResults(systems_, volumeResults);
        }
      });
}

void Calculator::loadPointChargeTrajectory_() {
  SPDLOG_DEBUG("Loading point charge trajectory from {} ...", proteinFile_);
  std::vector<PointCharge> tmpHolder;
  util::forEachLineIn(
      proteinFile_, [this, &tmpHolder](const std::string& line) {
        if (util::startswith(line, "ENDMDL")) {
          frameTrajectory_.emplace_back(tmpHolder);
          tmpHolder.clear();
        } else if (util::startswith(line, "ATOM") ||
                   util::startswith(line, "HETATM")) {
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
    frameTrajectory_.emplace_back(tmpHolder);
  }
}

std::vector<double> Calculator::loadChargesFile_() const {
  SPDLOG_DEBUG("Loading charges from external file {} ...", chargeFile_);
  std::vector<double> realCharges;
  util::forEachLineIn(chargeFile_, [&realCharges](const std::string& line) {
    if (util::startswith(line, "ATOM") || util::startswith(line, "HETATM")) {
      realCharges.emplace_back(
          std::stod(line.substr(PDB_CHARGE_START, PDB_CHARGE_WIDTH)));
    }
  });
  return realCharges;
}

void Calculator::fixCharges_() {
  SPDLOG_DEBUG("Fixing charges in structure file with real charges...");
  auto realCharges = loadChargesFile_();
  std::for_each(
      frameTrajectory_.begin(), frameTrajectory_.end(),
      [&realCharges](auto& frame) { frame.updateCharges(realCharges); });
}

void Calculator::writeTopologyResults_(const std::vector<PathSample>& data,
                                       const TopologyRegion& region,
                                       int i) const {
  SPDLOG_DEBUG("Writing topology results...");
  const std::string file = outputPrefix_ + '_' + std::to_string(i) + '_' +
                           region.volume->type() + ".top";
  std::ofstream outFile(file, std::ios::out);
  if (outFile.is_open()) {
    outFile << '#' << proteinFile_ << ' ' << i << '\n';
    outFile << '#' << region.details() << '\n';
    /* TODO add options writing to this file...*/
    std::for_each(data.begin(), data.end(),
                  [&outFile](const auto& line) { outFile << line << '\n'; });
    outFile << std::flush;
  } else {
    SPDLOG_ERROR("Could not open file {}", file);
    throw cpet::io_error("Could not open file " + file);
  }
}
}  // namespace cpet

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
#include "Constants.h"

namespace cpet {

Calculator::Calculator(std::string proteinFile, const std::string& optionFile,
                       std::string chargesFile, int nThreads)
    : proteinFile_(std::move(proteinFile)),
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
  std::for_each(option_.calculateEFieldTopology().begin(),
                option_.calculateEFieldTopology().end(),
                [&](const auto& region) {
                  region.computeTopologyWith(systems_, numberOfThreads_);
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
      option_.calculateEFieldVolumes().end(),
      [this](const auto& volume) { volume.computeVolumeWith(systems_); });
}

std::vector<double> Calculator::loadChargesFile_() const {
  SPDLOG_DEBUG("Loading charges from external file {} ...", chargeFile_);
  std::vector<double> realCharges;
  if (util::endswith(chargeFile_, ".pqr")) {
    util::forEachLineIn(chargeFile_, [&realCharges](const std::string& line) {
      if (util::startswith(line, "ATOM") || util::startswith(line, "HETATM")) {
        const auto tokens = util::split(line, ' ');
        if (tokens.size() <= constants::PQR_CHARGE_INDEX) {
          SPDLOG_ERROR("Error reading in charges file");
          throw cpet::value_error("pqr line too short: " + line);
        }
        realCharges.emplace_back(
            std::stod(tokens[constants::PQR_CHARGE_INDEX]));
      }
    });
  } else {
    /* We assume we have a PDB file */
    util::forEachLineIn(chargeFile_, [&realCharges](const std::string& line) {
      if (util::startswith(line, "ATOM") || util::startswith(line, "HETATM")) {
        realCharges.emplace_back(std::stod(line.substr(
            constants::PDB_CHARGE_START, constants::PDB_CHARGE_WIDTH)));
      }
    });
  }
  return realCharges;
}

void Calculator::fixCharges_() {
  SPDLOG_DEBUG("Fixing charges in structure file with real charges...");
  auto realCharges = loadChargesFile_();
  std::for_each(
      frameTrajectory_.begin(), frameTrajectory_.end(),
      [&realCharges](auto& frame) { frame.updateCharges(realCharges); });
}
}  // namespace cpet

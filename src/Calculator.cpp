/* C++ STL HEADER FILES */
#include <fstream>

/* EXTERNAL LIBRARY HEADER FILES */
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"

/* CPET HEADER FILES */
#include "Calculator.h"
#include "Exceptions.h"
#include "Instrumentation.h"
#include "System.h"
#include "Utilities.h"

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
    System sys(pointChargeTrajectory_[i], option_);
    sys.transformToUserSpace();

    for (const auto& region : option_.calculateEFieldTopology) {
      SPDLOG_INFO("======[Sampling topology]======");
      SPDLOG_INFO("[Volume ] ==>> {}", region.volume->description());
      SPDLOG_INFO("[Npoints] ==>> {}", region.numberOfSamples);
      SPDLOG_INFO("[Threads] ==>> {}", numberOfThreads_);
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
    SPDLOG_INFO("=~=~=~=~[Field at {}]=~=~=~=~", point);
    std::vector<Eigen::Vector3d> fieldTrajectoryAtPoint;

    for (size_t i = 0; i < systems_.size(); i++) {
      Eigen::Vector3d location;
      if (point.find(':') != std::string::npos) {
        location = PointCharge::find(pointChargeTrajectory_[i], AtomID(point))
                       ->coordinate;
      } else {
        auto point_split = split(point, ',');
        location = {std::stod(point_split[0]), std::stod(point_split[1]),
                    std::stod(point_split[2])};
      }
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
      std::vector<Eigen::Vector3d> tmpSystemResults;
      tmpSystemResults.reserve(volume.points.size());

      for (const auto& position : volume.points) {
        tmpSystemResults.push_back(system.electricFieldAt(position));
      }

      volumeResults.push_back(tmpSystemResults);
    }
    writeVolumeResults_(volumeResults, volume);
  }
}

void Calculator::loadPointChargeTrajectory_() {
  std::vector<PointCharge> tmpHolder;
  forEachLineIn(proteinFile_, [this, &tmpHolder](const std::string& line) {
    if (line.rfind("ENDMDL", 0) == 0) {
      pointChargeTrajectory_.push_back(tmpHolder);
      tmpHolder.clear();
    } else if (line.rfind("ATOM", 0) == 0 || line.rfind("HETATM", 0) == 0) {
      tmpHolder.emplace_back(Eigen::Vector3d({std::stod(line.substr(31, 8)),
                                              std::stod(line.substr(39, 8)),
                                              std::stod(line.substr(47, 8))}),
                             std::stod(line.substr(55, 8)),
                             AtomID::generateID(line));
    }
  });
  if (!tmpHolder.empty()) {
    pointChargeTrajectory_.push_back(tmpHolder);
  }
}

std::vector<double> Calculator::loadChargesFile_() const {
  std::vector<double> realCharges;
  forEachLineIn(chargeFile_, [&realCharges](const std::string& line) {
    if (line.rfind("ATOM", 0) == 0 || line.rfind("HETATM", 0) == 0) {
      realCharges.emplace_back(std::stod(line.substr(55, 8)));
    }
  });
  return realCharges;
}

void Calculator::fixCharges_() {
  auto realCharges = loadChargesFile_();

  for (auto& structure : pointChargeTrajectory_) {
    if (structure.size() != realCharges.size()) {
      SPDLOG_ERROR(
          "Inconsistent number of point charges in trajectory and in charge "
          "file");
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
      outFile << '#' << option_.calculateEFieldPoints[i] << '\n';
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

void Calculator::writeVolumeResults_(
    const std::vector<std::vector<Eigen::Vector3d>>& results,
    const EFieldVolume& volume) const {
  std::string file = outputPrefix_ + '_' + volume.name() + ".volume";
  std::ofstream outFile(file, std::ios::out);

  const Eigen::IOFormat fmt(6, Eigen::DontAlignCols, " ", " ", "", "", "", "");
  const Eigen::IOFormat commentFmt(6, 0, " ", "\n", "#", "");


  if (outFile.is_open()) {
    outFile << '#' << proteinFile_ << '\n';
    outFile << '#' << volume.details() << '\n';
    for (size_t i = 0; i < systems_.size(); i++) {
      outFile << "#Frame " << i << '\n';
      outFile << "#Center: " << systems_[i].center().transpose() << '\n';
      outFile << "#Basis Matrix:\n" << systems_[i].basisMatrix().format(commentFmt) << '\n';

      for (size_t j = 0; j < results[i].size(); j++) {
        outFile << volume.points[j].transpose().format(fmt) << ' ' << results[i][j].transpose().format(fmt) << '\n';
      }
    }
    outFile << std::flush;
  } else {
    SPDLOG_ERROR("Could not open file {}", file);
    throw cpet::io_error("Could not open file " + file);
  }
}
// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#include "EFieldVolume.h"

/* C++ STL HEADER FILES */
#include <utility>
#include <fstream>

/* EXTERNAL LIBRARY HEADER FILES */
#include <matplot/matplot.h>

/* CPET HEADER FILES */
#include "System.h"
#include "Box.h"

namespace cpet {

constexpr int DENSITY_PARAMETERS = 3;



EFieldVolume EFieldVolume::fromSimple(const std::vector<std::string>& options) {
  constexpr bool plot = true;
  constexpr size_t MIN_EFIELDVOLUME_OPTIONS = 5;

  if (options.size() < MIN_EFIELDVOLUME_OPTIONS) {
    throw cpet::invalid_option(
        "Invalid Option: plot3d expects at least 5 options");
  }

  const std::vector<std::string> densityOptions{
      options.begin(), options.begin() + DENSITY_PARAMETERS};
  const std::vector<std::string> volumeOptions{
      options.begin() + DENSITY_PARAMETERS, options.end()};

  if (!std::all_of(densityOptions.begin(),
                   densityOptions.begin() + DENSITY_PARAMETERS,
                   util::isDouble)) {
    throw cpet::invalid_option(
        "Invalid Option: plot3d expects density as numerics");
  }

  std::array<int, DENSITY_PARAMETERS> density;
  constexpr auto to_int = [](const std::string& str) -> int {
    return std::stoi(str);
  };
  std::transform(densityOptions.begin(),
                 densityOptions.begin() + DENSITY_PARAMETERS, density.begin(),
                 to_int);

  std::unique_ptr<Volume> vol = Volume::generateVolume(volumeOptions);

  return {std::move(vol), density, plot};
}

EFieldVolume EFieldVolume::fromBlock(const std::vector<std::string>& options) {
  std::unique_ptr<Volume> vol{nullptr};
  std::optional<std::array<int, DENSITY_PARAMETERS>> density;
  bool plot = false;
  std::optional<std::string> output;

  constexpr const char* SHOW_PLOT_KEY = "show";
  constexpr const char* VOLUME_KEY = "volume";
  constexpr const char* DENSITY_KEY = "density";
  constexpr const char* OUTPUT_KEY = "output";

  for (const auto& line : options) {
    const auto tokens = util::split(line, ' ');
    if (tokens.size() < 2) {
      continue;
    }

    const auto key = util::tolower(tokens[0]);
    const std::vector<std::string> key_options{tokens.begin() + 1,
                                               tokens.end()};

    if (key == SHOW_PLOT_KEY) {
      plot = (util::tolower(*key_options.begin()) == "true");
    } else if (key == VOLUME_KEY) {
      vol = Volume::generateVolume(key_options);
    } else if (key == DENSITY_KEY) {
      if (key_options.size() < DENSITY_PARAMETERS) {
        throw cpet::invalid_option("Invalid Option: Density requires 3 ints");
      }

      if (!std::any_of(key_options.begin(),
                       key_options.begin() + DENSITY_PARAMETERS,
                       util::isDouble)) {
        throw cpet::invalid_option(
            "Invalid Option: Density requires 3 ints, received non-numeric "
            "type");
      }
      density = std::array<int, DENSITY_PARAMETERS>{};
      constexpr auto to_int = [](const std::string& str) -> int {
        return std::stoi(str);
      };
      std::transform(key_options.begin(), key_options.end(), (*density).begin(),
                     to_int);
    } else if (key == OUTPUT_KEY) {
      output = *key_options.begin();
    }
  }

  if (!density) {
    throw cpet::invalid_option(
        "Invalid Option: No density specified for 3d plot");
  }
  if (vol == nullptr) {
    throw cpet::invalid_option(
        "Invalid Option: No volume specified for 3d plot");
  }

  return {std::move(vol), *density, plot, output};
}

void EFieldVolume::computeVolumeWith(const std::vector<System>& systems) const {
  std::vector<std::vector<Eigen::Vector3d>> volumeResults;
  volumeResults.reserve(systems.size());
  const auto compute_volume_data = [this](const System& system) {
    system.printCenterAndBasis();
    auto tmpSystemResults = system.computeElectricFieldIn(*this);
    if (showPlot_) {
      plot_(tmpSystemResults);
    }
    return tmpSystemResults;
  };
  std::transform(systems.begin(), systems.end(),
                 std::back_inserter(volumeResults), compute_volume_data);
  if (output_) {
    writeOutput_(systems, volumeResults);
  }

}

void EFieldVolume::plot_(
    const std::vector<Eigen::Vector3d>& electricField) const {
  const auto numberOfPoints = points_.size();
  std::array<std::vector<double>, 3> rotatedPositions;
  std::for_each(rotatedPositions.begin(), rotatedPositions.end(),
                [&numberOfPoints](auto& vec) { vec.reserve(numberOfPoints); });
  std::array<std::vector<double>, 4> rotatedElectricFields;
  std::for_each(rotatedElectricFields.begin(), rotatedElectricFields.end(),
                [&numberOfPoints](auto& vec) { vec.reserve(numberOfPoints); });

  for (size_t index = 0; index < 3; index++) {
    const auto extract_index =
        [&index](const Eigen::Vector3d& vector) -> double {
          return vector[static_cast<long>(index)];
        };
    std::transform(points_.begin(), points_.end(),
                   std::back_inserter(rotatedPositions.at(index)),
                   extract_index);
    std::transform(electricField.begin(), electricField.end(),
                   std::back_inserter(rotatedElectricFields.at(index)),
                   extract_index);
  }

  constexpr auto compute_magnitude =
      [](const Eigen::Vector3d& vector) -> double { return vector.norm(); };
  std::transform(electricField.begin(), electricField.end(),
                 std::back_inserter(*rotatedElectricFields.rbegin()),
                 compute_magnitude);

  constexpr double vectorScale = 0.3;

  matplot::quiver3(rotatedPositions[0], rotatedPositions[1],
                   rotatedPositions[2], rotatedElectricFields[0],
                   rotatedElectricFields[1], rotatedElectricFields[2],
                   rotatedElectricFields[3], vectorScale)
      ->normalize(true)
      .line_width(2);
  matplot::show();
}

void EFieldVolume::writeOutput_(
    const std::vector<System>& systems,
    const std::vector<std::vector<Eigen::Vector3d>>& results) const {
  if (!output_) {
    return;
  }

  const auto file = *output_;
  std::ofstream outFile(file, std::ios::out);

  const Eigen::IOFormat fmt(6, Eigen::DontAlignCols, " ", " ", "", "", "", "");
  const Eigen::IOFormat commentFmt(6, 0, " ", "\n", "#", "");

  if (outFile.is_open()) {
    outFile << '#' << this->details() << '\n';

    for (size_t i = 0; i < systems.size(); i++) {
      outFile << "#Frame " << i << '\n';
      outFile << "#Center: " << systems[i].center().transpose() << '\n';
      outFile << "#Basis Matrix:\n"
              << systems[i].basisMatrix().format(commentFmt) << '\n';

      for (size_t j = 0; j < results[i].size(); j++) {
        outFile << points_[j].transpose().format(fmt) << ' '
                << results[i][j].transpose().format(fmt) << '\n';
      }
    }
    outFile << std::flush;

  } else {
    SPDLOG_ERROR("Could not open file {}", file);
    throw cpet::io_error("Could not open file " + file);
  }
}
}  // namespace cpet
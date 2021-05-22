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

void EFieldVolume::plot(
    const std::vector<Eigen::Vector3d>& electricField) const {
  std::vector<double> x;
  x.reserve(points_.size());
  std::vector<double> y;
  y.reserve(points_.size());
  std::vector<double> z;
  z.reserve(points_.size());
  std::vector<double> ex;
  ex.reserve(electricField.size());
  std::vector<double> ey;
  ey.reserve(electricField.size());
  std::vector<double> ez;
  ez.reserve(electricField.size());
  std::vector<double> m;
  m.reserve(electricField.size());

  for (const auto& p : points_) {
    x.emplace_back(p[0]);
    y.emplace_back(p[1]);
    z.emplace_back(p[2]);
  }
  for (const auto& e : electricField) {
    ex.emplace_back(e[0]);
    ey.emplace_back(e[1]);
    ez.emplace_back(e[2]);
    m.emplace_back(sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]));
  }
  matplot::quiver3(x, y, z, ex, ey, ez, m, 0.3)->normalize(true).line_width(2);
  matplot::show();
}

void EFieldVolume::writeVolumeResults(
    const std::vector<System>& systems,
    const std::vector<std::vector<Eigen::Vector3d>>& results) const {
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

EFieldVolume EFieldVolume::fromSimple(const std::vector<std::string>& options) {
  constexpr bool plot = true;
  constexpr size_t MIN_EFIELDVOLUME_OPTIONS = 5;

  if (options.size() < MIN_EFIELDVOLUME_OPTIONS) {
    throw cpet::invalid_option(
        "Invalid Option: plot3d expects at least 5 options");
  }
  std::unique_ptr<Volume> vol;
  std::array<int, 3> density;

  for (size_t i = 0; i < 3; i++) {
    if (!isDouble(options[i])) {
      throw cpet::invalid_option(
          "Invalid Option: plot3d expects density as numerics");
    }
  }
  density = {std::stoi(options[0]), std::stoi(options[1]),
             std::stoi(options[2])};

  vol = Volume::generateVolume(
      std::vector<std::string>{options.begin() + 3, options.end()});
  return {std::move(vol), density, plot};
}

EFieldVolume EFieldVolume::fromBlock(const std::vector<std::string>& options) {
  std::unique_ptr<Volume> vol;
  std::array<int, 3> density;
  bool plot = false;
  std::optional<std::string> output;

  constexpr const char* SHOW_PLOT_KEY = "show";
  constexpr const char* VOLUME_KEY = "volume";
  constexpr const char* DENSITY_KEY = "density";
  constexpr const char* OUTPUT_KEY = "output";

  for (const auto& line : options) {
    auto tokens = split(line, ' ');
    if (tokens.size() < 2) {
      continue;
    }
    if (tokens[0] == SHOW_PLOT_KEY) {
      plot = (tokens[1] == "true");
    } else if (tokens[0] == VOLUME_KEY) {
      vol = Volume::generateVolume(
          std::vector<std::string>{tokens.begin() + 1, tokens.end()});
    } else if (tokens[0] == DENSITY_KEY) {
      if (tokens.size() < 4) {
        throw cpet::invalid_option("Invalid Option: Density requires 3 ints");
      }
      for (size_t i = 0; i < 3; i++) {
        if (!isDouble(tokens[1 + i])) {
          throw cpet::invalid_option(
              "Invalid Option: Density requires 3 ints, received non-numeric "
              "type");
        }
      }
      density = {std::stoi(tokens[1]), std::stoi(tokens[2]),
                 std::stoi(tokens[3])};
    } else if (tokens[0] == OUTPUT_KEY) {
      output = tokens[1];
    }
  }

  return {std::move(vol), density, plot, output};
}

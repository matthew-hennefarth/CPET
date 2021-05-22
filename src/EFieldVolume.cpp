#include "EFieldVolume.h"

#include <utility>
#include <fstream>

#include <matplot/matplot.h>

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
  if (options.size() < 7) {
    throw cpet::invalid_option(
        "Invalid Option: plot3d expects at least 7 options");
  }
  std::unique_ptr<Volume> vol;
  std::array<int, 3> density;

  if (options[0] == "box") {
    const std::array<double, 3> dims = {
        std::stod(options[1]), std::stod(options[2]), std::stod(options[3])};
    vol = std::make_unique<Box>(dims);
    density = {std::stoi(options[4]), std::stoi(options[5]),
               std::stoi(options[6])};
  } else {
    throw cpet::invalid_option(
        "Invalid Option: Unknown volume specified for plot3d");
  }
  return {std::move(vol), density, plot};
}

EFieldVolume EFieldVolume::fromBlock(const std::vector<std::string>& options) {}

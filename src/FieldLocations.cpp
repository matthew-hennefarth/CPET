// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "FieldLocations.h"

/* EXTERNAL LIBRARY HEADER FILES */
#include <Eigen/Dense>
#include <matplot/matplot.h>

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

    const auto key = util::tolower(tokens[0]);
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
    const std::vector<System>& systems) const {

  std::vector<std::vector<Eigen::Vector3d>> results;
  for (const auto& point : locations_) {
    SPDLOG_INFO("=~=~=~=~[Field at {}]=~=~=~=~", point.ID());
    std::vector<Eigen::Vector3d> fieldTrajectoryAtPoint;

    for (size_t i = 0; i < systems.size(); i++) {
      Eigen::Vector3d location;

      if (point.position()) {
        location = *(point.position());
      } else {
        location = systems[i].frame().find(point)->coordinate;
      }

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
    plot_(results);
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
void FieldLocations::plot_(
    const std::vector<std::vector<Eigen::Vector3d>>& results) const {
  const auto numberOfPlots =
      util::countSetBits(static_cast<unsigned int>(plotStyle_));
  if (numberOfPlots <= 0 || numberOfPlots > 4) {
    SPDLOG_WARN(
        "Number of plots less than 1 or greater than 4! Error in logic");
    return;
  }

  auto figure = matplot::figure();
  if (numberOfPlots < 3) {
    figure->tiledlayout(numberOfPlots, 1);
  } else {
    figure->tiledlayout(2, 2);
  }

  /* can make this an option eventually
   figure->size(500,500); */

  auto current_ax = matplot::nexttile(0);

  for (const auto& data : results) {
    std::array<std::vector<double>, 4> rotatedElectricFields;
    for (size_t index = 0; index < 3; index++) {
      const auto extract_index =
          [&index](const Eigen::Vector3d& vector) -> double {
        return vector[static_cast<long>(index)];
      };

      std::transform(data.begin(), data.end(),
                     std::back_inserter(rotatedElectricFields.at(index)),
                     extract_index);
    }

    constexpr auto compute_magnitude =
        [](const Eigen::Vector3d& vector) -> double { return vector.norm(); };

    std::transform(data.begin(), data.end(),
                   std::back_inserter(*rotatedElectricFields.rbegin()),
                   compute_magnitude);

    size_t plot_index = 0;

    // constexpr std::array<const char*, 4> titles{"X", "Y", "Z", "Magnitude"};
    const auto plot = [&rotatedElectricFields, &plot_index, &current_ax,
                       &figure](size_t index) {
      constexpr std::array<const char*, 4> titles{"X", "Y", "Z", "Magnitude"};
      current_ax = figure->nexttile(plot_index);
      matplot::hold(current_ax, matplot::on);
      matplot::plot(current_ax, rotatedElectricFields.at(index));
      current_ax->xlabel("Frame");
      current_ax->ylabel("Magnitude (V/Ang)");
      current_ax->title(titles.at(index));
      ++plot_index;
    };

    if (plotX_()) {
      plot(0);
    }
    if (plotY_()) {
      plot(1);
    }
    if (plotZ_()) {
      plot(2);
    }
    if (plotM_()) {
      plot(3);
    }
  }

  std::vector<std::string> legend_list;
  constexpr auto get_string_representation = [](const AtomID& aid) {
    return aid.ID();
  };
  std::transform(locations_.begin(), locations_.end(),
                 std::back_inserter(legend_list), get_string_representation);

  current_ax->legend(legend_list);
  matplot::show();
}

}  // namespace cpet
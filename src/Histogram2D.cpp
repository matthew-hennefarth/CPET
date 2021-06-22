// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <spdlog/spdlog.h>

#include "Histogram2D.h"

namespace cpet::histo {
std::vector<std::vector<int>> construct2DHistogram(
    const std::vector<double>& x, const std::vector<double>& y,
    const std::array<int, 2>& bins, const std::array<double, 2>& xlim,
    const std::array<double, 2>& ylim) noexcept {
  std::vector<std::vector<int>> result(
      static_cast<size_t>(bins[1]),
      std::vector<int>(static_cast<size_t>(bins[0]), 0));

  auto xEdges = constructEdges(xlim[0], xlim[1], bins[0]);
  auto yEdges = constructEdges(ylim[0], ylim[1], bins[0]);

  const auto numberOfElements = std::min(x.size(), y.size());

  const auto notInXLimit = [&](const double& value) {
    return value < xlim[0] || value > xlim[1];
  };
  const auto notInYLimit = [&](const double& value) {
    return value < ylim[0] || value > ylim[1];
  };

  for (size_t i = 0; i < numberOfElements; i++) {
    const double x_value = x[i];
    const double y_value = y[i];

    if (notInXLimit(x_value) || notInYLimit(y_value)) {
      continue;
    }

    const auto xEdge =
        std::find_if(xEdges.begin(), xEdges.end(),
                     [&](const auto& edge) { return x_value <= edge; });
    assert(xEdge != xEdges.end());
    const auto yEdge =
        std::find_if(yEdges.begin(), yEdges.end(),
                     [&](const auto& edge) { return y_value <= edge; });
    assert(yEdge != yEdges.end());

    const auto xIndex = static_cast<size_t>(xEdge - xEdges.begin());
    const auto yIndex = static_cast<size_t>(yEdge - yEdges.begin());
    ++result[yIndex][xIndex];
  }

  return result;
}

}  // namespace cpet::histo
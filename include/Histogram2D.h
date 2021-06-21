// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#ifndef HISTOGRAM2D_H
#define HISTOGRAM2D_H

#include <array>
#include <vector>
#include <cassert>

[[nodiscard]] std::vector<std::vector<int>> construct2DHistogram(
    const std::vector<double>& x, const std::vector<double>& y,
    const std::array<int, 2>& bins, const std::array<double, 2>& xlim,
    const std::array<double, 2>& ylim) noexcept;

[[nodiscard]] inline std::vector<double> constructEdges(const double min,
                                                        const double max,
                                                        const int bins) {
  std::vector<double> edges;
  if (bins <= 0 || max < min) {
    return edges;
  }

  edges.reserve(bins);

  const double bin_width =
      static_cast<double>(max - min) / static_cast<double>(bins);
  double current_edge = min;
  while ((current_edge += bin_width) <= max) {
    edges.emplace_back(current_edge);
  }
  return edges;
}

#endif  // HISTOGRAM2D_H

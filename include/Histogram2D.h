// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#ifndef HISTOGRAM2D_H
#define HISTOGRAM2D_H

#include <array>
#include <vector>
#include <cassert>
#include <algorithm>
#include <numeric>

namespace cpet::histo {

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

  edges.reserve(static_cast<size_t>(bins));

  const double bin_width = (max - min) / static_cast<double>(bins);
  double current_edge = min;

  while ((current_edge += bin_width) <= max) {
    edges.emplace_back(current_edge);
  }
  if (edges.size() < bins) {
    edges.emplace_back(max);
  }
  return edges;
}

[[nodiscard]] inline std::vector<double> normalize(
    const std::vector<int>& histogram) {
  const double sum = std::accumulate(histogram.begin(), histogram.end(), 0.0);
  std::vector<double> result;
  result.reserve(histogram.size());
  std::transform(
      histogram.begin(), histogram.end(), std::back_inserter(result),
      [&](const auto& element) { return static_cast<double>(element) / sum; });

  return result;
}
}  // namespace cpet::histo
#endif  // HISTOGRAM2D_H

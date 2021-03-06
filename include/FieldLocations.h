// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef FIELDLOCATIONS_H
#define FIELDLOCATIONS_H

/* C++ STL HEADER FILES */
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>
#include <utility>

/* CPET HEADER FILES */
#include "AtomID.h"
#include "Exceptions.h"
#include "Utilities.h"
#include "PointCharge.h"

namespace cpet {

/* Prevents circular inclusion */
class System;

/* We use bitmap to determine what to plot */
enum class PlotStyles : uint32_t {
  x = 1 << 0,
  y = 1 << 1,
  z = 1 << 2,
  m = 1 << 3
};

constexpr PlotStyles operator|(const PlotStyles lhs, const PlotStyles rhs) {
  return static_cast<PlotStyles>(
      static_cast<std::underlying_type<PlotStyles>::type>(lhs) |
      static_cast<std::underlying_type<PlotStyles>::type>(rhs));
}
constexpr PlotStyles operator&(const PlotStyles lhs, const PlotStyles rhs) {
  return static_cast<PlotStyles>(
      static_cast<std::underlying_type<PlotStyles>::type>(lhs) &
      static_cast<std::underlying_type<PlotStyles>::type>(rhs));
}

constexpr PlotStyles& operator|=(PlotStyles& lhs, const PlotStyles rhs) {
  lhs = static_cast<PlotStyles>(
      static_cast<std::underlying_type<PlotStyles>::type>(lhs) |
      static_cast<std::underlying_type<PlotStyles>::type>(rhs));
  return lhs;
}

class FieldLocations {
 public:
  void computeEFieldsWith(const std::vector<System>& systems) const;

  [[nodiscard]] constexpr const std::vector<AtomID>& locations()
      const noexcept {
    return locations_;
  }

  [[nodiscard]] inline const PlotStyles& plotStyle() const noexcept {
    return plotStyle_;
  }
  [[maybe_unused]] void plotStyle(const std::vector<std::string>& tokens) {
    plotStyle_ = decodePlotStyle_(tokens);
  }

  [[nodiscard]] constexpr const std::optional<std::string>& output()
      const noexcept {
    return output_;
  }

  template <typename S1>
  void output(S1&& output_name) {
    if (!output_name.empty()) {
      output_ = std::forward<S1>(output_name);
    }
  }

  [[nodiscard]] bool showPlots() const noexcept {
    // Check if user specified any plots
    return util::countSetBits(static_cast<unsigned int>(plotStyle_)) != 0;
  }

  [[nodiscard]] static FieldLocations fromSimple(
      const std::vector<std::string>& options);

  [[nodiscard]] static FieldLocations fromBlock(
      const std::vector<std::string>& options);

 private:
  std::vector<AtomID> locations_;
  PlotStyles plotStyle_{0};
  std::optional<std::string> output_{std::nullopt};

  [[nodiscard]] inline static PlotStyles decodePlotStyle_(
      const std::vector<std::string>& tokens) {
    static const std::unordered_map<std::string, PlotStyles> plotHash = {
        {"x", PlotStyles::x},
        {"y", PlotStyles::y},
        {"z", PlotStyles::z},
        {"m", PlotStyles::m},
        {"all", PlotStyles::x | PlotStyles::y | PlotStyles::z | PlotStyles::m}};
    PlotStyles plotstyle{0};
    for (const auto& token : tokens) {
      const auto iter = plotHash.find(util::tolower(token));
      if (iter == plotHash.end()) {
        // Unknown token passed
        throw cpet::invalid_option(
            "Invalid Option: Unknown plot token specified: " + token);
      }
      plotstyle |= iter->second;
    }
    return plotstyle;
  }

  [[nodiscard]] constexpr bool plotX_() const noexcept {
    return ((plotStyle_ & PlotStyles::x) == PlotStyles::x);
  }
  [[nodiscard]] constexpr bool plotY_() const noexcept {
    return ((plotStyle_ & PlotStyles::y) == PlotStyles::y);
  }
  [[nodiscard]] constexpr bool plotZ_() const noexcept {
    return ((plotStyle_ & PlotStyles::z) == PlotStyles::z);
  }
  [[nodiscard]] constexpr bool plotM_() const noexcept {
    return ((plotStyle_ & PlotStyles::m) == PlotStyles::m);
  }

  void writeOutput_(
      const std::vector<std::vector<Eigen::Vector3d>>& results) const;

  void plot_(const std::vector<std::vector<Eigen::Vector3d>>& results) const;
};
}  // namespace cpet

#endif  // FIELDLOCATIONS_H

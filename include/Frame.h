// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef FRAME_H
#define FRAME_H

/* C++ STL HEADER FILES */
#include <vector>
#include <algorithm>

/* EXTERNAL LIBRARY HEADER FILES */
#include <spdlog/spdlog.h>

/* CPET HEADER FILES */
#include "PointCharge.h"
#include "Utilities.h"

namespace cpet{

class Frame {
 public:
  Frame(const std::vector<PointCharge>& pcs) : pointCharges_(pcs) {}

  [[nodiscard]] inline std::vector<PointCharge>::const_iterator find(const AtomID& id) const {
    return util::find_if_ex(pointCharges_.begin(), pointCharges_.end(),
                            [&id](const auto& pc) { return pc.id == id; });
  }

  [[nodiscard]] inline std::vector<PointCharge>::const_iterator begin() const noexcept {
    return pointCharges_.begin();
  }

  [[nodiscard]] inline std::vector<PointCharge>::const_iterator end() const noexcept {
    return pointCharges_.end();
  }

  [[nodiscard]] inline std::vector<PointCharge>::iterator begin() noexcept {
    return pointCharges_.begin();
  }

  [[nodiscard]] inline std::vector<PointCharge>::iterator end() noexcept {
    return pointCharges_.end();
  }

  [[nodiscard]] inline std::vector<PointCharge>::const_reverse_iterator rbegin() const noexcept {
    return pointCharges_.rbegin();
  }

  [[nodiscard]] inline std::vector<PointCharge>::const_reverse_iterator rend() const noexcept {
    return pointCharges_.rend();
  }

  [[nodiscard]] inline std::vector<PointCharge>::reverse_iterator rbegin() noexcept {
    return pointCharges_.rbegin();
  }

  [[nodiscard]] inline std::vector<PointCharge>::reverse_iterator rend() noexcept {
    return pointCharges_.rend();
  }

  inline void updateCharges(const std::vector<double>& charges) {
    if (pointCharges_.size() != charges.size()) {
      SPDLOG_ERROR("Structure size: {}, number of charges: {}",
                   pointCharges_.size(), charges.size());
      throw cpet::value_error(
          "Inconsistent number of point charges in trajectory and in charge "
          "file");
      }
    for (size_t i = 0; i < pointCharges_.size(); i++) {
      pointCharges_[i].charge = charges[i];
    }
  }

 private:
  std::vector<PointCharge> pointCharges_;
};

}
#endif  // FRAME_H

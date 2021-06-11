// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ATOMID_H
#define ATOMID_H

/* C++ STL HEADER FILES */
#include <algorithm>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <sstream>

#include <Eigen/Dense>

/* CPET HEADER FILES */
#include "Exceptions.h"
#include "Utilities.h"

namespace cpet {

constexpr int MIN_PDB_LINE_LENGTH = 26;
constexpr int PDB_CHAIN_START = 21;
constexpr int PDB_CHAIN_WIDTH = 2;
constexpr int PDB_RESNUM_START = 22;
constexpr int PDB_RESNUM_WIDTH = 4;
constexpr int PDB_ATOMID_START = 12;
constexpr int PDB_ATOMID_WIDTH = 4;

class AtomID {
 public:
  enum class Constants { origin, e1, e2 };

  explicit inline AtomID(const Constants other_id)
      : isConstant_(true), id_(decodeConstant_(other_id)) {
    if (!validID()) {
      throw cpet::value_error("Invalid atom id: " + id_);
    }
  }

  template<typename S1, typename = std::enable_if_t<std::is_convertible_v<S1, std::string>>>
  explicit inline AtomID(S1&& other_id) : id_(std::forward<S1>(other_id)) {
    if (!validID()) {
      throw cpet::value_error("Invalid atom ID: " + id_);
    }
  }

  explicit inline AtomID(AtomID&&) = default;

  inline AtomID(const AtomID&) = default;
  inline ~AtomID() = default;
  inline AtomID& operator=(const AtomID&) = default;
  inline AtomID& operator=(AtomID&&) = default;

  template<typename S1, typename = std::enable_if_t<std::is_convertible_v<S1, std::string>>>
  inline AtomID& operator=(S1&& rhs) {
    setID(std::forward<S1>(rhs));
    isConstant_ = false;
    return *this;
  }

  inline AtomID& operator=(AtomID::Constants c) {
    id_ = decodeConstant_(c);
    if (!validID()) {
      throw cpet::value_error("Invalid AtomID specification: " + id_);
    }
    isConstant_ = true;
    return *this;
  }

  [[nodiscard]] inline bool validID() const noexcept { return validID(id_); }

  [[nodiscard]] static inline bool validID(std::string_view atomid) noexcept {
    auto splitID = util::split(atomid, ':');

    return splitID.size() == 3 &&
           (isVector(atomid) || util::isDouble(splitID[1]));
  }

  [[nodiscard]] inline bool operator==(const std::string_view rhs) const noexcept {
    return (id_ == rhs);
  }

  [[nodiscard]] inline bool operator==(Constants c) const noexcept {
    return (id_ == decodeConstant_(c));
  }

  [[nodiscard]] inline bool operator==(const AtomID& rhs) const noexcept {
    return (id_ == rhs.id_);
  }

  [[nodiscard]] inline bool operator!=(const std::string_view rhs) const noexcept {
    return !(*this == rhs);
  }

  [[nodiscard]] inline bool operator!=(Constants c) const noexcept {
    return !(*this == c);
  }

  [[nodiscard]] inline const std::string& ID() const noexcept { return id_; }

  template<typename S1>
  inline void setID(S1&& newID) {
    if (validID(std::forward<S1>(newID))) {
      id_ = std::forward<S1>(newID);
      position_.reset();
    } else {
      throw cpet::value_error("Invalid AtomID " + std::string(newID));
    }
  }

  [[nodiscard]] static inline AtomID generateID(const std::string_view pdbLine) {
    if (pdbLine.size() < MIN_PDB_LINE_LENGTH) {
      throw cpet::value_error("pdb line to short: " + static_cast<std::string>(pdbLine));
    }

    std::stringstream result_stream;
    result_stream << pdbLine.substr(PDB_CHAIN_START, PDB_CHAIN_WIDTH) << ':' << pdbLine.substr(PDB_RESNUM_START, PDB_RESNUM_WIDTH) << ':' << pdbLine.substr(PDB_ATOMID_START, PDB_ATOMID_WIDTH);
    auto result = result_stream.str();
    result.erase(remove(begin(result), end(result), ' '), end(result));
    return AtomID(result);
  }

  [[nodiscard]] inline std::optional<Eigen::Vector3d> position()
      const noexcept {
    if (!position_) {
      auto splitID = util::split(id_, ':');

      if (splitID.size() != 3) {
        position_.reset();
      } else {
        if (isVector()) {
          position_.emplace(std::stod(splitID[0]), std::stod(splitID[1]),
                            std::stod(splitID[2]));
        } else {
          position_.reset();
        }
      }
    }
    return position_;
  }

  [[nodiscard]] inline bool isConstant() const noexcept { return isConstant_; }

  [[nodiscard]] inline bool isVector() const noexcept { return isVector(id_); }

  [[nodiscard]] static inline bool isVector(std::string_view atomid) noexcept {
    auto splitID = util::split(atomid, ':');
    if (splitID.size() != 3) {
      return false;
    }

    return std::all_of(
        splitID.begin(), splitID.end(),
        [](const std::string& str) { return util::isDouble(str); });
  }

 private:
  bool isConstant_ = false;
  std::string id_;
  mutable std::optional<Eigen::Vector3d> position_;

  [[nodiscard]] static inline std::string decodeConstant_(Constants c) {
    switch (c) {
      case AtomID::Constants::origin:
        return "0:0:0";
      case AtomID::Constants::e1:
        return "1:0:0";
      case AtomID::Constants::e2:
        return "0:1:0";
      default:
        throw cpet::value_error("Cannot decode constant");
    }
  }
};
}  // namespace cpet
#endif  // ATOMID_H

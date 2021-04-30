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

#include "Eigen/Dense"

/* CPET HEADER FILES */
#include "Exceptions.h"
#include "Utilities.h"

class AtomID {
 public:
  enum class Constants { origin, e1, e2 };

  explicit inline AtomID(Constants other_id)
      : id(decodeConstant_(other_id)), isConstant_(true) {
    if (!validID()) {
      throw cpet::value_error("Invalid atom id: " + id);
    }
  }

  explicit inline AtomID(std::string other_id) : id(std::move(other_id)) {
    if (!validID()) {
      throw cpet::value_error("Invalid atom ID: " + id);
    }
  }

  inline AtomID(const AtomID&) = default;

  inline AtomID(AtomID&&) = default;

  inline AtomID& operator=(const AtomID&) = default;

  inline AtomID& operator=(AtomID&&) = default;

  inline AtomID& operator=(const std::string& rhs) {
    setID(rhs);
    return *this;
  }

  inline AtomID& operator=(AtomID::Constants c) {
    id = decodeConstant_(c);
    if (!validID()) {
      throw cpet::value_error("Invalid AtomID specification: " + id);
    }
    isConstant_ = true;
    return *this;
  }

  [[nodiscard]] inline bool validID() const noexcept { return validID(id); }

  [[nodiscard]] static inline bool validID(std::string_view atomid) noexcept {
    auto splitID = split(atomid, ':');

    if (splitID.size() != 3) {
      return false;
    }

    for (const auto& arg : splitID) {
      if (!isDouble(arg)) {
        goto invalid_arg;
      }
    }

    return true;
  invalid_arg:
    if (isDouble(splitID[1])) return true;

    return false;
  }

  [[nodiscard]] inline bool operator==(const std::string& rhs) const noexcept {
    return (id == rhs);
  }

  [[nodiscard]] inline bool operator==(Constants c) const noexcept {
    return (id == decodeConstant_(c));
  }

  [[nodiscard]] inline bool operator==(const AtomID& rhs) const noexcept {
    return (id == rhs.id);
  }

  [[nodiscard]] inline bool operator!=(const std::string& rhs) const noexcept {
    return !(*this == rhs);
  }

  [[nodiscard]] inline bool operator!=(Constants c) const noexcept {
    return !(*this == c);
  }

  [[nodiscard]] inline std::string& getID() noexcept { return id; }

  [[nodiscard]] inline const std::string& getID() const noexcept { return id; }

  inline void setID(const std::string& newID) {
    if (validID(newID)) {
      id = newID;
    } else {
      throw cpet::value_error("Invalid AtomID: " + newID);
    }
  }

  [[nodiscard]] static inline AtomID generateID(const std::string& pdbLine) {
    if (pdbLine.size() < 26) {
      throw cpet::value_error("Invalid pdb line: " + pdbLine);
    }
    std::string result = pdbLine.substr(21, 2) + ":" + pdbLine.substr(22, 4) +
                         ":" + pdbLine.substr(12, 4);

    result.erase(remove(begin(result), end(result), ' '), end(result));
    return AtomID(result);
  }

  [[nodiscard]] inline std::optional<Eigen::Vector3d> position()
      const noexcept {
    if (!position_) {
      auto splitID = split(id, ':');

      if (splitID.size() != 3) {
        position_.reset();
      } else {
        try {
          position_.emplace(std::stod(splitID[0]), std::stod(splitID[1]),
                            std::stod(splitID[2]));
        } catch (const std::invalid_argument&) {
          position_.reset();
        }
      }
    }
    return position_;
  }

  [[nodiscard]] inline bool isConstant() const noexcept { return isConstant_; }

  std::string id;

 private:
  bool isConstant_ = false;
  mutable std::optional<Eigen::Vector3d> position_;

  [[nodiscard]] static inline std::string decodeConstant_(Constants c) {
    switch (c) {
      case AtomID::Constants::origin:
        return "0:0:0";
      case AtomID::Constants::e1:
        return "1:0:0";
      case AtomID::Constants::e2:
        return "0:1:0";
    }
  }
};

#endif  // ATOMID_H

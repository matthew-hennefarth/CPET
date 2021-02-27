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
      : id(decodeConstant_(other_id)) {
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
    return *this;
  }

  [[nodiscard]] inline bool validID() const noexcept { return validID(id); }

  [[nodiscard]] inline bool validID(std::string_view atomid) const noexcept {
    auto splitID = split(atomid, ':');

    if (splitID.size() != 3) {
      return false;
    }

    try {
      position_.emplace(std::stod(splitID[0]), std::stod(splitID[1]),
                        std::stod(splitID[2]));
    } catch (const std::invalid_argument&) {
      position_.reset();
      try {
        stoi(splitID[1]);
      } catch (const std::invalid_argument&) {
        return false;
      }
    }
    return true;
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
    return position_;
  }

  std::string id;

 private:
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

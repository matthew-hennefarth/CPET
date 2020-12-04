#ifndef ATOMID_H
#define ATOMID_H

/* C++ STL HEADER FILES */
#include <string>
#include <utility>
#include <type_traits>
#include <algorithm>

/* CPET HEADER FILES */
#include "Utilities.h"
#include "Exceptions.h"

class AtomID{
    public:

        enum class Constants {
            origin,
            e1,
            e2
        };

        explicit inline AtomID(Constants other_id) noexcept : id(decodeConstant_(other_id)){}

        explicit inline AtomID(std::string other_id) : id(std::move(other_id)){
            if (!validID()){
                throw cpet::value_error("Invalid atom ID: " + id);
            }
        }

        inline AtomID(const AtomID&) = default;

        inline AtomID(AtomID&&) = default;

        inline AtomID& operator=(const AtomID&) = default;

        inline AtomID& operator=(AtomID&&) = default;

        inline AtomID& operator=(const std::string& rhs){
            setID(rhs);
            return *this;
        }

        inline AtomID& operator=(AtomID::Constants c) {
            id = decodeConstant_(c);
            return *this;
        }

        [[nodiscard]] inline bool validID() const noexcept{
            return validID(id);
        }

        [[nodiscard]] static inline bool validID(std::string_view id) noexcept {
            auto splitID = split(id, ':');

            if (splitID.size() != 3){
                return false;
            }

            try{
                stoi(splitID[1]);
            }
            catch (const std::invalid_argument&) {
                return false;
            }

            return true;
        }

        [[nodiscard]] inline bool operator==(const std::string& rhs) const noexcept {return (id == rhs);}

        [[nodiscard]] inline bool operator==(Constants c) const noexcept {return (id == decodeConstant_(c));}

        [[nodiscard]] inline bool operator==(const AtomID& rhs) const noexcept {return (id == rhs.id);}

        [[nodiscard]] inline bool operator!=(const std::string& rhs) const noexcept {return !(*this == rhs);}

        [[nodiscard]] inline bool operator!=(Constants c) const noexcept {return !(*this == c);}

        [[nodiscard]] inline std::string& getID() noexcept {return id;}

        [[nodiscard]] inline const std::string& getID() const noexcept {return id;}

        inline void setID(const std::string& newID) {
            if(validID(newID)){
                id = newID;
            }
            else{
                throw cpet::value_error("Invalid AtomID: " + newID);
            }
        }

        [[nodiscard]] static inline AtomID generateID(const std::string& pdbLine) {
            if (pdbLine.size() < 26){
                throw cpet::value_error("Invalid pdb line: " + pdbLine);
            }
            std::string result =  pdbLine.substr(21,2) + ":"
                                  + pdbLine.substr(22,4) + ":"
                                  + pdbLine.substr(12,4);

            result.erase(remove(begin(result), end(result), ' '), end(result));
            return AtomID(result);
        }

        std::string id;

    private:
        [[nodiscard]] static inline std::string decodeConstant_(Constants c) {
            switch (c) {
                case AtomID::Constants::origin:
                    return "-1:-1:-1";
                case AtomID::Constants::e1:
                    return "-1:-1:-2";
                case AtomID::Constants::e2:
                    return "-1:-1:-3";
            }
        }
};

#endif //ATOMID_H
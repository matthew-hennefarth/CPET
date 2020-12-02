//
// Created by Matthew Hennefarth on 12/1/20.
//

#ifndef ATOMID_H
#define ATOMID_H

#include <string>
#include <stdexcept>

#include "Utilities.h"

class AtomID{
    public:

        const static std::string origin;

        const static std::string e1;

        const static std::string e2;

        AtomID() noexcept = default;

        explicit AtomID(std::string id) : id(std::move(id)){
            if (!validID()){
                throw std::exception();
            }
        }

        [[nodiscard]] inline bool validID() const noexcept{
            return validID(id);
        }

        [[nodiscard]] static inline bool validID(const std::string& id) noexcept {
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

        AtomID& operator=(std::string newID) {
            setID(std::move(newID));
            return *this;
        }

        [[nodiscard]] inline bool operator==(const std::string& rhs) const noexcept {return (id == rhs);}

        [[nodiscard]] inline bool operator==(const AtomID& rhs) const noexcept {return (id == rhs.id);}

        [[nodiscard]] inline bool operator!=(const std::string& rhs) const noexcept {return !(*this == rhs);}

        [[nodiscard]] inline std::string& getID() noexcept {return id;}

        [[nodiscard]] inline const std::string& getID() const noexcept {return id;}

        inline void setID(std::string newID) {

            if(validID(newID)){
                id = std::move(newID);
            }
            else{
                throw std::exception();
            }

        }

        [[nodiscard]] static AtomID generateID(const std::string& pdbLine);

        std::string id;
};

#endif //ATOMID_H

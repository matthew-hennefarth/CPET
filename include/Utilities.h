//
// Created by Matthew Hennefarth on 11/12/20.
//

#ifndef UTILITIES_H
#define UTILITIES_H

#include <vector>
#include <string>
#include <functional>

void extractFromFile(const std::string_view& file, const std::function<void(const std::string&)>& func);

std::vector<std::string> split(const std::string_view &str, char delim);

template<class T>
void filter(std::vector<T>& list, const T& remove=T()) noexcept(true) {
    for (typename std::vector<T>::size_type i = 0; i < list.size(); i++){
        if(list[i] == remove){
            list.erase(list.begin() + static_cast<long>(i));
            i--;
        }
    }
}

#endif //UTILITIES_H

#ifndef UTILITIES_H
#define UTILITIES_H

/*
 * C++ STL HEADER FILES
 */
#include <vector>
#include <string>
#include <functional>
#include <random>
#include <fstream>

#include "spdlog/spdlog.h"

[[nodiscard]] inline std::unique_ptr<std::mt19937> &randomNumberGenerator() {
    static thread_local std::unique_ptr<std::mt19937> generator = nullptr;
    if (generator == nullptr){
        generator = std::make_unique<std::mt19937>(std::random_device()());
    }
    return generator;
}

void forEachLineIn(const std::string_view& file, const std::function<void(const std::string&)>& func);

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
template<typename T>
void writeToFile(const std::string& file, const std::vector<T>& out){
    std::ofstream outFile(file, std::ios::out);
    if(outFile.is_open()){
        for(const auto& line : out){
            outFile << line << '\n';
        }
        outFile << std::flush;
    }
    else{
        SPDLOG_ERROR("Could not open file {}", file);
    }
}

#endif //UTILITIES_H

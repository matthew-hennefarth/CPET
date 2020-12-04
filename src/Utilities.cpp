
/* C++ STL HEADER FILES */
#if __cplusplus <= 199711L
#include <algorithm>
#else
#include <numeric>
#endif

/* CPET HEADER FILES */
#include "Utilities.h"

void forEachLineIn(const std::string& file, const std::function<void(const std::string&)>& func){
    std::fstream inFile(file);
    std::string line;
    if(inFile.is_open()){
        while(std::getline(inFile, line)){
            func(line);
        }
    }
    else{
        throw cpet::io_error("Could not open file " + file);
    }
}

std::vector<std::string> split(std::string_view str, char delim) {
    std::vector<std::string> result;

    std::string::size_type start = 0;
    for (size_t i = 0; i < str.size(); i++){
        if (str[i] == delim && start != i) {
            result.emplace_back(str.substr(start, i - start));
            start = i + 1;
        }
        else if (str[i] == delim && start == i){
            start++;
        }
    }

    result.emplace_back(str.substr(start, str.size()));

    filter(result);

    return result;
}

//
// Created by Matthew Hennefarth on 11/12/20.
//

#if __cplusplus <= 199711L
#include <algorithm>
#else
#include <numeric>
#endif

#include <fstream>


#include "spdlog/spdlog.h"
#include "Utilities.h"

void extractFromFile(const std::string_view& file, const std::function<void(const std::string&)>& func){
    std::fstream inFile(std::string(file).c_str());
    std::string line;
    if(inFile.is_open()){
        while(std::getline(inFile, line)){
            func(line);
        }
        inFile.close();
    }
    else{
        SPDLOG_ERROR("Could not open file {}", file);
    }
}

std::vector<std::string> split(const std::string_view &str, char delim) {
    std::vector<std::string> result;

    std::string::size_type start = 0;
    for (std::string::size_type i = 0; i < str.size(); i++){
        if (str[i] == delim && start != i) {
            result.emplace_back(str.substr(start, i - start));
            start = i + 1;
        }
        else if (str[i] == delim && start == i){
            start++;
        }
    }

    result.emplace_back(str.substr(start, str.size()));

    filter<std::string>(result, "");

    return result;
}

std::vector<std::vector<size_t>> chunkIndex(const size_t& procs, const size_t& n){
    std::vector<size_t> elementsToCompute(procs,n / procs);
    for(size_t i = 0; i < n%procs; i++){
        elementsToCompute[i]++;
    }

    std::vector<std::vector<size_t>> chunks(procs);
    size_t index = 0;
    for(size_t i = 0 ; i < procs; i++){
        chunks[i].resize(elementsToCompute[i]);
        std::iota(std::begin(chunks[i]), std::end(chunks[i]), index);
        index += elementsToCompute[i];
//        for(size_t j = 0; j < elementsToCompute[i]; j++, index++){
//            chunks[i].push_back(index);
//        }
    }
    return chunks;
}


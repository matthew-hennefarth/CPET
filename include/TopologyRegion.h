#ifndef TOPOLOGYREGION_H
#define TOPOLOGYREGION_H

#include <memory>

#include "Volume.h"

struct TopologyRegion{

    TopologyRegion(std::unique_ptr<Volume> vol, size_t samples) : volume(std::move(vol)), numberOfSamples(samples){}

    std::unique_ptr<Volume> volume;

    size_t numberOfSamples;

    [[nodiscard]] inline std::string details() const noexcept{
        return "Samples: " + std::to_string(numberOfSamples) + "; Volume: " + volume->description();
    }

};

#endif //TOPOLOGYREGION_H

#ifndef TOPOLOGYREGION_H
#define TOPOLOGYREGION_H

/* C++ STL HEADER FILES */
#include <memory>
#include <string>

/* CPET HEADER FILES */
#include "Volume.h"

struct TopologyRegion {
  std::unique_ptr<Volume> volume;

  int numberOfSamples;

  inline TopologyRegion(std::unique_ptr<Volume> vol, int samples) noexcept
      : volume(std::move(vol)), numberOfSamples(samples) {}

  [[nodiscard]] inline std::string details() const noexcept {
    return "Samples: " + std::to_string(numberOfSamples) +
           "; Volume: " + volume->description();
  }
};

#endif  // TOPOLOGYREGION_H

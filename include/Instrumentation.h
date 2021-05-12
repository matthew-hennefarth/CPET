// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef INSTRUMENTATION_H
#define INSTRUMENTATION_H

/* C++ STL HEADER FILES */
#include <chrono>
#include <functional>
#include <memory>
#include <utility>

/* EXTERNAL LIBRARY HEADER FILES */
#include "spdlog/spdlog.h"

class Timer {
 public:
  inline Timer() noexcept { Start(); }

  inline Timer(const Timer&) = delete;
  
  inline Timer(Timer&&) = delete;

  inline Timer& operator=(const Timer&) = delete;

  inline Timer& operator=(Timer&&) = delete;

  explicit inline Timer(std::shared_ptr<spdlog::logger> logger) noexcept
      : logger_(std::move(logger)) {
    Start();
  }

  inline Timer(std::shared_ptr<spdlog::logger> logger,
               std::function<void(const float)> func) noexcept
      : logger_(std::move(logger)), func_(std::move(func)) {
    Start();
  }

  ~Timer() {
    end_ = std::chrono::steady_clock::now();
    duration_ = end_ - start_;

    const float sec = duration_.count();

    if (logger_ == nullptr) {
      SPDLOG_INFO("Timer: {} sec", sec);
    } else {
      logger_->info("Timer: {} sec", sec);
    }
    func_(sec);
  }

  inline void Start() noexcept { start_ = std::chrono::steady_clock::now(); }

 private:
  std::chrono::time_point<std::chrono::steady_clock> start_, end_{};

  std::chrono::duration<float> duration_{};

  std::shared_ptr<spdlog::logger> logger_{nullptr};

  std::function<void(const float)> func_{[](const float){}};
};
#endif  // INSTRUMENTATION_H

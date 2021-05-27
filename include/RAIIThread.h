// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef RAIITHREAD_H
#define RAIITHREAD_H

/* C++ STL HEADER FILES */
#include <thread>
#include <utility>

namespace cpet {
namespace util {

class RAIIThread {
 public:
  RAIIThread() = default;

  template <class Function, class... Args>
  explicit RAIIThread(Function&& F, Args... args) noexcept
      : thread_(F, args...) {}

  RAIIThread(RAIIThread&& other) noexcept : thread_(std::move(other.thread_)) {}

  /* Thread does not have a copy constructor */
  RAIIThread(const RAIIThread& t) = delete;

  ~RAIIThread() {
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  inline void join() { thread_.join(); }

  [[maybe_unused]] inline void detach() { thread_.detach(); }

  [[maybe_unused]] [[nodiscard]] inline bool joinable() const noexcept {
    return thread_.joinable();
  }

  [[nodiscard]] inline std::thread::id get_id() const noexcept {
    return thread_.get_id();
  }

  [[maybe_unused]] inline auto native_handle() {
    return thread_.native_handle();
  }

  inline void swap(RAIIThread& other) noexcept { thread_.swap(other.thread_); }

  inline RAIIThread& operator=(RAIIThread&& other) noexcept {
    thread_ = std::move(other.thread_);
    return *this;
  }

  /* Thread does not have an assignment operator */
  RAIIThread& operator=(const RAIIThread&) = delete;

 private:
  std::thread thread_;
};

}  // namespace util
}  // namespace cpet
#endif  // RAIITHREAD_H

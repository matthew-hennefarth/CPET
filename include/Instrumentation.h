#ifndef INSTRUMENTATION_H
#define INSTRUMENTATION_H

/* C++ STL HEADER FILES */
#include <chrono>
#include <memory>
#include <utility>
#include <functional>

/* EXTERNAL LIBRARY HEADER FILES */
#include "spdlog/spdlog.h"

class Timer{
    public:
        Timer() 
            : func_([](const float){})
        { 
            Start(); 
        }

        explicit Timer(std::shared_ptr<spdlog::logger> logger) 
            : logger_(std::move(logger)), func_([](const float){})
        {
            Start();
        }

        Timer(std::shared_ptr<spdlog::logger> logger, std::function<void(const float)> func) 
            : logger_(std::move(logger)), func_(std::move(func))
        {
            Start();
        }

        ~Timer()
        {
            end_ = std::chrono::steady_clock::now();
            duration_ = end_ - start_;

            const float sec = duration_.count();

            if (logger_ == nullptr){
                SPDLOG_INFO("Timer: {} sec", sec);
            }
            else{
                logger_->info("Timer: {} sec", sec);
            }
            func_(sec);
        }

        inline void Start() noexcept(true)
        {
            start_ = std::chrono::steady_clock::now();
        }

    private:
        std::chrono::time_point<std::chrono::steady_clock> start_, end_{};
        
        std::chrono::duration<float> duration_{};
        
        std::shared_ptr<spdlog::logger> logger_{nullptr};
        
        std::function<void(const float)> func_;
};
#endif //INSTRUMENTATION_H

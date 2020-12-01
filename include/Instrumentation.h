#ifndef INSTRUMENTATION_H
#define INSTRUMENTATION_H

#include <chrono>
#include <memory>
#include <utility>
#include <functional>

#include "spdlog/spdlog.h"

class Timer{
    public:
        Timer() 
            : _func([](const float){})
        { 
            Start(); 
        }

        explicit Timer(std::shared_ptr<spdlog::logger> logger) 
            : _logger(std::move(logger)), _func([](const float){})
        {
            Start();
        }

        Timer(std::shared_ptr<spdlog::logger> logger, std::function<void(const float)> func) 
            : _logger(std::move(logger)), _func(std::move(func))
        {
            Start();
        }

        ~Timer()
        {
            _end = std::chrono::steady_clock::now();
            _duration = _end - _start;

            const float sec = _duration.count();

            if (_logger == nullptr){
                SPDLOG_INFO("Timer: {} sec", sec);
            }
            else{
                _logger->info("Timer: {} sec", sec);
            }
            _func(sec);
        }

        inline void Start() noexcept(true)
        {
            _start = std::chrono::steady_clock::now();
        }

    private:
        std::chrono::time_point<std::chrono::steady_clock> _start, _end{};
        
        std::chrono::duration<float> _duration{};
        
        std::shared_ptr<spdlog::logger> _logger{nullptr};
        
        std::function<void(const float)> _func;
};
#endif //INSTRUMENTATION_H

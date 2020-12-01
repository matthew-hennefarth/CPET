#ifndef RAIITHREAD_H
#define RAIITHREAD_H

#include <thread>

class RAIIThread{
    public:
        RAIIThread() = default;

        template<class Function, class... Args>
        explicit RAIIThread(Function&& F, Args... args) noexcept : _thread(F, args...){}

        RAIIThread(RAIIThread&& other) noexcept : _thread(std::move(other._thread)) {}

        /* Thread does not have a copy constructor */
        RAIIThread(const RAIIThread& t) = delete;

        ~RAIIThread(){
            if(_thread.joinable()){
                _thread.join();
            }
        }

        constexpr void join() {
            _thread.join();
        }

        constexpr void detach() {
            _thread.detach();
        }

        inline bool joinable() const noexcept {
            return _thread.joinable();
        }

        [[nodiscard]] inline std::thread::id get_id() const noexcept {
            return _thread.get_id();
        }

        inline auto native_handle() {
            return _thread.native_handle();
        }

        constexpr void swap(RAIIThread& other) noexcept{
            _thread.swap(other._thread);
        }

        inline RAIIThread& operator=(RAIIThread&& other) noexcept {
            _thread = std::move(other._thread);
            return *this;
        }

        /* Thread does not have an assignment operator */
        RAIIThread& operator=(const RAIIThread&) = delete;

    private:
        std::thread _thread;
};

#endif //RAIITHREAD_H

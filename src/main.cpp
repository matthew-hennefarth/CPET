#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_sinks.h"

#include "System.h"
#include "Instrumentation.h"

//int main (int argc, char* argv[]) {
int main() {

#if SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_DEBUG
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%s %#] [%l] %v");
#else
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("%v");
#endif

    //TODO read in input with cxxopts

    System a("full.pqr", "options");

    auto logger = spdlog::stdout_logger_mt("Timer");
    logger->set_pattern("%v");

    int i = 10;
    std::vector<float> timer_results;
    timer_results.reserve(10);
    while(i --> 0)
    {
        logger->info("[Iteration] ==>> {}", 10-i);
        Timer t(logger, [&](const float f){timer_results.emplace_back(f);});
        a.calculateTopology(2);
    }
    float ave = 0;
    for(const auto& r : timer_results){
        ave += r;
    }
    ave /= timer_results.size();
    logger->info("[Average Time] ==>> {} sec", ave);

    return 0;
}

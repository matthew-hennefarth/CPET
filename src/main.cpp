#include "spdlog/spdlog.h"
#include "System.h"

//int main (int argc, char* argv[]) {
int main() {

#if SPDLOG_ACTIVE_LEVEL != SPDLOG_LEVEL_DEBUG
    spdlog::set_pattern("%v");
#else
    spdlog::set_pattern("[%s %#] [%l] %v");
#endif

    System a("full.pqr", "options");

    return 0;
}

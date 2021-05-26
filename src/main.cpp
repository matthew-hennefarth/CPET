// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

/* C++ STL HEADER FILES */
#include <filesystem>
#include <optional>
#include <string>

/* EXTERNAL LIBRARY HEADER FILES */
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#include <cxxopts.hpp>

/* CPET HEADER FILES */
#include "Calculator.h"
#include "Exceptions.h"
#include "config.h"

std::optional<std::string> validPDBFile(const cxxopts::ParseResult& result) {
  if (result.count("protein") != 0) {
    if (std::filesystem::exists(result["protein"].as<std::string>())) {
      return result["protein"].as<std::string>();
    }
  }
  return std::nullopt;
}

std::optional<std::string> validOptionFile(const cxxopts::ParseResult& result) {
  if (result.count("options") != 0) {
    if (std::filesystem::exists(result["options"].as<std::string>())) {
      return result["options"].as<std::string>();
    }
  }
  return std::nullopt;
}

std::optional<std::string> validChargeFile(const cxxopts::ParseResult& result) {
  if (!result["charges"].as<std::string>().empty()) {
    if (!std::filesystem::exists(result["charges"].as<std::string>())) {
      return std::nullopt;
    }
  }
  return result["charges"].as<std::string>();
}

std::optional<int> validThreads(const cxxopts::ParseResult& result) {
  if (result["threads"].as<int>() > 0) {
    return result["threads"].as<int>();
  }
  return std::nullopt;
}

int main(int argc, char** argv) {
  spdlog::set_pattern("%v");

  cxxopts::Options options(
      "cpet",
      std::string("Classical Protein Electric Field Topology, version ") +
          std::string(PROJECT_VER));
  options.add_options()(
        "d,debug", "Enable debugging",
        cxxopts::value<bool>()->default_value("false"))  // a bool parameter
        ("p,protein", "PDB", cxxopts::value<std::string>())(
            "o,options", "Option file", cxxopts::value<std::string>())(
            "c,charges", "Partial atomic charge definitions",
            cxxopts::value<std::string>()->default_value(""))(
            "t,threads", "Number of threads",
            cxxopts::value<int>()->default_value("1"))(
            "O,out", "Output file",
            cxxopts::value<std::string>()->default_value(""))("h,help",
                                                              "Print usage")(
            "v,verbose", "Verbose output",
            cxxopts::value<bool>()->default_value("false"));

  std::unique_ptr<cxxopts::ParseResult> tmp_result{nullptr};
  try {
    tmp_result =
        std::make_unique<cxxopts::ParseResult>(options.parse(argc, argv));
  } catch (cxxopts::OptionParseException& e) {
    SPDLOG_ERROR("Invalid parameters...");
    SPDLOG_ERROR(e.what());
    SPDLOG_WARN(options.help());
    return EXIT_FAILURE;
  }
  auto& result = *(tmp_result);

#if SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_DEBUG
  spdlog::set_level(spdlog::level::debug);
  spdlog::set_pattern("[%s %#] [%l] %v");
#else

  if (result["debug"].as<bool>()) {
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%s %#] [%l] %v");
  } else {
    if (result["verbose"].as<bool>()) {
      spdlog::set_level(spdlog::level::debug);
    } else {
      spdlog::set_level(spdlog::level::info);
    }
  }

#endif

  if (result.count("help") != 0) {
    SPDLOG_WARN(options.help());
    return EXIT_FAILURE;
  }

  std::optional<std::string> proteinFile;
  if (!(proteinFile = validPDBFile(result))) {
    SPDLOG_ERROR("Invalid protein file");
    SPDLOG_WARN(options.help());
    return EXIT_FAILURE;
  }

  std::optional<std::string> optionFile;
  if (!(optionFile = validOptionFile(result))) {
    SPDLOG_ERROR("Invalid option file");
    SPDLOG_WARN(options.help());
    return EXIT_FAILURE;
  }

  std::optional<int> numberOfThreads;
  if (!(numberOfThreads = validThreads(result))) {
    SPDLOG_ERROR("Invalid number of threads");
    SPDLOG_WARN(options.help());
    return EXIT_FAILURE;
  }

  std::optional<std::string> chargesFile;
  if (!(chargesFile = validChargeFile(result))) {
    SPDLOG_WARN("Invalid charge file");
    SPDLOG_WARN(options.help());
    return EXIT_FAILURE;
  }
  /* Begin the actual program here */
  try {
    cpet::Calculator c(proteinFile.value(), optionFile.value(),
                       chargesFile.value(), numberOfThreads.value());
    if (!result["out"].as<std::string>().empty()) {
      c.setOutputFilePrefix(result["out"].as<std::string>());
    }
    c.compute();
  } catch (const cpet::exception& exc) {
    SPDLOG_ERROR(exc.what());
    return EXIT_FAILURE;
  } catch (const std::exception& exc) {
    SPDLOG_ERROR("Unknown exception occured while running");
    SPDLOG_ERROR(exc.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

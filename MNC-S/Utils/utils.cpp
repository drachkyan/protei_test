#include "utils.h"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

void setupLogger(const std::string& provider, const std::string& path, const std::string& pattern) {
    std::vector<spdlog::sink_ptr> sinks;

    if (provider == "console" || provider == "both") {
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    }
    if (provider == "file" || provider == "both") {
        sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(path, true));
    }

    auto logger = std::make_shared<spdlog::logger>("main", sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::info);
    logger->set_pattern(pattern);
    logger->flush_on(spdlog::level::info);
    spdlog::set_default_logger(logger);
}
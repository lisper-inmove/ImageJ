#pragma once

#include <filesystem>

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <spdlog/spdlog.h>

static void setSpdlog() {
    auto file_logger = spdlog::rotating_logger_mt(
        "Main", "ImageJ.log", 5 * 1024 * 1024, 100
    );
    spdlog::set_default_logger(file_logger);
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::info);

}

#define __LOG_FILENAME__ (std::filesystem::path(__FILE__).filename().string().c_str())

#define LOG_DEBUG(fmt_, ...) spdlog::debug("[{}:{}] " fmt_, __LOG_FILENAME__, __LINE__, ##__VA_ARGS__)
#define LOG_INFO(fmt_, ...)  spdlog::info("[{}:{}] " fmt_, __LOG_FILENAME__, __LINE__, ##__VA_ARGS__)
#define LOG_WARN(fmt_, ...)  spdlog::warn("[{}:{}] " fmt_, __LOG_FILENAME__, __LINE__, ##__VA_ARGS__)
#define LOG_ERROR(fmt_, ...) spdlog::error("[{}:{}] " fmt_, __LOG_FILENAME__, __LINE__, ##__VA_ARGS__)
#define LOG_CRIT(fmt_, ...)  spdlog::critical("[{}:{}] " fmt_, __LOG_FILENAME__, __LINE__, ##__VA_ARGS__)

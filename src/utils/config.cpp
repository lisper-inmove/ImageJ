#include "utils/config.h"
#include <filesystem>

JConfig::JConfig() {
    std::filesystem::path exec_path = std::filesystem::current_path();
    std::filesystem::path config_path = exec_path / "config.yaml";
    config_ = YAML::LoadFile(config_path.string());
}

JConfig& JConfig::getInstance() {
    static JConfig instance;
    return instance;
}

YAML::Node& JConfig::getConfig() {
    return config_;
}

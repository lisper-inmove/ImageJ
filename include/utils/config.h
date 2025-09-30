#pragma once
#include <yaml-cpp/yaml.h>

class JConfig {
private:
    JConfig();

private:
    YAML::Node config_;

public:
    JConfig(JConfig& other) = delete;
    JConfig& operator=(JConfig& other) = delete;

public:
    static JConfig& getInstance();
    YAML::Node& getConfig();
};

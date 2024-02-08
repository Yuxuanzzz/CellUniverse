// Config.cpp
#include "Config.hpp"

#include <iostream>
#include <fstream>
#include "yaml-cpp/yaml.h"

/**
 * Include BacilliConfig and SphereConfig config classes
 * Include BaseConfig from pydantic
 */
#include "Sphere.cpp"

template <typename T>
class BaseConfig {
public:
    CellConfig cell;
    SimulationConfig simulation;
    ProbabilityConfig prob;
    BaseConfig(const YAML::Node& node) {
        // Parse YAML node and initialize config object
        simulation();
        prob();
        cell(node); //need to update cell/sphere.cpp
    }
};


template <typename T>
BaseConfig<T> loadConfig(const std::string& path) {
    YAML::Node config = YAML::LoadFile(path);

    if (config["cellType"].as<std::string>() == "sphere") {
        return BaseConfig<SphereConfig>(config);
    } else if (config["cellType"].as<std::string>() == "bacilli") {
        return BaseConfig<BacilliConfig>(config);
    } else {
        throw std::invalid_argument("Invalid cell type: " + config["cellType"].as<std::string>());
    }
}

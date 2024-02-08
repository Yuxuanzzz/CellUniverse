// ConfigTypes.hpp
#ifndef CONFIGTYPES_HPP
#define CONFIGTYPES_HPP

#include "Cell.hpp"

class SimulationConfig {
public:
    int iterationsPerCell;
    float backgroundColor;
    float cellColor;
    int padding = 0;
    float zScaling = 1;
    float blurSigma = 0;
    int zSlices = -1;
    std::vector<int> zValues;

    SimulationConfig() {
        iterationsPerCell = 0;
        backgroundColor = 0.0f;
        cellColor = 0.0f;
    }

    void checkZValues() const {
        if (!zValues.empty()) {
            throw std::invalid_argument("zValues should not be set manually");
        }
    }

    void checkZSlices() const {
        if (zSlices != -1) {
            throw std::invalid_argument("zSlices should not be set manually");
        }
    }
};

class ProbabilityConfig {
public:
    float perturbation;
    float split;

    static void checkProbability(std::vector<float> values) {
	//TODO
    }
    ProbabilityConfig()
    {
        perturbation = 0.0f;
        split = 0.0f; //unsure initialization
    }
};
#endif

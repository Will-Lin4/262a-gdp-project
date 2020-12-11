#pragma once
#ifndef MPL_DEMO_APP_OPTIONS_HPP
#define MPL_DEMO_APP_OPTIONS_HPP

#include <string>
#include <limits>
#include <json.hpp>

extern "C" {

#include <gdp/gdp.h>

}

using json = nlohmann::json;

class AppOptions {
public:
    static constexpr unsigned long MAX_JOBS = 1000;
    
    std::string scenario_;
    std::string algorithm_;
    std::string coordinator_;
    unsigned long jobs_{4};
    
    std::uint64_t problemId_;

    std::string env_;
    std::string robot_;
    std::string envFrame_;

    std::string start_;
    std::string goal_;
    std::string goalRadius_;

    std::string min_;
    std::string max_;

    double timeLimit_{std::numeric_limits<double>::infinity()};
    double checkResolution_{0};

    bool singlePrecision_{false};

public:
    inline AppOptions() {}
    AppOptions(int argc, char* argv[]);

    json toJson() const;
};

#endif

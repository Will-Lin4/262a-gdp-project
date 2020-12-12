#include <string>
#include <mpl/demo/lambda_common.hpp>
#include <iostream>

#include <gdp_agent.hpp>
#include <unistd.h>
#include "json.hpp"
#include <random> 
#include <time.h>
#include <algorithm>

using json = nlohmann::json;

namespace mpl::demo {
    void runSelectPlannerFromGDP(const std::string& problemId, double timeLimit) {
        auto start = time(NULL);
        gdp_agent agent;

        std::vector<uint8_t> cbor;
        std::default_random_engine engine(std::random_device{}());

        std::string inputLog = problemId + "_input";
        JI_LOG(INFO) << "Retrieving input from " << inputLog;

        uint8_t attempt = 1;
        do {
            if (attempt > 1) {
                std::uniform_int_distribution<uint64_t> distribution(0, (1 << attempt) * 1000);
                int cap_sleep = (timeLimit - (time(NULL) - start)) * 1000;
                if (cap_sleep > 0) {
                    uint64_t sleep_time = std::min(distribution(engine), (uint64_t) cap_sleep);
                    usleep(sleep_time);
                }
            }
            agent.open(inputLog);
            attempt += 1;
        } while (!EP_STAT_ISOK(agent.read_bytes(inputLog, 1, &cbor)) && time(NULL) - start < timeLimit);

        json json_options = json::from_cbor(cbor);        
        
        mpl::demo::AppOptions options;
        options.problemId_ = problemId;
        options.scenario_= json_options["scenario"];
        options.algorithm_ = json_options["algorithm"];                                          
        options.env_ = std::string(json_options["env"]);
        options.robot_ = std::string(json_options["robot"]);
        options.envFrame_ = json_options["envFrame"];
        options.start_ = json_options["start"];
        options.goal_ = json_options["goal"];
        options.goalRadius_ = json_options["goalRadius"];
        options.min_ = json_options["min"];
        options.max_ = json_options["max"];
        options.checkResolution_ = json_options["checkResolution"];
        options.singlePrecision_ = json_options["singlePrecision"];

        options.timeLimit_ = timeLimit;

        runSelectPlanner(options);
        JI_LOG(INFO) << "Started at: " << start;
    }
}

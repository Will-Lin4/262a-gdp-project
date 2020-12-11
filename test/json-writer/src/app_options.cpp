#include <app_options.hpp>
#include <getopt.h>
#include <iostream>
#include "json.hpp"
#include <gdp_agent.hpp>

namespace {

static void usage(const char *argv0) {
    std::cerr << "Usage: " << argv0 << R"( [options]
Options:
  -S, --scenario=(se3|fetch)    Set the scenario to run
  -a, --algorithm=(rrt|cforest) Set the algorithm to run
  -c, --coordinator=HOST:PORT   Specify the coordinator's host.  Port is optional.
  -j, --jobs=COUNT              Specify the number of simultaneous lambdas to run
  -I, --problem-id=ID           (this is for internal use only)
  -t, --time-limit=TIME         Specify the time limit of each lambda in seconds
  -e, --env=MESH                The environment mesh to use (valid for se3 and fetch)
  -E, --env-frame=X,Y,theta     The translation and rotation to apply to the environment (fetch only)
  -r, --robot=MESH              The robot's mesh (se3 only)
  -s, --start=W,I,J,K,X,Y,Z     The start configuration (se3 = rotation + translation, fetch = configuration)
  -g, --goal=W,I,J,K,X,Y,Z      (may be in joint space or IK frame)
  -G, --goal-radius=RADIUS      Specify the tolerances for the goal as either a scalar or twist (fetch only)
  -m, --min=X,Y,Z               Workspace minimum (se3 only)
  -M, --max=X,Y,Z               Workspace maximum (se3 only)
  -d, --check-resolution=DIST   Collision checking resolution (0 means use default)
  -f, --float                   Use single-precision math instead of double (not currently enabled)
)";
}

}

AppOptions::AppOptions(int argc, char *argv[]) {
    static struct option longopts[] = {
        { "scenario", required_argument, NULL, 'S' },
        { "algorithm", required_argument, NULL, 'a' },
        { "coordinator", required_argument, NULL, 'c' },
        { "jobs", required_argument, NULL, 'j' },
        { "env", required_argument, NULL, 'e' },
        { "env-frame", required_argument, NULL, 'E' },
        { "robot", required_argument, NULL, 'r' },
        { "goal", required_argument, NULL, 'g' },
        { "goal-radius", required_argument, NULL, 'G' },
        { "start", required_argument, NULL, 's' },
        { "min", required_argument, NULL, 'm' },
        { "max", required_argument, NULL, 'M' },
        { "problem-id", required_argument, NULL, 'I' },
        { "time-limit", required_argument, NULL, 't' },
        { "check-resolution", required_argument, NULL, 'd' },
        { "discretization", required_argument, NULL, 'd' }, // less-descriptive alieas
        { "float", no_argument, NULL, 'f' },
        
        { NULL, 0, NULL, 0 }
    };

    for (int ch ; (ch = getopt_long(argc, argv, "S:a:c:j:e:E:r:g:G:s:m:M:I:t:d:f", longopts, NULL)) != -1 ; ) {
        char *endp;
                
        switch (ch) {
        case 'S':
            scenario_ = optarg;
            break;
        case 'a':
            algorithm_ = optarg;
            break;
        case 'c':
            coordinator_ = optarg;
            break;
        case 'j':
            jobs_ = strtoul(optarg, &endp, 10);
            if (endp == optarg || *endp || jobs_ == 0 || jobs_ > MAX_JOBS)
                throw std::invalid_argument("bad value for --jobs");
            break;
        case 'e':
            env_ = optarg;
            break;
        case 'E':
            envFrame_ = optarg;
            break;
        case 'r':
            robot_ = optarg;
            break;
        case 'g':
            goal_ = optarg;
            break;
        case 'G':
            goalRadius_ = optarg;
            break;
        case 's':
            start_ = optarg;
            break;
        case 'm':
            min_ = optarg;
            break;
        case 'M':
            max_ = optarg;
            break;
        case 'I':
            problemId_ = strtoull(optarg, &endp, 0);
            if (endp == optarg || *endp)
                throw std::invalid_argument("bad value for --problem-id");
            break;
        case 't':
            timeLimit_ = std::strtod(optarg, &endp);
            if (endp == optarg || *endp || timeLimit_ < 0)
                throw std::invalid_argument("bad value for --time-limit");
            break;
        case 'd':
            checkResolution_ = std::strtod(optarg, &endp);
            if (endp == optarg || *endp || checkResolution_ < 0)
                throw std::invalid_argument("bad value for --check-resolution");
            break;
        case 'f':
            singlePrecision_ = true;
            break;
        default:
            usage(argv[0]);
            throw std::invalid_argument("see above");
        }            
    }
    
}

json AppOptions::toJson() const {
    json options;
    options["scenario"] = scenario_;
    options["algorithm"] = algorithm_;
    options["coordinator"] = coordinator_;
    options["jobs"] = jobs_;
    options["problemId"] = problemId_;
    options["env"] = env_;
    options["robot"] = robot_;
    options["envFrame"] = envFrame_;
    options["start"] = start_;
    options["goal"] = goal_;
    options["goalRadius"] = goalRadius_;
    options["min"] = min_;
    options["max"] = max_;
    options["timeLimit"] = timeLimit_;
    options["checkResolution"] = checkResolution_;
    options["singlePrecision"] = singlePrecision_;

    return options;
}

#include <string>
#include <mpl/demo/lambda_common.hpp>

namespace mpl::demo {
    void runSelectPlannerFromGDP(const std::string& gdp_name) {
        mpl::demo::AppOptions options;
        options.loadFromGDP(gdp_name);
        mpl::demo::runSelectPlanner(options);
    }
}

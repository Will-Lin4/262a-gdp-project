#include <mpl/demo/lambda_common.hpp>
// main.cpp
#include <aws/lambda-runtime/runtime.h>
#include <aws/core/Aws.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <iostream>
#include <memory>
#include <string>
#include <iostream>
#include <mpl/demo/gdp_wrapper.hpp>

extern "C" {
#include <gdp/gdp.h>
}

using namespace aws::lambda_runtime;

static void assign_value(std::string& value, const std::string& s) { value = s; }
static void assign_value(double& value, const std::string& s) { value = std::stod(s); }
static void assign_value(std::uint64_t& value, const std::string& s) { value = std::stoull(s); }

template <class T, class J>
static void set(T& value, const J& json, const std::string& name) {
    Aws::String name_aws(name.c_str(), name.size());
    if (json.ValueExists(name_aws)) {
        Aws::String aws_s = json.GetString(name_aws);
        std::string s(aws_s.c_str(), aws_s.size());
        assign_value(value, s);
    }
}

invocation_response my_handler(invocation_request const& request) try {
    using namespace Aws::Utils::Json;
    Aws::String payload(request.payload.c_str(), request.payload.size());
    JsonValue json(payload);
    
    if (!json.WasParseSuccessful()) {
        return invocation_response::failure("Failed to parse input JSON", "InvalidJSON");
    }

    auto v = json.View();

    std::string input_log;
    double time_limit;

    set(input_log, v, "problem_id");
    set(time_limit, v, "duration");

    mpl::demo::runSelectPlannerFromGDP(input_log, time_limit);
    return invocation_response::success("Solved!", "application/json");
} catch (const std::invalid_argument& ex) {
    std::cerr << "Invalid argument: " << ex.what() << std::endl;
    return invocation_response::failure(ex.what(), "InvalidArgument");
} catch (const std::exception& ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
    return invocation_response::failure(ex.what(), "Exception");
}

int main()
{
    run_handler(my_handler);
    return 0;
}


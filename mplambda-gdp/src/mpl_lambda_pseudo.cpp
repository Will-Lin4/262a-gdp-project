#include <mpl/demo/lambda_common.hpp>
#include <mpl/demo/gdp_wrapper.hpp>
#include <iostream>

int main(int argc, char *argv[]) try {
	/*
    static const std::string resourceDirectory = "/root/mplambda-gdp/resources/se3/";
    mpl::demo::AppOptions options(argc, argv);

    if (!options.env_.empty())
        options.env_ = resourceDirectory + options.env_;
    if (!options.robot_.empty())
        options.robot_ = resourceDirectory + options.robot_;

    mpl::demo::runSelectPlanner(options);
    */

    if (argc < 3) {
        std::cerr << "Insufficient arguments: " << std::endl;
        return EXIT_FAILURE;
    }

    const std::string input_log(argv[1]);
    double time_limit = std::stod(argv[2]);
    mpl::demo::runSelectPlannerFromGDP(input_log, time_limit);

    return EXIT_SUCCESS;
} catch (const std::invalid_argument& ex) {
    std::cerr << "Invalid argument: " << ex.what() << std::endl;
    return EXIT_FAILURE;
} catch (const std::exception& ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
    return EXIT_FAILURE;
}

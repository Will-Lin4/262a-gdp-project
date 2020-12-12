#include <mpl/gdp_comm.hpp>

mpl::GDPComm::~GDPComm() {

}

void mpl::GDPComm::setProblem(const demo::AppOptions& options) {
    // Compute hash of inputs
    // ProblemId acts as a nonce
    // Record hash


void mpl::GDPComm::connect(const std::string& gdpd_addr) {
    // gdp_init
    // Create output bucket
}

template <class PathFn>
void mpl::GDPComm::process(PathFn) {

}

template <class S, class Rep, class Period>
void mpl::GDPComm::sendPath(S cost, std::chrono::duration<Rep, Period> elapsed,
                            std::vector<std::tuple<Eigen::Quaternion<S>, Eigen::Matrix<S, 3, 1>>>&& path) {

}

void mpl::GDPComm::sendDone() {

}

inline bool mpl::GDPComm::isDone() {
    return done_;
}


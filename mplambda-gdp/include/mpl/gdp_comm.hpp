#pragma once
#ifndef MPL_GDP_COMM_HPP
#define MPL_GDP_COMM_HPP

#include <chrono>
#include <string>
#include <vector>
#include <Eigen/Dense>

namespace mpl {
    class GDPComm {
        std::string problemId_;
        bool done_;

    public:
        ~GDPComm();

        void setProblem(const demo::AppOptions& options);

        void connect(const std::string& gdpd_addr);

        template <class PathFn>
        void process(PathFn);

        template <class S, class Rep, class Period>
        void sendPath(S cost, std::chrono::duration<Rep, Period> elapsed, std::vector<std::tuple<Eigen::Quaternion<S>, Eigen::Matrix<S, 3, 1>>>&& path);
        
        void sendDone();

        inline bool isDone();
    };

}

#endif

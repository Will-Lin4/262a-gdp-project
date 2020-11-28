#pragma once
#ifndef MPL_GDP_COMM_HPP
#define MPL_GDP_COMM_HPP

#include <chrono>
#include <string>
#include <vector>
#include <Eigen/Dense>
#include <mpl/demo/app_options.hpp>

namespace mpl {
    class GDPComm {
        std::string problemId_;
        bool done_;

    public:
        ~GDPComm();

        inline void setProblem(std::string problemId) {
            problemId_ = problemId;
        }

        void process();

        template <class PathFn>
        void process(PathFn fn);

        template <class S, class Rep, class Period>
        void sendPath(S cost, std::chrono::duration<Rep, Period> elapsed, std::vector<std::tuple<Eigen::Quaternion<S>, Eigen::Matrix<S, 3, 1>>>&& path);

        template <class S, class Rep, class Period, int dim>
        void sendPath(S cost, std::chrono::duration<Rep, Period> elapsed, std::vector<Eigen::Matrix<S, dim, 1>>&& path);

        void sendDone();

        inline bool isDone() {
            return done_;
        }
    };

}


template <class PathFn>
void mpl::GDPComm::process(PathFn fn) {

}

template <class S, class Rep, class Period>
void mpl::GDPComm::sendPath(S cost, std::chrono::duration<Rep, Period> elapsed,
                            std::vector<std::tuple<Eigen::Quaternion<S>, Eigen::Matrix<S, 3, 1>>>&& path) {

}

template <class S, class Rep, class Period, int dim>
void mpl::GDPComm::sendPath(S cost, std::chrono::duration<Rep, Period> elapsed,
                            std::vector<Eigen::Matrix<S, dim, 1>>&& path){

}

#endif

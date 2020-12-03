#pragma once
#ifndef MPL_COMM_HPP
#define MPL_COMM_HPP

#include <string>
#include <netdb.h>
#include "write_queue.hpp"
#include "packet.hpp"
#include "syserr.hpp"
#include <gdp_agent.hpp>
#include <stdio.h>

#include <list>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace mpl {
    class Comm {
        int done_{false};

        //Buffer rBuf_{4*1024};
        gdp_agent gdp_;

        std::list<std::vector<uint8_t>> packet_buffer_;
        std::mutex buffer_mutex_;

        void writeBuffer(const Buffer& buffer);

        std::uint64_t problemId_{0};
        
        template <class T>
        void handle(T&&) {
            JI_LOG(WARN) << "unexpected packet type received: " << T::name();
        }

        static void receive_packet(std::vector<uint8_t> packet,
                                   void* comm_void_ptr);

        void handle(packet::Done&&);
        
        template <class PacketFn>
        void processImpl(PacketFn);

    public:
        ~Comm();

        void connect(const std::string& input_bucket);

        void setProblemId(std::uint64_t id) {
            problemId_ = id;
        }

        void process();

        template <class PathFn>
        void process(PathFn);

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

template <class S, class Rep, class Period>
void mpl::Comm::sendPath(
    S cost,
    std::chrono::duration<Rep, Period> elapsed,
    std::vector<std::tuple<Eigen::Quaternion<S>, Eigen::Matrix<S, 3, 1>>>&& path)
{
    using State = std::tuple<Eigen::Quaternion<S>, Eigen::Matrix<S, 3, 1>>;
    std::uint32_t elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    writeBuffer(packet::Path<State>(cost, elapsedMillis, std::move(path)));
}

template <class S, class Rep, class Period, int dim>
void mpl::Comm::sendPath(
    S cost,
    std::chrono::duration<Rep, Period> elapsed,
    std::vector<Eigen::Matrix<S, dim, 1>>&& path)
{
    using State = Eigen::Matrix<S, dim, 1>;
    std::uint32_t elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    writeBuffer(packet::Path<State>(cost, elapsedMillis, std::move(path)));
}
    

template <class PacketFn>
void mpl::Comm::processImpl(PacketFn fn) {
    if (packet_buffer_.size() == 0) {
        return;
    }

    std::vector<uint8_t> packet;
    {
        std::unique_lock<std::mutex> lock(buffer_mutex_);
        if (packet_buffer_.size() == 0) {
            return;
        }

        packet = std::move(packet_buffer_.front());
        packet_buffer_.pop_front();
    }

    ssize_t n = packet.size();
    Buffer rBuf(n);
    ::memcpy(rBuf.begin(), packet.data(), n);
    rBuf += n;
    rBuf.flip();

    packet::parse(rBuf, fn);
}

template <class PathFn>
void mpl::Comm::process(PathFn fn) {
    processImpl([&] (auto&& pkt) {
        using T = std::decay_t<decltype(pkt)>;
        if constexpr (packet::is_path<T>::value) {
            fn(pkt.cost(), std::move(pkt).path());
        } else {
            handle(std::forward<decltype(pkt)>(pkt));
        }
    });
}

#endif

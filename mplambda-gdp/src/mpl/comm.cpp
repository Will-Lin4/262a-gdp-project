#include <mpl/comm.hpp>
#include <mpl/packet.hpp>
#include <jilog.hpp>
#include <algorithm>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <poll.h>
#include <arpa/inet.h>
#include <random>
#include <thread>
#include <chrono>

mpl::Comm::~Comm() {

}

void mpl::Comm::receive_packet(std::vector<uint8_t> packet,
                               void* comm_void_ptr) {
    mpl::Comm* comm_ptr = (mpl::Comm*) comm_void_ptr;
    {
        std::unique_lock<std::mutex> lock(comm_ptr->buffer_mutex_);
        comm_ptr->packet_buffer_.emplace_back(std::move(packet));
    }
}

void mpl::Comm::connect(mpl::Comm* comm) {
    std::string outputLog = comm->problemId_ + "_output";

    uint8_t attempt = 1;
    std::default_random_engine engine(std::random_device{}());
    do {
        if (attempt > 1) {
            std::uniform_int_distribution<uint64_t> distribution(0, (1 << attempt) * 1000);
            uint64_t sleep_duration = distribution(engine);
            {
                std::unique_lock<std::mutex> lock(comm->sleep_mutex_);
                comm->sleep_cv_.wait_for(lock, std::chrono::milliseconds(sleep_duration),
                                         [comm] { return comm->done_; } );
                if (comm->done_) return;
            }
        }

        comm->gdp_.open(outputLog);
        attempt += 1;
    } while (!EP_STAT_ISOK(comm->gdp_.subscribe(outputLog, &receive_packet, comm)));

    JI_LOG(INFO) << "connected to coordinator bucket";
    comm->connected_ = true;

    // Wait for packets
    while (!comm->done_) {
        Buffer buffer;
        {
            std::unique_lock<std::mutex> lock(comm->path_mutex_);
            comm->path_cv_.wait(lock, [comm] { return comm->best_path_.has_value() || comm->done_; });
            if (comm->done_) return;
            buffer = std::move(*(comm->best_path_));
            comm->best_path_ = std::nullopt;
        }

        std::vector<uint8_t> buffer_bytes((uint8_t*) buffer.begin(), (uint8_t*) buffer.end());

        uint8_t write_attempt = 1;
        while (!comm->best_path_.has_value()
                && !EP_STAT_ISOK(
                    comm->gdp_.write_bytes(outputLog, buffer_bytes.data(), buffer_bytes.size()))
                && !comm->done_) {
            write_attempt += 1;

            std::uniform_int_distribution<uint64_t> distribution(0, (1 << write_attempt) * 1000);
            uint64_t sleep_duration = distribution(engine);
            {
                std::unique_lock<std::mutex> lock(comm->sleep_mutex_);
                comm->sleep_cv_.wait_for(lock, std::chrono::milliseconds(sleep_duration),
                                         [comm] { return comm->done_; } );
                if (comm->done_) return;
            }
        }
    }
}

std::thread mpl::Comm::connect_async() {
    return std::thread(connect, this);
}

void mpl::Comm::writeBuffer(const Buffer& buffer) {
    std::string outputLog = problemId_ + "_output";
    gdp_.open(outputLog);
    gdp_.write_bytes(outputLog, (uint8_t*) buffer.begin(), buffer.remaining());
}

void mpl::Comm::process() {
    processImpl([&] (auto&& pkt) { handle(std::forward<decltype(pkt)>(pkt)); });
}

void mpl::Comm::handle(packet::Done&&) {
    JI_LOG(INFO) << "received DONE";
    done_ = true;
    path_cv_.notify_all();
    sleep_cv_.notify_all();
}

void mpl::Comm::sendDone() {
    writeBuffer(packet::Done(0));
    done_ = true;
    path_cv_.notify_all();
    sleep_cv_.notify_all();
}

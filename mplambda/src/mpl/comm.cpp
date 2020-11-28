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

mpl::Comm::~Comm() {

}

void mpl::Comm::receive_packet(std::vector<uint8_t> packet,
                               void* comm_void_ptr) {
    mpl::Comm* comm_ptr = (mpl::Comm*) comm_void_ptr;
    std::unique_lock<std::mutex> lock(comm_ptr->buffer_mutex_);
    comm_ptr->packet_buffer_.emplace_back(std::move(packet));
    lock.release();
    comm_ptr->buffer_cv_.notify_one();
}

void mpl::Comm::connect(const std::string& input_bucket) {
    gdp_.open(input_bucket);
    gdp_.subscribe(input_bucket, &receive_packet, this);
}

void mpl::Comm::writeBuffer(const Buffer& buffer) {
    gdp_.open("mpl_out");
    gdp_.write_bytes("mpl_out", (uint8_t*) buffer.begin(), buffer.remaining());
}

void mpl::Comm::process() {
    processImpl([&] (auto&& pkt) { handle(std::forward<decltype(pkt)>(pkt)); });
}

void mpl::Comm::handle(packet::Done&&) {
    JI_LOG(INFO) << "received DONE";
    done_ = true;
}

void mpl::Comm::sendDone() {
    writeBuffer(packet::Done(problemId_));
}

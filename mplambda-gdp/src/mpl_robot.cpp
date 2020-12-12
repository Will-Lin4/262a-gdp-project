#include <jilog.hpp>
#include <mpl/buffer.hpp>
#include <mpl/write_queue.hpp>
#include <mpl/packet.hpp>
#include <mpl/syserr.hpp>
#include <mpl/demo/app_options.hpp>
#include <mpl/demo/se3_rigid_body_scenario.hpp>
#include <netdb.h>
#include <getopt.h>
#include <iostream>
#include <chrono>
#include <poll.h>

#include <gdp_agent.hpp>
#include <time.h>
#include <mutex>
#include <cstdlib>

double min_cost = 0;
std::mutex cost_mutex;

namespace mpl {

template <class T>
void process(T&&) {
    std::cout << time(NULL) << "\tUnexpected Packet" << std::endl;
}

void process(packet::Done&& pkt) {
    std::cout << time(NULL) << "\tDONE" << std::endl;
    std::exit(0);
}

template <class State>
void process(packet::Path<State>&& pkt) {
    auto cost = pkt.cost();
    if (cost < min_cost || min_cost == 0) {
        std::lock_guard<std::mutex> lock(cost_mutex);
        if (cost < min_cost || min_cost == 0) {
            min_cost = cost;
            std::cout << time(NULL) << "\t" << cost << std::endl;
        }
    }
}

}

void receive_packet(std::vector<uint8_t> packet, void* empty) {
    ssize_t n = packet.size();
    mpl::Buffer rBuf(n);
    ::memcpy(rBuf.begin(), packet.data(), n);
    rBuf += n;
    rBuf.flip();

    mpl::packet::parse(rBuf, [] (auto&& pkt) {
        mpl::process(std::forward<decltype(pkt)>(pkt));
    });
}

int main(int argc, char *argv[]) try {
    gdp_agent agent_;
    std::string input = argv[1];
    agent_.open(input);
    if (EP_STAT_ISOK(agent_.subscribe(input, receive_packet, nullptr))) {
        std::cout << time(NULL) << "\tCONNECTED" << std::endl;
        sleep(3600);
    } else {
        std::cout << time(NULL) << "\tFAIL" << std::endl;
    }

} catch (const std::exception& ex) {
    JI_LOG(FATAL) << "error: " << ex.what();
    return EXIT_FAILURE;
}


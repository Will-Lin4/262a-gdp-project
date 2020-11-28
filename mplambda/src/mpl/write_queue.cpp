#include <jilog.hpp>
#include <mpl/write_queue.hpp>
#include <mpl/syserr.hpp>
#include <gdp_agent.hpp>

void mpl::WriteQueue::writeTo(int socket) {
    if (empty())
        return;
    
    iovs_.clear();
    for (auto it = buffers_.begin() ; iovs_.size() < MAX_IOVS && it != buffers_.end() ; ++it) {
        iovs_.emplace_back();
        iovs_.back().iov_base = it->begin();
        iovs_.back().iov_len = it->remaining();
    }

    JI_LOG(TRACE) << "about to write " << iovs_.size() << " iovecs to " << socket;
    ssize_t n = ::writev(socket, iovs_.data(), iovs_.size());
    JI_LOG(TRACE) << "wrote " << n << " bytes to " << socket;
    if (n == -1) {
        if (errno == EAGAIN)
            return;
        throw syserr("writev");
    }

    while (n > 0) {
        if (n >= buffers_.front().remaining()) {
            n -= buffers_.front().remaining();
            buffers_.pop_front();
            JI_LOG(TRACE) << "removing completed buffer";
        } else {
            JI_LOG(TRACE) << "updating buffer in front";
            buffers_.front() += n;
            break;
        }
    }
}

void mpl::WriteQueue::writeTo(const std::string& bucket) {
    if (empty())
        return;
    
    std::vector<uint8_t> output;
    for (auto it = buffers_.begin() ; iovs_.size() < MAX_IOVS && it != buffers_.end() ; ++it) {
        output.insert(output.end(), it->begin(), it->end());
    }

    JI_LOG(TRACE) << "about to write " << iovs_.size() << " iovecs to " << bucket;

    gdp_agent agent;
    agent.create(bucket);
    agent.open(bucket);
    agent.write_bytes(bucket, output.data(), output.size());

    buffers_.clear();
}

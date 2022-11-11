//============================================================================
// Name        : node.hh
// Author      : Fabian Mora
// Version     : 1.0
// License     : MIT license
// Description :
//============================================================================

#ifndef __NODE_HH__
#define __NODE_HH__

#include <mpi.h>

#include "definitions.hh"
#include "util/util.hh"

namespace MPIBatch {
template <typename task_info_t>
class Node {
public:
    // Constexpr definitions
    static constexpr int w2m_status_tag = static_cast <int>(MessageTags::w2m_status);
    static constexpr int m2w_status_tag = static_cast <int>(MessageTags::m2w_status);
    static constexpr int send_data_tag = static_cast <int>(MessageTags::send_data);
    static constexpr int recv_data_tag = static_cast <int>(MessageTags::recv_data);
    static constexpr int kill_tag = static_cast <int>(MessageTags::kill);

    // Class members
    int rank = -1;
    bool active_cycle = false;
    bool scheduled = false;
    NodeState state = NodeState::unknown;
    WorkerStatus status = WorkerStatus::unknown;
    WorkerStatus status_buffer = WorkerStatus::unknown;
    MPI_Request send_status = MPI_REQUEST_NULL;
    MPI_Request recv_status = MPI_REQUEST_NULL;
    MPI_Request send_data = MPI_REQUEST_NULL;
    MPI_Request recv_data = MPI_REQUEST_NULL;

    RequestStatus send_status_s = RequestStatus::null;
    RequestStatus recv_status_s = RequestStatus::null;
    RequestStatus send_data_s = RequestStatus::null;
    RequestStatus recv_data_s = RequestStatus::null;

    time_point last_ping = now();

    task_info_t task_info = task_info_t(nullptr, invalid_task_id);

    int64_t serialization_id = invalid_task_id;

    Node();
    ~Node();

    Node(const Node&) = delete;
    Node(Node &&node);

    Node& operator=(const Node&) = delete;
    Node& operator=(Node &&node);

    void clear();
    void clear_task_info();
    void* status_addr();
    void* status_buffer_addr();

    bool syncQ();
    void sync();
};

template <typename task_info_t>
inline Node <task_info_t>::Node() {
}

template <typename task_info_t>
inline Node <task_info_t>::~Node() {
    clear();
}

template <typename task_info_t>
inline Node <task_info_t>::Node(Node &&node) {
    rank = node.rank;
    state = node.state;
    status = node.status;
    status_buffer = node.status_buffer;
    send_status = node.send_status;
    recv_status = node.recv_status;
    send_data = node.send_data;
    recv_data = node.recv_data;
    send_status_s = node.send_status_s;
    recv_status_s = node.recv_status_s;
    send_data_s = node.send_data_s;
    recv_data_s = node.recv_data_s;
    last_ping = node.last_ping;
    task_info = node.task_info;
    active_cycle = node.active_cycle;
    scheduled = node.scheduled;
    node.clear();
}

template <typename task_info_t>
inline Node <task_info_t>& Node <task_info_t>::operator =(Node &&node) {
    rank = node.rank;
    state = node.state;
    status = node.status;
    status_buffer = node.status_buffer;
    send_status = node.send_status;
    recv_status = node.recv_status;
    send_data = node.send_data;
    recv_data = node.recv_data;
    send_status_s = node.send_status_s;
    recv_status_s = node.recv_status_s;
    send_data_s = node.send_data_s;
    recv_data_s = node.recv_data_s;
    last_ping = node.last_ping;
    task_info = node.task_info;
    active_cycle = node.active_cycle;
    scheduled = node.scheduled;
    node.clear();
    return *this;
}

template <typename task_info_t>
inline void Node <task_info_t>::clear() {
    rank = -1;
    active_cycle = false;
    scheduled = false;
    state = NodeState::terminated;
    status = WorkerStatus::unknown;
    status_buffer = WorkerStatus::unknown;
    send_status = MPI_REQUEST_NULL;
    recv_status = MPI_REQUEST_NULL;
    send_data = MPI_REQUEST_NULL;
    recv_data = MPI_REQUEST_NULL;
    send_status_s = RequestStatus::null;
    recv_status_s = RequestStatus::null;
    send_data_s = RequestStatus::null;
    recv_data_s = RequestStatus::null;
    serialization_id = invalid_task_id;
    clear_task_info();
}

template <typename task_info_t>
inline void Node <task_info_t>::clear_task_info() {
    task_info = task_info_t(nullptr, invalid_task_id);
}

template <typename task_info_t>
inline void* Node <task_info_t>::status_addr() {
    return reinterpret_cast <void*>(&status);
}

template <typename task_info_t>
inline void* Node <task_info_t>::status_buffer_addr() {
    return reinterpret_cast <void*>(&status_buffer);
}

template <typename task_info_t>
inline bool Node <task_info_t>::syncQ() {
    return status == status_buffer;
}

template <typename task_info_t>
inline void Node <task_info_t>::sync() {
    status_buffer = status;
}

}
#endif

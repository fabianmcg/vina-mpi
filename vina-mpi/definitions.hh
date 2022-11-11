//============================================================================
// Name        : definitions.hh
// Author      : Fabian Mora
// Version     : 1.0
// License     : MIT license
// Description :
//============================================================================

#ifndef __DEFINITIONS_HH__
#define __DEFINITIONS_HH__

#include <chrono>
#include <type_traits>
#include <mpi.h>

namespace MPIBatch {

enum class WorkerStatus : int {
    killed = -2,
    unknown = -1,
    available = 0,
    running,
    wait,
    finished
};

enum class NodeState : int {
    unknown = -1,
    booting,
    ready,
    receiving_status,
    receiving_status_complete,
    sending_status,
    sending_status_complete,
    sending_data,
    sending_data_complete,
    receiving_data,
    receiving_data_complete,
    launch_task,
    terminated,
    cleaning
};

enum class RequestStatus : int {
    null,
    active,
    started,
    canceled,
    nonblocking
};

enum class MessageTags : int {
    any = MPI_ANY_TAG,
    general,
    w2m_status = 10,
    m2w_status = w2m_status,
    send_data,
    recv_data = send_data,
    kill
};

enum class LogType {
    warn = -2,
    error = -1,
    trace = 0,
    info
};

enum class NodeType {
    unknown = -1,
    master,
    worker
};

enum class ServerMode {
    autocontained,
    ompi_server
};

constexpr std::chrono::milliseconds small_sleep(10);
constexpr std::chrono::milliseconds medium_sleep(50);
constexpr std::chrono::milliseconds large_sleep(100);
constexpr std::chrono::milliseconds small_cycle_timeout(100);
constexpr std::chrono::milliseconds cycle_timeout(500);
constexpr std::chrono::seconds task_status_cycle(5);
constexpr std::chrono::seconds ping_timeout(5);
constexpr std::chrono::seconds send_recv_timeout(3);
constexpr std::chrono::seconds worker_timeout(10);
constexpr std::chrono::seconds master_timeout(20);

constexpr int64_t invalid_task_id = -1;

template <bool condidition>
using enable_it = std::enable_if_t<condidition, int>;

using time_point = std::chrono::high_resolution_clock::time_point;
using duration = std::chrono::duration <double>;

inline bool active_worker_statusQ(WorkerStatus status) {
    if (status != WorkerStatus::unknown && status != WorkerStatus::killed)
        return true;
    return false;
}
}
#endif

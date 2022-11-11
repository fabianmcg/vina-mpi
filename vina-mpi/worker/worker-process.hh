//============================================================================
// Name        : worker-process.hh
// Author      : Fabian Mora
// Version     : 1.0
// License     : MIT license
// Description :
//============================================================================

#ifndef __WORKER_PROCESS_HH__
#define __WORKER_PROCESS_HH__

#include <algorithm>
#include <chrono>
#include <map>
#include <string>
#include <thread>
#include <mpi.h>

#include "../definitions.hh"
#include "../io/logger.hh"
#include "../util/util.hh"
#include "../util/mpi.hh"
#include "../node.hh"

namespace MPIBatch {

template <ServerMode mode>
class WorkerProcess {
public:
    using node_t = Node<std::pair<void*, int64_t>>;
private:
    // MPI information
    int rank = -1;
    int rank_size = -1;
    std::string hostname = "";
    MPI_Comm communicator = MPI_COMM_NULL;

    // Internal data & methods
    node_t node = node_t();
    time_point master_last_ping = time_point();
    Logger __logger__;

    void init_channels();
    bool send_status(duration timeout, duration sleep);
    bool recv_status(duration timeout, duration sleep);
    template <typename data_t>
    bool recv_data(std::vector <data_t> &buffer, int size, MPI_Datatype type, int source,
            duration timeout, duration sleep);
    template <typename Serializer, typename Task, typename data_t>
    bool full_cycle(Serializer &serializer, Task &task, std::vector <data_t> &buffer,
            time_point &last_ping, time_point &last_task_ping, duration timeout, duration sleep);
public:
    WorkerProcess(std::string log_path = "", MPI_Comm communicator = MPI_COMM_WORLD);
    ~WorkerProcess();
    WorkerProcess& operator=(WorkerProcess &&other) = delete;
    WorkerProcess(const WorkerProcess &other) = delete;
    WorkerProcess(WorkerProcess &&other) = delete;
    WorkerProcess& operator=(const WorkerProcess &other) = delete;

    Logger& logger();

    template <typename Serializer, typename Task>
    int run(Serializer &serializer, Task &task);
};

template <ServerMode mode>
inline WorkerProcess <mode>::WorkerProcess(std::string log_path, MPI_Comm communicator) :
        __logger__(Logger()) {
    if (!log_path.empty())
        __logger__.open(log_path);
    __logger__.set_info(NodeType::worker, "", -1);
    int flag;
    mpi_error <true>(__logger__, MPI_Initialized(&flag));
    if (flag == 0) {
        throw(std::runtime_error("MPI has not been initialized"));
    }
    this->communicator = communicator;
    mpi_error <true>(__logger__, MPI_Comm_rank(this->communicator, &rank));
    int name_len;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    mpi_error <true>(__logger__, MPI_Get_processor_name(processor_name, &name_len));
    hostname = processor_name;
    __logger__.set_info(NodeType::worker, hostname, rank);
    mpi_error <true>(__logger__, MPI_Comm_size(this->communicator, &rank_size));
    master_last_ping = now();
    node.rank = rank;
    node.status = WorkerStatus::available;
    node.state = NodeState::booting;
}

template <ServerMode mode>
inline WorkerProcess <mode>::~WorkerProcess() {
    node.state = NodeState::cleaning;
    terminate_request <true, false>(__logger__, &(node.send_status), node.send_status_s);
    terminate_request <true, false>(__logger__, &(node.recv_status), node.recv_status_s);
    terminate_request <true, false>(__logger__, &(node.send_data), node.send_data_s);
    terminate_request <true, false>(__logger__, &(node.recv_data), node.recv_data_s);
    node.clear();
    rank = -1;
    rank_size = 0;
    hostname = "";
    communicator = MPI_COMM_NULL;
    master_last_ping = time_point();
}

template <ServerMode mode>
inline Logger& WorkerProcess <mode>::logger() {
    return __logger__;
}

template <ServerMode mode>
inline void WorkerProcess <mode>::init_channels() {
    MPI_Request *request = &(node.send_status);
    mpi_error <true>(__logger__,
            MPI_Send_init(node.status_buffer_addr(), 1, MPI_INT, 0, node_t::w2m_status_tag,
                    communicator, request));
    node.send_status_s = RequestStatus::active;
    request = &(node.recv_status);
    mpi_error <true>(__logger__,
            MPI_Recv_init(node.status_addr(), 1, MPI_INT, 0, node_t::m2w_status_tag, communicator,
                    request));
    node.recv_status_s = RequestStatus::active;
    node.state = NodeState::ready;
}

template <ServerMode mode>
inline bool WorkerProcess <mode>::send_status(duration timeout, duration sleep) {
    MPI_Request *request = &(node.send_status);
    node.state = NodeState::sending_status;
    if (node.send_status_s == RequestStatus::active) {
        node.sync();
        mpi_error <true>(__logger__, MPI_Start(request));
        node.send_status_s = RequestStatus::started;
    }
    if (node.send_status_s == RequestStatus::started) {
        bool result = try_request(__logger__, request, timeout, sleep);
        if (result) {
            node.send_status_s = RequestStatus::active;
            node.state = NodeState::sending_status_complete;
        }
        return result;
    }
    return false;
}

template <ServerMode mode>
inline bool WorkerProcess <mode>::recv_status(duration timeout, duration sleep) {
    MPI_Request *request = &(node.recv_status);
    node.state = NodeState::receiving_status;
    if (node.recv_status_s == RequestStatus::active) {
        mpi_error <true>(__logger__, MPI_Start(request));
        node.recv_status_s = RequestStatus::started;
    }
    if (node.recv_status_s == RequestStatus::started) {
        bool result = try_request(__logger__, request, timeout, sleep);
        if (result) {
            node.recv_status_s = RequestStatus::active;
            node.state = NodeState::receiving_status_complete;
        }
        return result;
    }
    return false;
}

template <ServerMode mode>
template <typename data_t>
bool WorkerProcess <mode>::recv_data(std::vector <data_t> &buffer, int size, MPI_Datatype type,
        int source, duration timeout, duration sleep) {
    MPI_Request *request = &(node.recv_data);
    node.state = NodeState::receiving_data;
    if (node.recv_data_s == RequestStatus::null) {
        buffer.resize(size);
        void *ptr = reinterpret_cast <void*>(buffer.data());
        mpi_error <true>(__logger__,
                MPI_Irecv(ptr, size, type, source, node_t::recv_data_tag, communicator, request));
        node.recv_data_s = RequestStatus::nonblocking;
    }
    if (node.recv_data_s == RequestStatus::nonblocking) {
        bool result = try_request(__logger__, request, timeout, sleep);
        if (result) {
            node.recv_data_s = RequestStatus::null;
            node.recv_data = MPI_REQUEST_NULL;
            node.state = NodeState::receiving_data_complete;
        }
        return result;
    }
    return false;
}

template <ServerMode mode>
template <typename Serializer, typename Task, typename data_t>
bool WorkerProcess <mode>::full_cycle(Serializer &serializer, Task &task,
        std::vector <data_t> &buffer, time_point &last_ping, time_point &last_task_ping,
        duration timeout, duration sleep) {
    MPI_Status mpi_status;
    bool modified_clock = false;
    if (node.state == NodeState::ready || node.state == NodeState::sending_status) {
        if (elapsed(now(), last_task_ping) >= task_status_cycle) {
            if (node.status == WorkerStatus::running) {
                if (task.finished()) {
                    node.status = WorkerStatus::finished;
                    if (task.task_instance.valid()) {
                        try {
                            task.task_instance.get();
                        } catch (std::exception &exc) {
                            __logger__(LogType::warn, "Worker ", rank,
                                    " has experienced an exception message: ", exc.what());
                        }
                    }
                }
            }
            last_task_ping = now();
        }
        send_status(timeout, sleep);
    }
    if (node.state == NodeState::sending_status_complete
            || node.state == NodeState::receiving_status) {
        auto [err, incomingQ] = iprobe(__logger__, 0, node_t::m2w_status_tag, &mpi_status, communicator);
        if (err == 0 && incomingQ) {
            recv_status(timeout, sleep);
            last_ping = now();
            modified_clock = true;
        }
    }
    if (node.state == NodeState::receiving_status_complete) {
        if (!active_worker_statusQ(node.status)) {
            node.state = NodeState::terminated;
        }
        if (node.status == WorkerStatus::available) {
            auto [err, incomingQ] = iprobe(__logger__, 0, node_t::recv_data_tag, &mpi_status,
                    communicator);
            if (err == 0 && incomingQ) {
                int size;
                mpi_error <true>(__logger__, MPI_Get_count(&mpi_status, serializer.mpi_type(), &size));
                recv_data(buffer, size, serializer.mpi_type(), 0, timeout, sleep);
                last_ping = now();
                modified_clock = true;
            }
        } else if (node.status == WorkerStatus::running) {
            node.state = NodeState::ready;
            node.active_cycle = false;
        }
    }
    if (node.state == NodeState::receiving_data_complete)
        node.state = NodeState::launch_task;
    if (node.state == NodeState::launch_task) {
        node.status = WorkerStatus::running;
        task.run(std::ref(__logger__), serializer.deserialize(buffer), &(node.status));
        node.state = NodeState::ready;
        node.active_cycle = false;
    }
    return modified_clock;
}

template <ServerMode mode>
template <typename Serializer, typename Task>
inline int WorkerProcess <mode>::run(Serializer &serializer, Task &task) {
    bool ntimed_out = false;
    time_point last_task_ping;
    std::vector <typename Serializer::data_t> buffer;
    __logger__(LogType::info, "Worker ", rank, " has started");
    node.status = WorkerStatus::available;
    init_channels();
    master_last_ping = now();
    last_task_ping = now();
    while ((ntimed_out = (elapsed(now(), master_last_ping) <= worker_timeout))
            && active_worker_statusQ(node.status)) {
        time_point start = now();
        if (node.active_cycle == false && active_worker_statusQ(node.status))
            node.active_cycle = true;
        if (node.active_cycle) {
            if (!full_cycle(serializer, task, buffer, master_last_ping, last_task_ping,
                    cycle_timeout, small_sleep))
                master_last_ping += now() - start;
        }
        if (node.status == WorkerStatus::available) {
            std::this_thread::sleep_for(medium_sleep);
        } else if (node.status == WorkerStatus::running) {
            std::this_thread::sleep_for(large_sleep);
        }
    }
    if (!ntimed_out) {
        __logger__(LogType::warn, "Communication channel has timed out ",
                static_cast <int>(node.status));
    }
    __logger__(LogType::info, "Worker ", rank, " has ended");
    return 0;
}
}
#endif

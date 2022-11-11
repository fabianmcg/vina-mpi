//============================================================================
// Name        : master-process.hh
// Author      : Fabian Mora
// Version     : 1.0
// License     : MIT license
// Description :
//============================================================================

#ifndef __MASTER_PROCESS_HH__
#define __MASTER_PROCESS_HH__

#include <algorithm>
#include <chrono>
#include <map>
#include <string>
#include <thread>
#include <mpi.h>

#include "../definitions.hh"
#include "../node.hh"
#include "../io/logger.hh"
#include "../util/mpi.hh"
#include "../util/util.hh"

namespace MPIBatch {
template <typename TaskQueue, ServerMode mode>
class MasterProcess {
public:
    using node_t = Node<typename TaskQueue::task_info_t>;
private:
    // MPI information
    int rank = -1;
    int rank_size = -1;
    std::string hostname = "";
    MPI_Comm communicator = MPI_COMM_NULL;

    // Internal data members
    std::map <int, node_t> nodes = std::map <int, node_t>();
    TaskQueue __queue__;
    Logger __logger__;

    // Private function members
    bool active_workerQ(int worker_rank);
    size_t active_workers();

    void init_channels();
    bool recv_status(int worker_rank, duration timeout, duration sleep);
    bool send_status(int worker_rank, duration timeout, duration sleep);
    template <typename Serializer>
    bool send_data(Serializer &serializer, int worker_rank, duration timeout, duration sleep);
    bool respond_worker(int worker_rank, duration timeout, duration sleep);
    template <typename Serializer>
    void full_cycle(Serializer &serializer, int worker_rank, duration timeout, duration sleep);
public:
    MasterProcess(std::string log_path = "", MPI_Comm communicator = MPI_COMM_WORLD,
            const TaskQueue &queue = TaskQueue());
    ~MasterProcess();
    MasterProcess& operator=(MasterProcess &&other) = delete;
    MasterProcess(const MasterProcess &other) = delete;
    MasterProcess(MasterProcess &&other) = delete;
    MasterProcess& operator=(const MasterProcess &other) = delete;

    TaskQueue& queue();
    const TaskQueue& queue() const;

    Logger& logger();

    void move_queue(TaskQueue &queue);

    template <typename Serializer>
    int run(Serializer &serializer, const duration &report_interval);
};

template <typename TaskQueue, ServerMode mode>
inline MasterProcess <TaskQueue, mode>::MasterProcess(std::string log_path, MPI_Comm communicator,
        const TaskQueue &queue) :
        __queue__(queue), __logger__(Logger()) {
    if (!log_path.empty())
        __logger__.open(log_path);
    __logger__.set_info(NodeType::master, "", -1);
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
    __logger__.set_info(NodeType::master, hostname, rank);
    mpi_error <true>(__logger__, MPI_Comm_size(this->communicator, &rank_size));
    if (rank_size > 0) {
        for (int i = 0; i < rank_size; ++i) {
            nodes[i] = node_t();
            nodes[i].rank = rank;
            nodes[i].state = NodeState::booting;
        }
        nodes[0].status = WorkerStatus::available;
    }
}

template <typename TaskQueue, ServerMode mode>
inline MasterProcess <TaskQueue, mode>::~MasterProcess() {
    for (int worker_rank = 1; worker_rank < rank_size; ++worker_rank) {
        nodes[worker_rank].state = NodeState::cleaning;
        terminate_request <true, false>(__logger__, &(nodes[worker_rank].send_status),
                nodes[worker_rank].send_status_s);
        terminate_request <true, false>(__logger__, &(nodes[worker_rank].recv_status),
                nodes[worker_rank].recv_status_s);
        terminate_request <true, false>(__logger__, &(nodes[worker_rank].send_data),
                nodes[worker_rank].send_data_s);
        terminate_request <true, false>(__logger__, &(nodes[worker_rank].recv_data),
                nodes[worker_rank].recv_data_s);
        nodes[worker_rank].clear();
    }
    rank = -1;
    rank_size = 0;
    hostname = "";
    communicator = MPI_COMM_NULL;
    nodes.clear();
}

template <typename TaskQueue, ServerMode mode>
inline bool MasterProcess <TaskQueue, mode>::active_workerQ(int worker_rank) {
    return active_worker_statusQ(nodes[worker_rank].status);
}

template <typename TaskQueue, ServerMode mode>
inline size_t MasterProcess <TaskQueue, mode>::active_workers() {
    size_t workers = 0;
    for (size_t i = 1; i < nodes.size(); ++i)
        if (active_workerQ(i))
            ++workers;
    return workers;
}

template <typename TaskQueue, ServerMode mode>
inline void MasterProcess <TaskQueue, mode>::init_channels() {
    for (int worker_rank = 1; worker_rank < rank_size; ++worker_rank) {
        void *status_address = nodes[worker_rank].status_addr();
        MPI_Request *request = &(nodes[worker_rank].recv_status);
        mpi_error <true>(__logger__,
                MPI_Recv_init(status_address, 1, MPI_INT, worker_rank, node_t::w2m_status_tag,
                        communicator, request));
        nodes[worker_rank].recv_status_s = RequestStatus::active;
        status_address = nodes[worker_rank].status_buffer_addr();
        request = &(nodes[worker_rank].send_status);
        mpi_error <true>(__logger__,
                MPI_Send_init(status_address, 1, MPI_INT, worker_rank, node_t::m2w_status_tag,
                        communicator, request));
        nodes[worker_rank].send_status_s = RequestStatus::active;
        nodes[worker_rank].state = NodeState::ready;
    }
}

template <typename TaskQueue, ServerMode mode>
inline bool MasterProcess <TaskQueue, mode>::recv_status(int worker_rank, duration timeout,
        duration sleep) {
    WorkerStatus worker_status = nodes[worker_rank].status;
    MPI_Request *request = &(nodes[worker_rank].recv_status);
    nodes[worker_rank].state = NodeState::receiving_status;
    if (nodes[worker_rank].recv_status_s == RequestStatus::active) {
        mpi_error <true>(__logger__, MPI_Start(request));
        nodes[worker_rank].recv_status_s = RequestStatus::started;
    }
    if (nodes[worker_rank].recv_status_s == RequestStatus::started) {
        bool result = try_request(__logger__, request, timeout, sleep);
        if (worker_status == WorkerStatus::unknown && active_workerQ(worker_rank))
            __logger__(LogType::info, "Worker: ", worker_rank, " has come to life");
        if (result) {
            nodes[worker_rank].recv_status_s = RequestStatus::active;
            nodes[worker_rank].state = NodeState::receiving_status_complete;
        }
        return result;
    }
    return false;
}

template <typename TaskQueue, ServerMode mode>
inline bool MasterProcess <TaskQueue, mode>::send_status(int worker_rank, duration timeout,
        duration sleep) {
    MPI_Request *request = &(nodes[worker_rank].send_status);
    nodes[worker_rank].state = NodeState::sending_status;
    if (nodes[worker_rank].send_status_s == RequestStatus::active) {
        nodes[worker_rank].sync();
        mpi_error <true>(__logger__, MPI_Start(request));
        nodes[worker_rank].send_status_s = RequestStatus::started;
    }
    if (nodes[worker_rank].send_status_s == RequestStatus::started) {
        bool result = try_request(__logger__, request, timeout, sleep);
        if (result) {
            nodes[worker_rank].send_status_s = RequestStatus::active;
            nodes[worker_rank].state = NodeState::sending_status_complete;
        }
        return result;
    }
    return false;
}

template <typename TaskQueue, ServerMode mode>
template <typename Serializer>
inline bool MasterProcess <TaskQueue, mode>::send_data(Serializer &serializer, int worker_rank,
        duration timeout, duration sleep) {
    node_t &node = nodes[worker_rank];
    if (node.task_info.first != nullptr) {
        MPI_Request *request = &(node.send_data);
        node.state = NodeState::sending_data;
        if (node.send_data_s == RequestStatus::null) {
            auto serialized_data = serializer(*(node.task_info).first);
            if ((std::get <1>(serialized_data) > 0) && (std::get <0>(serialized_data) != nullptr)) {
                mpi_error <true>(__logger__,
                        MPI_Isend(std::get <0>(serialized_data), std::get <1>(serialized_data),
                                std::get <2>(serialized_data), worker_rank, node_t::recv_data_tag,
                                communicator, request));
                node.serialization_id = std::get <3>(serialized_data);
                node.send_data_s = RequestStatus::nonblocking;
            } else {
                node.send_data_s = RequestStatus::null;
                node.send_data = MPI_REQUEST_NULL;
                node.state = NodeState::sending_data_complete;
                serializer.free(node.serialization_id);
                node.serialization_id = invalid_task_id;
            }
        }
        if (node.send_data_s == RequestStatus::nonblocking) {
            bool result = try_request(__logger__, request, timeout, sleep);
            if (result) {
                node.send_data_s = RequestStatus::null;
                node.send_data = MPI_REQUEST_NULL;
                node.state = NodeState::sending_data_complete;
                serializer.free(node.serialization_id);
                node.serialization_id = invalid_task_id;
            }
            return result;
        }
    }
    return false;
}

template <typename TaskQueue, ServerMode mode>
inline bool MasterProcess <TaskQueue, mode>::respond_worker(int worker_rank, duration timeout,
        duration sleep) {
    node_t &node = nodes[worker_rank];
    WorkerStatus &worker_status = node.status;
    bool msg_sent = false;
    if (worker_status == WorkerStatus::finished) {
        node.scheduled = false;
        __queue__.completed(node.task_info.second);
    }
    if ((worker_status == WorkerStatus::available) || (worker_status == WorkerStatus::finished)) {
        node.clear_task_info();
        if (__queue__.available_tasks()) {
            node.scheduled = true;
            worker_status = WorkerStatus::available;
            node.task_info = __queue__.pop();
            msg_sent = send_status(worker_rank, timeout, sleep);
        } else if (__queue__.empty() && active_workerQ(worker_rank)) {
            node.scheduled = false;
            worker_status = WorkerStatus::killed;
            msg_sent = send_status(worker_rank, timeout, sleep);
        }
    } else if (worker_status == WorkerStatus::running) {
        msg_sent = send_status(worker_rank, timeout, sleep);
    }
    return msg_sent;
}

template <typename TaskQueue, ServerMode mode>
template <typename Serializer>
inline void MasterProcess <TaskQueue, mode>::full_cycle(Serializer &serializer, int worker_rank,
        duration timeout, duration sleep) {
    node_t &node = nodes[worker_rank];
    if (node.state == NodeState::ready || node.state == NodeState::receiving_status)
        recv_status(worker_rank, timeout, sleep);
    if (node.state == NodeState::receiving_status_complete)
        respond_worker(worker_rank, timeout, sleep);
    else if (node.state == NodeState::sending_status)
        send_status(worker_rank, timeout, sleep);
    if (node.state == NodeState::sending_status_complete) {
        if (node.status != WorkerStatus::running && active_workerQ(worker_rank) && node.scheduled)
            send_data(serializer, worker_rank, timeout, sleep);
        else {
            node.state = NodeState::ready;
            node.active_cycle = false;
        }
    }
    if (node.state == NodeState::sending_data)
        send_data(serializer, worker_rank, timeout, sleep);
    if (node.state == NodeState::sending_data_complete) {
        node.state = NodeState::ready;
        node.active_cycle = false;
    }
}

template <typename TaskQueue, ServerMode mode>
inline TaskQueue& MasterProcess <TaskQueue, mode>::queue() {
    return __queue__;
}

template <typename TaskQueue, ServerMode mode>
inline const TaskQueue& MasterProcess <TaskQueue, mode>::queue() const {
    return __queue__;
}

template <typename TaskQueue, ServerMode mode>
inline Logger& MasterProcess <TaskQueue, mode>::logger() {
    return __logger__;
}

template <typename TaskQueue, ServerMode mode>
void MasterProcess <TaskQueue, mode>::move_queue(TaskQueue &queue) {
    queue = std::move(__queue__);
}

template <typename TaskQueue, ServerMode mode>
template <typename Serializer>
inline int MasterProcess <TaskQueue, mode>::run(Serializer &serializer,
        const duration &report_interval) {
    MPI_Status mpi_status;
    time_point last_ping;
    time_point last_report;
    bool report = true;
    bool ntimed_out = false;
    __logger__(LogType::info, "Server has started");
    init_channels();
    last_ping = now();
    last_report = now();
    while ((ntimed_out = (elapsed(now(), last_ping) <= master_timeout)) && !__queue__.finished()) {
        time_point start = now();
        if (report) {
            std::tuple <size_t, size_t, size_t> queue_status = __queue__.status();
            __logger__(LogType::info, "Active workers: ", active_workers(), ", Completed tasks: ",
                    std::get <0>(queue_status), ", Scheduled tasks: ", std::get <1>(queue_status),
                    ", Remaining tasks: ", std::get <2>(queue_status));
            report = false;
            last_report = now();
        }
        auto [err, incomingQ] = iprobe(__logger__, MPI_ANY_SOURCE, node_t::w2m_status_tag, &mpi_status,
                communicator);
        if (incomingQ && (err == 0)) {
            mpi_error <true>(__logger__, mpi_status.MPI_ERROR);
            int worker_rank = mpi_status.MPI_SOURCE;
            if (nodes[worker_rank].active_cycle == false) {
                nodes[worker_rank].active_cycle = true;
                full_cycle(serializer, worker_rank, cycle_timeout, medium_sleep);
                last_ping = now();
                nodes[worker_rank].last_ping = last_ping;
            }
        } else {
            for (size_t worker_rank = 1; worker_rank < nodes.size(); ++worker_rank)
                if (nodes[worker_rank].active_cycle == true)
                    full_cycle(serializer, worker_rank, small_cycle_timeout, small_sleep);
            last_ping += now() - start;
            std::this_thread::sleep_for(medium_sleep);
        }
        report = report || (elapsed(now(), last_report) >= report_interval);
    }
    if (!ntimed_out) {
        __logger__(LogType::info, "Communication channel has timed out, shutting down");
    }
    std::tuple <size_t, size_t, size_t> queue_status = __queue__.status();
    __logger__(LogType::info, "Peak number of workers: ", rank_size - 1, ", Completed tasks: ",
            std::get <0>(queue_status), ", Scheduled tasks: ", std::get <1>(queue_status),
            ", Remaining tasks: ", std::get <2>(queue_status));
    __logger__(LogType::info, "Server has ended");
    return 0;
}
}
#endif

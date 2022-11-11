//============================================================================
// Name        : mpi.hh
// Author      : Fabian Mora
// Version     : 1.0
// License     : MIT license
// Description :
//============================================================================

#ifndef __BATCH_MPI_HH__
#define __BATCH_MPI_HH__

#include <chrono>
#include <thread>
#include <mpi.h>

#include "../definitions.hh"
#include "../io/logger.hh"

namespace MPIBatch {
template <bool throwQ = true, LogType log_type = LogType::error>
int mpi_error(Logger &logger, int status) {
    if (status != MPI_SUCCESS) {
        int err_len;
        char err_msg[MPI_MAX_ERROR_STRING];
        MPI_Error_string(status, err_msg, &err_len);
        std::string msg = err_msg;
        logger(log_type, "MPI error code: ", status, ", error message: ", msg);
        if constexpr (throwQ) {
            throw(std::runtime_error(
                    "MPI error code: " + std::to_string(status) + ", error message: " + msg));
        }
        return 1;
    }
    return 0;
}

template <typename duration_t1, typename unit_t1, typename duration_t2, typename unit_t2>
std::pair <int, bool> iprobe(int source, int tag, MPI_Status *status, MPI_Comm communicator,
        std::chrono::duration <duration_t1, unit_t1> timeout,
        std::chrono::duration <duration_t2, unit_t2> sleep = medium_sleep) {
    std::pair <int, bool> result(MPI_SUCCESS, false);
    int flag;
    int64_t tries = timeout / medium_sleep;
    int64_t i = 0;
    int error;
    while (i++ < tries) {
        if ((error = MPI_Iprobe(source, tag, communicator, &flag, status)) != MPI_SUCCESS) {
            result.first = error;
            break;
        }
        if (flag == 0 && (i != tries))
            std::this_thread::sleep_for(sleep);
        else {
            result.second = true;
            break;
        }
    }
    return result;
}

template <bool throwQ = true>
std::pair <int, bool> iprobe(Logger &logger, int source, int tag, MPI_Status *status,
        MPI_Comm communicator) {
    int flag;
    std::pair <int, bool> result(MPI_SUCCESS, false);
    result.first = mpi_error <throwQ>(logger, MPI_Iprobe(source, tag, communicator, &flag, status));
    result.second = flag == 1;
    return result;
}

template <typename duration_t1, typename unit_t1, typename duration_t2, typename unit_t2>
std::pair <int, bool> try_request(MPI_Request *request, MPI_Status *mpi_status,
        std::chrono::duration <duration_t1, unit_t1> timeout,
        std::chrono::duration <duration_t2, unit_t2> sleep = medium_sleep) {
    std::pair <int, bool> result(MPI_SUCCESS, false);
    int error;
    int flag;
    int64_t tries = timeout / medium_sleep;
    int64_t i = 0;
    while (i++ < tries) {
        if ((error = MPI_Test(request, &flag, mpi_status)) != MPI_SUCCESS) {
            result.first = error;
            break;
        }
        if (flag == 0 && (i != tries))
            std::this_thread::sleep_for(sleep);
        else {
            result.second = true;
            break;
        }
    }
    return result;
}

template <typename duration_t1, typename unit_t1, typename duration_t2, typename unit_t2>
inline bool try_request(Logger &logger, MPI_Request *request,
        std::chrono::duration <duration_t1, unit_t1> timeout,
        std::chrono::duration <duration_t2, unit_t2> sleep) {
    if (request != nullptr) {
        if ((*request) != MPI_REQUEST_NULL) {
            MPI_Status mpi_status = MPI_Status();
            std::pair <int, bool> request_result = MPIBatch::try_request(request, &mpi_status,
                    timeout, sleep);
            mpi_error <true>(logger, request_result.first);
            return request_result.second;
        }
    }
    return false;
}

template <typename duration_t1, typename unit_t1, typename duration_t2, typename unit_t2>
int wait_request(MPI_Request *request) {
    int error = MPI_SUCCESS;
    if ((error = MPI_Start(request)) == MPI_SUCCESS) {
        error = MPI_Wait(request, MPI_STATUS_IGNORE);
    }
    return error;
}

template <bool cancel = false, bool throwQ = true>
inline int terminate_request(Logger &logger, MPI_Request *request, RequestStatus &status) {
    int error = MPI_SUCCESS;
    if (request != nullptr) {
        if (*request != MPI_REQUEST_NULL) {
            switch (status) {
            case RequestStatus::null:
                break;
            case RequestStatus::active:
                break;
            case RequestStatus::started:
                if constexpr (cancel)
                    error = mpi_error <throwQ>(logger, MPI_Cancel(request));
                if (error == MPI_SUCCESS)
                    error = mpi_error <throwQ>(logger, MPI_Wait(request, MPI_STATUS_IGNORE));
                break;
            case RequestStatus::canceled:
                error = mpi_error <throwQ>(logger, MPI_Wait(request, MPI_STATUS_IGNORE));
                break;
            case RequestStatus::nonblocking:
                break;
            default:
                break;
            }
        }
        if ((error == MPI_SUCCESS) && (status != RequestStatus::null))
            error = mpi_error <throwQ>(logger, MPI_Request_free(request));
    }
    status = RequestStatus::null;
    return error;
}
}
#endif

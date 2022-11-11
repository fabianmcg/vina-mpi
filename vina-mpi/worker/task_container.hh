//============================================================================
// Name        : task_container.hh
// Author      : Fabian Mora
// Version     : 1.0
// License     : MIT license
// Description :
//============================================================================

#ifndef __TASK_CONTAINER_HH__
#define __TASK_CONTAINER_HH__

#include <functional>
#include <future>
#include <type_traits>

namespace MPIBatch {

template <typename function_t>
struct task_container {
    using return_t = typename std::function <function_t>::result_type;
    std::function <function_t> task;
    std::future <return_t> task_instance;

    ~task_container() {
    }

    template <typename ...Args>
    void run(Args &&... args) {
        if(task) {
            task_instance = std::async(std::launch::async, task, args...);
        }
    }

    bool finished() {
        auto status = task_instance.wait_for(std::chrono::seconds(0));
        if (status == std::future_status::timeout)
            return false;
        else
            return true;
    }
};
}
#endif

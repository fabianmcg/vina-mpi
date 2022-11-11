//============================================================================
// Name        : task-queue.hh
// Author      : Fabian Mora
// Version     : 1.0
// License     : MIT license
// Description :
//============================================================================

#ifndef __TASK_QUEUE_HH__
#define __TASK_QUEUE_HH__

#include <map>
#include <deque>
#include <unordered_set>
#include "../definitions.hh"

namespace MPIBatch {
template <typename data_t>
class task_queue {
public:
    using task_info_t = std::pair <data_t*, int64_t>;
private:
    std::map <int64_t, data_t> __data__;
    std::deque <int64_t> __queue__;
    std::unordered_set <int64_t> __scheduled_queue__;
    std::deque <int64_t> __completed_queue__;
    int64_t __next_task_id__ = 0;
    void clear();
public:
    task_queue();
    ~task_queue();
    task_queue(const task_queue &queue);
    task_queue(task_queue &&queue);
    task_queue& operator =(const task_queue &queue);
    task_queue& operator =(task_queue &&queue);

    bool available_tasks() const;
    void completed(int64_t task_id);
    bool empty() const;
    bool finished() const;

    template <typename Iterator>
    void insert(const Iterator &begin, const Iterator &end);
    void push(data_t &&data);
    std::pair <data_t*, int64_t> pop();
    void requeue(std::pair <data_t*, int64_t> &task);
    std::tuple <size_t, size_t, size_t> status() const;
};

template <typename data_t>
inline task_queue <data_t>::task_queue() :
        __data__(), __queue__(), __scheduled_queue__(), __completed_queue__() {
}

template <typename data_t>
inline task_queue <data_t>::~task_queue() {
    clear();
}

template <typename data_t>
inline task_queue <data_t>::task_queue(const task_queue &queue) :
        __data__(queue.__data__), __queue__(queue.__queue__), __scheduled_queue__(
                queue.__scheduled_queue__), __completed_queue__(queue.__completed_queue__) {
    __next_task_id__ = queue.__next_task_id__;
}

template <typename data_t>
inline task_queue <data_t>::task_queue(task_queue &&queue) :
        __data__(std::move(queue.__data__)), __queue__(std::move(queue.__queue__)), __scheduled_queue__(
                std::move(queue.__scheduled_queue__)), __completed_queue__(
                std::move(queue.__completed_queue__)) {
    __next_task_id__ = queue.__next_task_id__;
    queue.clear();
}

template <typename data_t>
inline task_queue <data_t>& task_queue <data_t>::operator =(const task_queue &queue) {
    __data__ = queue.__data__;
    __queue__ = queue.__queue__;
    __scheduled_queue__ = queue.__scheduled_queue__;
    __completed_queue__ = queue.__completed_queue__;
    __next_task_id__ = queue.__next_task_id__;
    return *this;
}

template <typename data_t>
inline task_queue <data_t>& task_queue <data_t>::operator =(task_queue &&queue) {
    __data__ = std::move(queue.__data__);
    __queue__ = std::move(queue.__queue__);
    __scheduled_queue__ = std::move(queue.__scheduled_queue__);
    __completed_queue__ = std::move(queue.__completed_queue__);
    __next_task_id__ = queue.__next_task_id__;
    queue.clear();
    return *this;
}

template <typename data_t>
inline void task_queue <data_t>::clear() {
    __next_task_id__ = invalid_task_id;
    __queue__.clear();
    __scheduled_queue__.clear();
    __completed_queue__.clear();
    __data__.clear();
}

template <typename data_t>
inline bool task_queue <data_t>::available_tasks() const {
    return !empty();
}

template <typename data_t>
inline void task_queue <data_t>::completed(int64_t task_id) {
    if (task_id >= 0) {
        auto it = __scheduled_queue__.find(task_id);
        if (it != __scheduled_queue__.end()) {
            __completed_queue__.push_back(*it);
            __scheduled_queue__.erase(it);
        }
    }
}

template <typename data_t>
inline bool task_queue <data_t>::empty() const {
    return __queue__.empty();
}

template <typename data_t>
inline bool task_queue <data_t>::finished() const {
    return __queue__.empty() && __scheduled_queue__.empty();
}

template <typename data_t>
template <typename Iterator>
inline void task_queue <data_t>::insert(const Iterator &begin, const Iterator &end) {
    for (auto it = begin; it != end; ++it) {
        __data__[__next_task_id__] = *it;
        __queue__.push_back(__next_task_id__);
        ++__next_task_id__;
    }
}

template <typename data_t>
inline void task_queue <data_t>::push(data_t &&data) {
    __data__[__next_task_id__] = std::move(data);
    __queue__.push_back(__next_task_id__);
    ++__next_task_id__;
}

template <typename data_t>
inline std::pair <data_t*, int64_t> task_queue <data_t>::pop() {
    std::pair <data_t*, int64_t> result(nullptr, invalid_task_id);
    if (!empty()) {
        result.second = __queue__.front();
        __queue__.pop_front();
        result.first = &(__data__.at(result.second));
        __scheduled_queue__.insert(result.second);
    }
    return result;
}

template <typename data_t>
inline void task_queue <data_t>::requeue(std::pair <data_t*, int64_t> &task) {
    auto it = __scheduled_queue__.find(task.second);
    if (it != __scheduled_queue__.end() && (task.second >= 0)) {
        __queue__.push_front(task.second);
        __scheduled_queue__.erase(it);
    }
}

template <typename data_t>
inline std::tuple <size_t, size_t, size_t> task_queue <data_t>::status() const {
    std::tuple <size_t, size_t, size_t> result = std::tuple <size_t, size_t, size_t>(
            __completed_queue__.size(), __scheduled_queue__.size(), __queue__.size());
    return result;
}
}

#endif

#include "scheduler/queue.h"
using namespace std;

namespace rtos {
    void ReadyQueue::push(Task* task) {
        if (task) {
            queue.push(task);
        }
    }

    Task* ReadyQueue::pop() {
        if (queue.empty()) {
            return nullptr;
        }
        Task* task = queue.top();
        queue.pop();
        return task;
    }

    bool ReadyQueue::empty() const {
        return queue.empty();
    }

    size_t ReadyQueue::size() const {
        return queue.size();
    }

    Task* ReadyQueue::peek() const {
        if (queue.empty()) {
            return nullptr;
        }
        return queue.top();
    }

    void ReadyQueue::clear() {
        while (!queue.empty()) {
            queue.pop();
        }
    }
}
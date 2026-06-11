#pragma once
#include "task.h"
#include <vector>
#include <queue>
#include <algorithm>
#include "utils/logger.h"
using namespace std;

namespace rtos {
    struct TaskPriorityComparator {
        bool operator()(const Task* a, const Task* b) const {
            return a->priority > b->priority;
        }
    };

    class ReadyQueue {
    private:
        priority_queue<Task*, vector<Task*>, TaskPriorityComparator> queue;

    public:
        void push(Task* task);

        Task* pop();

        bool empty() const;

        size_t size() const;

        Task* peek() const;

        void clear();
    };
}
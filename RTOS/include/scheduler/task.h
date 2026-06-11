#pragma once
#include <string>
#include <functional>
#include <cstdint>
#include "utils/logger.h"
using namespace std;

namespace rtos {
    enum class TaskState {
        READY,
        RUNNING,
        BLOCKED,
        SUSPENDED,
        COMPLETED
    };

    struct Task {
        uint32_t id;
        string name;
        uint32_t period;
        uint32_t deadline;
        uint32_t wcet;
        uint32_t priority;
        TaskState state;

        uint32_t remaining_time;
        uint32_t next_release;
        uint32_t start_time;
        uint32_t missed_count;

        function<void()> job;

        Task(uint32_t id, const string& name, uint32_t period,
            uint32_t wcet, uint32_t deadline, function<void()> job);

        void reset();

        void execute();

        bool is_deadline_missed(uint32_t current_time) const;

        string to_string() const;
    };
}
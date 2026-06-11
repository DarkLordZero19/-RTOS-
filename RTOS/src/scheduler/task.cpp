#include "scheduler/task.h"
#include <sstream>
#include <iomanip>
#include <iostream>
using namespace std;

namespace rtos {
    Task::Task(uint32_t id, const string& name, uint32_t period,
        uint32_t wcet, uint32_t deadline, function<void()> job)
        : id(id), name(name), period(period), deadline(deadline),
        wcet(wcet), priority(0), state(TaskState::READY),
        remaining_time(wcet), next_release(0), start_time(0), job(job),
        missed_count(0) {

        if (id == 0) {
            cerr << "Ошибка: ID задачи не может быть 0.\n";
        }
        if (name.empty()) {
            cerr << "Ошибка: Имя задачи не может быть пустым.\n";
        }
        if (period == 0) {
            cerr << "Ошибка: Период задачи не может быть 0.\n";
        }
        if (wcet == 0 || wcet > period) {
            cerr << "Ошибка: Некорректное WCET.\n";
        }
    }

    void Task::reset() {
        remaining_time = wcet;
        state = TaskState::READY;
    }

    void Task::execute() {
        if (remaining_time > 0) {
            state = TaskState::RUNNING;
            remaining_time--;
            if (remaining_time == 0) {
                state = TaskState::COMPLETED;
            }
        }
    }

    bool Task::is_deadline_missed(uint32_t current_time) const {
        if (state == TaskState::RUNNING || state == TaskState::READY) {
            uint32_t elapsed = current_time - start_time;
            return elapsed > deadline;
        }
        return false;
    }

    string Task::to_string() const {
        stringstream ss;
        ss << left << setw(20) << name
            << " [ID: " << setw(3) << id << "]";

        ss << " P=" << setw(4) << period
            << "ms WCET=" << setw(3) << wcet << "ms"
            << " D=" << setw(4) << deadline << "ms";

        ss << " Prio=" << setw(4) << priority;

        ss << " State: ";
        switch (state) {
        case TaskState::READY:     ss << "READY    "; break;
        case TaskState::RUNNING:   ss << "RUNNING  "; break;
        case TaskState::BLOCKED:   ss << "BLOCKED  "; break;
        case TaskState::SUSPENDED: ss << "SUSPENDED"; break;
        case TaskState::COMPLETED: ss << "COMPLETED"; break;
        }

        ss << " Rem: " << setw(2) << remaining_time << "ms";
        return ss.str();
    }
}
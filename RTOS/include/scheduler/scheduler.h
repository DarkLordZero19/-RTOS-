#pragma once
#include "task.h"
#include "queue.h"
#include <vector>
#include <memory>
#include <map>
#include "utils/logger.h"
using namespace std;

namespace rtos {
    class RMScheduler {
    private:
        vector<unique_ptr<Task>> tasks;
        ReadyQueue ready_queue;
        Task* current_task;
        uint32_t system_time;
        uint32_t tick_counter;

        static constexpr uint32_t CONTEXT_SWITCH_OVERHEAD_MS = 1;

        uint32_t missed_deadlines;
        uint32_t context_switches;
        uint32_t idle_time;
        mutable utils::Logger logger;

        mutable long long min_sched_time_us;
        mutable long long max_sched_time_us;
        mutable long long total_sched_time_us;
        mutable unsigned int sched_calls;

        void calculate_priorities();
        void update_ready_queue();
        void check_deadlines();
        void reset_simulation();

    public:
        RMScheduler(const string& log_filename = "logs/scheduler.log");
        ~RMScheduler() = default;

        bool save_to_file(const string& filename);
        bool load_from_file(const string& filename);

        bool save_configuration(const string& config_name, const string& description = "");
        bool load_configuration(const string& config_name);
        vector<string> get_available_configurations() const;
        bool delete_configuration(const string& config_name);

        string serialize_tasks() const;
        bool deserialize_tasks(const string& data);

        size_t get_task_count() const { return tasks.size(); }

        double get_liu_layland_bound() const {
            size_t n = tasks.size();
            if (n == 0) return 0.0;
            return n * (pow(2.0, 1.0 / n) - 1);
        }

        bool add_task(const string& name, uint32_t period, uint32_t wcet, uint32_t deadline, function<void()> job);

        bool is_schedulable() const;

        double calculate_cpu_utilization() const;

        void tick();

        void simulate(uint32_t ticks);

        Task* get_current_task() const;

        void print_statistics() const;

        void print_task_table() const;
        void print_ready_queue() const;

        void reset();

        const vector<unique_ptr<Task>>& get_tasks() const { return tasks; }

        void export_to_csv(const string& filename) const;
    };
}
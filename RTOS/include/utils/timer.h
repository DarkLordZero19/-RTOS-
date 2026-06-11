#pragma once
#include <cstdint>
using namespace std;

namespace utils {
    class TimerEmulator {
    private:
        uint32_t tick_count;
        uint32_t tick_interval_ms;
        bool running;
    public:
        TimerEmulator(uint32_t interval_ms = 1);

        void start();

        void stop();

        uint32_t get_ticks() const;

        uint32_t get_time_ms() const;

        void increment_tick();

        void reset();

        bool is_running() const;
    };
}
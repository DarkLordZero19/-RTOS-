#include "utils/timer.h"
namespace utils {
    TimerEmulator::TimerEmulator(uint32_t interval_ms)
        : tick_count(0), tick_interval_ms(interval_ms), running(false) {
    }
    void TimerEmulator::start() {
        running = true;
    }
    void TimerEmulator::stop() {
        running = false;
    }
    uint32_t TimerEmulator::get_ticks() const {
        return tick_count;
    }
    uint32_t TimerEmulator::get_time_ms() const {
        return tick_count * tick_interval_ms;
    }
    void TimerEmulator::increment_tick() {
        if (running) {
            tick_count++;
        }
    }
    void TimerEmulator::reset() {
        tick_count = 0;
        running = false;
    }
    bool TimerEmulator::is_running() const {
        return running;
    }
}
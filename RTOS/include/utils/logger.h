#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>  
#include <iomanip>
#include <fstream>
#include <memory>
using namespace std;

namespace utils {

    enum class LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    class Logger {
    private:
        LogLevel current_level;
        ofstream log_file;
        bool file_logging_enabled;
        bool console_logging_enabled;

        string level_to_string(LogLevel level) const;

        string get_current_time() const;
        void write_to_file(LogLevel level, const string& message);
        void write_to_console(LogLevel level, const string& message);

    public:
        Logger(LogLevel level = LogLevel::INFO,
            const string& filename = "",
            bool enable_console = false);

        ~Logger();

        void set_level(LogLevel level);
        void enable_file_logging(const string& filename);
        void disable_file_logging();
        void enable_console_logging(bool enable);

        void log(LogLevel level, const string& message);

        void debug(const string& message);
        void info(const string& message);
        void warning(const string& message);
        void error(const string& message);

        void separator(char ch = '=', int length = 60);

        void header(const string& title);

        void flush();
    };
}
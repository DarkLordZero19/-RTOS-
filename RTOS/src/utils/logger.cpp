#include "utils/logger.h"
#include <sstream>
#include <iomanip>
#include <clocale>
using namespace std;
namespace utils {
    Logger::Logger(LogLevel level, const string& filename, bool enable_console)
        : current_level(level), file_logging_enabled(false), console_logging_enabled(enable_console) {
        setlocale(LC_ALL, "");

        if (!filename.empty()) {
            enable_file_logging(filename);
        }
    }

    Logger::~Logger() {
        if (log_file.is_open()) {
            log_file.close();
        }
    }

    void Logger::flush() {
        if (log_file.is_open()) {
            log_file.flush();
        }
    }

    void Logger::enable_file_logging(const string& filename) {
        log_file.open(filename, ios::app);
        if (log_file.is_open()) {
            file_logging_enabled = true;
            log_file << "\n" << string(60, '=') << "\n";
            log_file << "Сессия логирования начата: " << get_current_time() << "\n";
            log_file << string(60, '=') << "\n\n";
        } else {
            cerr << "Ошибка: Не удалось открыть файл логов: " << filename << endl;
            file_logging_enabled = false;
        }
    }

    void Logger::disable_file_logging() {
        if (log_file.is_open()) {
            log_file << "\n" << string(60, '=') << "\n";
            log_file << "Сессия логирования завершена: " << get_current_time() << "\n";
            log_file << string(60, '=') << "\n";
            log_file.close();
        }
        file_logging_enabled = false;
    }

    void Logger::enable_console_logging(bool enable) {
        console_logging_enabled = enable;
    }

    void Logger::set_level(LogLevel level) {
        current_level = level;
    }

    string Logger::level_to_string(LogLevel level) const {
        switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        default:                return "UNKNOWN";
        }
    }

    string Logger::get_current_time() const {
        time_t now = time(nullptr);
        struct tm timeinfo;

#ifdef _WIN32
        localtime_s(&timeinfo, &now);
#else
        localtime_r(&now, &timeinfo);
#endif

        char buffer[80];
        strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
        return string(buffer);
    }

    void Logger::write_to_file(LogLevel level, const string& message) {
        if (file_logging_enabled && log_file.is_open()) {
            log_file << "[" << get_current_time() << "]["
                << level_to_string(level) << "] "
                << message << endl;
        }
    }

    void Logger::write_to_console(LogLevel level, const string& message) {
        if (console_logging_enabled) {
            cout << "[" << get_current_time() << "]["
                << level_to_string(level) << "] "
                << message << endl;
        }
    }

    void Logger::log(LogLevel level, const string& message) {
        if (level < current_level) {
            return;
        }
        if (file_logging_enabled) {
            write_to_file(level, message);
        }
        if (console_logging_enabled) {
            write_to_console(level, message);
        }
    }

    void Logger::debug(const string& message) {
        log(LogLevel::DEBUG, message);
    }

    void Logger::info(const string& message) {
        log(LogLevel::INFO, message);
    }

    void Logger::warning(const string& message) {
        log(LogLevel::WARNING, message);
    }

    void Logger::error(const string& message) {
        log(LogLevel::ERROR, message);
    }

    void Logger::separator(char ch, int length) {
        string sep(length, ch);
        if (file_logging_enabled) {
            log_file << sep << endl;
        }
        cout << sep << endl;
    }

    void Logger::header(const string& title) {
        separator('=', 60);
        string centered_title = "  " + title;
        if (file_logging_enabled) {
            log_file << centered_title << endl;
        }
        cout << centered_title << endl;
        separator('=', 60);
    }
}
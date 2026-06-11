#include "scheduler/scheduler.h"
#include "utils/file_manager.h"
#include "utils/logger.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <thread>
#include <chrono>
#include <windows.h>
#include <direct.h>
#include <io.h>
using namespace std;

namespace rtos {
    RMScheduler::RMScheduler(const string& log_filename)
        : current_task(nullptr), system_time(0), tick_counter(0),
        missed_deadlines(0), context_switches(0), idle_time(0),
        logger(utils::LogLevel::INFO, log_filename, false),
        min_sched_time_us(LLONG_MAX), max_sched_time_us(0),
        total_sched_time_us(0), sched_calls(0) {
        logger.info("Планировщик RMS инициализирован");
    }

    bool RMScheduler::add_task(const string& name, uint32_t period,
        uint32_t wcet, uint32_t deadline, function<void()> job) {
        if (period == 0 || wcet == 0 || wcet > period) {
            logger.error("Некорректные временные параметры для задачи: " + name);
            return false;
        }
        if (name.empty() || name.length() > 50) {
            cerr << "Ошибка: Некорректное имя задачи.\n";
            logger.error("Некорректное имя задачи: " + name);
            return false;
        }
        if (period == 0 || wcet == 0 || wcet > period) {
            cerr << "Ошибка: Некорректные временные параметры.\n";
            logger.error("Некорректные временные параметры для задачи: " + name);
            return false;
        }
        if (deadline == 0 || deadline > period) {
            cerr << "Ошибка: Дедлайн должен быть в пределах (0, period].\n";
            logger.error("Некорректный дедлайн для задачи: " + name);
            return false;
        }
        for (const auto& task : tasks) {
            if (task->name == name) {
                cerr << "Ошибка: Задача с именем '" << name << "' уже существует.\n";
                logger.warning("Задача с именем '" + name + "' уже существует");
                return false;
            }
        }

        uint32_t task_id = static_cast<uint32_t>(tasks.size()) + 1;
        auto task = make_unique<Task>(task_id, name, period, wcet, deadline, job);

        task->reset();
        task->next_release = 0;
        task->start_time = 0;

        tasks.push_back(move(task));

        calculate_priorities();
        logger.info("Задача добавлена: " + name +
            " (P=" + to_string(period) +
            ", WCET=" + to_string(wcet) +
            ", D=" + to_string(deadline) + ")");
        return true;
    }

    void RMScheduler::calculate_priorities() {
        sort(tasks.begin(), tasks.end(),
            [](const unique_ptr<Task>& a, const unique_ptr<Task>& b) {
                return a->period < b->period;
            });
        for (size_t i = 0; i < tasks.size(); ++i) {
            tasks[i]->priority = static_cast<uint32_t>(i + 1);
        }
        logger.debug("Приоритеты RMS пересчитаны, задач: " + to_string(tasks.size()));
    }

    bool RMScheduler::is_schedulable() const {
        double utilization = calculate_cpu_utilization();
        size_t n = tasks.size();
        if (n == 0) {
            logger.debug("Нет задач - система планируема");
            return true;
        }
        if (utilization > 1.0) {
            logger.debug("Непланируемо: утилизация > 100% (" +
                to_string(utilization * 100) + "%)");
            return false;
        }

        double bound = n * (pow(2.0, 1.0 / n) - 1.0);
        logger.debug("Проверка Liu & Layland: U=" +
            to_string(utilization) +
            ", U_lub=" + to_string(bound) +
            ", n=" + to_string(n));
        return utilization <= bound;

        if (utilization <= bound) {
            logger.info("ГАРАНТИРОВАННО планируемо по Liu & Layland");
            return true;
        }
        else if (utilization <= 1.0) {
            logger.warning("МОЖЕТ БЫТЬ планируемо (U > U_lub но < или = 100%)");
            return false; // или true, если хотите более либеральную проверку
        }
        else {
            logger.error("НЕВОЗМОЖНО планируемо (U > 100%)");
            return false;
        }
    }

    double RMScheduler::calculate_cpu_utilization() const {
        double utilization = 0.0;
        for (const auto& task : tasks) {
            utilization += static_cast<double>(task->wcet) / task->period;
        }
        return utilization;
    }

    void RMScheduler::update_ready_queue() {
        ready_queue.clear();
        for (const auto& task : tasks) {
            if (system_time >= task->next_release) {
                task->reset();
                task->start_time = system_time;
                task->state = TaskState::READY;
                logger.debug("Задача активирована: " + task->name +
                    " (время: " + to_string(system_time) + ")");

                task->next_release += task->period;
                while (task->next_release <= system_time) {
                    task->next_release += task->period;
                }
            }

            if (task->state == TaskState::READY || task->state == TaskState::RUNNING) {
                ready_queue.push(task.get());
            }
        }
    }

    void RMScheduler::check_deadlines() {
        for (const auto& task : tasks) {
            if (task->state == TaskState::RUNNING || task->state == TaskState::READY) {
                uint32_t elapsed = system_time - task->start_time;
                if (elapsed > task->deadline) {
                    missed_deadlines++;
                    task->missed_count++;

                    task->state = TaskState::COMPLETED;

                    if (current_task == task.get()) {
                        current_task = nullptr;
                    }
                    logger.error("Пропущен дедлайн задачи: " + task->name +
                        " (прошло: " + to_string(elapsed) +
                        "мс, дедлайн: " + to_string(task->deadline) +
                        "мс). Задача принудительно завершена.");
                }
            }
        }
    }

    void RMScheduler::tick() {
        auto start = chrono::high_resolution_clock::now();

        update_ready_queue();

        check_deadlines();

        Task* next_task = ready_queue.peek();
        bool context_switched = false;

        if (next_task && (!current_task || next_task->priority < current_task->priority)) {
            if (current_task && current_task->state == TaskState::RUNNING) {
                current_task->state = TaskState::READY;
                logger.debug("Вытеснение задачи: " + current_task->name +
                    " -> " + next_task->name);
            }
            current_task = next_task;
            ready_queue.pop();
            context_switches++;
            context_switched = true;
            logger.debug("Переключение контекста на задачу: " + current_task->name);
        }
        else if (!current_task && next_task) {
            current_task = next_task;
            ready_queue.pop();
            context_switches++;
            context_switched = true;
            logger.debug("Начало выполнения задачи: " + current_task->name);
        }
        if (context_switched) {
            idle_time += CONTEXT_SWITCH_OVERHEAD_MS;
            logger.debug("Учтено время переключения контекста: " + to_string(CONTEXT_SWITCH_OVERHEAD_MS) + "мс");
        }

        auto end = chrono::high_resolution_clock::now();
        long long elapsed_us = chrono::duration_cast<chrono::microseconds>(end - start).count();
        if (elapsed_us < min_sched_time_us) min_sched_time_us = elapsed_us;
        if (elapsed_us > max_sched_time_us) max_sched_time_us = elapsed_us;
        total_sched_time_us += elapsed_us;
        sched_calls++;

        if (current_task) {
            current_task->execute();
            logger.debug("Выполняется: " + current_task->name +
                ", осталось: " + to_string(current_task->remaining_time) + "мс");
            if (current_task->state == TaskState::COMPLETED) {
                logger.debug("Задача завершена: " + current_task->name);
                current_task = nullptr;
            }
        }
        else {
            idle_time++;
            logger.debug("Простой системы (IDLE), общее время простоя: " +
                to_string(idle_time) + "мс");
        }

        tick_counter++;
        system_time++;
        logger.debug("Тик #" + to_string(system_time) +
            " (всего тиков: " + to_string(tick_counter) + ")");
    }

    bool RMScheduler::save_to_file(const string& filename) {
        logger.info("Попытка сохранения задач в файл: " + filename);
        if (filename.empty() || filename.length() > 255) {
            cerr << "Ошибка: Некорректное имя файла.\n";
            logger.error("Некорректное имя файла для сохранения");
            return false;
        }
        if (filename.find('.') == string::npos) {
            cerr << "Предупреждение: Файл без расширения.\n";
            logger.warning("Файл без расширения: " + filename);
        }
        if (tasks.empty()) {
            cerr << "Ошибка: Нет задач для сохранения.\n";
            logger.warning("Нет задач для сохранения");
            return false;
        }
        string data = serialize_tasks();
        vector<string> task_data = { data };
        bool success = utils::FileManager::save_tasks(filename, task_data);
        if (!success) {
            cerr << "Ошибка: Не удалось сохранить файл '" << filename << "'.\n";
            logger.error("Не удалось сохранить файл: " + filename);
        }
        else {
            logger.info("Задачи успешно сохранены в файл: " + filename);
        }
        return success;
    }

    bool RMScheduler::load_from_file(const string& filename) {
        logger.info("Попытка загрузки задач из файла: " + filename);

        if (filename.length() > 255) {
            logger.error("Слишком длинное имя файла: " + filename);
            return false;
        }
        if (!utils::FileManager::file_exists(filename)) {
            logger.error("Файл не существует: " + filename);
            return false;
        }
        vector<string> task_data;
        if (!utils::FileManager::load_tasks(filename, task_data)) {
            logger.error("Не удалось загрузить файл: " + filename);
            return false;
        }
        if (task_data.empty()) {
            logger.warning("Файл пуст: " + filename);
            return false;
        }
        bool result = deserialize_tasks(task_data[0]);
        if (result) {
            logger.info("Задачи успешно загружены из файла: " + filename);
            logger.info("Загружено задач: " + to_string(tasks.size()));
        }
        else {
            logger.error("Ошибка при десериализации задач из файла: " + filename);
        }
        return result;
    }

    string RMScheduler::serialize_tasks() const {
        stringstream ss;
        ss << tasks.size() << ";";
        for (const auto& task : tasks) {
            ss << task->id << ","
                << task->name << ","
                << task->period << ","
                << task->wcet << ","
                << task->deadline << ";";
        }
        return ss.str();
    }

    bool RMScheduler::deserialize_tasks(const string& data) {
        tasks.clear();
        ready_queue.clear();
        current_task = nullptr;
        logger.debug("Начало десериализации задач, данные: " +
            to_string(data.length()) + " символов");

        stringstream ss(data);
        string token;
        if (!getline(ss, token, ';')) {
            logger.error("Ошибка чтения количества задач");
            return false;
        }

        int task_count = stoi(token);
        logger.debug("Количество задач для загрузки: " + to_string(task_count));
        for (int i = 0; i < task_count; ++i) {
            if (!getline(ss, token, ';')) {
                logger.error("Ошибка чтения данных задачи #" + to_string(i + 1));
                return false;
            }
            stringstream task_ss(token);
            vector<string> fields;
            string field;

            while (getline(task_ss, field, ',')) {
                fields.push_back(field);
            }
            if (fields.size() != 5) {
                logger.error("Некорректное количество полей в задаче #" + to_string(i + 1));
                return false;
            }

            uint32_t id = stoi(fields[0]);
            string name = fields[1];
            uint32_t period = stoi(fields[2]);
            uint32_t wcet = stoi(fields[3]);
            uint32_t deadline = stoi(fields[4]);
            auto job = [name]() {
                cout << "  [" << name << "] Выполняется...\n";
                };
            auto task = make_unique<Task>(id, name, period, wcet, deadline, job);
            task->reset();
            task->next_release = 0;

            tasks.push_back(move(task));
            logger.debug("Задача загружена: " + name + " (ID: " + to_string(id) + ")");
        }
        calculate_priorities();
        return true;
    }

    Task* RMScheduler::get_current_task() const {
        return current_task;
    }

    bool create_logs_directory() {
        return _mkdir("logs") == 0 || GetLastError() == ERROR_ALREADY_EXISTS;
    }

    bool file_exists(const string& filename) {
        return _access(filename.c_str(), 0) == 0;
    }

    void RMScheduler::reset_simulation() {
        min_sched_time_us = LLONG_MAX;
        max_sched_time_us = 0;
        total_sched_time_us = 0;
        sched_calls = 0;
        for (const auto& task : tasks) {
            task->reset();
            task->next_release = 0;
            task->start_time = 0;
            task->state = TaskState::READY;
            task->missed_count = 0;
        }
        ready_queue.clear();
        current_task = nullptr;
        system_time = 0;
        tick_counter = 0;
        missed_deadlines = 0;
        context_switches = 0;
        idle_time = 0;
        logger.info("Симуляция сброшена для нового запуска");
    }

    bool RMScheduler::save_configuration(const string& config_name, const string& description) {
        if (config_name.empty()) {
            logger.error("Имя конфигурации не может быть пустым");
            return false;
        }
        if (tasks.empty()) {
            logger.warning("Нет задач для сохранения в конфигурации");
            return false;
        }

        create_logs_directory();

        string filename = "logs/task_config_" + config_name + ".txt";
        try {
            ofstream file(filename);
            if (!file.is_open()) {
                logger.error("Не удалось создать файл конфигурации: " + filename);
                return false;
            }

            file << "# Конфигурация задач: " << config_name << "\n";
            file << "# Описание: " << description << "\n";
            file << "# Создано: " << __DATE__ << " " << __TIME__ << "\n";
            file << "# Формат: Имя;Период;WCET;Дедлайн\n";
            file << "# Количество задач: " << tasks.size() << "\n\n";

            for (const auto& task : tasks) {
                file << task->name << ";"
                    << task->period << ";"
                    << task->wcet << ";"
                    << task->deadline << "\n";
            }
            file.close();

            logger.info("Конфигурация сохранена: " + config_name);
            logger.info("Файл: " + filename + ", задач: " + to_string(tasks.size()));
            return true;
        }
        catch (const exception& e) {
            logger.error("Ошибка при сохранении конфигурации: " + string(e.what()));
            return false;
        }
    }

    bool RMScheduler::load_configuration(const string& config_name) {
        if (config_name.empty()) {
            logger.error("Имя конфигурации не может быть пустым");
            return false;
        }

        string filename = "logs/task_config_" + config_name + ".txt";
        if (!file_exists(filename)) {
            logger.error("Файл конфигурации не найден: " + filename);
            return false;
        }
        try {
            ifstream file(filename);
            if (!file.is_open()) {
                logger.error("Не удалось открыть файл конфигурации: " + filename);
                return false;
            }
            reset();

            string line;
            int line_num = 0;
            int tasks_loaded = 0;
            while (getline(file, line)) {
                line_num++;
                if (line.empty() || line[0] == '#') {
                    continue;
                }

                stringstream ss(line);
                string name, period_str, wcet_str, deadline_str;
                if (!getline(ss, name, ';') ||
                    !getline(ss, period_str, ';') ||
                    !getline(ss, wcet_str, ';') ||
                    !getline(ss, deadline_str, ';')) {
                    logger.warning("Некорректный формат строки " + to_string(line_num) + " в файле " + filename);
                    continue;
                }
                try {
                    uint32_t period = stoul(period_str);
                    uint32_t wcet = stoul(wcet_str);
                    uint32_t deadline = stoul(deadline_str);

                    auto job = [name]() {
                        cout << "  [" << name << "] Выполняется...\n";
                        };
                    if (add_task(name, period, wcet, deadline, job)) {
                        tasks_loaded++;
                    }
                    else {
                        logger.warning("Не удалось добавить задачу: " + name);
                    }
                }
                catch (const exception& e) {
                    logger.warning("Ошибка парсинга строки " + to_string(line_num) + ": " + string(e.what()));
                }
            }

            file.close();

            if (tasks_loaded > 0) {
                logger.info("Конфигурация загружена: " + config_name);
                logger.info("Загружено задач: " + to_string(tasks_loaded));
                return true;
            }
            else {
                logger.error("Не удалось загрузить ни одной задачи из конфигурации: " + config_name);
                return false;
            }
        }
        catch (const exception& e) {
            logger.error("Ошибка при загрузке конфигурации: " + string(e.what()));
            reset();
            return false;
        }
    }

    vector<string> RMScheduler::get_available_configurations() const {
        vector<string> configs;

        create_logs_directory();

        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA("logs\\task_config_*.txt", &findData);

        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                string filename = findData.cFileName;
                if (filename.length() > 16) {
                    string config_name = filename.substr(12, filename.length() - 16);
                    if (!config_name.empty()) {
                        configs.push_back(config_name);
                    }
                }
            } while (FindNextFileA(hFind, &findData) != 0);
            FindClose(hFind);
        }

        return configs;
    }

    bool RMScheduler::delete_configuration(const string& config_name) {
        if (config_name.empty()) {
            logger.error("Имя конфигурации не может быть пустым");
            return false;
        }
        string filename = "logs/task_config_" + config_name + ".txt";
        try {
            if (!file_exists(filename)) {
                logger.error("Конфигурация не найдена: " + config_name);
                return false;
            }
            if (remove(filename.c_str()) == 0) {
                logger.info("Конфигурация удалена: " + config_name);
                return true;
            }
            else {
                logger.error("Не удалось удалить конфигурацию: " + config_name);
                return false;
            }
        }
        catch (const exception& e) {
            logger.error("Ошибка при удалении конфигурации: " + string(e.what()));
            return false;
        }
    }

    void RMScheduler::simulate(uint32_t ticks) {
        cout << "\n" << string(85, '=') << "\n";
        cout << "  НАЧАЛО СИМУЛЯЦИИ RMS ПЛАНИРОВЩИКА\n";
        cout << string(85, '=') << "\n\n";
        logger.header("НАЧАЛО СИМУЛЯЦИИ RMS ПЛАНИРОВЩИКА");
        logger.info("Длительность симуляции: " + to_string(ticks) + " тиков");
        logger.info("Количество задач: " + to_string(tasks.size()));
        reset_simulation();
        cout << left
            << setw(10) << "Время(мс)"
            << setw(25) << "Выполняется"
            << setw(15) << "Состояние"
            << setw(15) << "Осталось(мс)"
            << setw(50) << "Активированы"
            << "\n";
        cout << string(100, '-') << "\n";
        for (uint32_t i = 0; i < ticks; ++i) {
            tick();
            string activated_tasks;
            for (const auto& task : tasks) {
                uint32_t check_time = system_time - 1;
                if (check_time % task->period == 0) {
                    if (!activated_tasks.empty()) activated_tasks += ", ";
                    activated_tasks += task->name;
                }
            }

            uint32_t current_time = system_time - 1;
            cout << left << setw(10) << "[" + to_string(current_time) + "]";

            if (current_task) {
                cout << setw(25) << current_task->name;

                string state_str;

                if (current_task->remaining_time == current_task->wcet) {
                    state_str = "Начало";
                }
                else if (current_task->remaining_time == 0) {
                    state_str = "Завершена";
                }
                else if (current_task->remaining_time == 1) {
                    state_str = "Завершение";
                }
                else {
                    state_str = "Выполняется";
                }
                cout << setw(15) << state_str;

                if (current_task->remaining_time > 0) {
                    cout << setw(15) << "[" + to_string(current_task->remaining_time) + "]";
                }
                else {
                    cout << setw(15) << "-";
                }
            }
            else {
                cout << setw(25) << "IDLE (простой)"
                    << setw(15) << "Простой"
                    << setw(15) << "-";
            }
            const int MAX_ACTIVATED_LENGTH = 50;
            string activated_display = activated_tasks;
            if (activated_display.length() > MAX_ACTIVATED_LENGTH) {
                activated_display = activated_display.substr(0, MAX_ACTIVATED_LENGTH - 3) + "...";
            }
            cout << setw(50) << (activated_tasks.empty() ? "-" : activated_display);
            cout << "\n";

            this_thread::sleep_for(chrono::milliseconds(10));
        }
        cout << "\n" << string(85, '=') << "\n";
        cout << "   СИМУЛЯЦИЯ ЗАВЕРШЕНА\n";
        cout << string(85, '=') << "\n";
        logger.info("=== СИМУЛЯЦИЯ ЗАВЕРШЕНА ===");
    }

    void RMScheduler::print_statistics() const {
        cout << "\n";
        cout << "========================================\n";
        cout << "СТАТИСТИКА ВЫПОЛНЕНИЯ:\n";
        cout << "========================================\n";
        cout << "  Общее время симуляции: " << system_time << "мс\n";
        cout << "  Количество переключений контекста: " << context_switches << "\n";
        cout << "  Пропущенные дедлайны: " << missed_deadlines << "\n";
        if (system_time > 0) {
            cout << "  Время простоя: " << idle_time << "мс ("
                << fixed << setprecision(1)
                << (static_cast<double>(idle_time) / system_time * 100)
                << "%)\n";
        }
        else {
            cout << "  Время простоя: " << idle_time << "мс (0.0%)\n";
        }

        cout << "----------------------------------------\n";

        if (!tasks.empty()) {
            double utilization = calculate_cpu_utilization();
            size_t n = tasks.size();
            double ll_bound = n * (pow(2.0, 1.0 / n) - 1.0);

            cout << "АНАЛИЗ ПЛАНИРУЕМОСТИ (Liu & Layland):\n";
            cout << "  Общая утилизация CPU: "
                << fixed << setprecision(2) << (utilization * 100) << "%\n";
            cout << "  Граница для " << n << " задач: "
                << fixed << setprecision(2) << (ll_bound * 100) << "%\n";

            if (utilization <= ll_bound) {
                cout << "  [OK] ГАРАНТИРОВАННО планируемо\n";
            }
            else if (utilization <= 1.0) {
                cout << "  [WARNING] МОЖЕТ БЫТЬ планируемо (U > границы)\n";
            }
            else {
                cout << "  [ERROR] НЕПЛАНИРУЕМО (утилизация > 100%)\n";
            }
            cout << "----------------------------------------\n";
        }

        if (!tasks.empty()) {
            cout << "СТАТИСТИКА ПО ЗАДАЧАМ:\n";
            for (const auto& task : tasks) {
                cout << " " << task->name << ":\n";
                cout << "   Пропущено дедлайнов: " << task->missed_count << "\n";
                uint32_t releases = system_time / task->period + 1;
                cout << "   Запусков (приблизительно): " << releases << "\n";
                cout << "   Утилизация: " << fixed << setprecision(1)
                    << (static_cast<double>(task->wcet) / task->period * 100) << "%\n";
            }
        }

        if (sched_calls > 0) {
            cout << "----------------------------------------\n";
            cout << "ВРЕМЯ ПЛАНИРОВЩИКА (микросекунды):\n";
            cout << "  Минимальное: " << min_sched_time_us << " мкс\n";
            cout << "  Максимальное: " << max_sched_time_us << " мкс\n";
            cout << "  Среднее: " << fixed << setprecision(1)
                << (static_cast<double>(total_sched_time_us) / sched_calls) << " мкс\n";
            cout << "  Замеров: " << sched_calls << "\n";
            logger.info("Статистика планировщика: min=" + to_string(min_sched_time_us) +
                "us, max=" + to_string(max_sched_time_us) + "us, avg=" +
                to_string(static_cast<double>(total_sched_time_us) / sched_calls) + "us");
        }

        double utilization = calculate_cpu_utilization();
        cout << "\n  Общая утилизация CPU: "
            << fixed << setprecision(1) << (utilization * 100) << "%\n";

        cout << "========================================\n";

        logger.info("СТАТИСТИКА ВЫПОЛНЕНИЯ");
        logger.info("Общее время симуляции: " + to_string(system_time) + "мс");
        logger.info("Переключения контекста: " + to_string(context_switches));
        logger.info("Пропущенные дедлайны: " + to_string(missed_deadlines));
        if (!tasks.empty()) {
            double util = calculate_cpu_utilization();
            size_t n = tasks.size();
            double bound = n * (pow(2.0, 1.0 / n) - 1.0);
            logger.info("Утилизация CPU: " + to_string(util * 100) + "%");
            logger.info("Граница Liu & Layland: " + to_string(bound * 100) + "%");
            if (util <= bound) {
                logger.info("Гарантированно планируемо по Liu & Layland");
            }
            else if (util <= 1.0) {
                logger.warning("Утилизация превышает границу Liu & Layland");
            }
            else {
                logger.error("Утилизация превышает 100% - система непланируема");
            }
        }
        if (missed_deadlines > 0) {
            logger.error("Обнаружены пропущенные дедлайны!");
        }
        if (system_time > 0) {
            double idle_percent = static_cast<double>(idle_time) / system_time * 100;
            logger.info("Время простоя: " + to_string(idle_time) + "мс (" +
                to_string(idle_percent) + "%)");
        }
        logger.info("Общая утилизация CPU: " + to_string(utilization * 100) + "%");
    }

    void RMScheduler::print_task_table() const {
        cout << "\n";
        cout << "================================================================================\n";
        cout << "ID  ИМЯ                ПЕРИОД   WCET    ДЕДЛАЙН ПРИОРИТЕТ СОСТОЯНИЕ   ОСТАЛОСЬ  \n";
        cout << "--------------------------------------------------------------------------------\n";
        for (const auto& task : tasks) {
            cout << left
                << setw(3) << task->id
                << setw(20) << task->name
                << setw(8) << task->period
                << setw(8) << task->wcet
                << setw(9) << task->deadline
                << setw(10) << task->priority;

            string state_str;
            switch (task->state) {
            case TaskState::READY:     state_str = "ГОТОВ"; break;
            case TaskState::RUNNING:   state_str = "ВЫПОЛНЯЕТСЯ"; break;
            case TaskState::BLOCKED:   state_str = "БЛОКИРОВАНА"; break;
            case TaskState::SUSPENDED: state_str = "ПРИОСТАНОВЛЕНА"; break;
            case TaskState::COMPLETED: state_str = "ЗАВЕРШЕНА"; break;
            default:                   state_str = "НЕИЗВЕСТНО";
            }
            cout << setw(12) << state_str
                << setw(8) << task->remaining_time << "\n";
        }
        cout << "================================================================================\n";
        logger.info("Таблица задач выведена, всего задач: " + to_string(tasks.size()));
    }

    void RMScheduler::print_ready_queue() const {
        cout << "\nОчередь готовых задач:\n";

        ReadyQueue temp_queue = ready_queue;
        int count = 1;
        while (!temp_queue.empty()) {
            Task* task = temp_queue.pop();
            cout << "  " << count++ << ". " << task->name
                << " (Приоритет: " << task->priority << ")\n";
        }
        if (count == 1) {
            cout << "  Очередь пуста\n";
        }
        logger.info("Очередь готовых задач выведена, задач в очереди: " + to_string(count - 1));
    }

    void RMScheduler::reset() {
        logger.info("Сброс планировщика");
        tasks.clear();
        ready_queue.clear();
        current_task = nullptr;
        system_time = 0;
        tick_counter = 0;
        missed_deadlines = 0;
        context_switches = 0;
        idle_time = 0;
        logger.debug("Все счетчики и очереди сброшены");
    }

    void RMScheduler::export_to_csv(const string& filename) const {
        ofstream file(filename);
        if (!file.is_open()) {
            logger.error("Не удалось открыть файл для экспорта CSV: " + filename);
            return;
        }

        file << "Параметр;Значение\n";
        file << "Общее время симуляции (мс);" << system_time << "\n";
        file << "Переключения контекста;" << context_switches << "\n";
        file << "Пропущенные дедлайны;" << missed_deadlines << "\n";
        file << "Время простоя (мс);" << idle_time << "\n";
        if (system_time > 0) {
            file << "Процент простоя;" << fixed << setprecision(1) << (static_cast<double>(idle_time) / system_time * 100) << "\n";
        }
        file << "\n";

        file << "ID;Имя;Период (мс);WCET (мс);Дедлайн (мс);Приоритет;Состояние;Осталось (мс);Пропусков\n";
        for (const auto& task : tasks) {
            string state_str;
            switch (task->state) {
            case TaskState::READY: state_str = "ГОТОВ"; break;
            case TaskState::RUNNING: state_str = "ВЫПОЛНЯЕТСЯ"; break;
            case TaskState::BLOCKED: state_str = "БЛОКИРОВАНА"; break;
            case TaskState::SUSPENDED: state_str = "ПРИОСТАНОВЛЕНА"; break;
            case TaskState::COMPLETED: state_str = "ЗАВЕРШЕНА"; break;
            default: state_str = "НЕИЗВЕСТНО";
            }
            file << task->id << ";"
                << task->name << ";"
                << task->period << ";"
                << task->wcet << ";"
                << task->deadline << ";"
                << task->priority << ";"
                << state_str << ";"
                << task->remaining_time << ";"
                << task->missed_count << "\n";
        }
        file.close();
        logger.info("Экспорт CSV завершён: " + filename);
    }
}
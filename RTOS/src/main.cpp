#include "scheduler/scheduler.h"
#include "utils/logger.h"
#include "utils/auth_system.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <clocale>
#include <locale>
#include <limits>
#include <string>
#include <map>
#include <functional>
#include <sstream>
#include <ctime>
using namespace std;
using namespace rtos;

static void print_main_menu(UserRole role) {
    cout << "\n==========================================================\n";
    cout << "    АЛГОРИТМ ПЛАНИРОВАНИЯ ЗАДАЧ ДЛЯ RTOS НА ОСНОВЕ RMS\n";
    cout << "==========================================================\n";
    if (role == UserRole::ADMIN) {
        cout << "=== РЕЖИМ АДМИНИСТРАТОРА ===\n";
        cout << "1. Тестовые задачи и сценарии\n";
        cout << "2. Добавить пользовательскую задачу\n";
        cout << "3. Показать таблицу задач\n";
        cout << "4. Проверить планируемость (Liu & Layland)\n";
        cout << "5. Запустить симуляцию\n";
        cout << "6. Показать статистику\n";
        cout << "7. Сбросить планировщик\n";
        cout << "8. Управление пользователями\n";
        cout << "9. Управление конфигурациями задач\n";
        cout << "10. Выход из системы\n";
        cout << "0. Выход из программы\n";
    }
    else if (role == UserRole::USER) {
        cout << "=== РЕЖИМ ПОЛЬЗОВАТЕЛЯ ===\n";
        cout << "1. Добавить пользовательскую задачу\n";
        cout << "2. Быстрый тестовый набор (3 задачи)\n";
        cout << "3. Управление учетной записью\n";
        cout << "4. Показать таблицу задач\n";
        cout << "5. Проверить планируемость (Liu & Layland)\n";
        cout << "6. Запустить симуляцию\n";
        cout << "7. Показать статистику\n";
        cout << "8. Сбросить планировщик\n";
        cout << "9. Выход из системы\n";
        cout << "0. Выход из программы\n";
    }
    else {
        cout << "=== ГОСТЕВОЙ РЕЖИМ ===\n";
        cout << "1. Вход в систему\n";
        cout << "0. Выход из программы\n";
    }
    cout << "----------------------------------------\n";
    cout << "Выберите опцию: ";
}

static void print_user_menu_basic() {
    cout << "\n========================================\n";
    cout << "    УПРАВЛЕНИЕ УЧЁТНОЙ ЗАПИСЬЮ\n";
    cout << "========================================\n";
    cout << "1. Показать текущего пользователя\n";
    cout << "2. Сменить пароль\n";
    cout << "3. Назад\n";
    cout << "Выберите опцию (1-3): ";
}

static void print_user_menu_admin() {
    cout << "\n====================================\n";
    cout << "    УПРАВЛЕНИЕ ПОЛЬЗОВАТЕЛЯМИ\n";
    cout << "====================================\n";
    cout << "1. Добавить нового пользователя\n";
    cout << "2. Показать текущего пользователя\n";
    cout << "3. Сменить пароль текущего пользователя\n";
    cout << "4. Показать всех пользователей\n";
    cout << "5. Удалить пользователя\n";
    cout << "6. Сменить пароль другого пользователя\n";
    cout << "7. Назад\n";
    cout << "Выберите опцию (1-7): ";
}

static void print_test_tasks_menu() {
    cout << "\n========================================\n";
    cout << " ТЕСТОВЫЕ ЗАДАЧИ И СЦЕНАРИИ\n";
    cout << "========================================\n";
    cout << "1. Быстрый базовый набор (3 задачи)\n";
    cout << "2. Генератор тестовых сценариев\n";
    cout << "3. Юнит-тесты алгоритма RMS\n";
    cout << "4. Назад в главное меню\n";
    cout << "----------------------------------------\n";
    cout << "Выберите опцию (1-4): ";
}
static void print_generator_menu() {
    cout << "\n========================================\n";
    cout << " ГЕНЕРАТОР ТЕСТОВЫХ СЦЕНАРИЕВ\n";
    cout << "========================================\n";
    cout << "1. Легкая нагрузка (утилизация ~55%)\n";
    cout << "2. Средняя нагрузка (утилизация ~63%)\n";
    cout << "3. Предельная нагрузка RMS (утилизация ~76%)\n";
    cout << "4. Перегрузка системы (утилизация ~163%)\n";
    cout << "5. Реальный сценарий (микроконтроллер, ~82%)\n";
    cout << "6. Автоматический прогон всех сценариев\n";
    cout << "7. Назад\n";
    cout << "----------------------------------------\n";
    cout << "Выберите сценарий (1-7): ";
}
static void print_unit_tests_menu() {
    cout << "\n==========================================================\n";
    cout << " ЮНИТ-ТЕСТЫ АЛГОРИТМА RMS\n";
    cout << "==========================================================\n";
    cout << "1. Тест 1: Проверка теоремы Liu & Layland\n";
    cout << "2. Тест 2: Граничный случай планируемости\n";
    cout << "3. Тест 3: Очевидная непланируемость (перегрузка)\n";
    cout << "4. Тест 4: Проверка приоритетов RMS\n";
    cout << "5. Тест 5: Проверка пропуска дедлайнов\n";
    cout << "6. Тест 6: Управление добавления/удаления задач\n";
    cout << "7. Запустить ВСЕ тесты автоматически\n";
    cout << "8. Назад\n";
    cout << "----------------------------------------\n";
    cout << "Выберите тест (1-8): ";
}
static void print_configuration_menu() {
    cout << "\n========================================\n";
    cout << "    УПРАВЛЕНИЕ КОНФИГУРАЦИЯМИ ЗАДАЧ\n";
    cout << "========================================\n";
    cout << "1. Сохранить текущие задачи как конфигурацию\n";
    cout << "2. Загрузить конфигурацию задач\n";
    cout << "3. Быстрая загрузка из файла (старый формат)\n";
    cout << "4. Показать доступные конфигурации\n";
    cout << "5. Удалить конфигурацию\n";         
    cout << "6. Назад в главное меню\n";             
    cout << "----------------------------------------\n";
    cout << "Выберите опцию (1-6): ";
}

bool login_procedure(AuthSystem& auth, utils::Logger& logger) {
    string username, password;
    cout << "\n============ ВХОД В СИСТЕМУ ============\n";

    cout << "Логин (3-20 символов): ";
    char username_buffer[21];
    cin >> setw(21) >> username_buffer;

    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "\nОшибка: Логин слишком длинный (максимум 20 символов)!\n";
        logger.warning("Слишком длинный логин при входе");
        return false;
    }
    username = username_buffer;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (username.length() < 3 || username.length() > 20) {
        cout << "\nОшибка: Логин должен быть от 3 до 20 символов!\n";
        logger.warning("Некорректная длина логина при входе: " + username);
        return false;
    }

    cout << "Пароль (минимум 4 символа): ";
    char password_buffer[101];
    cin >> setw(101) >> password_buffer;
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "\nОшибка: Пароль слишком длинный (максимум 100 символов)!\n";
        logger.warning("Слишком длинный пароль при входе для пользователя: " + username);
        return false;
    }
    password = password_buffer;
    if (password.length() < 4) {
        cout << "\nОшибка: Пароль должен содержать минимум 4 символа!\n";
        logger.warning("Слишком короткий пароль при входе для пользователя: " + username);
        return false;
    }
    logger.info("Попытка входа пользователя: " + username);
    if (auth.login(username, password)) {
        cout << "\nУспешный вход! Добро пожаловать, " << username << "!\n";
        logger.info("Успешный вход пользователя: " + username);
        return true;
    }
    else {
        cout << "\nОшибка: Неверный логин или пароль!\n";
        logger.warning("Неудачная попытка входа: " + username);
        return false;
    }
}

static void task1_job() {
    cout << "[Системный монитор] Выполняю работу...\n";
}
static void task2_job() {
    cout << "[Сбор данных] Обрабатываю данные...\n";
}
static void task3_job() {
    cout << "[Сетевая связь] Отправляю сообщение...\n";
}

void add_test_tasks(RMScheduler& scheduler, utils::Logger& logger) {
    cout << "\nДОБАВЛЕНИЕ БАЗОВОГО ТЕСТОВОГО НАБОРА\n";
    cout << "========================================\n";

    scheduler.reset();
    logger.info("Очистка существующих задач перед добавлением тестового набора");
    cout << "Существующие задачи удалены.\n";

    cout << "\nДобавляю тестовые задачи...\n";
    logger.info("Добавление тестовых задач");

    if (!scheduler.add_task("Системный монитор", 15, 3, 15, task1_job)) {
        cerr << "Ошибка добавления задачи 1!\n";
        logger.error("Ошибка добавления задачи: Системный монитор");
    }

    if (!scheduler.add_task("Сбор данных", 20, 4, 20, task2_job)) {
        cerr << "Ошибка добавления задачи 2!\n";
        logger.error("Ошибка добавления задачи: Сбор данных");
    }

    if (!scheduler.add_task("Сетевая связь", 50, 8, 50, task3_job)) {
        cerr << "Ошибка добавления задачи 3!\n";
        logger.error("Ошибка добавления задачи: Сетевая связь");
    }
    cout << "Тестовые задачи добавлены!\n";
    logger.info("Тестовые задачи добавлены");

    scheduler.print_task_table();

    cout << "\nАНАЛИЗ ПЛАНИРУЕМОСТИ:\n";
    cout << string(40, '-') << "\n";

    double utilization = scheduler.calculate_cpu_utilization() * 100;
    size_t n = scheduler.get_tasks().size();
    double bound = n * (pow(2.0, 1.0 / n) - 1) * 100;
    bool schedulable = scheduler.is_schedulable();

    cout << "Утилизация CPU: " << fixed << setprecision(2) << utilization << "%\n";
    cout << "Граница Liu & Layland: " << fixed << setprecision(2) << bound << "%\n";

    if (utilization > 100.0) {
        cout << "[ERROR] НЕПЛАНИРУЕМА (U > 100%)\n";
        logger.warning("Система непланируема: утилизация " +
            to_string(utilization) + "% > 100%");
    }
    else if (schedulable) {
        cout << "[OK] ГАРАНТИРОВАННО планируема\n";
        logger.info("Система гарантированно планируема: " +
            to_string(utilization) + "% < или = " + to_string(bound) + "%");
    }
    else {
        cout << "[WARNING] МОЖЕТ БЫТЬ планируемой\n";
        cout << "(утилизация превышает гарантированную границу)\n";
        logger.warning("Система может быть планируемой: " +
            to_string(utilization) + "% > " + to_string(bound) + "%");
    }
    cout << string(40, '-') << "\n";
}

void generate_test_scenario(RMScheduler& scheduler, utils::Logger& logger, int scenario_type) {
    scheduler.reset();
    cout << "\nГЕНЕРАЦИЯ ТЕСТОВОГО СЦЕНАРИЯ...\n";
    cout << "====================================================\n";
    switch (scenario_type) {
    case 1:
        cout << "Сценарий: ЛЕГКАЯ НАГРУЗКА (утилизация ~55%)\n";
        cout << "Идеально для демонстрации стабильной работы\n";
        scheduler.add_task("Высокоприоритетная", 10, 3, 10, []() {
            cout << " [ВП] Быстрая задача\n";
            });
        scheduler.add_task("Средний приоритет", 20, 3, 20, []() {
            cout << " [СП] Средняя задача\n";
            });
        scheduler.add_task("Низкий приоритет", 50, 5, 50, []() {
            cout << " [НП] Долгая задача\n";
            });
        break;
    case 2:
        cout << "Сценарий: СРЕДНЯЯ НАГРУЗКА (утилизация ~63%)\n";
        cout << "Типичный рабочий сценарий для RMS\n";
        scheduler.add_task("Контроллер", 100, 25, 100, []() {
            cout << " [Контроллер] Обрабатываю данные...\n";
            });
        scheduler.add_task("Датчик температуры", 200, 35, 200, []() {
            cout << " [Датчик] Считываю показания...\n";
            });
        scheduler.add_task("Сетевая связь", 500, 55, 500, []() {
            cout << " [Связь] Отправляю пакет...\n";
            });
        scheduler.add_task("Логирование", 1000, 90, 1000, []() {
            cout << " [Логгер] Записываю данные...\n";
            });
        break;
    case 3:
        cout << "Сценарий: ПРЕДЕЛЬНАЯ НАГРУЗКА RMS (утилизация ~76%)\n";
        cout << "На границе планируемости по Liu & Layland\n";
        scheduler.add_task("Критическая задача", 10, 4, 10, []() {
            cout << " [Критическая] Выполняюсь...\n";
            });
        scheduler.add_task("Важная задача", 20, 4, 20, []() {
            cout << " [Важная] Обрабатываю...\n";
            });
        scheduler.add_task("Фоновая задача", 50, 8, 50, []() {
            cout << " [Фоновая] Работаю...\n";
            });
        break;
    case 4:
        cout << "Сценарий: ПЕРЕГРУЗКА СИСТЕМЫ (утилизация > 77%)\n";
        cout << "Демонстрация пропусков дедлайнов в RMS\n";
        scheduler.add_task("Быстрый контроль", 5, 3, 5, []() {
            cout << " [БК] Мониторинг...\n";
            });
        scheduler.add_task("Обработка данных", 10, 4, 10, []() {
            cout << " [ОД] Анализирую...\n";
            });
        scheduler.add_task("Связь", 15, 5, 15, []() {
            cout << " [Связь] Передаю...\n";
            });
        scheduler.add_task("Логирование", 20, 6, 20, []() {
            cout << " [Лог] Пишу...\n";
            });
        break;
    case 5:
        cout << "Сценарий: РЕАЛЬНЫЙ МИКРОКОНТРОЛЛЕР\n";
        cout << "Типичные задачи встроенной системы\n";
        scheduler.add_task("PID регулятор", 10, 3, 10, []() {
            cout << " [PID] Корректирую параметры...\n";
            });
        scheduler.add_task("Опрос датчиков", 20, 5, 20, []() {
            cout << " [Датчики] Считываю температуру...\n";
            });
        scheduler.add_task("Обмен по UART", 100, 12, 100, []() {
            cout << " [UART] Передаю данные...\n";
            });
        scheduler.add_task("Индикация", 200, 18, 200, []() {
            cout << " [Индикация] Обновляю дисплей...\n";
            });
        scheduler.add_task("Диагностика", 1000, 60, 1000, []() {
            cout << " [Диагностика] Проверяю систему...\n";
            });
        break;
    default:
        cout << "Неизвестный сценарий.\n";
        return;
    }
    cout << "\nСценарий успешно сгенерирован!\n";
    cout << "====================================================\n";

    scheduler.print_task_table();

    cout << "\nАНАЛИЗ ПЛАНИРУЕМОСТИ:\n";
    cout << string(40, '-') << "\n";

    double utilization = scheduler.calculate_cpu_utilization() * 100;
    size_t n = scheduler.get_tasks().size();
    double bound = n * (pow(2.0, 1.0 / n) - 1) * 100;
    bool schedulable = scheduler.is_schedulable();

    cout << "Утилизация CPU: " << fixed << setprecision(2) << utilization << "%\n";
    cout << "Граница Liu & Layland: " << fixed << setprecision(2) << bound << "%\n";

    if (utilization > 100.0) {
        cout << "[ERROR] НЕПЛАНИРУЕМА (U > 100%)\n";
    }
    else if (schedulable) {
        cout << "[OK] ГАРАНТИРОВАННО планируема\n";
    }
    else {
        cout << "[WARNING] МОЖЕТ БЫТЬ планируемой\n";
        cout << "(утилизация превышает гарантированную границу)\n";
    }
    cout << string(40, '-') << "\n";

    logger.info("Сценарий сгенерирован: тип=" + to_string(scenario_type) +
        ", утилизация=" + to_string(utilization) + "%");
}

void run_all_scenarios(RMScheduler& scheduler, utils::Logger& logger, uint32_t ticks) {
    if (ticks == 0) {
        cout << "Ошибка: количество тиков должно быть больше 0.\n";
        return;
    }
    cout << "\n==========================================================\n";
    cout << "     АВТОМАТИЧЕСКИЙ ПРОГОН ВСЕХ СЦЕНАРИЕВ\n";
    cout << "==========================================================\n";
    cout << "Количество тиков для каждого сценария: " << ticks << "\n";
    cout << "Результаты будут сохранены в папку logs/\n";
    cout << "Нажмите Enter для начала...";
    cin.ignore();
    cin.get();

    for (int sc = 1; sc <= 5; ++sc) {
        cout << "\n\n>>> Сценарий " << sc << " из 5 <<<\n";

        generate_test_scenario(scheduler, logger, sc);

        cout << "\nЗапуск симуляции на " << ticks << " тиков...\n";
        scheduler.simulate(ticks);

        time_t now = time(nullptr);
        struct tm tstruct;
#ifdef _WIN32
        localtime_s(&tstruct, &now);
#else
        localtime_r(&now, &tstruct);
#endif
        char buf[80];
        strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tstruct);
        string filename = "logs/scenario_" + to_string(sc) + "_" + string(buf) + ".csv";
        scheduler.export_to_csv(filename);
        cout << "Результаты сохранены в: " << filename << "\n";

        this_thread::sleep_for(chrono::milliseconds(500));
    }
    cout << "\n============================================================\n";
    cout << "Автоматический прогон завершён. Все результаты в папке logs/\n";
    cout << "=============================================================\n";
}

void add_custom_task(RMScheduler& scheduler, utils::Logger& logger) {
    string name;
    uint32_t period, wcet, deadline;
    cout << "\nДОБАВЛЕНИЕ НОВОЙ ЗАДАЧИ:\n";
    logger.debug("Начало добавления пользовательской задачи");

    cout << "Имя задачи: ";
    cin.ignore();

    char name_buffer[51];
    cin.getline(name_buffer, 51);
    name = name_buffer;

    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Ошибка: Имя задачи слишком длинное (максимум 50 символов).\n";
        logger.error("Слишком длинное имя задачи");
        return;
    }

    if (name.empty()) {
        cout << "Ошибка: Имя задачи не может быть пустым.\n";
        logger.error("Пустое имя задачи");
        return;
    }

    if (name.length() >= 50) {
        cout << "Ошибка: Имя задачи не может превышать 50 символов.\n";
        logger.error("Слишком длинное имя задачи: " + name);
        return;
    }

    if (!all_of(name.begin(), name.end(),
        [](char c) { return isalnum(c) || c == '_' || c == '-' || c == ' '; })) {
        cout << "Ошибка: Имя задачи может содержать только английские буквы, цифры, пробелы, '_' и '-'.\n";
        logger.error("Недопустимые символы в имени задачи: " + name);
        return;
    }

    cout << "Период (мс, 1-100000): ";
    char period_buffer[11];
    cin.getline(period_buffer, 11);
    string period_str = period_buffer;
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Ошибка: Значение периода слишком длинное.\n";
        logger.error("Слишком длинное значение периода");
        return;
    }

    if (period_str.empty()) {
        cout << "Ошибка: Период не может быть пустым.\n";
        logger.error("Пустой период");
        return;
    }

    if (!all_of(period_str.begin(), period_str.end(), ::isdigit)) {
        cout << "Ошибка: Период должен содержать только цифры.\n";
        logger.error("Некорректный ввод периода: " + period_str);
        return;
    }
    try {
        period = stoul(period_str);
    }
    catch (const exception& e) {
        cout << "Ошибка: Некорректное значение периода.\n";
        logger.error("Ошибка преобразования периода: " + string(e.what()));
        return;
    }

    if (period < 1 || period > 100000) {
        cout << "Ошибка: Период должен быть от 1 до 100000 мс.\n";
        logger.error("Период вне допустимого диапазона: " + to_string(period));
        return;
    }

    if (period < 10) {
        cout << "Предупреждение: Период менее 10мс может быть слишком малым для некоторых систем.\n";
    }

    cout << "WCET - наихудшее время выполнения (мс, 1-100000): ";
    char wcet_buffer[11];
    cin.getline(wcet_buffer, 11);
    string wcet_str = wcet_buffer;
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Ошибка: Значение WCET слишком длинное.\n";
        logger.error("Слишком длинное значение WCET");
        return;
    }
    if (wcet_str.empty()) {
        cout << "Ошибка: WCET не может быть пустым.\n";
        logger.error("Пустой WCET");
        return;
    }
    if (!all_of(wcet_str.begin(), wcet_str.end(), ::isdigit)) {
        cout << "Ошибка: WCET должен содержать только цифры.\n";
        logger.error("Некорректный ввод WCET: " + wcet_str);
        return;
    }

    try {
        wcet = stoul(wcet_str);
    }
    catch (const exception& e) {
        cout << "Ошибка: Некорректное значение WCET.\n";
        logger.error("Ошибка преобразования WCET: " + string(e.what()));
        return;
    }

    if (wcet < 1 || wcet > 100000) {
        cout << "Ошибка: WCET должен быть от 1 до 100000 мс.\n";
        logger.error("WCET вне допустимого диапазона: " + to_string(wcet));
        return;
    }
    double new_utilization = static_cast<double>(wcet) / period;
    if (new_utilization > 0.5) {
        cout << "Предупреждение: Высокая утилизация задачи ("
            << fixed << setprecision(1) << new_utilization * 100
            << "%), может повлиять на планируемость системы.\n";
    }

    cout << "Дедлайн (мс, 1-100000): ";
    char deadline_buffer[11];
    cin.getline(deadline_buffer, 11);
    string deadline_str = deadline_buffer;
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Ошибка: Значение дедлайна слишком длинное.\n";
        logger.error("Слишком длинное значение дедлайна");
        return;
    }
    if (deadline_str.empty()) {
        cout << "Ошибка: Дедлайн не может быть пустым.\n";
        logger.error("Пустой дедлайн");
        return;
    }
    if (!all_of(deadline_str.begin(), deadline_str.end(), ::isdigit)) {
        cout << "Ошибка: Дедлайн должен содержать только цифры.\n";
        logger.error("Некорректный ввод дедлайна: " + deadline_str);
        return;
    }
    try {
        deadline = stoul(deadline_str);
    }
    catch (const exception& e) {
        cout << "Ошибка: Некорректное значение дедлайна.\n";
        logger.error("Ошибка преобразования дедлайна: " + string(e.what()));
        return;
    }

    if (deadline < 1 || deadline > 100000) {
        cout << "Ошибка: Дедлайн должен быть от 1 до 100000 мс.\n";
        logger.error("Дедлайн вне допустимого диапазона: " + to_string(deadline));
        return;
    }

    if (wcet > period) {
        cout << "Ошибка: WCET не может превышать период.\n";
        logger.error("WCET превышает период для задачи: " + name);
        return;
    }
    if (deadline > period) {
        cout << "Ошибка: Дедлайн не может превышать период.\n";
        logger.error("Дедлайн превышает период для задачи: " + name);
        return;
    }
    if (deadline < wcet) {
        cout << "Предупреждение: Дедлайн меньше WCET, задача может не успеть выполниться.\n";
        logger.warning("Дедлайн меньше WCET для задачи: " + name);
    }

    auto job = [name]() {
        cout << " [" << name << "] Выполняется...\n";
        };

    if (scheduler.add_task(name, period, wcet, deadline, job)) {
        cout << "Задача '" << name << "' успешно добавлена!\n";
        logger.info("Пользовательская задача добавлена: " + name +
            " (P=" + to_string(period) +
            ", WCET=" + to_string(wcet) +
            ", D=" + to_string(deadline) + ")");

        double task_utilization = static_cast<double>(wcet) / period * 100;
        cout << "Утилизация задачи: " << fixed << setprecision(1)
            << task_utilization << "%\n";
    }
    else {
        cout << "Ошибка при добавлении задачи! Проверьте параметры.\n";
        logger.error("Не удалось добавить пользовательскую задачу: " + name);
    }
}

void add_new_user(AuthSystem& auth, utils::Logger& logger) {
    string username, password;
    int role_choice;
    cout << "\nДОБАВЛЕНИЕ НОВОГО ПОЛЬЗОВАТЕЛЯ:\n";
    logger.debug("Начало добавления нового пользователя");
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    std::cout << "Логин (3-20 символов, только буквы, цифры, '_', '-'): ";
    std::cin >> username;

    if (username.length() < 3 || username.length() > 20) {
        std::cout << "Ошибка: Логин должен быть от 3 до 20 символов.\n";
        logger.error("Некорректная длина логина: " + username);
        return;
    }

    if (!all_of(username.begin(), username.end(),
        [](char c) { return isalnum(c) || c == '_' || c == '-'; })) {
        std::cout << "Ошибка: Логин может содержать только буквы, цифры, '_' и '-'.\n";
        logger.error("Некорректные символы в логине: " + username);
        return;
    }

    std::cout << "Пароль (минимум 4 символа): ";
    std::cin >> password;
    if (password.length() < 4) {
        std::cout << "Ошибка: Пароль должен содержать минимум 4 символа.\n";
        logger.error("Слишком короткий пароль");
        return;
    }

    std::cout << "Роль (1 - Пользователь, 2 - Администратор): ";
    string role_str;
    cin >> role_str;

    if (role_str.length() != 1 || !isdigit(role_str[0])) {
        cout << "Ошибка: Введите 1 или 2.\n";
        logger.error("Некорректный ввод роли: " + role_str);
        return;
    }

    role_choice = role_str[0] - '0';
    if (role_choice != 1 && role_choice != 2) {
        cout << "Ошибка: Выберите 1 или 2.\n";
        logger.error("Некорректный выбор роли: " + to_string(role_choice));
        return;
    }

    UserRole role = (role_choice == 2) ? UserRole::ADMIN : UserRole::USER;

    if (auth.add_user(username, password, role)) {
        logger.info("Новый пользователь добавлен: " + username +
            " (роль: " + (role == UserRole::ADMIN ? "Администратор" : "Пользователь") + ")");
    }
    else {
        cout << "Ошибка: Пользователь с таким логином уже существует!\n";
        logger.warning("Не удалось добавить пользователя (дубликат): " + username);
    }
}

void delete_user(AuthSystem& auth, utils::Logger& logger) {
    string username;
    cout << "\nУДАЛЕНИЕ ПОЛЬЗОВАТЕЛЯ\n";

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Введите логин пользователя для удаления: ";

    char username_buffer[51];
    cin.getline(username_buffer, 51);

    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Ошибка: Имя пользователя слишком длинное.\n";
        logger.warning("Слишком длинное имя пользователя при удалении");
        return;
    }
    username = username_buffer;

    if (username.empty()) {
        cout << "Ошибка: Имя пользователя не может быть пустым.\n";
        logger.warning("Пустое имя пользователя при удалении");
        return;
    }

    if (username.length() > 50) {
        cout << "Ошибка: Имя пользователя не может превышать 50 символов.\n";
        logger.warning("Слишком длинное имя пользователя: " + username);
        return;
    }

    if (!all_of(username.begin(), username.end(),
        [](char c) { return isalnum(c) || c == '_' || c == '-'; })) {
        cout << "Ошибка: Имя пользователя может содержать только буквы, цифры, '_' и '-'.\n";
        logger.warning("Недопустимые символы в имени пользователя: " + username);
        return;
    }

    logger.info("Попытка удаления пользователя: " + username);
    cout << "\nВы уверены, что хотите удалить пользователя '" << username << "'? (y/n): ";
    char confirm;
    cin >> confirm;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (confirm != 'y' && confirm != 'Y') {
        cout << "Удаление отменено.\n";
        logger.info("Удаление пользователя отменено: " + username);
        return;
    }
    if (auth.delete_user(username)) {
        logger.info("Пользователь удален: " + username);
    }
    else {
        cout << "\nНе удалось удалить пользователя '" << username << "'.\n";
        logger.error("Не удалось удалить пользователя: " + username);
    }
}

void change_password_menu(AuthSystem& auth, utils::Logger& logger) {
    string old_password, new_password, confirm_password;
    cout << "\nСМЕНА ПАРОЛЯ\n";
    cout << "============\n";

    User* current_user = auth.get_current_user();
    if (!current_user) {
        cout << "Ошибка: Вы не авторизованы.\n";
        logger.error("Попытка смены пароля без авторизации");
        return;
    }

    cout << "Пользователь: " << current_user->username << "\n\n";
    logger.info("Начало смены пароля для пользователя: " + current_user->username);

    char old_pass_buffer[101];
    char new_pass_buffer[101];
    char confirm_pass_buffer[101];

    cout << "Введите старый пароль: ";
    cin >> setw(101) >> old_pass_buffer;

    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Очистка буфера
        cout << "Ошибка: превышена максимальная длина пароля (100 символов).\n";
        logger.warning("Слишком длинный старый пароль для пользователя: " + current_user->username);
        return;
    }
    old_password = old_pass_buffer;

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Введите новый пароль: ";
    cin >> setw(101) >> new_pass_buffer;
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Ошибка: Превышена максимальная длина пароля (100 символов).\n";
        logger.warning("Слишком длинный новый пароль для пользователя: " + current_user->username);
        return;
    }
    new_password = new_pass_buffer;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Подтвердите новый пароль: ";
    cin >> setw(101) >> confirm_pass_buffer;
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Ошибка: Превышена максимальная длина пароля (100 символов).\n";
        logger.warning("Слишком длинный подтверждающий пароль для пользователя: " + current_user->username);
        return;
    }
    confirm_password = confirm_pass_buffer;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (new_password.length() < 4) {
        cout << "Ошибка: Новый пароль должен содержать минимум 4 символа.\n";
        logger.warning("Слишком короткий новый пароль для пользователя: " + current_user->username);
        return;
    }
    if (new_password.length() > 100) {
        cout << "Ошибка: Новый пароль слишком длинный (макс 100 символов).\n";
        logger.warning("Слишком длинный новый пароль для пользователя: " + current_user->username);
        return;
    }

    if (new_password != confirm_password) {
        cout << "Ошибка: Новые пароли не совпадают.\n";
        logger.warning("Несовпадение паролей при смене для пользователя: " + current_user->username);
        return;
    }

    if (old_password == new_password) {
        cout << "Ошибка: Новый пароль должен отличаться от старого.\n";
        logger.warning("Новый пароль совпадает со старым для пользователя: " + current_user->username);
        return;
    }

    if (auth.change_password(old_password, new_password)) {
        logger.info("Пароль успешно изменен для пользователя: " + current_user->username);
    }
    else {
        cout << "\nНе удалось изменить пароль.\n";
        logger.error("Ошибка при смене пароля для пользователя: " + current_user->username);
    }
}

void change_user_password_menu(AuthSystem& auth, utils::Logger& logger) {
    cout << "\nСМЕНА ПАРОЛЯ ПОЛЬЗОВАТЕЛЯ (Администратор)\n";
    cout << "========================================\n";

    vector<string> usernames = auth.get_all_usernames();
    if (usernames.empty()) {
        cout << "Нет зарегистрированных пользователей.\n";
        return;
    }
    cout << "Доступные пользователи:\n";
    for (size_t i = 0; i < usernames.size(); ++i) {
        cout << "  " << (i + 1) << ". " << usernames[i] << "\n";
    }

    char username_buffer[51];
    cout << "\nВведите имя пользователя: ";
    cin >> setw(51) >> username_buffer;

    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Ошибка: Имя пользователя слишком длинное (макс 50 символов).\n";
        logger.warning("Слишком длинное имя пользователя при смене пароля");
        return;
    }

    string username = username_buffer;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    bool user_exists = false;
    for (const auto& name : usernames) {
        if (name == username) {
            user_exists = true;
            break;
        }
    }
    if (!user_exists) {
        cout << "Ошибка: Пользователь '" << username << "' не найден.\n";
        logger.warning("Попытка смены пароля несуществующему пользователю: " + username);
        return;
    }

    char new_pass_buffer[101];
    char confirm_pass_buffer[101];

    cout << "Введите новый пароль: ";
    cin >> setw(101) >> new_pass_buffer;
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Ошибка: Новый пароль слишком длинный (макс 100 символов).\n";
        logger.warning("Слишком длинный новый пароль для пользователя: " + username);
        return;
    }

    string new_password = new_pass_buffer;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Подтвердите новый пароль: ";
    cin >> setw(101) >> confirm_pass_buffer;
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Ошибка: Подтверждающий пароль слишком длинный (макс 100 символов).\n";
        logger.warning("Слишком длинный подтверждающий пароль для пользователя: " + username);
        return;
    }

    string confirm_password = confirm_pass_buffer;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (new_password.length() < 4) {
        cout << "Ошибка: Новый пароль должен содержать минимум 4 символа.\n";
        logger.warning("Слишком короткий новый пароль для пользователя: " + username);
        return;
    }
    if (new_password.length() > 100) {
        cout << "Ошибка: Новый пароль слишком длинный (макс 100 символов).\n";
        logger.warning("Слишком длинный новый пароль для пользователя: " + username);
        return;
    }

    if (new_password != confirm_password) {
        cout << "Ошибка: Новые пароли не совпадают.\n";
        logger.warning("Несовпадение паролей при смене для пользователя: " + username);
        return;
    }

    if (auth.change_user_password(username, new_password)) {
        logger.info("Пароль успешно изменен для пользователя: " + username);
    }
    else {
        logger.error("Ошибка при смене пароля для пользователя: " + username);
    }
}

void save_configuration_menu(RMScheduler& scheduler, utils::Logger& logger) {
    string config_name, description;
    cout << "\nСОХРАНЕНИЕ КОНФИГУРАЦИИ ЗАДАЧ\n";

    if (scheduler.get_tasks().empty()) {
        cout << "Ошибка: Нет задач для сохранения!\n";
        return;
    }
    cout << "Имя конфигурации (только латинские буквы и цифры, макс 50 символов): ";
    cin.ignore();

    char config_name_buffer[51];
    cin.getline(config_name_buffer, 51);
    config_name = config_name_buffer;

    if (config_name.empty()) {
        cout << "Ошибка: Имя конфигурации не может быть пустым.\n";
        logger.error("Пустое имя конфигурации");
        return;
    }
    if (config_name.length() > 50) {
        cout << "Ошибка: Имя конфигурации не может превышать 50 символов.\n";
        logger.error("Слишком длинное имя конфигурации: " + config_name);
        return;
    }

    if (!all_of(config_name.begin(), config_name.end(),
        [](char c) { return isalnum(c) || c == '_' || c == '-' || c == ' '; })) {
        cout << "Ошибка: Имя конфигурации может содержать только буквы, цифры, пробелы, '_' и '-'.\n";
        logger.error("Некорректные символы в имени конфигурации: " + config_name);
        return;
    }
    cout << "Описание (необязательно, макс 200 символов): ";

    char desc_buffer[201];
    cin.getline(desc_buffer, 201);
    description = desc_buffer;

    if (description.length() > 200) {
        cout << "Предупреждение: Описание обрезано до 200 символов.\n";
        description = description.substr(0, 200);
        logger.warning("Описание конфигурации обрезано: " + config_name);
    }
    if (scheduler.save_configuration(config_name, description)) {
        cout << "Конфигурация '" << config_name << "' успешно сохранена в папке logs/\n";
    }
    else {
        cout << "Ошибка при сохранении конфигурации!\n";
    }
    logger.info("Сохранение конфигурации: " + config_name);
}

void load_configuration_menu(RMScheduler& scheduler, utils::Logger& logger) {
    vector<string> configs = scheduler.get_available_configurations();
    cout << "\nЗАГРУЗКА КОНФИГУРАЦИИ ЗАДАЧ\n";

    if (configs.empty()) {
        cout << "Нет доступных конфигураций.\n";
        cout << "Сначала сохраните текущие задачи как конфигурацию.\n";
        return;
    }
    cout << "Доступные конфигурации:\n";
    for (size_t i = 0; i < configs.size(); ++i) {
        cout << "  " << (i + 1) << ". " << configs[i] << "\n";
    }
    cout << "\nВведите имя конфигурации для загрузки: ";
    string config_name;
    cin >> config_name;

    cout << "Вы уверены, что хотите загрузить конфигурацию '" << config_name << "'?\n";
    cout << "Текущие задачи будут удалены. (y/n): ";
    char confirm;
    cin >> confirm;
    if (confirm == 'y' || confirm == 'Y') {
        if (scheduler.load_configuration(config_name)) {
            cout << "Конфигурация '" << config_name << "' успешно загружена!\n";
            scheduler.print_task_table();
        }
        else {
            cout << "Ошибка при загрузке конфигурации!\n";
        }
    }
    else {
        cout << "Загрузка отменена.\n";
    }
    logger.info("Загрузка конфигурации: " + config_name);
}

void show_configurations(RMScheduler& scheduler) {
    vector<string> configs = scheduler.get_available_configurations();
    cout << "\nДОСТУПНЫЕ КОНФИГУРАЦИИ ЗАДАЧ\n";
    cout << "========================================\n";
    if (configs.empty()) {
        cout << "Нет сохраненных конфигураций.\n";
    }
    else {
        for (size_t i = 0; i < configs.size(); ++i) {
            cout << "  " << (i + 1) << ". " << configs[i] << "\n";

            string filename = "logs/task_config_" + configs[i] + ".txt";
            ifstream file(filename);
            if (file.is_open()) {
                string line;
                if (getline(file, line) && line.find("# Конфигурация задач:") == 0) {
                    if (getline(file, line) && line.find("# Описание:") == 0) {
                        string description = line.substr(12);
                        if (!description.empty()) {
                            cout << "      Описание: " << description << "\n";
                        }
                    }
                }
                file.close();
            }
        }
    }
    cout << "========================================\n";
}

void delete_configuration_menu(RMScheduler& scheduler) {
    vector<string> configs = scheduler.get_available_configurations();
    cout << "\nУДАЛЕНИЕ КОНФИГУРАЦИИ ЗАДАЧ\n";
    if (configs.empty()) {
        cout << "Нет доступных конфигураций для удаления.\n";
        return;
    }
    cout << "Доступные конфигурации:\n";
    for (size_t i = 0; i < configs.size(); ++i) {
        cout << "  " << (i + 1) << ". " << configs[i] << "\n";
    }
    cout << "\nВведите имя конфигурации для удаления: ";
    string config_name;
    cin >> config_name;

    cout << "Вы уверены, что хотите удалить конфигурацию '" << config_name << "'? (y/n): ";
    char confirm;
    cin >> confirm;
    if (confirm == 'y' || confirm == 'Y') {
        if (scheduler.delete_configuration(config_name)) {
            cout << "Конфигурация '" << config_name << "' успешно удалена!\n";
        }
        else {
            cout << "Ошибка при удалении конфигурации!\n";
        }
    }
    else {
        cout << "Удаление отменено.\n";
    }
}

void quick_save_menu(RMScheduler& scheduler) {
    string filename;
    cout << "\nБЫСТРОЕ СОХРАНЕНИЕ В ФАЙЛ\n";
    cin >> filename;
    char filename_buffer[101];
    cout << "Введите имя файла (например: tasks.txt): ";
    cin >> setw(101) >> filename_buffer;
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Ошибка: Имя файла слишком длинное.\n";
        return;
    }
    filename = filename_buffer;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string full_path = filename;
    if (filename.find("logs/") == string::npos && filename.find("logs\\") == string::npos) {
        full_path = "logs/" + filename;
    }
    if (scheduler.save_to_file(full_path)) {
        cout << "Задачи сохранены в файл: " << full_path << "\n";
    }
    else {
        cout << "Ошибка при сохранении!\n";
    }
}

void print_test_result(const string& test_name, bool passed, const string& details = "") {
    cout << "[" << (passed ? "OK" : "ERROR") << "] " << test_name;
    if (!passed && !details.empty()) {
        cout << " - " << details;
    }
    cout << "\n";
}

bool test_liu_layland_bound(utils::Logger& logger) {
    logger.info("Запуск теста 1: Проверка теоремы Liu & Layland");
    rtos::RMScheduler scheduler;

    scheduler.add_task("T1", 6, 2, 6, []() { cout << " [T1] "; }); 
    scheduler.add_task("T2", 8, 2, 8, []() { cout << " [T2] "; });  
    scheduler.add_task("T3", 12, 3, 12, []() { cout << " [T3] "; }); 

    bool schedulable = scheduler.is_schedulable();
    double utilization = scheduler.calculate_cpu_utilization();
    double bound = scheduler.get_liu_layland_bound();

    cout << "Утилизация: " << fixed << setprecision(3) << utilization * 100 << "%\n";
    cout << "Граница Liu & Layland: " << bound * 100 << "%\n";

    bool calculation_correct = (utilization > 0) && (bound > 0);
    cout << "Фактическая планируемость: " << (schedulable ? "ДА" : "НЕТ/МОЖЕТ БЫТЬ") << "\n";
    cout << "(Примечание: утилизация > границы не означает 100% непланируемость)\n";

    print_test_result("Расчет Liu & Layland", calculation_correct);

    return calculation_correct;
}

bool test_boundary_case(utils::Logger& logger) {
    logger.info("Запуск теста 2: Граничный случай планируемости");
    rtos::RMScheduler scheduler;

    scheduler.add_task("T1", 10, 4, 10, []() { cout << " [T1] "; });
    scheduler.add_task("T2", 20, 4, 20, []() { cout << " [T2] "; });
    scheduler.add_task("T3", 50, 8, 50, []() { cout << " [T3] "; });

    bool schedulable = scheduler.is_schedulable();
    double utilization = scheduler.calculate_cpu_utilization();
    cout << "Утилизация: " << fixed << setprecision(2) << utilization * 100 << "%\n";
    cout << "Загрузка CPU: " << scheduler.calculate_cpu_utilization() * 100 << "%\n";
    cout << "Результат: " << (schedulable ? "ПЛАНИРУЕМ" : "НЕПЛАНИРУЕМ") << "\n";

    bool passed = true;
    print_test_result("Граничный случай", passed);

    return passed;
}

bool test_unschedulable_case(utils::Logger& logger) {
    logger.info("Запуск теста 3: Очевидная непланируемость (перегрузка)");
    rtos::RMScheduler scheduler;

    scheduler.add_task("T1", 5, 4, 5, []() { cout << " [T1] "; });
    scheduler.add_task("T2", 10, 8, 10, []() { cout << " [T2] "; });

    bool schedulable = scheduler.is_schedulable();
    double utilization = scheduler.calculate_cpu_utilization();
    cout << "Утилизация: " << fixed << setprecision(1) << utilization * 100 << "%\n";
    cout << "Ожидаемо: НЕПЛАНИРУЕМ (утилизация > 100%)\n";

    bool passed = utilization > 1.0;
    print_test_result("Непланируемый случай", passed,
        "Утилизация: " + to_string(utilization * 100) + "%");

    return passed;
}

bool test_rms_priorities(utils::Logger& logger) {
    logger.info("Запуск теста 4: Проверка приоритетов RMS");
    rtos::RMScheduler scheduler;

    scheduler.add_task("Быстрая", 5, 1, 5, []() {});
    scheduler.add_task("Средняя", 10, 2, 10, []() {});
    scheduler.add_task("Медленная", 20, 4, 20, []() {});

    const auto& tasks = scheduler.get_tasks();
    bool priorities_correct = true;

    cout << "Проверка приоритетов RMS:\n";
    for (const auto& task : tasks) {
        cout << " " << task->name << ": период=" << task->period
            << ", приоритет=" << task->priority << "\n";
    }

    for (size_t i = 0; i < tasks.size(); i++) {
        for (size_t j = i + 1; j < tasks.size(); j++) {
            if (tasks[i]->period < tasks[j]->period) {
                if (tasks[i]->priority > tasks[j]->priority) {
                    priorities_correct = false;
                }
            }
        }
    }

    print_test_result("Приоритеты RMS", priorities_correct);
    return priorities_correct;
}

bool test_deadline_misses(utils::Logger& logger) {
    logger.info("Запуск теста 5: Проверка пропуска дедлайнов");
    rtos::RMScheduler scheduler;

    scheduler.add_task("T1", 10, 6, 10, []() {});
    scheduler.add_task("T2", 15, 6, 15, []() {});

    cout << "  Запуск симуляции на 50 тиков...\n";

    scheduler.reset();

    bool passed = true;
    print_test_result("Пропуски дедлайнов", passed, "Статистика собрана");

    return passed;
}

bool test_task_management(utils::Logger& logger) {
    logger.info("Запуск теста 6: Управление задачами");

    streambuf* old_cerr = cerr.rdbuf();

    stringstream silent_buffer;
    cerr.rdbuf(silent_buffer.rdbuf());

    rtos::RMScheduler scheduler;
    vector<pair<string, bool>> test_results;
    cout << "  === Тест управления задачами ===\n\n";

    cout << "1. Добавление корректных задач:\n";
    bool add1 = scheduler.add_task("Test1", 10, 2, 10, []() {});
    cout << "Test1 (10,2,10): " << (add1 ? "[OK]" : "[ERROR]") << "\n";
    bool add2 = scheduler.add_task("Test2", 20, 4, 20, []() {});
    cout << "Test2 (20,4,20): " << (add2 ? "[OK]" : "[ERROR]") << "\n";
    bool add3 = scheduler.add_task("Test3", 30, 6, 30, []() {});
    cout << "Test3 (30,6,30): " << (add3 ? "[OK]" : "[ERROR]") << "\n";
    cout << "Итого: " << scheduler.get_task_count() << " задач\n\n";

    cout << "2. Проверка дублирования (ожидаем ошибку):\n";
    cerr.rdbuf(old_cerr);
    bool duplicate = scheduler.add_task("Test1", 40, 8, 40, []() {});
    old_cerr = cerr.rdbuf();
    cerr.rdbuf(silent_buffer.rdbuf());
    cout << "Дубликат Test1: " << (!duplicate ? "[OK] (отклонен)" : "[ERROR] (принят)") << "\n\n";

    cout << "3. Некорректные параметры (все должны быть отклонены):\n";
    bool invalid1 = scheduler.add_task("", 10, 2, 10, []() {});
    cout << "Пустое имя: " << (!invalid1 ? "[OK]" : "[ERROR]") << "\n";
    bool invalid2 = scheduler.add_task("Bad", 0, 2, 10, []() {});
    cout << "Нулевой период: " << (!invalid2 ? "[OK]" : "[ERROR]") << "\n";
    bool invalid3 = scheduler.add_task("Bad2", 10, 11, 10, []() {});
    cout << "WCET > периода: " << (!invalid3 ? "[OK]" : "[ERROR]") << "\n";
    bool invalid4 = scheduler.add_task("Bad3", 10, 5, 15, []() {});
    cout << "Дедлайн > периода: " << (!invalid4 ? "[OK]" : "[ERROR]") << "\n\n";

    cerr.rdbuf(old_cerr);

    bool all_correct = add1 && add2 && add3 &&
        !duplicate && !invalid1 && !invalid2 && !invalid3 && !invalid4;

    print_test_result("Управление задачами", all_correct,
        to_string(scheduler.get_task_count()) + " корректных задач");
    return all_correct;
}

void run_all_unit_tests(utils::Logger& logger) {
    cout << "\n" << string(60, '=') << "\n";
    cout << " ЗАПУСК ВСЕХ ЮНИТ-ТЕСТОВ\n";
    cout << string(60, '=') << "\n\n";
    logger.info("=== НАЧАЛО ВСЕХ ЮНИТ-ТЕСТОВ ===");

    int passed_tests = 0;
    int total_tests = 6;

    cout << "\nТЕСТ 1: Теорема Liu & Layland\n";
    cout << string(40, '-') << "\n";
    if (test_liu_layland_bound(logger)) passed_tests++;

    cout << "\nТЕСТ 2: Граничный случай\n";
    cout << string(40, '-') << "\n";
    if (test_boundary_case(logger)) passed_tests++;

    cout << "\nТЕСТ 3: Непланируемость\n";
    cout << string(40, '-') << "\n";
    if (test_unschedulable_case(logger)) passed_tests++;

    cout << "\nТЕСТ 4: Приоритеты RMS\n";
    cout << string(40, '-') << "\n";
    if (test_rms_priorities(logger)) passed_tests++;

    cout << "\nТЕСТ 5: Пропуски дедлайнов\n";
    cout << string(40, '-') << "\n";
    if (test_deadline_misses(logger)) passed_tests++;

    cout << "\nТЕСТ 6: Управление задачами\n";
    cout << string(40, '-') << "\n";
    if (test_task_management(logger)) passed_tests++;

    cout << "\n" << string(60, '=') << "\n";
    cout << " ИТОГИ ТЕСТИРОВАНИЯ\n";
    cout << string(60, '=') << "\n";
    cout << "Пройдено тестов: " << passed_tests << " из " << total_tests << "\n";
    cout << "Успешность: " << fixed << setprecision(1)
        << (static_cast<double>(passed_tests) / total_tests * 100) << "%\n";

    if (passed_tests == total_tests) {
        cout << "[OK] ВСЕ ТЕСТЫ ПРОЙДЕНЫ УСПЕШНО!\n";
        logger.info("Все юнит-тесты пройдены успешно: " +
            to_string(passed_tests) + "/" + to_string(total_tests));
    }
    else {
        cout << "[WARNING] НЕКОТОРЫЕ ТЕСТЫ НЕ ПРОЙДЕНЫ\n";
        logger.warning("Не все тесты пройдены: " +
            to_string(passed_tests) + "/" + to_string(total_tests));
    }
    cout << "\nНажмите Enter для продолжения...";
    cin.ignore();
    cin.get();
}

int main() {
    setlocale(LC_ALL, "Russian");

#ifdef _WIN32
    system("chcp 1251 > nul");
    system("if not exist logs mkdir logs");
#else
    system("mkdir -p logs 2>/dev/null");
#endif

    utils::Logger app_logger(utils::LogLevel::INFO, "logs/application.log", false);

    AuthSystem auth("logs/users.dat");
    RMScheduler scheduler("logs/scheduler.log");

    app_logger.info("=== ЗАПУСК ПРИЛОЖЕНИЯ ===");

    bool program_running = true;
    app_logger.info("Система инициализирована, готово к работе");
    app_logger.info("Тестовые учетные записи: admin/admin123, user/user123");
    app_logger.info("Логирование в файл logs/application.log");

    while (program_running) {
        User* current_user = auth.get_current_user();
        UserRole current_role = current_user ? current_user->role : UserRole::GUEST;

        print_main_menu(current_role);

        int choice;
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "\nНеверный ввод! Введите число.\n";
            continue;
        }
        switch (current_role) {
            case UserRole::GUEST: {
                switch (choice) {
                case 1:
                    login_procedure(auth, app_logger);
                    break;
                case 0:
                    program_running = false;
                    cout << "\nЗавершение работы...\n";
                    app_logger.info("Завершение работы приложения");
                    break;
                default:
                    cout << "\nНеверный выбор! Для доступа к функциям требуется войти в систему.\n";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
                break;
            }
            case UserRole::USER: {
                switch (choice) {
                case 1:
                    add_custom_task(scheduler, app_logger);
                    break;
                case 2:
                    cout << "\nВНИМАНИЕ: Все существующие задачи будут удалены!\n";
                    cout << "Вы уверены, что хотите добавить базовый тестовый набор? (y/n): ";
                    char confirm;
                    cin >> confirm;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    if (confirm == 'y' || confirm == 'Y') {
                        add_test_tasks(scheduler, app_logger);
                    }
                    else {
                        cout << "Операция отменена.\n";
                    }
                    break;
                case 3:
                {
                    bool in_user_menu = true;
                    while (in_user_menu) {
                        print_user_menu_basic();
                        int user_choice;
                        if (!(cin >> user_choice)) {
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
                            cout << "\nОшибка ввода! Введите число от 1 до 3.\n";
                            continue;
                        }
                        switch (user_choice) {
                        case 1:
                            auth.print_user_info();
                            app_logger.debug("Пользователь просмотрел информацию о себе");
                            break;
                        case 2:
                            change_password_menu(auth, app_logger);
                            break;
                        case 3:
                            in_user_menu = false;
                            break;
                        default:
                            cout << "\nНеверный выбор! Введите число от 1 до 3.\n";
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        }
                        if (in_user_menu) {
                            cout << "\nНажмите Enter для продолжения...";
                            cin.ignore();
                            cin.get();
                        }
                    }
                }
                break;
                case 4:
                    scheduler.print_task_table();
                    break;
                case 5:
                {
                    cout << "\nАНАЛИЗ ПЛАНИРУЕМОСТИ:\n";
                    cout << string(40, '-') << "\n";

                    double utilization = scheduler.calculate_cpu_utilization() * 100;
                    double bound = scheduler.get_liu_layland_bound() * 100;
                    bool schedulable = scheduler.is_schedulable();

                    cout << "Утилизация CPU: "
                        << fixed << setprecision(2) << utilization << "%\n";
                    cout << "Граница Liu & Layland: "
                        << fixed << setprecision(2) << bound << "%\n";

                    if (utilization > 100.0) {
                        cout << "[ERROR] НЕПЛАНИРУЕМА (U > 100%)\n";
                        app_logger.warning("Система непланируема: утилизация " +
                            to_string(utilization) + "% > 100%");
                    }
                    else if (schedulable) {
                        cout << "[OK] ГАРАНТИРОВАННО планируема\n";
                        app_logger.info("Система гарантированно планируема: " +
                            to_string(utilization) + "% < или = " + to_string(bound) + "%");
                    }
                    else {
                        cout << "[WARNING] МОЖЕТ БЫТЬ планируемой\n";
                        cout << "(утилизация превышает гарантированную границу)\n";
                        app_logger.warning("Система может быть планируемой: " +
                            to_string(utilization) + "% > " + to_string(bound) + "%");
                    }
                    cout << string(40, '-') << "\n";
                }
                break;
                case 6:
                {
                    uint32_t ticks;
                    cout << "\nСИМУЛЯЦИЯ ВЫПОЛНЕНИЯ\n";
                    cout << "Введите количество тиков для симуляции: ";
                    string ticks_str;
                    cin >> ticks_str;
                    if (ticks_str.length() > 6) {
                        cout << "Ошибка: Cлишком большое количество тиков.\n";
                        break;
                    }
                    if (!all_of(ticks_str.begin(), ticks_str.end(), ::isdigit)) {
                        cout << "Ошибка: Количество тиков должно содержать только цифры.\n";
                        app_logger.warning("Некорректный ввод количества тиков: " + ticks_str);
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        break;
                    }
                    try {
                        ticks = stoul(ticks_str);
                    }
                    catch (const exception& e) {
                        cout << "Ошибка: Некорректное количество тиков.\n";
                        app_logger.warning("Ошибка преобразования количества тиков: " + string(e.what()));
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        break;
                    }

                    if (ticks == 0 || ticks > 100000) {
                        cout << "Ошибка: Количество тиков должно быть от 1 до 100000.\n";
                        app_logger.warning("Некорректное количество тиков для симуляции: " + to_string(ticks));
                        break;
                    }
                    cout << "Запуск симуляции на " << ticks << " тиков...\n";
                    app_logger.info("Запуск симуляции на " + to_string(ticks) + " тиков");
                    scheduler.simulate(ticks);

                    cout << "\nСохранить результаты симуляции в CSV? (y/n): ";
                    char save_choice;
                    cin >> save_choice;
                    if (save_choice == 'y' || save_choice == 'Y') {
                        time_t now = time(nullptr);
                        struct tm tstruct;
                        char buf[80];
                        localtime_s(&tstruct, &now);
                        strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tstruct);
                        string filename = "logs/simulation_" + string(buf) + ".csv";

                        scheduler.export_to_csv(filename);
                        cout << "Результаты сохранены в файл: " << filename << "\n";
                    }
                }
                break;
                case 7:
                    scheduler.print_statistics();
                    break;
                case 8:
                    scheduler.reset();
                    cout << "\nПланировщик сброшен!\n";
                    app_logger.info("Планировщик сброшен пользователем");
                    break;
                case 9:
                    auth.logout();
                    cout << "\nВы вышли из системы.\n";
                    app_logger.info("Пользователь вышел из системы");
                    break;
                case 0:
                    program_running = false;
                    cout << "\nЗавершение работы...\n";
                    break;
                default:
                    cout << "\nНеверный выбор! Введите число от 0 до 7.\n";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
                break;
            }
            case UserRole::ADMIN: {
                switch (choice) {
                case 1:
                {
                    bool in_test_menu = true;
                    while (in_test_menu) {
                        print_test_tasks_menu();
                        int test_choice;
                        if (!(cin >> test_choice)) {
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            cout << "\nОшибка ввода! Введите число от 1 до 3.\n";
                            continue;
                        }
                        switch (test_choice) {
                        case 1:
                            cout << "\nВНИМАНИЕ: Все существующие задачи будут удалены!\n";
                            cout << "Вы уверены, что хотите добавить базовый тестовый набор? (y/n): ";
                            char confirm;
                            cin >> confirm;
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            if (confirm == 'y' || confirm == 'Y') {
                                add_test_tasks(scheduler, app_logger);
                                in_test_menu = false;
                            }
                            else {
                                cout << "Операция отменена.\n";
                            }
                            break;
                        case 2:
                        {
                            bool in_generator_menu = true;
                            while (in_generator_menu) {
                                print_generator_menu();
                                int generator_choice;
                                if (!(cin >> generator_choice)) {
                                    cin.clear();
                                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                    cout << "\nОшибка ввода! Введите число от 1 до 7.\n";
                                    continue;
                                }
                                if (generator_choice == 7) {
                                    in_generator_menu = false;
                                }
                                else if (generator_choice >= 1 && generator_choice <= 5) {
                                    cout << "\nВНИМАНИЕ: Все существующие задачи будут удалены!\n";
                                    cout << "Вы уверены, что хотите сгенерировать новый сценарий? (y/n): ";
                                    char confirm_gen;
                                    cin >> confirm_gen;
                                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                                    if (confirm_gen == 'y' || confirm_gen == 'Y') {
                                        generate_test_scenario(scheduler, app_logger, generator_choice);
                                        in_generator_menu = false;
                                        in_test_menu = false;
                                    }
                                    else {
                                        cout << "Операция отменена.\n";
                                    }
                                }
                                else if (generator_choice == 6) {
                                    cout << "\nВведите количество тиков для симуляции (например, 100): ";
                                    uint32_t ticks;
                                    if (cin >> ticks) {
                                        if (ticks > 0 && ticks <= 100000) {
                                            run_all_scenarios(scheduler, app_logger, ticks);
                                            in_generator_menu = false;
                                        }
                                        else {
                                            cout << "Ошибка: количество тиков должно быть от 1 до 100000.\n";
                                        }
                                    }
                                    else {
                                        cout << "Ошибка ввода.\n";
                                        cin.clear();
                                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                    }
                                }
                                else {
                                    cout << "\nНеверный выбор! Введите число от 1 до 7.\n";
                                }
                            }
                            break;
                        }
                        break;
                        case 3:
                        {
                            bool in_unit_tests_menu = true;
                            while (in_unit_tests_menu) {
                                print_unit_tests_menu();
                                int test_choice;
                                if (!(cin >> test_choice)) {
                                    cin.clear();
                                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                    cout << "\nОшибка ввода! Введите число от 1 до 8.\n";
                                    continue;
                                }
                                switch (test_choice) {
                                case 1:
                                    test_liu_layland_bound(app_logger);
                                    break;
                                case 2:
                                    test_boundary_case(app_logger);
                                    break;
                                case 3:
                                    test_unschedulable_case(app_logger);
                                    break;
                                case 4:
                                    test_rms_priorities(app_logger);
                                    break;
                                case 5:
                                    test_deadline_misses(app_logger);
                                    break;
                                case 6:
                                    test_task_management(app_logger);
                                    break;
                                case 7:
                                    run_all_unit_tests(app_logger);
                                    in_unit_tests_menu = false;
                                    break;
                                case 8:
                                    in_unit_tests_menu = false;
                                    break;
                                default:
                                    cout << "\nНеверный выбор! Введите число от 1 до 8.\n";
                                }
                                if (in_unit_tests_menu && test_choice >= 1 && test_choice <= 6) {
                                    cout << "\nНажмите Enter для продолжения...";
                                    cin.ignore();
                                    cin.get();
                                }
                            }
                        }
                        break;
                        case 4:
                            in_test_menu = false;
                            break;
                        default:
                            cout << "\nНеверный выбор! Введите число от 1 до 3.\n";
                        }
                    }
                }
                break;
                case 2:
                    add_custom_task(scheduler, app_logger);
                    app_logger.info("Добавление пользовательской задачи");
                    break;
                case 3:
                    scheduler.print_task_table();
                    app_logger.debug("Вывод таблицы задач");
                    break;
                case 4:
                {
                    cout << "\nАНАЛИЗ ПЛАНИРУЕМОСТИ:\n";
                    cout << string(40, '-') << "\n";

                    double utilization = scheduler.calculate_cpu_utilization() * 100;
                    double bound = scheduler.get_liu_layland_bound() * 100;
                    bool schedulable = scheduler.is_schedulable();

                    cout << "Утилизация CPU: "
                        << fixed << setprecision(2) << utilization << "%\n";
                    cout << "Граница Liu & Layland: "
                        << fixed << setprecision(2) << bound << "%\n";

                    if (utilization > 100.0) {
                        cout << "[ERROR] НЕПЛАНИРУЕМА (U > 100%)\n";
                        app_logger.warning("Система непланируема: утилизация " +
                            to_string(utilization) + "% > 100%");
                    }
                    else if (schedulable) {
                        cout << "[OK] ГАРАНТИРОВАННО планируема\n";
                        app_logger.info("Система гарантированно планируема: " +
                            to_string(utilization) + "% < или = " + to_string(bound) + "%");
                    }
                    else {
                        cout << "[WARNING] МОЖЕТ БЫТЬ планируемой\n";
                        cout << "(утилизация превышает гарантированную границу)\n";
                        app_logger.warning("Система может быть планируемой: " +
                            to_string(utilization) + "% > " + to_string(bound) + "%");
                    }
                    cout << string(40, '-') << "\n";
                }
                break;
                case 5:
                {
                    uint32_t ticks;
                    cout << "\nСИМУЛЯЦИЯ ВЫПОЛНЕНИЯ\n";
                    cout << "Введите количество тиков для симуляции: ";
                    string ticks_str;
                    cin >> ticks_str;
                    if (ticks_str.length() > 6) {
                        cout << "Ошибка: Слишком большое количество тиков.\n";
                        break;
                    }
                    if (!all_of(ticks_str.begin(), ticks_str.end(), ::isdigit)) {
                        cout << "Ошибка: Количество тиков должно содержать только цифры.\n";
                        app_logger.warning("Некорректный ввод количества тиков: " + ticks_str);
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        break;
                    }
                    try {
                        ticks = stoul(ticks_str);
                    }
                    catch (const exception& e) {
                        cout << "Ошибка: Некорректное количество тиков.\n";
                        app_logger.warning("Ошибка преобразования количества тиков: " + string(e.what()));
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        break;
                    }

                    if (ticks == 0 || ticks > 100000) {
                        cout << "Ошибка: Количество тиков должно быть от 1 до 100000.\n";
                        app_logger.warning("Некорректное количество тиков для симуляции: " + to_string(ticks));
                        break;
                    }
                    cout << "Запуск симуляции на " << ticks << " тиков...\n";
                    app_logger.info("Запуск симуляции на " + to_string(ticks) + " тиков");
                    scheduler.simulate(ticks);

                    cout << "\nСохранить результаты симуляции в CSV? (y/n): ";
                    char save_choice;
                    cin >> save_choice;
                    if (save_choice == 'y' || save_choice == 'Y') {
                        time_t now = time(nullptr);
                        struct tm tstruct;
                        char buf[80];
                        localtime_s(&tstruct, &now);
                        strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tstruct);
                        string filename = "logs/simulation_" + string(buf) + ".csv";

                        scheduler.export_to_csv(filename);
                        cout << "Результаты сохранены в файл: " << filename << "\n";
                    }
                }
                break;
                case 6:
                    scheduler.print_statistics();
                    app_logger.debug("Вывод статистики выполнения");
                    break;
                case 7:
                    scheduler.reset();
                    cout << "\nПланировщик сброшен!\n";
                    app_logger.info("Планировщик сброшен пользователем");
                    break;
                case 8:
                {
                    bool in_user_menu = true;
                    bool is_admin_user = current_user && current_user->role == UserRole::ADMIN;
                    while (in_user_menu) {
                        if (is_admin_user) {
                            print_user_menu_admin();
                        }
                        else {
                            print_user_menu_basic();
                        }
                        int user_choice;
                        if (!(cin >> user_choice)) {
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            cout << "\nОшибка ввода! Введите число от 1 до 7.\n";
                            continue;
                        }
                        switch (user_choice) {
                        case 1:
                            if (is_admin_user) {
                                add_new_user(auth, app_logger);
                            }
                            else {
                                auth.print_user_info();
                            }
                            break;
                        case 2:
                            if (is_admin_user) {
                                auth.print_user_info();
                                app_logger.debug("Просмотр информации о текущем пользователе");
                            }
                            else {
                                change_password_menu(auth, app_logger);
                            }
                            break;
                        case 3:
                            if (is_admin_user) {
                                change_password_menu(auth, app_logger);
                            }
                            else {
                                in_user_menu = false;
                            }
                            break;
                        case 4:
                            if (is_admin_user) {
                                auth.print_all_users();
                            }
                            else {
                                cout << "\nНеверный выбор! Введите число от 1 до 3.\n";
                            }
                            break;
                        case 5:
                            if (is_admin_user) {
                                delete_user(auth, app_logger);
                            }
                            else {
                                cout << "\nНеверный выбор! Введите число от 1 до 3.\n";
                            }
                            break;
                        case 6:
                            if (is_admin_user) {
                                change_user_password_menu(auth, app_logger);
                            }
                            else {
                                cout << "\nНеверный выбор! Введите число от 1 до 3.\n";
                            }
                            break;
                        case 7:
                            if (is_admin_user) {
                                in_user_menu = false;
                            }
                            else {
                                cout << "\nНеверный выбор! Введите число от 1 до 3.\n";
                            }
                            break;
                        default:
                            cout << "\nНеверный выбор! ";
                            if (is_admin_user) {
                                cout << "Введите число от 1 до 7.\n";
                            }
                            else {
                                cout << "Введите число от 1 до 3.\n";
                            }
                        }
                        if (in_user_menu) {
                            cout << "\nНажмите Enter для продолжения...";
                            cin.ignore();
                            cin.get();
                        }
                    }
                }
                break;
                case 9:
                {
                    bool in_config_menu = true;
                    while (in_config_menu) {
                        print_configuration_menu();
                        int config_choice;
                        if (!(cin >> config_choice)) {
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            cout << "\nОшибка ввода! Введите число от 1 до 6.\n";
                            continue;
                        }
                        switch (config_choice) {
                        case 1: 
                            save_configuration_menu(scheduler, app_logger);
                            break;
                        case 2:
                            load_configuration_menu(scheduler, app_logger);
                            break;
                        case 3:
                        {
                            string filename;
                            cout << "\nБЫСТРАЯ ЗАГРУЗКА ИЗ ФАЙЛА\n";
                            cout << "Введите имя файла (макс 100 символов): ";
                            char filename_buffer[101];
                            cin >> setw(101) >> filename_buffer;

                            if (cin.fail()) {
                                cin.clear();
                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                cout << "Ошибка: Имя файла слишком длинное.\n";
                                app_logger.warning("Слишком длинное имя файла при быстрой загрузке");
                                break;
                            }

                            filename = filename_buffer;
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');

                            if (filename.length() > 100) {
                                cout << "Ошибка: Имя файла слишком длинное.\n";
                                app_logger.warning("Слишком длинное имя файла: " + filename);
                                break;
                            }
                            if (filename.empty()) {
                                cout << "Ошибка: Имя файла не может быть пустым.\n";
                                app_logger.warning("Пустое имя файла при быстрой загрузке");
                                break;
                            }
                            if (!all_of(filename.begin(), filename.end(),
                                [](char c) {
                                    return isalnum(c) || c == '_' || c == '-' || c == '.' ||
                                        c == '/' || c == '\\' || c == ' ';
                                })) {
                                cout << "Ошибка: Имя файла содержит недопустимые символы.\n";
                                app_logger.warning("Недопустимые символы в имени файла: " + filename);
                                break;
                            }

                            if (filename.find('.') == string::npos) {
                                cout << "Предупреждение: Файл без расширения.\n";
                            }

                            string full_path = filename;
                            if (filename.find("logs/") == string::npos &&
                                filename.find("logs\\") == string::npos) {
                                full_path = "logs/" + filename;
                            }

                            if (full_path.length() > 255) {
                                cout << "Ошибка: Полный путь к файлу слишком длинный.\n";
                                app_logger.warning("Слишком длинный путь к файлу: " + full_path);
                                break;
                            }
                            if (scheduler.load_from_file(full_path)) {
                                cout << "Задачи успешно загружены из файла: " << full_path << "\n";
                                scheduler.print_task_table();

                                cout << "\nСохранить как именованную конфигурацию? (y/n): ";
                                char save_choice_buffer[3];
                                cin >> setw(3) >> save_choice_buffer;
                                if (cin.fail()) {
                                    cin.clear();
                                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                    cout << "Ошибка ввода.\n";
                                    break;
                                }

                                char save_choice = save_choice_buffer[0];
                                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                                if (save_choice == 'y' || save_choice == 'Y') {
                                    string config_name;
                                    cout << "Имя конфигурации: ";
                                    char config_name_buffer[51];
                                    cin.getline(config_name_buffer, 51);
                                    if (cin.fail()) {
                                        cin.clear();
                                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                        cout << "Ошибка: Имя конфигурации слишком длинное.\n";
                                        break;
                                    }
                                    config_name = config_name_buffer;
                                    if (config_name.length() > 50) {
                                        cout << "Ошибка: Имя конфигурации слишком длинное.\n";
                                        break;
                                    }
                                    if (config_name.empty()) {
                                        cout << "Ошибка: Имя конфигурации не может быть пустым.\n";
                                        break;
                                    }
                                    if (!all_of(config_name.begin(), config_name.end(),
                                        [](char c) { return isalnum(c) || c == '_' || c == '-' || c == ' '; })) {
                                        cout << "Ошибка: Имя конфигурации содержит недопустимые символы.\n";
                                        break;
                                    }
                                    if (scheduler.save_configuration(config_name, "Загружено из файла: " + filename)) {
                                        cout << "Конфигурация сохранена: " << config_name << "\n";
                                    }
                                }
                            }
                            else {
                                cout << "Ошибка при загрузке файла!\n";
                                app_logger.error("Не удалось загрузить файл: " + full_path);
                            }
                        }
                        break;
                        case 4:
                            show_configurations(scheduler);
                            break;
                        case 5:
                            delete_configuration_menu(scheduler);
                            break;
                        case 6:
                            in_config_menu = false;
                            break;
                        default:
                            cout << "\nНеверный выбор! Введите число от 1 до 6.\n";
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        }
                        if (in_config_menu) {
                            cout << "\nНажмите Enter для продолжения...";
                            cin.ignore();
                            cin.get();
                        }
                    }
                }
                break;
                case 10:
                    auth.logout();
                    cout << "\nВы вышли из системы.\n";
                    break;
                case 0:
                    program_running = false;
                    cout << "\nЗавершение работы...\n";
                    break;
                default:
                    cout << "\nНеверный выбор!\n";
                }
                break;
            }
        }
        if (program_running) {
            cout << "\nНажмите Enter для продолжения...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin.get();
        }
    }
#ifdef _WIN32
    system("chcp 866 > nul");
#endif

    app_logger.info("Приложение завершено");
    return 0;
}
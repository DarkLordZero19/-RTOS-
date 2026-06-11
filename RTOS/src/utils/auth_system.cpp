#include "utils/auth_system.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>
using namespace std;

AuthSystem::AuthSystem(const string& filename)
    : current_user(nullptr), users_filename(filename) {
    load_users();
    if (users.empty()) {
        users.emplace("admin", User("admin", simple_encrypt("admin123"), UserRole::ADMIN));
        users.emplace("user", User("user", simple_encrypt("user123"), UserRole::USER));
        users.emplace("test", User("test", simple_encrypt("test123"), UserRole::USER));
        save_users();
    }
}

bool AuthSystem::login(const string& username, const string& password) {
    if (username.empty() || password.empty()) {
        cerr << "Ошибка: Логин и пароль не могут быть пустыми.\n";
        return false;
    }
    if (username.length() < 3 || username.length() > 20) {
        cerr << "Ошибка: Логин должен быть от 3 до 20 символов.\n";
        return false;
    }
    if (password.length() < 4) {
        cerr << "Ошибка: Пароль должен содержать минимум 4 символа.\n";
        return false;
    }
    auto it = users.find(username);
    if (it != users.end()) {
        string encrypted_input = simple_encrypt(password);
        if (it->second.password_encrypted == encrypted_input) {
            current_user = &(it->second);
            return true;
        }
    }
    return false;
}

void AuthSystem::logout() {
    current_user = nullptr;
}

User* AuthSystem::get_current_user() const {
    return current_user;
}

bool AuthSystem::has_permission(UserRole required_role) const {
    if (!current_user) return false;
    return static_cast<int>(current_user->role) >= static_cast<int>(required_role);
}

void AuthSystem::print_user_info() const {
    if (current_user) {
        string role_str;
        switch (current_user->role) {
        case UserRole::USER: role_str = "Пользователь"; break;
        case UserRole::ADMIN: role_str = "Администратор"; break;
        default: role_str = "Гость";
        }
        cout << "Текущий пользователь: " << current_user->username
            << " [" << role_str << "]\n";
    }
    else {
        cout << "Не авторизован\n";
    }
}

bool AuthSystem::change_password(const string& old_password, const string& new_password) {
    if (!current_user) {
        cerr << "Ошибка: Нет активного пользователя.\n";
        return false;
    }
    if (old_password.empty() || new_password.empty()) {
        cerr << "Ошибка: Пароли не могут быть пустыми.\n";
        return false;
    }
    if (new_password.length() < 4) {
        cerr << "Ошибка: Новый пароль должен содержать минимум 4 символа.\n";
        return false;
    }

    string encrypted_old = simple_encrypt(old_password);
    if (current_user->password_encrypted != encrypted_old) {
        cerr << "Ошибка: Старый пароль неверен.\n";
        return false;
    }

    current_user->password_encrypted = simple_encrypt(new_password);

    if (save_users()) {
        return true;
    }
    else {
        cerr << "Ошибка: Не удалось сохранить изменения.\n";
        current_user->password_encrypted = encrypted_old;
        return false;
    }
}

bool AuthSystem::change_user_password(const string& username, const string& new_password) {
    if (!current_user || current_user->role != UserRole::ADMIN) {
        cerr << "Ошибка: Недостаточно прав.\n";
        return false;
    }
    if (username.empty() || new_password.empty()) {
        cerr << "Ошибка: Имя пользователя и пароль не могут быть пустыми.\n";
        return false;
    }
    if (new_password.length() < 4) {
        cerr << "Ошибка: Пароль должен содержать минимум 4 символа.\n";
        return false;
    }
    auto it = users.find(username);
    if (it == users.end()) {
        cerr << "Ошибка: Пользователь не найден.\n";
        return false;
    }

    it->second.password_encrypted = simple_encrypt(new_password);

    if (save_users()) {
        cout << "Пароль пользователя '" << username << "' успешно изменен!\n";
        return true;
    }
    else {
        cerr << "Ошибка: Не удалось сохранить изменения.\n";
        return false;
    }
}

bool AuthSystem::add_user(const string& username, const string& password, UserRole role) {
    if (username.empty()) {
        cerr << "Ошибка: Имя пользователя не может быть пустым.\n";
        return false;
    }
    if (!all_of(username.begin(), username.end(),
        [](char c) { return isalnum(c) || c == '_' || c == '-'; })) {
        cerr << "Ошибка: Имя пользователя может содержать только буквы, цифры, '_' и '-'.\n";
        return false;
    }
    if (password.length() < 4) {
        cerr << "Ошибка: Пароль должен содержать минимум 4 символа.\n";
        return false;
    }
    if (users.find(username) != users.end()) {
        return false;
    }
    users.emplace(username, User(username, simple_encrypt(password), role));
    if (save_users()) {
        cout << "Пользователь '" << username << "' успешно добавлен и сохранен!\n";
        return true;
    }
    else {
        cerr << "Ошибка: Не удалось сохранить пользователя в файл.\n";
        users.erase(username);
        return false;
    }
}

bool AuthSystem::save_users() {
    ofstream file(users_filename, ios::binary);
    if (!file.is_open()) {
        cerr << "Ошибка: Не удалось открыть файл для сохранения пользователей: "
            << users_filename << "\n";
        return false;
    }
    try {
        size_t user_count = users.size();
        file.write(reinterpret_cast<const char*>(&user_count), sizeof(user_count));

        for (const auto& pair : users) {
            const string& username = pair.first;
            const User& user = pair.second;

            size_t name_len = username.length();
            file.write(reinterpret_cast<const char*>(&name_len), sizeof(name_len));

            file.write(username.c_str(), name_len);

            size_t hash_len = user.password_encrypted.length();
            file.write(reinterpret_cast<const char*>(&hash_len), sizeof(hash_len));

            file.write(user.password_encrypted.c_str(), hash_len);

            UserRole role = user.role;
            file.write(reinterpret_cast<const char*>(&role), sizeof(role));
        }
        file.close();
        return true;
    }
    catch (const exception& e) {
        cerr << "Ошибка при сохранении пользователей: " << e.what() << "\n";
        file.close();
        return false;
    }
}

bool AuthSystem::load_users() {
    ifstream file(users_filename, ios::binary);
    if (!file.is_open()) {
        return false;
    }
    try {
        users.clear();

        size_t user_count = 0;
        file.read(reinterpret_cast<char*>(&user_count), sizeof(user_count));

        if (user_count > 1000) {
            cerr << "Ошибка: Некорректное количество пользователей в файле: "
                << user_count << "\n";
            file.close();
            return false;
        }
        for (size_t i = 0; i < user_count; ++i) {
            size_t name_len = 0;
            file.read(reinterpret_cast<char*>(&name_len), sizeof(name_len));
            if (name_len == 0 || name_len > 100) {
                cerr << "Ошибка: Некорректная длина имени пользователя: "
                    << name_len << "\n";
                file.close();
                return false;
            }

            string username(name_len, '\0');
            file.read(&username[0], name_len);

            size_t hash_len = 0;
            file.read(reinterpret_cast<char*>(&hash_len), sizeof(hash_len));

            if (hash_len == 0 || hash_len > 256) {
                cerr << "Ошибка: Некорректная длина хеша пароля: "
                    << hash_len << "\n";
                file.close();
                return false;
            }

            string password_hash(hash_len, '\0');
            file.read(&password_hash[0], hash_len);

            UserRole role = UserRole::GUEST;
            file.read(reinterpret_cast<char*>(&role), sizeof(role));

            if (role != UserRole::GUEST && role != UserRole::USER && role != UserRole::ADMIN) {
                role = UserRole::USER;
                cerr << "Предупреждение: Исправлена некорректная роль для пользователя "
                    << username << "\n";
            }
            users.emplace(username, User(username, password_hash, role));
        }
        file.close();
        return true;
    }
    catch (const exception& e) {
        cerr << "Ошибка при загрузке пользователей: " << e.what() << "\n";
        users.clear();
        file.close();
        return false;
    }
}

void AuthSystem::print_all_users() const {
    cout << "\nСписок зарегистрированных пользователей:\n";
    cout << "========================================\n";

    for (const auto& pair : users) {
        const string& username = pair.first;
        const User& user = pair.second;
        string role_str;
        switch (user.role) {
        case UserRole::USER: role_str = "Пользователь"; break;
        case UserRole::ADMIN: role_str = "Администратор"; break;
        default: role_str = "Гость";
        }
        cout << "  " << username << " [" << role_str << "]\n";
    }
    cout << "========================================\n";
}

vector<string> AuthSystem::get_all_usernames() const {
    vector<string> usernames;
    for (const auto& pair : users) {
        usernames.push_back(pair.first);
    }
    return usernames;
}

bool AuthSystem::delete_user(const string& username) {

    auto it = users.find(username);
    if (it == users.end()) {
        cerr << "Ошибка: Пользователь '" << username << "' не найден.\n";
        return false;
    }

    if (current_user && current_user->username == username) {
        cerr << "Ошибка: Нельзя удалить текущего пользователя.\n";
        return false;
    }

    const User& user_to_delete = it->second;

    if (user_to_delete.role == UserRole::ADMIN) {
        int admin_count = 0;
        for (const auto& pair : users) {
            const User& user = pair.second;
            if (user.role == UserRole::ADMIN) {
                admin_count++;
            }
        }
        if (admin_count <= 1) {
            cerr << "Ошибка: Нельзя удалить последнего администратора.\n";
            return false;
        }
    }

    users.erase(username);

    if (save_users()) {
        cout << "Пользователь '" << username << "' успешно удален.\n";
        return true;
    }
    else {
        cerr << "Ошибка при сохранении изменений.\n";
        return false;
    }
}
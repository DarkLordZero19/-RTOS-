#ifndef AUTH_SYSTEM_H
#define AUTH_SYSTEM_H

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <functional>
#include <cctype>
#include <memory>
using namespace std;

enum class UserRole {
    GUEST,
    USER,
    ADMIN
};

struct User {
    std::string username;
    std::string password_encrypted;
    UserRole role;

    User() : username(""), password_encrypted(""), role(UserRole::GUEST) {}

    User(const std::string& name, const std::string& pwd, UserRole r)
        : username(name), password_encrypted(pwd), role(r) {}

    User(const User& other)
        : username(other.username), password_encrypted(other.password_encrypted), role(other.role) {}

    User& operator=(const User& other) {
        if (this != &other) {
            username = other.username;
            password_encrypted = other.password_encrypted;
            role = other.role;
        }
        return *this;
    }
};

class AuthSystem {
private:
    std::map<std::string, User> users;
    User* current_user;
    std::string users_filename; 

    std::string simple_encrypt(const std::string& input) {
        std::string result = input;
        for (char& c : result) {
            c = c ^ 0x55;
        }
        return result;
    }
    std::string simple_decrypt(const std::string& input) {
        return simple_encrypt(input);
    }

public:
    AuthSystem(const std::string& filename = "logs/users.dat");

    bool login(const std::string& username, const std::string& password);

    void logout();

    User* get_current_user() const;

    bool has_permission(UserRole required_role) const;

    void print_user_info() const;

    bool change_password(const std::string& old_password, const std::string& new_password);

    bool change_user_password(const std::string& username, const std::string& new_password);

    bool add_user(const std::string& username, const std::string& password, UserRole role);

    bool delete_user(const std::string& username);

    bool save_users(); 

    bool load_users();

    void print_all_users() const;

    std::vector<std::string> get_all_usernames() const;
};
#endif
#include "utils/file_manager.h"
using namespace std;

namespace utils {
    bool FileManager::save_tasks(const string& filename,
        const vector<string>& task_data) {
        ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        for (const auto& task : task_data) {
            file << task << endl;
        }
        file.close();
        return true;
    }

    bool FileManager::load_tasks(const string& filename, vector<string>& task_data) {

        if (filename.empty() || filename.length() > 255) {
            return false;
        }
        ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        task_data.clear();
        string line;

        size_t total_size = 0;
        const size_t MAX_FILE_SIZE = 1024 * 1024;

        while (getline(file, line)) {
            total_size += line.length();
            if (total_size > MAX_FILE_SIZE) {
                file.close();
                return false;
            }

            if (!line.empty()) {
                task_data.push_back(line);
            }
        }
        file.close();
        return true;
    }

    bool FileManager::file_exists(const string& filename) {
        ifstream file(filename);
        bool exists = file.good();
        file.close();
        return exists;
    }
}
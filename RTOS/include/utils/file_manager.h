#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
using namespace std;

namespace utils {
    class FileManager {
    public:
        static bool save_tasks(const string& filename,
            const vector<string>& task_data);
        static bool load_tasks(const string& filename,
            vector<string>& task_data);
        static bool file_exists(const string& filename);
    };
}
#include <iostream>
#include <string>
#include <clocale>
#include <locale>
#include <codecvt>
#ifdef _WIN32
#include <windows.h>
#endif
using namespace std;

namespace utils {
    class EncodingHelper {
    public:
        static void setup_russian_console() {
#ifdef _WIN32
            SetConsoleCP(1251);
            SetConsoleOutputCP(1251);
#endif
            setlocale(LC_ALL, "Russian");
            locale::global(locale(""));
        }

        static void restore_console() {
#ifdef _WIN32
            SetConsoleCP(866);
            SetConsoleOutputCP(866);
#endif
        }

#ifdef _WIN32
        static wstring to_wide(const string& str) {
            if (str.empty()) return L"";
            int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
            wstring wstr(size_needed, 0);
            MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size_needed);
            return wstr;
        }
        static string from_wide(const wstring& wstr) {
            if (wstr.empty()) return "";
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
            string str(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size_needed, NULL, NULL);
            return str;
        }
#endif
    };
}
#include "system.h"

#include <filesystem>

#ifdef _WIN32
#  include <windows.h>
#  include <shellapi.h>
#else
#  include <limits.h>
#  include <sys/stat.h>
#  include <utime.h>
#  ifdef __APPLE__
#    include <crt_externs.h>
#    include <mach-o/dyld.h>
#  endif
#endif

#include "utils.h"

namespace SysCmdLine {

    std::string wideToUtf8(const std::wstring &utf16Str) {
#ifdef _WIN32
        if (utf16Str.empty()) {
            return {};
        }
        const auto utf16Length = static_cast<int>(utf16Str.size());
        if (utf16Length >= (std::numeric_limits<int>::max)()) {
            return {};
        }
        const int utf8Length = ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, utf16Str.data(),
                                                     utf16Length, nullptr, 0, nullptr, nullptr);
        if (utf8Length <= 0) {
            return {};
        }
        std::string utf8Str;
        utf8Str.resize(utf8Length);
        ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, utf16Str.data(), utf16Length,
                              utf8Str.data(), utf8Length, nullptr, nullptr);
        return utf8Str;
#else
        return std::filesystem::path(utf16Str).string();
#endif
    }

    std::wstring utf8ToWide(const std::string &utf8Str) {
#ifdef _WIN32
        if (utf8Str.empty()) {
            return {};
        }
        const auto utf8Length = static_cast<int>(utf8Str.size());
        if (utf8Length >= (std::numeric_limits<int>::max)()) {
            return {};
        }
        const int utf16Length = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8Str.data(),
                                                      utf8Length, nullptr, 0);
        if (utf16Length <= 0) {
            return {};
        }
        std::wstring utf16Str;
        utf16Str.resize(utf16Length);
        ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8Str.data(), utf8Length,
                              utf16Str.data(), utf16Length);
        return utf16Str;
#else
        return std::filesystem::path(utf8Str).wstring();
#endif
    }

    std::string appPath() {
#ifdef _WIN32
        UINT bufferSize = MAX_PATH;
        std::vector<wchar_t> buffer(bufferSize + 1);
        bufferSize = ::GetModuleFileNameW(nullptr, buffer.data(), bufferSize);
        if (bufferSize > MAX_PATH) {
            buffer.resize(bufferSize);
            ::GetModuleFileNameW(nullptr, buffer.data(), bufferSize);
        }
        std::replace(buffer.begin(), buffer.end(), L'\\', L'/');
        return wideToUtf8(buffer.data());
#elif defined(__APPLE__)
        unsigned int bufferSize = PATH_MAX;
        std::vector<char> buffer(bufferSize + 1);
        // "_NSGetExecutablePath" will return "-1" if the buffer is not large enough
        // and "*bufferSize" will be set to the size required.
        if (_NSGetExecutablePath(buffer.data(), &bufferSize) != 0) {
            buffer.resize(bufferSize);
            _NSGetExecutablePath(buffer.data(), &bufferSize);
        }
        return std::string(buffer.data());
#else
        char buf[PATH_MAX];
        if (!realpath("/proc/self/exe", buf)) {
            return {};
        }
        return buf;
#endif
    }

    std::string appDirectory() {
        auto appDir = appPath();
        auto slashIdx = appDir.find_last_of('/');
        if (slashIdx != std::string::npos) {
            appDir = appDir.substr(0, slashIdx);
        }
        return appDir;
    }

    std::string appFileName() {
        auto appName = appPath();
        auto slashIdx = appName.find_last_of('/');
        if (slashIdx != std::string::npos) {
            appName = appName.substr(slashIdx + 1);
        }
        return appName;
    }

    std::string appName() {
        auto appName = appFileName();
#ifdef _WIN32
        auto dotIdx = appName.find_last_of('.');
        if (dotIdx != std::string::npos) {
            std::string suffix = Utils::toLower(appName.substr(dotIdx + 1));
            if (suffix == "exe") {
                appName = appName.substr(0, dotIdx);
            }
        }
#endif
        return appName;
    }

    std::vector<std::string> commandLineArguments() {
        std::vector<std::string> res;
#ifdef _WIN32
        int argc;
        auto argvW = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
        if (argvW == nullptr)
            return {};
        res.reserve(argc);
        for (int i = 0; i != argc; ++i) {
            res.push_back(wideToUtf8(argvW[i]));
        }
        ::LocalFree(argvW);
#elif defined(__APPLE__)
        auto argv = *(_NSGetArgv());
        for (int i = 0; argv[i] != nullptr; ++i) {
            res.push_back(argv[i]);
        }
#else
        std::ifstream file("/proc/self/cmdline", std::ios::in);
        if (file.fail())
            return {};
        std::string s;
        while (std::getline(file, s, '\0')) {
            res.push_back(s);
        }
        file.close();
#endif
        return res;
    }

    class PrintScopeGuard {
    public:
        enum Color {
            NoColor,
            Red,
            Green,
            Yellow,
        };

        explicit PrintScopeGuard(Color color = NoColor) : _color(color) {
#ifdef _WIN32
            _codepage = ::GetConsoleOutputCP();
            ::SetConsoleOutputCP(CP_UTF8);

            if (color != NoColor) {
                WORD winColor = FOREGROUND_INTENSITY;
                switch (color) {
                    case Red:
                        winColor |= FOREGROUND_RED;
                        break;
                    case Green:
                        winColor |= FOREGROUND_GREEN;
                        break;
                    case Yellow:
                        winColor |= FOREGROUND_RED | FOREGROUND_GREEN;
                        break;
                    default:
                        break;
                }
                _hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                GetConsoleScreenBufferInfo(_hConsole, &_csbi);
                SetConsoleTextAttribute(_hConsole, winColor);
            }
#else
            if (color != NoColor) {
                // ANSI escape code to set text color to red
                const char *colorStr;
                switch (color) {
                    case Red:
                        colorStr = "\033[91m";
                        break;
                    case Green:
                        colorStr = "\033[92m";
                        break;
                    case Yellow:
                        colorStr = "\033[93m";
                        break;
                    default:
                        break;
                }
                std::cout << colorStr;
            }
#endif
        }

        ~PrintScopeGuard() {
#ifdef _WIN32
            ::SetConsoleOutputCP(_codepage);

            if (_color != NoColor) {
                SetConsoleTextAttribute(_hConsole, _csbi.wAttributes);
            }
#else
            if (_color != NoColor) {
                // ANSI escape code to reset text color to default
                const char *resetColor = "\033[0m";
                std::cout << resetColor;
            }
#endif
        }

    private:
        Color _color;
#ifdef _WIN32
        UINT _codepage;
        HANDLE _hConsole;
        CONSOLE_SCREEN_BUFFER_INFO _csbi;
#endif
    };

    int u8printf(const char *fmt, ...) {
        PrintScopeGuard _guard;

        va_list args;
        va_start(args, fmt);
        int res = vprintf(fmt, args);
        va_end(args);
        return res;
    }

    int u8error(const char *fmt, ...) {
        PrintScopeGuard _guard(PrintScopeGuard::Red);

        va_list args;
        va_start(args, fmt);
        int res = vprintf(fmt, args);
        va_end(args);
        return res;
    }

    int u8warning(const char *fmt, ...) {
        PrintScopeGuard _guard(PrintScopeGuard::Yellow);

        va_list args;
        va_start(args, fmt);
        int res = vprintf(fmt, args);
        va_end(args);
        return res;
    }

}
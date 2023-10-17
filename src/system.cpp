#include "system.h"

#include <filesystem>

#ifdef _WIN32
#  include <Windows.h>
#else
#  include <limits.h>
#  include <sys/stat.h>
#  include <utime.h>
#  ifdef __APPLE__
#    include <crt_externs.h>
#    include <mach-o/dyld.h>
#  endif
#endif

namespace SysCmdLine {

    std::string wideToUtf8(const std::wstring &s) {
#ifdef _WIN32
        int len =
            WideCharToMultiByte(CP_UTF8, 0, s.data(), (int) s.size(), nullptr, 0, nullptr, nullptr);
        auto buf = new char[len + 1];
        WideCharToMultiByte(CP_UTF8, 0, s.data(), (int) s.size(), buf, len, nullptr, nullptr);
        buf[len] = '\0';

        std::string res(buf);
        delete[] buf;
        return res;
#else
        return std::filesystem::path(s).string();
#endif
    }

    std::wstring utf8ToWide(const std::string &s) {
#ifdef _WIN32
        int len = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int) s.size(), nullptr, 0);
        auto buf = new wchar_t[len + 1];
        MultiByteToWideChar(CP_UTF8, 0, s.data(), (int) s.size(), buf, len);
        buf[len] = '\0';

        std::wstring res(buf);
        delete[] buf;
        return res;
#else
        return std::filesystem::path(s).wstring();
#endif
    }

    std::string appPath() {
#ifdef _WIN32
        wchar_t buf[MAX_PATH];
        if (!::GetModuleFileNameW(nullptr, buf, MAX_PATH)) {
            return {};
        }
        std::wstring wstr = buf;
        std::replace(wstr.begin(), wstr.end(), L'\\', L'/');
        return wideToUtf8(wstr);
#elif defined(__APPLE__)
        char buf[PATH_MAX];
        uint32_t size = sizeof(buf);
        if (_NSGetExecutablePath(buf, &size) != 0) {
            return {};
        }
        return buf;
#else
        char buf[PATH_MAX];
        if (!realpath("/proc/self/exe", buf)) {
            return {};
        }
        return buf;
#endif
    }

    std::string appName() {
        auto appName = appPath();
        auto slashIdx = appName.find_last_of('/');
        if (slashIdx != std::string::npos) {
            appName = appName.substr(slashIdx + 1);
        }

#ifdef _WIN32
        auto dotIdx = appName.find_last_of('.');
        if (dotIdx != std::string::npos) {
            std::string suffix = appName.substr(dotIdx + 1);
            std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);
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

    int u8printf(const char *fmt, ...) {
#ifdef _WIN32
        auto codepage = ::GetConsoleOutputCP();
        ::SetConsoleOutputCP(CP_UTF8);
#endif
        va_list args;
        va_start(args, fmt);
        int res = vprintf(fmt, args);
        va_end(args);
#ifdef _WIN32
        ::SetConsoleOutputCP(codepage);
#endif
        return res;
    }

    int u8errprint(const char *fmt, ...) {
        // ANSI escape code to set text color to red
        const char* redColor = "\033[31m";
        printf("%s", redColor);

        va_list args;
        va_start(args, fmt);
        int res = u8printf(fmt, args);
        va_end(args);

        // ANSI escape code to reset text color to default
        const char* resetColor = "\033[0m";
        printf("%s", resetColor);
        return 0;
    }

}
#ifndef SYSTEM
#define SYSTEM

#include <string>
#include <vector>

#include <syscmdline/global.h>

namespace SysCmdLine {

    SYSCMDLINE_EXPORT std::string wideToUtf8(const std::wstring &s);
    SYSCMDLINE_EXPORT std::wstring utf8ToWide(const std::string &s);

    SYSCMDLINE_EXPORT std::string appPath();
    SYSCMDLINE_EXPORT std::string appDirectory();
    SYSCMDLINE_EXPORT std::string appFileName();
    SYSCMDLINE_EXPORT std::string appName();
    SYSCMDLINE_EXPORT std::vector<std::string> commandLineArguments();

    SYSCMDLINE_EXPORT int u8printf(const char *fmt, ...);
    SYSCMDLINE_EXPORT int u8error(const char *fmt, ...);
    SYSCMDLINE_EXPORT int u8warning(const char *fmt, ...);

}

#endif // SYSTEM

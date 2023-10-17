#ifndef SYSTEM
#define SYSTEM

#include <string>
#include <vector>

namespace SysCmdLine {

    std::string wideToUtf8(const std::wstring &s);
    std::wstring utf8ToWide(const std::string &s);

    std::string appPath();
    std::string appName();
    std::vector<std::string> commandLineArguments();

    void u8printf(const char *fmt, ...);

}

#endif // SYSTEM

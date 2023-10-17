#include <string>

namespace SysCmdLine::Strings {

    const char *error_strings[] = {
        "",
        R"(Unknown option "%1".)",
        R"(Unknown command or argument "%1".)",
        R"(Missing required argument "%1" of option "%2".)",
        R"(Missing required argument "%1".)",
        "Too many arguments.",
        R"(Invalid value "%1" of argument "%2".)",
    };

    const char *common_strings[] = {
        "Error", "Usage", "Description", "Arguments", "Options", "Commands",
    };

    const char *info_strings[] = {
        "Show version information",
        "Show help information",
    };

}
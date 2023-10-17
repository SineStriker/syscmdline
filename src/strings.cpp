#include <string>

namespace SysCmdLine::Strings {

    const char *error_strings[] = {
        "",
        "Unknown option \"{}\".",
        "Unknown command or argument \"{}\".",
        "Missing required argument \"{}\".",
        "Too many arguments",
    };

    const char *common_strings[] = {
        "Error", "Usage", "Description", "Arguments", "Options", "Commands",
    };

    const char *info_strings[] = {
        "Show version information",
        "Show help information",
    };

}
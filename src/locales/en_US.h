namespace SysCmdLine::Strings::en_US {

    static const char *error_strings[] = {
        R"(No error.)",
        R"(Unknown option "%1".)",
        R"(Unknown command or argument "%1".)",
        R"(Missing required argument "%1" of option "%2".)",
        R"(Missing required argument "%1".)",
        R"(Too many arguments.)",
        R"(Invalid value "%1" of argument "%2".)",
        R"(Invalid occurrence of option "%1", which should be argument "%2".)",
        R"(Missing required option "%1".)",
        R"(Option "%1" occurs too much, at most %2.)",
        R"(Invalid token "%1" of argument "%2", expect "%3".)",
        R"(Invalid token "%1" of argument "%2", reason: %3)",
        R"(Options "%1" and "%2" are mutually exclusive.)",
        R"(Option "%1" and other arguments cannot be specified simultaneously.)",
        R"(Option "%1" and other options cannot be specified simultaneously.)",
    };

    static const char *title_strings[] = {
        "Error",    "Usage",    "Description", "Arguments", "Options",
        "Commands", "Required", "Default",     "Expected",
    };

    static const char *command_strings[] = {
        "Show version information",
        "Show help information",
    };

    static const char *info_strings[] = {
        R"("%1" is not matched. Do you mean one of the following?)",
    };

    static const char **strings[] = {
        error_strings,
        title_strings,
        command_strings,
        info_strings,
    };

    static std::string provider(int category, int index) {
        return strings[category][index];
    }

}

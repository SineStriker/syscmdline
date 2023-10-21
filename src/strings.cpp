#include "strings.h"

namespace SysCmdLine::Strings {

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

    static std::string builtInTextProvider(TextCategory category, int index) {
        const char **arr;
        switch (category) {
            case ParseError:
                arr = error_strings;
                break;
            case Title:
                arr = title_strings;
                break;
            case DefaultCommand:
                arr = command_strings;
                break;
            case Information:
                arr = info_strings;
                break;
            default:
                return {};
        }
        return arr[index];
    }

    static TextProvider _provider = builtInTextProvider;

    void setTextProvider(TextProvider textProvider) {
        _provider = textProvider;
    }

    TextProvider textProvider() {
        return _provider;
    }

    std::string text(TextCategory category, int index) {
        return _provider(category, index);
    }

    static int _sizeConfig[] = {
        4,
        4,
        80,
    };

    int sizeConfig(int index) {
        return _sizeConfig[index];
    }

    void setSizeConfig(int index, int value) {
        _sizeConfig[index] = value;
    }

}
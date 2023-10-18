#include <string>

namespace SysCmdLine::Strings {

    const char *error_strings[] = {
        "",
        R"(Unknown option "%1".)",
        R"(Unknown command or argument "%1".)",
        R"(Missing required argument "%1" of option "%2".)",
        R"(Missing required argument "%1".)",
        R"(Too many arguments.)",
        R"(Invalid value "%1" of argument "%2".)",
    };

    const char *common_strings[] = {
        "Error", "Usage", "Description", "Arguments", "Options", "Commands",
    };

    const char *info_strings[] = {
        "Show version information",
        "Show help information",
    };

    // const char *error_strings[] = {
    //     "",
    //     R"(未定义的选项 "%1"。)",
    //     R"(未定义的参数或命令 "%1"。)",
    //     R"(选项 "%2" 缺少必选参数 "%1"。)",
    //     R"(缺少必选参数 "%1"。)",
    //     "参数 数量过多。",
    //     R"(参数 "%2" 指定为非法值 "%1"。)",
    // };

    // const char *common_strings[] = {
    //     "错误", "用法", "简介", "参数", "选项", "命令",
    // };

    // const char *info_strings[] = {
    //     "显示版本信息",
    //     "显示帮助信息",
    // };

}
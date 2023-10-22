namespace SysCmdLine::Strings::zh_CN {

    static const char *error_strings[] = {
        R"(没有错误。)",
        R"(未知的选项 "%1"。)",
        R"(未知的命令或参数 "%1"。)",
        R"(选项 "%2" 缺少必选参数 "%1"。)",
        R"(缺少必选参数 "%1"。)",
        R"(参数过多。)",
        R"(参数 "%2" 被指定非法的值 "%1"。)",
        R"(选项 "%1" 出现在非法的位置，此处应为参数 "%2"。)",
        R"(缺少必选选项 "%1"。)",
        R"(选项 "%1" 出现次数过多，最多 %2 次。)",
        R"(参数 "%2" 被指定非法的值 "%1"，应为 "%3"。)",
        R"(参数 "%2" 被指定非法的值 "%1"，原因: %3)",
        R"(选项 "%1" 与 "%2" 是互斥的。)",
        R"(选项 "%1" 与其他参数不能同时指定。)",
        R"(选项 "%1" 与其他选项不能同时指定。)",
    };

    static const char *title_strings[] = {
        "错误", "用法", "简介", "参数", "选项", "命令", "必选", "默认", "合法值",
    };

    static const char *command_strings[] = {
        "显示版本信息",
        "显示帮助信息",
    };

    static const char *info_strings[] = {
        R"("%1" 未匹配。您是否想指定以下之一？)",
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

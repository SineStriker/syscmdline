#include <iostream>
#include <filesystem>

#include <syscmdline/parser.h>
#include <syscmdline/system.h>
#include <syscmdline/strings.h>

using namespace SysCmdLine;

namespace CN {

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
    };

    static const char *title_strings[] = {
        "错误", "用法", "简介", "参数", "选项", "命令",
    };

    static const char *command_strings[] = {
        "显示版本信息",
        "显示帮助信息",
    };

    static const char *info_strings[] = {
        R"("%1" 未匹配。您是否想指定以下之一？)",
    };

    static std::string textProvider(Strings::TextCategory category, int index) {
        const char **arr;
        switch (category) {
            case Strings::ParseError:
                arr = error_strings;
                break;
            case Strings::Title:
                arr = title_strings;
                break;
            case Strings::DefaultCommand:
                arr = command_strings;
                break;
            case Strings::Information:
                arr = info_strings;
                break;
            default:
                return {};
        }
        return arr[index];
    }

}

static int routine(const ParseResult &result) {
    auto fileValues = Value::toStringList(result.valuesForArgument("files"));
    u8printf("将要被删除的文件：\n");
    for (const auto &item : std::as_const(fileValues)) {
        std::cout << Strings::indent << item << std::endl;
    }

    bool prompt = result.optionIsSet("/P");
    bool force = result.optionIsSet("/F");
    bool subdir = result.optionIsSet("/S");
    bool quiet = result.optionIsSet("/Q");

    u8printf("模式: \n");
    if (prompt) {
        std::cout << Strings::indent;
        u8printf("提示\n");
    }
    if (force) {
        std::cout << Strings::indent;
        u8printf("强制\n");
    }
    if (subdir) {
        std::cout << Strings::indent;
        u8printf("子文件夹\n");
    }
    if (quiet) {
        std::cout << Strings::indent;
        u8printf("静默\n");
    }

    return 0;
}

int main(int argc, char *argv[]) {
    SYSCMDLINE_UNUSED(argc);
    SYSCMDLINE_UNUSED(argv);

    Strings::setTextProvider(CN::textProvider);

    Option promptOption("/P", "删除每一个文件之前提示确认。");
    Option forceOption("/F", "强制删除只读文件。");
    Option subdirOption("/S", "删除所有子目录中的指定的文件。");
    Option quietOption("/Q", "安静模式。删除全局通配符时，不要求确认。");

    Argument fileArg("files", "指定一个或多个文件或者目录列表。");
    fileArg.setDisplayName("files");
    fileArg.setMultiValueEnabled(true);

    Command rootCommand("del");
    rootCommand.addArgument(fileArg);
    rootCommand.setOptions({
        promptOption,
        forceOption,
        subdirOption,
        quietOption,
    });
    rootCommand.addHelpOption(false, false, {"/?"});
    rootCommand.setHandler(routine);

    Parser parser(rootCommand);
    parser.setDisplayOptions(Parser::ShowOptionalOptionsOnUsage);
    return parser.invoke(commandLineArguments(), -1, Parser::IgnoreOptionCase);
}
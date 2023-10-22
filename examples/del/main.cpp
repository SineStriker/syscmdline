#include <iostream>
#include <filesystem>

#include <syscmdline/parser.h>
#include <syscmdline/system.h>
#include <syscmdline/strings.h>

using namespace SysCmdLine;

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

    Strings::setTextProvider(Strings::textProviderForLocale(Strings::SimplifiedChinese));

    Option promptOption("/P", "删除每一个文件之前提示确认");
    Option forceOption("/F", "强制删除只读文件");
    Option subdirOption("/S", "删除所有子目录中的指定的文件");
    Option quietOption("/Q", "安静模式。删除全局通配符时，不要求确认");

    Argument fileArg("files", "指定一个或多个文件或者目录列表");
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
    parser.setDisplayOptions(Parser::ShowOptionalOptionsOnUsage | Parser::ShowArgumentIsRequired);
    return parser.invoke(commandLineArguments(), -1, Parser::IgnoreOptionCase);
}
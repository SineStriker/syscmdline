#ifndef SYSCMDLINE_STRINGS_H
#define SYSCMDLINE_STRINGS_H

#include <string>

#include <syscmdline/global.h>

namespace SysCmdLine::Strings {

    // Text Configuration

    enum TextCategory {
        ParseError,
        Title,
        DefaultCommand,
        Information,
    };

    enum TitleText {
        Error,
        Usage,
        Description,
        Arguments,
        Options,
        Commands,
    };

    enum DefaultCommandText {
        Version,
        Help,
    };

    enum InformationText {
        MatchCommand,
    };

    using TextProvider = std::string (*)(TextCategory /* category */, int /* index */);

    SYSCMDLINE_EXPORT void setTextProvider(TextProvider textProvider);

    SYSCMDLINE_EXPORT TextProvider textProvider();

    SYSCMDLINE_EXPORT std::string text(TextCategory category, int index);

    // Size Configuration

    enum SizeConfigIndex {
        Indent,
        Spacing,
        ConsoleWidth,
    };

    SYSCMDLINE_EXPORT int sizeConfig(int index);

    SYSCMDLINE_EXPORT void setSizeConfig(int index, int value);

}

#endif // SYSCMDLINE_STRINGS_H

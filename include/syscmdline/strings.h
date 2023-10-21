#ifndef STRINGS_H
#define STRINGS_H

#include <string>
#include <iomanip>

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
        Required,
        Default,
        ExpectedValues,
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

    template <class Char, class Traits>
    std::basic_ostream<Char, Traits> &indent(std::basic_ostream<Char, Traits> &out) {
        return out << std::setw(sizeConfig(Indent)) << out.widen(' ');
    }

    template <class Char, class Traits>
    std::basic_ostream<Char, Traits> &spacing(std::basic_ostream<Char, Traits> &out) {
        return out << std::setw(sizeConfig(Spacing)) << out.widen(' ');
    }

}

#endif // STRINGS_H

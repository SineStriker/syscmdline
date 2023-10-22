#ifndef HELPLAYOUT_H
#define HELPLAYOUT_H

#include <string>
#include <sstream>
#include <vector>
#include <functional>

#include <syscmdline/global.h>
#include <syscmdline/shareddata.h>

namespace SysCmdLine {

    class Parser;
    class ParserData;
    class ParseResult;
    class ParseResultData;

    class HelpLayoutData;

    class SYSCMDLINE_EXPORT HelpLayout {
    public:
        HelpLayout();
        ~HelpLayout();

        HelpLayout(const HelpLayout &other);
        HelpLayout(HelpLayout &&other) noexcept;
        HelpLayout &operator=(const HelpLayout &other);
        HelpLayout &operator=(HelpLayout &&other) noexcept;

        enum HelpItem {
            HI_CustomText,
            HI_Prologue,
            HI_Information,
            HI_Warning,
            HI_Error,
            HI_Description,
            HI_Usage,
            HI_Arguments,
            HI_Options,
            HI_Commands,
            HI_Epilogue,
        };

        enum MessageType {
            ML_Debug,
            ML_Healthy,
            ML_Highlight,
            ML_Warning,
            ML_Critical,
        };

        enum SizeType {
            ST_Indent,
            ST_Spacing,
            ST_ConsoleWidth,
        };

        using Printer = std::function<void(const std::vector<std::string> & /*lines */)>;

    public:
        bool isNull() const;

        int size(SizeType sizeType) const;
        void setSize(SizeType sizeType, int value);

        void addText(const std::string &text, MessageType messageType = ML_Debug);
        void addItem(HelpItem type, const Printer &printer = {});

    public:
        template <class Char, class Traits>
        std::basic_ostream<Char, Traits> &indent(std::basic_ostream<Char, Traits> &out) {
            return out << std::setw(size(ST_Indent)) << out.widen(' ');
        }

        template <class Char, class Traits>
        std::basic_ostream<Char, Traits> &spacing(std::basic_ostream<Char, Traits> &out) {
            return out << std::setw(size(ST_Spacing)) << out.widen(' ');
        }

    protected:
        SharedDataPointer<HelpLayoutData> d_ptr;

        friend class Parser;
        friend class ParserData;
        friend class ParseResult;
        friend class ParseResultData;
    };

}

#endif // HELPLAYOUT_H

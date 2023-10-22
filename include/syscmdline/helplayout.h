/****************************************************************************
 *
 * MIT License
 *
 * Copyright (c) 2023 SineStriker
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 ****************************************************************************/

#ifndef HELPLAYOUT_H
#define HELPLAYOUT_H

#include <string>
#include <sstream>
#include <iomanip>
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

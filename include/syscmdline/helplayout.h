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
        SYSCMDLINE_DECL_DATA(HelpLayout)
    public:
        HelpLayout();
        ~HelpLayout();

        HelpLayout(const HelpLayout &other);
        HelpLayout(HelpLayout &&other) noexcept;
        HelpLayout &operator=(const HelpLayout &other);
        HelpLayout &operator=(HelpLayout &&other) noexcept;

        enum HelpItem {
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

        enum SizeType {
            ST_Indent,
            ST_Spacing,
            ST_ConsoleWidth,
        };

        using Printer =
            std::function<void(const std::string & /* title */,
                               const std::vector<std::string> & /* lines */, bool /* hasNext */)>;

    public:
        bool isNull() const;

        int size(SizeType sizeType) const;
        void setSize(SizeType sizeType, int value);

        void addItem(HelpItem type, const Printer &printer = {});

    public:
        static HelpLayout defaultHelpLayout();

        friend class Parser;
        friend class ParserData;
        friend class ParseResult;
        friend class ParseResultData;
    };

}

#endif // HELPLAYOUT_H

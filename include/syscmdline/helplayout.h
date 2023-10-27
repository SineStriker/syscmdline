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

#include <syscmdline/sharedbase.h>

namespace SysCmdLine {

    class HelpLayoutPrivate;

    class SYSCMDLINE_EXPORT HelpLayout : public SharedBase {
        SYSCMDLINE_DECL_PRIVATE(HelpLayout)
    public:
        HelpLayout();
        ~HelpLayout();

        HelpLayout(const HelpLayout &other);
        HelpLayout(HelpLayout &&other) noexcept;
        HelpLayout &operator=(const HelpLayout &other);
        HelpLayout &operator=(HelpLayout &&other) noexcept;

        enum MessageItem {
            MI_Information,
            MI_Warning,
            MI_Critical,
        };

        enum HelpItem {
            HI_Prologue,
            HI_Description,
            HI_Usage,
            HI_Arguments,
            HI_Options,
            HI_Commands,
            HI_Epilogue,

            HI_User = 1000,
        };

        struct Text {
            std::string title;
            std::string lines;
        };

        struct List {
            std::string title;
            std::vector<std::string> firstColumn;
            std::vector<std::string> secondColumn;
        };

        using PlainPrinter = std::function<void(bool /* hasNext */)>;

        using TextPrinter =
            std::function<void(int /* id */, const std::string & /* title */,
                               const std::vector<std::string> & /* lines */, bool /* hasNext */)>;

        using ListPrinter =
            std::function<void(int /* id */, const std::string & /* title */,
                               std::vector<std::string> & /* firstColumn */,
                               std::vector<std::string> & /* secondColumn */, bool /* hasNext */)>;

    public:
        bool isNull() const;

        void addHelpItem(HelpItem type);
        void addMessageItem(MessageItem type);
        void addUserTextItem(const Text &text, int id = -1);
        void addUserListItem(const List &list, int id = -1);
        void addCustomItem(const PlainPrinter &printer);

        void setTextPrinter(const TextPrinter &printer);
        void setListPrinter(const ListPrinter &printer);

    public:
        static HelpLayout defaultHelpLayout();
    };

}

#endif // HELPLAYOUT_H

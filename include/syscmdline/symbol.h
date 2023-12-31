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

#ifndef SYMBOL_H
#define SYMBOL_H

#include <functional>
#include <string>

#include <syscmdline/sharedbase.h>

namespace SysCmdLine {

    class SymbolPrivate;

    class SYSCMDLINE_EXPORT Symbol : public SharedBase {
        SYSCMDLINE_DECL_PRIVATE(Symbol)
    public:
        enum SymbolType {
            ST_Command,
            ST_Option,
            ST_Argument,
        };

        enum HelpPosition {
            HP_Usage,
            HP_ErrorText,
            HP_FirstColumn,
            HP_SecondColumn,
        };

        using HelpProvider =
            std::function<std::string(const Symbol * /* this */, HelpPosition /* pos */,
                                      int /* displayOptions */, void * /* extra */)>;

        inline std::string helpText(HelpPosition pos, int displayOptions) const;
        virtual std::string helpText(HelpPosition pos, int displayOptions, void *extra) const;

    public:
        SymbolType type() const;

        std::string description() const;
        void setDescription(const std::string &desc);

        HelpProvider helpProvider() const;
        void setHelpProvider(const HelpProvider &helpProvider);

    protected:
        Symbol(SymbolPrivate *d);
    };

    inline std::string Symbol::helpText(Symbol::HelpPosition pos, int displayOptions) const {
        return helpText(pos, displayOptions, nullptr);
    }

}

#endif // SYMBOL_H

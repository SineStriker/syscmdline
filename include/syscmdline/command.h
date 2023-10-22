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

#ifndef COMMAND_H
#define COMMAND_H

#include <syscmdline/option.h>

namespace SysCmdLine {

    class CommandCatalogueData;

    class SYSCMDLINE_EXPORT CommandCatalogue {
        SYSCMDLINE_DECL_DATA(CommandCatalogue)
    public:
        CommandCatalogue();
        ~CommandCatalogue();

        CommandCatalogue(const CommandCatalogue &other);
        CommandCatalogue(CommandCatalogue &&other) noexcept;
        CommandCatalogue &operator=(const CommandCatalogue &other);
        CommandCatalogue &operator=(CommandCatalogue &&other) noexcept;

    public:
        void addArgumentCatalogue(const std::string &name, const std::vector<std::string> &args);
        void addOptionCatalogue(const std::string &name, const std::vector<std::string> &options);
        void addCommandCatalogue(const std::string &name, const std::vector<std::string> &commands);

    protected:
        SharedDataPointer<CommandCatalogueData> d_ptr;

        friend class Command;
        friend class CommandData;
        friend class Parser;
        friend class ParserData;
        friend class ParseResult;
        friend class ParseResultData;
    };

    class CommandData;

    class SYSCMDLINE_EXPORT Command : public ArgumentHolder {
        SYSCMDLINE_DECL_DATA(Command)
    public:
        using Handler = std::function<int /* code */ (const ParseResult & /* parser */)>;

        Command();
        Command(const std::string &name, const std::string &desc = {},
                const std::vector<std::pair<Option, int>> &options = {},
                const std::vector<Command> &subCommands = {},
                const std::vector<Argument> &args = {}, const std::string &detailedDescription = {},
                const Handler &handler = {});
        ~Command();

        Command(const Command &other);
        Command(Command &&other) noexcept;
        Command &operator=(const Command &other);
        Command &operator=(Command &&other) noexcept;

        using Symbol::helpText;
        std::string helpText(HelpPosition pos, int displayOptions, void *extra) const override;

    public:
        Command command(const std::string &name) const;
        Command command(int index) const;
        const std::vector<Command> &commands() const;
        int indexOfCommand(const std::string &name) const;
        bool hasCommand(const std::string &name) const;
        void addCommand(const Command &command);
        void setCommands(const std::vector<Command> &commands);

        Option option(const std::string &name) const;
        Option option(int index) const;
        Option optionFromToken(const std::string &token) const;
        const std::vector<Option> &options() const;
        int indexOfOption(const std::string &name) const;
        bool hasOption(const std::string &name) const;
        bool hasOptionToken(const std::string &token) const;
        void addOption(const Option &option, int exclusiveGroup = -1);
        void setOptions(const std::vector<Option> &options);
        void setOptions(const std::vector<std::pair<Option, int /* exclusiveGroup */>> &options);

        std::vector<int> exclusiveGroups() const;
        std::vector<Option> exclusiveGroupOptions(int group) const;

        std::string detailedDescription() const;
        void setDetailedDescription(const std::string &detailedDescription);

        void addVersionOption(const std::string &ver, const std::vector<std::string> &tokens = {});
        void addHelpOption(bool showHelpIfNoArg = false, bool global = false,
                           const std::vector<std::string> &tokens = {});

        Handler handler() const;
        void setHandler(const Handler &handler);

        CommandCatalogue catalogue() const;
        void setCatalogue(const CommandCatalogue &catalogue);

        std::string version() const;

    protected:
        friend class Parser;
        friend class ParserData;
        friend class ParseResult;
        friend class ParseResultData;
    };

}

#endif // COMMAND_H

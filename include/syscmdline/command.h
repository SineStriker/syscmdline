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

    class ParseResult;

    class CommandCataloguePrivate;

    class SYSCMDLINE_EXPORT CommandCatalogue : public SharedBase {
        SYSCMDLINE_DECL_PRIVATE(CommandCatalogue)
    public:
        CommandCatalogue();

    public:
        void addArguments(const std::string &name, const std::vector<std::string> &args);
        void addOptions(const std::string &name, const std::vector<std::string> &options);
        void addCommands(const std::string &name, const std::vector<std::string> &commands);
    };

    class CommandPrivate;

    class SYSCMDLINE_EXPORT Command : public ArgumentHolder {
        SYSCMDLINE_DECL_PRIVATE(Command)
    public:
        using Handler = std::function<int /* code */ (const ParseResult & /* result */)>;

        Command();
        Command(const std::string &name, const std::string &desc = {});

        using Symbol::helpText;
        std::string helpText(HelpPosition pos, int displayOptions, void *extra) const override;

    public:
        std::string name() const;
        void setName(const std::string &name);

        int commandCount() const;
        Command command(int index) const;
        inline void addCommand(const Command &command);
        void addCommands(const std::vector<Command> &commands);

        int optionCount() const;
        Option option(int index) const;
        void addOption(const Option &option, const std::string &group = {});
        inline void addOptions(const std::vector<Option> &options, const std::string &group = {});

        std::string detailedDescription() const;
        void setDetailedDescription(const std::string &detailedDescription);

        Handler handler() const;
        void setHandler(const Handler &handler);

        CommandCatalogue catalogue() const;
        void setCatalogue(const CommandCatalogue &catalogue);

        std::string version() const;

        void addVersionOption(const std::string &ver, const std::vector<std::string> &tokens = {});
        void addHelpOption(bool showHelpIfNoArg = false, bool global = false,
                           const std::vector<std::string> &tokens = {});
    };

    inline void Command::addCommand(const Command &command) {
        addCommands({command});
    }

    inline void Command::addOptions(const std::vector<Option> &options, const std::string &group) {
        for (const auto &opt : options)
            addOption(opt, group);
    }

    // Check if a command is valid
    // Parsing an invalid command will cause UB
    SYSCMDLINE_EXPORT bool assertCommand(const Command &command);

}

#endif // COMMAND_H

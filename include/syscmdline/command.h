#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>
#include <functional>

#include <syscmdline/option.h>

namespace SysCmdLine {

    class CommandCatalogueData;

    class SYSCMDLINE_EXPORT CommandCatalogue {
    public:
        CommandCatalogue();
        ~CommandCatalogue();

        CommandCatalogue(const CommandCatalogue &other);
        CommandCatalogue(CommandCatalogue &&other) noexcept;
        CommandCatalogue &operator=(const CommandCatalogue &other);
        CommandCatalogue &operator=(CommandCatalogue &&other) noexcept;

    public:
        void addArgumentCategory(const std::string &name, const std::vector<std::string> &args);
        void addOptionCategory(const std::string &name, const std::vector<std::string> &options);
        void addCommandCatalogue(const std::string &name, const std::vector<std::string> &commands);

    protected:
        SharedDataPointer<CommandCatalogueData> d;

        friend class Command;
        friend class CommandData;
    };

    class CommandData;

    class SYSCMDLINE_EXPORT Command : public ArgumentHolder {
    public:
        using Handler = std::function<int /* code */ (const Parser & /* parser */)>;

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

        std::string displayedArguments() const override;

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

        bool multipleArgumentsEnabled() const;
        void setMultipleArgumentsEnabled(bool on);

        void addVersionOption(const std::string &ver, const std::vector<std::string> &tokens = {});
        void addHelpOption(bool showHelpIfNoArg = false, bool global = false,
                           const std::vector<std::string> &tokens = {});

        Handler handler() const;
        void setHandler(const Handler &handler);

        CommandCatalogue catalogue() const;
        void setCatalogue(const CommandCatalogue &catalogue);

        std::string version() const;
        std::string helpText(const std::vector<std::string> &cmd = {},
                             const std::vector<const Option *> &globalOptions = {},
                             int parserOptions = 0) const;

    protected:
        CommandData *d_func();
        const CommandData *d_func() const;

        friend class Parser;
        friend class ParserPrivate;
    };

}

#endif // COMMAND_H

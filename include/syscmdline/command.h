#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>
#include <functional>

#include <syscmdline/option.h>

namespace SysCmdLine {

    class CommandData;

    class SYSCMDLINE_EXPORT Command : public ArgumentHolder {
    public:
        using Handler = std::function<int(const Parser &)>;

        Command();
        Command(const std::string &name, const std::string &desc,
                const std::vector<Option> &options = {},
                const std::vector<Command> &subCommands = {},
                const std::vector<Argument> &args = {}, const std::string &version = {},
                const std::string &detailedDescription = {}, bool showHelpIfNoArg = false,
                const Handler &handler = {});
        ~Command();

        Command(const Command &other);
        Command(Command &&other) noexcept;
        Command &operator=(const Command &other);
        Command &operator=(Command &&other) noexcept;

    public:
        void addCommand(const Command &command);
        void addOption(const Option &option);

        const std::vector<Command> &commands() const;
        void setCommands(const std::vector<Command> &commands);

        const std::vector<Option> &options() const;
        void setOptions(const std::vector<Option> &options);

        const Command *command(const std::string &name) const;
        const Command *command(int index) const;

        const Option *option(const std::string &name) const;
        const Option *option(int index) const;

        std::string detailedDescription() const;
        void setDetailedDescription(const std::string &detailedDescription);

        void addVersionOption(const std::string &ver, const std::vector<std::string> &tokens = {});
        void addHelpOption(bool showHelpIfNoArg = false, bool global = false,
                           const std::vector<std::string> &tokens = {});

        Handler handler() const;
        void setHandler(const Handler &handler);

        std::string version() const;
        std::string helpText(const std::vector<std::string> &parentCommands = {},
                             const std::vector<const Option *> &globalOptions = {}) const;

    protected:
        CommandData *d_func();
        const CommandData *d_func() const;

        friend class Parser;
        friend class ParserPrivate;
    };


}

#endif // COMMAND_H

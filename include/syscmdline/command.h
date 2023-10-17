#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>
#include <functional>

#include <syscmdline/option.h>

namespace SysCmdLine {

    class SYSCMDLINE_EXPORT Command : public Symbol, public ArgumentHolder {
    public:
        Command();
        Command(const std::string &name, const std::string &desc = {});
        ~Command();

        using Handler = std::function<int(const Parser &)>;

    public:
        void addCommand(const Command &command);
        void addOption(const Option &option);

        inline const Command &command(const std::string &name) const;
        inline const Command &command(int index) const;

        inline const Option &option(const std::string &name) const;
        inline const Option &option(int index) const;

        inline const std::vector<Command> &commands() const;
        void setCommands(const std::vector<Command> &commands);

        inline const std::vector<Option> &options() const;
        void setOptions(const std::vector<Option> &options);

        inline std::string detailedDescription() const;
        inline void setDetailedDescription(const std::string &detailedDescription);

        void addVersionOption(const std::string &ver, const std::vector<std::string> &tokens = {});
        void addHelpOption(bool showHelpIfNoArg = false,
                           const std::vector<std::string> &tokens = {}, bool global = false);

        inline Handler handler() const;
        inline void setHandler(const Handler &handler);

        inline std::string version() const;
        std::string helpText(const std::vector<std::string> &parentCommands = {},
                             const std::vector<const Option *> &globalOptions = {}) const;

    protected:
        std::vector<Option> _options;
        std::unordered_map<std::string, size_t> _optionNameIndexes;
        std::vector<Command> _subCommands;
        std::unordered_map<std::string, size_t> _subCommandNameIndexes;
        std::string _version;
        std::string _detailedDescription;
        bool _showHelpIfNoArg;
        Handler _handler;

        friend class Parser;
        friend class ParserPrivate;
    };

    inline const Command &Command::command(const std::string &name) const {
        return _subCommands.at(_subCommandNameIndexes.find(name)->second);
    }

    inline const Command &Command::command(int index) const {
        return _subCommands.at(index);
    }

    inline const Option &Command::option(const std::string &name) const {
        return _options.at(_optionNameIndexes.find(name)->second);
    }

    inline const Option &Command::option(int index) const {
        return _options.at(index);
    }

    inline const std::vector<Command> &Command::commands() const {
        return _subCommands;
    }

    inline const std::vector<Option> &Command::options() const {
        return _options;
    }

    inline std::string Command::detailedDescription() const {
        return _detailedDescription;
    }

    inline void Command::setDetailedDescription(const std::string &detailedDescription) {
        _detailedDescription = detailedDescription;
    }

    inline Command::Handler Command::handler() const {
        return _handler;
    }

    inline void Command::setHandler(const Handler &handler) {
        _handler = handler;
    }

    inline std::string Command::version() const {
        return _version;
    }

}

#endif // COMMAND_H

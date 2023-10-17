#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>
#include <functional>

#include <syscmdline/option.h>

namespace SysCmdLine {

    class ParseResult;

    class Command : public Symbol, public ArgumentHolder {
    public:
        Command();
        Command(const std::string &name, const std::string &desc = {});
        ~Command();

        using Handler = std::function<int(const ParseResult &, const Command &)>;

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

        inline std::string version() const;
        inline void setVersion(const std::string &version);

        inline std::string detailedDescription() const;
        inline void setDetailedDescription(const std::string &detailedDescription);

        inline bool isVersionVisible() const;
        inline void setVersionVisible(bool on);

        inline bool isHelpVisible() const;
        inline void setHelpVisible(bool on);

        inline Handler handler() const;
        inline void setHandler(const Handler &handler);

        std::string helpText() const;

    protected:
        std::vector<Option> _options;
        std::unordered_map<std::string, size_t> _optionNameIndexes;
        std::vector<Command> _subCommands;
        std::unordered_map<std::string, size_t> _subCommandNameIndexes;
        std::string _version;
        std::string _detailedDescription;
        bool _hasVersion;
        bool _hasHelp;
        Handler _handler;
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

    inline std::string Command::version() const {
        return _version;
    }

    inline void Command::setVersion(const std::string &version) {
        _version = version;
    }

    inline std::string Command::detailedDescription() const {
        return _detailedDescription;
    }

    inline void Command::setDetailedDescription(const std::string &detailedDescription) {
        _detailedDescription = detailedDescription;
    }

    inline bool Command::isVersionVisible() const {
        return _hasVersion;
    }

    inline void Command::setVersionVisible(bool on) {
        _hasVersion = on;
    }

    inline bool Command::isHelpVisible() const {
        return _hasHelp;
    }

    inline void Command::setHelpVisible(bool on) {
        _hasHelp = on;
    }

    inline Command::Handler Command::handler() const {
        return _handler;
    }

    inline void Command::setHandler(const Handler &handler) {
        _handler = handler;
    }

}

#endif // COMMAND_H

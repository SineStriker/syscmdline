#include "command.h"

namespace SysCmdLine {

    Command::Command() : Symbol(ST_Command), _hasVersion(false), _hasHelp(false) {
    }

    Command::Command(const std::string &name, const std::string &desc)
        : Symbol(ST_Command, name, desc), _hasVersion(false), _hasHelp(false) {
    }

    Command::~Command() {
    }

    void Command::addCommand(const Command &command) {
        if (_subCommandNameIndexes.count(command.name())) {
            throw std::runtime_error("command name \"" + command.name() + "\" duplicated");
        }
        _subCommands.push_back(command);
        _argumentNameIndexes.insert(std::make_pair(command.name(), _subCommands.size() - 1));
    }

    void Command::addOption(const Option &option) {
        if (_optionNameIndexes.count(option.name())) {
            throw std::runtime_error("option name \"" + option.name() + "\" duplicated");
        }
        _options.push_back(option);
        _optionNameIndexes.insert(std::make_pair(option.name(), _options.size() - 1));
    }

    void Command::setCommands(const std::vector<Command> &commands) {
        _subCommands.clear();
        _subCommandNameIndexes.clear();
        if (commands.empty())
            return;

        _subCommands.reserve(commands.size());
        _subCommandNameIndexes.reserve(commands.size());
        for (const auto &cmd : commands) {
            addCommand(cmd);
        }
    }

    void Command::setOptions(const std::vector<Option> &options) {
        _options.clear();
        _optionNameIndexes.clear();
        if (options.empty())
            return;

        _options.reserve(options.size());
        _optionNameIndexes.reserve(options.size());
        for (const auto &opt : options) {
            addOption(opt);
        }
    }

    std::string Command::helpText() const {
    }

}
#include "command.h"

#include <stdexcept>
#include <sstream>
#include <iomanip>

#include "strings.h"

namespace SysCmdLine {

    Command::Command() : Symbol(ST_Command), _showHelpIfNoArg(false) {
    }

    Command::Command(const std::string &name, const std::string &desc)
        : Symbol(ST_Command, name, desc), _showHelpIfNoArg(false) {
    }

    Command::~Command() {
    }

    void Command::addCommand(const Command &command) {
        if (_subCommandNameIndexes.count(command.name())) {
            throw std::runtime_error("command name \"" + command.name() + "\" duplicated");
        }
        _subCommands.push_back(command);
        _subCommandNameIndexes.insert(std::make_pair(command.name(), _subCommands.size() - 1));
    }

    void Command::addOption(const Option &option) {
        if (_optionNameIndexes.count(option.name())) {
            throw std::runtime_error("option name \"" + option.name() + "\" duplicated");
        }
        for (const auto &token : option._tokens) {
            if (_optionTokenIndexes.count(token)) {
                throw std::runtime_error("option token \"" + token + "\" duplicated");
            }
        }
        _options.push_back(option);
        _optionNameIndexes.insert(std::make_pair(option.name(), _options.size() - 1));
        for (const auto &token : option._tokens) {
            _optionTokenIndexes.insert(std::make_pair(token, _options.size() - 1));
        }
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
        _optionTokenIndexes.clear();
        if (options.empty())
            return;

        _options.reserve(options.size());
        _optionNameIndexes.reserve(options.size());
        for (const auto &opt : options) {
            addOption(opt);
        }
    }

    void Command::addVersionOption(const std::string &ver, const std::vector<std::string> &tokens) {
        _version = ver;
        addOption(Option("version", Strings::info_strings[Strings::Version],
                         tokens.empty() ? std::vector<std::string>{"-v", "--version"} : tokens,
                         false, true, false));
    }

    void Command::addHelpOption(bool showHelpIfNoArg, const std::vector<std::string> &tokens,
                                bool global) {
        addOption(Option("help", Strings::info_strings[Strings::Help],
                         tokens.empty() ? std::vector<std::string>{"-h", "--help"} : tokens, false,
                         true, global));
        _showHelpIfNoArg = showHelpIfNoArg;
    }

    static const char INDENT[] = "    ";

    std::string Command::helpText(const std::vector<std::string> &parentCommands,
                                  const std::vector<const Option *> &globalOptions) const {
        std::stringstream ss;

        // Description
        const auto &desc = _detailedDescription.empty() ? _desc : _detailedDescription;
        if (!desc.empty()) {
            ss << Strings::common_strings[Strings::Description] << ": " << std::endl;
            ss << INDENT << desc << std::endl;
            ss << std::endl;
        }

        // Usage
        ss << Strings::common_strings[Strings::Usage] << ": " << std::endl;
        {
            ss << INDENT;
            for (const auto &item : parentCommands) {
                ss << item << " ";
            }
            ss << _name;

            if (!_arguments.empty()) {
                ss << " " << Argument::displayArgumentList(_arguments);
            }

            if (!_subCommands.empty()) {
                ss << " [commands]";
            }

            if (!_options.empty()) {
                ss << " [options]";
            }

            ss << std::endl;
        }

        // Arguments
        if (!_arguments.empty()) {
            ss << std::endl;

            size_t widest = 0;
            std::vector<std::pair<std::string, std::string>> texts;
            texts.reserve(_arguments.size());
            for (const auto &item : _arguments) {
                const auto &text = item.name();
                widest = std::max(text.size(), widest);
                texts.emplace_back(text, item.description());
            }

            ss << Strings::common_strings[Strings::Arguments] << ": " << std::endl;
            for (const auto &item : texts) {
                ss << INDENT << std::left << std::setw(widest) << item.first << INDENT
                   << item.second << std::endl;
            }
        }

        // Options
        if (_options.size() + globalOptions.size() > 0) {
            ss << std::endl;

            size_t widest = 0;
            std::vector<std::pair<std::string, std::string>> texts;
            texts.reserve(_options.size() + globalOptions.size());

            for (const auto &p : globalOptions) {
                const auto &item = *p;
                if (_optionNameIndexes.count(item.name()))
                    continue;

                const auto &text = item.displayTokens();
                widest = std::max(text.size(), widest);
                texts.emplace_back(text, item.description());
            }

            for (const auto &item : _options) {
                const auto &text = item.displayTokens();
                widest = std::max(text.size(), widest);
                texts.emplace_back(text, item.description());
            }

            ss << Strings::common_strings[Strings::Options] << ": " << std::endl;
            for (const auto &item : texts) {
                ss << INDENT << std::left << std::setw(widest) << item.first << INDENT
                   << item.second << std::endl;
            }
        }

        // Commands
        if (!_subCommands.empty()) {
            ss << std::endl;

            size_t widest = 0;
            std::vector<std::pair<std::string, std::string>> texts;
            texts.reserve(_subCommands.size());
            for (const auto &item : _subCommands) {
                const auto &text = item.name();
                widest = std::max(text.size(), widest);
                texts.emplace_back(text, item.description());
            }

            ss << Strings::common_strings[Strings::Commands] << ": " << std::endl;
            for (const auto &item : texts) {
                ss << INDENT << std::left << std::setw(widest) << item.first << INDENT
                   << item.second << std::endl;
            }
        }

        return ss.str();
    }

}
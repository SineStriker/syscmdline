#include "command.h"
#include "command_p.h"

#include <stdexcept>
#include <sstream>
#include <iomanip>

#include "option_p.h"
#include "strings.h"

namespace SysCmdLine {

    CommandData::CommandData(const std::string &name, const std::string &desc,
                             const std::vector<Option> &options,
                             const std::vector<Command> &subCommands,
                             const std::vector<Argument> &args, const std::string &version,
                             const std::string &detailedDescription, bool showHelpIfNoArg,
                             const Command::Handler &handler)
        : ArgumentHolderData(Symbol::ST_Command, name, desc, args), version(version),
          detailedDescription(detailedDescription), showHelpIfNoArg(showHelpIfNoArg),
          handler(handler) {
        if (!options.empty())
            setOptions(options);
        if (!subCommands.empty())
            setCommands(subCommands);
    }
    CommandData::~CommandData() {
    }

    SymbolData *CommandData::clone() const {
        return new CommandData(name, desc, options, subCommands, arguments, version,
                               detailedDescription, showHelpIfNoArg, handler);
    }

    void CommandData::setCommands(const std::vector<Command> &commands) {
        subCommands.clear();
        subCommandNameIndexes.clear();
        if (commands.empty())
            return;

        subCommands.reserve(commands.size());
        subCommandNameIndexes.reserve(commands.size());
        for (const auto &cmd : commands) {
            addCommand(cmd);
        }
    }

    void CommandData::setOptions(const std::vector<Option> &opts) {
        options.clear();
        optionNameIndexes.clear();
        optionTokenIndexes.clear();
        if (opts.empty())
            return;

        options.reserve(opts.size());
        optionNameIndexes.reserve(opts.size());
        for (const auto &opt : opts) {
            addOption(opt);
        }
    }

    void CommandData::addCommand(const Command &command) {
        if (subCommandNameIndexes.count(command.name())) {
            throw std::runtime_error("command name \"" + command.name() + "\" duplicated");
        }
        subCommands.push_back(command);
        subCommandNameIndexes.insert(std::make_pair(command.name(), subCommands.size() - 1));
    }

    void CommandData::addOption(const Option &option) {
        if (optionNameIndexes.count(option.name())) {
            throw std::runtime_error("option name \"" + option.name() + "\" duplicated");
        }
        for (const auto &token : option.d_func()->tokens) {
            if (optionTokenIndexes.count(token)) {
                throw std::runtime_error("option token \"" + token + "\" duplicated");
            }
        }

        options.push_back(option);
        optionNameIndexes.insert(std::make_pair(option.name(), options.size() - 1));
        for (const auto &token : option.d_func()->tokens) {
            optionTokenIndexes.insert(std::make_pair(token, options.size() - 1));
        }
    }

    Command::Command() : Command({}, {}) {
    }

    Command::Command(const std::string &name, const std::string &desc,
                     const std::vector<Option> &options, const std::vector<Command> &subCommands,
                     const std::vector<Argument> &args, const std::string &version,
                     const std::string &detailedDescription, bool showHelpIfNoArg,
                     const Command::Handler &handler)
        : ArgumentHolder(new CommandData(name, desc, options, subCommands, args, version,
                                         detailedDescription, showHelpIfNoArg, handler)) {
    }

    Command::~Command() {
    }

    Command::Command(const Command &other) : ArgumentHolder(nullptr) {
        d_ptr = other.d_ptr;
    }

    Command::Command(Command &&other) noexcept : ArgumentHolder(nullptr) {
        d_ptr.swap(other.d_ptr);
    }

    Command &Command::operator=(const Command &other) {
        if (this == &other) {
            return *this;
        }
        d_ptr = other.d_ptr;
        return *this;
    }

    Command &Command::operator=(Command &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        d_ptr.swap(other.d_ptr);
        return *this;
    }

    void Command::addCommand(const Command &command) {
        SYSCMDLINE_GET_DATA(Command);
        d->addCommand(command);
    }

    void Command::addOption(const Option &option) {
        SYSCMDLINE_GET_DATA(Command);
        d->addOption(option);
    }

    const std::vector<Command> &Command::commands() const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        return d->subCommands;
    }

    void Command::setCommands(const std::vector<Command> &commands) {
        SYSCMDLINE_GET_DATA(Command);
        d->setCommands(commands);
    }

    const std::vector<Option> &Command::options() const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        return d->options;
    }

    void Command::setOptions(const std::vector<Option> &options) {
        SYSCMDLINE_GET_DATA(Command);
        d->setOptions(options);
    }

    const Command *Command::command(const std::string &name) const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        auto it = d->subCommandNameIndexes.find(name);
        if (it == d->subCommandNameIndexes.end())
            return nullptr;
        return &d->subCommands[it->second];
    }

    const Command *Command::command(int index) const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        if (index < 0 || index >= d->subCommands.size())
            return nullptr;
        return &d->subCommands[index];
    }

    const Option *Command::option(const std::string &name) const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        auto it = d->optionNameIndexes.find(name);
        if (it == d->optionNameIndexes.end())
            return nullptr;
        return &d->options[it->second];
    }

    const Option *Command::option(int index) const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        if (index < 0 || index >= d->options.size())
            return nullptr;
        return &d->options[index];
    }

    std::string Command::detailedDescription() const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        return d->detailedDescription;
    }

    void Command::setDetailedDescription(const std::string &detailedDescription) {
        SYSCMDLINE_GET_DATA(Command);
        d->detailedDescription = detailedDescription;
    }

    Command::Handler Command::handler() const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        return d->handler;
    }

    void Command::setHandler(const Command::Handler &handler) {
        SYSCMDLINE_GET_DATA(Command);
        d->handler = handler;
    }

    std::string Command::version() const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        return d->version;
    }

    void Command::addVersionOption(const std::string &ver, const std::vector<std::string> &tokens) {
        SYSCMDLINE_GET_DATA(Command);
        d->version = ver;
        addOption(Option("version", Strings::info_strings[Strings::Version],
                         tokens.empty() ? std::vector<std::string>{"-v", "--version"} : tokens,
                         false, Option::IgnoreMissingArgument, false));
    }

    void Command::addHelpOption(bool showHelpIfNoArg, bool global,
                                const std::vector<std::string> &tokens) {
        SYSCMDLINE_GET_DATA(Command);
        addOption(Option("help", Strings::info_strings[Strings::Help],
                         tokens.empty() ? std::vector<std::string>{"-h", "--help"} : tokens, false,
                         Option::IgnoreMissingArgument, global));
        d->showHelpIfNoArg = showHelpIfNoArg;
    }

    static const char INDENT[] = "    ";

    std::string Command::helpText(const std::vector<std::string> &parentCommands,
                                  const std::vector<const Option *> &globalOptions) const {
        SYSCMDLINE_GET_CONST_DATA(Command);

        std::stringstream ss;

        // Description
        const auto &desc = d->detailedDescription.empty() ? d->desc : d->detailedDescription;
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
            ss << d->name;

            if (!d->arguments.empty()) {
                ss << " " << displayedArguments();
            }

            if (!d->subCommands.empty()) {
                ss << " [commands]";
            }

            if (!d->options.empty()) {
                ss << " [options]";
            }

            ss << std::endl;
        }

        // Arguments
        if (!d->arguments.empty()) {
            ss << std::endl;

            size_t widest = 0;
            std::vector<std::pair<std::string, std::string>> texts;
            texts.reserve(d->arguments.size());
            for (const auto &item : d->arguments) {
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
        if (d->options.size() + globalOptions.size() > 0) {
            ss << std::endl;

            size_t widest = 0;
            std::vector<std::pair<std::string, std::string>> texts;
            texts.reserve(d->options.size() + globalOptions.size());

            for (const auto &p : globalOptions) {
                const auto &item = *p;
                if (d->optionNameIndexes.count(item.name()))
                    continue;

                {
                    bool skip = false;
                    for (const auto &token : item.d_func()->tokens) {
                        if (d->optionTokenIndexes.count(token)) {
                            skip = true;
                            break;
                        }
                    }

                    if (skip) {
                        continue;
                    }
                }

                const auto &text = item.displayedTokens();
                widest = std::max(text.size(), widest);
                texts.emplace_back(text, item.description());
            }

            for (const auto &item : d->options) {
                const auto &text = item.displayedTokens();
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
        if (!d->subCommands.empty()) {
            ss << std::endl;

            size_t widest = 0;
            std::vector<std::pair<std::string, std::string>> texts;
            texts.reserve(d->subCommands.size());
            for (const auto &item : d->subCommands) {
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

    CommandData *Command::d_func() {
        SYSCMDLINE_GET_DATA(Command);
        return d;
    }

    const CommandData *Command::d_func() const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        return d;
    }

}
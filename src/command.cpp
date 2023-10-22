#include "command.h"
#include "command_p.h"

#include <set>
#include <stdexcept>
#include <algorithm>
#include <iostream>

#include "option_p.h"
#include "strings.h"
#include "parser.h"
#include "utils.h"

namespace SysCmdLine {

    CommandCatalogueData *CommandCatalogueData::clone() const {
        return new CommandCatalogueData(*this);
    }

    CommandCatalogue::CommandCatalogue() : d_ptr(new CommandCatalogueData()) {
    }

    CommandCatalogue::~CommandCatalogue() {
    }

    CommandCatalogue::CommandCatalogue(const CommandCatalogue &other) {
        d_ptr = other.d_ptr;
    }

    CommandCatalogue::CommandCatalogue(CommandCatalogue &&other) noexcept {
        d_ptr.swap(other.d_ptr);
    }

    CommandCatalogue &CommandCatalogue::operator=(const CommandCatalogue &other) {
        if (this == &other) {
            return *this;
        }
        d_ptr = other.d_ptr;
        return *this;
    }

    CommandCatalogue &CommandCatalogue::operator=(CommandCatalogue &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        d_ptr.swap(other.d_ptr);
        return *this;
    }

    void CommandCatalogue::addArgumentCatalogue(const std::string &name,
                                                const std::vector<std::string> &args) {
        size_t index;
        auto it = d_ptr->_argIndexes.find(name);
        if (it == d_ptr->_argIndexes.end()) {
            index = d_ptr->_arg.size();
            d_ptr->_arg.emplace_back(args.begin(), args.end());
            d_ptr->_argIndexes.insert(std::make_pair(name, index));
        } else {
            index = it->second;
            d_ptr->_arg[index].insert(args.begin(), args.end());
        }
    }

    void CommandCatalogue::addOptionCatalogue(const std::string &name,
                                              const std::vector<std::string> &options) {
        size_t index;
        auto it = d_ptr->_optIndexes.find(name);
        if (it == d_ptr->_optIndexes.end()) {
            index = d_ptr->_opt.size();
            d_ptr->_opt.emplace_back(options.begin(), options.end());
            d_ptr->_optIndexes.insert(std::make_pair(name, index));
        } else {
            index = it->second;
            d_ptr->_opt[index].insert(options.begin(), options.end());
        }
    }

    void CommandCatalogue::addCommandCatalogue(const std::string &name,
                                               const std::vector<std::string> &commands) {
        size_t index;
        auto it = d_ptr->_cmdIndexes.find(name);
        if (it == d_ptr->_cmdIndexes.end()) {
            index = d_ptr->_cmd.size();
            d_ptr->_cmd.emplace_back(commands.begin(), commands.end());
            d_ptr->_cmdIndexes.insert(std::make_pair(name, index));
        } else {
            index = it->second;
            d_ptr->_cmd[index].insert(commands.begin(), commands.end());
        }
    }

    CommandData::CommandData(const std::string &name, const std::string &desc,
                             const std::vector<std::pair<Option, int>> &options,
                             const std::vector<Command> &subCommands,
                             const std::vector<Argument> &args, const std::string &version,
                             const std::string &detailedDescription,
                             const Command::Handler &handler, const CommandCatalogue &catalogue)
        : ArgumentHolderData(Symbol::ST_Command, name, desc, args), superPriorOptionIndex(-1),
          version(version), detailedDescription(detailedDescription), handler(handler),
          catalogue(catalogue) {
        if (!options.empty())
            setOptions(options);
        if (!subCommands.empty())
            setCommands(subCommands);
    }
    CommandData::~CommandData() {
    }

    SymbolData *CommandData::clone() const {
        return new CommandData(*this);
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
        exclusiveGroups.clear();
        exclusiveGroupIndexes.clear();
        if (opts.empty())
            return;

        options.reserve(opts.size());
        optionNameIndexes.reserve(opts.size());
        for (const auto &opt : opts) {
            addOption(opt);
        }
    }

    void CommandData::setOptions(const std::vector<std::pair<Option, int>> &opts) {
        options.clear();
        optionNameIndexes.clear();
        optionTokenIndexes.clear();
        exclusiveGroups.clear();
        exclusiveGroupIndexes.clear();
        superPriorOptionIndex = -1;
        if (opts.empty())
            return;

        options.reserve(opts.size());
        optionNameIndexes.reserve(opts.size());
        for (const auto &pair : opts) {
            addOption(pair.first, pair.second);
        }
    }

    void CommandData::addCommand(const Command &command) {
        const auto &name = command.name();
        if (name.empty()) {
            throw std::runtime_error("empty command name");
        }
        if (subCommandNameIndexes.count(name)) {
            throw std::runtime_error("command name \"" + name + "\" duplicated");
        }
        subCommandNameIndexes.insert(std::make_pair(name, subCommands.size()));
        subCommands.push_back(command);
    }

    void CommandData::addOption(const Option &option, int exclusiveGroup) {
        const auto &name = option.name();
        if (name.empty()) {
            throw std::runtime_error("null option name");
        }
        if (name == "-" || name == "/") {
            throw std::runtime_error("invalid option name \"" + name + "\"");
        }
        if (optionNameIndexes.count(name)) {
            throw std::runtime_error("option name \"" + name + "\" duplicated");
        }

        Option newOption = option;
        if (newOption.tokens().empty()) {
            if (name.front() == '-' || name.front() == '/') {
                newOption.setToken(name);
            } else {
                if (name.size() == 1) {
                    newOption.setToken("-" + name);
                } else {
                    newOption.setToken("--" + name);
                }
            }
        }

        const auto &d = newOption.d_func();
        for (const auto &token : d->tokens) {
            if (optionTokenIndexes.count(token)) {
                throw std::runtime_error("option token \"" + token + "\" duplicated");
            }
        }

        if (exclusiveGroup >= 0 && newOption.isGlobal()) {
            throw std::runtime_error("global option \"" + name +
                                     "\" cannot be in any exclusive group");
        }

        switch (newOption.priorLevel()) {
            case Option::AutoSetWhenNoSymbols: {
                for (const auto &arg : d->arguments) {
                    if (arg.isRequired()) {
                        throw std::runtime_error("auto-option cannot have required arguments");
                    }
                }
                break;
            }
            case Option::ExclusiveToOptions:
            case Option::ExclusiveToAll: {
                if (superPriorOptionIndex >= 0) {
                    throw std::runtime_error("there can be at most one exclusively prior option.");
                }
                superPriorOptionIndex = int(options.size());
                break;
            }
            default:
                break;
        }

        auto last = options.size();
        optionNameIndexes.insert(std::make_pair(name, last));
        options.push_back(newOption);
        for (const auto &token : d->tokens) {
            optionTokenIndexes.insert(std::make_pair(token, last));
        }

        // Add exclusive group
        if (exclusiveGroup >= 0) {
            auto it = exclusiveGroups.find(exclusiveGroup);
            if (it == exclusiveGroups.end()) {
                exclusiveGroups.insert(std::make_pair(exclusiveGroup, std::vector<size_t>{last}));
            } else if (options[it->second.front()].isRequired() != newOption.isRequired()) {
                throw std::runtime_error("option \"" + name + "\" is " +
                                         (newOption.isRequired() ? "required" : "optional") +
                                         ", but exclusive group " + std::to_string(exclusiveGroup) +
                                         " isn't");
            } else {
                it->second.push_back(last);
            }
            exclusiveGroupIndexes.insert(std::make_pair(name, exclusiveGroup));
        }
    }

    Command::Command() : Command({}, {}) {
    }

    Command::Command(const std::string &name, const std::string &desc,
                     const std::vector<std::pair<Option, int>> &options,
                     const std::vector<Command> &subCommands, const std::vector<Argument> &args,
                     const std::string &detailedDescription, const Command::Handler &handler)
        : ArgumentHolder(new CommandData(name, desc, options, subCommands, args, {},
                                         detailedDescription, handler, {})) {
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

    std::string Command::helpText(Symbol::HelpPosition pos, int displayOptions, void *extra) const {
        SYSCMDLINE_GET_DATA(const Command);
        if (d->helpProvider)
            return d->helpProvider(this, pos, displayOptions, extra);

        switch (pos) {
            case HP_Usage: {
                auto &options = *reinterpret_cast<const decltype(d->options) *>(extra);

                std::string ss;
                ss += d->name;

                auto displayArgumentsHelp = [&](bool front) {
                    if (bool(displayOptions & Parser::ShowOptionsBehindArguments) != front &&
                        !d->arguments.empty()) {
                        ss += " " + displayedArguments(displayOptions);
                    }
                };

                // arguments
                displayArgumentsHelp(true);

                std::unordered_set<std::string> printedOptions;
                auto printExclusiveOptions = [&](const Option &opt, bool needParen) {
                    auto it = d->exclusiveGroupIndexes.find(opt.name());
                    if (it == d->exclusiveGroupIndexes.end()) {
                        ss += opt.helpText(Symbol::HP_Usage, displayOptions);
                        printedOptions.insert(opt.name());
                        return;
                    }

                    const auto &arr = d->exclusiveGroups.find(it->second)->second;
                    if (arr.size() <= 1) {
                        ss += opt.helpText(Symbol::HP_Usage, displayOptions);
                        printedOptions.insert(opt.name());
                        return;
                    }

                    if (needParen)
                        ss += "(";
                    std::vector<std::string> exclusiveOptions;
                    for (const auto &item : arr) {
                        const auto &curOpt = d->options[item];
                        exclusiveOptions.push_back(
                            curOpt.helpText(Symbol::HP_Usage, displayOptions));
                        printedOptions.insert(curOpt.name());
                    }

                    ss += Utils::join<char>(exclusiveOptions, " | ");
                    if (needParen)
                        ss += ")";
                };

                // required options
                if (!(displayOptions & Parser::DontShowRequiredOptionsOnUsage)) {
                    for (const auto &opt : options) {
                        if (!opt.isRequired()) {
                            continue;
                        }

                        if (printedOptions.count(opt.name()))
                            continue;

                        // check exclusive
                        ss += " ";
                        printExclusiveOptions(opt, true);
                    }
                }

                // optional options
                if ((displayOptions & Parser::ShowOptionalOptionsOnUsage) &&
                    printedOptions.size() < options.size()) {
                    for (const auto &opt : options) {
                        if (opt.isRequired()) {
                            continue;
                        }

                        if (printedOptions.count(opt.name()))
                            continue;

                        // check exclusive
                        ss += " [";
                        printExclusiveOptions(opt, false);
                        ss += "]";
                    }
                }

                // arguments
                displayArgumentsHelp(false);

                // command
                if (!d->subCommands.empty()) {
                    ss += " [commands]";
                }

                // options
                if (printedOptions.size() < options.size() || !d->subCommands.empty()) {
                    ss += " [options]";
                }
                return ss;
            }
            case HP_ErrorText:
            case HP_FirstColumn: {
                return d->name;
            }
            case HP_SecondColumn: {
                return d->desc;
            }
        }
        return {};
    }

    Command Command::command(const std::string &name) const {
        SYSCMDLINE_GET_DATA(const Command);
        auto it = d->subCommandNameIndexes.find(name);
        if (it == d->subCommandNameIndexes.end())
            return {};
        return d->subCommands[it->second];
    }

    Command Command::command(int index) const {
        SYSCMDLINE_GET_DATA(const Command);
        if (index < 0 || index >= d->subCommands.size())
            return {};
        return d->subCommands[index];
    }

    const std::vector<Command> &Command::commands() const {
        SYSCMDLINE_GET_DATA(const Command);
        return d->subCommands;
    }

    int Command::indexOfCommand(const std::string &name) const {
        SYSCMDLINE_GET_DATA(const Command);
        auto it = d->subCommandNameIndexes.find(name);
        if (it == d->subCommandNameIndexes.end())
            return -1;
        return int(it->second);
    }

    bool Command::hasCommand(const std::string &name) const {
        SYSCMDLINE_GET_DATA(const Command);
        return d->subCommandNameIndexes.count(name);
    }

    void Command::addCommand(const Command &command) {
        SYSCMDLINE_GET_DATA(Command);
        d->addCommand(command);
    }

    void Command::setCommands(const std::vector<Command> &commands) {
        SYSCMDLINE_GET_DATA(Command);
        d->setCommands(commands);
    }

    Option Command::option(const std::string &name) const {
        SYSCMDLINE_GET_DATA(const Command);
        auto it = d->optionNameIndexes.find(name);
        if (it == d->optionNameIndexes.end())
            return {};
        return d->options[it->second];
    }

    Option Command::option(int index) const {
        SYSCMDLINE_GET_DATA(const Command);
        if (index < 0 || index >= d->options.size())
            return {};
        return d->options[index];
    }

    Option Command::optionFromToken(const std::string &token) const {
        SYSCMDLINE_GET_DATA(const Command);
        auto it = d->optionTokenIndexes.find(token);
        if (it == d->optionTokenIndexes.end())
            return {};
        return d->options[it->second];
    }

    const std::vector<Option> &Command::options() const {
        SYSCMDLINE_GET_DATA(const Command);
        return d->options;
    }

    int Command::indexOfOption(const std::string &name) const {
        SYSCMDLINE_GET_DATA(const Command);
        auto it = d->optionNameIndexes.find(name);
        if (it == d->optionNameIndexes.end())
            return -1;
        return int(it->second);
    }

    bool Command::hasOption(const std::string &name) const {
        SYSCMDLINE_GET_DATA(const Command);
        return d->optionTokenIndexes.count(name);
    }

    bool Command::hasOptionToken(const std::string &token) const {
        SYSCMDLINE_GET_DATA(const Command);
        return d->optionTokenIndexes.count(token);
    }

    void Command::addOption(const Option &option, int exclusiveGroup) {
        SYSCMDLINE_GET_DATA(Command);
        d->addOption(option, exclusiveGroup);
    }

    void Command::setOptions(const std::vector<Option> &options) {
        SYSCMDLINE_GET_DATA(Command);
        d->setOptions(options);
    }

    void Command::setOptions(const std::vector<std::pair<Option, int>> &options) {
        SYSCMDLINE_GET_DATA(Command);
        d->setOptions(options);
    }

    std::vector<int> Command::exclusiveGroups() const {
        SYSCMDLINE_GET_DATA(const Command);
        std::set<int> groups;
        for (const auto &item : d->exclusiveGroups) {
            groups.insert(item.first);
        }
        return {groups.begin(), groups.end()};
    }

    std::vector<Option> Command::exclusiveGroupOptions(int group) const {
        SYSCMDLINE_GET_DATA(const Command);
        auto it = d->exclusiveGroups.find(group);
        if (it == d->exclusiveGroups.end())
            return {};

        std::vector<Option> res;
        res.reserve(it->second.size());
        for (const auto &item : it->second) {
            res.push_back(d->options.at(item));
        }
        return res;
    }

    std::string Command::detailedDescription() const {
        SYSCMDLINE_GET_DATA(const Command);
        return d->detailedDescription;
    }

    void Command::setDetailedDescription(const std::string &detailedDescription) {
        if (detailedDescription == this->detailedDescription())
            return;

        SYSCMDLINE_GET_DATA(Command);
        d->detailedDescription = detailedDescription;
    }

    Command::Handler Command::handler() const {
        SYSCMDLINE_GET_DATA(const Command);
        return d->handler;
    }

    void Command::setHandler(const Handler &handler) {
        SYSCMDLINE_GET_DATA(Command);
        d->handler = handler;
    }

    CommandCatalogue Command::catalogue() const {
        SYSCMDLINE_GET_DATA(const Command);
        return d->catalogue;
    }

    void Command::setCatalogue(const CommandCatalogue &catalogue) {
        SYSCMDLINE_GET_DATA(Command);
        d->catalogue = catalogue;
    }

    std::string Command::version() const {
        SYSCMDLINE_GET_DATA(const Command);
        return d->version;
    }

    void Command::addVersionOption(const std::string &ver, const std::vector<std::string> &tokens) {
        SYSCMDLINE_GET_DATA(Command);
        d->version = ver;
        addOption(Option("version", Strings::text(Strings::DefaultCommand, Strings::Version),
                         tokens.empty() ? std::vector<std::string>{"-v", "--version"} : tokens,
                         false, Option::NoShortMatch, Option::IgnoreMissingSymbols, false));
    }

    void Command::addHelpOption(bool showHelpIfNoArg, bool global,
                                const std::vector<std::string> &tokens) {
        SYSCMDLINE_GET_DATA(Command);
        addOption(Option(
            "help", Strings::text(Strings::DefaultCommand, Strings::Help),
            tokens.empty() ? std::vector<std::string>{"-h", "--help"} : tokens, false,
            Option::NoShortMatch,
            showHelpIfNoArg ? Option::AutoSetWhenNoSymbols : Option::IgnoreMissingSymbols, global));
    }

}
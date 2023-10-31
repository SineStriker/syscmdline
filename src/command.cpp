#include "command.h"
#include "command_p.h"

#include <algorithm>
#include <utility>

#include "parser.h"
#include "utils_p.h"
#include "option_p.h"
#include "system.h"

namespace SysCmdLine {

    SharedBasePrivate *CommandCataloguePrivate::clone() const {
        return new CommandCataloguePrivate(*this);
    }

    static void addIndexes(GenericMap &indexes, StringList &keys, const std::string &key,
                           const StringList &val) {
        auto it = indexes.find(key);
        if (it == indexes.end()) {
            indexes.insert(std::make_pair(key, Ele{.sl = new StringList(val)}));
            keys.push_back(key);
            return;
        }
        auto list = *it->second.sl;
        list.insert(list.end(), val.begin(), val.end());

#ifdef SYSCMDLINE_ENABLE_VALIDITY_CHECK
        for (size_t i = 0; i < list.size(); ++i) {
            for (size_t j = 0; j < i; ++j) {
                if (list[i] == list[j]) {
                    throw std::runtime_error(Utils::formatText(
                        "duplicated items \"%1\" and \"%2\" in catalogue", {
                                                                               list[i],
                                                                               list[j],
                                                                           }));
                }
            }
        }
#endif
    }

    CommandCatalogue::CommandCatalogue() : SharedBase(new CommandCataloguePrivate()) {
    }

    void CommandCatalogue::addArguments(const std::string &name, const StringList &args) {
        Q_D(CommandCatalogue);
        addIndexes(d->arg.data, d->arguments, name, args);
    }

    void CommandCatalogue::addOptions(const std::string &name, const StringList &options) {
        Q_D(CommandCatalogue);
        addIndexes(d->opt.data, d->options, name, options);
    }

    void CommandCatalogue::addCommands(const std::string &name, const StringList &commands) {
        Q_D(CommandCatalogue);
        addIndexes(d->cmd.data, d->commands, name, commands);
    }

    CommandPrivate::CommandPrivate(std::string name, const std::string &desc)
        : ArgumentHolderPrivate(Symbol::ST_Command, desc), name(std::move(name)) {
    }

    SharedBasePrivate *CommandPrivate::clone() const {
        return new CommandPrivate(*this);
    }

#ifdef SYSCMDLINE_ENABLE_VALIDITY_CHECK
    void CommandPrivate::checkAddedCommand(const Command &cmd) const {
        const auto &name = cmd.name();

        // Empty command name?
        if (name.empty()) {
            throw std::runtime_error("command doesn't have a name");
        }

        // Duplicated command name?
        if (std::any_of(commands.begin(), commands.end(), [&name](const Command &command) {
                return command.name() == name; //
            })) {
            throw std::runtime_error(Utils::formatText("command name \"%1\" duplicated", {name}));
        }
    }

    void CommandPrivate::checkAddedOption(const Option &opt,
                                          const std::string &exclusiveGroup) const {
        const auto &d = opt.d_func();

        // Empty token?
        if (d->tokens.empty()) {
            throw std::runtime_error("option doesn't have any token");
        }

        for (const auto &token : d->tokens) {
            // empty token?
            if (token.empty()) {
                throw std::runtime_error("empty option token");
            }

            // Invalid token?
            if (!(token.front() == '-' || token.front() == '/')) {
                throw std::runtime_error(
                    Utils::formatText("option token \"%1\" is invalid", {token}));
            }

            // Duplicated token?
            if (std::any_of(options.begin(), options.end(), [&token](const Option &opt) {
                    for (const auto &item : opt.d_func()->tokens)
                        if (item == token)
                            return true;
                    return false;
                })) {
                throw std::runtime_error(
                    Utils::formatText("option token \"%1\" duplicated", {token}));
            }
        }

        // Global and exclusive option?
        if (!exclusiveGroup.empty() && opt.isGlobal()) {
            throw std::runtime_error(Utils::formatText(
                "global option \"%s\" cannot be in any exclusive group", {opt.token()}));
        }

        switch (opt.priorLevel()) {
            case Option::AutoSetWhenNoSymbols: {
                // Auto-option but required?
                if (d->required) {
                    throw std::runtime_error(
                        Utils::formatText("auto-option \"%s\" cannot be required", {opt.token()}));
                }

                // Auto-option with argument?
                if (!d->arguments.empty()) {
                    throw std::runtime_error(Utils::formatText(
                        "auto-option \"%s\" cannot have any argument", {opt.token()}));
                }
                break;
            }
            case Option::ExclusiveToOptions:
            case Option::ExclusiveToAll: {
                // Multiple exclusively prior option?
                if (opt.isRequired() &&
                    std::any_of(options.begin(), options.end(), [](const Option &opt) {
                        return opt.priorLevel() >= Option::ExclusiveToOptions && opt.isRequired();
                    })) {
                    throw std::runtime_error("at most one exclusively prior and required option");
                }
                break;
            }
            default:
                break;
        }
        // Inconsistent exclusive group?
        if (!exclusiveGroup.empty()) {
            for (size_t i = 0; i < options.size(); ++i) {
                if (optionGroupNames[i] == exclusiveGroup &&
                    options[i].isRequired() != opt.isRequired()) {
                    throw std::runtime_error(
                        Utils::formatText("option \"%1\" is %2, but exclusive group \"3\" isn't",
                                          {
                                              opt.token(),
                                              opt.isRequired() ? "required" : "optional",
                                              exclusiveGroup,
                                          }));
                }
            }
        }
    }
#endif

    Command::Command() : Command({}, {}) {
    }

    Command::Command(const std::string &name, const std::string &desc)
        : ArgumentHolder(new CommandPrivate(name, desc)) {
    }

    std::string Command::helpText(Symbol::HelpPosition pos, int displayOptions, void *extra) const {
        Q_D2(Command);
        if (auto ss = ArgumentHolder::helpText(pos, displayOptions, extra); !ss.empty()) {
            return ss;
        }

        switch (pos) {
            case HP_Usage: {
                // `extra` should be null or a 3 pointer array
                auto a = reinterpret_cast<void **>(extra);

                // all options
                const auto *globalOptions =
                    a ? reinterpret_cast<std::vector<Option> *>(a[0]) : nullptr;
                auto options = d->options;
                if (globalOptions) {
                    options.insert(options.begin(), globalOptions->begin(), globalOptions->end());
                }
                StringList groupNames(globalOptions ? globalOptions->size() : 0);
                groupNames.insert(groupNames.end(), d->optionGroupNames.begin(),
                                  d->optionGroupNames.end());

                std::string ss;

                // write command name
                ss += d->name;

                // Build exclusive option group indexes
                // group name -> option subscripts (vector<int> *)
                GenericMap exclusiveGroupIndexes = [](const CommandPrivate *d) {
                    GenericMap res;
                    for (int i = 0; i < d->optionGroupNames.size(); ++i) {
                        const auto &group = d->optionGroupNames[i];
                        if (group.empty())
                            continue;

                        auto it = res.find(group);
                        if (it != res.end()) {
                            it->second.il->push_back(i);
                            continue;
                        }
                        res[group] = Ele{.il = new IntList({i})};
                    }
                    return res;
                }(d);

                auto addArgumentsHelp = [&](bool front) {
                    if (bool((displayOptions & Parser::ShowOptionsBehindArguments)) != front &&
                        !d->arguments.empty()) {
                        ss += " " + displayedArguments(displayOptions);
                    }
                };

                std::vector<int> visitedOptions(options.size()); // pointer -> none
                int visitedCount = 0;
                auto addVisited = [&visitedOptions, &visitedCount](int idx) {
                    if (visitedOptions[idx])
                        return;
                    visitedOptions[idx] = 1;
                    visitedCount++;
                };

                auto addExclusiveOptions = [&](int optIdx, bool required) {
                    const auto &opt = options[optIdx];
                    const auto &groupName = groupNames[optIdx];
                    const IntList *optionIndexes;

                    // Search group
                    if (groupName.empty() ||
                        (optionIndexes = exclusiveGroupIndexes.find(groupName)->second.il)
                                ->size() <= 1) {
                        ss += opt.helpText(Symbol::HP_Usage, displayOptions);
                        addVisited(optIdx);
                        return;
                    }

                    if (required)
                        ss += "(";

                    assert((optionIndexes = exclusiveGroupIndexes.find(groupName)->second.il));

                    // Add exclusive
                    StringList exclusiveOptions;
                    exclusiveOptions.reserve(optionIndexes->size());
                    for (const auto &idx : *optionIndexes) {
                        const auto &curOpt = options[idx];
                        exclusiveOptions.push_back(
                            curOpt.helpText(Symbol::HP_Usage, displayOptions));
                        addVisited(idx);
                    }

                    ss += Utils::join(exclusiveOptions, " | ");
                    if (required)
                        ss += ")";
                };

                auto addOptionsHelp = [&](bool required) {
                    for (int i = 0; i < options.size(); ++i) {
                        const auto &opt = options[i];
                        if (opt.isRequired() != required || visitedOptions[i]) {
                            continue;
                        }

                        ss += " ";
                        if (!required)
                            ss += "[";
                        addExclusiveOptions(i, required);
                        if (!required)
                            ss += "]";
                    }
                };

                // write forward arguments
                addArgumentsHelp(true);

                // write required options
                if (!(displayOptions & Parser::DontShowRequiredOptionsOnUsage))
                    addOptionsHelp(true);

                // write optional options
                if ((displayOptions & Parser::ShowOptionalOptionsOnUsage) &&
                    visitedCount < options.size())
                    addOptionsHelp(false);

                // write backward arguments
                addArgumentsHelp(false);

                // tell the caller if commands and options should be written
                if (a) {
                    // command
                    *(bool *) a[1] = !d->commands.empty();

                    // options
                    *(bool *) a[2] = visitedCount < options.size() || !d->commands.empty();
                }

                // release indexes
                for (const auto &pair : std::as_const(exclusiveGroupIndexes)) {
                    delete pair.second.il;
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

    std::string Command::name() const {
        Q_D2(Command);
        return d->name;
    }

    void Command::setName(const std::string &name) {
        Q_D(Command);
        d->name = name;
    }

    int Command::commandCount() const {
        Q_D2(Command);
        return int(d->commands.size());
    }

    Command Command::command(int index) const {
        Q_D2(Command);
        return d->commands[index];
    }

    void Command::addCommands(const std::vector<Command> &commands) {
        Q_D(Command);
#ifdef SYSCMDLINE_ENABLE_VALIDITY_CHECK
        for (const auto &cmd : commands) {
            d->checkAddedCommand(cmd);
            d->commands.push_back(cmd);
        }
#else
        d->commands.insert(d->commands.end(), commands.begin(), commands.end());
#endif
    }

    int Command::optionCount() const {
        Q_D2(Command);
        return int(d->options.size());
    }

    Option Command::option(int index) const {
        Q_D2(Command);
        return d->options[index];
    }

    void Command::addOptions(const std::vector<Option> &options, const std::string &group) {
        Q_D(Command);
#ifdef SYSCMDLINE_ENABLE_VALIDITY_CHECK
        for (const auto &opt : options) {
            d->checkAddedOption(opt, group);
            d->options.push_back(opt);
            d->optionGroupNames.push_back(group);
        }
#else
        d->options.insert(d->options.end(), options.begin(), options.end());
        d->optionGroupNames.insert(d->optionGroupNames.end(), options.size(), group);
#endif
    }

    std::string Command::detailedDescription() const {
        Q_D2(Command);
        return d->detailedDescription;
    }

    void Command::setDetailedDescription(const std::string &detailedDescription) {
        Q_D(Command);
        d->detailedDescription = detailedDescription;
    }

    Command::Handler Command::handler() const {
        Q_D2(Command);
        return d->handler;
    }

    void Command::setHandler(const Handler &handler) {
        Q_D(Command);
        d->handler = handler;
    }

    CommandCatalogue Command::catalogue() const {
        Q_D2(Command);
        return d->catalogue;
    }

    void Command::setCatalogue(const CommandCatalogue &catalogue) {
        Q_D(Command);
        d->catalogue = catalogue;
    }

    std::string Command::version() const {
        Q_D2(Command);
        return d->version;
    }

    void Command::addVersionOption(const std::string &ver, const StringList &tokens) {
        Q_D(Command);
        d->version = ver;

        Option versionOption(Option::Version);
        versionOption.setTokens(tokens.empty() ? StringList{"-v", "--version"} : tokens);
        versionOption.setPriorLevel(Option::IgnoreMissingSymbols);
        addOption(versionOption);
    }

    void Command::addHelpOption(bool showHelpIfNoArg, bool global, const StringList &tokens) {
        Q_D(Command);
        Option helpOption(Option::Help);
        helpOption.setTokens(tokens.empty() ? StringList{"-h", "--help"} : tokens);
        helpOption.setPriorLevel(showHelpIfNoArg ? Option::AutoSetWhenNoSymbols
                                                 : Option::IgnoreMissingSymbols);
        helpOption.setGlobal(global);
        addOption(helpOption);
    }

}
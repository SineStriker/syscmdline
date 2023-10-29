#include "command.h"
#include "command_p.h"

#include <algorithm>
#include <utility>

#include "parser.h"
#include "utils_p.h"
#include "option_p.h"
#include "system.h"

namespace SysCmdLine {

    StringListMapWrapper::StringListMapWrapper() = default;

    StringListMapWrapper::StringListMapWrapper(const StringListMapWrapper &other) {
        data = map_copy<StringList>(other.data);
    }

    StringListMapWrapper::~StringListMapWrapper() {
        map_deleteAll<StringList>(data);
    }

    SharedBasePrivate *CommandCataloguePrivate::clone() const {
        return new CommandCataloguePrivate(*this);
    }

    static void addIndexes(StringMap &indexes, StringList &keys, const std::string &key,
                           const StringList &val) {
        auto vec = map_search<StringList>(indexes, key);
        if (!vec) {
            map_insert<StringList>(indexes, key, val);
            keys.push_back(key);
            return;
        }
        *vec = Utils::concatVector(*vec, val);
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
                auto options =
                    globalOptions ? Utils::concatVector(*globalOptions, d->options) : d->options;
                StringList groupNames(globalOptions ? globalOptions->size() : 0);
                groupNames = Utils::concatVector(groupNames, d->optionGroupNames);

                std::string ss;

                // write command name
                ss += d->name;

                // Build exclusive option group indexes
                // group name -> option subscripts (vector<int> *)
                StringMap exclusiveGroupIndexes = [](const CommandPrivate *d) {
                    StringMap res;
                    for (int i = 0; i < d->optionGroupNames.size(); ++i) {
                        const auto &group = d->optionGroupNames[i];
                        if (group.empty())
                            continue;

                        if (auto optionIndexList = map_search<IntList>(res, group)) {
                            optionIndexList->push_back(i);
                            continue;
                        }
                        res[group] = size_t(new IntList({i}));
                    }
                    return res;
                }(d);

                auto addArgumentsHelp = [&](bool front) {
                    if (bool(displayOptions & Parser::ShowOptionsBehindArguments) != front &&
                        !d->arguments.empty()) {
                        ss += " " + displayedArguments(displayOptions);
                    }
                };

                SSizeMap visitedOptions; // pointer -> none
                auto addExclusiveOptions = [&](int optIdx, bool required) {
                    const auto &opt = options[optIdx];
                    const auto &groupName = groupNames[optIdx];
                    const IntList *optionIndexes;

                    // Search group
                    if (groupName.empty() ||
                        (optionIndexes = map_search<IntList>(exclusiveGroupIndexes, groupName))
                                ->size() <= 1) {
                        ss += opt.helpText(Symbol::HP_Usage, displayOptions);
                        visitedOptions.insert(std::make_pair(size_t(&opt), 0));
                        return;
                    }

                    if (required)
                        ss += "(";

                    // Add exclusive
                    StringList exclusiveOptions;
                    for (const auto &item : *optionIndexes) {
                        const auto &curOpt = d->options[item];
                        exclusiveOptions.push_back(
                            curOpt.helpText(Symbol::HP_Usage, displayOptions));
                        visitedOptions.insert(std::make_pair(size_t(&opt), 0));
                    }

                    ss += Utils::join(exclusiveOptions, " | ");
                    if (required)
                        ss += ")";
                };

                auto addOptionsHelp = [&](bool required) {
                    for (int i = 0; i < options.size(); ++i) {
                        const auto &opt = options[i];
                        if (opt.isRequired() != required || visitedOptions.count(size_t(&opt))) {
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
                    visitedOptions.size() < options.size())
                    addOptionsHelp(false);

                // write backward arguments
                addArgumentsHelp(false);

                // tell the caller if commands and options should be written
                if (a) {
                    // command
                    *(bool *) a[1] = !d->commands.empty();

                    // options
                    *(bool *) a[2] = visitedOptions.size() < options.size() || !d->commands.empty();
                }

                // release indexes
                map_deleteAll<IntList>(exclusiveGroupIndexes);
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
        d->commands = Utils::concatVector(d->commands, commands);
    }

    int Command::optionCount() const {
        Q_D2(Command);
        return int(d->options.size());
    }

    Option Command::option(int index) const {
        Q_D2(Command);
        return d->options[index];
    }

    void Command::addOption(const Option &option, const std::string &group) {
        Q_D(Command);
        d->options.push_back(option);
        d->optionGroupNames.push_back(group);
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

    bool assertCommand(const Command &command) {
        const auto &checkArguments = [](const std::vector<Argument> &input,
                                        const std::string &parent) {
            StringMap argumentNameIndexes;
            int multiValueIndex = -1;
            std::vector<Argument> arguments(input.size());
            for (const auto &arg : input) {
                const auto &d = arg.d_func();
                const auto &name = d->name;

                // Empty argument name?
                if (name.empty()) {
                    u8printf("%s: argument %d doesn't have a name\n", parent.data(),
                             int(arguments.size()));
                    return false;
                }

                // Duplicated argument name?
                if (argumentNameIndexes.count(name)) {
                    u8printf("%s: argument name \"%s\" duplicated\n", parent.data(), name.data());
                    return false;
                }

                // Required argument behind optional one?
                if (!arguments.empty() && arguments.back().isOptional() && d->required) {
                    u8printf("%s: required argument after optional arguments is prohibited\n",
                             parent.data());
                    return false;
                }

                if (arg.multiValueEnabled()) {
                    // Multiple multi-value argument?
                    if (multiValueIndex >= 0) {
                        u8printf("%s: more than one multi-value argument\n", parent.data());
                        return false;
                    }
                    multiValueIndex = int(arguments.size());
                } else if (multiValueIndex >= 0 && !d->required) {
                    // Optional argument after multi-value argument?
                    u8printf("%s: optional argument after multi-value argument is prohibited\n",
                             parent.data());
                    return false;
                }

                // Invalid default value?
                {
                    const auto &expectedValues = d->expectedValues;
                    const auto &defaultValue = d->defaultValue;
                    if (!expectedValues.empty() && defaultValue.type() != Value::Null &&
                        std::find(expectedValues.begin(), expectedValues.end(), defaultValue) ==
                            expectedValues.end()) {
                        u8printf("%s: default value \"%s\" is not in expected values\n",
                                 parent.data(), defaultValue.toString().data());
                        return false;
                    }
                }
                argumentNameIndexes.insert(std::make_pair(name, arguments.size()));
                arguments.push_back(arg);
            }

            return true;
        };

        const auto &checkOptions = [](const std::vector<Option> &input,
                                      const std::vector<std::string> &groups,
                                      const std::string &parent) {
            StringMap optionTokenIndexes;
            std::vector<Option> options(input.size());
            int superPriorOptionIndex = -1;
            for (size_t i = 0; i < input.size(); ++i) {
                const auto &option = input[i];
                const auto &exclusiveGroup = groups[i];

                const auto &d = option.d_func();

                // Empty token?
                if (d->tokens.empty()) {
                    u8printf("%s: null option tokens\n", parent.data());
                    return false;
                }

                for (const auto &token : d->tokens) {
                    // Invalid token?
                    if (token.empty() || !(token.front() == '-' || token.front() == '/')) {
                        u8printf("%s: option token \"%s\" invalid\n", parent.data(), token.data());
                        return false;
                    }

                    // Duplicated token?
                    if (optionTokenIndexes.count(token)) {
                        u8printf("%s: option token \"%s\" duplicated\n", parent.data(),
                                 token.data());
                        return false;
                    }
                    optionTokenIndexes.insert(std::make_pair(token, i));
                }

                // Global and exclusive option?
                if (!exclusiveGroup.empty() && option.isGlobal()) {
                    u8printf("%s: global option \"%s\" cannot be in any exclusive group\n",
                             parent.data(), option.token().data());
                    return false;
                }

                switch (option.priorLevel()) {
                    case Option::AutoSetWhenNoSymbols: {
                        // Auto-option but required?
                        if (d->required) {
                            u8printf("%s: auto-option \"%s\" cannot be required\n", parent.data(),
                                     option.token().data());
                            return false;
                        }

                        // Auto-option with argument?
                        if (!d->arguments.empty()) {
                            u8printf("%s: auto-option \"%s\" cannot have any arguments\n",
                                     parent.data(), option.token().data());
                            return false;
                        }
                        break;
                    }
                    case Option::ExclusiveToOptions:
                    case Option::ExclusiveToAll: {
                        // Multiple exclusively prior option?
                        if (superPriorOptionIndex >= 0) {
                            u8printf("%s: more than one exclusively prior option.\n",
                                     parent.data());
                            return false;
                        }
                        superPriorOptionIndex = int(options.size());
                        break;
                    }
                    default:
                        break;
                }

                for (const auto &token : d->tokens) {
                    optionTokenIndexes.insert(std::make_pair(token, options.size()));
                }

                // Inconsistent exclusive group?
                if (!exclusiveGroup.empty()) {
                    for (size_t j = 0; j < input.size(); ++j) {
                        if (j == i)
                            continue;
                        if (groups[j] == exclusiveGroup &&
                            input[j].isRequired() != option.isRequired()) {
                            u8printf("%s: option \"%s\" is %s, but exclusive group \"%s\" isn't\n",
                                     parent.data(), option.token().data(),
                                     option.isRequired() ? "required" : "optional",
                                     exclusiveGroup.data());
                            return false;
                        }
                    }
                }

                options.push_back(option);
            }
            return true;
        };

        auto d = command.d_func();

        // Check name for root
        if (d->name.empty()) {
            u8printf("command doesn't have a name\n");
            return false;
        }

        // check command names
        {
            StringMap commandNameIndexes;
            for (size_t i = 0; i < d->commands.size(); ++i) {
                const auto &cmd = d->commands[i];
                const auto &name = cmd.name();

                // Empty command name?
                if (name.empty()) {
                    u8printf("%s: command %d doesn't have a name\n", name.data(), int(i));
                    return false;
                }

                // Duplicated command name?
                if (commandNameIndexes.count(name)) {
                    u8printf("%s: command name \"%s\" duplicated\n", d->name.data(), name.data());
                    return false;
                }

                commandNameIndexes.insert(std::make_pair(name, i));
            }
        }

        if (!checkArguments(d->arguments, d->name))
            return false;

        if (!checkOptions(d->options, d->optionGroupNames, d->name))
            return false;

        for (const auto &option : d->options) {
            if (!checkArguments(option.d_func()->arguments, option.token())) {
                return false;
            }
        }

        // Check children
        return std::all_of(d->commands.begin(), d->commands.end(), assertCommand);
    }

}
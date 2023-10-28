#include "command.h"
#include "command_p.h"

#include <algorithm>
#include <utility>

#include "parser.h"
#include "utils.h"

namespace SysCmdLine {

    CommandCataloguePrivate::CommandCataloguePrivate() = default;

    CommandCataloguePrivate::CommandCataloguePrivate(const CommandCataloguePrivate &other)
        : SharedBasePrivate(other) {
        arg = map_copy<StringList>(other.arg);
        opt = map_copy<StringList>(other.opt);
        cmd = map_copy<StringList>(other.cmd);
    }

    CommandCataloguePrivate::~CommandCataloguePrivate() {
        map_deleteAll<StringList>(arg);
        map_deleteAll<StringList>(opt);
        map_deleteAll<StringList>(cmd);
    }

    SharedBasePrivate *CommandCataloguePrivate::clone() const {
        return new CommandCataloguePrivate(*this);
    }

    CommandCatalogue::CommandCatalogue() : SharedBase(new CommandCataloguePrivate()) {
    }

    void CommandCatalogue::addArguments(const std::string &name, const StringList &args) {
        Q_D(CommandCatalogue);
        auto vec = map_search<StringList>(d->arg, name);
        if (!vec) {
            map_insert<StringList>(d->arg, name, args);
            return;
        }
        *vec = Utils::concatVector(*vec, args);
    }

    void CommandCatalogue::addOptions(const std::string &name, const StringList &options) {
        Q_D(CommandCatalogue);
        auto vec = map_search<StringList>(d->opt, name);
        if (!vec) {
            map_insert<StringList>(d->opt, name, options);
            return;
        }
        *vec = Utils::concatVector(*vec, options);
    }

    void CommandCatalogue::addCommands(const std::string &name, const StringList &commands) {
        Q_D(CommandCatalogue);
        auto vec = map_search<StringList>(d->cmd, name);
        if (!vec) {
            map_insert<StringList>(d->cmd, name, commands);
            return;
        }
        *vec = Utils::concatVector(*vec, commands);
    }

    CommandPrivate::CommandPrivate(std::string name, const std::string &desc)
        : ArgumentHolderPrivate(Symbol::ST_Command, desc), name(std::move(name)),
          helpOption(Option::Help), versionOption(Option::Version) {
    }

    SharedBasePrivate *CommandPrivate::clone() const {
        return new CommandPrivate(*this);
    }

    StringMap CommandPrivate::buildExclusiveOptionMap() const {
        StringMap res;
        for (int i = 0; i < options.size(); ++i) {
            const auto &group = optionGroupNames[i];
            if (group.empty())
                continue;

            if (auto optionIndexList = map_search<IntList>(res, group)) {
                optionIndexList->push_back(i);
                continue;
            }
            res[group] = size_t(new IntList({i}));
        }
        return res;
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
                StringMap exclusiveGroupIndexes = d->buildExclusiveOptionMap();

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
                    const IntList *optionIndexes = nullptr;

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
        d->versionOption.setTokens(tokens);
        d->versionOption.setPriorLevel(Option::IgnoreMissingSymbols);
    }

    void Command::addHelpOption(bool showHelpIfNoArg, bool global, const StringList &tokens) {
        Q_D(Command);
        d->helpOption.setTokens(tokens);
        d->helpOption.setPriorLevel(showHelpIfNoArg ? Option::AutoSetWhenNoSymbols
                                                    : Option::IgnoreMissingSymbols);
        d->helpOption.setGlobal(global);
    }

    bool assertCommand(const Command &command) {
        return {};
    }

}
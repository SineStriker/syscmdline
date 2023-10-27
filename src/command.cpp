#include "command.h"
#include "command_p.h"

#include <set>
#include <stdexcept>
#include <algorithm>
#include <utility>

#include "option_p.h"
#include "strings.h"
#include "parser.h"
#include "utils.h"

#include "capi/hash.h"

namespace SysCmdLine {

    SharedBasePrivate *CommandCataloguePrivate::clone() const {
        return new CommandCataloguePrivate(*this);
    }

    CommandPrivate::CommandPrivate(std::string name, const std::string &desc)
        : ArgumentHolderPrivate(Symbol::ST_Command, desc), name(std::move(name)),
          helpOption(Option::Help), versionOption(Option::Version) {
    }

    SharedBasePrivate *CommandPrivate::clone() const {
        return nullptr;
    }

    Command::Command() : Command({}, {}) {
    }

    Command::Command(const std::string &name, const std::string &desc)
        : ArgumentHolder(new CommandPrivate(name, desc)) {
    }

    std::string Command::helpText(Symbol::HelpPosition pos, int displayOptions, void *extra) const {
        Q_D2(Command);
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

                std::set<const Option *> printedOptions;
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
                        printedOptions.insert(&curOpt);
                    }

                    ss += Utils::join(exclusiveOptions, " | ");
                    if (needParen)
                        ss += ")";
                };

                // required options
                if (!(displayOptions & Parser::DontShowRequiredOptionsOnUsage)) {
                    for (const auto &opt : options) {
                        if (!opt.first.isRequired()) {
                            continue;
                        }

                        if (printedOptions.count(opt.first.name()))
                            continue;

                        // check exclusive
                        ss += " ";
                        printExclusiveOptions(opt.first, true);
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
                if (!d->commands.empty()) {
                    ss += " [commands]";
                }

                // options
                if (printedOptions.size() < options.size() || !d->commands.empty()) {
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
        d->commands.reserve(d->commands.size() + commands.size());
        for (const auto &item : commands)
            d->commands.push_back(item);
    }

    int Command::optionCount() const {
        Q_D2(Command);
        return int(d->options.size());
    }

    Option Command::option(int index) const {
        Q_D2(Command);
        return d->options[index].first;
    }

    void Command::addOptions(const std::vector<std::pair<Option, int>> &options) {
        Q_D(Command);
        d->options.reserve(d->options.size() + options.size());
        for (const auto &item : options)
            d->options.push_back(item);
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

    void Command::addVersionOption(const std::string &ver, const std::vector<std::string> &tokens) {
        Q_D(Command);
        d->version = ver;
        d->versionOption.setTokens(tokens);
        d->versionOption.setPriorLevel(Option::IgnoreMissingSymbols);
    }

    void Command::addHelpOption(bool showHelpIfNoArg, bool global,
                                const std::vector<std::string> &tokens) {
        Q_D(Command);
        d->helpOption.setTokens(tokens);
        d->helpOption.setPriorLevel(showHelpIfNoArg ? Option::AutoSetWhenNoSymbols
                                                    : Option::IgnoreMissingSymbols);
        d->helpOption.setGlobal(global);
    }

}
#include "parser.h"
#include "parser_p.h"

#include <list>
#include <map>
#include <set>
#include <stdexcept>
#include <sstream>
#include <cctype>
#include <algorithm>

#include "strings.h"
#include "utils.h"
#include "system.h"
#include "command_p.h"
#include "option_p.h"

namespace SysCmdLine {

    static void listItems(std::stringstream &ss, const std::string &title,
                          const std::vector<std::pair<std::string, std::string>> &contents) {
        if (contents.empty())
            return;

        ss << std::endl;

        int widest = 0;
        for (const auto &item : contents) {
            widest = std::max<int>(int(item.first.size()), widest);
        }

        ss << title << ": " << std::endl;

        for (const auto &item : contents) {
            auto lines = Utils::split<char>(item.second, "\n");
            if (lines.empty())
                lines.emplace_back();

            ss << Strings::indent << std::left << std::setw(widest) << item.first //
               << Strings::spacing                                                //
               << lines.front() << std::endl;
            for (int i = 1; i < lines.size(); ++i) {
                ss << Strings::indent << std::left << std::setw(widest) << ' ' //
                   << Strings::spacing                                         //
                   << lines.at(i) << std::endl;
            }
        }
    }

    static void collectItems(
        std::stringstream &ss, const std::vector<std::unordered_set<std::string>> &catalogue,
        const std::unordered_map<std::string, size_t> &catalogueIndexes,
        const std::unordered_map<std::string, size_t> &itemIndexes, const std::string &defaultTitle,
        const std::function<std::pair<std::string, std::string>(size_t)> &getter) {

        auto indexes = itemIndexes;
        for (const auto &pair : catalogueIndexes) {
            std::set<size_t> subscriptSet;
            for (const auto &name : catalogue[pair.second]) {
                auto it = indexes.find(name);
                if (it == indexes.end())
                    continue;
                subscriptSet.insert(it->second);
                indexes.erase(it);
            }

            std::vector<std::pair<std::string, std::string>> texts;
            for (const auto &subscript : std::as_const(subscriptSet)) {
                texts.emplace_back(getter(subscript));
            }
            listItems(ss, pair.first, texts);
        }

        {
            std::set<size_t> subscriptSet;
            for (const auto &pair : std::as_const(indexes)) {
                subscriptSet.insert(pair.second);
            }

            std::vector<std::pair<std::string, std::string>> texts;
            texts.reserve(subscriptSet.size());
            for (const auto &subscript : std::as_const(subscriptSet)) {
                texts.emplace_back(getter(subscript));
            }
            listItems(ss, defaultTitle, texts);
        }
    }

    std::string ParseResultData::commandHelpText() const {
        const auto &d = command->d_func();
        const auto &dd = d->catalogue.d_func();
        const auto displayOptions = parserData->displayOptions;

        // Build option indexes
        auto options = d->options;
        auto optionNameIndexes = d->optionNameIndexes;

        options.reserve(options.size() + globalOptions.size());
        optionNameIndexes.reserve(optionNameIndexes.size() + globalOptions.size());
        for (const auto &item : globalOptions) {
            optionNameIndexes.insert(std::make_pair(item->name(), options.size()));
            options.push_back(*item);
        }

        std::stringstream ss;

        // Description
        const auto &desc = d->detailedDescription.empty() ? d->desc : d->detailedDescription;
        if (!desc.empty()) {
            ss << Strings::text(Strings::Title, Strings::Description) << ": " << std::endl;

            auto lines = Utils::split<char>(desc, "\n");
            for (const auto &line : std::as_const(lines))
                ss << Strings::indent << line << std::endl;
            ss << std::endl;
        }

        // Usage
        ss << Strings::text(Strings::Title, Strings::Usage) << ": " << std::endl;
        {

            ss << Strings::indent;

            // parent commands
            {
                const Command *p = &parserData->rootCommand;
                for (const auto &item : std::as_const(stack)) {
                    ss << p->name() << " ";
                    p = &p->d_func()->subCommands[item];
                }
            }

            // name
            ss << d->name;

            // arguments
            if (displayOptions & Parser::ShowOptionsBehindArguments) {
                if (!d->arguments.empty()) {
                    ss << " " << command->displayedArguments();
                }
            }

            // required options
            std::unordered_set<std::string> printedOptions;
            auto printExclusiveOptions = [&](const Option &opt, bool needParen) {
                auto it = d->exclusiveGroupIndexes.find(opt.name());
                if (it == d->exclusiveGroupIndexes.end()) {
                    ss << opt.displayedText(false);
                    printedOptions.insert(opt.name());
                    return;
                }

                const auto &arr = d->exclusiveGroups.find(it->second)->second;
                if (arr.size() <= 1) {
                    ss << opt.displayedText(false);
                    printedOptions.insert(opt.name());
                    return;
                }

                if (needParen)
                    ss << "(";
                std::vector<std::string> exclusiveOptions;
                for (const auto &item : arr) {
                    const auto &curOpt = d->options[item];
                    exclusiveOptions.push_back(curOpt.displayedText(false));
                    printedOptions.insert(curOpt.name());
                }

                ss << Utils::join<char>(exclusiveOptions, " | ");
                if (needParen)
                    ss << ")";
            };

            if (!(displayOptions & Parser::DontShowRequiredOptionsOnUsage)) {
                for (const auto &opt : options) {
                    if (!opt.isRequired()) {
                        continue;
                    }

                    if (printedOptions.count(opt.name()))
                        continue;

                    // check exclusive
                    ss << " ";
                    printExclusiveOptions(opt, true);
                }
            }

            if ((displayOptions & Parser::ShowOptionalOptionsOnUsage) &&
                printedOptions.size() < options.size()) {
                for (const auto &opt : options) {
                    if (opt.isRequired()) {
                        continue;
                    }

                    if (printedOptions.count(opt.name()))
                        continue;

                    // check exclusive
                    ss << " [";
                    printExclusiveOptions(opt, false);
                    ss << "]";
                }
            }

            // arguments
            if (!(displayOptions & Parser::ShowOptionsBehindArguments)) {
                if (!d->arguments.empty()) {
                    ss << " " << command->displayedArguments();
                }
            }

            // command
            if (!d->subCommands.empty()) {
                ss << " [commands]";
            }

            // options
            if (printedOptions.size() < options.size() || !d->subCommands.empty()) {
                ss << " [options]";
            }

            ss << std::endl;
        }

        // Arguments
        if (!d->arguments.empty()) {
            collectItems(
                ss, dd->_arg, dd->_argIndexes, d->argumentNameIndexes,
                Strings::text(Strings::Title, Strings::Arguments),

                // getter
                [&](size_t idx) -> std::pair<std::string, std::string> {
                    const auto &arg = d->arguments[idx];

                    const auto &d1 = arg.d_func();
                    std::string appendix;

                    // Required
                    if (d1->required && (displayOptions & Parser::ShowArgumentIsRequired)) {
                        appendix += " [" + Strings::text(Strings::Title, Strings::Required) + "]";
                    }

                    // Default Value
                    if (d1->defaultValue.type() != Value::Null &&
                        (displayOptions & Parser::ShowArgumentDefaultValue)) {
                        appendix += " [" + Strings::text(Strings::Title, Strings::Default) + ": " +
                                    d1->defaultValue.toString() + "]";
                    }

                    // Expected Values
                    if (!d1->expectedValues.empty() &&
                        (displayOptions & Parser::ShowArgumentExpectedValues)) {
                        std::vector<std::string> values;
                        values.reserve(d1->expectedValues.size());
                        for (const auto &item : d1->expectedValues) {
                            switch (item.type()) {
                                case Value::String:
                                    values.push_back("\"" + item.toString() + "\"");
                                    break;
                                default:
                                    values.push_back(item.toString());
                                    break;
                            }
                        }
                        appendix += " [" + Strings::text(Strings::Title, Strings::ExpectedValues) +
                                    ": " + Utils::join<char>(values, ", ") + "]";
                    }
                    return {arg.displayedText(), d1->desc + appendix};
                });
        }

        // Options
        if (!options.empty()) {
            collectItems(ss, dd->_opt, dd->_optIndexes, optionNameIndexes,
                         Strings::text(Strings::Title, Strings::Options),

                         // getter
                         [&](size_t idx) -> std::pair<std::string, std::string> {
                             const auto &opt = options[idx];

                             const auto &d1 = opt.d_func();
                             std::string appendix;

                             // Required
                             if (d1->required && (displayOptions & Parser::ShowOptionIsRequired)) {
                                 appendix +=
                                     " [" + Strings::text(Strings::Title, Strings::Required) + "]";
                             }
                             return {opt.displayedText(), d1->desc + appendix};
                         });
        }

        // Commands
        if (!d->subCommands.empty()) {
            collectItems(ss, dd->_cmd, dd->_cmdIndexes, d->subCommandNameIndexes,
                         Strings::text(Strings::Title, Strings::Commands),

                         // getter
                         [&](size_t idx) -> std::pair<std::string, std::string> {
                             const auto &cmd = d->subCommands[idx];

                             const auto &d1 = cmd.d_func();
                             return {d1->name, d1->desc};
                         });
        }

        return ss.str();
    }

    std::string ParseResultData::correctionText() const {
        std::vector<std::string> expectedValues;

        switch (error) {
            case ParseResult::UnknownOption:
            case ParseResult::InvalidOptionPosition: {
                for (const auto &opt : globalOptions) {
                    for (const auto &token : opt->tokens()) {
                        expectedValues.push_back(token);
                    }
                }
                for (const auto &opt : command->options()) {
                    for (const auto &token : opt.tokens()) {
                        expectedValues.push_back(token);
                    }
                }

                if (error == ParseResult::UnknownOption)
                    break;

                // Fallback as invalid argument case
            }
            case ParseResult::InvalidArgumentValue: {
                const auto &arg = command->argument(errorPlaceholders[1]);
                for (const auto &item : arg.expectedValues()) {
                    expectedValues.push_back(item.toString());
                }

                if (!command->hasArgument(arg.name())) // option argument?
                    break;

                // Fallback as unknown command case
            }
            case ParseResult::UnknownCommand: {
                for (const auto &cmd : command->commands()) {
                    expectedValues.push_back(cmd.name());
                }
                break;
            }
            default:
                return {};
        }

        auto input = errorPlaceholders[0];
        auto suggestions = Utils::calcClosestTexts(expectedValues, input, int(input.size()) / 2);
        if (suggestions.empty())
            return {};

        std::stringstream ss;
        ss << Utils::formatText(Strings::text(Strings::Information, Strings::MatchCommand), {input})
           << std::endl;
        for (const auto &item : std::as_const(suggestions)) {
            ss << Strings::indent << item << std::endl;
        }
        return ss.str();
    }

    Value ParseResultData::getDefaultResult(const SysCmdLine::ArgumentHolder *argumentHolder,
                                            const std::string &argName) {
        const auto &d2 = argumentHolder->d_func();
        auto it2 = d2->argumentNameIndexes.find(argName);
        if (it2 == d2->argumentNameIndexes.end())
            return {};
        return d2->arguments.at(it2->second).defaultValue();
    }

    Value ParseResultData::getDefaultResult(const std::string &optName,
                                            const std::string &argName) const {
        const auto &d = command->d_func();
        auto it = d->optionNameIndexes.find(optName);
        if (it == d->optionNameIndexes.end())
            return {};
        return getDefaultResult(&d->options.at(it->second), argName);
    }

    void ParseResultData::showHelp(const std::function<void()> &messageCaller) const {
        if (!parserData->intro[Parser::Prologue].empty()) {
            u8printf("%s\n\n", parserData->intro[Parser::Prologue].data());
        }

        if (messageCaller) {
            messageCaller();
            u8printf("\n");
        }

        u8printf("%s", commandHelpText().data());

        if (!parserData->intro[Parser::Epilogue].empty()) {
            u8printf("\n%s\n", parserData->intro[Parser::Epilogue].data());
        }
    }

    ParseResult::ParseResult() {
    }

    ParseResult::~ParseResult() {
    }

    ParseResult::ParseResult(const ParseResult &other) {
        d_ptr = other.d_ptr;
    }

    ParseResult::ParseResult(ParseResult &&other) noexcept {
        d_ptr.swap(other.d_ptr);
    }

    ParseResult &ParseResult::operator=(const ParseResult &other) {
        if (this == &other) {
            return *this;
        }
        d_ptr = other.d_ptr;
        return *this;
    }

    ParseResult &ParseResult::operator=(ParseResult &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        d_ptr.swap(other.d_ptr);
        return *this;
    }

    Command ParseResult::rootCommand() const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        return d->parserData->rootCommand;
    }

    const std::vector<std::string> &ParseResult::arguments() const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        return d->arguments;
    }

    int ParseResult::invoke(int errCode) const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        if (d->error != NoError) {
            showError();
            return errCode;
        }
        return dispatch();
    }

    int ParseResult::dispatch() const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        if (d->error != NoError) {
            throw std::runtime_error("cannot dispatch handler when parser failed");
        }

        const auto &cmd = *d->command;
        const auto &handler = cmd.handler();

        if (d->versionSet) {
            u8printf("%s\n", cmd.version().data());
            return 0;
        }

        if (d->helpSet) {
            showHelpText();
            return 0;
        }

        if (cmd.d_func()->showHelpIfNoArg && isResultNull()) {
            showHelpText();
            return 0;
        }

        if (!handler) {
            throw std::runtime_error("command \"" + cmd.name() + "\" doesn't have a valid handler");
        }

        return handler(*this);
    }

    ParseResult::Error ParseResult::error() const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        return d->error;
    }

    std::string ParseResult::errorText() const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        if (d->error == NoError)
            return {};
        return Utils::formatText(Strings::text(Strings::ParseError, d->error),
                                 d->errorPlaceholders);
    }

    std::string ParseResult::correctionText() const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        return d->correctionText();
    }

    Command ParseResult::command() const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        return *d->command;
    }

    std::vector<Option> ParseResult::globalOptions() const {
        SYSCMDLINE_GET_DATA(const ParseResult);

        std::vector<Option> res;
        res.reserve(d->globalOptions.size());
        for (const auto &item : d->globalOptions) {
            res.push_back(*item);
        }
        return res;
    }

    std::vector<int> ParseResult::commandIndexStack() const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        return d->stack;
    }

    void ParseResult::showError() const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        if (d->error == NoError)
            return;

        auto errCallback = [this, d]() {
            if (!(d->parserData->displayOptions & Parser::SkipCorrection)) {
                if (auto correction = d->correctionText(); !correction.empty()) {
                    u8printf("%s", correction.data());
                }
            }
            u8error("%s: %s\n", Strings::text(Strings::Title, Strings::Error).data(),
                    errorText().data());
        };

        if (!(d_ptr->parserData->displayOptions & Parser::DontShowHelpOnError) &&
            d->command->d_func()->optionNameIndexes.count("help")) {
            d->showHelp(errCallback);
        } else {
            errCallback();
        }
    }

    void ParseResult::showHelpText() const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        d->showHelp();
    }

    void ParseResult::showErrorAndHelpText(const std::string &message) const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        d->showHelp([&message]() {
            u8error("%s\n", message.data()); //
        });
    }

    void ParseResult::showWarningAndHelpText(const std::string &message) const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        d->showHelp([&message]() {
            u8warning("%s\n", message.data()); //
        });
    }

    Value ParseResult::valueForArgument(const std::string &argName) const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        auto it = d->argResult.find(argName);
        if (it == d->argResult.end()) {
            return d_ptr->getDefaultResult(d_ptr->command, argName);
        }
        return it->second.front();
    }

    std::vector<Value> ParseResult::valuesForArgument(const std::string &argName) const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        auto it = d->argResult.find(argName);
        if (it == d->argResult.end()) {
            return {};
        }
        return it->second;
    }

    int ParseResult::optionCount(const std::string &optName) const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        const auto &map = d->optResult;
        auto it = map.find(optName);
        if (it == map.end()) {
            return 0;
        }
        return int(it->second.size());
    }

    Value ParseResult::valueForOption(const std::string &optName, const std::string &argName,
                                      int count) const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        auto it = d->optResult.find(optName);
        if (it == d->optResult.end() || count >= it->second.size()) {
            return d->getDefaultResult(optName, argName);
        }

        const auto &map = it->second.at(count);
        auto it2 = map.find(argName);
        if (it2 == map.end()) {
            return {};
        }
        return it2->second;
    }

    Value ParseResult::valueForOption(const std::string &optName, int argIndex, int count) const {
        const Option &opt = d_ptr->command->option(optName);
        const auto &args = opt.arguments();
        if (argIndex >= args.size())
            return {};

        auto it = d_ptr->optResult.find(optName);
        if (it == d_ptr->optResult.end() || count >= it->second.size()) {
            return d_ptr->getDefaultResult(optName, args[argIndex].name());
        }

        const auto &map = it->second.at(count);
        auto it2 = map.find(args[argIndex].name());
        if (it2 == map.end()) {
            return {};
        }
        return it2->second;
    }

    std::vector<std::string> ParseResult::effectiveOptions() const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        std::vector<std::string> res;
        res.reserve(d->optResult.size());
        for (const auto &item : d->optResult) {
            res.push_back(item.first);
        }
        return res;
    }

    std::vector<std::string> ParseResult::effectiveArguments() const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        std::vector<std::string> res;
        res.reserve(d->argResult.size());
        for (const auto &item : d->argResult) {
            res.push_back(item.first);
        }
        return res;
    }

    bool ParseResult::isHelpSet() const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        return d->helpSet;
    }

    bool ParseResult::isVersionSet() const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        return d->versionSet;
    }

    bool ParseResult::isResultNull() const {
        SYSCMDLINE_GET_DATA(const ParseResult);
        return d->argResult.empty() && d->optResult.empty();
    }

    ParseResult::ParseResult(ParseResultData *d) : d_ptr(d) {
    }

    Parser::Parser() : d_ptr(new ParserData()) {
    }

    Parser::Parser(const Command &rootCommand) : d_ptr(new ParserData()) {
        setRootCommand(rootCommand);
    }

    Parser::~Parser() {
    }

    Parser::Parser(const Parser &other) {
        d_ptr = other.d_ptr;
    }

    Parser::Parser(Parser &&other) noexcept {
        d_ptr.swap(other.d_ptr);
    }

    Parser &Parser::operator=(const Parser &other) {
        if (this == &other) {
            return *this;
        }
        d_ptr = other.d_ptr;
        return *this;
    }

    Parser &Parser::operator=(Parser &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        d_ptr.swap(other.d_ptr);
        return *this;
    }

    std::string Parser::intro(Position pos) const {
        SYSCMDLINE_GET_DATA(const Parser);
        return d->intro[pos];
    }

    void Parser::setIntro(Position pos, const std::string &text) {
        SYSCMDLINE_GET_DATA( Parser);
        d->intro[pos] = text;
    }

    int Parser::displayOptions() const {
        SYSCMDLINE_GET_DATA(const Parser);
        return d->displayOptions;
    }

    void Parser::setDisplayOptions(int displayOptions) {
        SYSCMDLINE_GET_DATA( Parser);
        d->displayOptions = displayOptions;
    }

    Command Parser::rootCommand() const {
        SYSCMDLINE_GET_DATA(const Parser);
        return d->rootCommand;
    }

    void Parser::setRootCommand(const Command &rootCommand) {
        SYSCMDLINE_GET_DATA( Parser);
        if (rootCommand.d_func()->name.empty()) {
            throw std::runtime_error("empty root command name");
        }
        d->rootCommand = rootCommand;
    }

    ParseResult Parser::parse(const std::vector<std::string> &args, int parseOptions) {
        auto result = new ParseResultData(d_ptr, args);
        auto &error = result->error;
        auto &errorPlaceholders = result->errorPlaceholders;

        // Search command
        const Command *cmd = &d_ptr->rootCommand;
        std::list<const Option *> globalOptions;
        std::unordered_map<std::string, decltype(globalOptions)::iterator> globalOptionIndexes;
        std::unordered_map<std::string, decltype(globalOptions)::iterator> globalOptionTokenIndexes;

        auto removeDuplicatedOptions = [&](const Option &opt) {
            // Search name
            {
                auto optIdxIt = globalOptionIndexes.find(opt.name());
                if (optIdxIt != globalOptionIndexes.end()) {
                    auto optIt = optIdxIt->second;
                    const auto &targetOpt = *optIt;

                    // Remove all token indexes
                    for (const auto &targetToken : targetOpt->d_func()->tokens) {
                        globalOptionTokenIndexes.erase(targetToken);
                    }

                    // Remove name index
                    globalOptionIndexes.erase(optIdxIt);

                    // Remove option
                    globalOptions.erase(optIt);
                }
            }

            // Search tokens
            for (const auto &token : opt.d_func()->tokens) {
                auto optTokenIdxIt = globalOptionTokenIndexes.find(token);
                if (optTokenIdxIt != globalOptionTokenIndexes.end()) {
                    auto optIt = optTokenIdxIt->second;
                    const auto &targetOpt = *optIt;

                    // Remove all token indexes
                    for (const auto &targetToken : targetOpt->d_func()->tokens) {
                        globalOptionTokenIndexes.erase(targetToken);
                    }

                    // Remove name index
                    globalOptionIndexes.erase(targetOpt->name());

                    // Remove option
                    globalOptions.erase(optIt);
                }
            }
        };

        // Find target command
        size_t i = 1;
        for (; i < args.size(); ++i) {
            auto lastCmd = cmd;
            {
                const auto &d = cmd->d_func();
                const auto &arg = args[i];

                auto it = d->subCommandNameIndexes.find(arg);
                if (it == d->subCommandNameIndexes.end()) {
                    if (parseOptions & Parser::IgnoreCommandCase) {
                        // Search
                        bool found = false;
                        for (int j = 0; j < d->subCommands.size(); ++j) {
                            const auto &subCmd = d->subCommands.at(j);
                            if (Utils::toLower(subCmd.name()) == Utils::toLower(arg)) {
                                result->stack.push_back(j);
                                cmd = &subCmd;

                                found = true;
                                break;
                            }
                        }

                        if (!found)
                            break;

                    } else {
                        break;
                    }
                } else {
                    result->stack.push_back(int(it->second));
                    cmd = &cmd->d_func()->subCommands.at(it->second);
                }
            }

            // Collect global options
            for (const auto &opt : lastCmd->d_func()->options) {
                if (!opt.isGlobal())
                    continue;

                removeDuplicatedOptions(opt);

                // Add option
                auto targetIterator = globalOptions.insert(globalOptions.end(), &opt);

                // Add name index
                globalOptionIndexes.insert(std::make_pair(opt.name(), targetIterator));

                // Add token indexes
                for (const auto &token : opt.d_func()->tokens) {
                    globalOptionTokenIndexes.insert(std::make_pair(token, targetIterator));
                }
            }
        }

        // Remove duplicated global options
        if (!globalOptionIndexes.empty()) {
            for (const auto &opt : cmd->d_func()->options) {
                removeDuplicatedOptions(opt);
            }
        }
        result->command = cmd;
        result->globalOptions = {globalOptions.begin(), globalOptions.end()};

        // Build option indexes
        std::map<std::string, const Option *> allOptionIndexes;
        for (const auto &item : std::as_const(globalOptions)) {
            for (const auto &token : item->d_func()->tokens) {
                allOptionIndexes.insert(std::make_pair(token, item));
            }
        }
        for (const auto &item : std::as_const(cmd->d_func()->options)) {
            for (const auto &token : item.d_func()->tokens) {
                allOptionIndexes.insert(std::make_pair(token, &item));
            }
        }

        // Build case-insensitive option indexes if needed
        std::map<std::string, const Option *> lowerCaseOptionIndexes;
        if (parseOptions & Parser::IgnoreOptionCase) {
            for (const auto &item : std::as_const(globalOptions)) {
                for (const auto &token : item->d_func()->tokens) {
                    lowerCaseOptionIndexes.insert(std::make_pair(Utils::toLower(token), item));
                }
            }
            for (const auto &item : std::as_const(cmd->d_func()->options)) {
                for (const auto &token : item.d_func()->tokens) {
                    lowerCaseOptionIndexes.insert(std::make_pair(Utils::toLower(token), &item));
                }
            }
        }

        auto searchShortOptions = [](const std::map<std::string, const Option *> &indexes,
                                     const std::string &token, char sign,
                                     int *pos) -> const Option * {
            // Search for short option
            auto it = indexes.lower_bound(token);
            if (it != indexes.begin() && it != indexes.end() && token.find(it->first) != 0) {
                --it;
            }

            const auto &prefix = it->first;
            if (prefix == "--")
                return nullptr;

            if (it != indexes.end() && Utils::starts_with(token, prefix)) {
                const auto &opt = it->second;
                const auto &args = opt->d_func()->arguments;
                if (args.size() != 1 || !args.front().isRequired()) {
                    return nullptr;
                }

                switch (opt->shortMatchRule()) {
                    case Option::ShortMatchSingleChar: {
                        if (token.size() > 2)
                            break;
                    }
                    case Option::ShortMatchSingleLetter: {
                        if (!std::isalpha(token.at(1)))
                            break;
                    }
                    case Option::ShortMatchAll: {
                        if (pos)
                            *pos = int(prefix.size());
                        return opt;
                    }
                    default:
                        break;
                }

                if (token.at(prefix.size()) == sign) {
                    if (pos)
                        *pos = int(prefix.size()) + 1;
                    return opt;
                }
            }
            return nullptr;
        };

        auto searchOptionImpl = [&](const std::map<std::string, const Option *> &indexes,
                                    const std::string &token, int *pos) -> const Option * {
            if (pos)
                *pos = -1;

            if (indexes.empty())
                return nullptr;

            auto it = indexes.find(token);
            if (it != indexes.end()) {
                return it->second;
            }

            if (token.size() > 1) {
                if (!(parseOptions & Parser::DontAllowUnixKeyValueOptions)) {
                    if (token.front() == '-') {
                        return searchShortOptions(indexes, token, '-', pos);
                    }
                }

                if (parseOptions & Parser::AllowDosKeyValueOptions) {
                    if (token.front() == '/') {
                        return searchShortOptions(indexes, token, ':', pos);
                    }
                }
            }
            return nullptr;
        };

        auto searchOption = [&](const std::string &token, int *pos = nullptr) -> const Option * {
            auto opt = searchOptionImpl(allOptionIndexes, token, pos);
            if (opt)
                return opt;
            if (parseOptions & Parser::IgnoreOptionCase) {
                return searchOptionImpl(lowerCaseOptionIndexes, Utils::toLower(token), pos);
            }
            return nullptr;
        };

        auto searchContinuousFlags = [&](const std::string &flags) -> std::vector<const Option *> {
            std::vector<const Option *> res;
            for (const auto &flag : flags) {
                auto it = allOptionIndexes.find(std::string("-") + flag);
                if (it == allOptionIndexes.end()) {
                    return {};
                }
                const auto &opt = it->second;
                const auto &d = opt->d_func();
                if (!d->arguments.empty()) {
                    return {};
                }
                res.push_back(opt);
            }
            return res;
        };

        auto checkArgument = [&](const Argument *arg, const std::string &token, Value *out,
                                 bool setError = true) {
            const auto &d = arg->d_func();
            const auto &expectedValues = d->expectedValues;
            if (!expectedValues.empty()) {
                for (const auto &item : expectedValues) {
                    auto val = Value::fromString(token, item.type());
                    if (val == item) {
                        *out = val;
                        return true;
                    }
                }
                if (setError) {
                    if (token.front() == '-') {
                        error = ParseResult::InvalidOptionPosition;
                        errorPlaceholders = {token, arg->name()};
                    } else {
                        error = ParseResult::InvalidArgumentValue;
                        errorPlaceholders = {token, arg->name()};
                    }
                }
                return false;
            }

            if (d->validator) {
                std::string errorMessage;
                if (d->validator(token, out, &errorMessage)) {
                    return true;
                }
                error = ParseResult::ArgumentValidateFailed;
                errorPlaceholders = {token, arg->name(), errorMessage};
                return false;
            }

            const char *expected;
            const auto &type = d->defaultValue.type();
            switch (type) {
                case Value::Int: {
                    expected = "int";
                    break;
                }
                case Value::Double: {
                    expected = "double";
                    break;
                }
                case Value::String: {
                    expected = "string";
                    break;
                }
                default: {
                    *out = token; // no default value
                    return true;
                }
            }
            if (auto val = Value::fromString(token, type); val.type() != Value::Null) {
                *out = val;
                return true;
            }
            error = ParseResult::ArgumentTypeMismatch;
            errorPlaceholders = {token, arg->name(), expected};
            return false;
        };

        std::unordered_map<int, const Option *> encounteredExclusiveGroups;
        auto searchExclusiveOption = [&](const Option *opt,
                                         int *outGroup = nullptr) -> const Option * {
            if (outGroup) {
                *outGroup = -1;
            }

            const auto &map = cmd->d_func()->exclusiveGroupIndexes;
            auto it = map.find(opt->name());
            if (it != map.end()) {
                const auto &group = it->second;
                if (outGroup) {
                    *outGroup = group;
                }

                auto it2 = encounteredExclusiveGroups.find(group);
                if (it2 != encounteredExclusiveGroups.end()) {
                    return it2->second;
                }
            }
            return nullptr;
        };

        auto checkOptionCommon =
            [&](const Option *opt, const std::vector<ParseResultData::ArgResult> &resVec) -> bool {
            // Check max occurrence
            if (opt->maxOccurrence() > 0 && resVec.size() == opt->maxOccurrence()) {
                error = ParseResult::OptionOccurTooMuch;
                errorPlaceholders = {
                    opt->displayedTokens(),
                    std::to_string(opt->maxOccurrence()),
                };
                return false;
            }

            // Check exclusive
            {
                int group;
                const auto &exclusiveOpt = searchExclusiveOption(opt, &group);
                if (exclusiveOpt) {
                    error = ParseResult::MutuallyExclusiveOptions;
                    errorPlaceholders = {
                        exclusiveOpt->displayedText(),
                        opt->displayedText(),
                    };
                    return false;
                }

                // Note current option
                if (group >= 0)
                    encounteredExclusiveGroups.insert(std::make_pair(group, opt));
            }
            return true;
        };

        // Parse options
        std::vector<std::string> positionalArguments;
        const Option *priorOpt = nullptr;
        for (auto j = i; j < args.size(); ++j) {
            const auto &token = args[j];

            // Consider option
            int pos;
            if (auto opt = searchOption(token, &pos); opt) {
                auto &resVec = result->optResult[opt->name()];
                const auto &optArgs = opt->d_func()->arguments;
                ParseResultData::ArgResult curArgResult;

                // Check following argument
                size_t x = 0;
                if (pos >= 0) {
                    const auto &arg = optArgs.front();
                    Value val;
                    if (!checkArgument(&arg, token.substr(pos), &val, false)) {
                        break;
                    }
                    curArgResult.insert(std::make_pair(arg.name(), val));
                    x = 1;
                }

                // Check option common
                if (!checkOptionCommon(opt, resVec)) {
                    break;
                }

                size_t start = j + 1 - x;
                size_t max = std::min(args.size() - start, optArgs.size());
                for (; x < max; ++x) {
                    const auto &nextToken = args[x + start];
                    const auto &arg = optArgs.at(x);

                    // Break by next option
                    if (nextToken.front() == '-' && !arg.isRequired() && searchOption(nextToken)) {
                        break;
                    }

                    // Check argument
                    Value val;
                    if (!checkArgument(&arg, nextToken, &val, arg.isRequired())) {
                        break;
                    }
                    curArgResult.insert(std::make_pair(arg.name(), val));
                }

                if (error != ParseResult::NoError)
                    break;

                // Check required arguments
                if (x < optArgs.size()) {
                    const auto &arg = optArgs.at(x);
                    if (arg.isRequired()) {
                        error = ParseResult::MissingOptionArgument;
                        errorPlaceholders = {arg.name(), token};
                        break;
                    }
                }

                resVec.emplace_back(curArgResult);

                if (opt->priorLevel() > (priorOpt ? priorOpt->priorLevel() : Option::NoPrior))
                    priorOpt = opt;
                j = start - 1 + x;
                continue;
            }

            auto isFlags = [](const std::string &s) -> bool {
                if (s.size() <= 1 || s.front() != '-')
                    return false;
                return std::all_of(s.begin() + 1, s.end(), [](char c) {
                    return std::isalpha(c) || std::isdigit(c); //
                });
            };

            // Consider short flags
            if ((parseOptions & Parser::AllowUnixGroupFlags) && isFlags(token)) {
                auto opts = searchContinuousFlags(token.substr(1));
                if (!opts.empty()) {
                    bool failed = false;
                    for (const auto &opt : std::as_const(opts)) {
                        auto &resVec = result->optResult[opt->name()];
                        // Check option common
                        if (!checkOptionCommon(opt, resVec)) {
                            failed = true;
                            break;
                        }
                        resVec.emplace_back();
                    }

                    if (failed) {
                        break;
                    }
                    continue;
                }
            }

            // Consider argument
            positionalArguments.push_back(token);
        }

        // Parse arguments
        const Argument *missingArgument = nullptr;
        {
            const auto &d = cmd->d_func();
            const auto &cmdArgs = d->arguments;

            // Parse forward
            size_t end = cmdArgs.size();
            if (d->multiValueIndex >= 0) {
                end = d->multiValueIndex + 1; // Stop after multi-value arg
            }

            size_t k = 0;
            for (size_t max = std::min(positionalArguments.size(), end); k < max; ++k) {
                const auto &arg = cmdArgs.at(k);
                Value val;
                if (!checkArgument(&arg, positionalArguments.at(k), &val)) {
                    goto out_parse_arguments;
                }
                result->argResult[arg.name()].push_back(val);
            }

            if (k < end) {
                const auto &arg = cmdArgs.at(k);
                if (arg.isRequired()) {
                    missingArgument = &arg;
                }
                goto out_parse_arguments;
            }

            // Parse backward
            end = positionalArguments.size();
            if (d->multiValueIndex >= 0 && d->multiValueIndex < cmdArgs.size() - 1) {
                size_t backwardStart = d->multiValueIndex + 1;
                size_t backwardCount = cmdArgs.size() - d->multiValueIndex - 1;
                if (positionalArguments.size() < backwardCount + k) {
                    missingArgument = &cmdArgs.at(d->multiValueIndex + 1);
                    goto out_parse_arguments;
                }
                end -= backwardCount;

                for (size_t j = 0; j < backwardCount; ++j) {
                    const auto &arg = cmdArgs.at(d->multiValueIndex + j + 1);
                    Value val;
                    if (!checkArgument(&arg, positionalArguments.at(end + j), &val)) {
                        goto out_parse_arguments;
                    }
                    result->argResult[arg.name()].push_back(val);
                }
            }

            if (end <= k) {
                goto out_parse_arguments;
            }

            // Too many
            if (d->multiValueIndex < 0) {
                const auto &token = positionalArguments.at(k);
                if (token.front() == '-') {
                    error = ParseResult::UnknownOption;
                    errorPlaceholders = {token};
                    goto out_parse_arguments;
                }

                if (cmdArgs.empty()) {
                    error = ParseResult::TooManyArguments;
                    goto out_parse_arguments;
                }

                error = ParseResult::UnknownCommand;
                errorPlaceholders = {token};
                goto out_parse_arguments;
            }

            // Consider multiple arguments
            const auto &arg = cmdArgs.at(d->multiValueIndex);
            auto &resVec = result->argResult[arg.name()];
            for (size_t j = k; j < end; ++j) {
                const auto &token = positionalArguments.at(j);
                Value val;
                if (!checkArgument(&arg, token, &val)) {
                    break;
                }
                resVec.push_back(val);
            }
        }

    out_parse_arguments:

        // Check required arguments
        if (error == ParseResult::NoError) {
            if ((cmd->d_func()->showHelpIfNoArg && result->optResult.empty() &&
                 result->argResult.empty()) ||
                (priorOpt && priorOpt->priorLevel() >= Option::IgnoreMissingSymbols)) {

                if (priorOpt) {
                    switch (priorOpt->priorLevel()) {
                        case Option::ExclusiveToArguments: {
                            if (!result->argResult.empty()) {
                                error = ParseResult::PriorOptionWithArguments;
                                errorPlaceholders = {priorOpt->displayedTokens()};
                            }
                            break;
                        }
                        case Option::ExclusiveToOptions: {
                            if (!result->optResult.empty()) {
                                error = ParseResult::PriorOptionWithOptions;
                                errorPlaceholders = {priorOpt->displayedTokens()};
                            }
                            break;
                        }
                        case Option::ExclusiveToAll: {
                            if (!result->argResult.empty()) {
                                error = ParseResult::PriorOptionWithArguments;
                                errorPlaceholders = {priorOpt->displayedTokens()};
                            } else if (!result->optResult.empty()) {
                                error = ParseResult::PriorOptionWithOptions;
                                errorPlaceholders = {priorOpt->displayedTokens()};
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }

            } else if (missingArgument) {
                // Required arguments
                error = ParseResult::MissingCommandArgument;
                errorPlaceholders = {missingArgument->name()};
            } else {
                // Required options
                const Option *missingOpt = nullptr;
                for (const auto &opt : cmd->d_func()->options) {
                    if (opt.isRequired() && !searchExclusiveOption(&opt) &&
                        !result->optResult.count(opt.name())) {
                        missingOpt = &opt;
                        break;
                    }
                }

                if (!missingOpt) {
                    for (const auto &opt : std::as_const(globalOptions)) {
                        if (opt->isRequired() && !result->optResult.count(opt->name())) {
                            missingOpt = opt;
                            break;
                        }
                    }
                }

                if (missingOpt) {
                    error = ParseResult::MissingRequiredOption;
                    errorPlaceholders = {missingOpt->displayedTokens()};
                }
            }
        }

        if (error != ParseResult::NoError) {
            result->argResult.clear();
            result->optResult.clear();
        } else {
            if (result->optResult.count("version")) {
                result->versionSet = true;
            }

            if (result->optResult.count("help")) {
                result->helpSet = true;
            }
        }
        return result;
    }

}
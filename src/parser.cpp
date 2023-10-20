#include "parser.h"
#include "parser_p.h"

#include <list>
#include <map>
#include <stdexcept>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <iomanip>

#include "strings.h"
#include "utils.h"
#include "system.h"
#include "command_p.h"
#include "option_p.h"

namespace SysCmdLine {

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
        auto suggestions = Utils::getClosestTexts(expectedValues, input, int(input.size()) / 2);
        if (suggestions.empty())
            return {};

        std::stringstream ss;
        ss << Utils::formatText(Strings::text(Strings::Information, Strings::MatchCommand), {input})
           << std::endl;
        for (const auto &item : std::as_const(suggestions)) {
            ss << std::setw(Strings::sizeConfig(Strings::Indent)) << " " << item << std::endl;
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
        std::vector<std::string> parentCommands;
        const Command *p = &parserData->rootCommand;
        for (const auto &item : std::as_const(stack)) {
            parentCommands.push_back(p->name());
            p = &p->d_func()->subCommands[item];
        }

        if (!parserData->texts[Parser::Top].empty()) {
            u8printf("%s\n\n", parserData->texts[Parser::Top].data());
        }

        if (messageCaller) {
            messageCaller();
            u8printf("\n");
        }

        u8printf("%s",
                 p->helpText(parentCommands, globalOptions, parserData->displayOptions).data());

        if (!parserData->texts[Parser::Bottom].empty()) {
            u8printf("\n%s\n", parserData->texts[Parser::Bottom].data());
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

    const std::vector<std::string> &ParseResult::arguments() const {
        return d_ptr->arguments;
    }

    int ParseResult::invoke(int errCode) const {
        if (d_ptr->error != NoError) {
            showError();
            return errCode;
        }
        return dispatch();
    }

    int ParseResult::dispatch() const {
        const auto &d = d_ptr.data();
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
        return d_ptr->error;
    }

    std::string ParseResult::errorText() const {
        if (d_ptr->error == NoError)
            return {};
        return Utils::formatText(Strings::text(Strings::ParseError, d_ptr->error),
                                 d_ptr->errorPlaceholders);
    }

    std::string ParseResult::correctionText() const {
        return d_ptr->correctionText();
    }

    Command ParseResult::targetCommand() const {
        return *d_ptr->command;
    }

    std::vector<Option> ParseResult::targetGlobalOptions() const {
        std::vector<Option> res;
        res.reserve(d_ptr->globalOptions.size());
        for (const auto &item : d_ptr->globalOptions) {
            res.push_back(*item);
        }
        return res;
    }

    std::vector<int> ParseResult::targetStack() const {
        return d_ptr->stack;
    }

    void ParseResult::showError() const {
        const auto &d = d_ptr.data();
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
        d_ptr->showHelp();
    }

    void ParseResult::showErrorAndHelpText(const std::string &message) const {
        d_ptr->showHelp([&message]() {
            u8error("%s\n", message.data()); //
        });
    }

    void ParseResult::showWarningAndHelpText(const std::string &message) const {
        d_ptr->showHelp([&message]() {
            u8warning("%s\n", message.data()); //
        });
    }

    Value ParseResult::valueForArgument(const std::string &argName) const {
        auto it = d_ptr->argResult.find(argName);
        if (it == d_ptr->argResult.end()) {
            return d_ptr->getDefaultResult(d_ptr->command, argName);
        }
        return it->second.front();
    }

    std::vector<Value> ParseResult::valuesForArgument(const std::string &argName) const {
        auto it = d_ptr->argResult.find(argName);
        if (it == d_ptr->argResult.end()) {
            return {};
        }
        return it->second;
    }

    int ParseResult::optionCount(const std::string &optName) const {
        const auto &map = d_ptr->optResult;
        auto it = map.find(optName);
        if (it == map.end()) {
            return 0;
        }
        return int(it->second.size());
    }

    Value ParseResult::valueForOption(const std::string &optName, const std::string &argName,
                                      int count) const {
        auto it = d_ptr->optResult.find(optName);
        if (it == d_ptr->optResult.end() || count >= it->second.size()) {
            return d_ptr->getDefaultResult(optName, argName);
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
        std::vector<std::string> res;
        res.reserve(d_ptr->optResult.size());
        for (const auto &item : d_ptr->optResult) {
            res.push_back(item.first);
        }
        return res;
    }

    std::vector<std::string> ParseResult::effectiveArguments() const {
        std::vector<std::string> res;
        res.reserve(d_ptr->argResult.size());
        for (const auto &item : d_ptr->argResult) {
            res.push_back(item.first);
        }
        return res;
    }

    bool ParseResult::isHelpSet() const {
        return d_ptr->helpSet;
    }

    bool ParseResult::isVersionSet() const {
        return d_ptr->versionSet;
    }

    bool ParseResult::isResultNull() const {
        return d_ptr->argResult.empty() && d_ptr->optResult.empty();
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

    Command Parser::rootCommand() const {
        return d_ptr->rootCommand;
    }

    void Parser::setRootCommand(const Command &rootCommand) {
        if (rootCommand.d_func()->name.empty()) {
            throw std::runtime_error("empty root command name");
        }
        d_ptr->rootCommand = rootCommand;
    }

    std::string Parser::text(Parser::Side side) const {
        return d_ptr->texts[side];
    }

    void Parser::setText(Parser::Side side, const std::string &text) {
        d_ptr->texts[side] = text;
    }

    int Parser::displayOptions() const {
        return d_ptr->displayOptions;
    }

    void Parser::setDisplayOptions(int displayOptions) {
        d_ptr->displayOptions = displayOptions;
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
                if (!(parseOptions & Parser::DontAllowUnixStyleOptions)) {
                    if (token.front() == '-') {
                        return searchShortOptions(indexes, token, '-', pos);
                    }
                }

                if (parseOptions & Parser::AllowDosStyleOptions) {
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
        Option::PriorLevel priorLevel = Option::NoPrior;
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

                priorLevel = std::max(priorLevel, opt->priorLevel());
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
            if ((parseOptions & Parser::ConsiderContinuousFlags) && isFlags(token)) {
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
                priorLevel >= Option::IgnoreMissingArgument) {
                // ...
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
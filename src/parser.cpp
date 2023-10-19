#include "parser.h"

#include <list>
#include <map>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <regex>

#include "strings.h"
#include "system.h"
#include "command_p.h"
#include "option_p.h"

namespace SysCmdLine {

    class ParseResult {
    public:
        Parser::Error error;
        std::vector<std::string> errorPlaceholders;
        std::vector<int> stack;
        std::vector<const Option *> globalOptions;

        const Command *command;

        using ArgResult = std::unordered_map<std::string, Value>;

        std::unordered_map<std::string, std::vector<Value>> argResult;
        std::unordered_map<std::string, std::vector<ArgResult>> optResult;

        bool versionSet;
        bool helpSet;

        ParseResult()
            : error(Parser::NoError), command(nullptr), versionSet(false), helpSet(false) {
        }

        std::string correctionText() const {
            std::vector<std::string> expectedValues;

            switch (error) {
                case Parser::UnknownOption:
                case Parser::InvalidOptionPosition: {
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

                    if (error == Parser::UnknownOption)
                        break;

                    // Fallback as invalid argument case
                }
                case Parser::InvalidArgumentValue: {
                    const auto &arg = command->argument(errorPlaceholders[1]);
                    for (const auto &item : arg.expectedValues()) {
                        expectedValues.push_back(item.toString());
                    }

                    if (!command->hasArgument(arg.name())) // option argument?
                        break;

                    // Fallback as unknown command case
                }
                case Parser::UnknownCommand: {
                    for (const auto &cmd : command->commands()) {
                        expectedValues.push_back(cmd.name());
                    }
                    break;
                }
                default:
                    return {};
            }

            auto input = errorPlaceholders[0];
            auto suggestions =
                Strings::getClosestTexts(expectedValues, input, std::max(2, int(input.size()) / 2));
            if (suggestions.empty())
                return {};

            std::stringstream ss;
            ss << Strings::formatText(Strings::helper_strings[Strings::MatchCommand], {input})
               << std::endl;
            for (const auto &item : std::as_const(suggestions)) {
                ss << Strings::INDENT << item << std::endl;
            }
            return ss.str();
        }
    };

    class ParserPrivate {
    public:
        Command rootCommand;
        std::string texts[2];
        ParseResult *result;
        int displayOptions;

        ParserPrivate() : result(nullptr), displayOptions(Parser::Normal) {
        }

        ~ParserPrivate() {
            delete result;
        }

        void parse(const std::vector<std::string> &args, int parserOptions) {
            delete result;
            result = new ParseResult();

            // Search command
            const Command *cmd = &rootCommand;
            std::list<const Option *> globalOptions;
            std::unordered_map<std::string, decltype(globalOptions)::iterator> globalOptionIndexes;
            std::unordered_map<std::string, decltype(globalOptions)::iterator>
                globalOptionTokenIndexes;

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
                        if (parserOptions & Parser::IgnoreCommandCase) {
                            // Search
                            bool found = false;
                            for (int j = 0; j < d->subCommands.size(); ++j) {
                                const auto &subCmd = d->subCommands.at(j);
                                if (Strings::toLower(subCmd.name()) == Strings::toLower(arg)) {
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
            if (parserOptions & Parser::IgnoreOptionCase) {
                for (const auto &item : std::as_const(globalOptions)) {
                    for (const auto &token : item->d_func()->tokens) {
                        lowerCaseOptionIndexes.insert(
                            std::make_pair(Strings::toLower(token), item));
                    }
                }
                for (const auto &item : std::as_const(cmd->d_func()->options)) {
                    for (const auto &token : item.d_func()->tokens) {
                        lowerCaseOptionIndexes.insert(
                            std::make_pair(Strings::toLower(token), &item));
                    }
                }
            }

            auto searchOptionImpl = [](const std::map<std::string, const Option *> &indexes,
                                       const std::string &token, int *pos) -> const Option * {
                if (pos)
                    *pos = -1;

                if (indexes.empty())
                    return nullptr;

                {
                    auto it = indexes.find(token);
                    if (it != indexes.end()) {
                        return it->second;
                    }
                }

                if (token.front() != '-') {
                    return nullptr;
                }

                // Search for short option
                auto it = indexes.lower_bound(token);
                if (it != indexes.begin() && it != indexes.end() && token.find(it->first) != 0) {
                    --it;
                }

                const auto &prefix = it->first;
                if (it != indexes.end() && Strings::starts_with(token, prefix)) {
                    const auto &opt = it->second;
                    const auto &args = opt->d_func()->arguments;
                    if (args.size() != 1 || !args.front().isRequired()) {
                        return nullptr;
                    }

                    if (opt->isShortOption()) {
                        if (pos)
                            *pos = prefix.size();
                        return opt;
                    }

                    if (token.at(prefix.size()) == '=') {
                        if (pos)
                            *pos = prefix.size() + 1;
                        return opt;
                    }
                }
                return nullptr;
            };

            auto searchOption = [&](const std::string &token,
                                    int *pos = nullptr) -> const Option * {
                auto opt = searchOptionImpl(allOptionIndexes, token, pos);
                if (opt)
                    return opt;
                if (parserOptions & Parser::IgnoreOptionCase) {
                    return searchOptionImpl(lowerCaseOptionIndexes, Strings::toLower(token), pos);
                }
                return nullptr;
            };

            auto searchShortFlagOption =
                [&](const std::string &flags) -> std::vector<const Option *> {
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
                            result->error = Parser::InvalidOptionPosition;
                            result->errorPlaceholders = {token, arg->name()};
                        } else {
                            result->error = Parser::InvalidArgumentValue;
                            result->errorPlaceholders = {token, arg->name()};
                        }
                    }
                    return false;
                }

                if (d->validator) {
                    std::string errorMessage;
                    if (d->validator(token, out, &errorMessage)) {
                        return true;
                    }
                    result->error = Parser::ArgumentValidateFailed;
                    result->errorPlaceholders = {token, arg->name(), errorMessage};
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
                result->error = Parser::ArgumentTypeMismatch;
                result->errorPlaceholders = {token, arg->name(), expected};
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
                [&](const Option *opt, const std::vector<ParseResult::ArgResult> &resVec) -> bool {
                // Check max occurrence
                if (opt->maxOccurrence() > 0 && resVec.size() == opt->maxOccurrence()) {
                    result->error = Parser::OptionOccurTooMuch;
                    result->errorPlaceholders = {
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
                        result->error = Parser::MutuallyExclusiveOptions;
                        result->errorPlaceholders = {
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
                    ParseResult::ArgResult curArgResult;

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
                        if (nextToken.front() == '-' && !arg.isRequired() &&
                            searchOption(nextToken)) {
                            break;
                        }

                        // Check argument
                        Value val;
                        if (!checkArgument(&arg, nextToken, &val, arg.isRequired())) {
                            break;
                        }
                        curArgResult.insert(std::make_pair(arg.name(), val));
                    }

                    if (result->error != Parser::NoError)
                        break;

                    // Check required arguments
                    if (x < optArgs.size()) {
                        const auto &arg = optArgs.at(x);
                        if (arg.isRequired()) {
                            result->error = Parser::MissingOptionArgument;
                            result->errorPlaceholders = {arg.name(), token};
                            break;
                        }
                    }

                    resVec.emplace_back(curArgResult);

                    priorLevel = std::max(priorLevel, opt->priorLevel());
                    j = start - 1 + x;
                    continue;
                }

                // Consider short flags
                if ((parserOptions & Parser::ConsiderShortFlags) &&
                    std::regex_match(token, std::regex("^-[a-zA-Z]+$"))) {
                    auto opts = searchShortFlagOption(token.substr(1));
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
                        result->error = Parser::UnknownOption;
                        result->errorPlaceholders = {token};
                        goto out_parse_arguments;
                    }

                    if (cmdArgs.empty()) {
                        result->error = Parser::TooManyArguments;
                        goto out_parse_arguments;
                    }

                    result->error = Parser::UnknownCommand;
                    result->errorPlaceholders = {token};
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
            if (result->error == Parser::NoError) {
                if (cmd->d_func()->showHelpIfNoArg &&
                    (result->optResult.empty() && result->argResult.empty())) {
                    // ...
                } else if (priorLevel >= Option::IgnoreMissingArgument) {
                    // ...
                } else if (missingArgument) {
                    // Required arguments
                    result->error = Parser::MissingCommandArgument;
                    result->errorPlaceholders = {missingArgument->name()};
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
                        result->error = Parser::MissingRequiredOption;
                        result->errorPlaceholders = {missingOpt->displayedTokens()};
                    }
                }
            }

            if (result->error != Parser::NoError) {
                result->argResult.clear();
                result->optResult.clear();
                return;
            }

            if (result->optResult.count("version")) {
                result->versionSet = true;
            }

            if (result->optResult.count("help")) {
                result->helpSet = true;
            }
        }

        inline void checkResult() const {
            if (!result)
                throw std::runtime_error("no valid parse result");
        }

        Value getDefaultResult(const ArgumentHolder *argumentHolder,
                               const std::string &argName) const {
            const auto &d2 = argumentHolder->d_func();
            auto it2 = d2->argumentNameIndexes.find(argName);
            if (it2 == d2->argumentNameIndexes.end())
                return {};
            return d2->arguments.at(it2->second).defaultValue();
        }

        Value getDefaultResult(const std::string &optName, const std::string &argName) const {
            const auto &d = result->command->d_func();
            auto it = d->optionNameIndexes.find(optName);
            if (it == d->optionNameIndexes.end())
                return {};
            return getDefaultResult(&d->options.at(it->second), argName);
        }

        void showHelp(const std::function<void()> &messageCaller = {}) {
            std::vector<std::string> parentCommands;
            const Command *p = &rootCommand;
            for (const auto &item : std::as_const(result->stack)) {
                parentCommands.push_back(p->name());
                p = &p->d_func()->subCommands[item];
            }

            if (!texts[Parser::Top].empty()) {
                u8printf("%s\n\n", texts[Parser::Top].data());
            }

            if (messageCaller) {
                messageCaller();
                u8printf("\n");
            }

            u8printf("%s",
                     p->helpText(parentCommands, result->globalOptions, displayOptions).data());

            if (!texts[Parser::Bottom].empty()) {
                u8printf("\n%s\n", texts[Parser::Bottom].data());
            }
        }
    };

    Parser::Parser() : d(new ParserPrivate()) {
    }

    Parser::Parser(const Command &rootCommand) : d(new ParserPrivate()) {
        setRootCommand(rootCommand);
    }

    Parser::~Parser() {
        delete d;
    }

    Command Parser::rootCommand() const {
        return d->rootCommand;
    }

    void Parser::setRootCommand(const Command &rootCommand) {
        if (rootCommand.d_func()->name.empty()) {
            throw std::runtime_error("empty root command name");
        }

        if (d->result) {
            delete d->result;
            d->result = nullptr;
        }

        d->rootCommand = rootCommand;
    }

    std::string Parser::text(Parser::Side side) const {
        return d->texts[side];
    }

    void Parser::setText(Parser::Side side, const std::string &text) {
        d->texts[side] = text;
    }

    int Parser::displayOptions() const {
        return d->displayOptions;
    }

    void Parser::setDisplayOptions(int options) {
        d->displayOptions = options;
    }

    bool Parser::parse(const std::vector<std::string> &args, int options) {
        d->parse(args, options);
        return d->result->error == NoError;
    }

    int Parser::invoke() const {
        if (!d->result || d->result->error != NoError) {
            throw std::runtime_error("cannot invoke handler when parser failed");
        }

        const auto &cmd = *d->result->command;
        const auto &handler = cmd.handler();

        if (d->result->versionSet) {
            u8printf("%s\n", cmd.version().data());
            return 0;
        }

        if (d->result->helpSet) {
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

    int Parser::invoke(const std::vector<std::string> &args, int errCode, int options) {
        if (!parse(args, options)) {
            showError();
            return errCode;
        }
        return invoke();
    }

    bool Parser::parsed() const {
        return d->result;
    }

    Parser::Error Parser::error() const {
        return d->result ? d->result->error : NoError;
    }

    std::string Parser::errorText() const {
        if (!d->result)
            return {};

        if (d->result->error == NoError)
            return {};

        return Strings::formatText(Strings::error_strings[d->result->error],
                                   d->result->errorPlaceholders);
    }

    std::string Parser::correctionText() const {
        if (!d->result)
            return {};
        return d->result->correctionText();
    }

    Command Parser::targetCommand() const {
        d->checkResult();
        return *d->result->command;
    }

    std::vector<Option> Parser::targetGlobalOptions() const {
        d->checkResult();

        std::vector<Option> res;
        res.reserve(d->result->globalOptions.size());
        for (const auto &item : d->result->globalOptions) {
            res.push_back(*item);
        }
        return res;
    }

    std::vector<int> Parser::targetStack() const {
        d->checkResult();
        return d->result->stack;
    }

    Value Parser::valueForArgument(const std::string &argName) const {
        d->checkResult();
        auto it = d->result->argResult.find(argName);
        if (it == d->result->argResult.end()) {
            return d->getDefaultResult(d->result->command, argName);
        }
        return it->second.front();
    }

    std::vector<Value> Parser::valuesForArgument(const std::string &argName) const {
        d->checkResult();
        auto it = d->result->argResult.find(argName);
        if (it == d->result->argResult.end()) {
            return {};
        }
        return it->second;
    }

    int Parser::optionCount(const std::string &optName) const {
        d->checkResult();

        const auto &map = d->result->optResult;
        auto it = map.find(optName);
        if (it == map.end()) {
            return 0;
        }
        return it->second.size();
    }

    Value Parser::valueForOption(const std::string &optName, const std::string &argName,
                                 int count) const {
        d->checkResult();

        auto it = d->result->optResult.find(optName);
        if (it == d->result->optResult.end() || count >= it->second.size()) {
            return d->getDefaultResult(optName, argName);
        }

        const auto &map = it->second.at(count);
        auto it2 = map.find(argName);
        if (it2 == map.end()) {
            return {};
        }
        return it2->second;
    }

    Value Parser::valueForOption(const std::string &optName, int argIndex, int count) const {
        d->checkResult();

        const Option &opt = d->result->command->option(optName);
        const auto &args = opt.arguments();
        if (argIndex >= args.size())
            return {};

        auto it = d->result->optResult.find(optName);
        if (it == d->result->optResult.end() || count >= it->second.size()) {
            return d->getDefaultResult(optName, args[argIndex].name());
        }

        const auto &map = it->second.at(count);
        auto it2 = map.find(args[argIndex].name());
        if (it2 == map.end()) {
            return {};
        }
        return it2->second;
    }

    std::vector<std::string> Parser::effectiveOptions() const {
        d->checkResult();

        std::vector<std::string> res;
        res.reserve(d->result->optResult.size());
        for (const auto &item : d->result->optResult) {
            res.push_back(item.first);
        }
        return res;
    }

    std::vector<std::string> Parser::effectiveArguments() const {
        d->checkResult();

        std::vector<std::string> res;
        res.reserve(d->result->argResult.size());
        for (const auto &item : d->result->argResult) {
            res.push_back(item.first);
        }
        return res;
    }

    bool Parser::isHelpSet() const {
        return d->result && d->result->helpSet;
    }

    bool Parser::isVersionSet() const {
        return d->result && d->result->versionSet;
    }

    bool Parser::isResultNull() const {
        d->checkResult();
        return d->result->argResult.empty() && d->result->optResult.empty();
    }

    void Parser::showError() const {
        if (!d->result)
            return;

        if (d->result->error == NoError)
            return;

        auto errCallback = [this]() {
            if (!(d->displayOptions & SkipCorrection)) {
                if (auto correction = d->result->correctionText(); !correction.empty()) {
                    u8printf("%s", correction.data());
                }
            }
            u8error("%s: %s\n", Strings::common_strings[Strings::Error], errorText().data());
        };

        if (!(d->displayOptions & DontShowHelpOnError) &&
            d->result->command->d_func()->optionNameIndexes.count("help")) {
            d->showHelp(errCallback);
        } else {
            errCallback();
        }
    }

    void Parser::showHelpText() const {
        if (!d->result) {
            return;
        }
        d->showHelp();
    }

    void Parser::showErrorAndHelpText(const std::string &message) const {
        if (!d->result) {
            return;
        }
        d->showHelp([&message]() {
            u8error("%s\n", message.data()); //
        });
    }

    void Parser::showWarningAndHelpText(const std::string &message) const {
        if (!d->result) {
            return;
        }
        d->showHelp([&message]() {
            u8warning("%s\n", message.data()); //
        });
    }

}
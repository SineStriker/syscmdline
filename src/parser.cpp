#include "parser.h"

#include <list>
#include <map>
#include <stdexcept>
#include <sstream>

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

        using ArgResult = std::unordered_map<std::string, std::string>;

        ArgResult argResult;
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
                        expectedValues.push_back(item);
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
        bool showHelpOnError;

        ParserPrivate() : result(nullptr), showHelpOnError(true) {
        }

        ~ParserPrivate() {
            delete result;
        }

        void parse(const std::vector<std::string> &args) {
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
                    auto it = cmd->d_func()->subCommandNameIndexes.find(args[i]);
                    if (it == cmd->d_func()->subCommandNameIndexes.end()) {
                        break;
                    }
                    result->stack.push_back(int(it->second));
                    cmd = &cmd->d_func()->subCommands.at(it->second);
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

            auto searchOption = [&](const std::string &token) -> const Option * {
                {
                    auto it = allOptionIndexes.find(token);
                    if (it != allOptionIndexes.end()) {
                        return it->second;
                    }
                }

                if (token.front() != '-') {
                    return nullptr;
                }

                // Search for short option
                auto it = allOptionIndexes.lower_bound(token);
                if (it != allOptionIndexes.begin() && it != allOptionIndexes.end() &&
                    token.find(it->first) != 0) {
                    --it;
                }
                if (it != allOptionIndexes.end() && token.find(it->first) == 0) {
                    const auto &opt = it->second;
                    if (opt->isShortOption())
                        return opt;
                }
                return nullptr;
            };

            auto checkArgument = [&](const Argument *arg, const std::string &value,
                                     bool setError = true) {
                const auto &expectedValues = arg->d_func()->expectedValues;
                if (expectedValues.empty()) {
                    return true;
                }
                if (std::find(expectedValues.begin(), expectedValues.end(), value) ==
                    expectedValues.end()) {
                    if (setError) {
                        if (value.front() == '-') {
                            result->error = Parser::InvalidOptionPosition;
                            result->errorPlaceholders = {value, arg->name()};
                        } else {
                            result->error = Parser::InvalidArgumentValue;
                            result->errorPlaceholders = {value, arg->name()};
                        }
                    }
                    return false;
                }
                return true;
            };

            // Parse options
            size_t k = 0;
            Option::PriorLevel priorLevel = Option::NoPrior;
            for (auto j = i; j < args.size(); ++j) {
                const auto &token = args[j];

                // Consider option
                if (auto opt = searchOption(token); opt) {
                    size_t x = 0;
                    size_t max = std::min(args.size() - j - 1, opt->d_func()->arguments.size());

                    auto &resVec = result->optResult[opt->name()];

                    // Check max occurrence
                    if (opt->maxOccurrence() > 0 && resVec.size() == opt->maxOccurrence() &&
                        priorLevel < Option::IgnoreRedundantArgument) {
                        result->error = Parser::OptionOccurTooMuch;
                        result->errorPlaceholders = {
                            opt->displayedTokens(),
                            std::to_string(opt->maxOccurrence()),
                        };
                        break;
                    }

                    ParseResult::ArgResult curArgResult;
                    for (; x < max; ++x) {
                        const auto &nextToken = args[x + j + 1];
                        const auto &arg = opt->d_func()->arguments.at(x);

                        // Break by next option
                        if (nextToken.front() == '-' && !arg.isRequired() &&
                            searchOption(nextToken)) {
                            break;
                        }

                        // Check argument
                        if (!checkArgument(&arg, nextToken, arg.isRequired())) {
                            break;
                        }
                        curArgResult.insert(std::make_pair(arg.name(), nextToken));
                    }

                    if (result->error != Parser::NoError)
                        break;

                    // Check required arguments
                    if (x < opt->d_func()->arguments.size()) {
                        const auto &arg = opt->d_func()->arguments.at(x);
                        if (arg.isRequired()) {
                            result->error = Parser::MissingOptionArgument;
                            result->errorPlaceholders = {arg.name(), token};
                            break;
                        }
                    }

                    resVec.emplace_back(curArgResult);

                    priorLevel = std::max(priorLevel, opt->priorLevel());
                    j += x;
                    continue;
                }

                // Consider argument
                {
                    if (k == cmd->d_func()->arguments.size()) {
                        if (priorLevel >= Option::IgnoreRedundantArgument) {
                            break;
                        }

                        if (token.front() == '-') {
                            result->error = Parser::UnknownOption;
                            result->errorPlaceholders = {token};
                            break;
                        }

                        if (cmd->d_func()->arguments.empty()) {
                            result->error = Parser::TooManyArguments;
                            break;
                        }

                        result->error = Parser::UnknownCommand;
                        result->errorPlaceholders = {token};
                        break;
                    }

                    const auto &arg = cmd->d_func()->arguments.at(k);
                    if (!checkArgument(&arg, token)) {
                        break;
                    }
                    result->argResult.insert(std::make_pair(arg.name(), token));
                    k++;
                }
            }

            // Check required arguments
            if (result->error == Parser::NoError) {
                if (cmd->d_func()->showHelpIfNoArg &&
                    (result->optResult.empty() && result->argResult.empty())) {
                    // ...
                } else if (priorLevel >= Option::IgnoreMissingArgument) {
                    // ...
                } else {
                    // Required arguments
                    if (k < cmd->d_func()->arguments.size()) {
                        const auto &arg = cmd->d_func()->arguments.at(k);
                        if (arg.isRequired()) {
                            result->error = Parser::MissingCommandArgument;
                            result->errorPlaceholders = {arg.name()};
                        }
                    }

                    // Required options
                    const Option *missingOpt = nullptr;
                    for (const auto &opt : cmd->d_func()->options) {
                        if (opt.isRequired() && !result->optResult.count(opt.name())) {
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

        void showHelp(const std::function<void()> &messageCaller = {}) {
            if (!result) {
                return;
            }

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

            u8printf("%s", p->helpText(parentCommands, result->globalOptions).data());

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

    bool Parser::showHelpOnError() const {
        return d->showHelpOnError;
    }

    void Parser::setShowHelpOnError(bool on) {
        d->showHelpOnError = on;
    }

    bool Parser::parse(const std::vector<std::string> &args) {
        d->parse(args);
        return d->result->error == NoError;
    }

    int Parser::invoke(const std::vector<std::string> &args, int errorCode) {
        if (!parse(args)) {
            showError();
            return errorCode;
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

    std::string Parser::valueForArgument(const std::string &argName) const {
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

    std::string Parser::valueForOption(const std::string &optName, const std::string &argName,
                                       int count) const {
        d->checkResult();

        auto it = d->result->optResult.find(optName);
        if (it == d->result->optResult.end() || count >= it->second.size()) {
            return {};
        }

        const auto &map = it->second.at(count);
        auto it2 = map.find(argName);
        if (it2 == map.end()) {
            return {};
        }
        return it2->second;
    }

    std::string Parser::valueForOption(const std::string &optName, int argIndex, int count) const {
        d->checkResult();

        const Option &opt = d->result->command->option(optName);
        const auto &args = opt.arguments();
        if (argIndex >= args.size())
            return {};

        auto it = d->result->optResult.find(optName);
        if (it == d->result->optResult.end() || count >= it->second.size()) {
            return {};
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
        auto errCallback = [this]() {
            if (auto correction = d->result->correctionText(); !correction.empty()) {
                u8printf("%s", correction.data());
            }
            u8error("%s: %s\n", Strings::common_strings[Strings::Error], errorText().data());
        };

        d->checkResult();
        if (d->showHelpOnError && d->result->command->d_func()->optionNameIndexes.count("help")) {
            d->showHelp(errCallback);
        } else {
            errCallback();
        }
    }

    void Parser::showHelpText() const {
        d->showHelp();
    }

    void Parser::showErrorAndHelpText(const std::string &message) const {
        d->showHelp([&message]() {
            u8error("%s\n", message.data()); //
        });
    }

    void Parser::showWarningAndHelpText(const std::string &message) const {
        d->showHelp([&message]() {
            u8warning("%s\n", message.data()); //
        });
    }

}
#include "parser.h"

#include <list>
#include <map>
#include <stdexcept>

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
    };

    class ParserPrivate {
    public:
        Command rootCommand;
        std::string texts[2];
        ParseResult *result;

        ParserPrivate() : result(nullptr) {
        }

        ~ParserPrivate() {
            delete result;
        }

        void parse(const std::vector<std::string> &args) {
            delete result;
            result = new ParseResult();


            // Search command
            const Command *cmd = &rootCommand;
            const Command *lastCmd = nullptr;
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

            size_t i = 1;
            for (; i < args.size(); ++i) {
                {
                    auto it = cmd->d_func()->subCommandNameIndexes.find(args[i]);
                    if (it == cmd->d_func()->subCommandNameIndexes.end()) {
                        break;
                    }
                    result->stack.push_back(int(it->second));
                    cmd = &cmd->d_func()->subCommands.at(it->second);
                }

                // Collect global options
                if (lastCmd) {
                    for (const auto &opt : cmd->d_func()->options) {
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
                lastCmd = cmd;
            }
            result->command = cmd;

            // Remove duplicated global options
            if (!globalOptionIndexes.empty()) {
                for (const auto &opt : cmd->d_func()->options) {
                    removeDuplicatedOptions(opt);
                }
            }

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
                        result->error = Parser::InvalidArgumentValue;
                        result->errorPlaceholders = {value, arg->name()};
                    }
                    return false;
                }
                return true;
            };

            // Parse options
            size_t k = 0;
            bool hasPrior = false;
            for (auto j = i; j < args.size(); ++j) {
                const auto &token = args[j];

                // Consider option
                if (auto opt = searchOption(token); opt) {
                    size_t x = 0;
                    size_t max = std::min(args.size() - j, opt->d_func()->arguments.size());

                    ParseResult::ArgResult curArgResult;
                    for (; x < max; ++x) {
                        const auto &nextToken = args[x + j];
                        const auto &arg = opt->d_func()->arguments.at(x);

                        // Break by next option
                        if (nextToken.front() == '-' && !arg.isRequired() &&
                            searchOption(nextToken)) {
                            break;
                        }

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

                    result->optResult[opt->name()].emplace_back(curArgResult);

                    hasPrior |= opt->isPrior();
                    j += x;
                    continue;
                }

                // Consider argument
                {
                    if (k == cmd->d_func()->arguments.size()) {
                        if (token.front() == '-') {
                            result->error = Parser::UnknownOption;
                            result->errorPlaceholders = {token};
                            break;
                        }

                        if (cmd->d_func()->arguments.empty()) {
                            result->error = Parser::TooManyArguments;
                            break;
                        }

                        result->error = Parser::UnknownArgument;
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
                } else if (hasPrior) {
                    // ...
                } else {
                    if (k < cmd->d_func()->arguments.size()) {
                        const auto &arg = cmd->d_func()->arguments.at(k);
                        if (arg.isRequired()) {
                            result->error = Parser::MissingCommandArgument;
                            result->errorPlaceholders = {arg.name()};
                        }
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

        static std::string formatText(const std::string &format,
                                      const std::vector<std::string> &args) {
            std::string result = format;
            for (size_t i = 0; i < args.size(); i++) {
                std::string placeholder = "%" + std::to_string(i + 1);
                size_t pos = result.find(placeholder);
                while (pos != std::string::npos) {
                    result.replace(pos, placeholder.length(), args[i]);
                    pos = result.find(placeholder, pos + args[i].size());
                }
            }
            return result;
        }

        void showHelp(const std::function<void()> &messageCaller = {}) {
            if (!result) {
                return;
            }

            std::vector<std::string> parentCommands;
            const Command *p = &rootCommand;
            for (const auto &item : std::as_const(result->stack)) {
                parentCommands.push_back(p->name());
                p = p->command(item);
            }

            if (!texts[Parser::Top].empty()) {
                u8printf("%s\n\n", texts[Parser::Top].data());
            }

            if (messageCaller) {
                messageCaller();
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

    const Command &Parser::rootCommand() const {
        return d->rootCommand;
    }

    void Parser::setRootCommand(const Command &rootCommand) {
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

    bool Parser::parse(const std::vector<std::string> &args) {
        d->parse(args);
        return d->result->error == NoError;
    }

    int Parser::invoke(const std::vector<std::string> &args, int errorCode) {
        if (!parse(args)) {
            d->showHelp([this]() {
                u8error("%s: %s\n\n", Strings::common_strings[Strings::Error], errorText().data());
            });
            return errorCode;
        }
        return invoke();
    }

    int Parser::invoke() const {
        d->checkResult();

        const auto &cmd = *targetCommand();
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
        return ParserPrivate::formatText(Strings::error_strings[d->result->error],
                                         d->result->errorPlaceholders);
    }

    const Command *Parser::targetCommand() const {
        d->checkResult();
        return d->result->command;
    }

    const std::vector<const Option *> &Parser::targetGlobalOptions() const {
        d->checkResult();
        return d->result->globalOptions;
    }

    std::vector<std::pair<int, std::string>> Parser::targetStack() const {
        d->checkResult();

        std::vector<std::pair<int, std::string>> res;
        res.reserve(d->result->stack.size());

        const Command *p = &d->rootCommand;
        for (const auto &item : std::as_const(d->result->stack)) {
            p = p->command(item);
            res.emplace_back(item, p->name());
        }
        return res;
    }

    std::string Parser::value(const std::string &argName) const {
        d->checkResult();

        auto it = d->result->argResult.find(argName);
        if (it == d->result->argResult.end()) {
            return {};
        }
        return it->second;
    }

    int Parser::count(const std::string &optName) const {
        d->checkResult();

        const auto &map = d->result->optResult;
        auto it = map.find(optName);
        if (it == map.end()) {
            return 0;
        }
        return it->second.size();
    }

    std::string Parser::value(const std::string &optName, const std::string &argName, int count) {
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

    void Parser::showHelpText() const {
        d->showHelp();
    }

    void Parser::showErrorAndHelpText(const std::string &message) const {
        d->showHelp([&message]() {
            u8error("%s\n\n", message.data()); //
        });
    }

    void Parser::showWarningAndHelpText(const std::string &message) const {
        d->showHelp([&message]() {
            u8warning("%s\n\n", message.data()); //
        });
    }
}
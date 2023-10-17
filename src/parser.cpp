#include "parser.h"

#include <list>
#include <map>
#include <stdexcept>

#include "strings.h"
#include "system.h"

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
            size_t i = 1;
            for (; i < args.size(); ++i) {
                {
                    auto it = cmd->_subCommandNameIndexes.find(args[i]);
                    if (it == cmd->_subCommandNameIndexes.end()) {
                        break;
                    }
                    result->stack.push_back(int(it->second));
                    cmd = &cmd->_subCommands.at(it->second);
                }

                // Collect global options
                if (lastCmd) {
                    for (const auto &opt : cmd->_options) {
                        if (!opt.isGlobal())
                            continue;

                        auto it = globalOptionIndexes.find(opt.name());
                        if (it != globalOptionIndexes.end()) {
                            globalOptions.erase(it->second);
                            globalOptionIndexes.erase(it);
                        }
                        globalOptionIndexes.insert(std::make_pair(
                            opt.name(), globalOptions.insert(globalOptions.end(), &opt)));
                    }
                }
                lastCmd = cmd;
            }
            result->command = cmd;

            // Remove duplicated global options
            if (!globalOptionIndexes.empty()) {
                for (const auto &opt : cmd->_options) {
                    if (!opt.isGlobal())
                        continue;

                    auto it = globalOptionIndexes.find(opt.name());
                    if (it != globalOptionIndexes.end()) {
                        globalOptions.erase(it->second);
                        globalOptionIndexes.erase(it);
                    }
                }
            }

            // Build option indexes
            std::map<std::string, const Option *> allOptionIndexes;
            for (const auto &item : std::as_const(globalOptions)) {
                for (const auto &token : item->_tokens) {
                    allOptionIndexes.insert(std::make_pair(token, item));
                }
            }
            for (const auto &item : std::as_const(cmd->_options)) {
                for (const auto &token : item._tokens) {
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

            // Parse options
            size_t k = 0;
            for (auto j = i; j < args.size(); ++j) {
                const auto &token = args[j];

                // Consider option
                if (auto opt = searchOption(token); opt) {
                    size_t x = 0;
                    size_t max = std::min(args.size() - j, opt->_arguments.size());

                    ParseResult::ArgResult curArgResult;
                    for (; x < max; ++x) {
                        const auto &nextToken = args[x + j];
                        const auto &arg = opt->_arguments.at(x);

                        // Break by next option
                        if (nextToken.front() == '-' && !arg.isRequired() &&
                            searchOption(nextToken)) {
                            break;
                        }
                        curArgResult.insert(std::make_pair(arg.name(), nextToken));
                    }

                    if (x < opt->_arguments.size()) {
                        const auto &arg = opt->_arguments.at(x);
                        if (arg.isRequired()) {
                            result->error = Parser::MissingRequiredArgument;
                            result->errorPlaceholders = {arg.name()};
                            break;
                        }
                    }

                    result->optResult[opt->name()].emplace_back(curArgResult);

                    j += x;
                    continue;
                }

                // Consider argument
                {
                    if (k == cmd->_arguments.size()) {
                        if (token.front() == '-') {
                            result->error = Parser::UnknownOption;
                            result->errorPlaceholders = {token};
                            break;
                        }
                        result->error = Parser::UnknownArgument;
                        result->errorPlaceholders = {token};
                        break;
                    }

                    const auto &arg = cmd->_arguments.at(k);
                    result->argResult.insert(std::make_pair(arg.name(), token));
                    k++;
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

        static std::string formatText(const std::string &fmt,
                                      const std::vector<std::string> &placeHolders) {
            std::string res;
            std::string::size_type start = 0;
            for (const auto &item : placeHolders) {
                std::string::size_type idx = fmt.find("{}");
                if (idx == std::string::npos) {
                    break;
                }
                res.append(fmt.substr(start, idx - start));
                start = idx + 2;
            }

            if (start < fmt.size()) {
                res.append(fmt.substr(start));
            }
            return res;
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
            u8errprint("%s: %s\n", Strings::common_strings[Strings::Error], errorText().data());
            return errorCode;
        }
        return invoke();
    }

    int Parser::invoke() const {
        d->checkResult();

        const auto &cmd = *targetCommand();
        const auto &handler = cmd.handler();

        if (d->result->versionSet) {
            u8printf("%s\n", cmd._version.data());
            return 0;
        }

        if (d->result->helpSet) {
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
            p = &p->command(item);
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
        return std::string();
    }

    int Parser::count(const std::string &optName) const {
        return 0;
    }

    std::string Parser::value(const std::string &optName, const std::string &argName, int count) {
        return std::string();
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

    bool Parser::isResultNull() const {
        d->checkResult();
        return d->result->argResult.empty() && d->result->optResult.empty();
    }

    bool Parser::isHelpSet() const {
        return d->result && d->result->helpSet;
    }

    bool Parser::isVersionSet() const {
        return d->result && d->result->versionSet;
    }

    void Parser::showHelpText() const {
        if (!d->result) {
            return;
        }

        std::vector<std::string> parentCommands;
        const Command *p = &d->rootCommand;
        for (const auto &item : std::as_const(d->result->stack)) {
            parentCommands.push_back(p->name());
            p = &p->command(item);
        }

        if (!d->texts[Top].empty()) {
            u8printf("%s\n\n", d->texts[Top].data());
        }

        u8printf("%s", p->helpText(parentCommands, d->result->globalOptions).data());

        if (!d->texts[Bottom].empty()) {
            u8printf("\n%s\n", d->texts[Bottom].data());
        }
    }

}
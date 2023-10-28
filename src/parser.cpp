#include "parser.h"
#include "parser_p.h"

#include <list>
#include <map>
#include <set>
#include <stdexcept>
#include <cctype>
#include <algorithm>

#include "strings.h"
#include "utils.h"
#include "system.h"
#include "command_p.h"
#include "option_p.h"
#include "helplayout_p.h"
#include "parseresult_p.h"

namespace SysCmdLine {

    namespace Strings::en_US {

        static const char *error_strings[] = {
            R"(No error.)",
            R"(Unknown option "%1".)",
            R"(Unknown command or argument "%1".)",
            R"(Missing required argument "%1" of option "%2".)",
            R"(Missing required argument "%1".)",
            R"(Too many arguments.)",
            R"(Invalid value "%1" of argument "%2".)",
            R"(Invalid occurrence of option "%1", which should be argument "%2".)",
            R"(Missing required option "%1".)",
            R"(Option "%1" occurs too much, at most %2.)",
            R"(Invalid token "%1" of argument "%2", expect "%3".)",
            R"(Invalid token "%1" of argument "%2", reason: %3)",
            R"(Options "%1" and "%2" are mutually exclusive.)",
            R"(Option "%1" and other arguments cannot be specified simultaneously.)",
            R"(Option "%1" and other options cannot be specified simultaneously.)",
        };

        static const char *title_strings[] = {
            "Error",    "Usage",    "Description", "Arguments", "Options",
            "Commands", "Required", "Default",     "Expected",
        };

        static const char *command_strings[] = {
            "Show version information",
            "Show help information",
        };

        static const char *info_strings[] = {
            R"("%1" is not matched. Do you mean one of the following?)",
        };

        static const char *token_strings[] = {
            "commands",
            "options",
        };

        static const char **strings[] = {
            error_strings, title_strings, command_strings, info_strings, token_strings,
        };

        static std::string provider(int category, int index) {
            return strings[category][index];
        }

    }

    ParserPrivate::ParserPrivate()
        : displayOptions(Parser::Normal), helpLayout(HelpLayout::defaultHelpLayout()),
          textProvider(Strings::en_US::provider) {
    }

    Parser::Parser() : SharedBase(new ParserPrivate()) {
    }

    Parser::Parser(const Command &rootCommand) : Parser() {
        setRootCommand(rootCommand);
    }

    Parser::~Parser() {
    }

    int Parser::displayOptions() const {
        Q_D2(Parser);
        return d->displayOptions;
    }

    void Parser::setDisplayOptions(int displayOptions) {
        Q_D(Parser);
        d->displayOptions = displayOptions;
    }

    HelpLayout Parser::helpLayout() const {
        Q_D2(Parser);
        return d->helpLayout;
    }

    void Parser::setHelpLayout(const HelpLayout &helpLayout) {
        Q_D(Parser);
        d->helpLayout = helpLayout;
    }

    Command Parser::rootCommand() const {
        Q_D2(Parser);
        return d->rootCommand;
    }

    void Parser::setRootCommand(const Command &rootCommand) {
        Q_D(Parser);
        if (rootCommand.d_func()->name.empty()) {
            throw std::runtime_error("empty root command name");
        }
        d->rootCommand = rootCommand;
    }

    class ParserCore {
    public:
        ParserCore(const std::vector<std::string> &params, int parseOptions, int displayOptions,
                   const Command *rootCommand)
            : params(params), parseOptions(parseOptions), displayOptions(displayOptions),
              rootCommand(rootCommand), result(new ParseResultPrivate()), core(result->core),
              nonCommandIndex(1) {
        }

        ~ParserCore() {
        }

        void searchTargetCommandAndBuildIndexes() {
            // 1. Find target command
            size_t globalOptionCount = 0;
            {
                auto cmd = rootCommand;
                size_t i = 1;
                for (; i < params.size(); ++i) {
                    const auto &dd = cmd->d_func();
                    const auto &param = params[i];

                    size_t j = 0;
                    size_t size = dd->commands.size();
                    for (; j < size; ++j) {
                        if (dd->commands[j].name() == param) {
                            j = i;
                            break;
                        }
                    }

                    if (j == size) {
                        // Try case insensitive
                        if (parseOptions & Parser::IgnoreCommandCase) {
                            for (j = 0; j < size; ++j) {
                                if (Utils::toLower(dd->commands[j].name()) ==
                                    Utils::toLower(param)) {
                                    j = i;
                                    break;
                                }
                            }

                            // Not found
                            if (j == size) {
                                break;
                            }
                        } else {
                            break;
                        }
                    }

                    result->stack.push_back(j);
                    globalOptionCount += cmd->optionCount();
                    cmd = &cmd->d_func()->commands.at(j);
                }
                nonCommandIndex = i;
                targetCommand = cmd;
                targetCommandData = cmd->d_func();
            }

            // 2. Collect options along the command path
            const Option **globalOptionList = new const Option *[globalOptionCount]; // Alloc
            {
                int globalOptionListIndex = 0;
                auto cmd = rootCommand;
                for (size_t i = 0; i < result->stack.size(); ++i) {
                    const auto &d = cmd->d_func();

                    // Add options
                    for (size_t j = 0; j < d->options.size(); ++j) {
                        globalOptionList[globalOptionListIndex++] = &d->options[j];
                    }
                    cmd = &d->commands[i];
                }
            }

            // 3. Remove duplicated options from end to begin
            int realGlobalOptionCount = globalOptionCount;
            {
                StringMap visitedTokens;
                for (int i = globalOptionCount - 1; i >= 0; --i) {
                    auto &opt = globalOptionList[i];
                    const auto &dd = opt->d_func();

                    bool visited = false;
                    for (const auto &token : dd->tokens) {
                        if (visitedTokens.count(token)) {
                            visited = true;
                        }
                    }

                    if (visited) {
                        realGlobalOptionCount--;
                        opt = nullptr; // mark invalid
                        continue;
                    }

                    for (const auto &token : dd->tokens) {
                        visitedTokens.insert(std::make_pair(token, 0));
                    }
                }
            }

            // 4. Alloc option spaces
            {
                core.allOptionsSize = realGlobalOptionCount + targetCommandData->options.size();
                core.globalOptionsSize = realGlobalOptionCount;
                core.allOptionsResult = new ParseResultData2::OptionData[core.allOptionsSize];

                // init options
                int allOptionsIndex = 0;
                for (int i = 0; i < globalOptionCount; ++i) {
                    auto opt = globalOptionList[i];
                    if (!opt)
                        continue;
                    initOptionData(core.allOptionsResult[allOptionsIndex++], opt);
                }

                for (size_t i = 0; i < targetCommandData->options.size(); ++i) {
                    initOptionData(core.allOptionsResult[allOptionsIndex++],
                                   &targetCommandData->options[i]);
                }
            }

            delete[] globalOptionList; // Free

            // 5. Alloc command argument space
            core.argResult = new std::vector<Value>[targetCommandData->arguments.size()];
            core.optionalArgIndex = -1;
            core.multiValueArgIndex = -1;

            // Build arg name indexes
            for (size_t i = 0; i < targetCommandData->arguments.size(); ++i) {
                const auto &arg = targetCommandData->arguments[i];
                if (arg.isOptional() && core.optionalArgIndex < 0) {
                    core.optionalArgIndex = i;
                }
                if (arg.multiValueEnabled() && core.multiValueArgIndex < 0) {
                    core.multiValueArgIndex = i;
                }
                core.argNameIndexes.insert(std::make_pair(arg.name(), i));
            }

            // 6. Build option indexes
            for (size_t i = 0; i < core.allOptionsSize; ++i) {
                const auto &opt = core.allOptionsResult[i].option;
                for (const auto &token : opt->d_func()->tokens) {
                    allOptionTokenIndexes.insert(std::make_pair(token, i));
                }
            }

            // Build case-insensitive option indexes if needed
            if (parseOptions & Parser::IgnoreOptionCase) {
                for (size_t i = 0; i < core.allOptionsSize; ++i) {
                    const auto &opt = core.allOptionsResult[i].option;
                    for (const auto &token : opt->d_func()->tokens) {
                        lowerOptionTokenIndexes.insert(std::make_pair(Utils::toLower(token), i));
                    }
                }
            }
        }

        const std::vector<std::string> &params;
        const int parseOptions;
        const int displayOptions;
        const Command *const rootCommand;

        ParseResultPrivate *result;
        ParseResultData2 &core;
        size_t nonCommandIndex;
        const Command *targetCommand;
        const CommandPrivate *targetCommandData; // for convenience

        StringMap allOptionTokenIndexes;
        StringMap lowerOptionTokenIndexes;
        StringMap encounteredExclusiveGroups;

        // Resuable functions

        // ...
        void initOptionData(ParseResultData2::OptionData &data, const Option *opt) {
            const auto &d = opt->d_func();
            data.option = opt;
            data.argSize = d->arguments.size();
            data.argResult = nullptr;

            data.optionalArgIndex = -1;
            data.multiValueArgIndex = -1;

            // Build arg name indexes
            for (size_t i = 0; i < d->arguments.size(); ++i) {
                const auto &arg = d->arguments[i];
                if (arg.isOptional() && data.optionalArgIndex < 0) {
                    data.optionalArgIndex = i;
                }
                if (arg.multiValueEnabled() && data.multiValueArgIndex < 0) {
                    data.multiValueArgIndex = i;
                }
                data.argNameIndexes.insert(std::make_pair(arg.name(), i));
            }
        };

        // indexes: token indexes map
        // token:   token
        // sign:    beginning sign, '-' or '/'
        // pos:     followed argument beginning index
        // ->       option index
        int searchShortOptions(const StringMap &indexes, const std::string &token, char sign,
                               int *pos) {
            // Search for short option
            auto it = indexes.lower_bound(token);
            if (it != indexes.begin() && it != indexes.end() && token.find(it->first) != 0) {
                --it;
            }

            const auto &prefix = it->first;

            // Ignore `--` option because it's too special
            if (prefix == "--")
                return -1;

            if (it != indexes.end() && Utils::starts_with(token, prefix)) {
                const auto &idx = it->second;
                const auto &opt = core.allOptionsResult[idx].option;
                const auto &args = opt->d_func()->arguments;
                if (args.size() != 1 || !args.front().isRequired()) {
                    return idx;
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
                        return idx;
                    }
                    default:
                        break;
                }

                if (token.at(prefix.size()) == sign) {
                    if (pos)
                        *pos = int(prefix.size()) + 1;
                    return idx;
                }
            }
            return -1;
        };

        // indexes: token indexes map
        // token:   token
        // pos:     followed argument beginning index
        // ->       option index
        int searchOptionImpl(const StringMap &indexes, const std::string &token, int *pos) {
            if (pos)
                *pos = -1;

            if (indexes.empty())
                return -1;


            auto it = indexes.find(token);
            if (it != indexes.end()) {
                return it->second;
            }

            if (token.size() > 1) {
                // Try unix short option
                if (!(parseOptions & Parser::DontAllowUnixShortOptions) && token.front() == '-') {
                    return searchShortOptions(indexes, token, '-', pos);
                }

                // Try dos short option
                if (parseOptions & Parser::AllowDosShortOptions && token.front() == '/') {
                    return searchShortOptions(indexes, token, ':', pos);
                }
            }
            return -1;
        };

        // token:   token
        // pos:     followed argument beginning index
        // ->       option index
        int searchOption(const std::string &token, int *pos = nullptr) {
            // first search case sensitive map
            if (auto idx = searchOptionImpl(allOptionTokenIndexes, token, pos); idx >= 0)
                return idx;

            // second search case insensitive map if flag is set
            if (parseOptions & Parser::IgnoreOptionCase) {
                return searchOptionImpl(lowerOptionTokenIndexes, Utils::toLower(token), pos);
            }
            return -1;
        };

        // flags: group flags without preceding '-'
        // ->     option indexes
        std::vector<int> searchGroupFlags(const std::string &flags) {
            std::vector<int> res;
            for (const auto &flag : flags) {
                auto it = allOptionTokenIndexes.find(std::string("-") + flag);
                if (it == allOptionTokenIndexes.end()) {
                    return {};
                }
                const auto &idx = it->second;
                const auto &opt = core.allOptionsResult[idx].option;
                if (!opt->d_func()->arguments.empty()) {
                    return {};
                }
                res.push_back(idx);
            }
            return res;
        };

        void buildError(ParseResult::Error error, const std::vector<std::string> &placeholders,
                        const std::string &cancellationToken, const Argument *arg,
                        const Option *opt = nullptr) {
            result->error = error;
            result->errorPlaceholders = placeholders;
            result->cancellationToken = cancellationToken;
            result->errorArgument = arg;
            result->errorOption = opt;
        };


        // arg:       input argument
        // token:     token
        // val:       return value if success
        // setError:  whether to build error message if failed
        bool checkArgument(const Argument *arg, const std::string &token, Value *out,
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
                        buildError(ParseResult::InvalidOptionPosition, {token, arg->name()}, token,
                                   arg);
                    } else {
                        buildError(ParseResult::InvalidArgumentValue, {token, arg->name()}, token,
                                   arg);
                    }
                }
                return false;
            }

            if (d->validator) {
                std::string errorMessage;
                if (d->validator(token, out, &errorMessage)) {
                    return true;
                }
                if (setError) {
                    buildError(ParseResult::ArgumentValidateFailed,
                               {token, arg->name(), errorMessage}, token, arg);
                }
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
            if (setError) {
                buildError(ParseResult::ArgumentTypeMismatch, {token, arg->name(), expected}, token,
                           arg);
            }
            return false;
        };

        // optIndex:         option index in command's list
        // insertIfNotFound: if not colliding, set current group as visited
        // ->                colliding option index
        int searchExclusiveOption(int optIndex, bool insertIfNotFound = false) {
            const auto &groupName = targetCommandData->optionGroupNames[optIndex];
            if (groupName.empty())
                return -1;

            auto it2 = encounteredExclusiveGroups.find(groupName);
            if (it2 != encounteredExclusiveGroups.end()) {
                return it2->second; // colliding
            }

            if (insertIfNotFound) {
                encounteredExclusiveGroups[groupName] = optIndex;
            }
            return -1;
        };
    };

    ParseResult Parser::parse(const std::vector<std::string> &params, int parseOptions) {
        Q_D2(Parser);


        StringMap encounteredExclusiveGroups;



        bool hasArgument = false;
        bool hasOption = false;
        const Option *priorOpt = nullptr;
        auto checkOptionCommon = [&hasArgument, &hasOption, &priorOpt, &buildError, displayOptions,
                                  &core,
                                  &searchExclusiveOption](const std::string &token, int optIndex,
                                                          int occurrence) -> bool {
            const auto &opt = core.allOptionsResult[optIndex].option;
            // Check max occurrence
            if (opt->maxOccurrence() > 0 && occurrence == opt->maxOccurrence()) {
                buildError(ParseResult::OptionOccurTooMuch,
                           {
                               opt->helpText(Symbol::HP_ErrorText, displayOptions),
                               std::to_string(opt->maxOccurrence()),
                           },
                           token, nullptr, opt);
                return false;
            }

            // Check priority
            if (priorOpt) {
                switch (priorOpt->priorLevel()) {
                    case Option::ExclusiveToArguments: {
                        if (hasArgument) {
                            buildError(ParseResult::PriorOptionWithArguments,
                                       {
                                           priorOpt->helpText(Symbol::HP_ErrorText, displayOptions),
                                       },
                                       token, nullptr, opt);
                        }
                        break;
                    }
                    case Option::ExclusiveToOptions: {
                        if (hasOption) {
                            buildError(ParseResult::PriorOptionWithOptions,
                                       {
                                           priorOpt->helpText(Symbol::HP_ErrorText, displayOptions),
                                       },
                                       token, nullptr, opt);
                        }
                        break;
                    }
                    case Option::ExclusiveToAll: {
                        if (hasArgument) {
                            buildError(ParseResult::PriorOptionWithArguments,
                                       {
                                           priorOpt->helpText(Symbol::HP_ErrorText, displayOptions),
                                       },
                                       token, nullptr, opt);
                        } else if (hasOption) {
                            buildError(ParseResult::PriorOptionWithOptions,
                                       {
                                           priorOpt->helpText(Symbol::HP_ErrorText, displayOptions),
                                       },
                                       token, nullptr, opt);
                        }
                        break;
                    }
                    default:
                        break;
                }
            }

            // Check exclusive
            {
                const auto &exclusiveIdx =
                    searchExclusiveOption(optIndex - core.globalOptionsSize, true);
                if (exclusiveIdx >= 0) {
                    buildError(ParseResult::MutuallyExclusiveOptions,
                               {
                                   core.allOptionsResult[core.globalOptionsSize + exclusiveIdx]
                                       .option->helpText(Symbol::HP_ErrorText, displayOptions),
                                   opt->helpText(Symbol::HP_ErrorText, displayOptions),
                               },
                               token, nullptr, opt);
                    return false;
                }
            }
            return true;
        };

        auto isFlags = [](const std::string &s) -> bool {
            if (s.size() <= 1 || s.front() != '-')
                return false;
            return std::all_of(s.begin() + 1, s.end(), [](char c) {
                return std::isalpha(c) || std::isdigit(c); //
            });
        };

        struct OptionRange {
            std::vector<int> starts;
            std::vector<int> lengths;
            std::vector<std::string> precedingTokens;

            void push(int s, int l, std::string preceding = {}) {
                starts.push_back(s);
                lengths.push_back(l);
                precedingTokens.emplace_back(std::move(preceding));
            }
        };

        OptionRange *optRanges = new OptionRange[core.allOptionsSize]; // Alloc
        std::vector<std::string> positionalArguments;

        // Parse options
        for (auto i = nonCommandIndex; i < params.size(); ++i) {
            const auto &token = params[i];

            // Consider option
            int pos;
            if (auto optIndex = searchOption(token, &pos); optIndex >= 0) {
                const auto &optData = core.allOptionsResult[optIndex];
                const auto &opt = optData.option;
                auto &rangeData = optRanges[optIndex];

                // Check option common
                if (!checkOptionCommon(token, optIndex, rangeData.starts.size())) {
                    break;
                }

                const auto &dd = opt->d_func();

                // Collect positional arguments
                if (pos >= 0) {
                    // Must be a single value option
                    rangeData.push(i, 1, token.substr(pos));
                } else if (!dd->arguments.empty()) {
                    int minArgCount = (optData.optionalArgIndex < 0)
                                          ? dd->arguments.size()
                                          : (optData.optionalArgIndex + 1);
                    int maxArgCount = (optData.multiValueArgIndex < 0) ? minArgCount : 65536;

                    // TODO: collect arguments
                    int count = minArgCount;
                    auto j = i + minArgCount + 1; // next of last required index
                    if (j > params.size()) {
                        size_t argIndex = params.size() - i - 1;
                        buildError(ParseResult::MissingOptionArgument,
                                   {
                                       dd->arguments[argIndex].helpText(Symbol::HP_ErrorText,
                                                                        displayOptions),
                                       opt->helpText(Symbol::HP_ErrorText, displayOptions),
                                   },
                                   {}, nullptr, opt);
                        break;
                    }

                    auto end = std::min(params.size(), i + maxArgCount + 1);
                    for (; j < end; ++j) {
                        const auto &curToken = params[j];

                        // Breaked at next option
                        if (searchOption(curToken) >= 0) {
                            break;
                        }
                    }

                    rangeData.push(i, j - i - 1);
                } else {
                    rangeData.starts.push_back(i);
                    rangeData.lengths.push_back(0);
                    rangeData.precedingTokens.emplace_back();
                    rangeData.push(i, 0);
                }

                if (opt->priorLevel() > (priorOpt ? priorOpt->priorLevel() : Option::NoPrior))
                    priorOpt = opt;
                continue;
            }

            // Consider short flags
            if ((parseOptions & Parser::AllowUnixGroupFlags) && isFlags(token)) {
                auto opts = searchGroupFlags(token.substr(1));
                if (!opts.empty()) {
                    bool failed = false;
                    for (const auto &optIdx : std::as_const(opts)) {
                        auto &rangeData = optRanges[optIdx];

                        // Check option common
                        if (!checkOptionCommon(token, optIdx, rangeData.starts.size())) {
                            failed = true;
                            break;
                        }

                        rangeData.push(i, 0);
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

        if (result->error != ParseResult::NoError) {
            // TODO:
        }

        // args:    arguments
        // tokens:  tokens
        // res:     result array
        // ->       missing index
        // if failed, the error will be set, check it first.
        auto parseArgument = [&buildError, &checkArgument](const std::vector<Argument> &args,
                                                           const std::vector<std::string> &tokens,
                                                           std::vector<Value> *res,
                                                           int multiValueIndex) -> int {
            // Parse forward
            size_t end = args.size();
            if (multiValueIndex >= 0) {
                end = multiValueIndex + 1; // Stop after multi-value arg
            }

            size_t k = 0;
            for (size_t max = std::min(tokens.size(), end); k < max; ++k) {
                const auto &arg = args.at(k);
                Value val;
                if (!checkArgument(&arg, tokens.at(k), &val)) {
                    return -1;
                }
                res[k].push_back(val);
            }

            if (k < end) {
                const auto &arg = args.at(k);
                if (arg.isRequired()) {
                    return k;
                }
                return -1;
            }

            // Parse backward
            end = tokens.size();
            if (multiValueIndex >= 0 && multiValueIndex < args.size() - 1) {
                size_t backwardStart = multiValueIndex + 1;
                size_t backwardCount = args.size() - multiValueIndex - 1;
                if (tokens.size() < backwardCount + k) {
                    return multiValueIndex + 1;
                }
                end -= backwardCount;

                for (size_t j = 0; j < backwardCount; ++j) {
                    const auto &arg = args.at(multiValueIndex + j + 1);
                    Value val;
                    if (!checkArgument(&arg, tokens.at(end + j), &val)) {
                        return -1;
                    }
                    res[multiValueIndex + j + 1].push_back(val);
                }
            }

            if (end <= k) {
                return -1;
            }

            // Too many
            if (multiValueIndex < 0) {
                const auto &token = tokens.at(k);
                if (token.front() == '-') {
                    buildError(ParseResult::UnknownOption, {token}, token, nullptr);
                    return -1;
                }

                if (args.empty()) {
                    buildError(ParseResult::UnknownCommand, {token}, token, nullptr);
                    return -1;
                }

                buildError(ParseResult::TooManyArguments, {}, token, nullptr);
                return -1;
            }

            // Consider multiple arguments
            const auto &arg = args.at(multiValueIndex);
            auto &resVec = res[multiValueIndex];
            for (size_t j = k; j < end; ++j) {
                const auto &token = tokens.at(j);
                Value val;
                if (!checkArgument(&arg, token, &val)) {
                    break;
                }
                resVec.push_back(val);
            }
            return -1;
        };

    out_parse_arguments:

        // Check required arguments
        if (error == ParseResult::NoError) {
            bool hasAutoOption = false;

            // Automatic set options
            if (result->argResult.empty() && result->optResult.empty()) {
                for (const auto &opt : cmd->d_func()->options) {
                    if (opt.priorLevel() == Option::AutoSetWhenNoSymbols) {
                        result->optResult[opt.name()] = {};
                        hasAutoOption = true;
                        break;
                    }
                }

                for (const auto &opt : std::as_const(globalOptions)) {
                    if (opt->priorLevel() == Option::AutoSetWhenNoSymbols) {
                        result->optResult[opt->name()] = {};
                        hasAutoOption = true;
                        break;
                    }
                }
            }

            if (priorOpt || hasAutoOption) {
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
                    errorPlaceholders = {
                        missingOpt->helpText(Symbol::HP_ErrorText, displayOptions),
                    };
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
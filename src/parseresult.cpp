#include "parseresult.h"
#include "parseresult_p.h"

#include <stdexcept>

#include "utils.h"
#include "strings.h"
#include "system.h"

#include "parser_p.h"
#include "helplayout_p.h"

#include "option_p.h"
#include "command_p.h"

namespace SysCmdLine {

    std::string ParseResultPrivate::correctionText() const {
        std::vector<std::string> expectedValues;
        switch (error) {
            case ParseResult::UnknownOption:
            case ParseResult::InvalidOptionPosition: {
                for (int i = 0; i < core.allOptionsSize; ++i) {
                    const auto &opt = *core.allOptionsResult[i].option;
                    for (const auto &token : opt.d_func()->tokens) {
                        expectedValues.push_back(token);
                    }
                }

                if (error == ParseResult::UnknownOption)
                    break;

                // Fallback as invalid argument case
            }

            case ParseResult::InvalidArgumentValue: {
                auto d = errorArgument->d_func();
                for (const auto &item : d->expectedValues) {
                    expectedValues.push_back(item.toString());
                }

                if (errorOption) // option's argument?
                    break;

                // Fallback as unknown command case
            }

            case ParseResult::UnknownCommand: {
                for (const auto &cmd : command->d_func()->commands) {
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

        std::string ss;
        const auto &parserData = parser.d_func();
        ss += Utils::formatText(
            parserData->textProvider(Strings::Information, Strings::MatchCommand), {input});
        for (const auto &item : std::as_const(suggestions)) {
            ss += "\n" + parserData->indent() + item;
        }
        return ss;
    }

    void ParseResultPrivate::showMessage(const std::string &info, const std::string &warn,
                                         const std::string &err, bool isMsg) const {

        // Extract arguments, options, commands from categories

        struct Lists {
            HelpLayout::List *data;
            int size;
        };

        const auto &getLists = [](int displayOptions, const StringMap &catalog,
                                  const StringList &catalogNames,         // catalog

                                  const StringMap &symbolIndexes,         // name -> index
                                  int symbolCount, const Symbol *(*getter)(int, const void *),
                                  const void *user,                       // get symbol from index

                                  std::string (*getName)(const Symbol *), // get name of symbol
                                  int *maxWidth) -> Lists {
            Lists res;
            res.size = catalogNames.size() + 1;
            res.data = new HelpLayout::List[res.size];

            // symbol indexes
            SSizeMap restIndexes;
            for (size_t i = 0; i < symbolCount; ++i) {
                restIndexes.insert(std::make_pair(i, 0));
            }

            // catalogues
            for (size_t i = 0; i < catalogNames.size(); ++i) {
                const auto &catalogName = catalogNames[i];
                auto &list = res.data[i];
                list.title = catalogName;

                auto &symbolNames = *map_search<StringList>(catalog, catalogName);
                for (const auto &name : symbolNames) {
                    const auto &idx = symbolIndexes.find(name)->second;
                    const auto &sym = getter(idx, user);

                    auto first = sym->helpText(Symbol::HP_FirstColumn, displayOptions);
                    auto second = sym->helpText(Symbol::HP_SecondColumn, displayOptions);
                    *maxWidth = std::max(int(first.size()), *maxWidth);
                    list.firstColumn.emplace_back(first);
                    list.secondColumn.emplace_back(second);
                    restIndexes.erase(idx);
                }
            }

            // rest
            {
                auto &list = res.data[catalogNames.size()];
                for (const auto &pair : restIndexes) {
                    const auto &sym = getter(pair.first, user);

                    auto first = sym->helpText(Symbol::HP_FirstColumn, displayOptions);
                    auto second = sym->helpText(Symbol::HP_SecondColumn, displayOptions);
                    *maxWidth = std::max(int(first.size()), *maxWidth);
                    list.firstColumn.emplace_back(first);
                    list.secondColumn.emplace_back(second);
                }
            }

            return res;
        };

        const auto &d = command->d_func();
        const auto &parserData = parser.d_func();
        const auto &catalogueData = d->catalogue.d_func();
        const auto displayOptions = parserData->displayOptions;

        bool noHelp = isMsg && (displayOptions & Parser::DontShowHelpOnError);
        bool noIntro = isMsg && (displayOptions & Parser::DontShowIntroOnError);

        // Alloc
        int maxWidth = 0;
        Lists argLists = noHelp
                             ? Lists()
                             : getLists(
                                   displayOptions, catalogueData->arg, catalogueData->arguments,
                                   core.argNameIndexes, int(d->arguments.size()),
                                   [](int i, const void *user) -> const Symbol * {
                                       return &reinterpret_cast<decltype(d)>(user)->arguments[i]; //
                                   },
                                   d,
                                   [](const Symbol *s) {
                                       return static_cast<const Argument *>(s)->name(); //
                                   },
                                   &maxWidth);

        Lists optLists = noHelp ? Lists()
                                : getLists(
                                      displayOptions, catalogueData->opt, catalogueData->options,
                                      core.allOptionTokenIndexes, int(core.allOptionsSize),
                                      [](int i, const void *user) -> const Symbol * {
                                          return reinterpret_cast<const ParseResultData2 *>(user)
                                              ->allOptionsResult[i]
                                              .option; //
                                      },
                                      &core,
                                      [](const Symbol *s) {
                                          return static_cast<const Option *>(s)->token(); //
                                      },
                                      &maxWidth);

        Lists cmdLists = noHelp
                             ? Lists()
                             : getLists(
                                   displayOptions, catalogueData->cmd, catalogueData->commands,
                                   core.argNameIndexes, int(d->commands.size()),
                                   [](int i, const void *user) -> const Symbol * {
                                       return &reinterpret_cast<decltype(d)>(user)->commands[i]; //
                                   },
                                   d,
                                   [](const Symbol *s) {
                                       return static_cast<const Command *>(s)->name(); //
                                   },
                                   &maxWidth);

        const auto &helpLayoutData = parserData->helpLayout.d_func();
        if (displayOptions & Parser::DisplayOption::AlignAllCatalogues) {
            for (const auto &helpItem : helpLayoutData->itemDataList) {
                if (helpItem.itemType != HelpLayoutPrivate::UserHelpList) {
                    continue;
                }
                for (const auto &item : helpItem.list.firstColumn) {
                    maxWidth = std::max(int(item.size()), maxWidth);
                }
            }
        } else {
            maxWidth = 0;
        }

        const auto &cmdDesc = d->detailedDescription.empty() ? d->desc : d->detailedDescription;

        // Get last
        int last = helpLayoutData->itemDataList.size() - 1;
        for (int i = last; i >= 0; --i) {
            const auto &item = helpLayoutData->itemDataList[i];

            bool empty = true;
            switch (item.itemType) {
                case HelpLayoutPrivate::HelpText: {
                    if (noHelp)
                        break;
                    switch (static_cast<HelpLayout::HelpTextItem>(item.index)) {
                        case HelpLayout::HT_Prologue: {
                            empty = parserData->prologue.empty();
                            break;
                        }
                        case HelpLayout::HT_Epilogue: {
                            empty = parserData->epilogue.empty();
                            break;
                        }
                        case HelpLayout::HT_Description: {
                            empty = cmdDesc.empty();
                            break;
                        }
                        case HelpLayout::HT_Usage: {
                            empty = false;
                            break;
                        }
                    }
                    break;
                }
                case HelpLayoutPrivate::HelpList: {
                    if (noHelp)
                        break;
                    switch (static_cast<HelpLayout::HelpListItem>(item.index)) {
                        case HelpLayout::HL_Arguments: {
                            empty = argLists.size == 0;
                            break;
                        }
                        case HelpLayout::HL_Options: {
                            empty = optLists.size == 0;
                            break;
                        }
                        case HelpLayout::HL_Commands: {
                            empty = cmdLists.size == 0;
                            break;
                        }
                    }
                    break;
                }
                case HelpLayoutPrivate::Message: {
                    switch (static_cast<HelpLayout::MessageItem>(item.index)) {
                        case HelpLayout::MI_Information: {
                            empty = info.empty();
                            break;
                        }
                        case HelpLayout::MI_Warning: {
                            empty = warn.empty();
                            break;
                        }
                        case HelpLayout::MI_Critical: {
                            empty = err.empty();
                            break;
                        }
                    }
                    break;
                }
                case HelpLayoutPrivate::UserHelpText: {
                    if (noHelp)
                        break;
                    empty = item.text.lines.empty();
                    break;
                }
                case HelpLayoutPrivate::UserHelpList: {
                    if (noHelp)
                        break;
                    empty = item.list.firstColumn.empty();
                    break;
                }
                case HelpLayoutPrivate::UserHelpPlain: {
                    if (noHelp)
                        break;
                    empty = false;
                    break;
                }
            }

            if (!empty) {
                last = i;
                break;
            }
        }

        // Output
        for (int i = 0; i <= last; ++i) {
            const auto &item = helpLayoutData->itemDataList[i];
            HelpLayout::Context ctx;
            ctx.parser = &parser;
            bool hasNext = i < last;
            ctx.hasNext = hasNext;
            switch (item.itemType) {
                case HelpLayoutPrivate::HelpText: {
                    HelpLayout::Text text;
                    ctx.text = &text;
                    switch (static_cast<HelpLayout::HelpTextItem>(item.index)) {
                        case HelpLayout::HT_Prologue: {
                            if (noIntro)
                                break;
                            text.lines = parserData->prologue;
                            break;
                        }
                        case HelpLayout::HT_Epilogue: {
                            if (noIntro)
                                break;
                            text.lines = parserData->epilogue;
                            break;
                        }
                        case HelpLayout::HT_Description: {
                            if (noHelp)
                                break;
                            text.lines = cmdDesc;
                            break;
                        }
                        case HelpLayout::HT_Usage: {
                            if (noHelp)
                                break;
                            std::vector<Option> allOptions;
                            allOptions.reserve(core.globalOptionsSize);
                            for (int i = 0; i < core.globalOptionsSize; ++i) {
                                allOptions.emplace_back(*core.allOptionsResult[i].option);
                            }
                            bool hasCommands;
                            bool hasOptions;
                            void *a[3] = {
                                &allOptions,
                                &hasCommands,
                                &hasOptions,
                            };
                            text.lines = command->helpText(Symbol::HP_Usage, displayOptions, a);
                            if (hasCommands) {
                                text.lines += " [" +
                                              parserData->textProvider(Strings::Token,
                                                                       Strings::OptionalCommands) +
                                              "]";
                            }
                            if (hasOptions) {
                                text.lines += " [" +
                                              parserData->textProvider(Strings::Token,
                                                                       Strings::OptionalOptions) +
                                              "]";
                            }
                            break;
                        }
                    }
                    item.out(ctx);
                    break;
                }
                case HelpLayoutPrivate::HelpList: {
                    if (noHelp)
                        break;
                    ctx.spacing = maxWidth;
                    switch (static_cast<HelpLayout::HelpListItem>(item.index)) {
                        case HelpLayout::HL_Arguments: {
                            for (int j = 0; j < argLists.size; ++j) {
                                ctx.hasNext =
                                    hasNext ||
                                    (j < argLists.size - 1 &&
                                     !argLists.data[argLists.size - 1].firstColumn.empty());
                                ctx.list = &argLists.data[j];
                                item.out(ctx);
                            }
                            break;
                        }
                        case HelpLayout::HL_Options: {
                            for (int j = 0; j < optLists.size; ++j) {
                                ctx.hasNext =
                                    hasNext ||
                                    (j < optLists.size - 1 &&
                                     !optLists.data[optLists.size - 1].firstColumn.empty());
                                ctx.list = &optLists.data[j];
                                item.out(ctx);
                            }
                            break;
                        }
                        case HelpLayout::HL_Commands: {
                            for (int j = 0; j < cmdLists.size; ++j) {
                                ctx.hasNext =
                                    hasNext ||
                                    (j < cmdLists.size - 1 &&
                                     !cmdLists.data[cmdLists.size - 1].firstColumn.empty());
                                ctx.list = &cmdLists.data[j];
                                item.out(ctx);
                            }
                            break;
                        }
                    }
                    break;
                }
                case HelpLayoutPrivate::Message: {
                    HelpLayout::Text text;
                    ctx.text = &text;
                    switch (static_cast<HelpLayout::MessageItem>(item.index)) {
                        case HelpLayout::MI_Information: {
                            text.lines = info;
                            break;
                        }
                        case HelpLayout::MI_Warning: {
                            text.lines = warn;
                            break;
                        }
                        case HelpLayout::MI_Critical: {
                            text.lines = err;
                            break;
                        }
                    }
                    item.out(ctx);
                    break;
                }
                case HelpLayoutPrivate::UserHelpText: {
                    if (noHelp)
                        break;
                    ctx.text = &item.text;
                    item.out(ctx);
                    break;
                }
                case HelpLayoutPrivate::UserHelpList: {
                    if (noHelp)
                        break;
                    ctx.list = &item.list;
                    ctx.spacing = maxWidth;
                    item.out(ctx);
                    break;
                }
                case HelpLayoutPrivate::UserHelpPlain: {
                    if (noHelp)
                        break;
                    item.out(ctx);
                    break;
                }
            }
        }

        // Free
        delete[] argLists.data;
        delete[] optLists.data;
        delete[] cmdLists.data;
    }

    ParseResult::ParseResult() : SharedBase(nullptr) {
    }

    ParseResult::~ParseResult() {
    }

    Command ParseResult::rootCommand() const {
        Q_D2(ParseResult);
        return d->parser.rootCommand();
    }

    const std::vector<std::string> &ParseResult::arguments() const {
        Q_D2(ParseResult);
        return d->arguments;
    }

    int ParseResult::invoke(int errCode) const {
        Q_D2(ParseResult);
        if (d->error != NoError) {
            showError();
            return errCode;
        }
        return dispatch();
    }

    int ParseResult::dispatch() const {
        Q_D2(ParseResult);
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

        if (!handler) {
            throw std::runtime_error("command \"" + cmd.name() + "\" doesn't have a valid handler");
        }

        return handler(*this);
    }

    ParseResult::Error ParseResult::error() const {
        Q_D2(ParseResult);
        return d->error;
    }

    std::string ParseResult::errorText() const {
        Q_D2(ParseResult);
        if (d->error == NoError)
            return {};
        return Utils::formatText(d->parser.d_func()->textProvider(Strings::ParseError, d->error),
                                 d->errorPlaceholders);
    }

    std::string ParseResult::correctionText() const {
        Q_D2(ParseResult);
        return d->correctionText();
    }

    std::string ParseResult::cancellationToken() const {
        Q_D2(ParseResult);
        return d->cancellationToken;
    }

    Command ParseResult::command() const {
        Q_D2(ParseResult);
        return *d->command;
    }

    std::vector<Option> ParseResult::globalOptions() const {
        Q_D2(ParseResult);
        std::vector<Option> res;
        res.reserve(d->core.globalOptionsSize);
        for (int i = 0; i < d->core.globalOptionsSize; ++i) {
            res.push_back(*d->core.allOptionsResult[i].option);
        }
        return res;
    }

    std::vector<int> ParseResult::commandIndexStack() const {
        Q_D2(ParseResult);
        return d->stack;
    }

    void ParseResult::showError() const {
        Q_D2(ParseResult);
        if (d->error == NoError)
            return;

        const auto &parserData = d->parser.d_func();
        const auto &displayOptions = parserData->displayOptions;
        d->showMessage(
            (!(displayOptions & Parser::SkipCorrection)) ? d->correctionText() : std::string(), {},
            parserData->textProvider(Strings::Title, Strings::Error) + ": " + errorText(),
            displayOptions & Parser::DontShowHelpOnError);
    }

    void ParseResult::showHelpText() const {
        Q_D2(ParseResult);
        d->showMessage({}, {}, {});
    }

    void ParseResult::showMessage(const std::string &info, const std::string &warn,
                                  const std::string &err) const {
        Q_D2(ParseResult);
        const auto &displayOptions = d->parser.d_func()->displayOptions;
        d->showMessage(info, warn, err, displayOptions & Parser::DontShowHelpOnError);
    }

    bool ParseResult::isHelpSet() const {
        Q_D2(ParseResult);
        return d->helpSet;
    }

    bool ParseResult::isVersionSet() const {
        Q_D2(ParseResult);
        return d->versionSet;
    }

    ParseResult::ParseResult(ParseResultPrivate *d) : SharedBase(d) {
    }

}
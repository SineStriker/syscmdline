#include "parseresult.h"
#include "parseresult_p.h"

#include "utils.h"
#include "strings.h"
#include "system.h"

#include "parser_p.h"

#include "option_p.h"
#include "command_p.h"

namespace SysCmdLine {

    std::string ParseResultPrivate::correctionText() const {
        std::vector<std::string> expectedValues;
        switch (error) {
            case ParseResult::UnknownOption:
            case ParseResult::InvalidOptionPosition: {
                for (int i = 0; i < core.allOptionsSize; ++i) {
                    const auto &opt = core.allOptions[i];
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

    // static void defaultPrinter(MessageType messageType, const std::string &title,
    //                            const std::vector<std::string> &lines, bool hasNext) {
    //     bool highlight = false;
    //     switch (messageType) {
    //         case MT_Information:
    //         case MT_Healthy:
    //         case MT_Warning:
    //         case MT_Critical:
    //             highlight = true;
    //             break;
    //         default:
    //             break;
    //     }

    //     if (!title.empty()) {
    //         u8printf("%s:\n", title.data());
    //     }

    //     for (const auto &item : lines) {
    //         u8debug(messageType, highlight, "%s\n", item.data());
    //     }

    //     if (hasNext) {
    //         u8printf("\n");
    //     }
    // }

    // void ParseResultPrivate::showMessage(const std::string &info, const std::string &warn,
    //                                      const std::string &err, bool noHelp) const {
    //     const auto &d = parserData.data();
    //     const auto &dd = parserData->helpLayout.d_func();

    //     struct HelpText {
    //         std::pair<std::string, std::vector<std::string>> description;
    //         std::pair<std::string, std::vector<std::string>> usage;
    //         std::vector<std::pair<std::string, std::vector<std::string>>> arguments;
    //         std::vector<std::pair<std::string, std::vector<std::string>>> options;
    //         std::vector<std::pair<std::string, std::vector<std::string>>> commands;
    //     };
    //     const auto &ht = [noHelp, this]() {
    //         HelpText result;

    //         const auto &d = command->d_func();
    //         const auto &dd = d->catalogue.d_func();
    //         const auto displayOptions = parserData->displayOptions;
    //         const auto &helpLayout = parserData->helpLayout;

    //         // Build option indexes
    //         auto options = d->options;
    //         auto optionNameIndexes = d->optionNameIndexes;

    //         options.reserve(options.size() + globalOptions.size());
    //         optionNameIndexes.reserve(optionNameIndexes.size() + globalOptions.size());
    //         for (const auto &item : globalOptions) {
    //             optionNameIndexes.insert(std::make_pair(item->name(), options.size()));
    //             options.push_back(*item);
    //         }

    //         // Description
    //         {
    //             const auto &desc =
    //                 d->detailedDescription.empty() ? d->desc : d->detailedDescription;
    //             result.description.first = Strings::text(Strings::Title, Strings::Description);
    //             if (!desc.empty()) {
    //                 std::string ss;
    //                 ss += std::string(helpLayout.size(HelpLayout::ST_Indent), ' ') + desc;
    //                 result.description.second = {ss};
    //             }
    //         }

    //         // Usage
    //         {
    //             std::string ss;
    //             ss += std::string(helpLayout.size(HelpLayout::ST_Indent), ' ');

    //             // parent commands
    //             {
    //                 const Command *p = &parserData->rootCommand;
    //                 for (const auto &item : std::as_const(stack)) {
    //                     ss += p->name() + " ";
    //                     p = &p->d_func()->subCommands[item];
    //                 }
    //             }

    //             // usage
    //             ss += command->helpText(Symbol::HP_Usage, displayOptions, &options);

    //             result.usage = {
    //                 Strings::text(Strings::Title, Strings::Usage),
    //                 {ss},
    //             };
    //         }

    //         if (!noHelp) {
    //             int allWidest = 0;
    //             bool allAlign = displayOptions & Parser::AlignAllCatalogues;
    //             bool sameAlign = displayOptions & Parser::AlignSameCatalogues;

    //             bool hasArgs = false;
    //             bool hasOptions = false;
    //             bool hasCommands = false;
    //             for (const auto &item : parserData->helpLayout.d_func()->layoutItems) {
    //                 switch (item.item) {
    //                     case HelpLayout::HI_Arguments:
    //                         hasArgs = true;
    //                         break;
    //                     case HelpLayout::HI_Options:
    //                         hasOptions = true;
    //                         break;
    //                     case HelpLayout::HI_Commands:
    //                         hasCommands = true;
    //                         break;
    //                     default:
    //                         break;
    //                 }
    //             }

    //             const auto &helper_d = parserData->helpLayout.d_func();
    //             if (allAlign) {
    //                 if (hasArgs) {
    //                     for (const auto &item : d->arguments) {
    //                         allWidest = std::max<int>(
    //                             int(item.helpText(Symbol::HP_FirstColumn,
    //                             displayOptions).size()), allWidest);
    //                     }
    //                 }
    //                 if (hasOptions) {
    //                     for (const auto &item : std::as_const(options)) {
    //                         allWidest = std::max<int>(
    //                             int(item.helpText(Symbol::HP_FirstColumn,
    //                             displayOptions).size()), allWidest);
    //                     }
    //                 }
    //                 if (hasCommands) {
    //                     for (const auto &item : d->subCommands) {
    //                         allWidest = std::max<int>(
    //                             int(item.helpText(Symbol::HP_FirstColumn,
    //                             displayOptions).size()), allWidest);
    //                     }
    //                 }
    //             }

    //             // Arguments
    //             if (!d->arguments.empty() && hasArgs) {
    //                 int widest = allWidest;
    //                 if (sameAlign && !allAlign) {
    //                     for (const auto &item : d->arguments) {
    //                         widest = std::max<int>(
    //                             int(item.helpText(Symbol::HP_FirstColumn,
    //                             displayOptions).size()), widest);
    //                     }
    //                 }
    //                 result.arguments = collectItems(
    //                     widest, helpLayout, dd->_arg, dd->_argIndexes, d->argumentNameIndexes,
    //                     Strings::text(Strings::Title, Strings::Arguments),

    //                     // getter
    //                     [d, displayOptions](size_t idx) {
    //                         return d->arguments[idx].helpText(Symbol::HP_FirstColumn,
    //                                                           displayOptions);
    //                     },
    //                     [d, displayOptions](size_t idx) {
    //                         return d->arguments[idx].helpText(Symbol::HP_SecondColumn,
    //                                                           displayOptions);
    //                     });
    //             }

    //             // Options
    //             if (!options.empty() && hasOptions) {
    //                 int widest = allWidest;
    //                 if (sameAlign && !allAlign) {
    //                     for (const auto &item : std::as_const(options)) {
    //                         widest = std::max<int>(
    //                             int(item.helpText(Symbol::HP_FirstColumn,
    //                             displayOptions).size()), widest);
    //                     }
    //                 }
    //                 result.options = collectItems(
    //                     widest, helpLayout, dd->_opt, dd->_optIndexes, optionNameIndexes,
    //                     Strings::text(Strings::Title, Strings::Options),

    //                     // getter
    //                     [&options, displayOptions](size_t idx) {
    //                         return options[idx].helpText(Symbol::HP_FirstColumn, displayOptions);
    //                     },
    //                     [&options, displayOptions](size_t idx) {
    //                         return options[idx].helpText(Symbol::HP_SecondColumn,
    //                         displayOptions);
    //                     });
    //             }

    //             // Commands
    //             if (!d->subCommands.empty() && hasCommands) {
    //                 int widest = allWidest;
    //                 if (sameAlign && !allAlign) {
    //                     for (const auto &item : d->subCommands) {
    //                         widest = std::max<int>(
    //                             int(item.helpText(Symbol::HP_FirstColumn,
    //                             displayOptions).size()), widest);
    //                     }
    //                 }
    //                 result.commands = collectItems(
    //                     widest, helpLayout, dd->_cmd, dd->_cmdIndexes, d->subCommandNameIndexes,
    //                     Strings::text(Strings::Title, Strings::Commands),

    //                     // getter
    //                     [d, displayOptions](size_t idx) {
    //                         return d->subCommands[idx].helpText(Symbol::HP_FirstColumn,
    //                                                             displayOptions);
    //                     },
    //                     [d, displayOptions](size_t idx) {
    //                         return d->subCommands[idx].helpText(Symbol::HP_SecondColumn,
    //                                                             displayOptions);
    //                     });
    //             }
    //         }
    //         return result;
    //     }();

    //     int last = -1;
    //     for (int i = 0; i < dd->layoutItems.size(); ++i) {
    //         const auto &item = dd->layoutItems.at(i);
    //         bool hasValue = false;
    //         switch (item.item) {
    //             case HelpLayout::HI_CustomText: {
    //                 if (item.printer && !noHelp) {
    //                     hasValue = true;
    //                 }
    //                 break;
    //             }
    //             case HelpLayout::HI_Prologue: {
    //                 if (!d->intro[Parser::Prologue].empty() && !noHelp) {
    //                     hasValue = true;
    //                 }
    //                 break;
    //             }
    //             case HelpLayout::HI_Information: {
    //                 if (!info.empty()) {
    //                     hasValue = true;
    //                 }
    //                 break;
    //             }
    //             case HelpLayout::HI_Warning: {
    //                 if (!warn.empty()) {
    //                     hasValue = true;
    //                 }
    //                 break;
    //             }
    //             case HelpLayout::HI_Error: {
    //                 if (!err.empty()) {
    //                     hasValue = true;
    //                 }
    //                 break;
    //             }
    //             case HelpLayout::HI_Description: {
    //                 if (!ht.description.second.empty() && !noHelp) {
    //                     hasValue = true;
    //                 }
    //                 break;
    //             }
    //             case HelpLayout::HI_Usage: {
    //                 if (!ht.usage.second.empty() && !noHelp) {
    //                     hasValue = true;
    //                 }
    //                 break;
    //             }
    //             case HelpLayout::HI_Arguments: {
    //                 if (!ht.arguments.empty() && !noHelp) {
    //                     hasValue = true;
    //                 }
    //                 break;
    //             }
    //             case HelpLayout::HI_Options: {
    //                 if (!ht.options.empty() && !noHelp) {
    //                     hasValue = true;
    //                 }
    //                 break;
    //             }
    //             case HelpLayout::HI_Commands: {
    //                 if (!ht.commands.empty() && !noHelp) {
    //                     hasValue = true;
    //                 }
    //                 break;
    //             }
    //             case HelpLayout::HI_Epilogue: {
    //                 if (!d->intro[Parser::Epilogue].empty() && !noHelp) {
    //                     hasValue = true;
    //                 }
    //                 break;
    //             }
    //         }
    //         if (hasValue) {
    //             last = i;
    //         }
    //     }

    //     auto displayStr = [](const HelpLayoutData::LayoutItem &item, bool hasNext,
    //                          const std::string &s, MessageType messageType = MT_Debug) {
    //         if (s.empty())
    //             return;
    //         if (item.printer) {
    //             item.printer({}, {s}, hasNext);
    //         } else {
    //             defaultPrinter(messageType, {}, {s}, hasNext);
    //         }
    //     };

    //     auto displayPair = [](const HelpLayoutData::LayoutItem &item, bool hasNext,
    //                           const std::pair<std::string, std::vector<std::string>> &p,
    //                           MessageType messageType = MT_Debug) {
    //         if (p.second.empty())
    //             return;
    //         if (item.printer) {
    //             item.printer(p.first, p.second, hasNext);
    //         } else {
    //             defaultPrinter(messageType, p.first, p.second, hasNext);
    //         }
    //     };

    //     auto displayArr =
    //         [](const HelpLayoutData::LayoutItem &item, bool hasNext,
    //            const std::vector<std::pair<std::string, std::vector<std::string>>> &arr) {
    //             int last = int(arr.size()) - 1;
    //             if (!hasNext) {
    //                 for (int j = 0; j < arr.size(); ++j) {
    //                     if (!arr.at(j).second.empty()) {
    //                         last = j;
    //                     }
    //                 }
    //             }
    //             for (int j = 0; j <= last; ++j) {
    //                 const auto &item2 = arr.at(j);
    //                 if (item.printer) {
    //                     item.printer(item2.first, item2.second, hasNext || j < last);
    //                 } else {
    //                     defaultPrinter(MessageType::MT_Debug, item2.first, item2.second,
    //                                    hasNext || j < last);
    //                 }
    //             }
    //         };

    //     for (int i = 0; i <= last; ++i) {
    //         const auto &item = dd->layoutItems.at(i);
    //         bool hasNext = i < last;
    //         switch (item.item) {
    //             case HelpLayout::HI_CustomText: {
    //                 if (item.printer && !noHelp) {
    //                     item.printer({}, {}, hasNext);
    //                 }
    //                 break;
    //             }
    //             case HelpLayout::HI_Prologue: {
    //                 if (!noHelp)
    //                     displayStr(item, hasNext, d->intro[Parser::Prologue]);
    //                 break;
    //             }
    //             case HelpLayout::HI_Information: {
    //                 displayStr(item, hasNext, info);
    //                 break;
    //             }
    //             case HelpLayout::HI_Warning: {
    //                 displayStr(item, hasNext, warn, MT_Warning);
    //                 break;
    //             }
    //             case HelpLayout::HI_Error: {
    //                 displayStr(item, hasNext, err, MT_Critical);
    //                 break;
    //             }
    //             case HelpLayout::HI_Description: {
    //                 if (!noHelp)
    //                     displayPair(item, hasNext, ht.description);
    //                 break;
    //             }
    //             case HelpLayout::HI_Usage: {
    //                 if (!noHelp)
    //                     displayPair(item, hasNext, ht.usage);
    //                 break;
    //             }
    //             case HelpLayout::HI_Arguments: {
    //                 if (!noHelp)
    //                     displayArr(item, hasNext, ht.arguments);
    //                 break;
    //             }
    //             case HelpLayout::HI_Options: {
    //                 if (!noHelp)
    //                     displayArr(item, hasNext, ht.options);
    //                 break;
    //             }
    //             case HelpLayout::HI_Commands: {
    //                 if (!noHelp)
    //                     displayArr(item, hasNext, ht.commands);
    //                 break;
    //             }
    //             case HelpLayout::HI_Epilogue: {
    //                 if (!noHelp)
    //                     displayStr(item, hasNext, d->intro[Parser::Epilogue]);
    //                 break;
    //             }
    //         }
    //     }
    // }

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
            res.push_back(d->core.allOptions[i]);
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
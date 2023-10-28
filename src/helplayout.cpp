#include "helplayout.h"
#include "helplayout_p.h"

#include "system.h"
#include "parser_p.h"

namespace SysCmdLine {

    static inline void printLast(bool hasNext) {
        if (hasNext)
            u8printf("\n");
    }

    static void defaultTextPrinter(const HelpLayout::Context &ctx) {
        if (ctx.text->title.empty()) {
            u8printf("%s:\n", ctx.text->title.data());
            u8printf("%s%s\n", ctx.parser->d_func()->indent().data(), ctx.text->lines.data());
        } else {
            u8printf("%s\n", ctx.text->lines.data());
        }
        printLast(ctx.hasNext);
    }

    static void defaultListPrinter(const HelpLayout::Context &ctx) {
        printLast(ctx.hasNext);

        u8printf("%s:\n", ctx.text->title.data());
    }

    static void defaultWarnPrinter(const HelpLayout::Context &ctx) {
        if (ctx.text->title.empty()) {
            u8debug(MT_Warning, false, "%s:\n", ctx.text->title.data());
            u8debug(MT_Warning, false, "%s%s\n", ctx.parser->d_func()->indent().data(),
                    ctx.text->lines.data());
        } else {
            u8debug(MT_Warning, false, "%s\n", ctx.text->lines.data());
        }
        printLast(ctx.hasNext);
    }

    static void defaultErrorPrinter(const HelpLayout::Context &ctx) {
        if (ctx.text->title.empty()) {
            u8debug(MT_Critical, true, "%s:\n", ctx.text->title.data());
            u8debug(MT_Critical, true, "%s%s\n", ctx.parser->d_func()->indent().data(),
                    ctx.text->lines.data());
        } else {
            u8debug(MT_Critical, true, "%s\n", ctx.text->lines.data());
        }
        printLast(ctx.hasNext);
    }

    HelpLayout::HelpLayout() : SharedBase(new HelpLayoutPrivate()) {
    }

    HelpLayout::~HelpLayout() {
    }

    void HelpLayout::addHelpTextItem(HelpTextItem type, const Output &out) {
        Q_D(HelpLayout);
        d->itemDataList.push_back(
            {HelpLayoutPrivate::HelpText, type, out ? out : defaultTextPrinter, {}, {}});
    }
    void HelpLayout::addHelpListItem(HelpListItem type, const Output &out) {
        Q_D(HelpLayout);
        d->itemDataList.push_back(
            {HelpLayoutPrivate::HelpList, type, out ? out : defaultListPrinter, {}, {}});
    }
    void HelpLayout::addMessageItem(MessageItem type, const Output &out) {
        Q_D(HelpLayout);
        d->itemDataList.push_back({HelpLayoutPrivate::Message, type, out ? out : [](MessageItem item) {
                                       switch (item) {
                                           case MI_Warning:
                                               return defaultWarnPrinter;
                                           case MI_Critical:
                                               return defaultErrorPrinter;
                                           default:
                                               break ;
                                       }
                                       return defaultTextPrinter;
                                   }(type), {}, {}});
    }
    void HelpLayout::addUserHelpTextItem(const Text &text, const Output &out) {
        Q_D(HelpLayout);
        d->itemDataList.push_back({HelpLayoutPrivate::UserHelpText, 0, out, text, {}});
    }
    void HelpLayout::addUserHelpListItem(const List &list, const Output &out) {
        Q_D(HelpLayout);
        d->itemDataList.push_back({HelpLayoutPrivate::UserHelpList, 0, out, {}, list});
    }
    void HelpLayout::addUserHelpPlainItem(const Output &out) {
        Q_D(HelpLayout);
        d->itemDataList.push_back({HelpLayoutPrivate::UserHelpPlain, 0, out, {}, {}});
    }

    HelpLayout HelpLayout::defaultHelpLayout() {
        HelpLayout res;
        res.addHelpTextItem(HT_Prologue);
        res.addMessageItem(MI_Information);
        res.addMessageItem(MI_Warning);
        res.addMessageItem(MI_Critical);
        res.addHelpTextItem(HT_Description);
        res.addHelpTextItem(HT_Usage);
        res.addHelpListItem(HL_Arguments);
        res.addHelpListItem(HL_Options);
        res.addHelpListItem(HL_Commands);
        res.addHelpTextItem(HT_Epilogue);
        return res;
    }

}
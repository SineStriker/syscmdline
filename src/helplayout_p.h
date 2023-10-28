#ifndef HELPLAYOUT_P_H
#define HELPLAYOUT_P_H

#include <utility>

#include "sharedbase_p.h"
#include "helplayout.h"

namespace SysCmdLine {

    //    struct OutputItemData {
    //        enum Type {
    //            Help,
    //            Message,
    //            UserText,
    //            UserList,
    //            Plain,
    //        };
    //
    //        Type t;
    //
    //        explicit OutputItemData(Type t) : t(t) {
    //        }
    //        virtual ~OutputItemData() = default;
    //
    //        virtual OutputItemData *clone() const = 0;
    //    };
    //
    //    struct HelpItemData : public OutputItemData {
    //        explicit HelpItemData(HelpLayout::HelpItem helpItem)
    //            : OutputItemData(Help), helpItem(helpItem) {
    //        }
    //
    //        HelpLayout::HelpItem helpItem;
    //
    //        OutputItemData *clone() const {
    //            return new HelpItemData(*this);
    //        }
    //    };
    //
    //    struct MessageItemData : public OutputItemData {
    //        explicit MessageItemData(HelpLayout::MessageItem messageItem)
    //            : OutputItemData(Message), messageItem(messageItem) {
    //        }
    //
    //        HelpLayout::MessageItem messageItem;
    //
    //        OutputItemData *clone() const {
    //            return new MessageItemData(*this);
    //        }
    //    };
    //
    //    struct UserTextItemData : public OutputItemData {
    //        explicit UserTextItemData(HelpLayout::Text text, HelpLayout::TextOutput out)
    //            : OutputItemData(UserText), text(std::move(text)), out(std::move(out)) {
    //        }
    //
    //        HelpLayout::Text text;
    //        HelpLayout::TextOutput out;
    //
    //        OutputItemData *clone() const {
    //            return new UserTextItemData(*this);
    //        }
    //    };
    //
    //    struct UserListItemData : public OutputItemData {
    //        explicit UserListItemData(HelpLayout::List list, HelpLayout::ListOptput out)
    //            : OutputItemData(UserList), list(std::move(list)), out(std::move(out)) {
    //        }
    //
    //        HelpLayout::List list;
    //        HelpLayout::ListOptput out;
    //
    //        OutputItemData *clone() const {
    //            return new UserListItemData(*this);
    //        }
    //    };
    //
    //    struct PlainItemData : public OutputItemData {
    //        explicit PlainItemData(HelpLayout::PlainOutput out)
    //            : OutputItemData(Plain), out(std::move(out)) {
    //        }
    //
    //        HelpLayout::PlainOutput out;
    //
    //        OutputItemData *clone() const {
    //            return new PlainItemData(*this);
    //        }
    //    };
    //
    //    struct OutputItemDataList {
    //        OutputItemData **data;
    //        int size;
    //        int capacity;
    //
    //        OutputItemDataList();
    //        OutputItemDataList(const OutputItemDataList &other);
    //        ~OutputItemDataList();
    //
    //        void addItem(OutputItemData *item);
    //    };

    class HelpLayoutPrivate : public SharedBasePrivate {
    public:
        HelpLayoutPrivate *clone() const {
            return new HelpLayoutPrivate(*this);
        }
    };

}

#endif // HELPLAYOUT_P_H

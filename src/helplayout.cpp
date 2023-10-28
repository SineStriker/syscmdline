#include "helplayout.h"
#include "helplayout_p.h"

namespace SysCmdLine {

    //    OutputItemDataList::OutputItemDataList() : data(nullptr), size(0), capacity(0) {
    //    }
    //
    //    OutputItemDataList::OutputItemDataList(const OutputItemDataList &other)
    //        : data(nullptr), size(0), capacity(0) {
    //        if (other.size == 0) {
    //            return;
    //        }
    //
    //        data = new OutputItemData *[other.capacity];
    //        size = other.size;
    //        capacity = other.capacity;
    //        for (int i = 0; i < size; ++i) {
    //            data[i] = other.data[i]->clone();
    //        }
    //    }
    //
    //    OutputItemDataList::~OutputItemDataList() {
    //        for (int i = 0; i < size; ++i) {
    //            delete data[i];
    //        }
    //        delete[] data;
    //    }
    //
    //    void OutputItemDataList::addItem(SysCmdLine::OutputItemData *item) {
    //        if (size == capacity) {
    //            int new_capacity = capacity == 0 ? 10 : capacity * 2;
    //            auto new_data = new OutputItemData *[new_capacity];
    //            for (int i = 0; i < size; ++i) {
    //                new_data[i] = data[i];
    //            }
    //            delete data;
    //            data = new_data;
    //            capacity = new_capacity;
    //        }
    //        data[size] = item;
    //        size++;
    //    }

    HelpLayout::HelpLayout() : SharedBase(new HelpLayoutPrivate()) {
    }

    HelpLayout::~HelpLayout() {
    }

    void HelpLayout::addHelpTextItem(HelpLayout::HelpTextItem type,
                                     const HelpLayout::TextOutput &out) {
    }
    void HelpLayout::addHelpListItem(HelpLayout::HelpListItem type,
                                     const HelpLayout::ListOptput &out) {
    }
    void HelpLayout::addMessageItem(HelpLayout::MessageItem type,
                                    const HelpLayout::TextOutput &out) {
    }
    void HelpLayout::addUserHelpTextItem(const HelpLayout::Text &text,
                                         const HelpLayout::TextOutput &out) {
    }
    void HelpLayout::addUserHelpListItem(const HelpLayout::List &list,
                                         const HelpLayout::ListOptput &out) {
    }
    void HelpLayout::addUserHelpPlainItem(const HelpLayout::PlainOutput &out) {
    }

    HelpLayout HelpLayout::defaultHelpLayout() {
        return HelpLayout();
    }

}
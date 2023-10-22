#include "helplayout.h"
#include "helplayout_p.h"

#include "system.h"

namespace SysCmdLine {

    const HelpLayout &HelpLayoutData::defaultHelpLayout() {
        static HelpLayout hl;
        return hl;
    }

    HelpLayout::HelpLayout() {
    }

    HelpLayout::~HelpLayout() {
    }
    
    HelpLayout::HelpLayout(const HelpLayout &other) {
        d_ptr = other.d_ptr;
    }

    HelpLayout::HelpLayout(HelpLayout &&other) noexcept {
        d_ptr.swap(other.d_ptr);
    }

    HelpLayout &HelpLayout::operator=(const HelpLayout &other) {
        if (this == &other) {
            return *this;
        }
        d_ptr = other.d_ptr;
        return *this;
    }

    HelpLayout &HelpLayout::operator=(HelpLayout &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        d_ptr.swap(other.d_ptr);
        return *this;
    }

    bool HelpLayout::isNull() const {
        SYSCMDLINE_GET_DATA(const HelpLayout);
        return d->layoutItems.empty();
    }

    int HelpLayout::size(HelpLayout::SizeType sizeType) const {
        SYSCMDLINE_GET_DATA(const HelpLayout);
        return d->sizeConfig[sizeType];
    }

    void HelpLayout::setSize(HelpLayout::SizeType sizeType, int value) {
        SYSCMDLINE_GET_DATA(HelpLayout);
        d->sizeConfig[sizeType] = value;
    }

    void HelpLayout::addText(const std::string &text, MessageType messageType) {
        addItem(HI_CustomText, [text](const std::vector<std::string> &) {
            u8printf("%s\n", text.data()); //
        });
    }

    void HelpLayout::addItem(HelpLayout::HelpItem type, const HelpLayout::Printer &printer) {
        SYSCMDLINE_GET_DATA(HelpLayout);
        d->layoutItems.push_back({type, printer});
    }

}
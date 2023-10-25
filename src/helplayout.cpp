#include "helplayout.h"
#include "helplayout_p.h"

namespace SysCmdLine {

    HelpLayout::HelpLayout() : d_ptr(new HelpLayoutData()) {
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

    void HelpLayout::addItem(HelpLayout::HelpItem type, const HelpLayout::Printer &printer) {
        SYSCMDLINE_GET_DATA(HelpLayout);
        d->layoutItems.push_back({type, printer});
    }

    static HelpLayout buildDefaultHelpLayout() {
        HelpLayout layout;
        layout.addItem(HelpLayout::HI_Prologue);
        layout.addItem(HelpLayout::HI_Information);
        layout.addItem(HelpLayout::HI_Warning);
        layout.addItem(HelpLayout::HI_Error);
        layout.addItem(HelpLayout::HI_Description);
        layout.addItem(HelpLayout::HI_Usage);
        layout.addItem(HelpLayout::HI_Arguments);
        layout.addItem(HelpLayout::HI_Options);
        layout.addItem(HelpLayout::HI_Commands);
        layout.addItem(HelpLayout::HI_Epilogue);
        return layout;
    }

    HelpLayout HelpLayout::defaultHelpLayout() {
        static HelpLayout hl = buildDefaultHelpLayout();
        return hl;
    }

}
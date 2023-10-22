#include "helplayout.h"

namespace SysCmdLine {

    class HelpLayoutData : public SharedData {
    public:
        HelpLayoutData() {
        }
        ~HelpLayoutData() {
        }

        HelpLayoutData *clone() const {
            return new HelpLayoutData(*this);
        }
    };

    HelpLayout::HelpLayout() {
    }

    HelpLayout::~HelpLayout() {
    }

    int HelpLayout::size(HelpLayout::SizeType sizeType) const {
        return 0;
    }

    void HelpLayout::setSize(HelpLayout::SizeType sizeType, int value) {
    }

    void HelpLayout::add(const std::string &s) {
    }

    void HelpLayout::add(HelpLayout::HelpItem type, const HelpLayout::LayoutBuilder &builder) {
    }

}
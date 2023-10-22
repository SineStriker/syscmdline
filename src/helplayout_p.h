#ifndef HELPLAYOUT_P_H
#define HELPLAYOUT_P_H

#include "helplayout.h"

namespace SysCmdLine {

    class HelpLayoutData : public SharedData {
    public:
        HelpLayoutData() {
        }
        ~HelpLayoutData() {
        }

        struct LayoutItem {
            HelpLayout::HelpItem item;
            HelpLayout::Printer printer;
        };
        std::vector<LayoutItem> layoutItems;

        int sizeConfig[3] = {
            4,
            4,
            80,
        };

        HelpLayoutData *clone() const {
            return new HelpLayoutData(*this);
        }
    };

}

#endif // HELPLAYOUT_P_H

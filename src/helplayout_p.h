#ifndef HELPLAYOUT_P_H
#define HELPLAYOUT_P_H

#include "sharedbase_p.h"
#include "helplayout.h"

namespace SysCmdLine {

    class HelpLayoutPrivate : public SharedBasePrivate {
    public:
        HelpLayoutPrivate() {
        }

        ~HelpLayoutPrivate() {
        }
       
        HelpLayoutPrivate *clone() const {
            return new HelpLayoutPrivate(*this);
        }
    };

}

#endif // HELPLAYOUT_P_H

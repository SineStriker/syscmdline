#ifndef OPTION_P_H
#define OPTION_P_H

#include "argument_p.h"
#include "option.h"

namespace SysCmdLine {

    class OptionPrivate : public ArgumentHolderPrivate {
    public:
        OptionPrivate(Option::SpecialType specialType, const std::vector<std::string> &tokens,
                      const std::string &desc, bool required);

        SymbolPrivate *clone() const override;

    public:
        Option::SpecialType specialType;
        std::vector<std::string> tokens;
        bool required;
        Option::ShortMatchRule shortMatchRule;
        Option::PriorLevel priorLevel;
        bool global;
        int maxOccurrence;
    };

}

#endif // OPTION_P_H

#ifndef OPTION_P_H
#define OPTION_P_H

#include "argument_p.h"
#include "option.h"

namespace SysCmdLine {

    class OptionData : public ArgumentHolderData {
    public:
        OptionData(const std::string &name, const std::string &desc,
                   const std::vector<std::string> &tokens, const std::vector<Argument> &args,
                   bool is_short, bool prior, bool global);
        ~OptionData();

        SymbolData *clone() const override;

    public:
        void setTokens(const std::vector<std::string> &tokens);

        std::vector<std::string> tokens;
        bool is_short;
        bool prior;
        bool global;
    };

}

#endif // OPTION_P_H

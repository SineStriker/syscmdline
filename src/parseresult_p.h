#ifndef PARSERESULT_P_H
#define PARSERESULT_P_H

#include "sharedbase_p.h"
#include "parser.h"

#include "map_p.h"

namespace SysCmdLine {

    struct ArgumentHolderData {
        int optionalArgIndex;
        int multiValueArgIndex;
        StringMap argNameIndexes; // name -> index of `argResult`
        int argSize;              // equal to `argumentCount()`
    };

    struct OptionData : public ArgumentHolderData {
        const Option *option;           // pointer to the current option
        std::vector<Value> **argResult; // arg result
        int count;                      // arg result count

        ~OptionData() {
            for (int i = 0; i < count; ++i) {
                delete[] argResult[i];
            }
            delete[] argResult;
        }
    };

    struct ParseResultData2 : public ArgumentHolderData {
        std::vector<Value> *argResult;   // arg result

        OptionData *allOptionsResult;    // option result
        int allOptionsSize;              // command option count + global option count
        int globalOptionsSize;           // global option count
        StringMap allOptionTokenIndexes; // token -> index of `allOptionsResult`

        ~ParseResultData2() {
            delete[] allOptionsResult;
            delete[] argResult;
        }
    };

    class ParseResultPrivate : public SharedBasePrivate {
    public:
        // This is a read-only class, we can store the pointers pointing to
        // its own data, and we don't need to implement the clone method
        SharedBasePrivate *clone() const {
            return nullptr;
        }

        // parse information
        Parser parser;
        std::vector<std::string> arguments;

        // error related
        ParseResult::Error error = ParseResult::NoError;
        std::vector<std::string> errorPlaceholders;
        const Option *errorOption = nullptr;
        const Argument *errorArgument = nullptr;
        std::string cancellationToken;

        // success results
        std::vector<int> stack;
        const Command *command;

        ParseResultData2 core;

        bool versionSet;
        bool helpSet;

        std::string correctionText() const;
        void showMessage(const std::string &info, const std::string &warn, const std::string &err,
                         bool noHelp = false) const;
    };

}

#endif // PARSERESULT_P_H

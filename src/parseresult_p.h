#ifndef PARSERESULT_P_H
#define PARSERESULT_P_H

#include "sharedbase_p.h"
#include "parser.h"

#include "map_p.h"

namespace SysCmdLine {

    struct ParseResultData2 {
        struct OptionData {
            const Option *option;           // pointer to the current option
            std::vector<Value> **argResult; // arg result
            int count;                      // arg result count
            int argSize;                    // equal to `option->argumentCount()`
            int optionalArgIndex;
            int multiValueArgIndex;
            StringMap argNameIndexes; // name -> index of `argResult`

            ~OptionData() {
                for (int i = 0; i < count; ++i) {
                    delete[] argResult[i];
                }
                delete[] argResult;
            }
        };

        OptionData *allOptionsResult;    // option result
        int allOptionsSize;              // command option count + global option count
        int globalOptionsSize;           // global option count
        StringMap allOptionTokenIndexes; // token -> index of `allOptionsResult`

        std::vector<Value> *argResult; // arg result
        int optionalArgIndex;
        int multiValueArgIndex;
        StringMap argNameIndexes; // name -> index of `argResult`

        ~ParseResultData2() {
            delete[] allOptionsResult;
            delete[] argResult;
        }
    };

    class ParseResultPrivate : public SharedBasePrivate {
    public:
        ParseResultPrivate();

        // This is a read-only class, we can store the pointers pointing to
        // its own data, and we don't need to implement the clone method
        SharedBasePrivate *clone() const {
            return nullptr;
        }

        // parse information
        Parser parser;
        std::vector<std::string> arguments;

        // error related
        ParseResult::Error error;
        std::vector<std::string> errorPlaceholders;
        const Option *errorOption;
        const Argument *errorArgument;

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

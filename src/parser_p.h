#ifndef PARSER_P_H
#define PARSER_P_H

#include "parser.h"

namespace SysCmdLine {

    class ParserData : public SharedData {
    public:
        Command rootCommand;
        std::string texts[2];
        int displayOptions;

        ParserData() : displayOptions(Parser::Normal) {
        }

        ~ParserData() {
        }

        ParserData *clone() const {
            return new ParserData(*this);
        }
    };

    class ParseResultData : public SharedData {
    public:
        using ArgResult = std::unordered_map<std::string, Value>;

        SharedDataPointer<ParserData> parserData;
        std::vector<std::string> arguments;

        ParseResult::Error error;
        std::vector<std::string> errorPlaceholders;
        std::vector<int> stack;
        std::vector<const Option *> globalOptions;

        const Command *command;
        std::unordered_map<std::string, std::vector<Value>> argResult;
        std::unordered_map<std::string, std::vector<ArgResult>> optResult;

        bool versionSet;
        bool helpSet;

        ParseResultData(const SharedDataPointer<ParserData> &parserData,
                        const std::vector<std::string> &args)
            : parserData(parserData), arguments(args), error(ParseResult::NoError),
              command(nullptr), versionSet(false), helpSet(false) {
        }

        ~ParseResultData() {
        }

        ParseResultData *clone() const {
            return new ParseResultData(*this);
        }

        std::string correctionText() const;
        static Value getDefaultResult(const ArgumentHolder *argumentHolder,
                                      const std::string &argName);
        Value getDefaultResult(const std::string &optName, const std::string &argName) const;
        void showHelp(const std::function<void()> &messageCaller = {}) const;
    };

}

#endif // PARSER_P_H

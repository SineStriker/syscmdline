#include "parser.h"

#include <stdexcept>

#include "strings.h"
#include "system.h"

namespace SysCmdLine {

    class ParseResult {
    public:
        Parser::Error error;
        std::vector<std::string> errorPlaceholders;
        std::vector<int> stack;
        std::vector<const Option *> globalOptions;
        const Command *command;

        bool versionSet;
        bool helpSet;

        ParseResult() : error(Parser::NoError), versionSet(false), helpSet(false) {
        }
    };

    class ParserPrivate {
    public:
        Command rootCommand;
        std::string texts[2];
        ParseResult *result;

        ParserPrivate() : result(nullptr) {
        }

        ~ParserPrivate() {
            delete result;
        }

        void parse(const std::vector<std::string> &args) {
            delete result;
            result = new ParseResult();
        }

        void checkResult() const {
            if (!result)
                throw std::runtime_error("no valid parse result");
        }

        static std::string formatText(const std::string &fmt,
                                      const std::vector<std::string> &placeHolders) {
            std::string res;
            std::string::size_type start = 0;
            for (const auto &item : placeHolders) {
                std::string::size_type idx = fmt.find("{}");
                if (idx == std::string::npos) {
                    break;
                }
                res.append(fmt.substr(start, idx - start));
                start = idx + 2;
            }

            if (start < fmt.size()) {
                res.append(fmt.substr(start));
            }
            return res;
        }
    };

    Parser::Parser() : d(new ParserPrivate()) {
    }

    Parser::Parser(const Command &rootCommand) : d(new ParserPrivate()) {
        setRootCommand(rootCommand);
    }

    Parser::~Parser() {
        delete d;
    }

    const Command &Parser::rootCommand() const {
        return d->rootCommand;
    }

    void Parser::setRootCommand(const Command &rootCommand) {
        if (d->result) {
            delete d->result;
            d->result = nullptr;
        }
        d->rootCommand = rootCommand;
    }

    std::string Parser::text(Parser::Side side) const {
        return d->texts[side];
    }

    void Parser::setText(Parser::Side side, const std::string &text) {
        d->texts[side] = text;
    }

    bool Parser::parse(const std::vector<std::string> &args) {
        d->parse(args);
        return d->result->error == NoError;
    }

    int Parser::invoke(const std::vector<std::string> &args) {
        if (!parse(args)) {
            u8printf("%s: %s\n", Strings::common_strings[Strings::Error], errorText().data());
            return -1;
        }

        const auto &cmd = *targetCommand();
        const auto &handler = cmd.handler();

        if (!d->result->versionSet) {
            u8printf("%s", cmd._version.data());
            return 0;
        }

        if (d->result->helpSet) {
            showHelpText();
            return 0;
        }

        if (!handler) {
            throw std::runtime_error("command \"" + cmd.name() + "\" doesn't have a valid handler");
        }
        return handler(*this, cmd);
    }

    bool Parser::parsed() const {
        return d->result;
    }

    Parser::Error Parser::error() const {
        return d->result ? d->result->error : NoError;
    }

    std::string Parser::errorText() const {
        if (!d->result)
            return {};

        if (d->result->error == NoError)
            return {};
        return ParserPrivate::formatText(Strings::error_strings[d->result->error],
                                         d->result->errorPlaceholders);
    }

    const Command *Parser::targetCommand() const {
        d->checkResult();
        return d->result->command;
    }

    const std::vector<const Option *> &Parser::targetGlobalOptions() const {
        d->checkResult();
        return d->result->globalOptions;
    }

    std::vector<std::pair<int, std::string>> Parser::targetStack() const {
        d->checkResult();

        std::vector<std::pair<int, std::string>> res;
        res.reserve(d->result->stack.size());

        const Command *p = &d->rootCommand;
        for (const auto &item : std::as_const(d->result->stack)) {
            p = &p->command(item);
            res.emplace_back(item, p->name());
        }
        return res;
    }

    std::string Parser::value(const Argument &arg) const {
        return std::string();
    }

    int Parser::count(const Option &opt) const {
        return 0;
    }

    std::string Parser::value(const Option &opt, const Argument &arg, int count) {
        return std::string();
    }

    bool Parser::isHelpSet() const {
        return d->result && d->result->helpSet;
    }

    bool Parser::isVersionSet() const {
        return d->result && d->result->versionSet;
    }

    void Parser::showHelpText() const {
        if (!d->result) {
            return;
        }

        std::vector<std::string> parentCommands;
        const Command *p = &d->rootCommand;
        for (const auto &item : std::as_const(d->result->stack)) {
            parentCommands.push_back(p->name());
            p = &p->command(item);
        }

        u8printf("%s", p->helpText(parentCommands, d->result->globalOptions).data());
    }

}
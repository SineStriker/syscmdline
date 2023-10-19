#include "command.h"
#include "command_p.h"

#include <set>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <unordered_set>

#include "option_p.h"
#include "strings.h"

namespace SysCmdLine {

    class CommandCatalogueData : public SharedData {
    public:
        std::vector<std::unordered_set<std::string>> _arg;
        std::vector<std::unordered_set<std::string>> _opt;
        std::vector<std::unordered_set<std::string>> _cmd;

        std::unordered_map<std::string, size_t> _argIndexes;
        std::unordered_map<std::string, size_t> _optIndexes;
        std::unordered_map<std::string, size_t> _cmdIndexes;

        CommandCatalogueData *clone() const {
            auto cc = new CommandCatalogueData();
            cc->_arg = _arg;
            cc->_opt = _opt;
            cc->_cmd = _cmd;
            cc->_argIndexes = _argIndexes;
            cc->_optIndexes = _optIndexes;
            cc->_cmdIndexes = _cmdIndexes;
            return cc;
        }
    };

    CommandCatalogue::CommandCatalogue() : d(new CommandCatalogueData()) {
    }

    CommandCatalogue::~CommandCatalogue() {
    }

    CommandCatalogue::CommandCatalogue(const CommandCatalogue &other) {
        d = other.d;
    }

    CommandCatalogue::CommandCatalogue(CommandCatalogue &&other) noexcept {
        d.swap(other.d);
    }

    CommandCatalogue &CommandCatalogue::operator=(const CommandCatalogue &other) {
        if (this == &other) {
            return *this;
        }
        d = other.d;
        return *this;
    }

    CommandCatalogue &CommandCatalogue::operator=(CommandCatalogue &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        d.swap(other.d);
        return *this;
    }

    void CommandCatalogue::addArgumentCategory(const std::string &name,
                                               const std::vector<std::string> &args) {
        size_t index;
        auto it = d->_argIndexes.find(name);
        if (it == d->_argIndexes.end()) {
            index = d->_arg.size();
            d->_arg.emplace_back(args.begin(), args.end());
            d->_argIndexes.insert(std::make_pair(name, index));
        } else {
            index = it->second;
            d->_arg[index].insert(args.begin(), args.end());
        }
    }

    void CommandCatalogue::addOptionCategory(const std::string &name,
                                             const std::vector<std::string> &options) {
        size_t index;
        auto it = d->_optIndexes.find(name);
        if (it == d->_optIndexes.end()) {
            index = d->_opt.size();
            d->_opt.emplace_back(options.begin(), options.end());
            d->_optIndexes.insert(std::make_pair(name, index));
        } else {
            index = it->second;
            d->_opt[index].insert(options.begin(), options.end());
        }
    }

    void CommandCatalogue::addCommandCatalogue(const std::string &name,
                                               const std::vector<std::string> &commands) {
        size_t index;
        auto it = d->_cmdIndexes.find(name);
        if (it == d->_cmdIndexes.end()) {
            index = d->_cmd.size();
            d->_cmd.emplace_back(commands.begin(), commands.end());
            d->_cmdIndexes.insert(std::make_pair(name, index));
        } else {
            index = it->second;
            d->_cmd[index].insert(commands.begin(), commands.end());
        }
    }

    CommandData::CommandData(const std::string &name, const std::string &desc,
                             const std::vector<Option> &options,
                             const std::vector<Command> &subCommands,
                             const std::vector<Argument> &args, const std::string &version,
                             const std::string &detailedDescription, bool showHelpIfNoArg,
                             const Command::Handler &handler, const CommandCatalogue &catalogue)
        : ArgumentHolderData(Symbol::ST_Command, name, desc, args), version(version),
          detailedDescription(detailedDescription), showHelpIfNoArg(showHelpIfNoArg),
          handler(handler), catalogue(catalogue) {
        if (!options.empty())
            setOptions(options);
        if (!subCommands.empty())
            setCommands(subCommands);
    }
    CommandData::~CommandData() {
    }

    SymbolData *CommandData::clone() const {
        return new CommandData(name, desc, options, subCommands, arguments, version,
                               detailedDescription, showHelpIfNoArg, handler, catalogue);
    }

    void CommandData::setCommands(const std::vector<Command> &commands) {
        subCommands.clear();
        subCommandNameIndexes.clear();
        if (commands.empty())
            return;

        subCommands.reserve(commands.size());
        subCommandNameIndexes.reserve(commands.size());
        for (const auto &cmd : commands) {
            addCommand(cmd);
        }
    }

    void CommandData::setOptions(const std::vector<Option> &opts) {
        options.clear();
        optionNameIndexes.clear();
        optionTokenIndexes.clear();
        if (opts.empty())
            return;

        options.reserve(opts.size());
        optionNameIndexes.reserve(opts.size());
        for (const auto &opt : opts) {
            addOption(opt);
        }
    }

    void CommandData::addCommand(const Command &command) {
        const auto &name = command.name();
        if (name.empty()) {
            throw std::runtime_error("empty command name");
        }
        if (subCommandNameIndexes.count(name)) {
            throw std::runtime_error("command name \"" + name + "\" duplicated");
        }
        subCommandNameIndexes.insert(std::make_pair(name, subCommands.size()));
        subCommands.push_back(command);
    }

    void CommandData::addOption(const Option &option) {
        const auto &name = option.name();
        if (name.empty()) {
            throw std::runtime_error("null option name");
        }
        if (name == "-" || name == "--") {
            throw std::runtime_error("invalid option name \"" + name + "\"");
        }
        if (optionNameIndexes.count(name)) {
            throw std::runtime_error("option name \"" + name + "\" duplicated");
        }

        Option newOption = option;
        if (std::as_const(newOption).d_func()->tokens.empty()) {
            if (name.front() == '-') {
                newOption.setToken(name);
            } else {
                if (name.size() == 1) {
                    newOption.setToken("-" + name);
                } else {
                    newOption.setToken("--" + name);
                }
            }
        }

        const auto &d = newOption.d_func();
        for (const auto &token : d->tokens) {
            if (optionTokenIndexes.count(token)) {
                throw std::runtime_error("option token \"" + token + "\" duplicated");
            }
        }

        auto last = options.size();
        optionNameIndexes.insert(std::make_pair(name, last));
        options.push_back(newOption);
        for (const auto &token : d->tokens) {
            optionTokenIndexes.insert(std::make_pair(token, last));
        }
    }

    Command::Command() : Command({}, {}) {
    }

    Command::Command(const std::string &name, const std::string &desc,
                     const std::vector<Option> &options, const std::vector<Command> &subCommands,
                     const std::vector<Argument> &args, const std::string &detailedDescription,
                     const Command::Handler &handler)
        : ArgumentHolder(new CommandData(name, desc, options, subCommands, args, {},
                                         detailedDescription, false, handler, {})) {
    }

    Command::~Command() {
    }

    Command::Command(const Command &other) : ArgumentHolder(nullptr) {
        d_ptr = other.d_ptr;
    }

    Command::Command(Command &&other) noexcept : ArgumentHolder(nullptr) {
        d_ptr.swap(other.d_ptr);
    }

    Command &Command::operator=(const Command &other) {
        if (this == &other) {
            return *this;
        }
        d_ptr = other.d_ptr;
        return *this;
    }

    Command &Command::operator=(Command &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        d_ptr.swap(other.d_ptr);
        return *this;
    }

    Command Command::command(const std::string &name) const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        auto it = d->subCommandNameIndexes.find(name);
        if (it == d->subCommandNameIndexes.end())
            return {};
        return d->subCommands[it->second];
    }

    Command Command::command(int index) const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        if (index < 0 || index >= d->subCommands.size())
            return {};
        return d->subCommands[index];
    }

    const std::vector<Command> &Command::commands() const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        return d->subCommands;
    }

    int Command::indexOfCommand(const std::string &name) const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        auto it = d->subCommandNameIndexes.find(name);
        if (it == d->subCommandNameIndexes.end())
            return -1;
        return it->second;
    }

    bool Command::hasCommand(const std::string &name) const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        return d->subCommandNameIndexes.count(name);
    }

    void Command::addCommand(const Command &command) {
        SYSCMDLINE_GET_DATA(Command);
        d->addCommand(command);
    }

    void Command::setCommands(const std::vector<Command> &commands) {
        SYSCMDLINE_GET_DATA(Command);
        d->setCommands(commands);
    }

    Option Command::option(const std::string &name) const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        auto it = d->optionNameIndexes.find(name);
        if (it == d->optionNameIndexes.end())
            return {};
        return d->options[it->second];
    }

    Option Command::option(int index) const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        if (index < 0 || index >= d->options.size())
            return {};
        return d->options[index];
    }

    Option Command::optionFromToken(const std::string &token) const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        auto it = d->optionTokenIndexes.find(token);
        if (it == d->optionTokenIndexes.end())
            return {};
        return d->options[it->second];
    }

    int Command::indexOfOption(const std::string &name) const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        auto it = d->optionNameIndexes.find(name);
        if (it == d->optionNameIndexes.end())
            return -1;
        return it->second;
    }

    bool Command::hasOption(const std::string &name) const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        return d->optionTokenIndexes.count(name);
    }

    bool Command::hasOptionToken(const std::string &token) const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        return d->optionTokenIndexes.count(token);
    }

    const std::vector<Option> &Command::options() const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        return d->options;
    }

    void Command::addOption(const Option &option) {
        SYSCMDLINE_GET_DATA(Command);
        d->addOption(option);
    }

    void Command::setOptions(const std::vector<Option> &options) {
        SYSCMDLINE_GET_DATA(Command);
        d->setOptions(options);
    }

    std::string Command::detailedDescription() const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        return d->detailedDescription;
    }

    void Command::setDetailedDescription(const std::string &detailedDescription) {
        if (detailedDescription == this->detailedDescription())
            return;

        SYSCMDLINE_GET_DATA(Command);
        d->detailedDescription = detailedDescription;
    }

    Command::Handler Command::handler() const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        return d->handler;
    }

    void Command::setHandler(const Command::Handler &handler) {
        SYSCMDLINE_GET_DATA(Command);
        d->handler = handler;
    }

    CommandCatalogue Command::catalogue() const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        return d->catalogue;
    }

    void Command::setCatalogue(const CommandCatalogue &catalogue) {
        SYSCMDLINE_GET_DATA(Command);
        d->catalogue = catalogue;
    }

    std::string Command::version() const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        return d->version;
    }

    void Command::addVersionOption(const std::string &ver, const std::vector<std::string> &tokens) {
        SYSCMDLINE_GET_DATA(Command);
        d->version = ver;
        addOption(Option("version", Strings::info_strings[Strings::Version],
                         tokens.empty() ? std::vector<std::string>{"-v", "--version"} : tokens,
                         false, false, Option::IgnoreMissingArgument, false));
    }

    void Command::addHelpOption(bool showHelpIfNoArg, bool global,
                                const std::vector<std::string> &tokens) {
        SYSCMDLINE_GET_DATA(Command);
        addOption(Option("help", Strings::info_strings[Strings::Help],
                         tokens.empty() ? std::vector<std::string>{"-h", "--help"} : tokens, false,
                         false, Option::IgnoreMissingArgument, global));
        d->showHelpIfNoArg = showHelpIfNoArg;
    }

    static void listItems(std::stringstream &ss, const std::string &title,
                          const std::vector<std::pair<std::string, std::string>> &contents) {
        if (contents.empty())
            return;

        ss << std::endl;

        size_t widest = 0;
        for (const auto &item : contents) {
            widest = std::max(item.first.size(), widest);
        }

        ss << title << ": " << std::endl;
        for (const auto &item : contents) {
            ss << Strings::INDENT << std::left << std::setw(widest) << item.first << Strings::INDENT
               << item.second << std::endl;
        }
    }

    template <class T, class F1, class F2>
    static void collectItems(std::stringstream &ss,
                             const std::vector<std::unordered_set<std::string>> &catalogue,
                             const std::unordered_map<std::string, size_t> &catalogueIndexes,
                             const std::vector<T> &items,
                             const std::unordered_map<std::string, size_t> &itemIndexes,
                             const std::string &defaultTitle, F1 f1, F2 f2) {

        auto indexes = itemIndexes;
        for (const auto &pair : catalogueIndexes) {
            std::set<size_t> subscriptSet;
            for (const auto &name : catalogue[pair.second]) {
                auto it = indexes.find(name);
                if (it == indexes.end())
                    continue;
                subscriptSet.insert(it->second);
                indexes.erase(it);
            }

            std::vector<std::pair<std::string, std::string>> texts;
            for (const auto &subscript : std::as_const(subscriptSet)) {
                const auto &item = items[subscript];
                texts.emplace_back(f1(item), f2(item));
            }
            listItems(ss, pair.first, texts);
        }

        {
            std::set<size_t> subscriptSet;
            for (const auto &pair : std::as_const(indexes)) {
                subscriptSet.insert(pair.second);
            }

            std::vector<std::pair<std::string, std::string>> texts;
            texts.reserve(subscriptSet.size());
            for (const auto &subscript : std::as_const(subscriptSet)) {
                const auto &item = items[subscript];
                texts.emplace_back(f1(item), f2(item));
            }
            listItems(ss, defaultTitle, texts);
        }
    }

    std::string Command::helpText(const std::vector<std::string> &parentCommands,
                                  const std::vector<const Option *> &globalOptions) const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        const auto &dd = d->catalogue.d.constData();

        // Build option indexes
        auto options = d->options;
        auto optionNameIndexes = d->optionNameIndexes;

        options.reserve(options.size() + globalOptions.size());
        optionNameIndexes.reserve(optionNameIndexes.size() + globalOptions.size());
        for (const auto &item : globalOptions) {
            optionNameIndexes.insert(std::make_pair(item->name(), options.size()));
            options.push_back(*item);
        }

        std::stringstream ss;

        // Description
        const auto &desc = d->detailedDescription.empty() ? d->desc : d->detailedDescription;
        if (!desc.empty()) {
            ss << Strings::common_strings[Strings::Description] << ": " << std::endl;
            ss << Strings::INDENT << desc << std::endl;
            ss << std::endl;
        }

        // Usage
        ss << Strings::common_strings[Strings::Usage] << ": " << std::endl;
        {
            ss << Strings::INDENT;
            for (const auto &item : parentCommands) {
                ss << item << " ";
            }

            // name
            ss << d->name;

            // arguments
            if (!d->arguments.empty()) {
                ss << " " << displayedArguments();
            }

            // required options
            size_t requiredCount = 0;
            for (const auto &opt : options) {
                if (!opt.isRequired()) {
                    continue;
                }
                requiredCount++;
                ss << " " << opt.displayedText(false);
            }

            // command
            if (!d->subCommands.empty()) {
                ss << " [commands]";
            }

            // options
            if (requiredCount < options.size() || !d->subCommands.empty()) {
                ss << " [options]";
            }

            ss << std::endl;
        }

        // Arguments
        if (!d->arguments.empty()) {
            collectItems(
                ss, dd->_arg, dd->_argIndexes, d->arguments, d->argumentNameIndexes,
                Strings::common_strings[Strings::Arguments],
                [](const Argument &arg) { return arg.name(); },
                [](const Argument &arg) { return arg.description(); });
        }

        // Options
        if (!options.empty()) {
            collectItems(
                ss, dd->_opt, dd->_optIndexes, options, optionNameIndexes,
                Strings::common_strings[Strings::Options],
                [](const Option &opt) { return opt.displayedText(); },
                [](const Option &opt) { return opt.description(); });
        }

        // Commands
        if (!d->subCommands.empty()) {
            collectItems(
                ss, dd->_cmd, dd->_cmdIndexes, d->subCommands, d->subCommandNameIndexes,
                Strings::common_strings[Strings::Commands],
                [](const Command &cmd) { return cmd.name(); },
                [](const Command &cmd) { return cmd.description(); });
        }

        return ss.str();
    }

    CommandData *Command::d_func() {
        SYSCMDLINE_GET_DATA(Command);
        return d;
    }

    const CommandData *Command::d_func() const {
        SYSCMDLINE_GET_CONST_DATA(Command);
        return d;
    }

}
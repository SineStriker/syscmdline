#ifndef COMMAND_P_H
#define COMMAND_P_H

#include <unordered_set>

#include "argument_p.h"
#include "command.h"

namespace SysCmdLine {

    class CommandCatalogueData : public SharedData {
    public:
        std::vector<std::unordered_set<std::string>> _arg;
        std::vector<std::unordered_set<std::string>> _opt;
        std::vector<std::unordered_set<std::string>> _cmd;

        std::unordered_map<std::string, size_t> _argIndexes;
        std::unordered_map<std::string, size_t> _optIndexes;
        std::unordered_map<std::string, size_t> _cmdIndexes;

        CommandCatalogueData *clone() const;
    };

    class CommandData : public ArgumentHolderData {
    public:
        CommandData(const std::string &name, const std::string &desc,
                    const std::vector<std::pair<Option, int>> &options,
                    const std::vector<Command> &subCommands, const std::vector<Argument> &args,
                    const std::string &version, const std::string &detailedDescription,
                    bool showHelpIfNoArg, const Command::Handler &handler,
                    const CommandCatalogue &catalogue);
        ~CommandData();

        SymbolData *clone() const override;

    public:
        void setCommands(const std::vector<Command> &commands);
        void setOptions(const std::vector<Option> &opts);
        void setOptions(const std::vector<std::pair<Option, int>> &opts);

        void addCommand(const Command &command);
        void addOption(const Option &option, int exclusiveGroup = -1);

        std::vector<Option> options;
        std::unordered_map<std::string, size_t> optionNameIndexes;
        std::unordered_map<std::string, size_t> optionTokenIndexes;
        std::unordered_map<int, std::vector<size_t>> exclusiveGroups;
        std::unordered_map<std::string, int> exclusiveGroupIndexes;

        std::vector<Command> subCommands;
        std::unordered_map<std::string, size_t> subCommandNameIndexes;

        std::string version;
        std::string detailedDescription;
        bool showHelpIfNoArg;
        Command::Handler handler;

        CommandCatalogue catalogue;
    };

}

#endif // COMMAND_P_H

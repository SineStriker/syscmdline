#ifndef COMMAND_P_H
#define COMMAND_P_H

#include "argument_p.h"
#include "command.h"

namespace SysCmdLine {

    class CommandData : public ArgumentHolderData {
    public:
        CommandData(const std::string &name, const std::string &desc,
                    const std::vector<Option> &options, const std::vector<Command> &subCommands,
                    const std::vector<Argument> &args, const std::string &version,
                    const std::string &detailedDescription, bool showHelpIfNoArg,
                    bool multipleArguments, const Command::Handler &handler,
                    const CommandCatalogue &catalogue);
        ~CommandData();

        SymbolData *clone() const override;

    public:
        void setCommands(const std::vector<Command> &commands);
        void setOptions(const std::vector<Option> &opts);

        void addCommand(const Command &command);
        void addOption(const Option &option);

        std::vector<Option> options;
        std::unordered_map<std::string, size_t> optionNameIndexes;
        std::unordered_map<std::string, size_t> optionTokenIndexes;
        std::vector<Command> subCommands;
        std::unordered_map<std::string, size_t> subCommandNameIndexes;
        std::string version;
        std::string detailedDescription;
        bool showHelpIfNoArg;
        bool multipleArguments;
        Command::Handler handler;

        CommandCatalogue catalogue;
    };

}

#endif // COMMAND_P_H

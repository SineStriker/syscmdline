#ifndef COMMAND_P_H
#define COMMAND_P_H

#include "argument_p.h"
#include "command.h"

#include "map_p.h"

namespace SysCmdLine {

    class CommandCataloguePrivate : public SharedBasePrivate {
    public:
        CommandCataloguePrivate();
        CommandCataloguePrivate(const CommandCataloguePrivate &other);
        ~CommandCataloguePrivate();

        SharedBasePrivate *clone() const;

        StringMap arg;
        StringMap opt;
        StringMap cmd;

        StringList arguments;
        StringList options;
        StringList commands;
    };

    class CommandPrivate : public ArgumentHolderPrivate {
    public:
        CommandPrivate(std::string name, const std::string &desc);

        SharedBasePrivate *clone() const override;

    public:
        std::string name;

        std::vector<Option> options;
        std::vector<std::string> optionGroupNames;
        std::vector<Command> commands;

        Option helpOption;
        Option versionOption;

        std::string version;
        std::string detailedDescription;
        CommandCatalogue catalogue;

        Command::Handler handler;

        // returns a map from (token: std::string) to (indexes: std::vector<int> *)
        // call `map_deleteAll<IntList>()` if no longer use
        StringMap buildExclusiveOptionMap() const;
    };

}

#endif // COMMAND_P_H

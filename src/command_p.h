#ifndef COMMAND_P_H
#define COMMAND_P_H

#include "argument_p.h"

namespace SysCmdLine {

    class CommandCataloguePrivate : public SharedBasePrivate {
    public:
        SharedBasePrivate *clone() const;
    };

    class CommandPrivate : public ArgumentHolderPrivate {
    public:
        CommandPrivate(std::string name, const std::string &desc);

        SharedBasePrivate *clone() const override;

    public:
        std::string name;

        std::vector<std::pair<Option, int>> options;
        std::vector<Command> commands;

        Option helpOption;
        Option versionOption;

        std::string version;
        std::string detailedDescription;
        CommandCatalogue catalogue;

        Command::Handler handler;
    };

}

#endif // COMMAND_P_H

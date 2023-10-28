#ifndef COMMAND_P_H
#define COMMAND_P_H

#include "argument_p.h"
#include "command.h"

#include "map_p.h"

namespace SysCmdLine {

    struct StringListMapWrapper {
        StringListMapWrapper();
        StringListMapWrapper(const StringListMapWrapper &other);
        ~StringListMapWrapper();

        StringMap data;
    };

    class CommandCataloguePrivate : public SharedBasePrivate {
    public:
        SharedBasePrivate *clone() const;

        StringListMapWrapper arg;
        StringListMapWrapper opt;
        StringListMapWrapper cmd;

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
    };

}

#endif // COMMAND_P_H

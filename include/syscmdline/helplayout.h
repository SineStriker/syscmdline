#ifndef HELPLAYOUT_H
#define HELPLAYOUT_H

#include <string>
#include <sstream>
#include <vector>
#include <functional>

#include <syscmdline/global.h>
#include <syscmdline/shareddata.h>

namespace SysCmdLine {

    class HelpLayoutData;

    class SYSCMDLINE_EXPORT HelpLayout {
    public:
        HelpLayout();
        ~HelpLayout();

        enum HelpItem {
            HI_CustomText,
            HI_Prologue,
            HI_Information,
            HI_Warning,
            HI_Error,
            HI_Description,
            HI_Usage,
            HI_Arguments,
            HI_Options,
            HI_Commands,
            HI_Epilogue,
        };

        enum SizeType {
            ST_Indent,
            ST_Spacing,
            ST_ConsoleWidth,
        };

        using LayoutBuilder = std::function<void(const std::vector<std::string> & /*lines */)>;

    public:
        int size(SizeType sizeType) const;
        void setSize(SizeType sizeType, int value);

        void add(const std::string &s);
        void add(HelpItem type, const LayoutBuilder &builder = {});

    protected:
        SharedDataPointer<HelpLayoutData> d_ptr;
    };

}

#endif // HELPLAYOUT_H

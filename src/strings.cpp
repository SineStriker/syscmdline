#include "strings.h"

#include "./locales/en_US.h"
#include "./locales/zh_CN.h"

namespace SysCmdLine::Strings {

    static TextProvider _provider = en_US::provider;

    void setTextProvider(TextProvider textProvider) {
        _provider = textProvider;
    }

    TextProvider textProvider() {
        return _provider;
    }

    TextProvider textProviderForLocale(Locale locale) {
        switch (locale) {
            case English:
                return en_US::provider;
            case SimplifiedChinese:
                return zh_CN::provider;
            default:
                break;
        }
        return nullptr;
    }

    std::string text(TextCategory category, int index) {
        return _provider(category, index);
    }

    static int _sizeConfig[] = {
        4,
        4,
        80,
    };

    int sizeConfig(int index) {
        return _sizeConfig[index];
    }

    void setSizeConfig(int index, int value) {
        _sizeConfig[index] = value;
    }

}
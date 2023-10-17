#ifndef OPTION_H
#define OPTION_H

#include <string>
#include <vector>

#include <syscmdline/argumentholder.h>

namespace SysCmdLine {

    template <class T>
    class BasicOption : public BasicSymbol<T>, public BasicArgumentHolder<T> {
    public:
        using super = BasicSymbol<T>;

        using value_type = typename super::value_type;
        using string_type = typename super::string_type;

        using arg_type = BasicArgument<value_type>;

        ~BasicOption() = default;

        BasicOption() : super(ST_Option), _short(false) {
        }

        BasicOption(const string_type &name, const std::vector<string_type> &tokens,
                    const std::vector<BasicArgument<value_type>> &arguments = {},
                    const string_type &desc = {})
            : super(ST_Option, name, desc), BasicArgumentHolder<T>(arguments), _tokens(tokens),
              _short(false) {
        }

        BasicOption(const string_type &name, const std::vector<string_type> &tokens, bool is_short,
                    const std::vector<BasicArgument<value_type>> &arguments,
                    const string_type &desc = {})
            : BasicOption(name, tokens, arguments, desc), _short(is_short) {
        }

    public:
        std::vector<string_type> tokens() const {
            return _tokens;
        }

        void setTokens(const std::vector<string_type> &tokens) {
            _tokens = tokens;
        }

        bool isShortOption() const {
            return _short;
        }

        void setShortOption(bool on) {
            _short = on;
        }

    private:
        std::vector<string_type> _tokens;
        bool _short;
    };

    using Option = BasicOption<char>;
    using WOption = BasicOption<wchar_t>;

}

#endif // OPTION_H

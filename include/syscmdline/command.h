#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>
#include <functional>

#include <syscmdline/option.h>

namespace SysCmdLine {

    template <class T>
    class BasicCommand : public BasicSymbol<T>, public BasicArgumentHolder<T> {
    public:
        using super = BasicSymbol<T>;

        using value_type = typename super::value_type;
        using string_type = typename super::string_type;

        using arg_type = BasicArgument<value_type>;
        using option_type = BasicOption<value_type>;
        using command_type = BasicCommand<value_type>;

        using Handler = std::function<int(const command_type &)>;

        ~BasicCommand();

        BasicCommand(const BasicCommand &other);
        BasicCommand(BasicCommand &&other) noexcept;
        BasicCommand &operator=(const BasicCommand &other);
        BasicCommand &operator=(BasicCommand &&other) noexcept;

    public:
        void addCommand(const BasicCommand &command);
        void addOption(const option_type &option);

        std::vector<command_type> commands() const;
        void setCommands(const std::vector<command_type> &command);

        std::vector<option_type> options() const;
        void setOptions(const std::vector<option_type> &options);

        string_type version() const;
        void setVersion(const string_type &version);

        string_type detailedDescription() const;
        void setDetailedDescription(const string_type &detailedDescription);

        Handler handler() const;
        void setHandler(const Handler &handler);

    private:
        std::vector<option_type> _options;
        std::vector<command_type *> _subCommands;
        string_type _version;
        string_type _detailedDescription;
        Handler _handler;
    };

    template <class T>
    BasicCommand<T>::~BasicCommand() {
        for (const auto &item : this->_subCommands)
            delete item;
    }

    template <class T>
    BasicCommand<T>::BasicCommand(const BasicCommand &other) {
        this->_arguments = other._arguments;
        this->_options = other._options;
        this->_subCommands.reserve(other._subCommands.size());
        for (const auto &item : other._subCommands)
            this->_subCommands.emplace_back(*new BasicCommand(*item));
    }

    template <class T>
    BasicCommand<T>::BasicCommand(BasicCommand &&other) noexcept {
        std::swap(this->_arguments, other._arguments);
        std::swap(this->_options, other._options);
        std::swap(this->_subCommands, other._subCommands);
    }

    template <class T>
    BasicCommand<T> &BasicCommand<T>::operator=(const BasicCommand &other) {
        if (this == &other)
            return *this;
        this->_arguments = other._arguments;
        this->_options = other._options;
        this->_subCommands.reserve(other._subCommands.size());
        for (const auto &item : other._subCommands)
            this->_subCommands.emplace_back(*new BasicCommand(*item));
        return *this;
    }

    template <class T>
    BasicCommand<T> &BasicCommand<T>::operator=(BasicCommand &&other) noexcept {
        if (this == &other)
            return *this;
        std::swap(this->_arguments, other._arguments);
        std::swap(this->_options, other._options);
        std::swap(this->_subCommands, other._subCommands);
        return *this;
    }

    template <class T>
    void BasicCommand<T>::addCommand(const BasicCommand &command) {
        _subCommands.push_back(*new BasicCommand(command));
    }

    template <class T>
    void BasicCommand<T>::addOption(const BasicCommand::option_type &option) {
        _options.push_back(option);
    }

    template <class T>
    typename BasicCommand<T>::string_type BasicCommand<T>::version() const {
        return this->_version;
    }

    template <class T>
    void BasicCommand<T>::setVersion(const BasicCommand::string_type &version) {
        _version = version;
    }

    template <class T>
    typename BasicCommand<T>::string_type BasicCommand<T>::detailedDescription() const {
        return this->_detailedDescription;
    }

    template <class T>
    void BasicCommand<T>::setDetailedDescription(
        const BasicCommand::string_type &detailedDescription) {
        _detailedDescription = detailedDescription;
    }

    template <class T>
    typename BasicCommand<T>::Handler BasicCommand<T>::handler() const {
        return _handler;
    }

    template <class T>
    void BasicCommand<T>::setHandler(const BasicCommand::Handler &handler) {
        _handler = handler;
    }

}

#endif // COMMAND_H

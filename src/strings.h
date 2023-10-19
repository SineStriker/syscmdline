#ifndef SYSCMDLINE_STRINGS_H
#define SYSCMDLINE_STRINGS_H

#include <string>
#include <vector>

namespace SysCmdLine::Strings {

    enum CommonString {
        Error,
        Usage,
        Description,
        Arguments,
        Options,
        Commands,
    };

    enum InfoString {
        Version,
        Help,
    };

    enum HelperString {
        MatchCommand,
    };

    extern const char INDENT[];

    extern const char *error_strings[];

    extern const char *common_strings[];

    extern const char *info_strings[];

    extern const char *helper_strings[];

    template <class T>
    std::vector<std::basic_string<T>> split(const std::basic_string<T> &s,
                                            const std::basic_string<T> &delimiter) {
        std::vector<std::basic_string<T>> tokens;
        typename std::basic_string<T>::size_type start = 0;
        typename std::basic_string<T>::size_type end = s.find(delimiter);
        while (end != std::basic_string<T>::npos) {
            tokens.push_back(s.substr(start, end - start));
            start = end + delimiter.size();
            end = s.find(delimiter, start);
        }
        tokens.push_back(s.substr(start));
        return tokens;
    }

    template <class T>
    std::basic_string<T> join(const std::vector<std::basic_string<T>> &v,
                              const std::basic_string<T> &delimiter) {
        if (v.empty())
            return {};

        std::basic_string<T> res;
        for (int i = 0; i < v.size() - 1; ++i) {
            res.append(v[i]);
            res.append(delimiter);
        }
        res.append(v.back());
        return res;
    }

    std::string formatText(const std::string &format, const std::vector<std::string> &args);

    int levenshteinDistance(const std::string &s1, const std::string &s2);

    std::vector<std::string> getClosestTexts(const std::vector<std::string> &texts,
                                             const std::string &input, int threshold);

    std::string removeSideQuotes(const std::string &s);

    std::string toUpper(std::string s);

    std::string toLower(std::string s);

    template <class T>
    constexpr bool starts_with(const std::basic_string<T> &s, const std::basic_string<T> &prefix) {
        return s.substr(0, prefix.size()) == prefix;
    }

}

#endif // SYSCMDLINE_STRINGS_H

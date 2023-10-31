#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

namespace SysCmdLine::Utils {

    std::vector<std::string> split(const std::string &s, const std::string &delimiter);

    std::string join(const std::vector<std::string> &v, const std::string &delimiter);

    std::string formatText(const std::string &format, const std::vector<std::string> &args);

    int levenshteinDistance(const std::string &s1, const std::string &s2);

    std::vector<std::string> calcClosestTexts(const std::vector<std::string> &texts,
                                              const std::string &input, int threshold);

    std::string removeSideQuotes(const std::string &s);

    std::string toUpper(std::string s);

    std::string toLower(std::string s);

    template <class T>
    std::vector<T> concatVector(const std::vector<T> &v1, const std::vector<T> &v2) {
        std::vector<T> res;
        res.reserve(v1.size() + v2.size());
        for (const auto &item : v1)
            res.emplace_back(item);
        for (const auto &item : v2)
            res.emplace_back(item);
        return res;
    }

}


#endif // UTILS_H

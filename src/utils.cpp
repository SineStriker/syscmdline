#include "utils.h"

#include <algorithm>

namespace SysCmdLine::Utils {

    std::string formatText(const std::string &format, const std::vector<std::string> &args) {
        std::string result = format;
        for (size_t i = 0; i < args.size(); i++) {
            std::string placeholder = "%" + std::to_string(i + 1);
            size_t pos = result.find(placeholder);
            while (pos != std::string::npos) {
                result.replace(pos, placeholder.length(), args[i]);
                pos = result.find(placeholder, pos + args[i].size());
            }
        }
        return result;
    }

    int levenshteinDistance(const std::string &s1, const std::string &s2) {
        int len1 = s1.size();
        int len2 = s2.size();

        std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1));

        for (int i = 0; i <= len1; i++) {
            for (int j = 0; j <= len2; j++) {
                if (i == 0) {
                    dp[i][j] = j;
                } else if (j == 0) {
                    dp[i][j] = i;
                } else if (s1[i - 1] == s2[j - 1]) {
                    dp[i][j] = dp[i - 1][j - 1];
                } else {
                    dp[i][j] = 1 + std::min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]});
                }
            }
        }

        return dp[len1][len2];
    }

    std::vector<std::string> calcClosestTexts(const std::vector<std::string> &texts,
                                             const std::string &input, int threshold) {

        std::vector<std::string> suggestions;
        for (const auto &cmd : texts) {
            int distance = levenshteinDistance(input, cmd);
            if (distance <= threshold) {
                suggestions.push_back(cmd);
            }
        }
        return suggestions;
    }

    std::string removeSideQuotes(const std::string &s) {
        if (s.size() < 2)
            return s;
        if ((s.front() == '\'' && s.back() == '\'') || (s.front() == '\"' && s.back() == '\"'))
            return s.substr(1, s.size() - 2);
        return s;
    }

    std::string toUpper(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
        return s;
    }

    std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }

}
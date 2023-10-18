#include "strings.h"

#include <algorithm>

namespace SysCmdLine::Strings {

    const char INDENT[] = "    ";

    const char *error_strings[] = {
        "",
        R"(Unknown option "%1".)",
        R"(Unknown command or argument "%1".)",
        R"(Missing required argument "%1" of option "%2".)",
        R"(Missing required argument "%1".)",
        R"(Too many arguments.)",
        R"(Invalid value "%1" of argument "%2".)",
    };

    const char *common_strings[] = {
        "Error", "Usage", "Description", "Arguments", "Options", "Commands",
    };

    const char *info_strings[] = {
        "Show version information",
        "Show help information",
    };

    const char *helper_strings[] = {
        R"("%1" is not matched. Do you mean one of the following?)",
    };

    // const char *error_strings[] = {
    //     "",
    //     R"(未定义的选项 "%1"。)",
    //     R"(未定义的参数或命令 "%1"。)",
    //     R"(选项 "%2" 缺少必选参数 "%1"。)",
    //     R"(缺少必选参数 "%1"。)",
    //     "参数 数量过多。",
    //     R"(参数 "%2" 指定为非法值 "%1"。)",
    // };

    // const char *common_strings[] = {
    //     "错误", "用法", "简介", "参数", "选项", "命令",
    // };

    // const char *info_strings[] = {
    //     "显示版本信息",
    //     "显示帮助信息",
    // };

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

    std::vector<std::string> getClosestTexts(const std::vector<std::string> &texts,
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

}
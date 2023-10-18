#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

std::vector<std::string> commands = {"list",     "create", "delete", "update", "help",
                                     "checkout", "clone",  "cut",    "merge",  "list2"};

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

std::vector<std::string> getClosestCommands(const std::string &input, int threshold) {
    std::vector<std::string> suggestions;
    for (const auto &cmd : commands) {
        int distance = levenshteinDistance(input, cmd);
        if (distance <= threshold) {
            suggestions.push_back(cmd);
        }
    }
    return suggestions;
}

int main() {
    std::cout << "Enter a command (or part of it): ";
    std::string input;
    std::cin >> input;

    int threshold = 2;

    std::vector<std::string> suggestions;
    do {
        suggestions = getClosestCommands(input, threshold);
    } while (suggestions.empty() && (++threshold) < (input.size()));

    if (!suggestions.empty()) {
        std::cout << "Did you mean: " << std::endl;
        for (const auto &s : suggestions) {
            std::cout << "  " << s << std::endl;
        }
    } else {
        std::cout << "No suggestions found." << std::endl;
    }

    return 0;
}
#include <iostream>
#include <string>
#include <cctype>

// Function to convert a string to Jaden Case
std::string toJadenCase(std::string str) {
    if (str.empty()) {
        return "";
    }

    bool capitalizeNext = true;
    for (char &c : str) {
        if (isspace(c)) {
            capitalizeNext = true;
        } else if (capitalizeNext) {
            c = toupper(c);
            capitalizeNext = false;
        }
    }
    return str;
}

int main() {
    std::string testCases[] = {
        "How can mirrors be real if our eyes aren't real",
        "most trees are blue",
        "",
        "If a book store never runs out of a certain book, does that mean that nobody reads it, or everybody reads it.",
        "P.S. I'm still in the studio."
    };

    for (int i = 0; i < 5; ++i) {
        std::cout << "Test Case " << i + 1 << ":" << std::endl;
        std::cout << "Original:    \"" << testCases[i] << "\"" << std::endl;
        std::string jadenCased = toJadenCase(testCases[i]);
        std::cout << "Jaden-Cased: \"" << jadenCased << "\"" << std::endl;
        std::cout << std::endl;
    }

    return 0;
}
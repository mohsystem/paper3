#include <iostream>
#include <string>
#include <sstream>
#include <cctype>

std::string toJadenCase(const std::string& str) {
    if (str.empty()) {
        return "";
    }

    std::stringstream ss(str);
    std::string word;
    std::string result = "";

    bool firstWord = true;
    while (ss >> word) {
        if (!firstWord) {
            result += " ";
        }
        if (!word.empty()) {
            word[0] = toupper(word[0]);
        }
        result += word;
        firstWord = false;
    }

    return result;
}

int main() {
    // Test Case 1
    std::string test1 = "How can mirrors be real if our eyes aren't real";
    std::cout << "Original: " << test1 << std::endl;
    std::cout << "Jaden-Cased: " << toJadenCase(test1) << std::endl;

    // Test Case 2
    std::string test2 = "most trees are blue";
    std::cout << "\nOriginal: " << test2 << std::endl;
    std::cout << "Jaden-Cased: " << toJadenCase(test2) << std::endl;

    // Test Case 3
    std::string test3 = "";
    std::cout << "\nOriginal: \"" << test3 << "\"" << std::endl;
    std::cout << "Jaden-Cased: \"" << toJadenCase(test3) << "\"" << std::endl;

    // Test Case 4
    std::string test4 = "a b c d";
    std::cout << "\nOriginal: " << test4 << std::endl;
    std::cout << "Jaden-Cased: " << toJadenCase(test4) << std::endl;

    // Test Case 5
    std::string test5 = "if a book store never runs out of a certain book, does that mean that nobody reads it, or everybody reads it";
    std::cout << "\nOriginal: " << test5 << std::endl;
    std::cout << "Jaden-Cased: " << toJadenCase(test5) << std::endl;

    return 0;
}
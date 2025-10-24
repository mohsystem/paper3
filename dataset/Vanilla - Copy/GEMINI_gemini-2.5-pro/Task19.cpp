#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

std::string spinWords(const std::string &str) {
    std::stringstream ss(str);
    std::string word;
    std::string result = "";
    bool firstWord = true;

    while (ss >> word) {
        if (!firstWord) {
            result += " ";
        }
        if (word.length() >= 5) {
            std::reverse(word.begin(), word.end());
        }
        result += word;
        firstWord = false;
    }
    return result;
}

int main() {
    // Test Case 1
    std::string input1 = "Hey fellow warriors";
    std::cout << "Input: \"" << input1 << "\"" << std::endl;
    std::cout << "Output: \"" << spinWords(input1) << "\"\n" << std::endl; // Expected: "Hey wollef sroirraw"

    // Test Case 2
    std::string input2 = "This is a test";
    std::cout << "Input: \"" << input2 << "\"" << std::endl;
    std::cout << "Output: \"" << spinWords(input2) << "\"\n" << std::endl; // Expected: "This is a test"

    // Test Case 3
    std::string input3 = "This is another test";
    std::cout << "Input: \"" << input3 << "\"" << std::endl;
    std::cout << "Output: \"" << spinWords(input3) << "\"\n" << std::endl; // Expected: "This is rehtona test"

    // Test Case 4
    std::string input4 = "Welcome";
    std::cout << "Input: \"" << input4 << "\"" << std::endl;
    std::cout << "Output: \"" << spinWords(input4) << "\"\n" << std::endl; // Expected: "emocleW"

    // Test Case 5
    std::string input5 = "Just kidding there is still one more";
    std::cout << "Input: \"" << input5 << "\"" << std::endl;
    std::cout << "Output: \"" << spinWords(input5) << "\"\n" << std::endl; // Expected: "Just gniddik ereht is llits one more"

    return 0;
}
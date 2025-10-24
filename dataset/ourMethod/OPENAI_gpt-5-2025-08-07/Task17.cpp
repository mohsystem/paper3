#include <iostream>
#include <string>
#include <cctype>

std::string ToJadenCase(const std::string& input) {
    std::string output = input;
    bool atWordStart = true;
    for (size_t i = 0; i < output.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(output[i]);
        if (std::isspace(ch)) {
            atWordStart = true;
        } else {
            if (atWordStart && std::isalpha(ch)) {
                output[i] = static_cast<char>(std::toupper(ch));
            }
            atWordStart = false;
        }
    }
    return output;
}

int main() {
    const std::string tests[5] = {
        "How can mirrors be real if our eyes aren't real",
        "",
        "hello\tworld\nfrom  c++",
        "already Capitalized Words",
        "multiple   spaces here"
    };

    for (int i = 0; i < 5; ++i) {
        std::string result = ToJadenCase(tests[i]);
        std::cout << "Input:  [" << tests[i] << "]\n";
        std::cout << "Output: [" << result << "]\n";
        std::cout << "----\n";
    }
    return 0;
}
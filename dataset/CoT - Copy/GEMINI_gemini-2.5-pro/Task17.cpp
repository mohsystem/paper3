#include <iostream>
#include <string>
#include <cctype>
#include <vector>

class Task17 {
public:
    static std::string toJadenCase(std::string phrase) {
        if (phrase.empty()) {
            return "";
        }

        bool capitalize = true;
        for (char &c : phrase) {
            // isalpha/isspace expect an int; casting to unsigned char prevents
            // issues with negative char values (undefined behavior).
            if (capitalize && std::isalpha(static_cast<unsigned char>(c))) {
                c = std::toupper(static_cast<unsigned char>(c));
                capitalize = false;
            } else if (std::isspace(static_cast<unsigned char>(c))) {
                capitalize = true;
            }
        }
        return phrase;
    }
};

int main() {
    std::vector<std::string> testCases = {
        "How can mirrors be real if our eyes aren't real",
        "",
        "most trees are blue",
        "The Quick Brown Fox Jumps Over The Lazy Dog",
        "hello world"
    };

    std::cout << "CPP Test Cases:" << std::endl;
    for (size_t i = 0; i < testCases.size(); ++i) {
        std::cout << "Test Case " << i + 1 << ":" << std::endl;
        std::cout << "  Original: \"" << testCases[i] << "\"" << std::endl;
        std::cout << "  Jaden-Cased: \"" << Task17::toJadenCase(testCases[i]) << "\"\n" << std::endl;
    }

    return 0;
}
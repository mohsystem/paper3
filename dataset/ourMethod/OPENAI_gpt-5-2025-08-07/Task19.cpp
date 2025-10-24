#include <iostream>
#include <string>
#include <cctype>

static bool isLettersAndSpaces(const std::string& s) {
    for (unsigned char ch : s) {
        if (ch != ' ' && !std::isalpha(ch)) {
            return false;
        }
    }
    return true;
}

std::string spinWords(const std::string& input) {
    // Validate input: only letters and spaces allowed
    if (!isLettersAndSpaces(input)) {
        return std::string(); // fail closed with empty string
    }

    std::string result;
    result.reserve(input.size());
    const size_t n = input.size();
    size_t i = 0;

    while (i < n) {
        if (input[i] == ' ') {
            result.push_back(' ');
            ++i;
            continue;
        }

        size_t j = i;
        while (j < n && input[j] != ' ') {
            ++j;
        }
        size_t word_len = j - i;

        if (word_len >= 5) {
            for (size_t k = 0; k < word_len; ++k) {
                result.push_back(input[j - 1 - k]);
            }
        } else {
            result.append(input, i, word_len);
        }
        i = j;
    }

    return result;
}

int main() {
    const std::string tests[5] = {
        "Hey fellow warriors",
        "This is a test",
        "This is another test",
        "Welcome",
        "abcde fghij klmno pqrst uvwxyz"
    };

    for (const auto& t : tests) {
        std::string out = spinWords(t);
        std::cout << "In:  " << t << "\nOut: " << out << "\n---\n";
    }

    return 0;
}
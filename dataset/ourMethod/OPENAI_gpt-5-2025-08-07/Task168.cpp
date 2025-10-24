#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

static std::string tweakLetters(const std::string& s, const std::vector<int>& shifts) {
    if (s.size() != shifts.size()) {
        throw std::invalid_argument("String length and shifts length must match.");
    }
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        char ch = s[i];
        if (ch < 'a' || ch > 'z') {
            throw std::invalid_argument("Only lowercase a-z are supported.");
        }
        int delta = shifts[i] % 26;
        int pos = ch - 'a';
        char outc = static_cast<char>('a' + (pos + delta + 26) % 26);
        out.push_back(outc);
    }
    return out;
}

static void runTest(const std::string& input, const std::vector<int>& shifts, const std::string& expected) {
    try {
        std::string result = tweakLetters(input, shifts);
        std::cout << "tweakLetters(\"" << input << "\", [";
        for (size_t i = 0; i < shifts.size(); ++i) {
            std::cout << shifts[i] << (i + 1 < shifts.size() ? ", " : "");
        }
        std::cout << "]) => \"" << result << "\"";
        if (!expected.empty()) {
            std::cout << " | expected: \"" << expected << "\"";
        }
        std::cout << std::endl;
    } catch (const std::exception& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
    }
}

int main() {
    runTest("apple", {0, 1, -1, 0, -1}, "aqold");
    runTest("many", {0, 0, 0, -1}, "manx");
    runTest("rhino", {1, 1, 1, 1, 1}, "sijop");
    runTest("zzz", {1, 1, 1}, "aaa");
    runTest("abc", {-1, 0, 1}, "zbd");
    return 0;
}
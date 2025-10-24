#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <limits>

class Task114 {
public:
    static std::string copyString(const std::string& s) {
        return std::string(s);
    }

    static std::string concatStrings(const std::string& a, const std::string& b) {
        if (a.size() > (std::numeric_limits<size_t>::max)() - b.size()) {
            throw std::length_error("Resulting string too large");
        }
        std::string result;
        result.reserve(a.size() + b.size());
        result.append(a);
        result.append(b);
        return result;
    }

    static std::string reverseString(const std::string& s) {
        std::string out = s;
        std::reverse(out.begin(), out.end());
        return out;
    }

    static std::string toUpperCaseSafe(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (unsigned char ch : s) {
            out.push_back(static_cast<char>(std::toupper(ch)));
        }
        return out;
    }

    static std::string replaceSubstringSafe(const std::string& s, const std::string& target, const std::string& replacement) {
        if (target.empty()) {
            // Avoid infinite loop; return a copy
            return std::string(s);
        }
        std::string out;
        size_t pos = 0, found;
        while ((found = s.find(target, pos)) != std::string::npos) {
            if (found > pos) out.append(s, pos, found - pos);
            out.append(replacement);
            pos = found + target.size();
        }
        if (pos < s.size()) out.append(s, pos, s.size() - pos);
        return out;
    }

    static bool isPalindrome(const std::string& s) {
        size_t i = 0, j = s.size();
        if (j == 0) return true;
        j--;
        while (i < j) {
            unsigned char ci = static_cast<unsigned char>(s[i]);
            unsigned char cj = static_cast<unsigned char>(s[j]);
            if (!std::isalnum(ci)) { i++; continue; }
            if (!std::isalnum(cj)) { j--; continue; }
            if (std::tolower(ci) != std::tolower(cj)) return false;
            i++; j--;
        }
        return true;
    }
};

int main() {
    // 5 test cases
    std::string t1 = Task114::copyString("Hello, World!");
    std::cout << "Copy: " << t1 << "\n";

    std::string t2 = Task114::concatStrings(std::string("Hello "), std::string("World"));
    std::cout << "Concat: " << t2 << "\n";

    std::string t3 = Task114::reverseString("abcdefg");
    std::cout << "Reverse: " << t3 << "\n";

    std::string t4 = Task114::toUpperCaseSafe("SecuRe123!");
    std::cout << "Upper: " << t4 << "\n";

    std::string t5 = Task114::replaceSubstringSafe("the cat sat on the mat", "at", "oodle");
    std::cout << "Replace: " << t5 << "\n";

    return 0;
}
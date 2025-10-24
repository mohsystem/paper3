#include <iostream>
#include <string>
#include <set>
#include <algorithm>

class Task30 {
public:
    /**
     * Takes two strings s1 and s2 including only letters from a to z.
     * Returns a new sorted string, the longest possible, containing distinct letters.
     */
    static std::string longest(const std::string& s1, const std::string& s2) {
        std::set<char> distinct_chars;

        // Insert characters from s1 into the set
        for (char c : s1) {
            distinct_chars.insert(c);
        }

        // Insert characters from s2 into the set
        for (char c : s2) {
            distinct_chars.insert(c);
        }

        // A set is already sorted, so we can construct a string from its iterators
        return std::string(distinct_chars.begin(), distinct_chars.end());
    }
};

int main() {
    std::cout << "C++ Test Cases:" << std::endl;

    // Test Case 1
    std::string s1_1 = "xyaabbbccccdefww";
    std::string s2_1 = "xxxxyyyyabklmopq";
    std::cout << "Test 1: " << Task30::longest(s1_1, s2_1) << std::endl; // Expected: "abcdefklmopqwxy"

    // Test Case 2
    std::string s1_2 = "abcdefghijklmnopqrstuvwxyz";
    std::string s2_2 = "abcdefghijklmnopqrstuvwxyz";
    std::cout << "Test 2: " << Task30::longest(s1_2, s2_2) << std::endl; // Expected: "abcdefghijklmnopqrstuvwxyz"

    // Test Case 3
    std::string s1_3 = "aretheyhere";
    std::string s2_3 = "yestheyarehere";
    std::cout << "Test 3: " << Task30::longest(s1_3, s2_3) << std::endl; // Expected: "aehrsty"

    // Test Case 4
    std::string s1_4 = "loopingisfunbutdangerous";
    std::string s2_4 = "lessdangerousthancoding";
    std::cout << "Test 4: " << Task30::longest(s1_4, s2_4) << std::endl; // Expected: "abcdefghilnoprstu"

    // Test Case 5
    std::string s1_5 = "inmanylanguages";
    std::string s2_5 = "theresapairoffunctions";
    std::cout << "Test 5: " << Task30::longest(s1_5, s2_5) << std::endl; // Expected: "acefghilmnoprstu"

    return 0;
}
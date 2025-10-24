#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

/**
 * @brief Takes two strings and returns a new sorted string containing distinct
 *        letters from both input strings.
 *
 * @param s1 The first input string, containing only letters from 'a' to 'z'.
 * @param s2 The second input string, containing only letters from 'a' to 'z'.
 * @return A new sorted string with unique characters from s1 and s2.
 */
std::string longest(const std::string& s1, const std::string& s2) {
    // Concatenate the two strings.
    std::string combined = s1 + s2;

    // Sort the combined string to bring identical characters together.
    std::sort(combined.begin(), combined.end());

    // Use std::unique to move all unique elements to the front and get an
    // iterator to the new end of the unique range. Then, erase the duplicate
    // elements from the end of the string.
    combined.erase(std::unique(combined.begin(), combined.end()), combined.end());

    return combined;
}

void run_test(const std::string& test_name, const std::string& s1, const std::string& s2) {
    std::cout << test_name << ": longest(\"" << s1 << "\", \"" << s2 << "\") -> \""
              << longest(s1, s2) << "\"" << std::endl;
}

int main() {
    // Test Case 1
    run_test("Test 1", "xyaabbbccccdefww", "xxxxyyyyabklmopq");

    // Test Case 2
    run_test("Test 2", "abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz");

    // Test Case 3
    run_test("Test 3", "aretheyhere", "yestheyarehere");

    // Test Case 4
    run_test("Test 4", "loopingisfunbutdangerous", "lessdangerousthancoding");

    // Test Case 5
    run_test("Test 5", "", "zyxw");

    return 0;
}
#include <iostream>
#include <string>
#include <vector>
#include <cctype>

/**
 * @brief Checks if a given string is a pangram.
 *
 * A pangram is a sentence containing every letter of the English alphabet at least once.
 * This function is case-insensitive and ignores numbers, punctuation, and whitespace.
 *
 * @param s The input string to check.
 * @return true if the string is a pangram, false otherwise.
 */
bool isPangram(const std::string& s) {
    // A boolean array to mark the presence of each letter of the alphabet.
    std::vector<bool> seen(26, false);
    
    // Iterate over each character in the input string.
    for (char c : s) {
        // Check if the character is an alphabet letter.
        if (std::isalpha(static_cast<unsigned char>(c))) {
            // Convert the character to lowercase to handle case-insensitivity.
            char lower_c = std::tolower(static_cast<unsigned char>(c));
            // Mark the corresponding letter as seen.
            // 'a' corresponds to index 0, 'b' to 1, and so on.
            seen[lower_c - 'a'] = true;
        }
    }
    
    // Check if all 26 letters have been seen.
    for (bool found : seen) {
        if (!found) {
            return false; // If any letter is missing, it's not a pangram.
        }
    }
    
    return true; // All letters were found.
}

int main() {
    // Set cout to print "true" or "false" for bools
    std::cout << std::boolalpha;

    // Test Case 1: A classic pangram
    std::string s1 = "The quick brown fox jumps over the lazy dog";
    std::cout << "Test 1: \"" << s1 << "\" -> " << isPangram(s1) << std::endl;

    // Test Case 2: A non-pangram
    std::string s2 = "This is not a pangram";
    std::cout << "Test 2: \"" << s2 << "\" -> " << isPangram(s2) << std::endl;

    // Test Case 3: Pangram with all letters, upper and lower case
    std::string s3 = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::cout << "Test 3: \"" << s3 << "\" -> " << isPangram(s3) << std::endl;

    // Test Case 4: Another common pangram
    std::string s4 = "Pack my box with five dozen liquor jugs.";
    std::cout << "Test 4: \"" << s4 << "\" -> " << isPangram(s4) << std::endl;

    // Test Case 5: A string missing a few letters
    std::string s5 = "A quick brown fox jumps over the lazy cat";
    std::cout << "Test 5: \"" << s5 << "\" -> " << isPangram(s5) << std::endl;
    
    return 0;
}
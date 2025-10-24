#include <iostream>
#include <string>
#include <cctype>

/**
 * @brief Determines whether a string is an isogram.
 * An isogram is a word that has no repeating letters, consecutive or non-consecutive.
 * This function ignores letter case and treats an empty string as an isogram.
 *
 * @param str The input string to check. Assumed to contain only letters.
 * @return true if the string is an isogram, false otherwise.
 */
bool is_isogram(const std::string& str) {
    // A boolean array to mark the presence of characters 'a' through 'z'.
    // Initialized to all false.
    bool seen[26] = {false};

    // Iterate over each character in the string
    for (char ch : str) {
        // Check if the character is an alphabet character
        if (std::isalpha(static_cast<unsigned char>(ch))) {
            // Convert character to lowercase to treat 'A' and 'a' as the same
            char lower_ch = std::tolower(static_cast<unsigned char>(ch));
            
            // Calculate the index for the character (0 for 'a', 1 for 'b', etc.)
            int index = lower_ch - 'a';

            // If we have seen this character before, it's not an isogram
            if (seen[index]) {
                return false;
            }
            
            // Mark this character as seen
            seen[index] = true;
        }
    }

    // If the loop completes without finding duplicates, it's an isogram
    return true;
}

int main() {
    const int num_tests = 5;
    std::string test_cases[num_tests] = {
        "Dermatoglyphics", // true
        "aba",             // false
        "moOse",           // false
        "isogram",         // true
        ""                 // true
    };
    bool expected_results[num_tests] = {true, false, false, true, true};

    std::cout << "Running C++ test cases...\n";
    for (int i = 0; i < num_tests; ++i) {
        bool result = is_isogram(test_cases[i]);
        std::cout << "Test " << i + 1 << ": \"" << test_cases[i] << "\"\n";
        std::cout << "Expected: " << std::boolalpha << expected_results[i] 
                  << ", Got: " << std::boolalpha << result;
        if (result == expected_results[i]) {
            std::cout << " -> PASS\n";
        } else {
            std::cout << " -> FAIL\n";
        }
    }

    return 0;
}
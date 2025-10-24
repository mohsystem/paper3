#include <iostream>
#include <string>
#include <vector>
#include <cctype>

class Task4 {
public:
    /**
     * Determines whether a string is an isogram (has no repeating letters, ignoring case).
     *
     * @param str The input string, assumed to contain only letters.
     * @return true if the string is an isogram, false otherwise.
     */
    static bool isIsogram(const std::string& str) {
        // An empty string is considered an isogram.
        if (str.empty()) {
            return true;
        }

        // Use a boolean vector to track seen characters (a-z).
        std::vector<bool> seen(26, false);

        // Iterate through each character of the string.
        for (char c : str) {
            // Convert character to lowercase.
            char lower_c = std::tolower(static_cast<unsigned char>(c));
            int index = lower_c - 'a';

            // If the character has been seen before, it's not an isogram.
            if (seen[index]) {
                return false;
            }

            // Mark the character as seen.
            seen[index] = true;
        }

        // If the loop completes, no repeating characters were found.
        return true;
    }
};

int main() {
    // Test cases
    std::string testCases[] = {"Dermatoglyphics", "aba", "moOse", "isIsogram", ""};
    bool expectedResults[] = {true, false, false, false, true};

    std::cout << std::boolalpha; // Print booleans as "true" or "false"
    for (int i = 0; i < 5; ++i) {
        bool result = Task4::isIsogram(testCases[i]);
        std::cout << "Input: \"" << testCases[i] << "\" -> Output: " << result 
                  << " | Expected: " << expectedResults[i] << std::endl;
    }

    return 0;
}
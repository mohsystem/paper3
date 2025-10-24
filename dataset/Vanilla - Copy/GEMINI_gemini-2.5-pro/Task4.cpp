#include <iostream>
#include <string>
#include <cctype>
#include <set>

/**
 * @brief Determines if a string is an isogram (no repeating letters, case-insensitive).
 *
 * @param str The string to check.
 * @return true if the string is an isogram, false otherwise.
 */
bool isIsogram(std::string str) {
    std::set<char> seen;
    for (char ch : str) {
        // Convert character to lowercase for case-insensitive comparison.
        char lower_ch = std::tolower(static_cast<unsigned char>(ch));
        
        // The insert method of a set returns a pair. The second element of the pair
        // is a boolean that is true if the insertion took place, and false if the
        // element was already in the set.
        if (!seen.insert(lower_ch).second) {
            // If insertion fails, it means we have a duplicate character.
            return false;
        }
    }
    // If the loop completes, no duplicates were found.
    return true;
}

int main() {
    std::string testCases[] = {
        "Dermatoglyphics",
        "aba",
        "moOse",
        "",
        "isogram"
    };

    std::cout << "CPP Test Cases:" << std::endl;
    // Use std::boolalpha to print "true" or "false" instead of 1 or 0.
    std::cout << std::boolalpha; 
    for (const std::string& test : testCases) {
        std::cout << "\"" << test << "\" --> " << isIsogram(test) << std::endl;
    }

    return 0;
}
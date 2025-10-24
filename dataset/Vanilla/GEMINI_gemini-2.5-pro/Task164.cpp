#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

/**
 * Extracts the last name from a full name string.
 * @param name The full name string.
 * @return The last name.
 */
std::string getLastName(const std::string& name) {
    size_t last_space_pos = name.find_last_of(' ');
    if (last_space_pos != std::string::npos) {
        return name.substr(last_space_pos + 1);
    }
    return name; // Fallback if no space
}

/**
 * Sorts a vector of names based on the length of the last name, then alphabetically by last name.
 * @param names A vector of strings, where each string is a name in "First Last" format.
 * @return The sorted vector of names.
 */
std::vector<std::string> lastNameLensort(std::vector<std::string> names) {
    std::sort(names.begin(), names.end(), [](const std::string& a, const std::string& b) {
        std::string lastNameA = getLastName(a);
        std::string lastNameB = getLastName(b);

        if (lastNameA.length() != lastNameB.length()) {
            return lastNameA.length() < lastNameB.length();
        }
        return lastNameA < lastNameB;
    });
    return names;
}

/**
 * Helper function to print a vector of strings.
 * @param vec The vector to print.
 */
void printVector(const std::vector<std::string>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << "\"" << vec[i] << "\"";
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]";
}

int main() {
    std::vector<std::vector<std::string>> testCases = {
        {"Jennifer Figueroa", "Heather Mcgee", "Amanda Schwartz", "Nicole Yoder", "Melissa Hoffman"},
        {"A B", "C D", "E F"},
        {"Morty Smith", "Rick Sanchez", "Jerry Smith"},
        {"Peter Parker", "Clark Kent", "Bruce Wayne"},
        {"Leo Tolstoy", "Jane Austen", "Charles Dickens"}
    };

    for (size_t i = 0; i < testCases.size(); ++i) {
        std::cout << "Test Case " << i + 1 << ":" << std::endl;
        std::cout << "Input: ";
        printVector(testCases[i]);
        std::cout << std::endl;

        std::vector<std::string> result = lastNameLensort(testCases[i]);

        std::cout << "Output: ";
        printVector(result);
        std::cout << std::endl << std::endl;
    }

    return 0;
}
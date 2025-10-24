#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>

class Task164 {
private:
    /**
     * @brief Securely extracts the last name from a full name string.
     * This helper function handles empty strings and names with leading/trailing whitespace.
     * @param fullName The full name string.
     * @return The extracted last name.
     */
    static std::string getLastName(const std::string& fullName) {
        // Find the last non-whitespace character to handle trailing spaces
        size_t end_pos = fullName.find_last_not_of(" \t\n\r");
        if (std::string::npos == end_pos) {
            return ""; // String is empty or contains only whitespace
        }

        // From that position, find the last whitespace character, which precedes the last name
        size_t start_pos = fullName.find_last_of(" \t\n\r", end_pos);
        if (std::string::npos == start_pos) {
            // No space found, the whole (trimmed) string is the last name
            return fullName.substr(0, end_pos + 1);
        }

        // Return the substring that represents the last name
        return fullName.substr(start_pos + 1, end_pos - start_pos);
    }

public:
    /**
     * @brief Sorts a vector of names by the length of their last names.
     * If lengths are equal, it sorts alphabetically by last name.
     * @param names A vector of strings, where each string is a full name.
     * @return A new, sorted vector of strings.
     */
    static std::vector<std::string> lastNameLensort(std::vector<std::string> names) {
        // The function takes by value, so we are already working on a safe copy.
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
};

// Helper function to print a vector of strings for testing
void printVector(const std::string& label, const std::vector<std::string>& vec) {
    std::cout << label;
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << "\"" << vec[i] << "\"";
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}

int main() {
    // Test Case 1: Example from prompt
    std::vector<std::string> test1 = {"Jennifer Figueroa", "Heather Mcgee", "Amanda Schwartz", "Nicole Yoder", "Melissa Hoffman"};
    std::cout << "Test Case 1:" << std::endl;
    printVector("Input: ", test1);
    printVector("Output: ", Task164::lastNameLensort(test1));
    std::cout << std::endl;

    // Test Case 2: Last names with same length
    std::vector<std::string> test2 = {"Steven King", "John Smith", "Jane Doe"};
    std::cout << "Test Case 2:" << std::endl;
    printVector("Input: ", test2);
    printVector("Output: ", Task164::lastNameLensort(test2));
    std::cout << std::endl;

    // Test Case 3: Names without spaces
    std::vector<std::string> test3 = {"Beyonce", "Adele", "Cher"};
    std::cout << "Test Case 3:" << std::endl;
    printVector("Input: ", test3);
    printVector("Output: ", Task164::lastNameLensort(test3));
    std::cout << std::endl;

    // Test Case 4: Mixed names (with and without spaces)
    std::vector<std::string> test4 = {"David Copperfield", "David Blaine", "Penn"};
    std::cout << "Test Case 4:" << std::endl;
    printVector("Input: ", test4);
    printVector("Output: ", Task164::lastNameLensort(test4));
    std::cout << std::endl;

    // Test Case 5: Edge cases
    std::vector<std::string> test5 = {"", " ", "Single", "Peter Pan"};
    std::cout << "Test Case 5:" << std::endl;
    printVector("Input: ", test5);
    printVector("Output: ", Task164::lastNameLensort(test5));
    std::cout << std::endl;

    return 0;
}
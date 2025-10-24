#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

/**
 * Extracts the last name from a full name string.
 * If no space is found, the whole string is returned.
 * @param fullName The full name string.
 * @return The last name part of the string.
 */
std::string getLastName(const std::string& fullName) {
    size_t last_space_pos = fullName.find_last_of(' ');
    if (last_space_pos == std::string::npos) {
        return fullName;
    }
    return fullName.substr(last_space_pos + 1);
}

/**
 * Sorts a vector of names based on the length of the last name, then alphabetically by last name.
 * @param names A vector of strings, where each string is a full name.
 * @return A new vector with the names sorted.
 */
std::vector<std::string> lastNameLensort(const std::vector<std::string>& names) {
    std::vector<std::string> sortedNames = names; // Create a copy
    
    std::sort(sortedNames.begin(), sortedNames.end(), 
        [](const std::string& a, const std::string& b) {
            std::string lastNameA = getLastName(a);
            std::string lastNameB = getLastName(b);
            
            if (lastNameA.length() != lastNameB.length()) {
                return lastNameA.length() < lastNameB.length();
            }
            return lastNameA < lastNameB;
        });
        
    return sortedNames;
}

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
        {
            "Jennifer Figueroa", "Heather Mcgee", "Amanda Schwartz",
            "Nicole Yoder", "Melissa Hoffman"
        },
        {
            "John Smith", "Jane Doe", "Peter Jones", "Mary Johnson"
        },
        {
            "alpha beta", "gamma delta"
        },
        {
            "A B", "C D", "E F"
        },
        {
            "SingleName", "Another SingleName"
        }
    };

    for (size_t i = 0; i < testCases.size(); ++i) {
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        std::cout << "Original: ";
        printVector(testCases[i]);
        std::cout << std::endl;
        
        std::vector<std::string> result = lastNameLensort(testCases[i]);
        
        std::cout << "Sorted:   ";
        printVector(result);
        std::cout << std::endl << std::endl;
    }
    
    return 0;
}
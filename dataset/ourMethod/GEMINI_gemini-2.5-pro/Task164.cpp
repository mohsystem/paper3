#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <string_view>

// Helper function to extract the last name as a string_view for efficiency
std::string_view getLastName(const std::string& fullName) {
    size_t lastSpacePos = fullName.find_last_of(' ');
    if (lastSpacePos == std::string::npos) {
        return std::string_view(fullName);
    }
    return std::string_view(fullName).substr(lastSpacePos + 1);
}

std::vector<std::string> lastNameLensort(const std::vector<std::string>& names) {
    std::vector<std::string> sortedNames = names;

    std::sort(sortedNames.begin(), sortedNames.end(), 
        [](const std::string& a, const std::string& b) {
            std::string_view lastNameA = getLastName(a);
            std::string_view lastNameB = getLastName(b);

            if (lastNameA.length() != lastNameB.length()) {
                return lastNameA.length() < lastNameB.length();
            }
            return lastNameA < lastNameB;
        });

    return sortedNames;
}

void printNames(const std::string& testCaseName, const std::vector<std::string>& names) {
    std::cout << testCaseName << ":" << std::endl;
    std::cout << "Input: [";
    for (size_t i = 0; i < names.size(); ++i) {
        std::cout << "\"" << names[i] << "\"" << (i == names.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
    
    std::vector<std::string> sorted = lastNameLensort(names);
    
    std::cout << "Output: [";
    for (size_t i = 0; i < sorted.size(); ++i) {
        std::cout << "\"" << sorted[i] << "\"" << (i == sorted.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl << std::endl;
}

int main() {
    // Test Case 1
    std::vector<std::string> names1 = {
        "Jennifer Figueroa", "Heather Mcgee", "Amanda Schwartz", "Nicole Yoder", "Melissa Hoffman"
    };
    printNames("Test Case 1", names1);

    // Test Case 2: Same last name length
    std::vector<std::string> names2 = {"John Smith", "Jane Doe", "Peter Jones"};
    printNames("Test Case 2", names2);

    // Test Case 3: Empty array
    std::vector<std::string> names3 = {};
    printNames("Test Case 3", names3);

    // Test Case 4: Single-word names
    std::vector<std::string> names4 = {"Cher", "Madonna", "Prince"};
    printNames("Test Case 4", names4);

    // Test Case 5: Names with same last names
    std::vector<std::string> names5 = {"David Lee", "Bruce Lee", "Sara Lee"};
    printNames("Test Case 5", names5);
    
    return 0;
}
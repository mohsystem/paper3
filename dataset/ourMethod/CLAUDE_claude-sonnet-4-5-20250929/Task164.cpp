
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <cstring>

// Function to extract last name from full name
// Returns empty string if format is invalid
std::string getLastName(const std::string& fullName) {
    // Input validation: check for null/empty string
    if (fullName.empty() || fullName.length() > 1000) {
        return "";
    }
    
    // Find last space to separate first and last name
    size_t lastSpace = fullName.rfind(' ');
    
    // Validate format: must have at least one space
    if (lastSpace == std::string::npos || lastSpace == 0 || lastSpace == fullName.length() - 1) {
        return "";
    }
    
    // Extract last name (substring is bounds-safe in C++)
    return fullName.substr(lastSpace + 1);
}

// Comparison function for sorting
bool compareNames(const std::string& name1, const std::string& name2) {
    std::string lastName1 = getLastName(name1);
    std::string lastName2 = getLastName(name2);
    
    // Handle invalid entries by placing them at the end
    if (lastName1.empty() && lastName2.empty()) return false;
    if (lastName1.empty()) return false;
    if (lastName2.empty()) return true;
    
    // Primary sort: by length of last name (ascending)
    if (lastName1.length() != lastName2.length()) {
        return lastName1.length() < lastName2.length();
    }
    
    // Secondary sort: alphabetically by last name (case-sensitive)
    return lastName1 < lastName2;
}

// Main sorting function
std::vector<std::string> lastNameLensort(const std::vector<std::string>& names) {
    // Input validation: check array size to prevent excessive memory usage
    if (names.size() > 100000) {
        return std::vector<std::string>();
    }
    
    // Create a copy to avoid modifying original array
    std::vector<std::string> result = names;
    
    // Validate each name entry before processing
    for (const auto& name : result) {
        if (name.empty() || name.length() > 1000) {
            return std::vector<std::string>();
        }
    }
    
    // Use std::sort with custom comparator (safe and efficient)
    std::sort(result.begin(), result.end(), compareNames);
    
    return result;
}

int main() {
    // Test case 1: Example from prompt
    std::vector<std::string> test1 = {
        "Jennifer Figueroa",
        "Heather Mcgee",
        "Amanda Schwartz",
        "Nicole Yoder",
        "Melissa Hoffman"
    };
    std::vector<std::string> result1 = lastNameLensort(test1);
    std::cout << "Test 1: ";
    for (const auto& name : result1) {
        std::cout << name << ", ";
    }
    std::cout << std::endl;
    
    // Test case 2: Same length last names
    std::vector<std::string> test2 = {
        "John Smith",
        "Jane Abbot",
        "Bob Jones"
    };
    std::vector<std::string> result2 = lastNameLensort(test2);
    std::cout << "Test 2: ";
    for (const auto& name : result2) {
        std::cout << name << ", ";
    }
    std::cout << std::endl;
    
    // Test case 3: Single name
    std::vector<std::string> test3 = {"Alice Johnson"};
    std::vector<std::string> result3 = lastNameLensort(test3);
    std::cout << "Test 3: ";
    for (const auto& name : result3) {
        std::cout << name << ", ";
    }
    std::cout << std::endl;
    
    // Test case 4: Empty array
    std::vector<std::string> test4 = {};
    std::vector<std::string> result4 = lastNameLensort(test4);
    std::cout << "Test 4: ";
    for (const auto& name : result4) {
        std::cout << name << ", ";
    }
    std::cout << std::endl;
    
    // Test case 5: Various lengths
    std::vector<std::string> test5 = {
        "Michael Anderson",
        "Sarah Lee",
        "David Wu",
        "Emma Thompson"
    };
    std::vector<std::string> result5 = lastNameLensort(test5);
    std::cout << "Test 5: ";
    for (const auto& name : result5) {
        std::cout << name << ", ";
    }
    std::cout << std::endl;
    
    return 0;
}

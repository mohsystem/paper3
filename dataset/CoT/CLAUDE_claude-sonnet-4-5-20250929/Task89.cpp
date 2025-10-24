
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <algorithm>

// Constant-time string comparison to prevent timing attacks
bool constantTimeEquals(const std::string& a, const std::string& b) {
    if (a.empty() && b.empty()) {
        return true;
    }
    
    size_t lengthA = a.length();
    size_t lengthB = b.length();
    
    // Use the longer length to prevent timing attacks
    size_t maxLength = std::max(lengthA, lengthB);
    int result = lengthA ^ lengthB; // Different lengths contribute to mismatch
    
    for (size_t i = 0; i < maxLength; i++) {
        char charA = (i < lengthA) ? a[i] : 0;
        char charB = (i < lengthB) ? b[i] : 0;
        result |= charA ^ charB;
    }
    
    return result == 0;
}

// Trim whitespace from string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \\t\\n\\r");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \\t\\n\\r");
    return str.substr(first, (last - first + 1));
}

bool verifyPassword(const std::string& filePath, const std::string& enteredPassword) {
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open password file" << std::endl;
        return false;
    }
    
    std::string storedPassword;
    std::getline(file, storedPassword);
    file.close();
    
    // Trim whitespace
    storedPassword = trim(storedPassword);
    
    // Constant-time comparison
    bool matches = constantTimeEquals(storedPassword, enteredPassword);
    
    // Clear sensitive data from memory
    std::fill(storedPassword.begin(), storedPassword.end(), '\\0');
    
    return matches;
}

int main() {
    std::cout << "=== Password Verification Test Cases ===" << std::endl << std::endl;
    
    // Create test password file
    std::string testFile = "test_password.txt";
    std::ofstream outFile(testFile);
    if (outFile.is_open()) {
        outFile << "SecurePass123!";
        outFile.close();
    } else {
        std::cerr << "Failed to create test file" << std::endl;
        return 1;
    }
    
    // Test Case 1: Correct password
    std::cout << "Test 1 - Correct password:" << std::endl;
    bool result1 = verifyPassword(testFile, "SecurePass123!");
    std::cout << "Result: " << (result1 ? "MATCH" : "NO MATCH") << std::endl << std::endl;
    
    // Test Case 2: Incorrect password
    std::cout << "Test 2 - Incorrect password:" << std::endl;
    bool result2 = verifyPassword(testFile, "WrongPassword");
    std::cout << "Result: " << (result2 ? "MATCH" : "NO MATCH") << std::endl << std::endl;
    
    // Test Case 3: Password with different case
    std::cout << "Test 3 - Different case:" << std::endl;
    bool result3 = verifyPassword(testFile, "securepass123!");
    std::cout << "Result: " << (result3 ? "MATCH" : "NO MATCH") << std::endl << std::endl;
    
    // Test Case 4: Empty password
    std::cout << "Test 4 - Empty password:" << std::endl;
    bool result4 = verifyPassword(testFile, "");
    std::cout << "Result: " << (result4 ? "MATCH" : "NO MATCH") << std::endl << std::endl;
    
    // Test Case 5: Password with extra characters
    std::cout << "Test 5 - Password with extra characters:" << std::endl;
    bool result5 = verifyPassword(testFile, "SecurePass123!Extra");
    std::cout << "Result: " << (result5 ? "MATCH" : "NO MATCH") << std::endl << std::endl;
    
    // Cleanup test file
    std::remove(testFile.c_str());
    
    return 0;
}

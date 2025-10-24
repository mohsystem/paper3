
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <limits>

// Constant-time comparison to prevent timing attacks
bool constantTimeCompare(const std::string& a, const std::string& b) {
    // Always compare full lengths to avoid timing leaks
    if (a.length() != b.length()) {
        return false;
    }
    
    volatile unsigned char result = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        result |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
    }
    
    return result == 0;
}

// Validates that password meets basic requirements
bool validatePasswordInput(const std::string& password) {
    // Check length constraints (1-1024 characters)
    if (password.empty() || password.length() > 1024) {
        return false;
    }
    
    // Ensure no null bytes in password
    if (password.find('\\0') != std::string::npos) {
        return false;
    }
    
    return true;
}

// Securely reads password from file with validation
bool readPasswordFromFile(const std::string& filename, std::string& storedPassword) {
    // Validate filename to prevent path traversal
    if (filename.empty() || filename.length() > 255) {
        return false;
    }
    
    // Check for path traversal attempts
    if (filename.find("..") != std::string::npos || 
        filename.find('/') != std::string::npos ||
        filename.find('\\\\') != std::string::npos) {\n        return false;\n    }\n    \n    std::ifstream file(filename, std::ios::binary);\n    if (!file.is_open()) {\n        return false;\n    }\n    \n    // Read with size limit to prevent memory exhaustion\n    const size_t MAX_FILE_SIZE = 2048;\n    std::vector<char> buffer(MAX_FILE_SIZE + 1, 0);\n    \n    file.read(buffer.data(), MAX_FILE_SIZE);\n    std::streamsize bytesRead = file.gcount();\n    \n    if (bytesRead <= 0 || bytesRead > static_cast<std::streamsize>(MAX_FILE_SIZE)) {\n        file.close();\n        return false;\n    }\n    \n    file.close();\n    \n    // Ensure null termination\n    buffer[bytesRead] = '\\0';\n    \n    // Convert to string and remove trailing newline/whitespace\n    storedPassword = std::string(buffer.data(), bytesRead);\n    \n    // Remove trailing newline characters\n    while (!storedPassword.empty() && \n           (storedPassword.back() == '\
' || storedPassword.back() == '\\r')) {\n        storedPassword.pop_back();\n    }\n    \n    // Validate stored password\n    if (!validatePasswordInput(storedPassword)) {\n        storedPassword.clear();\n        return false;\n    }\n    \n    return true;\n}\n\n// Main password verification function\nbool verifyPassword(const std::string& filename, const std::string& userPassword) {\n    // Validate user input\n    if (!validatePasswordInput(userPassword)) {\n        return false;\n    }\n    \n    std::string storedPassword;\n    if (!readPasswordFromFile(filename, storedPassword)) {\n        return false;\n    }\n    \n    // Use constant-time comparison to prevent timing attacks\n    bool result = constantTimeCompare(storedPassword, userPassword);\n    \n    // Securely clear stored password from memory\n    std::fill(storedPassword.begin(), storedPassword.end(), '\\0');
    
    return result;
}

int main() {
    // Test case 1: Valid password match
    {
        std::ofstream testFile("test1.txt");
        testFile << "SecurePass123";
        testFile.close();
        
        bool result = verifyPassword("test1.txt", "SecurePass123");
        std::cout << "Test 1 (valid match): " << (result ? "PASS" : "FAIL") << std::endl;
    }
    
    // Test case 2: Invalid password (mismatch)
    {
        std::ofstream testFile("test2.txt");
        testFile << "CorrectPassword";
        testFile.close();
        
        bool result = verifyPassword("test2.txt", "WrongPassword");
        std::cout << "Test 2 (mismatch): " << (!result ? "PASS" : "FAIL") << std::endl;
    }
    
    // Test case 3: Empty password attempt
    {
        std::ofstream testFile("test3.txt");
        testFile << "ValidPass";
        testFile.close();
        
        bool result = verifyPassword("test3.txt", "");
        std::cout << "Test 3 (empty input): " << (!result ? "PASS" : "FAIL") << std::endl;
    }
    
    // Test case 4: File with newline
    {
        std::ofstream testFile("test4.txt");
        testFile << "PasswordWithNewline\\n";
        testFile.close();
        
        bool result = verifyPassword("test4.txt", "PasswordWithNewline");
        std::cout << "Test 4 (newline handling): " << (result ? "PASS" : "FAIL") << std::endl;
    }
    
    // Test case 5: Path traversal attempt (security test)
    {
        bool result = verifyPassword("../etc/passwd", "anypassword");
        std::cout << "Test 5 (path traversal blocked): " << (!result ? "PASS" : "FAIL") << std::endl;
    }
    
    return 0;
}


#include <iostream>
#include <string>
#include <cctype>

std::string encrypt(const std::string& input) {
    if (input.empty()) {
        return input;
    }
    
    std::string encrypted = "";
    int shift = 3; // Caesar cipher shift
    
    for (size_t i = 0; i < input.length(); i++) {
        char ch = input[i];
        
        if (isupper(ch)) {
            char encryptedChar = ((ch - 'A' + shift) % 26) + 'A';
            encrypted += encryptedChar;
        } else if (islower(ch)) {
            char encryptedChar = ((ch - 'a' + shift) % 26) + 'a';
            encrypted += encryptedChar;
        } else if (isdigit(ch)) {
            char encryptedChar = ((ch - '0' + shift) % 10) + '0';
            encrypted += encryptedChar;
        } else {
            encrypted += ch;
        }
    }
    
    return encrypted;
}

int main() {
    // Test case 1: Simple lowercase string
    std::string test1 = "hello";
    std::cout << "Test 1 - Input: " << test1 << std::endl;
    std::cout << "Encrypted: " << encrypt(test1) << std::endl;
    std::cout << std::endl;
    
    // Test case 2: Mixed case string
    std::string test2 = "HelloWorld";
    std::cout << "Test 2 - Input: " << test2 << std::endl;
    std::cout << "Encrypted: " << encrypt(test2) << std::endl;
    std::cout << std::endl;
    
    // Test case 3: String with numbers
    std::string test3 = "Test123";
    std::cout << "Test 3 - Input: " << test3 << std::endl;
    std::cout << "Encrypted: " << encrypt(test3) << std::endl;
    std::cout << std::endl;
    
    // Test case 4: String with special characters
    std::string test4 = "Hello, World! 2024";
    std::cout << "Test 4 - Input: " << test4 << std::endl;
    std::cout << "Encrypted: " << encrypt(test4) << std::endl;
    std::cout << std::endl;
    
    // Test case 5: Edge case with letters near end of alphabet
    std::string test5 = "xyz XYZ 789";
    std::cout << "Test 5 - Input: " << test5 << std::endl;
    std::cout << "Encrypted: " << encrypt(test5) << std::endl;
    std::cout << std::endl;
    
    return 0;
}

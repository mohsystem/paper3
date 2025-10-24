#include <iostream>
#include <string>
#include <cctype>

/**
 * Encrypts a string using the Caesar cipher algorithm.
 * @param text The string to encrypt.
 * @param shift The number of positions to shift letters.
 * @return The encrypted string.
 */
std::string encrypt(const std::string& text, int shift) {
    std::string result = "";
    for (char c : text) {
        if (c >= 'a' && c <= 'z') {
            result += (char)(((c - 'a' + shift) % 26) + 'a');
        } else if (c >= 'A' && c <= 'Z') {
            result += (char)(((c - 'A' + shift) % 26) + 'A');
        } else {
            result += c;
        }
    }
    return result;
}

int main() {
    // Test Case 1
    std::string text1 = "Hello, World!";
    int shift1 = 3;
    std::cout << "Test Case 1:" << std::endl;
    std::cout << "Original:  " << text1 << std::endl;
    std::cout << "Shift:     " << shift1 << std::endl;
    std::cout << "Encrypted: " << encrypt(text1, shift1) << std::endl;
    std::cout << "--------------------" << std::endl;

    // Test Case 2
    std::string text2 = "Programming is fun!";
    int shift2 = 7;
    std::cout << "Test Case 2:" << std::endl;
    std::cout << "Original:  " << text2 << std::endl;
    std::cout << "Shift:     " << shift2 << std::endl;
    std::cout << "Encrypted: " << encrypt(text2, shift2) << std::endl;
    std::cout << "--------------------" << std::endl;
    
    // Test Case 3
    std::string text3 = "Caesar Cipher";
    int shift3 = 13; // ROT13
    std::cout << "Test Case 3:" << std::endl;
    std::cout << "Original:  " << text3 << std::endl;
    std::cout << "Shift:     " << shift3 << std::endl;
    std::cout << "Encrypted: " << encrypt(text3, shift3) << std::endl;
    std::cout << "--------------------" << std::endl;

    // Test Case 4
    std::string text4 = "All your base are belong to us.";
    int shift4 = 1;
    std::cout << "Test Case 4:" << std::endl;
    std::cout << "Original:  " << text4 << std::endl;
    std::cout << "Shift:     " << shift4 << std::endl;
    std::cout << "Encrypted: " << encrypt(text4, shift4) << std::endl;
    std::cout << "--------------------" << std::endl;

    // Test Case 5
    std::string text5 = "Testing with numbers 123 and symbols !@#.";
    int shift5 = 5;
    std::cout << "Test Case 5:" << std::endl;
    std::cout << "Original:  " << text5 << std::endl;
    std::cout << "Shift:     " << shift5 << std::endl;
    std::cout << "Encrypted: " << encrypt(text5, shift5) << std::endl;
    std::cout << "--------------------" << std::endl;

    return 0;
}
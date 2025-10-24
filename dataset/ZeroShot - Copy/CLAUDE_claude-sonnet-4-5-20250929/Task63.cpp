
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstring>

// Simple Base64 decoder
std::string base64_decode(const std::string& encoded) {
    const std::string base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::string decoded;
    std::vector<int> vec(256, -1);
    for (int i = 0; i < 64; i++)
        vec[base64_chars[i]] = i;
    
    int val = 0, valb = -8;
    for (unsigned char c : encoded) {
        if (vec[c] == -1) break;
        val = (val << 6) + vec[c];
        valb += 6;
        if (valb >= 0) {
            decoded.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return decoded;
}

// Simple Base64 encoder for testing
std::string base64_encode(const std::string& input) {
    const std::string base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::string encoded;
    int val = 0, valb = -6;
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (encoded.size() % 4) encoded.push_back('=');
    return encoded;
}

std::string processRequest(const std::string& rawData) {
    try {
        // Step 1 & 2: Decode Base64 (UTF-8 is default in C++)
        std::string decodedString = base64_decode(rawData);
        
        // Step 3 & 4 & 5: Return the decoded data (JSON parsing would require external library)
        // For simplicity, returning the decoded string
        return decodedString;
        
    } catch (const std::exception& e) {
        std::cerr << "Error processing request: " << e.what() << std::endl;
        return "";
    }
}

int main() {
    // Test Case 1: Simple JSON object
    std::string test1 = base64_encode("{\\"name\\":\\"John\\",\\"age\\":30}");
    std::cout << "Test 1: " << processRequest(test1) << std::endl;
    
    // Test Case 2: JSON array
    std::string test2 = base64_encode("[1,2,3,4,5]");
    std::cout << "Test 2: " << processRequest(test2) << std::endl;
    
    // Test Case 3: Nested JSON
    std::string test3 = base64_encode("{\\"user\\":{\\"id\\":1,\\"name\\":\\"Alice\\"}}");
    std::cout << "Test 3: " << processRequest(test3) << std::endl;
    
    // Test Case 4: JSON with special characters
    std::string test4 = base64_encode("{\\"message\\":\\"Hello, World!\\"}");
    std::cout << "Test 4: " << processRequest(test4) << std::endl;
    
    // Test Case 5: Empty JSON object
    std::string test5 = base64_encode("{}");
    std::cout << "Test 5: " << processRequest(test5) << std::endl;
    
    return 0;
}

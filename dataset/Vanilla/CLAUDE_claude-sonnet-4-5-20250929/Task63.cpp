
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>

// Simple Base64 decoding implementation
std::string base64_decode(const std::string& encoded_string) {
    const std::string base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    
    std::string decoded;
    std::vector<int> vec(256, -1);
    for (int i = 0; i < 64; i++)
        vec[base64_chars[i]] = i;
    
    int val = 0, valb = -8;
    for (unsigned char c : encoded_string) {
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

std::string processRequest(const std::string& rawData) {
    try {
        // Step 1: Ensure raw_data is in UTF-8 (assuming input is already UTF-8)
        
        // Step 2: Decode the raw_data using Base64
        std::string decodedData = base64_decode(rawData);
        
        // Step 3 & 4: Deserialize and store the decoded data
        // For simplicity, returning the decoded JSON string as-is
        std::string storedData = decodedData;
        
        // Step 5: Return the stored data
        return storedData;
    } catch (...) {
        return "Error processing request";
    }
}

int main() {
    // Helper function to encode for testing
    auto base64_encode = [](const std::string& input) -> std::string {
        const std::string base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";
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
    };
    
    // Test case 1: Simple JSON object
    std::string test1 = base64_encode("{\\"name\\":\\"John\\",\\"age\\":30}");
    std::cout << "Test 1: " << processRequest(test1) << std::endl;
    
    // Test case 2: JSON with nested object
    std::string test2 = base64_encode("{\\"user\\":{\\"id\\":1,\\"name\\":\\"Alice\\"}}");
    std::cout << "Test 2: " << processRequest(test2) << std::endl;
    
    // Test case 3: JSON with array
    std::string test3 = base64_encode("{\\"items\\":[1,2,3,4,5]}");
    std::cout << "Test 3: " << processRequest(test3) << std::endl;
    
    // Test case 4: JSON with boolean and null
    std::string test4 = base64_encode("{\\"active\\":true,\\"data\\":null}");
    std::cout << "Test 4: " << processRequest(test4) << std::endl;
    
    // Test case 5: Complex JSON
    std::string test5 = base64_encode("{\\"company\\":\\"TechCorp\\",\\"employees\\":[{\\"name\\":\\"Bob\\",\\"role\\":\\"Developer\\"}]}");
    std::cout << "Test 5: " << processRequest(test5) << std::endl;
    
    return 0;
}

#include <iostream>
#include <string>
#include <vector>
#include <cctype>

// --- Base64 decoding function (Public Domain) ---
// This self-contained function handles standard Base64 decoding, including padding.
std::string base64_decode(const std::string& encoded_string) {
    const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    
    auto is_base64 = [&](unsigned char c) {
        return (isalnum(c) || (c == '+') || (c == '/'));
    };

    int in_len = encoded_string.size();
    if (in_len % 4 != 0) {
        std::cerr << "Error: Invalid Base64 length." << std::endl;
        return "";
    }
    
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; 
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                char_array_4[i] = base64_chars.find(char_array_4[i]);
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0x0F) << 4) + ((char_array_4[2] & 0x3C) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x03) << 6) + char_array_4[3];

            for (i = 0; i < 3; i++) {
                ret += char_array_3[i];
            }
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++) {
            char_array_4[j] = 0;
        }

        for (j = 0; j < 4; j++) {
            char_array_4[j] = base64_chars.find(char_array_4[j]);
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0x0F) << 4) + ((char_array_4[2] & 0x3C) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x03) << 6) + char_array_4[3];

        for (j = 0; j < i - 1; j++) {
            ret += char_array_3[j];
        }
    }

    return ret;
}
// --- End of Base64 decoding function ---

/**
 * @brief Processes a request by decoding a Base64 encoded UTF-8 string.
 * 
 * In a real application, the returned string (assumed to be JSON/XML)
 * would be parsed by a dedicated library (e.g., nlohmann/json, RapidJSON).
 * This function performs the decoding step and returns the raw deserialized string.
 * 
 * @param base64Utf8Data The Base64 encoded UTF-8 string.
 * @return The decoded string, or an empty string on failure.
 */
std::string processRequest(const std::string& base64Utf8Data) {
    if (base64Utf8Data.empty()) {
        return "";
    }
    // Note: A robust implementation should also validate the input string
    // for invalid characters. This simplified version relies on the decoder's behavior.
    std::string decodedString = base64_decode(base64Utf8Data);
    return decodedString;
}

int main() {
    std::string testCases[] = {
        // Test Case 1: Simple valid JSON
        "eyJpZCI6IDEyMywgInVzZXIiOiAidGVzdCJ9",
        // Test Case 2: More complex JSON
        "eyJuYW1lIjogIkJvYiIsICJhY3RpdmUiOiB0cnVlLCAicm9sZXMiOiBbImFkbWluIiwgImVkaXRvciJdfQ==",
        // Test Case 3: Empty JSON object
        "e30=",
        // Test Case 4: Invalid Base64 string (will fail length check and return empty)
        "invalid-base64@#$",
        // Test Case 5: Valid Base64, but not valid JSON
        "VGhpcyBpcyBub3QgSlNPTg=="
    };

    for (int i = 0; i < 5; ++i) {
        std::cout << "--- Test Case " << (i + 1) << " ---" << std::endl;
        std::cout << "Input: " << testCases[i] << std::endl;
        std::string result = processRequest(testCases[i]);
        if (!result.empty() || testCases[i].empty() || testCases[i] == "e30=") {
             // Here, you would normally pass 'result' to a JSON parser.
            std::cout << "Output: " << result << std::endl;
        } else {
            std::cout << "Output: Processing failed." << std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}
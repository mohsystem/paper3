#include <iostream>
#include <string>
#include <vector>
#include <optional>

// --- BEGIN: Self-contained Base64 decoder ---
// Copyright (C) 2004-2008 René Nyffenegger
// This source code is provided 'as-is', without any express or implied
// warranty. In no event will the author be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this source code must not be misrepresented; you must not
//    claim that you wrote the original source code. If you use this source code
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original source code.
// 3. This notice may not be removed or altered from any source distribution.
namespace base64 {
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    static inline bool is_base64(unsigned char c) {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }

    std::optional<std::string> b64_decode(std::string const& encoded_string) {
        size_t in_len = encoded_string.size();
        int i = 0;
        int j = 0;
        int in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        std::string ret;

        while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
            char_array_4[i++] = encoded_string[in_]; in_++;
            if (i == 4) {
                for (i = 0; i < 4; i++)
                    char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; (i < 3); i++)
                    ret += char_array_3[i];
                i = 0;
            }
        }

        if (i) {
            for (j = i; j < 4; j++)
                char_array_4[j] = 0;

            for (j = 0; j < 4; j++)
                char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
        }
        
        // Final validation: check for non-base64 characters in the rest of the string
        while(in_ < encoded_string.length()) {
            if (encoded_string[in_] != '=' && !isspace(encoded_string[in_])) {
                return std::nullopt; // Invalid character found
            }
            in_++;
        }

        return ret;
    }
}
// --- END: Self-contained Base64 decoder ---


// --- BEGIN: Self-contained JSON parser (nlohmann/json) ---
// To keep the example self-contained and compilable, you would normally
// include a single-header json library here. For brevity, we will use
// a well-known library `nlohmann/json` and assume it is available.
// Please download `json.hpp` from https://github.com/nlohmann/json
// and place it in your include path.
#include "json.hpp"
// --- END: Self-contained JSON parser ---

using json = nlohmann::json;

/**
 * @brief Processes a request by decoding and deserializing the raw data.
 * @param rawData A std::string containing UTF-8, Base64 encoded JSON data.
 * @return An optional containing the parsed json object, or std::nullopt on failure.
 */
std::optional<json> processRequest(const std::string& rawData) {
    // 1. & 2. Retrieve raw_data and decode from Base64
    auto decoded_data = base64::b64_decode(rawData);
    
    if (!decoded_data) {
        return std::nullopt; // Base64 decoding failed
    }
    
    // 3. Deserialize the decoded data (JSON)
    try {
        // 4. Store the deserialized data in a variable.
        json data = json::parse(*decoded_data);
        // 5. Return the stored data.
        return data;
    } catch (json::parse_error& e) {
        // JSON parsing failed
        return std::nullopt;
    }
}

int main() {
    std::vector<std::string> testCases = {
        // Test Case 1: Valid input
        "eyJ1c2VyIjogInRlc3QiLCAiaWQiOiAxMjMsICJhY3RpdmUiOiB0cnVlfQ==",
        // Test Case 2: Another valid input
        "eyJoZWxsbyI6ICJ3b3JsZCJ9",
        // Test Case 3: Invalid Base64
        "this-is-not-base64",
        // Test Case 4: Valid Base64, but not JSON
        "aGVsbG8gd29ybGQ=", // "hello world"
        // Test Case 5: Empty input
        ""
    };

    int i = 1;
    for (const auto& testCase : testCases) {
        std::cout << "Test Case " << i++ << ":" << std::endl;
        std::cout << "Input: \"" << testCase << "\"" << std::endl;
        auto result = processRequest(testCase);
        if (result) {
            std::cout << "Output: " << result->dump(2) << std::endl;
        } else {
            std::cout << "Output: Processing failed (null)" << std::endl;
        }
        std::cout << "--------------------" << std::endl;
    }
    return 0;
}
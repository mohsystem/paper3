#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

/*
   ______                     _________            __
  / ____/___  ____ ___  ____  / ____/ (_)__  ____  / /_
 / /   / __ \/ __ `__ \/ __ \/ /   / / / _ \/ __ \/ __/
/ /___/ /_/ / / / / / / /_/ / /___/ / /  __/ / / / /_
\____/\____/_/ /_/ /_/\____/\____/_/_/\___/_/ /_/\__/
v3.11.3
This file is a slimmed-down version of the original json.hpp from
https://github.com/nlohmann/json, containing only the necessary components
for parsing and basic operations to keep the file size manageable.
The original file is available under the MIT License.
*/
#include <utility>
#include <cstddef>
namespace nlohmann {
    class json {
    public:
        using value_t = enum class value_t : std::uint8_t { null, object, array, string, boolean, number_integer, number_unsigned, number_float, binary, discarded };
        json() = default;
        json(std::nullptr_t) {}
        json(value_t v) { if (v == value_t::object) { m_value.object = new std::map<std::string, json>(); } /* simplified */ }
        static json parse(const std::string& s);
        bool is_null() const { return m_type == value_t::null; }
        std::string dump(int indent = -1) const;
        // Simplified representation
    private:
        value_t m_type = value_t::null;
        union json_value {
            std::map<std::string, json>* object;
            std::vector<json>* array;
            std::string* string;
            bool boolean;
            // other types omitted for brevity
        } m_value;
        // A real implementation is vastly more complex
    };
} // namespace nlohmann
// NOTE: The above nlohmann::json class is a placeholder.
// The full, working version of nlohmann/json.hpp is extremely large (over 2MB)
// and impractical to include directly in this response.
// The code below is written AS IF the full library were included.
// To compile this code, you must replace the placeholder above with the
// actual content of `json.hpp` from: https://github.com/nlohmann/json/releases

// For demonstration, a mock parse function
namespace nlohmann {
    json json::parse(const std::string& s) {
        if (s.find('{') != std::string::npos && s.find('}') != std::string::npos) {
             // This is a mock parser. It doesn't actually parse.
             // It just creates a dummy object to allow the code to run.
            json j;
            j.m_type = value_t::object;
            j.m_value.string = new std::string(s); // Store the raw string for dumping
            return j;
        } else if (s.empty() || s[0] != '{') {
            throw std::runtime_error("parse_error");
        }
        return json();
    }
    std::string json::dump(int) const {
        if (m_type == value_t::object && m_value.string != nullptr) return *m_value.string;
        return "null";
    }
}


// --- BEGIN 3rd Party Library: C++ Base64 ---
// Copyright (C) 2004-2017 René Nyffenegger
// This source code is provided 'as-is', without any express or implied warranty.
// In no event will the author be held liable for any damages arising from the use of this software.
// The complete library can be found at: https://github.com/ReneNyffenegger/cpp-base64
static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_decode(std::string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}
// --- END 3rd Party Library: C++ Base64 ---


// The main class is conceptually 'Task63'
class Task63 {
public:
    /**
     * Processes a raw request by Base64 decoding and JSON deserializing the data.
     *
     * @param rawData The Base64 encoded string from the request.
     * @return A json object containing the deserialized data. Returns a null json object on error.
     */
    static nlohmann::json processRequest(const std::string& rawData) {
        if (rawData.empty()) {
            std::cerr << "Error: Input data is empty." << std::endl;
            return nullptr;
        }

        try {
            // Step 1 & 2: Decode the Base64 string.
            std::string decoded_string = base64_decode(rawData);
            if (decoded_string.empty() && !rawData.empty()) {
                 throw std::runtime_error("Base64 decoding failed or produced empty result.");
            }

            // Step 3: Deserialize the decoded data using a safe JSON parser.
            // SECURITY: nlohmann/json is a widely used and safe C++ JSON library.
            // It parses data into structures and does not execute arbitrary code,
            // which prevents deserialization vulnerabilities like RCE.
            nlohmann::json deserialized_data = nlohmann::json::parse(decoded_string);
            
            // Step 4 & 5: Store and return the data.
            return deserialized_data;
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return nullptr;
        } catch (...) {
            // Catching potential parse errors from a real nlohmann::json library
            std::cerr << "Error: Failed to deserialize JSON. The decoded data is not valid JSON." << std::endl;
            return nullptr;
        }
    }
};

int main() {
    std::vector<std::string> testCases = {
        // Test Case 1: Valid JSON data
        "eyJ1c2VyIjogImFkbWluIiwgImlkIjogMTIzfQ==", // Decodes to: {"user": "admin", "id": 123}
        // Test Case 2: Another valid JSON
        "eyJrZXkiOiAidmFsdWUiLCAibnVtYmVyIjogNDJ9", // Decodes to: {"key": "value", "number": 42}
        // Test Case 3: Invalid Base64 string
        "invalid-base64-string",
        // Test Case 4: Valid Base64, but the content is not JSON
        "SGVsbG8gV29ybGQ=", // Decodes to: "Hello World"
        // Test Case 5: Empty string
        ""
    };

    int testNum = 1;
    for (const auto& testCase : testCases) {
        std::cout << "--- Test Case " << testNum++ << " ---" << std::endl;
        std::cout << "Input: " << testCase << std::endl;
        nlohmann::json result = Task63::processRequest(testCase);
        if (!result.is_null()) {
            // The mock `dump` will just print the decoded string.
            // A real implementation would pretty-print the JSON.
            std::cout << "Processed Data: " << result.dump(2) << std::endl;
        } else {
            std::cout << "Processing failed." << std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}
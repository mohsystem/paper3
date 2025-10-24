#include <iostream>
#include <string>
#include <vector>
#include <cctype>

// Self-contained Base64 decoding function
// (Adapted from a public-domain implementation)
namespace Base64 {
    const std::string base64_chars =
                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                 "abcdefghijklmnopqrstuvwxyz"
                 "0123456789+/";

    static inline bool is_base64(unsigned char c) {
      return (isalnum(c) || (c == '+') || (c == '/'));
    }

    std::string decode(const std::string& encoded_string) {
      size_t in_len = encoded_string.size();
      size_t i = 0;
      size_t j = 0;
      size_t in_ = 0;
      unsigned char char_array_4[4], char_array_3[3];
      std::string ret;
      ret.reserve(in_len * 3 / 4);

      while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i == 4) {
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
        for (j = i; j < 4; j++)
          char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
          char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
      }

      return ret;
    }
} // namespace Base64

/**
 * Processes a raw request by Base64 decoding the data.
 *
 * @param rawData The Base64 encoded, UTF-8 string data.
 * @return The deserialized data as a std::string (in this case, a JSON string).
 *         For full deserialization into a C++ object, a library like nlohmann/json would be used.
 */
std::string processRequest(const std::string& rawData) {
    // Step 1: Retrieve the raw_data. (Passed as parameter)
    // Step 2: Decode the raw_data using Base64.
    // Step 3: "Deserialize" to string format.
    std::string deserializedData = Base64::decode(rawData);

    // Step 4 & 5: Store the data in a variable and return it.
    return deserializedData;
}

class Task63 {
public:
    static void runTests() {
        std::vector<std::string> testCases = {
            "eyJpZCI6IDEyMywgIm5hbWUiOiAidGVzdF91c2VyIiwgImFjdGl2ZSI6IHRydWV9",
            "eyJtZXNzYWdlIjogIkhlbGxvLCBXb3JsZCEifQ==",
            "eyJkYXRhIjogWzEsIDIsIDNdLCAic3RhdHVzIjogIm9rIn0=",
            "eyJlcnJvciI6ICJOb3QgRm91bmQiLCAiY29kZSI6IDQwNH0=",
            "e30="
        };
        
        std::cout << "--- C++ Test Cases ---" << std::endl;
        for (size_t i = 0; i < testCases.size(); ++i) {
            std::cout << "Test Case " << (i + 1) << ":" << std::endl;
            const std::string& rawData = testCases[i];
            std::cout << "Input (Base64): " << rawData << std::endl;
            std::string result = processRequest(rawData);
            std::cout << "Output (Deserialized): " << result << std::endl;
            std::cout << std::endl;
        }
    }
};

int main() {
    Task63::runTests();
    return 0;
}
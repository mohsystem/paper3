/*
 * Requires the nlohmann/json library (a single header file).
 * Download `json.hpp` from https://github.com/nlohmann/json/releases
 * and place it in your project's include path.
 *
 * Compile command example:
 * g++ -std=c++17 -I/path/to/include/dir -o task62 Task62.cpp
 */
#include <iostream>
#include <string>
#include <vector>
#include <optional>

// The nlohmann/json library is robust and widely used for secure JSON handling.
// It must be available in the include path.
#include "json.hpp"

// for convenience
using json = nlohmann::json;

/**
 * Parses a JSON string and returns its root element.
 * Uses the nlohmann/json library, which provides safe parsing and
 * throws exceptions on errors, preventing crashes on malformed input.
 *
 * @param jsonString The JSON string to parse.
 * @return An optional containing the json object if parsing is successful,
 *         otherwise an empty optional.
 */
std::optional<json> getJsonRoot(const std::string& jsonString) {
    if (jsonString.empty()) {
        std::cerr << "Error: Input JSON string is empty." << std::endl;
        return std::nullopt;
    }
    try {
        // The parse function is hardened against common exploits.
        // The `false` second argument disables exceptions but we use a try-catch
        // block for better error reporting.
        json rootElement = json::parse(jsonString);
        return rootElement;
    } catch (const json::parse_error& e) {
        // Securely handle parsing errors without exposing excessive internal detail.
        std::cerr << "Error parsing JSON string: " << e.what() << std::endl;
        return std::nullopt;
    }
}

int main() {
    std::vector<std::string> testCases = {
        // 1. Valid JSON object
        "{\"name\": \"John Doe\", \"age\": 30, \"isStudent\": false, \"courses\": [\"Math\", \"Science\"]}",
        // 2. Valid JSON array
        "[1, \"test\", true, null, {\"key\": \"value\"}]",
        // 3. Valid JSON primitive (string)
        "\"Hello, Secure World!\"",
        // 4. Valid JSON primitive (number)
        "123.456",
        // 5. Invalid JSON string (malformed)
        "{\"name\": \"Jane Doe\", \"age\":}"
    };

    for (size_t i = 0; i < testCases.size(); ++i) {
        std::cout << "--- Test Case " << (i + 1) << " ---" << std::endl;
        std::cout << "Input: " << testCases[i] << std::endl;
        
        std::optional<json> rootElementOpt = getJsonRoot(testCases[i]);
        
        if (rootElementOpt) {
            // Use .value() or * to access the contained json object
            const json& rootElement = rootElementOpt.value();
            std::cout << "Root Element Type: " << rootElement.type_name() << std::endl;
            // The dump() method pretty-prints the JSON with an indent of 4 spaces
            std::cout << "Parsed Output: " << rootElement.dump(4) << std::endl;
        } else {
            std::cout << "Failed to parse JSON." << std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}
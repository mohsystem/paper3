#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stdexcept>

// Helper function to trim whitespace from both ends of a string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (std::string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, (last - first + 1));
}

/**
 * Parses the provided string to identify the type of the root JSON element.
 *
 * @param jsonString The string representing a JSON document.
 * @return A string indicating the root element's type ("Object", "Array", "String",
 *         "Number", "Boolean", "Null", or "Invalid").
 */
std::string getRootElementType(const std::string& jsonString) {
    std::string trimmed = trim(jsonString);

    if (trimmed.empty()) {
        return "Invalid";
    }

    char firstChar = trimmed.front();
    char lastChar = trimmed.back();

    if (firstChar == '{' && lastChar == '}') {
        return "Object";
    }
    if (firstChar == '[' && lastChar == ']') {
        return "Array";
    }
    if (firstChar == '"' && lastChar == '"') {
        return "String";
    }
    if (trimmed == "true" || trimmed == "false") {
        return "Boolean";
    }
    if (trimmed == "null") {
        return "Null";
    }

    // Try to parse as a number
    try {
        size_t pos;
        std::stod(trimmed, &pos);
        // Ensure the entire string was consumed
        if (pos == trimmed.length()) {
            return "Number";
        }
    } catch (const std::invalid_argument&) {
        // Not a number
    } catch (const std::out_of_range&) {
        // Number is out of range, but still a number syntactically
        return "Number";
    }

    return "Invalid";
}

int main() {
    std::vector<std::string> testCases = {
        "  { \"name\": \"John\", \"age\": 30 }  ", // Test case 1: Object
        "[1, \"apple\", true, null]",              // Test case 2: Array
        "\"This is a root string.\"",              // Test case 3: String
        "  -123.45e6  ",                          // Test case 4: Number
        "null"                                     // Test case 5: Null
    };

    std::cout << "Running C++ Test Cases:" << std::endl;
    for (int i = 0; i < testCases.size(); ++i) {
        std::string json = testCases[i];
        std::string result = getRootElementType(json);
        std::cout << "Test Case " << (i + 1) << ": \"" << json << "\" -> " << result << std::endl;
    }

    return 0;
}
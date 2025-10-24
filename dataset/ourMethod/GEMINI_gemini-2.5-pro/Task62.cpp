#include <iostream>
#include <string>
#include <vector>
#include <cctype>

// Helper function to trim whitespace from both ends of a string
std::string trim(const std::string& str) {
    const char* whitespace = " \t\n\r\f\v";
    size_t first = str.find_first_not_of(whitespace);
    if (std::string::npos == first) {
        return "";
    }
    size_t last = str.find_last_not_of(whitespace);
    return str.substr(first, (last - first + 1));
}

/**
 * @brief Identifies the root element type of a JSON-like string.
 *
 * This is a non-validating heuristic. It checks the first and last
 * non-whitespace characters and attempts to parse the string as a known
 * literal or a number. It does not perform a full, validating JSON parse.
 *
 * @param jsonString The string to analyze.
 * @return A string representing the root element type ("object", "array", "string",
 *         "number", "boolean", "null") or "invalid" if it cannot be determined.
 */
std::string getRootElementType(const std::string& jsonString) {
    std::string trimmed = trim(jsonString);

    if (trimmed.empty()) {
        return "invalid";
    }

    if (trimmed.length() > 1) {
        if (trimmed.front() == '{' && trimmed.back() == '}') {
            return "object";
        }
        if (trimmed.front() == '[' && trimmed.back() == ']') {
            return "array";
        }
        if (trimmed.front() == '"' && trimmed.back() == '"') {
            return "string";
        }
    }

    if (trimmed == "true" || trimmed == "false") {
        return "boolean";
    }
    if (trimmed == "null") {
        return "null";
    }

    // Check for number using std::stod
    try {
        size_t pos;
        std::stod(trimmed, &pos);
        // Ensure the entire string was consumed by the number parser
        if (pos == trimmed.length()) {
            return "number";
        }
    } catch (const std::invalid_argument&) {
        // Not a number, fall through
    } catch (const std::out_of_range&) {
        // Is a number, but out of range of double, still counts as number type
        return "number";
    }
    
    return "invalid";
}

int main() {
    std::vector<std::pair<std::string, std::string>> test_cases = {
        {"  { \"key\": \"value\" }  ", "object"},
        {" [1, 2, 3] ", "array"},
        {"\"a string\"", "string"},
        {"-1.2e-3", "number"},
        {"true", "boolean"}
    };

    std::cout << "Running CPP tests..." << std::endl;
    int i = 1;
    for (const auto& test_case : test_cases) {
        std::string result = getRootElementType(test_case.first);
        std::string status = (result == test_case.second) ? "PASS" : "FAIL";
        std::cout << "Test Case " << i++ << ": -> "
                  << result << " (Expected: " << test_case.second << ") -> " << status << std::endl;
    }

    return 0;
}
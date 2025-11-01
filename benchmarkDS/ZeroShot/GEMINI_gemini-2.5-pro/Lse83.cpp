#include <iostream>
#include <string>
#include <vector>

/*
 * NOTE: C++ does not have a standard library for creating a web server.
 * Building a secure web application requires external libraries like Boost.Asio,
 * crow, or Pistache. The following code simulates the core logic of
 * processing a payload string, as a full web application cannot be created
 * in a single, standard C++ file.
 */

/**
 * @brief Securely extracts the value of a 'name' field from a JSON-like string.
 *
 * This function performs a simple search for the "name" key. A production
 * system should use a robust JSON parsing library (e.g., nlohmann/json, rapidjson)
 * to handle complex cases and ensure security.
 *
 * @param payload The raw string payload from a request.
 * @return The value of the 'name' field, or a "not found" message.
 */
std::string checkNameInPayload(const std::string& payload) {
    if (payload.empty()) {
        return "Field 'name' not found.";
    }

    // A simple, non-robust search for {"name":"value"}
    std::string key = "\"name\":\"";
    size_t start_pos = payload.find(key);

    if (start_pos == std::string::npos) {
        return "Field 'name' not found.";
    }

    start_pos += key.length();

    size_t end_pos = payload.find('"', start_pos);

    if (end_pos == std::string::npos) {
        return "Field 'name' not found (malformed payload).";
    }

    // Security: Limit the length of the extracted name to prevent abuse.
    const size_t max_len = 1024;
    if (end_pos - start_pos > max_len) {
        return "[Error: Name is too long]";
    }

    return payload.substr(start_pos, end_pos - start_pos);
}

int main() {
    // Test cases for the payload processing logic.
    std::vector<std::string> test_cases;

    // Test Case 1: Valid payload with a 'name' field.
    test_cases.push_back("{\"user\":\"guest\",\"name\":\"Charlie\"}");

    // Test Case 2: Payload without a 'name' field.
    test_cases.push_back("{\"id\":456,\"role\":\"admin\"}");

    // Test Case 3: Empty payload.
    test_cases.push_back("");
    
    // Test Case 4: Malformed payload (missing closing quote).
    test_cases.push_back("{\"name\":\"David");

    // Test Case 5: Payload with script content to test XSS handling.
    // The logic here just extracts the string. In a web context, this string
    // MUST be properly encoded (e.g., HTML escaped) before being rendered.
    test_cases.push_back("{\"name\":\"Eve <script>attack()</script>\"}");

    for (size_t i = 0; i < test_cases.size(); ++i) {
        std::cout << "Test Case " << i + 1 << ":\n";
        std::cout << "  Payload: " << test_cases[i] << "\n";
        std::string result = checkNameInPayload(test_cases[i]);
        std::cout << "  Result: " << result << "\n---\n";
    }

    return 0;
}
#include <iostream>
#include <string>
#include <vector>

// Note: Creating a full web application in a single C++ file without external
// libraries is highly complex. It involves low-level socket programming and
// manual HTTP parsing.
//
// The following code simulates the core logic: processing a payload string
// to find and extract the 'name' field. This function could be integrated
// into a web server built with a library like Boost.Asio, cpp-httplib, or Crow.

/**
 * @brief Parses a URL-encoded string payload to find the value of the 'name' field.
 * 
 * @param payload The input string, formatted as "key1=value1&key2=value2...".
 * @return The value of the 'name' field if found, otherwise a message indicating it was not found.
 */
std::string processPayload(const std::string& payload) {
    if (payload.empty()) {
        return "Payload is empty.";
    }

    // Search for "name="
    // We look for "&name=" or the payload starting with "name=" to avoid matching fields like "username=".
    std::string name_key = "name=";
    size_t start_pos = payload.find(name_key);

    // If "name=" is found, but not at the beginning, check if it's preceded by '&'
    if (start_pos != 0 && start_pos != std::string::npos) {
        if (payload[start_pos - 1] != '&') {
            // It might be part of another key (e.g., "firstname="), so search again from after this point
             start_pos = payload.find("&" + name_key, start_pos);
             if (start_pos != std::string::npos) {
                // If we found "&name=", adjust start_pos to point after the '&'
                start_pos++;
             }
        }
    }
    
    if (start_pos == std::string::npos) {
        return "Field 'name' not found in payload.";
    }

    // The value starts right after "name="
    size_t value_start = start_pos + name_key.length();
    
    // The value ends at the next '&' or at the end of the string
    size_t value_end = payload.find('&', value_start);
    
    if (value_end == std::string::npos) {
        // No more '&', so the value is the rest of the string
        return payload.substr(value_start);
    } else {
        // The value is between value_start and the next '&'
        return payload.substr(value_start, value_end - value_start);
    }
}

int main() {
    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: 'name' field in the middle
    std::string payload1 = "id=1&name=Alice&role=user";
    std::cout << "Test 1 Payload: " << payload1 << std::endl;
    std::cout << "Test 1 Result: " << processPayload(payload1) << std::endl; // Expected: Alice

    // Test Case 2: 'name' field is missing
    std::string payload2 = "id=2&role=admin";
    std::cout << "Test 2 Payload: " << payload2 << std::endl;
    std::cout << "Test 2 Result: " << processPayload(payload2) << std::endl; // Expected: Field 'name' not found...

    // Test Case 3: Empty payload
    std::string payload3 = "";
    std::cout << "Test 3 Payload: (empty)" << std::endl;
    std::cout << "Test 3 Result: " << processPayload(payload3) << std::endl; // Expected: Payload is empty.

    // Test Case 4: 'name' is the first field
    std::string payload4 = "name=Bob&id=3";
    std::cout << "Test 4 Payload: " << payload4 << std::endl;
    std::cout << "Test 4 Result: " << processPayload(payload4) << std::endl; // Expected: Bob
    
    // Test Case 5: 'name' is the last field
    std::string payload5 = "id=4&role=guest&name=Charlie";
    std::cout << "Test 5 Payload: " << payload5 << std::endl;
    std::cout << "Test 5 Result: " << processPayload(payload5) << std::endl; // Expected: Charlie

    std::cout << "--------------------------" << std::endl;

    return 0;
}
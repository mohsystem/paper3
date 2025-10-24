#include <iostream>
#include <string>
#include <optional>
#include <vector>

// For simplicity and to meet the "one source file" requirement, a popular
// header-only JSON library (nlohmann/json) is included here.
// In a real project, you would use a package manager or git submodule.
// Source: https://github.com/nlohmann/json
#include <nlohmann/json.hpp>

// Use the nlohmann::json library
using json = nlohmann::json;

/**
 * A simple data struct to hold user information.
 * It contains only data members, making it a safe target for deserialization.
 */
struct UserData {
    int id;
    std::string name;
    bool isAdmin;
};

/**
 * Securely deserializes a JSON string into a UserData struct.
 *
 * SECURITY NOTE: This function is secure because it uses a well-vetted, data-only
 * JSON parser. It avoids custom or complex binary parsing routines that could be
 * vulnerable to buffer overflows or other memory corruption bugs. The use of
 * try-catch blocks prevents crashes on malformed input.
 *
 * @param jsonString The user-supplied JSON string.
 * @return An std::optional<UserData> containing the object if deserialization
 *         is successful, or std::nullopt otherwise.
 */
std::optional<UserData> deserializeUserData(const std::string& jsonString) {
    if (jsonString.empty()) {
        std::cerr << "Error: Input JSON string is empty." << std::endl;
        return std::nullopt;
    }

    try {
        // json::parse can throw an exception on malformed input, which we catch.
        json j = json::parse(jsonString);

        UserData user;
        
        // Safely extract values. The at() method throws if a key is not found,
        // while value() provides a default. We will check for existence.
        if (j.contains("id") && j["id"].is_number_integer()) {
            user.id = j["id"];
        } else {
            std::cerr << "Error: 'id' field is missing or not an integer." << std::endl;
            return std::nullopt;
        }

        if (j.contains("name") && j["name"].is_string()) {
            user.name = j["name"];
        } else {
            std::cerr << "Error: 'name' field is missing or not a string." << std::endl;
            return std::nullopt;
        }

        if (j.contains("isAdmin") && j["isAdmin"].is_boolean()) {
            user.isAdmin = j["isAdmin"];
        } else {
            std::cerr << "Error: 'isAdmin' field is missing or not a boolean." << std::endl;
            return std::nullopt;
        }

        return user;
    } catch (json::parse_error& e) {
        std::cerr << "Error deserializing JSON: " << e.what() << std::endl;
        return std::nullopt;
    }
}

int main() {
    // --- Test Cases ---
    std::vector<std::string> testCases = {
        // Test Case 1: Valid JSON
        R"({"id": 301, "name": "Ivan", "isAdmin": false})",
        // Test Case 2: JSON with extra, unknown fields (will be safely ignored)
        R"({"id": 302, "name": "Judy", "isAdmin": true, "city": "NYC"})",
        // Test Case 3: Malformed JSON (extra comma)
        R"({"id": 303, "name": "Mallory", "isAdmin": false,})",
        // Test Case 4: JSON with a missing field ("isAdmin")
        R"({"id": 304, "name": "Mike"})",
        // Test Case 5: JSON with wrong data type
        R"({"id": "305", "name": "Nancy", "isAdmin": false})"
    };

    std::cout << "--- Running C++ Test Cases ---" << std::endl;
    for (int i = 0; i < testCases.size(); ++i) {
        std::cout << "\n--- Test Case " << (i + 1) << " ---" << std::endl;
        std::cout << "Input: " << testCases[i] << std::endl;
        auto userOpt = deserializeUserData(testCases[i]);
        if (userOpt) {
            UserData user = userOpt.value();
            std::cout << "Deserialized successfully: UserData{id=" << user.id
                      << ", name='" << user.name << "', isAdmin=" << std::boolalpha
                      << user.isAdmin << "}" << std::endl;
        } else {
            std::cout << "Deserialization failed." << std::endl;
        }
    }
    return 0;
}
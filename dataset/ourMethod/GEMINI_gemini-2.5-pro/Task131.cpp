#include <iostream>
#include <string>
#include <optional>
#include <stdexcept>
#include <limits>

struct UserData {
    int id;
    std::string name;

    bool operator==(const UserData& other) const {
        return id == other.id && name == other.name;
    }
};

/**
 * Serializes a UserData object into a simple string format.
 * Format: "id=<integer>;name=<string>"
 * @param user The UserData object to serialize.
 * @return A string representation of the user data.
 */
std::string serialize(const UserData& user) {
    return "id=" + std::to_string(user.id) + ";name=" + user.name;
}

/**
 * Deserializes a string into a UserData object.
 * This function uses a simple, custom data-only format to avoid
 * the security risks associated with complex deserialization libraries.
 * It returns std::optional to safely handle parsing failures.
 *
 * @param data The string to deserialize.
 * @return An std::optional<UserData> containing the object on success, or std::nullopt on failure.
 */
std::optional<UserData> deserialize(const std::string& data) {
    if (data.empty()) {
        return std::nullopt;
    }

    const std::string id_prefix = "id=";
    const std::string name_prefix = ";name=";
    const size_t MAX_NAME_LENGTH = 256;

    size_t id_start = data.find(id_prefix);
    size_t name_start = data.find(name_prefix);

    // Validate format and order of tokens
    if (id_start != 0 || name_start == std::string::npos || name_start <= id_start) {
        return std::nullopt;
    }

    size_t id_val_start = id_prefix.length();
    size_t id_val_len = name_start - id_val_start;
    std::string id_str = data.substr(id_val_start, id_val_len);

    size_t name_val_start = name_start + name_prefix.length();
    std::string name_str = data.substr(name_val_start);

    if (name_str.empty() || name_str.length() > MAX_NAME_LENGTH) {
        return std::nullopt;
    }

    try {
        size_t pos;
        int id = std::stoi(id_str, &pos);
        // Ensure the entire ID string was consumed
        if (pos != id_str.length()) {
            return std::nullopt;
        }
        return UserData{id, name_str};
    } catch (const std::invalid_argument&) {
        return std::nullopt; // Not an integer
    } catch (const std::out_of_range&) {
        return std::nullopt; // Integer out of range
    }
}

void run_test(int test_num, const std::string& description, const std::string& input, bool expect_success) {
    std::cout << "Test " << test_num << " (" << description << "): ";
    auto result = deserialize(input);
    if (result.has_value() == expect_success) {
        std::cout << "PASSED";
        if (result) {
            std::cout << " - id=" << result->id << ", name='" << result->name << "'";
        }
    } else {
        std::cout << "FAILED";
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "--- C++ Deserialization Tests ---" << std::endl;

    // Test Case 1: Valid data
    UserData user1 = {101, "Alice"};
    std::string serialized1 = serialize(user1);
    run_test(1, "Valid", serialized1, true);

    // Test Case 2: Invalid format (wrong order)
    std::string serialized2 = "name=Bob;id=102";
    run_test(2, "Wrong Order", serialized2, false);

    // Test Case 3: Invalid ID (not a number)
    std::string serialized3 = "id=abc;name=Charlie";
    run_test(3, "Invalid ID", serialized3, false);

    // Test Case 4: Missing part (no name)
    std::string serialized4 = "id=104";
    run_test(4, "Missing Part", serialized4, false);

    // Test Case 5: Empty input
    std::string serialized5 = "";
    run_test(5, "Empty Input", serialized5, false);

    return 0;
}
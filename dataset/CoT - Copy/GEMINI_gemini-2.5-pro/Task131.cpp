#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>

// C++ does not have a standard built-in serialization library. Deserializing
// user-supplied data requires manual parsing. Key security risks include
// buffer overflows and resource exhaustion attacks. Using standard library
// components like std::string and std::stringstream, along with careful parsing
// and error handling, helps mitigate these risks. This example uses a simple,
// custom key-value string format.

struct UserData {
    std::string name;
    int id;
    bool isActive;

    bool operator==(const UserData& other) const {
        return name == other.name && id == other.id && isActive == other.isActive;
    }
};

/**
 * @brief Serializes a UserData object into a simple string format.
 * Format: "name:value;id:value;active:1or0;"
 */
std::string serialize(const UserData& user) {
    std::stringstream ss;
    ss << "name:" << user.name << ";"
       << "id:" << user.id << ";"
       << "active:" << (user.isActive ? "1" : "0") << ";";
    return ss.str();
}

/**
 * @brief Deserializes a string into a UserData object securely.
 * This function safely parses the custom key-value format. It includes
 * error handling (try-catch block) for parsing integers to prevent crashes
 * on malformed input (e.g., "id:abc").
 * @param data The string data to deserialize.
 * @param outUser A reference to a UserData object to populate.
 * @return True on success, false on parsing failure.
 */
bool deserialize(const std::string& data, UserData& outUser) {
    std::stringstream ss(data);
    std::string segment;

    try {
        while (std::getline(ss, segment, ';')) {
            if (segment.empty()) continue;

            size_t colon_pos = segment.find(':');
            if (colon_pos == std::string::npos) return false; // Malformed segment

            std::string key = segment.substr(0, colon_pos);
            std::string value = segment.substr(colon_pos + 1);

            if (key == "name") {
                outUser.name = value;
            } else if (key == "id") {
                outUser.id = std::stoi(value);
            } else if (key == "active") {
                outUser.isActive = (std::stoi(value) == 1);
            }
        }
    } catch (const std::invalid_argument& ia) {
        std::cerr << "Parsing error: Invalid number format in input data." << std::endl;
        return false;
    } catch (const std::out_of_range& oor) {
        std::cerr << "Parsing error: Number out of range in input data." << std::endl;
        return false;
    }

    return true;
}

std::ostream& operator<<(std::ostream& os, const UserData& user) {
    os << "UserData{name='" << user.name << "', id=" << user.id
       << ", isActive=" << std::boolalpha << user.isActive << "}";
    return os;
}

int main() {
    std::cout << "--- C++ Secure Deserialization Demo ---" << std::endl;

    std::vector<UserData> testCases = {
        {"Alice", 101, true},
        {"Bob", 202, false},
        {"Charlie", 303, true},
        {"David (Admin)", 404, false},
        {"", 0, false}
    };

    int testNum = 1;
    for (const auto& originalUser : testCases) {
        std::cout << "\n--- Test Case " << testNum++ << " ---" << std::endl;
        std::cout << "Original:     " << originalUser << std::endl;

        std::string serializedData = serialize(originalUser);
        std::cout << "Serialized:   " << serializedData << std::endl;

        UserData deserializedUser;
        if (deserialize(serializedData, deserializedUser)) {
            std::cout << "Deserialized: " << deserializedUser << std::endl;
            std::cout << "Objects are equal: " << std::boolalpha << (originalUser == deserializedUser) << std::endl;
        } else {
            std::cout << "Deserialization failed." << std::endl;
        }
    }
    
    std::cout << "\n--- Security Test Case (Malformed Data) ---" << std::endl;
    std::string malformedData = "name:Eve;id:not-a-number;active:1;";
    std::cout << "Attempting to deserialize: " << malformedData << std::endl;
    UserData malformedUser;
    if (!deserialize(malformedData, malformedUser)) {
        std::cout << "SUCCESS: Malformed data was correctly handled and rejected." << std::endl;
    } else {
        std::cout << "FAILURE: Malformed data was not handled correctly." << std::endl;
    }

    return 0;
}
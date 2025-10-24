#include <iostream>
#include <string>
#include <sstream>
#include <vector>

// Simple struct to hold user data
struct UserData {
    int id;
    std::string name;
};

/**
 * Helper function to serialize UserData into a string.
 * Format: [id][name_length][name_data]
 * We use stringstream for a simple binary-like representation.
 */
std::string serializeData(const UserData& user) {
    std::stringstream ss;
    
    // Write the ID
    ss.write(reinterpret_cast<const char*>(&user.id), sizeof(user.id));
    
    // Write the length of the name
    size_t name_len = user.name.length();
    ss.write(reinterpret_cast<const char*>(&name_len), sizeof(name_len));
    
    // Write the name data
    ss.write(user.name.c_str(), name_len);
    
    return ss.str();
}

/**
 * Deserializes user-supplied string data back into a UserData object.
 * @param data The string containing the serialized data.
 * @return A UserData object. Throws an exception on failure.
 */
UserData deserializeData(const std::string& data) {
    UserData user;
    std::stringstream ss(data);
    
    // Read the ID
    ss.read(reinterpret_cast<char*>(&user.id), sizeof(user.id));
    if (ss.fail()) {
        throw std::runtime_error("Failed to read ID during deserialization.");
    }
    
    // Read the length of the name
    size_t name_len;
    ss.read(reinterpret_cast<char*>(&name_len), sizeof(name_len));
    if (ss.fail()) {
        throw std::runtime_error("Failed to read name length during deserialization.");
    }

    // Read the name data
    std::vector<char> name_buffer(name_len);
    ss.read(name_buffer.data(), name_len);
     if (ss.fail()) {
        throw std::runtime_error("Failed to read name data during deserialization.");
    }
    user.name.assign(name_buffer.data(), name_len);

    return user;
}

void run_test(int id, const std::string& name) {
    static int test_num = 1;
    std::cout << "\n--- Test Case " << test_num++ << " ---" << std::endl;
    
    UserData original_user = {id, name};
    std::cout << "Original Object: { id: " << original_user.id << ", name: \"" << original_user.name << "\" }" << std::endl;

    // 1. Serialize the object
    std::string serialized_data = serializeData(original_user);
    std::cout << "Serialized Data (size): " << serialized_data.length() << " bytes" << std::endl;

    // 2. Deserialize the data
    try {
        UserData deserialized_user = deserializeData(serialized_data);
        std::cout << "Deserialized Object: { id: " << deserialized_user.id << ", name: \"" << deserialized_user.name << "\" }" << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "--- C++ Deserialization Test ---" << std::endl;
    
    // 5 Test Cases
    run_test(201, "Alice");
    run_test(202, "Bob");
    run_test(203, "Charlie");
    run_test(204, "A very long name to test buffer handling");
    run_test(205, ""); // Empty name

    return 0;
}
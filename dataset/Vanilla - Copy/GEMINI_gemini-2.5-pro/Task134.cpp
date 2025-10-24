#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <iomanip>
#include <sstream>
#include <optional>
#include <algorithm>

const int KEY_SIZE_BYTES = 32; // 256 bits

class KeyManager {
private:
    std::map<std::string, std::vector<unsigned char>> keyStore;
    std::mt19937 random_generator;

public:
    KeyManager() {
        std::random_device rd;
        random_generator.seed(rd());
    }

    /**
     * Generates a new cryptographic key and stores it with the given ID.
     * @param id The identifier for the key.
     * @return The generated key.
     */
    std::vector<unsigned char> generateKey(const std::string& id) {
        if (id.empty()) {
            throw std::invalid_argument("Key ID cannot be empty.");
        }
        std::vector<unsigned char> key(KEY_SIZE_BYTES);
        std::uniform_int_distribution<int> distribution(0, 255);
        
        std::generate(key.begin(), key.end(), [&]() {
            return static_cast<unsigned char>(distribution(random_generator));
        });
        
        keyStore[id] = key;
        return key;
    }

    /**
     * Retrieves a key by its ID.
     * @param id The identifier for the key.
     * @return An optional containing the key, or empty if not found.
     */
    std::optional<std::vector<unsigned char>> getKey(const std::string& id) {
        auto it = keyStore.find(id);
        if (it != keyStore.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    /**
     * Deletes a key by its ID.
     * @param id The identifier for the key.
     * @return true if the key was deleted, false otherwise.
     */
    bool deleteKey(const std::string& id) {
        return keyStore.erase(id) > 0;
    }

    /**
     * Lists all key identifiers currently in the store.
     * @return A vector of all key IDs.
     */
    std::vector<std::string> listKeys() {
        std::vector<std::string> ids;
        for (const auto& pair : keyStore) {
            ids.push_back(pair.first);
        }
        return ids;
    }
};

/**
 * Helper to convert a vector of bytes to a hexadecimal string.
 * @param bytes The vector to convert.
 * @return The hexadecimal string representation.
 */
std::string bytesToHexString(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

void run_tests() {
    KeyManager keyManager;
    std::cout << "Cryptographic Key Manager - C++" << std::endl;
    std::cout << "---------------------------------" << std::endl;

    // Test Case 1: Generate a key for "user1"
    std::cout << "1. Generating key for 'user1'..." << std::endl;
    std::vector<unsigned char> key1 = keyManager.generateKey("user1");
    std::cout << "   Generated key for 'user1': " << bytesToHexString(key1) << std::endl;
    std::cout << std::endl;

    // Test Case 2: Generate a key for "user2"
    std::cout << "2. Generating key for 'user2'..." << std::endl;
    std::vector<unsigned char> key2 = keyManager.generateKey("user2");
    std::cout << "   Generated key for 'user2': " << bytesToHexString(key2) << std::endl;
    std::cout << std::endl;

    // Test Case 3: Retrieve and print the key for "user1"
    std::cout << "3. Retrieving key for 'user1'..." << std::endl;
    auto retrievedKey1 = keyManager.getKey("user1");
    if (retrievedKey1) {
        std::cout << "   Retrieved key for 'user1': " << bytesToHexString(*retrievedKey1) << std::endl;
    } else {
        std::cout << "   Key for 'user1' not found." << std::endl;
    }
    std::cout << std::endl;

    // Test Case 4: Attempt to retrieve a key for "user3" (non-existent)
    std::cout << "4. Attempting to retrieve non-existent key 'user3'..." << std::endl;
    auto retrievedKey3 = keyManager.getKey("user3");
    std::cout << "   Retrieved key for 'user3': " << (retrievedKey3 ? bytesToHexString(*retrievedKey3) : "Not Found") << std::endl;
    std::cout << std::endl;

    // Test Case 5: Delete the key for "user2" and list remaining keys
    std::cout << "5. Deleting key for 'user2' and listing keys..." << std::endl;
    bool deleted = keyManager.deleteKey("user2");
    std::cout << "   Key 'user2' deleted: " << (deleted ? "true" : "false") << std::endl;
    std::vector<std::string> keys = keyManager.listKeys();
    std::cout << "   Current keys in store: [";
    for (size_t i = 0; i < keys.size(); ++i) {
        std::cout << keys[i] << (i == keys.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
    std::cout << "---------------------------------" << std::endl;
}

int main() {
    run_tests();
    return 0;
}
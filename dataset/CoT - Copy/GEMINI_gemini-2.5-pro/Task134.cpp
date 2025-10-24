#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <stdexcept>
#include <algorithm> // For std::fill
#include <openssl/rand.h> // For RAND_bytes
#include <openssl/err.h>  // For error reporting

// NOTE: This is a demonstration of cryptographic key management concepts.
// Storing raw key material in application memory is NOT a secure practice
// for production systems. Production systems should use Hardware Security
// Modules (HSMs) or dedicated key management services.
//
// To compile: g++ your_file_name.cpp -o your_program_name -lssl -lcrypto
// Or: clang++ your_file_name.cpp -o your_program_name -lssl -lcrypto

// Helper function to print byte vectors in hex format for readability
void print_hex(const std::vector<unsigned char>& data) {
    for (unsigned char c : data) {
        printf("%02x", c);
    }
    printf("\n");
}

class KeyManager {
private:
    std::map<std::string, std::vector<unsigned char>> keyStore;

public:
    /**
     * Generates a new key using OpenSSL's cryptographically secure RNG.
     * @param keyName The alias to store the key under.
     * @param keySizeInBytes The size of the key in bytes.
     */
    void generateKey(const std::string& keyName, size_t keySizeInBytes) {
        if (keyName.empty()) {
            throw std::invalid_argument("Key name cannot be empty.");
        }
        if (keySizeInBytes == 0) {
            throw std::invalid_argument("Key size must be greater than zero.");
        }

        std::vector<unsigned char> key(keySizeInBytes);
        if (RAND_bytes(key.data(), key.size()) != 1) {
            // Get OpenSSL error
            char err_buf[256];
            ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));
            throw std::runtime_error("Failed to generate random bytes: " + std::string(err_buf));
        }
        keyStore[keyName] = key;
    }

    /**
     * Retrieves a copy of the key material for the given key name.
     * @param keyName The alias of the key to retrieve.
     * @return A copy of the key as a vector of unsigned chars.
     * @throws std::out_of_range if the key is not found.
     */
    std::vector<unsigned char> getKey(const std::string& keyName) {
        auto it = keyStore.find(keyName);
        if (it == keyStore.end()) {
            throw std::out_of_range("Key not found: " + keyName);
        }
        // The map returns a copy, which is what we want for security.
        return it->second;
    }

    /**
     * Deletes a key from the store and overwrites its memory.
     * @param keyName The alias of the key to delete.
     * @return true if the key was found and deleted, false otherwise.
     */
    bool deleteKey(const std::string& keyName) {
        auto it = keyStore.find(keyName);
        if (it != keyStore.end()) {
            // Securely overwrite the key material before erasing.
            std::fill(it->second.begin(), it->second.end(), 0);
            keyStore.erase(it);
            return true;
        }
        return false;
    }

    /**
     * Lists all key names currently in the store.
     * @return A vector of key names.
     */
    std::vector<std::string> listKeys() {
        std::vector<std::string> names;
        for (const auto& pair : keyStore) {
            names.push_back(pair.first);
        }
        return names;
    }
};

class Task134 {
public:
    static void run() {
        KeyManager keyManager;
        std::cout << "--- C++ Key Manager Test ---" << std::endl;

        // Test Case 1: Generate a new key
        std::cout << "\n1. Generating a 32-byte (256-bit) key named 'api_key_1'..." << std::endl;
        try {
            keyManager.generateKey("api_key_1", 32);
            std::cout << "Key generated successfully." << std::endl;
            std::cout << "Current keys: ";
            for (const auto& name : keyManager.listKeys()) std::cout << name << " ";
            std::cout << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        // Test Case 2: Retrieve the key
        std::cout << "\n2. Retrieving key 'api_key_1'..." << std::endl;
        try {
            std::vector<unsigned char> retrievedKey = keyManager.getKey("api_key_1");
            std::cout << "Retrieved key (Hex): ";
            print_hex(retrievedKey);
            std::cout << "Retrieved key length: " << retrievedKey.size() * 8 << " bits" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        // Test Case 3: Attempt to retrieve a non-existent key
        std::cout << "\n3. Attempting to retrieve non-existent key 'fake_key'..." << std::endl;
        try {
            keyManager.getKey("fake_key");
            std::cerr << "FAIL: Retrieved 'fake_key' unexpectedly." << std::endl;
        } catch (const std::out_of_range&) {
            std::cout << "Key not found (as expected)." << std::endl;
        }

        // Test Case 4: Delete the key
        std::cout << "\n4. Deleting key 'api_key_1'..." << std::endl;
        bool deleted = keyManager.deleteKey("api_key_1");
        std::cout << "Key deleted successfully: " << std::boolalpha << deleted << std::endl;
        std::cout << "Current keys: ";
        auto keys = keyManager.listKeys();
        if (keys.empty()) {
            std::cout << "(none)";
        } else {
            for (const auto& name : keys) std::cout << name << " ";
        }
        std::cout << std::endl;

        // Test Case 5: Attempt to retrieve the deleted key
        std::cout << "\n5. Attempting to retrieve deleted key 'api_key_1'..." << std::endl;
        try {
            keyManager.getKey("api_key_1");
            std::cerr << "FAIL: Retrieved 'api_key_1' after deletion." << std::endl;
        } catch (const std::out_of_range&) {
            std::cout << "Key not found (as expected)." << std::endl;
        }
    }
};

int main() {
    Task134::run();
    return 0;
}
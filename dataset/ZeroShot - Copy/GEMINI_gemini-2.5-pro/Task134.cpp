#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <stdexcept>
#include <algorithm>
#include <cstring> // For memset

/**
 * WARNING: This is a non-secure, educational example of a Key Manager's structure.
 * - It uses a simple XOR operation for "wrapping" keys, which is insecure.
 * - It uses a non-cryptographic random number generator.
 * - Master key handling is trivial and insecure.
 *
 * In a real-world C++ application, you MUST use a well-vetted cryptographic
 * library like OpenSSL, Botan, or Crypto++. Do NOT use this code in production.
 */

// A secure way to zero out memory to prevent compilers from optimizing it away.
void secure_zero_memory(void* v, size_t n) {
    volatile unsigned char* p = (volatile unsigned char*)v;
    while (n--) {
        *p++ = 0;
    }
}

class KeyManager {
private:
    std::map<std::string, std::vector<unsigned char>> key_store;
    std::vector<unsigned char> master_key;

    // INSECURE: Simple XOR for demonstration of wrapping/unwrapping concept.
    // A real implementation would use an AEAD cipher like AES-GCM.
    std::vector<unsigned char> xor_cipher(const std::vector<unsigned char>& data) {
        if (master_key.empty()) {
            throw std::runtime_error("Master key is not set.");
        }
        std::vector<unsigned char> result = data;
        for (size_t i = 0; i < data.size(); ++i) {
            result[i] = data[i] ^ master_key[i % master_key.size()];
        }
        return result;
    }

public:
    // INSECURE: Master key is just the password bytes. A real implementation
    // would use a strong KDF like Argon2 or PBKDF2.
    KeyManager(const std::string& password) {
        if (password.empty()) {
            throw std::invalid_argument("Password cannot be empty.");
        }
        master_key.assign(password.begin(), password.end());
    }

    ~KeyManager() {
        // Securely clear the master key from memory on destruction
        secure_zero_memory(master_key.data(), master_key.size());
    }

    // INSECURE: Uses std::mt19937, not a cryptographically secure PRNG.
    // A real implementation would use OpenSSL's RAND_bytes or OS-specific sources.
    bool generateKey(const std::string& alias, size_t key_size_bytes) {
        if (alias.empty() || key_size_bytes == 0) {
            return false;
        }
        std::vector<unsigned char> new_key(key_size_bytes);
        
        // Seed with a real random device
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, 255);

        for (size_t i = 0; i < key_size_bytes; ++i) {
            new_key[i] = static_cast<unsigned char>(distrib(gen));
        }

        // "Wrap" the key and store it
        key_store[alias] = xor_cipher(new_key);
        
        // Securely clear the plaintext key
        secure_zero_memory(new_key.data(), new_key.size());
        
        return true;
    }
    
    std::vector<unsigned char> getKey(const std::string& alias) {
        auto it = key_store.find(alias);
        if (it == key_store.end()) {
            throw std::runtime_error("Key not found for alias: " + alias);
        }
        
        // "Unwrap" the key and return it.
        // The caller is responsible for clearing this key from memory after use.
        return xor_cipher(it->second);
    }

    bool deleteKey(const std::string& alias) {
        auto it = key_store.find(alias);
        if (it != key_store.end()) {
            // Securely clear the wrapped key before erasing
            secure_zero_memory(it->second.data(), it->second.size());
            key_store.erase(it);
            return true;
        }
        return false;
    }
};

// Simple assert-like function for tests
void assert_true(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "Assertion failed: " << message << std::endl;
        exit(1);
    }
}

int main() {
    std::cout << "--- C++ KeyManager Tests ---" << std::endl;
    std::cout << "WARNING: This is a non-secure demonstration." << std::endl;

    try {
        KeyManager keyManager("a-very-strong-master-password");

        // Test Case 1: Generate a new key
        std::cout << "1. Generating key 'api-key-1'..." << std::endl;
        bool generated = keyManager.generateKey("api-key-1", 32);
        std::cout << "   Key generated: " << (generated ? "true" : "false") << std::endl;
        assert_true(generated, "Test 1 Failed");

        // Test Case 2: Retrieve the key
        std::cout << "\n2. Retrieving key 'api-key-1'..." << std::endl;
        std::vector<unsigned char> key1 = keyManager.getKey("api-key-1");
        std::cout << "   Key retrieved: " << (!key1.empty()) << std::endl;
        std::cout << "   Key length (bytes): " << key1.size() << std::endl;
        assert_true(!key1.empty() && key1.size() == 32, "Test 2 Failed");
        // Securely clear key after use
        secure_zero_memory(key1.data(), key1.size());

        // Test Case 3: Generate a second key
        std::cout << "\n3. Generating key 'db-key-2'..." << std::endl;
        bool generated2 = keyManager.generateKey("db-key-2", 16);
        std::cout << "   Key generated: " << (generated2 ? "true" : "false") << std::endl;
        std::vector<unsigned char> key2 = keyManager.getKey("db-key-2");
        std::cout << "   Second key retrieved: " << (!key2.empty()) << std::endl;
        assert_true(generated2 && !key2.empty(), "Test 3 Failed");
        secure_zero_memory(key2.data(), key2.size());


        // Test Case 4: Delete the first key
        std::cout << "\n4. Deleting key 'api-key-1'..." << std::endl;
        bool deleted = keyManager.deleteKey("api-key-1");
        std::cout << "   Key deleted: " << (deleted ? "true" : "false") << std::endl;
        assert_true(deleted, "Test 4 Failed");

        // Test Case 5: Attempt to retrieve the deleted key
        std::cout << "\n5. Attempting to retrieve deleted key 'api-key-1'..." << std::endl;
        bool caught = false;
        try {
            keyManager.getKey("api-key-1");
        } catch (const std::runtime_error& e) {
            caught = true;
            std::cout << "   Successfully caught expected exception." << std::endl;
        }
        assert_true(caught, "Test 5 Failed");

        std::cout << "\nAll tests passed." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
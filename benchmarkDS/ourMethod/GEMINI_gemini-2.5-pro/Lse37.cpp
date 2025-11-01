#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/crypto.h> // For CRYPTO_memcmp

// Using a struct to hold credentials
struct UserCredentials {
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hashedPassword;
};

// Global "database" for simplicity. In a real app, this would be a proper database.
static std::unordered_map<std::string, UserCredentials> userDatabase;

const int ITERATIONS = 600000;
const int SALT_SIZE = 16;
const int KEY_LENGTH = 32; // 256 bits

// Function to securely hash a password
std::vector<unsigned char> hashPassword(const std::string& password, const std::vector<unsigned char>& salt) {
    std::vector<unsigned char> hash(KEY_LENGTH);
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        password.length(),
        salt.data(),
        salt.size(),
        ITERATIONS,
        EVP_sha256(),
        hash.size(),
        hash.data()
    );

    if (result != 1) {
        throw std::runtime_error("PBKDF2 HMAC failed.");
    }
    return hash;
}

// Pre-populate the database for the example.
void setupDatabase() {
    std::string username = "admin";
    std::string password = "password123";

    UserCredentials creds;
    creds.salt.resize(SALT_SIZE);

    if (RAND_bytes(creds.salt.data(), creds.salt.size()) != 1) {
        throw std::runtime_error("Failed to generate random salt.");
    }
    
    try {
        creds.hashedPassword = hashPassword(password, creds.salt);
        userDatabase[username] = creds;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error setting up database: " << e.what() << std::endl;
        exit(1);
    }
    
    // Add a user to trigger a simulated error (corrupted entry with empty hash)
    UserCredentials corruptedCreds;
    userDatabase["error_user"] = corruptedCreds;
}

const char* login(const std::string& username, const std::string& password) {
    // Rule #1: Input validation
    if (username.empty() || password.empty()) {
        return "cant_login";
    }
    // Prevent DoS with overly long inputs
    if (username.length() > 100 || password.length() > 100) {
        return "cant_login";
    }

    try {
        auto it = userDatabase.find(username);
        if (it == userDatabase.end()) {
            // User not found. To mitigate timing attacks on username enumeration,
            // we could perform a dummy hash operation here.
            return "cant_login";
        }

        const UserCredentials& storedCreds = it->second;

        // Simulate internal error (e.g., corrupted data)
        if (storedCreds.salt.empty() || storedCreds.hashedPassword.empty()) {
             throw std::runtime_error("User data is corrupted.");
        }

        std::vector<unsigned char> providedPasswordHash = hashPassword(password, storedCreds.salt);

        // Rule #16: Use constant-time comparison to prevent timing attacks
        // First check size to avoid comparing buffers of different lengths
        if (storedCreds.hashedPassword.size() != providedPasswordHash.size()) {
            return "cant_login";
        }

        if (CRYPTO_memcmp(storedCreds.hashedPassword.data(), providedPasswordHash.data(), storedCreds.hashedPassword.size()) == 0) {
            return "success";
        } else {
            return "cant_login";
        }

    } catch (const std::runtime_error&) {
        // Rule #15: Exception handling
        // In a real app, log the error securely.
        return "internal_server_error";
    } catch (...) {
        // Catch any other exceptions
        return "internal_server_error";
    }
}


void run_tests() {
    std::cout << "--- 5 Test Cases ---" << std::endl;

    // 1. Correct login
    const char* result1 = login("admin", "password123");
    std::cout << "Test 1 (Correct login): " << result1 << std::endl;

    // 2. Incorrect password
    const char* result2 = login("admin", "wrongpassword");
    std::cout << "Test 2 (Incorrect password): " << result2 << std::endl;

    // 3. Incorrect username
    const char* result3 = login("unknownuser", "password123");
    std::cout << "Test 3 (Incorrect username): " << result3 << std::endl;

    // 4. Empty input
    const char* result4 = login("", "");
    std::cout << "Test 4 (Empty input): " << result4 << std::endl;

    // 5. Internal error simulation
    const char* result5 = login("error_user", "any_password");
    std::cout << "Test 5 (Internal error simulation): " << result5 << std::endl;
}

int main() {
    try {
        setupDatabase();
        run_tests();
    } catch (const std::runtime_error& e) {
        std::cerr << "A critical error occurred during setup: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
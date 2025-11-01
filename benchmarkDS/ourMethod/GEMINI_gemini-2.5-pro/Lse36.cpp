#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/crypto.h>

// Strong cryptographic parameters
const int SALT_LENGTH = 16;
const int KEY_LENGTH = 32; // 32 bytes = 256 bits
const int ITERATION_COUNT = 310000;

// Represents a user record in the database
struct UserData {
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hashedPassword;
};

// In a real application, this would be a database.
static std::map<std::string, UserData> userDatabase;

// Securely clears a memory buffer.
void secure_zero_memory(void* ptr, size_t size) {
    #if defined(_WIN32)
        SecureZeroMemory(ptr, size);
    #elif defined(__GNUC__) || defined(__clang__)
        // Use a volatile pointer to prevent the compiler from optimizing the memset away.
        volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
        while (size--) {
            *p++ = 0;
        }
    #else
        // A portable fallback, though less guaranteed against optimization.
        memset(ptr, 0, size);
    #endif
}

std::vector<unsigned char> generateSalt() {
    std::vector<unsigned char> salt(SALT_LENGTH);
    if (RAND_bytes(salt.data(), SALT_LENGTH) != 1) {
        throw std::runtime_error("Error generating random salt.");
    }
    return salt;
}

std::vector<unsigned char> hashPassword(const std::string& password, const std::vector<unsigned char>& salt) {
    std::vector<unsigned char> hash(KEY_LENGTH);
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(), 
        password.length(), 
        salt.data(), 
        salt.size(), 
        ITERATION_COUNT, 
        EVP_sha256(), 
        KEY_LENGTH, 
        hash.data()
    );

    if (result != 1) {
        throw std::runtime_error("Error hashing password with PBKDF2.");
    }
    return hash;
}

void registerUser(const std::string& username, std::string password) {
    if (username.empty() || password.empty()) {
        std::cout << "Registration failed: Username and password cannot be empty." << std::endl;
        return;
    }
    
    try {
        auto salt = generateSalt();
        auto hashedPassword = hashPassword(password, salt);
        
        userDatabase[username] = {salt, hashedPassword};
        std::cout << "User '" << username << "' registered successfully." << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Registration error: " << e.what() << std::endl;
    }
    // Securely clear password from memory
    secure_zero_memory(&password[0], password.size());
}

bool login(const std::string& username, std::string password) {
    // 1. Input validation
    if (username.empty() || password.empty()) {
        std::cout << "Login failed: Username and password cannot be empty." << std::endl;
        return false;
    }

    // 2. Query database for username
    auto it = userDatabase.find(username);
    if (it == userDatabase.end()) {
        std::cout << "Login failed: Invalid username or password." << std::endl;
        // Securely clear password from memory before returning
        secure_zero_memory(&password[0], password.size());
        return false;
    }

    const UserData& user = it->second;
    bool success = false;

    try {
        // 3. Hash the provided password with the stored salt
        std::vector<unsigned char> providedPasswordHash = hashPassword(password, user.salt);
        
        // 4. Compare hashes in constant time
        bool hashesMatch = (user.hashedPassword.size() == providedPasswordHash.size()) &&
                            (CRYPTO_memcmp(user.hashedPassword.data(), providedPasswordHash.data(), user.hashedPassword.size()) == 0);

        if (hashesMatch) {
            std::cout << "Login successful for user '" << username << "'. Redirecting to home page..." << std::endl;
            success = true;
        } else {
            std::cout << "Login failed: Invalid username or password." << std::endl;
        }
        
        // Securely clear derived hash
        secure_zero_memory(providedPasswordHash.data(), providedPasswordHash.size());

    } catch (const std::runtime_error& e) {
        std::cerr << "Login error during hashing: " << e.what() << std::endl;
    }

    // Securely clear password from memory
    secure_zero_memory(&password[0], password.size());
    return success;
}

int main() {
    // Setup: Register some users for testing
    registerUser("alice", "Password123!");
    registerUser("bob", "SecurePass@2024");

    std::cout << "\n--- Running Test Cases ---" << std::endl;

    // Test Case 1: Successful login
    std::cout << "\n[Test 1] Correct credentials for 'alice'" << std::endl;
    login("alice", "Password123!");

    // Test Case 2: Incorrect password
    std::cout << "\n[Test 2] Incorrect password for 'alice'" << std::endl;
    login("alice", "wrongpassword");

    // Test Case 3: Non-existent user
    std::cout << "\n[Test 3] Non-existent user 'charlie'" << std::endl;
    login("charlie", "anypassword");

    // Test Case 4: Successful login for another user
    std::cout << "\n[Test 4] Correct credentials for 'bob'" << std::endl;
    login("bob", "SecurePass@2024");

    // Test Case 5: Empty password
    std::cout << "\n[Test 5] Empty password for 'alice'" << std::endl;
    login("alice", "");

    return 0;
}
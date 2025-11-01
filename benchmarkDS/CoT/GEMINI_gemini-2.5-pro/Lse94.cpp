#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>

// IMPORTANT: This code requires the OpenSSL library.
// To compile on Linux/macOS, you need to link against it:
// g++ your_file_name.cpp -o output_name -lssl -lcrypto
#include <openssl/sha.h>
#include <openssl/rand.h>

/**
 * Converts a byte array to its hexadecimal string representation.
 * @param data Pointer to the byte array.
 * @param len Length of the byte array.
 * @return The hexadecimal string.
 */
std::string bytesToHex(const unsigned char* data, size_t len) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        ss << std::setw(2) << static_cast<unsigned int>(data[i]);
    }
    return ss.str();
}

/**
 * Registers a user by hashing their password and simulating a database insert.
 * @param username The username to register.
 * @param password The plaintext password.
 */
void registerUser(const std::string& username, const std::string& password) {
    std::cout << "Registering user: " << username << std::endl;

    // 1. Generate a cryptographically secure random salt
    unsigned char salt[16];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        std::cerr << "Error: Could not generate random salt." << std::endl;
        return;
    }

    // 2. Hash the password with the salt
    // Note: For production, use a key derivation function like PBKDF2 (PKCS5_PBKDF2_HMAC)
    // from OpenSSL, which is more secure against brute-force attacks.
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256_ctx;
    SHA256_Init(&sha256_ctx);
    SHA256_Update(&sha256_ctx, salt, sizeof(salt));
    SHA256_Update(&sha256_ctx, password.c_str(), password.length());
    SHA256_Final(hash, &sha256_ctx);
    
    // Convert binary salt and hash to hex for storage
    std::string saltHex = bytesToHex(salt, sizeof(salt));
    std::string hashedPasswordHex = bytesToHex(hash, SHA256_DIGEST_LENGTH);

    // 3. Simulate inserting into the database
    std::cout << "  -> Simulating database insert..." << std::endl;
    std::cout << "     Username: " << username << std::endl;
    std::cout << "     Salt: " << saltHex << std::endl;
    std::cout << "     Hashed Password: " << hashedPasswordHex << std::endl;
    std::cout << "Registration complete." << std::endl << std::endl;
}

int main() {
    // 5 Test Cases
    registerUser("alice", "Password123!");
    registerUser("bob", "my$ecretP@ss");
    registerUser("charlie", "qwerty");
    registerUser("diana", "12345678");
    registerUser("eve", "pa$$w0rd");

    return 0;
}
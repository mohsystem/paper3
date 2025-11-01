#include <iostream>
#include <string>
#include <map>
#include <vector>

// Simulate a database using std::map
std::map<std::string, std::string> userDatabase;

/**
 * Registers a user by creating a placeholder for their hashed password.
 * @param username The username to register.
 * @param password The plain-text password.
 */
void registerUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        std::cout << "Error: Username or password cannot be empty." << std::endl;
        return;
    }
    if (userDatabase.count(username)) {
        std::cout << "Error: Username '" << username << "' already exists." << std::endl;
        return;
    }

    /*
     * ========================= IMPORTANT SECURITY NOTE =========================
     * C++ does not have a standard library for cryptography. Securely hashing
     * passwords requires a dedicated, well-vetted external library like OpenSSL,
     * Crypto++, or a modern library that implements bcrypt, scrypt, or Argon2.
     *
     * DO NOT attempt to implement your own hashing function or use simple hashes
     * like MD5 or SHA-1 directly for passwords. They are not designed for this
     * purpose and are vulnerable to modern attacks.
     *
     * The following line is a NON-FUNCTIONAL PLACEHOLDER to demonstrate where
     * the secure hash would be stored. In a real application, you would:
     * 1. Generate a random salt using the crypto library.
     * 2. Hash the password and salt together using a function like bcrypt().
     * 3. Store the resulting hash (which usually includes the salt).
     *
     * Example using a hypothetical library:
     * std::string salt = CryptoLib::generate_salt(16);
     * std::string hash = CryptoLib::bcrypt_hash(password, salt);
     * std::string stored_value = hash; // bcrypt format includes salt
     * =========================================================================
     */
    std::string stored_value = "PLACEHOLDER_FOR_SALT_AND_HASH_FROM_A_CRYPTO_LIBRARY";

    // "Insert" the username and the placeholder hash into the database.
    userDatabase[username] = stored_value;
    std::cout << "User '" << username << "' registered successfully (with placeholder hash)." << std::endl;
}

/**
 * Prints the contents of the simulated user database.
 */
void printDatabase() {
    std::cout << "\n--- Simulated User Database ---" << std::endl;
    if (userDatabase.empty()) {
        std::cout << "Database is empty." << std::endl;
    } else {
        for (const auto& pair : userDatabase) {
            std::cout << "Username: " << pair.first << ", Stored Hash: " << pair.second << std::endl;
        }
    }
    std::cout << "-----------------------------" << std::endl << std::endl;
}

int main() {
    // --- 5 Test Cases ---
    std::cout << "--- Running C++ Test Cases ---" << std::endl;
    registerUser("alice", "Password123!");
    registerUser("bob", "my$ecretP@ss");
    registerUser("charlie", "Ch@rlieBrown");
    registerUser("david", "d@v1d_the_k1ng");
    registerUser("eve", "EveIsAwesome#2024");

    // Test case for existing user
    registerUser("alice", "AnotherPassword");

    // Print the final state of the database
    printDatabase();

    return 0;
}
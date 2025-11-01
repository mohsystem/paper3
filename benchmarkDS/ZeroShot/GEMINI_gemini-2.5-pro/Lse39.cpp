#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

// ========================== SECURITY WARNING ============================
// The hashing function used here is a PLACEHOLDER and is NOT SECURE.
// It is for demonstration purposes only to fit the single-file requirement.
// In a real-world application, you MUST use a well-vetted, standard
// password hashing library like Argon2, scrypt, or bcrypt.
// You can also use PBKDF2 from a library like OpenSSL.
// NEVER store passwords in plaintext or with simple hash functions like SHA-256.
// ========================================================================
std::string insecure_placeholder_hash(const std::string& password, const std::string& salt) {
    // THIS IS NOT A REAL HASH FUNCTION!
    // It simply concatenates the salt and password.
    // Replace this with a call to a real password hashing function.
    return salt + password;
}

// A mock "salt" generator. In a real app, use a cryptographic PRNG.
std::string generate_salt(size_t length) {
    // For demonstration, we use a fixed but seemingly random string.
    std::string s = "abcdefghijklmnopqrstuvwxyz0123456789";
    std::string salt = "";
    for(size_t i = 0; i < length; ++i) {
        salt += s[i % s.length()];
    }
    return salt;
}


class Lse39 {
private:
    struct UserData {
        std::string salt;
        std::string hashedPassword;
    };
    
    // In a real application, this would be a database.
    std::unordered_map<std::string, UserData> userDatabase;

    /**
     * @brief Performs a constant-time comparison of two strings.
     * It compares all bytes to prevent timing side-channel attacks.
     * @param a The first string.
     * @param b The second string.
     * @return true if strings are equal, false otherwise.
     */
    bool constant_time_compare(const std::string& a, const std::string& b) {
        if (a.length() != b.length()) {
            return false;
        }

        int result = 0;
        for (size_t i = 0; i < a.length(); ++i) {
            result |= a[i] ^ b[i];
        }

        return result == 0;
    }

public:
    void registerUser(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            std::cout << "Registration failed: Username and password cannot be empty." << std::endl;
            return;
        }
        std::string salt = generate_salt(16);
        std::string hashedPassword = insecure_placeholder_hash(password, salt);
        userDatabase[username] = {salt, hashedPassword};
    }

    bool login(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return false;
        }

        auto it = userDatabase.find(username);
        if (it == userDatabase.end()) {
            // To prevent timing attacks, perform a dummy hash calculation.
            insecure_placeholder_hash(password, generate_salt(16));
            return false;
        }

        const UserData& userData = it->second;
        std::string providedPasswordHash = insecure_placeholder_hash(password, userData.salt);
        
        return constant_time_compare(userData.hashedPassword, providedPasswordHash);
    }
};

int main() {
    Lse39 loginSystem;
    
    // "Register" a user for testing
    loginSystem.registerUser("admin", "S3cureP@ssword!123");

    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    // Test Case 1: Correct username and correct password
    std::cout << "Test Case 1 (Correct credentials): ";
    bool success1 = loginSystem.login("admin", "S3cureP@ssword!123");
    std::cout << (success1 ? "Login successful. Redirecting to session page." : "Invalid username or password.") << std::endl;

    // Test Case 2: Correct username and incorrect password
    std::cout << "Test Case 2 (Incorrect password): ";
    bool success2 = loginSystem.login("admin", "wrongpassword");
    std::cout << (success2 ? "Login successful. Redirecting to session page." : "Invalid username or password.") << std::endl;

    // Test Case 3: Incorrect username
    std::cout << "Test Case 3 (Incorrect username): ";
    bool success3 = loginSystem.login("guest", "S3cureP@ssword!123");
    std::cout << (success3 ? "Login successful. Redirecting to session page." : "Invalid username or password.") << std::endl;

    // Test Case 4: Correct username, empty password
    std::cout << "Test Case 4 (Empty password): ";
    bool success4 = loginSystem.login("admin", "");
    std::cout << (success4 ? "Login successful. Redirecting to session page." : "Invalid username or password.") << std::endl;

    // Test Case 5: Empty username
    std::cout << "Test Case 5 (Empty username): ";
    bool success5 = loginSystem.login("", "S3cureP@ssword!123");
    std::cout << (success5 ? "Login successful. Redirecting to session page." : "Invalid username or password.") << std::endl;

    return 0;
}
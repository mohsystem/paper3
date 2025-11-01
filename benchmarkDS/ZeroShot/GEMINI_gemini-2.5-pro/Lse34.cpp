#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <algorithm>

// !!!!!!!!!!!!!!!!!!!!!!!!!!!  SECURITY WARNING  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// This C++ code is for DEMONSTRATION PURPOSES ONLY. It illustrates the
// architectural pattern of salting and hashing but does NOT use
// cryptographically secure functions, as they are not in the C++ standard library.
//
// IN A REAL-WORLD APPLICATION, YOU MUST USE A DEDICATED CRYPTOGRAPHY LIBRARY
// like OpenSSL, Crypto++, or libsodium to handle:
//
// 1.  SECURE HASHING: Use a strong key derivation function like Argon2, bcrypt,
//     or PBKDF2. The simple string concatenation used here is COMPLETELY INSECURE
//     and can be broken instantly.
//
// 2.  CONSTANT-TIME COMPARISON: Use a function like `CRYPTO_memcmp` from
//     OpenSSL to prevent timing attacks. The standard string comparison (`==`)
//     is not secure for this purpose as it can leak information about the hash.
//
// 3.  CRYPTOGRAPHICALLY SECURE RANDOMNESS: Use the library's secure random
//     number generator for salts, not std::mt19937, which is not secure.
//
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

struct UserCredentials {
    std::string salt;
    std::string hashedPassword;
};

// Simulated in-memory user database
std::map<std::string, UserCredentials> userDatabase;

// **INSECURE** placeholder for generating a salt. Use a real crypto library.
std::string generateSalt(size_t length) {
    const std::string CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);
    std::string random_string;
    for (size_t i = 0; i < length; ++i) {
        random_string += CHARACTERS[distribution(generator)];
    }
    return random_string;
}

// **INSECURE** placeholder for a hashing function. Use a real crypto library.
std::string insecureHashFunction(const std::string& password, const std::string& salt) {
    // This is NOT a hash function. It's for demonstrating the data flow ONLY.
    return "hash_of(" + salt + password + ")"; 
}

void registerUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        std::cout << "Registration failed: Username and password cannot be empty." << std::endl;
        return;
    }
    if (userDatabase.count(username)) {
        std::cout << "Registration failed: Username already exists." << std::endl;
        return;
    }
    
    std::string salt = generateSalt(16);
    std::string hashedPassword = insecureHashFunction(password, salt);
    
    userDatabase[username] = {salt, hashedPassword};
    std::cout << "User '" << username << "' registered successfully (using insecure demo methods)." << std::endl;
}

std::string loginUser(const std::string& username, const std::string& password) {
    auto it = userDatabase.find(username);
    // Generic message to prevent username enumeration attacks.
    if (it == userDatabase.end() || password.empty()) {
        return "Login failed: Invalid username or password. Redirecting to login page.";
    }

    const UserCredentials& credentials = it->second;
    std::string salt = credentials.salt;
    std::string storedHash = credentials.hashedPassword;

    std::string providedPasswordHash = insecureHashFunction(password, salt);

    // **INSECURE COMPARISON**: This is vulnerable to timing attacks. Use a constant-time comparison function from a crypto library.
    if (storedHash == providedPasswordHash) {
        return "Login successful! Redirecting to dashboard...";
    } else {
        return "Login failed: Invalid username or password. Redirecting to login page.";
    }
}


int main() {
    // Setup: Register a user
    std::string testUser = "testUser";
    std::string testPass = "P@ssword123!";
    registerUser(testUser, testPass);
    std::cout << "\n--- Running Test Cases ---" << std::endl;

    // Test Case 1: Successful Login
    std::cout << "Test Case 1: Correct username and password." << std::endl;
    std::string result1 = loginUser(testUser, testPass);
    std::cout << "Result: " << result1 << std::endl;
    std::cout << "--------------------------" << std::endl;

    // Test Case 2: Incorrect Password
    std::cout << "Test Case 2: Correct username, incorrect password." << std::endl;
    std::string result2 = loginUser(testUser, "WrongPassword");
    std::cout << "Result: " << result2 << std::endl;
    std::cout << "--------------------------" << std::endl;

    // Test Case 3: Incorrect Username
    std::cout << "Test Case 3: Incorrect username." << std::endl;
    std::string result3 = loginUser("nonexistentUser", testPass);
    std::cout << "Result: " << result3 << std::endl;
    std::cout << "--------------------------" << std::endl;
    
    // Test Case 4: Case-sensitive Username
    std::cout << "Test Case 4: Case-sensitive username check (should fail)." << std::endl;
    std::string result4 = loginUser("testuser", testPass);
    std::cout << "Result: " << result4 << std::endl;
    std::cout << "--------------------------" << std::endl;

    // Test Case 5: Empty Password
    std::cout << "Test Case 5: Correct username, empty password (should fail)." << std::endl;
    std::string result5 = loginUser(testUser, "");
    std::cout << "Result: " << result5 << std::endl;
    std::cout << "--------------------------" << std::endl;

    return 0;
}
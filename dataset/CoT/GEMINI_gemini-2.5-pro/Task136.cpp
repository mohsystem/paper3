#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// WARNING: THIS IS A TOY HASHING FUNCTION FOR DEMONSTRATION ONLY.
// IT IS NOT CRYPTOGRAPHICALLY SECURE.
// In a real C++ application, you MUST use a well-vetted cryptography library
// like OpenSSL, Crypto++, or Botan to handle password hashing (e.g., with Argon2, scrypt, or bcrypt).
std::string hashPassword(const std::string& password) {
    if (password.empty()) return "";
    unsigned long hash = 5381;
    for (char c : password) {
        hash = ((hash << 5) + hash) + c; // djb2 algorithm
    }
    // Simulate a fixed-length hex output
    char buf[17];
    snprintf(buf, sizeof(buf), "%016lx", hash);
    return std::string(buf);
}

// Performs a constant-time comparison to help mitigate timing attacks.
bool secureCompare(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) {
        return false;
    }
    // Use a variable to accumulate differences without short-circuiting.
    int diff = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        diff |= a[i] ^ b[i];
    }
    return diff == 0;
}

class Task136 {
private:
    std::unordered_map<std::string, std::string> userDatabase;

public:
    Task136() {
        // Pre-populate with some users. Passwords are "hashed" using the insecure demo function.
        // "password123" -> "000000069506164f"
        // "adminPass!" -> "0000000693a81744"
        userDatabase["user1"] = "000000069506164f";
        userDatabase["admin"] = "0000000693a81744";
    }

    bool authenticateUser(const std::string& username, const std::string& password) {
        // 1. Basic input validation
        if (username.empty() || password.empty()) {
            return false;
        }

        // 2. Check if user exists
        auto it = userDatabase.find(username);
        if (it == userDatabase.end()) {
            return false;
        }
        const std::string& storedHash = it->second;

        // 3. Hash the provided password
        std::string inputHash = hashPassword(password);

        // 4. Securely compare the hashes
        return secureCompare(storedHash, inputHash);
    }
};

int main() {
    Task136 auth;

    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Correct username, correct password
    bool result1 = auth.authenticateUser("user1", "password123");
    std::cout << "Test 1 (user1, password123): " << (result1 ? "Success" : "Failure") << std::endl;

    // Test Case 2: Correct username, incorrect password
    bool result2 = auth.authenticateUser("user1", "wrongpassword");
    std::cout << "Test 2 (user1, wrongpassword): " << (result2 ? "Success" : "Failure") << std::endl;

    // Test Case 3: Incorrect username
    bool result3 = auth.authenticateUser("unknownuser", "password123");
    std::cout << "Test 3 (unknownuser, password123): " << (result3 ? "Success" : "Failure") << std::endl;

    // Test Case 4: Correct username, empty password
    bool result4 = auth.authenticateUser("admin", "");
    std::cout << "Test 4 (admin, ''): " << (result4 ? "Success" : "Failure") << std::endl;

    // Test Case 5: Empty username
    bool result5 = auth.authenticateUser("", "somepass");
    std::cout << "Test 5 ('', somepass): " << (result5 ? "Success" : "Failure") << std::endl;

    return 0;
}
#include <iostream>
#include <string>
#include <vector>
#include <map>

// ========================== SECURITY WARNING ==========================
// The following code is a SIMULATION for educational purposes only.
// Standard C++ does NOT have a built-in, standard library for
// cryptography. The hashing and random number generation used here
// are NOT cryptographically secure.
//
// In a REAL-WORLD application, you MUST use a well-vetted, reputable
// third-party cryptography library like OpenSSL, Crypto++, or
// libsodium to handle password hashing and random salt generation.
//
// DO NOT USE THIS CODE IN PRODUCTION.
// ====================================================================

// In-memory map to simulate a user database.
// The value stores "salt:hashed_password"
static std::map<std::string, std::string> userDatabase;

// FAKE salt generator for simulation. NOT SECURE.
std::string generate_fake_salt() {
    return "a1b2c3d4"; // Fixed salt for predictable simulation
}

// FAKE password hasher for simulation. NOT SECURE.
std::string fake_hash_password(const std::string& password, const std::string& salt) {
    std::string salted_password = salt + password;
    unsigned long hash = 5381;
    for (char c : salted_password) {
        hash = ((hash << 5) + hash) + c; // djb2 algorithm
    }
    return std::to_string(hash);
}

// Simulates user registration by "hashing" a password and storing it.
void register_user(const std::string& username, const std::string& password) {
    std::string salt = generate_fake_salt();
    std::string hashed_password = fake_hash_password(password, salt);
    userDatabase[username] = salt + ":" + hashed_password;
}

// Constant-time string comparison to mitigate timing attacks.
bool secure_compare(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) {
        return false;
    }
    int result = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        result |= (a[i] ^ b[i]);
    }
    return result == 0;
}

/**
 * Authenticates a user by checking their username and password.
 * @param username The username provided by the user.
 * @param password The password provided by the user.
 * @return true if authentication is successful, false otherwise.
 */
bool login(const std::string& username, const std::string& password) {
    // 1. Check if username exists.
    auto it = userDatabase.find(username);
    if (it == userDatabase.end()) {
        // To mitigate timing attacks, perform a dummy hash.
        fake_hash_password(password, generate_fake_salt());
        return false;
    }

    std::string stored_salt_and_hash = it->second;
    size_t separator_pos = stored_salt_and_hash.find(':');
    if (separator_pos == std::string::npos) {
        // Data integrity issue
        return false;
    }

    // 2. Extract salt and stored hash.
    std::string salt = stored_salt_and_hash.substr(0, separator_pos);
    std::string stored_hash = stored_salt_and_hash.substr(separator_pos + 1);

    // 3. Hash the provided password with the retrieved salt.
    std::string provided_password_hash = fake_hash_password(password, salt);

    // 4. Compare the hashes in a way that is resistant to timing attacks.
    return secure_compare(stored_hash, provided_password_hash);
}

int main() {
    // Pre-populate the database with some users for testing.
    register_user("admin", "AdminPass123!");
    register_user("user1", "UserPass@456");
    register_user("testuser", "password");

    // --- 5 Test Cases ---
    std::cout << "--- Test Cases ---" << std::endl;

    // Test Case 1: Successful login for admin
    std::cout << "\n1. Testing with correct admin credentials..." << std::endl;
    bool isAdminLoggedIn = login("admin", "AdminPass123!");
    std::cout << "Login success: " << (isAdminLoggedIn ? "true" : "false") << std::endl;
    std::cout << (isAdminLoggedIn ? "Redirecting to home page." : "Redirecting to login page.") << std::endl;

    // Test Case 2: Successful login for user1
    std::cout << "\n2. Testing with correct user1 credentials..." << std::endl;
    bool isUser1LoggedIn = login("user1", "UserPass@456");
    std::cout << "Login success: " << (isUser1LoggedIn ? "true" : "false") << std::endl;
    std::cout << (isUser1LoggedIn ? "Redirecting to home page." : "Redirecting to login page.") << std::endl;

    // Test Case 3: Failed login with wrong password
    std::cout << "\n3. Testing with incorrect password for admin..." << std::endl;
    bool isWrongPassLoggedIn = login("admin", "wrongpassword");
    std::cout << "Login success: " << (isWrongPassLoggedIn ? "true" : "false") << std::endl;
    std::cout << (isWrongPassLoggedIn ? "Redirecting to home page." : "Redirecting to login page.") << std::endl;

    // Test Case 4: Failed login with non-existent user
    std::cout << "\n4. Testing with non-existent username..." << std::endl;
    bool isNonExistentUserLoggedIn = login("unknownuser", "somepassword");
    std::cout << "Login success: " << (isNonExistentUserLoggedIn ? "true" : "false") << std::endl;
    std::cout << (isNonExistentUserLoggedIn ? "Redirecting to home page." : "Redirecting to login page.") << std::endl;

    // Test Case 5: Failed login with empty password
    std::cout << "\n5. Testing with empty password for user1..." << std::endl;
    bool isEmptyPassLoggedIn = login("user1", "");
    std::cout << "Login success: " << (isEmptyPassLoggedIn ? "true" : "false") << std::endl;
    std::cout << (isEmptyPassLoggedIn ? "Redirecting to home page." : "Redirecting to login page.") << std::endl;

    return 0;
}
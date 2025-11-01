#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/crypto.h>

// In-memory user database simulation.
// Key: username, Value: a string containing salt and hashed password in hex format.
static std::map<std::string, std::string> userDatabase;

const int SALT_BYTES = 16;
const int PBKDF2_ITERATIONS = 310000;
const int HASH_BYTES = 32;

// Helper to convert byte array to hex string
std::string to_hex(const unsigned char* data, size_t len) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        ss << std::setw(2) << static_cast<unsigned>(data[i]);
    }
    return ss.str();
}

// Helper to convert hex string to byte vector
std::vector<unsigned char> from_hex(const std::string& hex) {
    std::vector<unsigned char> bytes;
    for (unsigned int i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(strtol(byteString.c_str(), nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

// Hashes a password with a given salt using PBKDF2-HMAC-SHA256
std::vector<unsigned char> hash_password(const std::string& password, const std::vector<unsigned char>& salt) {
    std::vector<unsigned char> hash(HASH_BYTES);
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        password.length(),
        salt.data(),
        salt.size(),
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        hash.size(),
        hash.data()
    );
    if (result != 1) {
        throw std::runtime_error("Failed to hash password with PKCS5_PBKDF2_HMAC.");
    }
    return hash;
}

// Simulates user registration
void registerUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        std::cout << "Registration failed: Username and password cannot be empty." << std::endl;
        return;
    }

    std::vector<unsigned char> salt(SALT_BYTES);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        throw std::runtime_error("Failed to generate random salt.");
    }

    try {
        std::vector<unsigned char> hashed_pw = hash_password(password, salt);
        std::string salt_hex = to_hex(salt.data(), salt.size());
        std::string hash_hex = to_hex(hashed_pw.data(), hashed_pw.size());
        userDatabase[username] = salt_hex + ":" + hash_hex;
        std::cout << "User '" << username << "' registered successfully." << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error during registration: " << e.what() << std::endl;
    }
}

// Simulates the login process
std::string login(const std::string& username, const std::string& password) {
    // Rule #1: Input validation
    if (username.empty() || password.empty()) {
        return "Redirecting to login page: Invalid input.";
    }

    auto it = userDatabase.find(username);
    if (it == userDatabase.end()) {
        return "Redirecting to login page: Invalid credentials.";
    }

    std::string stored_credentials = it->second;
    size_t delimiter_pos = stored_credentials.find(':');
    if (delimiter_pos == std::string::npos) {
        return "Redirecting to login page: Server error (invalid credential format).";
    }

    try {
        std::string salt_hex = stored_credentials.substr(0, delimiter_pos);
        std::string stored_hash_hex = stored_credentials.substr(delimiter_pos + 1);

        std::vector<unsigned char> salt = from_hex(salt_hex);
        std::vector<unsigned char> stored_hash = from_hex(stored_hash_hex);

        std::vector<unsigned char> provided_hash = hash_password(password, salt);

        // Constant-time comparison to prevent timing attacks
        if (stored_hash.size() == provided_hash.size() &&
            CRYPTO_memcmp(stored_hash.data(), provided_hash.data(), stored_hash.size()) == 0) {
            return "Login successful! Redirecting to dashboard page.";
        } else {
            return "Redirecting to login page: Invalid credentials.";
        }
    } catch (const std::exception& e) {
        return "Redirecting to login page: Server error.";
    }
}


int main() {
    try {
        // Populate the "database" with a user
        registerUser("admin", "S3cureP@ssword!23");
        registerUser("testuser", "password123");

        std::cout << "\n--- Running Test Cases ---" << std::endl;

        // Test Case 1: Correct credentials
        std::cout << "1. Testing with correct credentials (admin):" << std::endl;
        std::cout << "   Result: " << login("admin", "S3cureP@ssword!23") << std::endl;
        
        // Test Case 2: Correct username, incorrect password
        std::cout << "\n2. Testing with incorrect password (admin):" << std::endl;
        std::cout << "   Result: " << login("admin", "wrongpassword") << std::endl;
        
        // Test Case 3: Incorrect username
        std::cout << "\n3. Testing with incorrect username:" << std::endl;
        std::cout << "   Result: " << login("unknownuser", "S3cureP@ssword!23") << std::endl;

        // Test Case 4: Correct username, empty password
        std::cout << "\n4. Testing with empty password:" << std::endl;
        std::cout << "   Result: " << login("admin", "") << std::endl;

        // Test Case 5: Empty username
        std::cout << "\n5. Testing with empty username:" << std::endl;
        std::cout << "   Result: " << login("", "S3cureP@ssword!23") << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "A critical error occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
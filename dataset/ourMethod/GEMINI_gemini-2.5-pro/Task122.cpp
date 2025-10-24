#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <map>
#include <memory>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/crypto.h> // For CRYPTO_memcmp

// NOTE: This code requires OpenSSL. Compile with:
// g++ -std=c++17 task122.cpp -o task122 -lssl -lcrypto

// Configuration constants
const int ITERATIONS = 210000;
const int KEY_LENGTH_BYTES = 32;
const int SALT_LENGTH_BYTES = 16;
const size_t MIN_PASSWORD_LENGTH = 8;

// Helper to RAII-style manage OpenSSL BIGNUM context
struct BIO_deleter { void operator()(BIO* p) const { BIO_free_all(p); } };
using unique_BIO = std::unique_ptr<BIO, BIO_deleter>;

// Base64 encode
std::string base64_encode(const std::vector<unsigned char>& data) {
    unique_BIO b64(BIO_new(BIO_f_base64()));
    BIO_set_flags(b64.get(), BIO_FLAGS_BASE64_NO_NL);
    unique_BIO mem(BIO_new(BIO_s_mem()));
    BIO_push(b64.get(), mem.get());

    if (BIO_write(b64.get(), data.data(), data.size()) <= 0) {
        throw std::runtime_error("Failed to base64 encode");
    }
    BIO_flush(b64.get());

    BUF_MEM* bptr;
    BIO_get_mem_ptr(mem.get(), &bptr);
    std::string result(bptr->data, bptr->length);
    return result;
}

// Base64 decode
std::vector<unsigned char> base64_decode(const std::string& data) {
    unique_BIO b64(BIO_new(BIO_f_base64()));
    BIO_set_flags(b64.get(), BIO_FLAGS_BASE64_NO_NL);
    unique_BIO mem(BIO_new_mem_buf(data.c_str(), data.length()));
    BIO_push(b64.get(), mem.get());

    std::vector<unsigned char> decoded_data(data.length()); // Max possible size
    int decoded_len = BIO_read(b64.get(), decoded_data.data(), decoded_data.size());
    if (decoded_len < 0) {
        throw std::runtime_error("Failed to base64 decode");
    }
    decoded_data.resize(decoded_len);
    return decoded_data;
}

// Hashes a password using PBKDF2-HMAC-SHA256
std::string hashPassword(const std::string& password) {
    if (password.length() < MIN_PASSWORD_LENGTH) {
        std::cerr << "Error: Password does not meet the length requirement." << std::endl;
        return "";
    }

    std::vector<unsigned char> salt(SALT_LENGTH_BYTES);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        throw std::runtime_error("Failed to generate random salt");
    }

    std::vector<unsigned char> hash(KEY_LENGTH_BYTES);
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                           salt.data(), salt.size(),
                           ITERATIONS, EVP_sha256(),
                           hash.size(), hash.data()) != 1) {
        throw std::runtime_error("Failed to execute PBKDF2");
    }

    std::string salt_b64 = base64_encode(salt);
    std::string hash_b64 = base64_encode(hash);

    std::ostringstream oss;
    oss << "$pbkdf2-sha256$" << ITERATIONS << "$" << salt_b64 << "$" << hash_b64;
    return oss.str();
}

// Verifies a password against a stored hash
bool verifyPassword(const std::string& password, const std::string& storedHash) {
    std::stringstream ss(storedHash);
    std::string part;
    std::vector<std::string> parts;
    while(std::getline(ss, part, '$')) {
        parts.push_back(part);
    }
    
    // Format: "" "pbkdf2-sha256" "iterations" "salt" "hash"
    if (parts.size() != 5) return false;

    try {
        int iterations = std::stoi(parts[2]);
        std::vector<unsigned char> salt = base64_decode(parts[3]);
        std::vector<unsigned char> original_hash = base64_decode(parts[4]);

        std::vector<unsigned char> comparison_hash(KEY_LENGTH_BYTES);
        if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                               salt.data(), salt.size(),
                               iterations, EVP_sha256(),
                               comparison_hash.size(), comparison_hash.data()) != 1) {
            return false;
        }

        // Constant-time comparison
        return (original_hash.size() == comparison_hash.size()) &&
               (CRYPTO_memcmp(original_hash.data(), comparison_hash.data(), original_hash.size()) == 0);

    } catch (const std::exception& e) {
        // Error during parsing or decoding
        return false;
    }
}

// In-memory user store for demonstration
static std::map<std::string, std::string> userDatabase;

void signUp(const std::string& username, const std::string& password) {
    std::cout << "Attempting to sign up user: " << username << std::endl;
    try {
        std::string hashedPassword = hashPassword(password);
        if (!hashedPassword.empty()) {
            userDatabase[username] = hashedPassword;
            std::cout << "User '" << username << "' signed up successfully." << std::endl;
        } else {
            std::cout << "Signup for user '" << username << "' failed." << std::endl;
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Error during signup: " << e.what() << std::endl;
    }
}

void login(const std::string& username, const std::string& password) {
    std::cout << "Attempting to log in user: " << username << std::endl;
    auto it = userDatabase.find(username);
    if (it == userDatabase.end()) {
        std::cout << "Login failed: User not found." << std::endl;
        return;
    }

    if (verifyPassword(password, it->second)) {
        std::cout << "Login successful for user '" << username << "'." << std::endl;
    } else {
        std::cout << "Login failed: Incorrect password." << std::endl;
    }
}

int main() {
    // Test Case 1: Successful signup and login
    std::cout << "--- Test Case 1 ---" << std::endl;
    std::string user1 = "alice";
    std::string pass1 = "Str0ngP@ssw0rd1";
    signUp(user1, pass1);
    login(user1, pass1);
    std::cout << std::endl;

    // Test Case 2: Login with incorrect password
    std::cout << "--- Test Case 2 ---" << std::endl;
    std::string wrongPass1 = "WrongPassword!_";
    login(user1, wrongPass1);
    std::cout << std::endl;

    // Test Case 3: Signup with a weak (short) password
    std::cout << "--- Test Case 3 ---" << std::endl;
    std::string user2 = "bob";
    std::string pass2_weak = "short";
    signUp(user2, pass2_weak);
    std::cout << std::endl;

    // Test Case 4: Successful signup for a second user
    std::cout << "--- Test Case 4 ---" << std::endl;
    std::string user3 = "charlie";
    std::string pass3 = "An0th3rS3curePwd";
    signUp(user3, pass3);
    login(user3, pass3);
    std::cout << std::endl;

    // Test Case 5: Attempt to log in a non-existent user
    std::cout << "--- Test Case 5 ---" << std::endl;
    login("david", "SomePassword123");
    std::cout << std::endl;

    return 0;
}
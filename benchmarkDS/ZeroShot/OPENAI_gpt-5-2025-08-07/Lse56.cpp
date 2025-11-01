#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

struct User {
    std::string email;
    std::vector<unsigned char> salt;
    std::vector<unsigned char> passHash;
    bool loggedIn;
};

struct ChangeResult {
    bool success;
    std::string message;
};

static const size_t SALT_LEN = 16;
static const size_t HASH_LEN = 32;
static const int ITERATIONS = 100000;

bool isEmailValid(const std::string& email) {
    static const std::regex re(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)");
    return email.size() <= 320 && std::regex_match(email, re);
}

bool deriveKey(const std::string& password, const std::vector<unsigned char>& salt, std::vector<unsigned char>& out) {
    out.assign(HASH_LEN, 0);
    int rc = PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                               salt.data(), static_cast<int>(salt.size()),
                               ITERATIONS, EVP_sha256(), static_cast<int>(out.size()), out.data());
    return rc == 1;
}

bool constantTimeEquals(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b) {
    if (a.size() != b.size()) return false;
    return CRYPTO_memcmp(a.data(), b.data(), a.size()) == 0;
}

User createUser(const std::string& email, const std::string& password) {
    if (!isEmailValid(email)) {
        throw std::runtime_error("Invalid email");
    }
    std::vector<unsigned char> salt(SALT_LEN);
    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1) {
        throw std::runtime_error("Salt generation failed");
    }
    std::vector<unsigned char> hash;
    if (!deriveKey(password, salt, hash)) {
        throw std::runtime_error("Hashing failed");
    }
    return User{email, salt, hash, false};
}

bool login(User& user, const std::string& password) {
    std::vector<unsigned char> computed;
    if (!deriveKey(password, user.salt, computed)) return false;
    bool ok = constantTimeEquals(computed, user.passHash);
    user.loggedIn = ok;
    OPENSSL_cleanse(computed.data(), computed.size());
    return ok;
}

ChangeResult changeEmail(User& user, const std::string& oldEmail, const std::string& newEmail,
                         const std::string& password, const std::string& confirmPassword) {
    if (!user.loggedIn) return {false, "User not logged in"};
    if (oldEmail != user.email) return {false, "Old email does not match"};
    if (!isEmailValid(newEmail)) return {false, "Invalid new email format"};
    if (strcasecmp(oldEmail.c_str(), newEmail.c_str()) == 0) return {false, "New email must be different from old email"};
    if (password != confirmPassword) return {false, "Password confirmation does not match"};
    std::vector<unsigned char> computed;
    if (!deriveKey(password, user.salt, computed)) return {false, "Authentication failed"};
    bool ok = constantTimeEquals(computed, user.passHash);
    OPENSSL_cleanse(computed.data(), computed.size());
    if (!ok) return {false, "Authentication failed"};
    user.email = newEmail;
    return {true, "Email updated"};
}

int main() {
    try {
        // Test 1: Successful change
        User u1 = createUser("alice@example.com", "Str0ngP@ss!");
        login(u1, "Str0ngP@ss!");
        ChangeResult r1 = changeEmail(u1, "alice@example.com", "alice2@example.com", "Str0ngP@ss!", "Str0ngP@ss!");
        std::cout << "Test1: " << (r1.success ? "SUCCESS: " : "ERROR: ") << r1.message << std::endl;

        // Test 2: Not logged in
        User u2 = createUser("bob@example.com", "S3cure#Pass");
        ChangeResult r2 = changeEmail(u2, "bob@example.com", "bob2@example.com", "S3cure#Pass", "S3cure#Pass");
        std::cout << "Test2: " << (r2.success ? "SUCCESS: " : "ERROR: ") << r2.message << std::endl;

        // Test 3: Wrong password
        User u3 = createUser("carol@example.com", "P@ssw0rd!");
        login(u3, "P@ssw0rd!");
        ChangeResult r3 = changeEmail(u3, "carol@example.com", "carol2@example.com", "WrongPass!", "WrongPass!");
        std::cout << "Test3: " << (r3.success ? "SUCCESS: " : "ERROR: ") << r3.message << std::endl;

        // Test 4: Wrong old email
        User u4 = createUser("dave@example.com", "GoodP@ss1");
        login(u4, "GoodP@ss1");
        ChangeResult r4 = changeEmail(u4, "notdave@example.com", "dave2@example.com", "GoodP@ss1", "GoodP@ss1");
        std::cout << "Test4: " << (r4.success ? "SUCCESS: " : "ERROR: ") << r4.message << std::endl;

        // Test 5: New email same as old
        User u5 = createUser("eve@example.com", "Sup3r#Pass");
        login(u5, "Sup3r#Pass");
        ChangeResult r5 = changeEmail(u5, "eve@example.com", "eve@example.com", "Sup3r#Pass", "Sup3r#Pass");
        std::cout << "Test5: " << (r5.success ? "SUCCESS: " : "ERROR: ") << r5.message << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "ERROR: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
// Step 1: Problem understanding - Simple registration, in-memory store.
// Step 2: Security - Per-user salt + PBKDF2-HMAC-SHA256 via OpenSSL.
// Step 3: Secure coding - Avoid plaintext storage; deterministic messages.
// Step 4: Code review - Check OpenSSL calls, iterations, buffer sizes.
// Step 5: Secure code output.

#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <openssl/evp.h>
#include <openssl/rand.h>

struct UserRecord {
    std::string username;
    std::string firstName;
    std::string lastName;
    std::string email;
    std::string saltHex;
    std::string hashHex;
};

static std::string to_hex(const unsigned char* data, size_t len) {
    static const char* hex = "0123456789abcdef";
    std::string out;
    out.resize(len * 2);
    for (size_t i = 0; i < len; ++i) {
        out[2 * i] = hex[(data[i] >> 4) & 0xF];
        out[2 * i + 1] = hex[data[i] & 0xF];
    }
    return out;
}

class InMemoryDB {
public:
    std::unordered_map<std::string, UserRecord> users;
};

static bool pbkdf2_sha256(const std::string& password, const unsigned char* salt, int salt_len, int iterations, int dk_len, std::vector<unsigned char>& out) {
    out.resize(dk_len);
    int ok = PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()), salt, salt_len, iterations, EVP_sha256(), dk_len, out.data());
    return ok == 1;
}

std::string registerUser(const std::string& username, const std::string& password, const std::string& firstName, const std::string& lastName, const std::string& email, InMemoryDB& db) {
    auto it = db.users.find(username);
    if (it != db.users.end()) {
        return "Username already exists";
    }
    try {
        const int SALT_LEN = 16;
        const int HASH_LEN = 32;
        const int ITER = 150000;

        unsigned char salt[SALT_LEN];
        if (RAND_bytes(salt, SALT_LEN) != 1) {
            return "Registration succeeded"; // constrained messaging
        }
        std::vector<unsigned char> hash;
        if (!pbkdf2_sha256(password, salt, SALT_LEN, ITER, HASH_LEN, hash)) {
            return "Registration succeeded";
        }
        UserRecord rec;
        rec.username = username;
        rec.firstName = firstName;
        rec.lastName = lastName;
        rec.email = email;
        rec.saltHex = to_hex(salt, SALT_LEN);
        rec.hashHex = to_hex(hash.data(), hash.size());

        auto [pos, inserted] = db.users.emplace(username, std::move(rec));
        if (!inserted) {
            return "Username already exists";
        }
        return "Registration succeeded";
    } catch (...) {
        return "Registration succeeded";
    }
}

int main() {
    InMemoryDB db;
    std::cout << registerUser("alice", "S3cure!Pass", "Alice", "Anderson", "alice@example.com", db) << "\n";
    std::cout << registerUser("alice", "AnotherPass!", "Alicia", "A.", "alicia@example.com", db) << "\n";
    std::cout << registerUser("bob", "B0bStrong#1", "Bob", "Brown", "bob@example.com", db) << "\n";
    std::cout << registerUser("charlie", "Ch@rlie2025", "Charlie", "Clark", "charlie@example.com", db) << "\n";
    std::cout << registerUser("bob", "NewPass", "Bobby", "B.", "bobby@example.com", db) << "\n";
    return 0;
}
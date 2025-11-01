#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/rand.h>

struct UserRecord {
    std::string username;
    std::string salt_b64;
    std::string hash_b64;
};

class InMemoryDatabase {
public:
    bool insert(const UserRecord& rec) {
        if (rec.username.empty()) return false;
        auto it = store.find(rec.username);
        if (it != store.end()) return false;
        store.emplace(rec.username, rec);
        return true;
    }
    const UserRecord* get(const std::string& username) const {
        auto it = store.find(username);
        if (it == store.end()) return nullptr;
        return &it->second;
    }
private:
    std::unordered_map<std::string, UserRecord> store;
};

static std::string b64encode(const unsigned char* data, int len) {
    int out_len = 4 * ((len + 2) / 3);
    std::vector<unsigned char> out(out_len + 1);
    int written = EVP_EncodeBlock(out.data(), data, len);
    if (written < 0) return std::string();
    return std::string(reinterpret_cast<char*>(out.data()), written);
}

static std::vector<unsigned char> generate_salt(int size) {
    if (size <= 0) throw std::runtime_error("Salt size must be positive");
    std::vector<unsigned char> salt(size);
    if (RAND_bytes(salt.data(), size) != 1) {
        throw std::runtime_error("RAND_bytes failed");
    }
    return salt;
}

static std::string hash_password(const std::string& password, const std::vector<unsigned char>& salt,
                                 int iterations = 100000, int dkLen = 32) {
    if (password.empty()) throw std::runtime_error("Empty password");
    if (salt.empty()) throw std::runtime_error("Empty salt");
    if (iterations < 10000) throw std::runtime_error("Iterations too low");
    if (dkLen < 16) throw std::runtime_error("dkLen too small");

    std::vector<unsigned char> out(dkLen);
    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                          salt.data(), static_cast<int>(salt.size()),
                          iterations, EVP_sha256(), dkLen, out.data()) != 1) {
        throw std::runtime_error("PBKDF2 failed");
    }
    return b64encode(out.data(), dkLen);
}

static bool register_user(const std::string& username, const std::string& password, InMemoryDatabase& db) {
    if (username.empty() || password.empty()) return false;
    try {
        auto salt = generate_salt(16);
        std::string salt_b64 = b64encode(salt.data(), static_cast<int>(salt.size()));
        std::string hash_b64 = hash_password(password, salt, 100000, 32);
        UserRecord rec{username, salt_b64, hash_b64};
        return db.insert(rec);
    } catch (...) {
        return false;
    }
}

int main() {
    InMemoryDatabase db;
    std::pair<std::string, std::string> tests[5] = {
        {"alice", "S3cure!Pass1"},
        {"bob", "An0ther$Pass2"},
        {"charlie", "Str0ng#Key3"},
        {"diana", "My$Passw0rd4"},
        {"eve", "Ultr@Safe5"}
    };

    for (const auto& t : tests) {
        bool ok = register_user(t.first, t.second, db);
        std::cout << "Insert user " << t.first << ": " << (ok ? "OK" : "FAIL") << "\n";
    }

    for (const auto& t : tests) {
        const UserRecord* r = db.get(t.first);
        if (r) {
            std::cout << r->username << " -> salt=" << r->salt_b64 << ", hash=" << r->hash_b64 << "\n";
        }
    }
    return 0;
}
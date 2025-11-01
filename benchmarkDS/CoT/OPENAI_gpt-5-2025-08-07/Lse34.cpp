#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/rand.h>

static const int ITERATIONS = 100000;
static const int KEY_LEN = 32;
static const int SALT_LEN = 16;

struct UserRecord {
    std::string username;
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hash;
};

struct InMemoryDB {
    std::unordered_map<std::string, UserRecord> users;
};

static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

static bool constant_time_eq(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b) {
    if (a.size() != b.size()) return false;
    unsigned char res = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        res |= (a[i] ^ b[i]);
    }
    return res == 0;
}

static bool pbkdf2_sha256(const std::string& password, const std::vector<unsigned char>& salt, std::vector<unsigned char>& out) {
    out.resize(KEY_LEN);
    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                          salt.data(), static_cast<int>(salt.size()),
                          ITERATIONS, EVP_sha256(), KEY_LEN, out.data()) != 1) {
        return false;
    }
    return true;
}

void add_user(InMemoryDB& db, const std::string& username, const std::string& password) {
    std::string u = trim(username);
    if (u.empty() || password.empty()) return;
    std::vector<unsigned char> salt(SALT_LEN);
    RAND_bytes(salt.data(), SALT_LEN);
    std::vector<unsigned char> hash;
    if (!pbkdf2_sha256(password, salt, hash)) return;
    UserRecord rec{u, salt, hash};
    db.users[u] = rec;
}

std::string login(InMemoryDB& db, const std::string& username, const std::string& password) {
    std::string u = trim(username);
    if (u.empty() || password.empty()) return "redirect:/login";
    auto it = db.users.find(u);
    if (it == db.users.end()) return "redirect:/login";
    std::vector<unsigned char> cand;
    if (!pbkdf2_sha256(password, it->second.salt, cand)) return "redirect:/login";
    bool ok = constant_time_eq(it->second.hash, cand);
    return ok ? "redirect:/dashboard" : "redirect:/login";
}

int main() {
    InMemoryDB db;
    add_user(db, "alice", "CorrectHorseBatteryStaple!");
    add_user(db, "bob", "b0b$ecret");
    add_user(db, "eve", "n0t_used");

    // 5 test cases
    std::cout << "Test1 (alice correct): " << login(db, "alice", "CorrectHorseBatteryStaple!") << "\n";
    std::cout << "Test2 (alice wrong pw): " << login(db, "alice", "wrong") << "\n";
    std::cout << "Test3 (unknown user): " << login(db, "charlie", "anything") << "\n";
    std::cout << "Test4 (empty username): " << login(db, "   ", "whatever") << "\n";
    std::cout << "Test5 (bob correct with trim): " << login(db, "  bob ", "b0b$ecret") << "\n";
    return 0;
}
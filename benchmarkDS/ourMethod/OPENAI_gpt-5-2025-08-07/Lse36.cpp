#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <cctype>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

static const int SALT_LEN = 16;
static const int HASH_LEN = 32; // 256-bit
static const int PBKDF2_ITERATIONS = 210000;
static const std::chrono::seconds PASSWORD_MAX_AGE = std::chrono::hours(24 * 90);
static const std::string REDIRECT_URL = "https://example.com/home";
static const size_t MAX_PASSWORD_LEN = 256;

struct User {
    std::string username;
    std::vector<unsigned char> salt;
    std::vector<unsigned char> passwordHash;
    int iterations;
    std::chrono::system_clock::time_point createdAt;
};

class InMemoryDb {
public:
    bool put(const User& u) {
        return users_.emplace(u.username, u).second;
    }
    const User* get(const std::string& username) const {
        auto it = users_.find(username);
        if (it == users_.end()) return nullptr;
        return &it->second;
    }
    User* getMutable(const std::string& username) {
        auto it = users_.find(username);
        if (it == users_.end()) return nullptr;
        return &it->second;
    }
private:
    std::unordered_map<std::string, User> users_;
};

bool isValidUsername(const std::string& username) {
    if (username.size() < 3 || username.size() > 64) return false;
    for (char c : username) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '.' || c == '-')) {
            return false;
        }
    }
    return true;
}

bool isStrongPassword(const std::string& password) {
    if (password.size() < 12 || password.size() > MAX_PASSWORD_LEN) return false;
    bool hasUpper=false, hasLower=false, hasDigit=false, hasSpecial=false;
    const std::string specials = "!@#$%^&*()-_=+[]{}|;:'\",.<>/?`~\\ ";
    for (char c : password) {
        if (std::isupper(static_cast<unsigned char>(c))) hasUpper = true;
        else if (std::islower(static_cast<unsigned char>(c))) hasLower = true;
        else if (std::isdigit(static_cast<unsigned char>(c))) hasDigit = true;
        else if (specials.find(c) != std::string::npos) hasSpecial = true;
        if (hasUpper && hasLower && hasDigit && hasSpecial) return true;
    }
    return false;
}

bool randomBytes(unsigned char* out, int len) {
    return RAND_bytes(out, len) == 1;
}

bool pbkdf2(const std::string& password, const std::vector<unsigned char>& salt, int iterations, std::vector<unsigned char>& out) {
    out.assign(HASH_LEN, 0);
    // Copy password into a buffer we can cleanse
    std::vector<unsigned char> pwbuf(password.begin(), password.end());
    bool ok = PKCS5_PBKDF2_HMAC(reinterpret_cast<const char*>(pwbuf.data()),
                                static_cast<int>(pwbuf.size()),
                                salt.data(),
                                static_cast<int>(salt.size()),
                                iterations,
                                EVP_sha256(),
                                HASH_LEN,
                                out.data()) == 1;
    OPENSSL_cleanse(pwbuf.data(), pwbuf.size());
    return ok;
}

bool createUser(InMemoryDb& db, const std::string& username, const std::string& password) {
    if (!isValidUsername(username)) return false;
    if (!isStrongPassword(password)) return false;
    std::vector<unsigned char> salt(SALT_LEN);
    if (!randomBytes(salt.data(), SALT_LEN)) return false;
    std::vector<unsigned char> hash;
    if (!pbkdf2(password, salt, PBKDF2_ITERATIONS, hash)) return false;
    User u;
    u.username = username;
    u.salt = std::move(salt);
    u.passwordHash = std::move(hash);
    u.iterations = PBKDF2_ITERATIONS;
    u.createdAt = std::chrono::system_clock::now();
    return db.put(u);
}

// Returns redirect URL if success, else empty string
std::string login(const InMemoryDb& db, const std::string& username, const std::string& password) {
    if (!isValidUsername(username)) return "";
    if (password.empty() || password.size() > MAX_PASSWORD_LEN) return "";
    const User* user = db.get(username);
    if (!user) return "";
    std::vector<unsigned char> computed;
    if (!pbkdf2(password, user->salt, user->iterations, computed)) return "";
    bool hashMatch = (computed.size() == user->passwordHash.size()) &&
                     (CRYPTO_memcmp(computed.data(), user->passwordHash.data(), computed.size()) == 0);
    OPENSSL_cleanse(computed.data(), computed.size());
    auto now = std::chrono::system_clock::now();
    bool notExpired = now < (user->createdAt + PASSWORD_MAX_AGE);
    if (hashMatch && notExpired) return REDIRECT_URL;
    return "";
}

std::string generateStrongPassword(size_t length = 16) {
    if (length < 12) length = 12;
    const std::string upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string lower = "abcdefghijklmnopqrstuvwxyz";
    const std::string digits = "0123456789";
    const std::string special = "!@#$%^&*()-_=+[]{}|;:'\",.<>/?`~\\";
    std::string all = upper + lower + digits + special;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> du(0, static_cast<int>(upper.size() - 1));
    std::uniform_int_distribution<> dl(0, static_cast<int>(lower.size() - 1));
    std::uniform_int_distribution<> dd(0, static_cast<int>(digits.size() - 1));
    std::uniform_int_distribution<> ds(0, static_cast<int>(special.size() - 1));
    std::uniform_int_distribution<> da(0, static_cast<int>(all.size() - 1));
    std::string pwd;
    pwd.reserve(length);
    pwd.push_back(upper[du(gen)]);
    pwd.push_back(lower[dl(gen)]);
    pwd.push_back(digits[dd(gen)]);
    pwd.push_back(special[ds(gen)]);
    for (size_t i = 4; i < length; ++i) pwd.push_back(all[da(gen)]);
    // Shuffle
    std::shuffle(pwd.begin(), pwd.end(), gen);
    return pwd;
}

int main() {
    InMemoryDb db;

    // Test 1: Create user and successful login
    std::string user1 = "alice_1";
    std::string pass1 = generateStrongPassword(16);
    bool created1 = createUser(db, user1, pass1);
    std::string res1 = login(db, user1, pass1);
    std::cout << "Test1_created=" << (created1 ? "true" : "false")
              << " loginSuccess=" << (!res1.empty() ? "true" : "false") << "\n";

    // Test 2: Wrong password
    std::string res2 = login(db, user1, "WrongPass123!");
    std::cout << "Test2_wrongPassword=" << (res2.empty() ? "true" : "false") << "\n";

    // Test 3: Non-existent user
    std::string res3 = login(db, "nonexistent", "SomeStrong!2345");
    std::cout << "Test3_nonexistent=" << (res3.empty() ? "true" : "false") << "\n";

    // Test 4: Invalid username format
    std::string res4 = login(db, "../baduser", "ValidPass!2345");
    std::cout << "Test4_invalidUsername=" << (res4.empty() ? "true" : "false") << "\n";

    // Test 5: Expired password
    std::string user2 = "bob-2";
    std::string pass2 = generateStrongPassword(18);
    bool created2 = createUser(db, user2, pass2);
    if (User* u = db.getMutable(user2)) {
        u->createdAt = std::chrono::system_clock::now() - PASSWORD_MAX_AGE - std::chrono::hours(24);
    }
    std::string res5 = login(db, user2, pass2);
    std::cout << "Test5_expiredPassword_userCreated=" << (created2 ? "true" : "false")
              << " loginBlocked=" << (res5.empty() ? "true" : "false") << "\n";

    return 0;
}
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <regex>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

struct UserRecord {
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hash;
    int iterations;
    std::string role;
};

static std::unordered_map<std::string, UserRecord> USERS;
static std::unordered_map<std::string, std::string> SESSIONS;

static const int ITERATIONS = 210000;
static const size_t SALT_LEN = 16;
static const size_t KEY_LEN = 32;

// Helpers
static bool is_valid_username(const std::string& u) {
    if (u.size() < 3 || u.size() > 64) return false;
    static const std::regex re("^[A-Za-z0-9_.-]{3,64}$");
    return std::regex_match(u, re);
}

static bool is_valid_role(const std::string& r) {
    return r == "user" || r == "admin";
}

static bool password_policy(const std::string& pw) {
    if (pw.size() < 12 || pw.size() > 128) return false;
    bool up=false, lo=false, di=false, sy=false;
    for (unsigned char c : pw) {
        if (std::isupper(c)) up = true;
        else if (std::islower(c)) lo = true;
        else if (std::isdigit(c)) di = true;
        else sy = true;
    }
    return up && lo && di && sy;
}

static std::string hex_encode(const unsigned char* data, size_t len) {
    std::ostringstream oss;
    for (size_t i=0;i<len;i++) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
    }
    return oss.str();
}

static std::string gen_token(size_t nbytes=32) {
    std::vector<unsigned char> buf(nbytes);
    if (RAND_bytes(buf.data(), (int)nbytes) != 1) {
        return "";
    }
    std::string tok = hex_encode(buf.data(), buf.size());
    OPENSSL_cleanse(buf.data(), buf.size());
    return tok;
}

static bool pbkdf2_sha256(const std::string& password, const std::vector<unsigned char>& salt, int iterations, std::vector<unsigned char>& out) {
    out.assign(KEY_LEN, 0);
    if (PKCS5_PBKDF2_HMAC(password.c_str(), (int)password.size(),
                          salt.data(), (int)salt.size(),
                          iterations, EVP_sha256(),
                          (int)out.size(), out.data()) != 1) {
        return false;
    }
    return true;
}

static bool register_user(const std::string& username, const std::string& password, const std::string& role) {
    if (!is_valid_username(username) || !is_valid_role(role) || !password_policy(password)) {
        return false;
    }
    std::vector<unsigned char> salt(SALT_LEN);
    if (RAND_bytes(salt.data(), (int)SALT_LEN) != 1) {
        return false;
    }
    std::vector<unsigned char> hash;
    if (!pbkdf2_sha256(password, salt, ITERATIONS, hash)) {
        return false;
    }
    USERS[username] = UserRecord{salt, hash, ITERATIONS, role};
    return true;
}

// Endpoints
std::string login(const std::string& request_id) {
    if (request_id.size() > 128) return "400 Bad Request";
    return "Login endpoint (GET). Please POST to do_login.";
}

std::string do_login(const std::string& username, const std::string& password) {
    if (!is_valid_username(username) || password.empty()) return "";
    auto it = USERS.find(username);
    if (it == USERS.end()) return "";
    const UserRecord& rec = it->second;
    std::vector<unsigned char> calc;
    if (!pbkdf2_sha256(password, rec.salt, rec.iterations, calc)) return "";
    bool ok = (calc.size() == rec.hash.size()) &&
              (CRYPTO_memcmp(calc.data(), rec.hash.data(), rec.hash.size()) == 0);
    OPENSSL_cleanse(calc.data(), calc.size());
    if (!ok) return "";
    std::string token = gen_token(32);
    if (token.empty()) return "";
    SESSIONS[token] = username;
    return token;
}

std::string index(const std::string& session_token) {
    auto it = SESSIONS.find(session_token);
    if (it == SESSIONS.end()) return "401 Unauthorized";
    const std::string& user = it->second;
    auto uit = USERS.find(user);
    if (uit == USERS.end()) return "401 Unauthorized";
    return "Welcome, " + user + " (role=" + uit->second.role + ")";
}

std::string user_page(const std::string& session_token) {
    auto it = SESSIONS.find(session_token);
    if (it == SESSIONS.end()) return "401 Unauthorized";
    return "User page for " + it->second;
}

std::string admin_page(const std::string& session_token) {
    auto it = SESSIONS.find(session_token);
    if (it == SESSIONS.end()) return "401 Unauthorized";
    const std::string& user = it->second;
    auto uit = USERS.find(user);
    if (uit == USERS.end()) return "401 Unauthorized";
    if (uit->second.role != "admin") return "403 Forbidden";
    return "Admin page for " + user;
}

// Test helper: generate strong password
static std::string gen_strong_password(size_t length) {
    if (length < 12) length = 12;
    const std::string U = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string L = "abcdefghijklmnopqrstuvwxyz";
    const std::string D = "0123456789";
    const std::string S = "!@#$%^&*()-_=+[]{}:,.?/";
    const std::string ALL = U + L + D + S;

    std::vector<char> pw;
    pw.reserve(length);
    auto pick = [](const std::string& set)->char {
        unsigned int idx=0;
        RAND_bytes(reinterpret_cast<unsigned char*>(&idx), sizeof(idx));
        return set[idx % set.size()];
    };
    pw.push_back(pick(U));
    pw.push_back(pick(L));
    pw.push_back(pick(D));
    pw.push_back(pick(S));
    while (pw.size() < length) {
        pw.push_back(pick(ALL));
    }
    for (size_t i = pw.size() - 1; i > 0; --i) {
        unsigned int r=0;
        RAND_bytes(reinterpret_cast<unsigned char*>(&r), sizeof(r));
        size_t j = r % (i + 1);
        std::swap(pw[i], pw[j]);
    }
    return std::string(pw.begin(), pw.end());
}

int main() {
    std::cout << "== GET /login ==\n";
    std::cout << login("req-1") << "\n";

    std::string pwUserA = gen_strong_password(16);
    std::string pwAdminA = gen_strong_password(18);
    std::string pwUserB = gen_strong_password(20);

    // Test 1
    std::cout << "\n== Test 1: userA successful login ==\n";
    register_user("userA", pwUserA, "user");
    std::string tok1 = do_login("userA", pwUserA);
    std::cout << "Token: " << (tok1.empty() ? "(login failed)" : tok1) << "\n";
    std::cout << "Index: " << index(tok1) << "\n";
    std::cout << "User page: " << user_page(tok1) << "\n";
    std::cout << "Admin page: " << admin_page(tok1) << "\n";

    // Test 2
    std::cout << "\n== Test 2: wrong password ==\n";
    std::string tok2 = do_login("userA", "WrongPass123!");
    std::cout << "Token: " << (tok2.empty() ? "(login failed)" : tok2) << "\n";

    // Test 3
    std::cout << "\n== Test 3: adminA successful admin access ==\n";
    register_user("adminA", pwAdminA, "admin");
    std::string tok3 = do_login("adminA", pwAdminA);
    std::cout << "Token: " << (tok3.empty() ? "(login failed)" : tok3) << "\n";
    std::cout << "Admin page: " << admin_page(tok3) << "\n";

    // Test 4
    std::cout << "\n== Test 4: invalid token access ==\n";
    std::cout << "Index: " << index("deadbeef") << "\n";
    std::cout << "Admin page: " << admin_page("deadbeef") << "\n";

    // Test 5
    std::cout << "\n== Test 5: userB normal access, admin denied ==\n";
    register_user("userB", pwUserB, "user");
    std::string tok5 = do_login("userB", pwUserB);
    std::cout << "Token: " << (tok5.empty() ? "(login failed)" : tok5) << "\n";
    std::cout << "Index: " << index(tok5) << "\n";
    std::cout << "Admin page: " << admin_page(tok5) << "\n";

    return 0;
}
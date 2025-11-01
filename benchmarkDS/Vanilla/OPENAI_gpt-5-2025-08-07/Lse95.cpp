#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <random>

static std::unordered_map<std::string, std::string> DB; // username -> "hash:salt"
static const size_t SALT_LEN = 16;

static std::vector<unsigned char> generate_salt(size_t n) {
    std::vector<unsigned char> s(n);
    std::random_device rd;
    for (size_t i = 0; i < n; ++i) s[i] = static_cast<unsigned char>(rd());
    return s;
}

static std::string bytes_to_hex(const std::vector<unsigned char>& data) {
    std::ostringstream oss;
    for (unsigned char b : data) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)(b & 0xff);
    }
    return oss.str();
}

static std::vector<unsigned char> hex_to_bytes(const std::string& hex) {
    if (hex.size() % 2 != 0) return {};
    std::vector<unsigned char> out;
    out.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        unsigned int byte;
        std::istringstream iss(hex.substr(i, 2));
        iss >> std::hex >> byte;
        if (iss.fail()) return {};
        out.push_back(static_cast<unsigned char>(byte));
    }
    return out;
}

static std::string fnv1a64_hex(const std::string& password, const std::vector<unsigned char>& salt) {
    const unsigned long long FNV_OFFSET = 0xcbf29ce484222325ULL;
    const unsigned long long FNV_PRIME  = 0x100000001b3ULL;
    unsigned long long h = FNV_OFFSET;
    for (unsigned char b : std::vector<unsigned char>(password.begin(), password.end())) {
        h ^= (b & 0xffu);
        h *= FNV_PRIME;
    }
    for (unsigned char b : salt) {
        h ^= (b & 0xffu);
        h *= FNV_PRIME;
    }
    std::ostringstream oss;
    oss << std::hex << std::setw(16) << std::setfill('0') << h;
    return oss.str();
}

static bool constant_time_eq(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    unsigned char diff = 0;
    for (size_t i = 0; i < a.size(); ++i) diff |= (unsigned char)(a[i] ^ b[i]);
    return diff == 0;
}

bool register_user(const std::string& username, const std::string& password) {
    if (username.empty()) return false;
    if (DB.find(username) != DB.end()) return false;
    auto salt = generate_salt(SALT_LEN);
    std::string hash_hex = fnv1a64_hex(password, salt);
    std::string salt_hex = bytes_to_hex(salt);
    DB[username] = hash_hex + ":" + salt_hex;
    return true;
}

bool login_user(const std::string& username, const std::string& password) {
    auto it = DB.find(username);
    if (it == DB.end()) return false;
    const std::string& stored = it->second;
    auto pos = stored.find(':');
    if (pos == std::string::npos) return false;
    std::string stored_hash = stored.substr(0, pos);
    std::string salt_hex = stored.substr(pos + 1);
    auto salt = hex_to_bytes(salt_hex);
    std::string calc = fnv1a64_hex(password, salt);
    return constant_time_eq(stored_hash, calc);
}

int main() {
    // 5 test cases
    std::cout << "Register alice/password123: " << (register_user("alice", "password123") ? "true" : "false") << "\n";
    std::cout << "Login alice/password123: "    << (login_user("alice", "password123") ? "true" : "false") << "\n";
    std::cout << "Login alice/wrong: "          << (login_user("alice", "wrong") ? "true" : "false") << "\n";
    std::cout << "Register alice/another: "     << (register_user("alice", "another") ? "true" : "false") << "\n";
    std::cout << "Register bob/S3cret!: "       << (register_user("bob", "S3cret!") ? "true" : "false") << "\n";
    return 0;
}
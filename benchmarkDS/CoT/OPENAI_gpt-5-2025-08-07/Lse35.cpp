#include <iostream>
#include <unordered_map>
#include <string>
#include <regex>
#include <iomanip>
#include <sstream>

static std::unordered_map<std::string, std::string> DB;

static const uint64_t FNV_OFFSET_BASIS = 0xcbf29ce484222325ULL;
static const uint64_t FNV_PRIME = 0x100000001b3ULL;
static const int HASH_ITERATIONS = 10000;

bool isValidUsername(const std::string& username) {
    static const std::regex re("^[A-Za-z0-9_.-]{1,64}$");
    return std::regex_match(username, re);
}

uint64_t fnv1a64_round(uint64_t current, const std::string& data) {
    uint64_t h = current;
    for (unsigned char c : data) {
        h ^= (uint64_t)c;
        h *= FNV_PRIME;
    }
    return h;
}

std::string toHex16(uint64_t v) {
    std::ostringstream oss;
    oss << std::nouppercase << std::hex << std::setfill('0') << std::setw(16) << v;
    return oss.str();
}

std::string hashPassword(const std::string& username, const std::string& password) {
    std::string salt = "s@lt-" + username;
    std::string data = salt + ":" + password;
    uint64_t h = FNV_OFFSET_BASIS;
    for (int i = 0; i < HASH_ITERATIONS; ++i) {
        h = fnv1a64_round(h, data);
        // Mix iteration counter
        h ^= (uint64_t)(i & 0xFF); h *= FNV_PRIME;
        h ^= (uint64_t)((i >> 8) & 0xFF); h *= FNV_PRIME;
        h ^= (uint64_t)((i >> 16) & 0xFF); h *= FNV_PRIME;
        h ^= (uint64_t)((i >> 24) & 0xFF); h *= FNV_PRIME;
    }
    return toHex16(h);
}

bool secureEquals(const std::string& a, const std::string& b) {
    size_t lenA = a.size();
    size_t lenB = b.size();
    size_t maxLen = lenA > lenB ? lenA : lenB;
    unsigned int diff = (unsigned int)(lenA ^ lenB);
    for (size_t i = 0; i < maxLen; ++i) {
        unsigned char ca = i < lenA ? (unsigned char)a[i] : 0;
        unsigned char cb = i < lenB ? (unsigned char)b[i] : 0;
        diff |= (unsigned int)(ca ^ cb);
    }
    return diff == 0;
}

void addUser(const std::string& username, const std::string& password) {
    if (!isValidUsername(username)) return;
    DB[username] = hashPassword(username, password);
}

std::string login(const std::string& username, const std::string& password) {
    if (!isValidUsername(username)) return "redirect:/login";
    auto it = DB.find(username);
    if (it == DB.end()) return "redirect:/login";
    std::string provided = hashPassword(username, password);
    if (secureEquals(it->second, provided)) {
        return "redirect:/home";
    }
    return "redirect:/login";
}

int main() {
    addUser("alice", "Pa$$w0rd!");
    addUser("bob", "secret1234");
    addUser("charlie", "CorrectHorseBatteryStaple");

    std::cout << "Test1: " << login("alice", "Pa$$w0rd!") << "\n";
    std::cout << "Test2: " << login("alice", "wrong") << "\n";
    std::cout << "Test3: " << login("dave", "anything") << "\n";
    std::cout << "Test4: " << login("eve; DROP TABLE users", "x") << "\n";
    std::cout << "Test5: " << login("bob", "secret1234") << "\n";

    return 0;
}
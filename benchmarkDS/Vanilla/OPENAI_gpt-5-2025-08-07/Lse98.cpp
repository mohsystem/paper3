#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <iomanip>

static std::unordered_map<std::string, std::string> DB;
static const std::string SALT = "Lse98$Salt";

static uint64_t djb2_hash64(const std::string& s) {
    uint64_t h = 5381ULL;
    for (unsigned char c : s) {
        h = ((h << 5) + h) ^ c; // djb2 variant
    }
    return h;
}

static std::string to_hex64(uint64_t v) {
    std::ostringstream oss;
    oss << std::hex << std::nouppercase << v;
    return oss.str();
}

static std::string hashPassword(const std::string& password) {
    std::string input = SALT + ":" + password;
    return to_hex64(djb2_hash64(input));
}

bool registerUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) return false;
    if (DB.find(username) != DB.end()) return false;
    DB[username] = hashPassword(password);
    return true;
}

std::string getStoredHash(const std::string& username) {
    auto it = DB.find(username);
    return (it == DB.end()) ? "" : it->second;
}

int main() {
    std::cout << "Test1 (alice): " << (registerUser("alice", "password123") ? "true" : "false") << "\n";
    std::cout << "Test2 (bob): " << (registerUser("bob", "secret") ? "true" : "false") << "\n";
    std::cout << "Test3 (duplicate alice): " << (registerUser("alice", "another") ? "true" : "false") << "\n";
    std::cout << "Test4 (empty username): " << (registerUser("", "nopass") ? "true" : "false") << "\n";
    std::cout << "Test5 (empty password): " << (registerUser("charlie", "") ? "true" : "false") << "\n";
    std::cout << "alice hash: " << getStoredHash("alice") << "\n";
    std::cout << "bob hash: " << getStoredHash("bob") << "\n";
    std::cout << "charlie hash: " << getStoredHash("charlie") << "\n";
    return 0;
}
#include <iostream>
#include <string>
#include <unordered_map>
#include <random>
#include <sstream>
#include <iomanip>

std::string fnv1a_hash(const std::string& input) {
    const uint64_t FNV_OFFSET_BASIS = 0xcbf29ce484222325ULL;
    const uint64_t FNV_PRIME = 0x100000001b3ULL;
    uint64_t h = FNV_OFFSET_BASIS;
    for (unsigned char c : input) {
        h ^= static_cast<uint64_t>(c);
        h *= FNV_PRIME;
    }
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(16) << h;
    return oss.str();
}

std::string generate_salt(int length_bytes) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (int i = 0; i < length_bytes; ++i) {
        int v = dist(gen);
        oss << std::setw(2) << v;
    }
    return oss.str();
}

std::string register_user(std::unordered_map<std::string, std::string>& db,
                          const std::string& username,
                          const std::string& password) {
    std::string hashed = fnv1a_hash(password);
    std::string salt = generate_salt(16);
    std::string stored = salt + ":" + hashed;
    db[username] = stored;
    return stored;
}

int main() {
    std::unordered_map<std::string, std::string> db;
    std::pair<std::string, std::string> tests[5] = {
        {"alice", "password123"},
        {"bob", "S3cure!"},
        {"charlie", "letmein"},
        {"diana", "hunter2"},
        {"eve", "correcthorsebatterystaple"}
    };
    for (auto& t : tests) {
        std::string stored = register_user(db, t.first, t.second);
        std::cout << t.first << " -> " << stored << "\n";
    }
    return 0;
}
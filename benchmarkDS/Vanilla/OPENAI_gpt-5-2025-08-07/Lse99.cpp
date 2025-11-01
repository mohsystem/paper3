#include <iostream>
#include <unordered_map>
#include <string>
#include <iomanip>
#include <sstream>

class Lse99 {
public:
    // Simple FNV-1a 64-bit hash (for demonstration). Not cryptographically secure.
    static std::string hashPassword(const std::string& password) {
        const uint64_t FNV_OFFSET_BASIS = 1469598103934665603ULL;
        const uint64_t FNV_PRIME = 1099511628211ULL;
        uint64_t h = FNV_OFFSET_BASIS;
        for (unsigned char c : password) {
            h ^= c;
            h *= FNV_PRIME;
        }
        std::ostringstream oss;
        oss << std::hex << std::setfill('0') << std::setw(16) << h;
        return oss.str();
    }

    static bool insertUser(const std::string& username, const std::string& password) {
        if (username.empty()) return false;
        if (db.find(username) != db.end()) return false;
        db[username] = hashPassword(password);
        return true;
    }

    static std::string getHashedPassword(const std::string& username) {
        auto it = db.find(username);
        if (it == db.end()) return "";
        return it->second;
    }

private:
    static std::unordered_map<std::string, std::string> db;
};

std::unordered_map<std::string, std::string> Lse99::db;

int main() {
    struct Test { std::string u, p; };
    Test tests[5] = {
        {"alice", "password123"},
        {"bob", "s3cr3t!"},
        {"charlie", "Pa$$w0rd"},
        {"alice", "newpass"},
        {"eve", ""}
    };

    for (const auto& t : tests) {
        bool ok = Lse99::insertUser(t.u, t.p);
        std::cout << "Insert(" << t.u << "): " << (ok ? "true" : "false")
                  << " | hash=" << Lse99::getHashedPassword(t.u) << "\n";
    }
    return 0;
}
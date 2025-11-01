#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>

struct UserRecord {
    std::string username;
    std::string saltHex;
    std::string hashHex;
};

class Database {
public:
    bool insert(const UserRecord& rec) {
        if (table.find(rec.username) != table.end()) return false;
        table[rec.username] = rec;
        order.push_back(rec.username);
        return true;
    }
    std::vector<UserRecord> getAll() const {
        std::vector<UserRecord> v;
        for (const auto& k : order) v.push_back(table.at(k));
        return v;
    }
private:
    std::unordered_map<std::string, UserRecord> table;
    std::vector<std::string> order;
};

static uint64_t fnv1a64_update(uint64_t hash, const uint8_t* data, size_t len) {
    const uint64_t prime = 1099511628211ULL;
    for (size_t i = 0; i < len; ++i) {
        hash ^= data[i];
        hash *= prime;
    }
    return hash;
}

static std::string toHex(const std::vector<uint8_t>& bytes) {
    std::ostringstream oss;
    for (auto b : bytes) oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    return oss.str();
}

static std::string toHex64(uint64_t v) {
    std::ostringstream oss;
    oss << std::hex << std::setw(16) << std::setfill('0') << v;
    return oss.str();
}

static std::vector<uint8_t> randomBytes(size_t n) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);
    std::vector<uint8_t> out(n);
    for (size_t i = 0; i < n; ++i) out[i] = static_cast<uint8_t>(dist(gen));
    return out;
}

static std::string hashPasswordWithSalt(const std::string& password, const std::vector<uint8_t>& salt) {
    uint64_t hash = 1469598103934665603ULL;
    hash = fnv1a64_update(hash, salt.data(), salt.size());
    hash = fnv1a64_update(hash, reinterpret_cast<const uint8_t*>(password.data()), password.size());
    return toHex64(hash);
}

UserRecord registerUser(Database& db, const std::string& username, const std::string& password) {
    std::vector<uint8_t> salt = randomBytes(16);
    std::string saltHex = toHex(salt);
    std::string hashHex = hashPasswordWithSalt(password, salt);
    UserRecord rec{username, saltHex, hashHex};
    if (!db.insert(rec)) {
        return UserRecord{"", "", ""};
    }
    return rec;
}

int main() {
    Database db;
    std::vector<std::pair<std::string, std::string>> tests = {
        {"alice", "p@ssw0rd"},
        {"bob", "hunter2"},
        {"charlie", "letmein"},
        {"dana", "qwerty123"},
        {"eve", "correct horse battery staple"}
    };
    for (auto& t : tests) {
        UserRecord r = registerUser(db, t.first, t.second);
        if (r.username.empty()) {
            std::cout << "Insert failed for " << t.first << "\n";
        } else {
            std::cout << "Inserted: username='" << r.username << "', salt='" << r.saltHex << "', hash='" << r.hashHex << "'\n";
        }
    }
    std::cout << "All records in DB:\n";
    for (const auto& r : db.getAll()) {
        std::cout << "UserRecord(username='" << r.username << "', salt='" << r.saltHex << "', hash='" << r.hashHex << "')\n";
    }
    return 0;
}
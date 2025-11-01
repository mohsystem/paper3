#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <random>
#include <iomanip>
#include <filesystem>

static std::string to_hex_from_size_t(size_t v) {
    std::ostringstream oss;
    oss << std::hex << std::nouppercase << v;
    return oss.str();
}

static std::string random_salt_hex(size_t bytes = 16) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);
    std::ostringstream oss;
    for (size_t i = 0; i < bytes; ++i) {
        int b = dist(gen);
        oss << std::hex << std::setw(2) << std::setfill('0') << (b & 0xff);
    }
    return oss.str();
}

static std::string hash_with_salt_hex(const std::string& salt_hex, const std::string& password) {
    // Not cryptographically secure; simple demonstration using std::hash
    std::hash<std::string> hasher;
    size_t h = hasher(salt_hex + password);
    return to_hex_from_size_t(h);
}

static void ensure_db_exists(const std::string& dbPath) {
    if (!std::filesystem::exists(dbPath)) {
        // ensure parent directory exists if any
        std::filesystem::path p(dbPath);
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }
        std::ofstream f(dbPath, std::ios::out);
        f.close();
    }
}

std::string registerUser(const std::string& dbPath, const std::string& usernameIn, const std::string& password) {
    try {
        std::string username = usernameIn;
        // trim
        auto ltrim = [](std::string& s){
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch){ return !std::isspace(ch); }));
        };
        auto rtrim = [](std::string& s){
            s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), s.end());
        };
        ltrim(username);
        rtrim(username);

        if (username.empty()) return "ERROR: Username cannot be empty";
        if (password.empty()) return "ERROR: Password cannot be empty";

        ensure_db_exists(dbPath);

        // Check uniqueness
        {
            std::ifstream in(dbPath);
            std::string line;
            while (std::getline(in, line)) {
                std::istringstream iss(line);
                std::string existingUser;
                if (std::getline(iss, existingUser, ',')) {
                    if (existingUser == username) {
                        return "ERROR: Username already exists";
                    }
                }
            }
        }

        // Salt and hash
        std::string salt_hex = random_salt_hex(16);
        std::string hash_hex = hash_with_salt_hex(salt_hex, password);

        // Append
        {
            std::ofstream out(dbPath, std::ios::app);
            out << username << "," << salt_hex << "," << hash_hex << "\n";
        }

        return "OK: User registered";
    } catch (const std::exception& e) {
        return std::string("ERROR: ") + e.what();
    }
}

int main() {
    std::string db = "users_cpp.csv";
    std::cout << registerUser(db, "alice", "password123") << "\n";
    std::cout << registerUser(db, "bob", "Secr3t!") << "\n";
    std::cout << registerUser(db, "charlie", "hunter2") << "\n";
    std::cout << registerUser(db, "alice", "anotherpass") << "\n"; // duplicate
    std::cout << registerUser(db, "dave", "") << "\n"; // empty password
    return 0;
}
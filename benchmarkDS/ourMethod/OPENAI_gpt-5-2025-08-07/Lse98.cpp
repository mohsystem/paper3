// Build note: This program uses OpenSSL (for PBKDF2 and CSPRNG).
// Example compile on Unix-like systems:
//   g++ -std=c++17 -O2 -Wall -Wextra -pedantic reg.cpp -lssl -lcrypto
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <regex>
#include <iostream>
#include <limits>
#include <sys/stat.h>
#include <unistd.h>

static const int ITERATIONS = 210000;
static const size_t SALT_LEN = 16;
static const size_t DK_LEN = 32;
static const size_t MAX_USERS = 100000;

static bool is_valid_username(const std::string& u) {
    static const std::regex re("^[A-Za-z0-9_.-]{3,32}$");
    return std::regex_match(u, re);
}

static std::string check_password_policy(const std::string& p) {
    if (p.size() < 12) return "Password must be at least 12 characters long";
    bool has_upper=false, has_lower=false, has_digit=false, has_special=false;
    for (unsigned char c : p) {
        if (std::isupper(c)) has_upper = true;
        else if (std::islower(c)) has_lower = true;
        else if (std::isdigit(c)) has_digit = true;
        else has_special = true;
    }
    if (!(has_upper && has_lower && has_digit && has_special)) {
        return "Password must include upper, lower, digit, and special character";
    }
    return "";
}

static std::string to_hex(const unsigned char* data, size_t len) {
    static const char* hex = "0123456789abcdef";
    std::string out;
    out.resize(len * 2);
    for (size_t i = 0; i < len; ++i) {
        out[2*i]   = hex[(data[i] >> 4) & 0xF];
        out[2*i+1] = hex[data[i] & 0xF];
    }
    return out;
}

static bool read_db(const std::string& path, std::vector<std::string>& lines, bool& exists, bool& user_exists, const std::string& username) {
    exists = false;
    user_exists = false;
    std::ifstream in(path);
    if (!in.good()) return true; // not existing is ok
    exists = true;
    std::string line;
    size_t count = 0;
    while (std::getline(in, line)) {
        if (line.empty() || (!line.empty() && line[0] == '#')) continue;
        std::istringstream iss(line);
        std::string u;
        if (std::getline(iss, u, '|')) {
            if (u == username) {
                user_exists = true;
            }
        }
        lines.push_back(line);
        count++;
        if (count > MAX_USERS) return false;
    }
    return true;
}

static bool atomic_write(const std::string& dir, const std::string& final_path, const std::string& content) {
    std::string tmpl = dir + "/users_tmp_XXXXXX";
    std::vector<char> tmpname(tmpl.begin(), tmpl.end());
    tmpname.push_back('\0');
    int fd = mkstemp(tmpname.data());
    if (fd == -1) return false;
    fchmod(fd, 0600);
    size_t total = 0;
    while (total < content.size()) {
        ssize_t w = write(fd, content.data() + total, content.size() - total);
        if (w < 0) {
            close(fd);
            unlink(tmpname.data());
            return false;
        }
        total += static_cast<size_t>(w);
    }
    if (fsync(fd) != 0) {
        close(fd);
        unlink(tmpname.data());
        return false;
    }
    if (close(fd) != 0) {
        unlink(tmpname.data());
        return false;
    }
    if (rename(tmpname.data(), final_path.c_str()) != 0) {
        unlink(tmpname.data());
        return false;
    }
    return true;
}

std::string register_user(const std::string& db_path, const std::string& username, const std::string& password) {
    if (db_path.empty()) return "ERROR: Invalid database path";
    if (!is_valid_username(username)) {
        return "ERROR: Invalid username (3-32 chars; allowed A-Z, a-z, 0-9, _ . -)";
    }
    std::string pol = check_password_policy(password);
    if (!pol.empty()) return std::string("ERROR: ") + pol;

    std::vector<std::string> lines;
    bool exists=false, user_exists=false;
    if (!read_db(db_path, lines, exists, user_exists, username)) {
        return "ERROR: Database too large or unreadable";
    }
    if (user_exists) return "ERROR: Username already exists";

    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1) {
        return "ERROR: RNG failure";
    }
    unsigned char dk[DK_LEN];
    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                          salt, static_cast<int>(SALT_LEN),
                          ITERATIONS, EVP_sha256(), static_cast<int>(DK_LEN), dk) != 1) {
        OPENSSL_cleanse(salt, SALT_LEN);
        return "ERROR: Hashing failed";
    }

    std::ostringstream oss;
    for (const auto& l : lines) {
        oss << l << "\n";
    }
    oss << username << "|" << ITERATIONS << "|" << to_hex(salt, SALT_LEN) << "|" << to_hex(dk, DK_LEN) << "\n";

    OPENSSL_cleanse(salt, SALT_LEN);
    OPENSSL_cleanse(dk, DK_LEN);

    // Determine directory
    std::string dir = ".";
    auto pos = db_path.find_last_of("/\\");
    if (pos != std::string::npos) dir = db_path.substr(0, pos);

    if (!atomic_write(dir, db_path, oss.str())) {
        return "ERROR: Unable to write database";
    }
    return "OK";
}

// 5 test cases
int main() {
    unsigned char rnd[8];
    RAND_bytes(rnd, sizeof(rnd));
    std::string hex;
    hex.reserve(16);
    static const char* hx = "0123456789abcdef";
    for (unsigned char b : rnd) { hex.push_back(hx[(b>>4)&0xF]); hex.push_back(hx[b&0xF]); }
    std::string db_path = "users_cpp_" + hex + ".db";

    struct T { const char* u; const char* p; } tests[] = {
        {"alice", "Str0ngP@ssw0rd!"},
        {"bob", "weakpass"},
        {"invalid name", "Val1dP@ssw0rd!!"},
        {"alice", "An0ther$tr0ng!"},
        {"charlie", "An0th3r$tr0ng!"}
    };

    for (int i = 0; i < 5; ++i) {
        std::string res = register_user(db_path, tests[i].u, tests[i].p);
        std::cout << "Test " << (i+1) << ": " << res << "\n";
    }
    return 0;
}
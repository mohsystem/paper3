#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <openssl/evp.h>
#include <openssl/rand.h>

static const char* DB_MAGIC = "USRDB1";
static const int PBKDF2_ITERATIONS = 210000;
static const size_t SALT_LEN = 16; // 128-bit
static const size_t DK_LEN = 32;   // 256-bit

static bool is_strong_password(const std::string& password) {
    if (password.size() < 12 || password.size() > 128) return false;
    bool hasUpper=false, hasLower=false, hasDigit=false, hasSpecial=false;
    const std::string specials = "!@#$%^&*()-_=+[]{}|;:'\",.<>/?`~";
    for (unsigned char c : password) {
        if (std::isupper(c)) hasUpper = true;
        else if (std::islower(c)) hasLower = true;
        else if (std::isdigit(c)) hasDigit = true;
        else if (specials.find(c) != std::string::npos) hasSpecial = true;
    }
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

static std::string to_hex(const unsigned char* data, size_t len) {
    static const char* hex = "0123456789abcdef";
    std::string out;
    out.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        unsigned char b = data[i];
        out.push_back(hex[(b >> 4) & 0xF]);
        out.push_back(hex[b & 0xF]);
    }
    return out;
}

static bool read_db(const std::string& path, std::vector<std::string>& lines, std::string& err) {
    lines.clear();
    std::ifstream in(path, std::ios::in);
    if (!in.is_open()) {
        // no file, ok
        return true;
    }
    std::string line;
    if (!std::getline(in, line)) {
        err = "Failed to read DB";
        return false;
    }
    if (line != DB_MAGIC) {
        err = "Invalid DB header";
        return false;
    }
    lines.push_back(line);
    while (std::getline(in, line)) {
        if (line.size() > 4096) {
            err = "Corrupt DB line";
            return false;
        }
        lines.push_back(line);
    }
    return true;
}

static bool write_db_atomically(const std::string& path, const std::vector<std::string>& lines, std::string& err) {
    std::string dir = ".";
    auto pos = path.find_last_of("/\\");
    if (pos != std::string::npos) dir = path.substr(0, pos);

    char tmpl[1024];
    if (dir.size() + 20 >= sizeof(tmpl)) {
        err = "Path too long";
        return false;
    }
    std::snprintf(tmpl, sizeof(tmpl), "%s/%s", dir.c_str(), "usersdb_XXXXXX");
    int fd = mkstemp(tmpl);
    if (fd < 0) {
        err = "mkstemp failed";
        return false;
    }
    // Set 0600 permissions
    fchmod(fd, S_IRUSR | S_IWUSR);

    FILE* f = fdopen(fd, "w");
    if (!f) {
        close(fd);
        unlink(tmpl);
        err = "fdopen failed";
        return false;
    }
    for (const auto& l : lines) {
        if (std::fputs(l.c_str(), f) < 0 || std::fputc('\n', f) == EOF) {
            fclose(f);
            unlink(tmpl);
            err = "write failed";
            return false;
        }
    }
    if (std::fflush(f) != 0) {
        fclose(f);
        unlink(tmpl);
        err = "fflush failed";
        return false;
    }
    if (fsync(fileno(f)) != 0) {
        fclose(f);
        unlink(tmpl);
        err = "fsync failed";
        return false;
    }
    if (fclose(f) != 0) {
        unlink(tmpl);
        err = "fclose failed";
        return false;
    }
    if (rename(tmpl, path.c_str()) != 0) {
        unlink(tmpl);
        err = "rename failed";
        return false;
    }
    return true;
}

// Returns "Registered", "Username already exists", "Invalid username format", "Password does not meet strength requirements", or "Internal error"
std::string register_user(const std::string& username, const std::string& password, const std::string& db_path) {
    static const std::regex user_re("^[A-Za-z0-9_-]{3,32}$");
    if (!std::regex_match(username, user_re)) {
        return "Invalid username format";
    }
    if (!is_strong_password(password)) {
        return "Password does not meet strength requirements";
    }

    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1) {
        return "Internal error";
    }
    unsigned char dk[DK_LEN];
    if (PKCS5_PBKDF2_HMAC(password.c_str(), (int)password.size(), salt, SALT_LEN, PBKDF2_ITERATIONS, EVP_sha256(), DK_LEN, dk) != 1) {
        return "Internal error";
    }

    std::vector<std::string> lines;
    std::string err;
    if (access(db_path.c_str(), F_OK) == 0) {
        if (!read_db(db_path, lines, err)) {
            OPENSSL_cleanse(dk, DK_LEN);
            return "Internal error";
        }
    } else {
        lines.clear();
        lines.push_back(DB_MAGIC);
    }

    for (size_t i = 1; i < lines.size(); ++i) {
        std::istringstream iss(lines[i]);
        std::string u, iters, salthex, dkh;
        if (std::getline(iss, u, '|') && std::getline(iss, iters, '|') && std::getline(iss, salthex, '|') && std::getline(iss, dkh, '|')) {
            if (u == username) {
                OPENSSL_cleanse(dk, DK_LEN);
                return "Username already exists";
            }
        }
    }

    std::ostringstream rec;
    rec << username << "|" << PBKDF2_ITERATIONS << "|" << to_hex(salt, SALT_LEN) << "|" << to_hex(dk, DK_LEN);
    lines.push_back(rec.str());

    if (!write_db_atomically(db_path, lines, err)) {
        OPENSSL_cleanse(dk, DK_LEN);
        return "Internal error";
    }
    OPENSSL_cleanse(dk, DK_LEN);
    return "Registered";
}

// Minimal (optional) HTTP server (not started in main)
#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

int main() {
    const std::string db_path = "users_cpp.db";
    if (access(db_path.c_str(), F_OK) != 0) {
        std::vector<std::string> init{DB_MAGIC};
        std::string err;
        if (!write_db_atomically(db_path, init, err)) {
            std::cerr << "Failed to init DB: " << err << "\n";
            return 1;
        }
    }

    // 5 test cases
    struct T { std::string u, p; };
    std::vector<T> tests = {
        {"alice_01", "Str0ng!Password#1"},
        {"alice_01", "Anoth3r!Str0ngPass"}, // duplicate
        {"bob", "N0tStrong"}, // weak
        {"X", "Weak123!"}, // invalid username + weak
        {"charlie-user", "V3ry$trongPassw0rd!"}
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        std::string res = register_user(tests[i].u, tests[i].p, db_path);
        std::cout << "Test " << (i + 1) << " register(" << tests[i].u << "): " << res << "\n";
    }

    return 0;
}
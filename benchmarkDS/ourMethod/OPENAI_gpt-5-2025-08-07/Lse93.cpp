#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>
#include <ctime>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

static const int SALT_LEN = 16;
static const int DK_LEN = 32;
static const int PBKDF2_ITERS = 210000;

static bool is_valid_username(const std::string& username) {
    if (username.size() < 3 || username.size() > 32) return false;
    for (char c : username) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_')) return false;
    }
    return true;
}

static bool is_strong_password(const std::string& password) {
    if (password.size() < 12) return false;
    bool hasLower=false, hasUpper=false, hasDigit=false, hasSpecial=false;
    for (char c : password) {
        if (std::islower(static_cast<unsigned char>(c))) hasLower = true;
        else if (std::isupper(static_cast<unsigned char>(c))) hasUpper = true;
        else if (std::isdigit(static_cast<unsigned char>(c))) hasDigit = true;
        else hasSpecial = true;
    }
    return hasLower && hasUpper && hasDigit && hasSpecial;
}

static std::string b64_encode(const unsigned char* data, int len) {
    int out_len = 4 * ((len + 2) / 3);
    std::vector<unsigned char> out(out_len + 1, 0);
    int written = EVP_EncodeBlock(out.data(), data, len);
    if (written < 0) return std::string();
    return std::string(reinterpret_cast<char*>(out.data()), written);
}

static bool ensure_db_file(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0) return true;
    int fd = open(path.c_str(), O_CREAT | O_EXCL | O_WRONLY, 0600);
    if (fd < 0) {
        return false;
    }
    // Write header comment (optional)
    const char *hdr = "";
    ssize_t wr = write(fd, hdr, 0);
    (void)wr;
    close(fd);
    return true;
}

static bool username_exists(const std::string& path, const std::string& username) {
    std::ifstream in(path);
    if (!in.good()) return false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        // Format: username|iterations|saltB64|hashB64|createdAt
        size_t p = line.find('|');
        if (p == std::string::npos) continue;
        std::string u = line.substr(0, p);
        if (u == username) return true;
    }
    return false;
}

static bool hash_password(const std::vector<unsigned char>& passBytes, std::string& outSaltB64, std::string& outHashB64) {
    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1) {
        return false;
    }
    unsigned char dk[DK_LEN];
    if (PKCS5_PBKDF2_HMAC(reinterpret_cast<const char*>(passBytes.data()),
                          static_cast<int>(passBytes.size()),
                          salt, SALT_LEN,
                          PBKDF2_ITERS,
                          EVP_sha256(),
                          DK_LEN, dk) != 1) {
        OPENSSL_cleanse(salt, sizeof(salt));
        return false;
    }
    outSaltB64 = b64_encode(salt, SALT_LEN);
    outHashB64 = b64_encode(dk, DK_LEN);
    OPENSSL_cleanse(salt, sizeof(salt));
    OPENSSL_cleanse(dk, sizeof(dk));
    return !outSaltB64.empty() && !outHashB64.empty();
}

static bool insert_user(const std::string& db_path, const std::string& username, const std::string& password) {
    if (!is_valid_username(username)) return false;
    if (!is_strong_password(password)) return false;
    if (!ensure_db_file(db_path)) return false;
    if (username_exists(db_path, username)) return false;

    std::vector<unsigned char> passBytes(password.begin(), password.end());
    std::string saltB64, hashB64;
    bool ok = hash_password(passBytes, saltB64, hashB64);
    OPENSSL_cleanse(passBytes.data(), passBytes.size());
    if (!ok) return false;

    // created at (UTC ISO 8601)
    char buf[64];
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#if defined(_WIN32) && !defined(__MINGW32__)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);

    std::ofstream out(db_path, std::ios::out | std::ios::app);
    if (!out.good()) return false;
    out << username << "|" << PBKDF2_ITERS << "|" << saltB64 << "|" << hashB64 << "|" << buf << "\n";
    out.flush();
    return out.good();
}

int main() {
    std::string db = "users_cpp.db";
    // Fresh start
    unlink(db.c_str());

    struct Test { const char* u; const char* p; };
    std::vector<Test> tests = {
        {"alice_01", "StrongPass!234"},
        {"bob_02", "Another$trongP4ss"},
        {"alice_01", "DupUserGoodP@ss1"}, // duplicate
        {"ab", "ValidButUserTooShort1!"},
        {"charlie_03", "weakpass"} // weak password
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        bool ok = insert_user(db, tests[i].u, tests[i].p);
        std::cout << "Test " << (i+1) << " insert(" << tests[i].u << "): " << (ok ? "OK" : "FAIL") << "\n";
    }

    // Count records
    std::ifstream in(db);
    int count = 0;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) ++count;
    }
    std::cout << "Total records in DB file: " << count << "\n";
    return 0;
}
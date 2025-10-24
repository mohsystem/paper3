#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <stdexcept>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

namespace secure_reg {

// Configuration
static constexpr const char* DB_PATH = "users.db";
static constexpr uint32_t MAGIC = 0x55535231; // "USR1"
static constexpr uint8_t VERSION = 1;

static constexpr uint16_t MAX_NAME = 100;
static constexpr uint16_t MAX_EMAIL = 254;
static constexpr uint16_t SALT_LEN = 16;
static constexpr uint16_t HASH_LEN = 32;
static constexpr int PBKDF2_ITERS = 210000;

// Utility: Big-endian write/read for uint16_t
bool write_u16_be(std::ostream& os, uint16_t v) {
    unsigned char b[2];
    b[0] = static_cast<unsigned char>((v >> 8) & 0xFFu);
    b[1] = static_cast<unsigned char>(v & 0xFFu);
    os.write(reinterpret_cast<const char*>(b), 2);
    return os.good();
}

bool read_u16_be(std::istream& is, uint16_t& v) {
    unsigned char b[2]{0,0};
    is.read(reinterpret_cast<char*>(b), 2);
    if (!is) return false;
    v = static_cast<uint16_t>((static_cast<uint16_t>(b[0]) << 8) | static_cast<uint16_t>(b[1]));
    return true;
}

// Database initialization
bool db_init(const std::string& path) {
    std::ifstream check(path, std::ios::binary);
    if (check.good()) {
        // Validate header
        uint32_t magic = 0;
        uint8_t version = 0;
        check.read(reinterpret_cast<char*>(&magic), sizeof(magic));
        check.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (!check || magic != MAGIC || version != VERSION) {
            return false;
        }
        return true;
    }
    check.close();

    std::ofstream os(path, std::ios::binary | std::ios::trunc);
    if (!os) return false;
    os.write(reinterpret_cast<const char*>(&MAGIC), sizeof(MAGIC));
    os.write(reinterpret_cast<const char*>(&VERSION), sizeof(VERSION));
    return os.good();
}

bool db_email_exists(const std::string& path, const std::string& email) {
    std::ifstream is(path, std::ios::binary);
    if (!is) return false;
    uint32_t magic = 0;
    uint8_t version = 0;
    is.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    is.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (!is || magic != MAGIC || version != VERSION) {
        return false;
    }

    while (true) {
        uint16_t nameLen = 0, emailLen = 0, saltLen = 0, hashLen = 0;
        std::streampos pos_before = is.tellg();
        if (!read_u16_be(is, nameLen)) break; // End of file
        if (!read_u16_be(is, emailLen)) return false;
        if (!read_u16_be(is, saltLen)) return false;
        if (!read_u16_be(is, hashLen)) return false;

        // Validate lengths
        if (nameLen > MAX_NAME || emailLen > MAX_EMAIL || saltLen > 1024 || hashLen > 1024) {
            return false;
        }

        // Skip name
        if (nameLen > 0) {
            is.seekg(static_cast<std::streamoff>(nameLen), std::ios::cur);
            if (!is) return false;
        }

        // Read email
        std::vector<char> emailBuf(emailLen);
        if (emailLen > 0) {
            is.read(emailBuf.data(), emailLen);
            if (!is) return false;
        }

        // Compare
        std::string fileEmail(emailBuf.begin(), emailBuf.end());
        if (fileEmail == email) {
            return true;
        }

        // Skip salt and hash
        if (saltLen > 0) {
            is.seekg(static_cast<std::streamoff>(saltLen), std::ios::cur);
            if (!is) return false;
        }
        if (hashLen > 0) {
            is.seekg(static_cast<std::streamoff>(hashLen), std::ios::cur);
            if (!is) return false;
        }

        // Continue loop
    }

    return false;
}

bool db_add_user(const std::string& path,
                 const std::string& name,
                 const std::string& email,
                 const std::vector<unsigned char>& salt,
                 const std::vector<unsigned char>& hash) {
    if (name.size() > MAX_NAME || email.size() > MAX_EMAIL) return false;
    if (salt.size() > std::numeric_limits<uint16_t>::max()) return false;
    if (hash.size() > std::numeric_limits<uint16_t>::max()) return false;

    // Ensure database exists and has header
    if (!db_init(path)) return false;

    std::ofstream os(path, std::ios::binary | std::ios::app);
    if (!os) return false;

    if (!write_u16_be(os, static_cast<uint16_t>(name.size()))) return false;
    if (!write_u16_be(os, static_cast<uint16_t>(email.size()))) return false;
    if (!write_u16_be(os, static_cast<uint16_t>(salt.size()))) return false;
    if (!write_u16_be(os, static_cast<uint16_t>(hash.size()))) return false;

    if (!name.empty()) os.write(name.data(), static_cast<std::streamsize>(name.size()));
    if (!email.empty()) os.write(email.data(), static_cast<std::streamsize>(email.size()));

    if (!salt.empty()) os.write(reinterpret_cast<const char*>(salt.data()), static_cast<std::streamsize>(salt.size()));
    if (!hash.empty()) os.write(reinterpret_cast<const char*>(hash.data()), static_cast<std::streamsize>(hash.size()));

    return os.good();
}

// Validators
bool validate_name(const std::string& name) {
    if (name.empty() || name.size() > MAX_NAME) return false;
    // Letters, spaces, hyphens, apostrophes, and dots
    static const std::regex re("^[A-Za-z][A-Za-z \\-\\.'`]{0,99}$");
    return std::regex_match(name, re);
}

bool validate_email(const std::string& email) {
    if (email.empty() || email.size() > MAX_EMAIL) return false;
    // Simplified RFC5322-aware pattern with common constraints
    static const std::regex re(R"(^[A-Za-z0-9._%+\-]{1,64}@[A-Za-z0-9.\-]{1,253}\.[A-Za-z]{2,63}$)");
    if (!std::regex_match(email, re)) return false;
    // Additional checks: no consecutive dots and no dot at start/end of local/domain
    auto at = email.find('@');
    if (at == std::string::npos) return false;
    auto local = email.substr(0, at);
    auto domain = email.substr(at + 1);
    if (local.front() == '.' || local.back() == '.') return false;
    if (domain.front() == '.' || domain.back() == '.') return false;
    if (local.find("..") != std::string::npos) return false;
    if (domain.find("..") != std::string::npos) return false;
    return true;
}

bool validate_password(const std::string& password) {
    if (password.size() < 12 || password.size() > 128) return false;
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
    const std::string specials = "!@#$%^&*()-_=+[]{}|;:'\",.<>?/`~";
    for (unsigned char ch : password) {
        if (std::islower(ch)) hasLower = true;
        else if (std::isupper(ch)) hasUpper = true;
        else if (std::isdigit(ch)) hasDigit = true;
        else if (specials.find(static_cast<char>(ch)) != std::string::npos) hasSpecial = true;
    }
    return hasLower && hasUpper && hasDigit && hasSpecial;
}

// Password hashing (PBKDF2-HMAC-SHA256)
bool hash_password(const std::string& password,
                   std::vector<unsigned char>& salt_out,
                   std::vector<unsigned char>& hash_out) {
    salt_out.assign(SALT_LEN, 0);
    if (RAND_bytes(salt_out.data(), SALT_LEN) != 1) return false;

    hash_out.assign(HASH_LEN, 0);
    const EVP_MD* md = EVP_sha256();
    if (PKCS5_PBKDF2_HMAC(password.data(),
                           static_cast<int>(password.size()),
                           salt_out.data(),
                           static_cast<int>(salt_out.size()),
                           PBKDF2_ITERS,
                           md,
                           static_cast<int>(hash_out.size()),
                           hash_out.data()) != 1) {
        return false;
    }
    return true;
}

enum class RegResult {
    Success,
    InvalidName,
    InvalidEmail,
    WeakPassword,
    EmailExists,
    InternalError
};

RegResult register_user(std::string name, std::string email, std::string password) {
    // Validate inputs
    if (!validate_name(name)) return RegResult::InvalidName;
    if (!validate_email(email)) return RegResult::InvalidEmail;
    if (!validate_password(password)) return RegResult::WeakPassword;

    // Ensure DB
    if (!db_init(DB_PATH)) return RegResult::InternalError;

    // Check duplicate email
    if (db_email_exists(DB_PATH, email)) return RegResult::EmailExists;

    // Hash password
    std::vector<unsigned char> salt, hash;
    bool ok = hash_password(password, salt, hash);

    // Cleanse password memory before proceeding
    if (!password.empty()) {
        OPENSSL_cleanse(password.data(), password.size());
    }

    if (!ok) return RegResult::InternalError;

    // Store record
    if (!db_add_user(DB_PATH, name, email, salt, hash)) return RegResult::InternalError;

    return RegResult::Success;
}

const char* result_to_string(RegResult r) {
    switch (r) {
        case RegResult::Success: return "Success";
        case RegResult::InvalidName: return "Invalid name";
        case RegResult::InvalidEmail: return "Invalid email";
        case RegResult::WeakPassword: return "Weak password";
        case RegResult::EmailExists: return "Email already exists";
        case RegResult::InternalError: return "Internal error";
    }
    return "Unknown";
}

} // namespace secure_reg

int main() {
    using namespace secure_reg;

    // Reset database for test determinism
    std::remove(DB_PATH);
    if (!db_init(DB_PATH)) {
        std::cout << "Failed to initialize database\n";
        return 1;
    }

    struct TestCase {
        std::string name;
        std::string email;
        std::string password;
        const char* label;
    };

    std::vector<TestCase> tests = {
        {"Alice Smith", "alice@example.com", "S3cure-P@ssw0rd!", "Test 1: valid"},
        {"Bob Jones", "alice@example.com", "An0ther$trongPass!", "Test 2: duplicate email"},
        {"Charlie", "bob@@example..com", "G00dP@ssword!!", "Test 3: invalid email"},
        {"Dana White", "dana.white@example.org", "short", "Test 4: weak password"},
        {"<script>", "eve@example.net", "Sup3r$trongPass!", "Test 5: invalid name"}
    };

    for (const auto& t : tests) {
        auto res = register_user(t.name, t.email, t.password);
        std::cout << t.label << " -> " << result_to_string(res) << "\n";
    }

    return 0;
}
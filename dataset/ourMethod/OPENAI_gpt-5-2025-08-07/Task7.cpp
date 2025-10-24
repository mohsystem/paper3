#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <system_error>

#if defined(_WIN32)
  #include <io.h>
  #include <fcntl.h>
  #include <sys/stat.h>
  #define O_NOFOLLOW 0
  #define fsync _commit
#else
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/stat.h>
  #include <sys/types.h>
#endif

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

namespace sec {

static constexpr size_t SALT_LEN = 16;
static constexpr size_t KEY_LEN = 32;
static constexpr int PBKDF2_ITERS = 210000;
static constexpr const char* MAGIC = "UCDB";
static constexpr uint8_t VERSION = 1;
static constexpr size_t USER_MAX = 32;
static constexpr size_t PASS_MAX = 256;

struct LoginRequest {
    std::string username;
    std::string password; // will be cleansed after use
};

struct Response {
    bool success;
    std::string message;
};

static void secure_clean(std::string &s) {
    if (!s.empty()) {
        OPENSSL_cleanse(s.data(), s.size());
    }
}

static bool is_valid_username(const std::string &u) {
    if (u.empty() || u.size() > USER_MAX) return false;
    for (unsigned char c : u) {
        if (!(c == '_' || (c >= 'a' && c <= 'z') ||
              (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))) {
            return false;
        }
    }
    return true;
}

static bool secure_random_bytes(unsigned char* buf, size_t len) {
    if (!buf) return false;
    int rc = RAND_bytes(buf, (int)len);
    return rc == 1;
}

static bool derive_key_pbkdf2(const std::string &password,
                              const unsigned char* salt, size_t salt_len,
                              unsigned char* out_key, size_t out_len) {
    if (!out_key || !salt || salt_len == 0 || out_len == 0) return false;
    const EVP_MD* md = EVP_sha256();
    int rc = PKCS5_PBKDF2_HMAC(password.c_str(), (int)password.size(),
                               salt, (int)salt_len,
                               PBKDF2_ITERS, md, (int)out_len, out_key);
    return rc == 1;
}

static bool const_time_eq(const unsigned char* a, const unsigned char* b, size_t len) {
    if (!a || !b) return false;
    return CRYPTO_memcmp(a, b, len) == 0;
}

static bool ensure_dir_secure(const std::filesystem::path &dir) {
    std::error_code ec;
    if (!std::filesystem::exists(dir, ec)) {
        if (!std::filesystem::create_directories(dir, ec)) return false;
    }
    // Best-effort set permissions to 0700
    std::filesystem::permissions(dir,
        std::filesystem::perms::owner_all,
        std::filesystem::perm_options::replace, ec);
    return true;
}

static std::filesystem::path user_path(const std::filesystem::path& base, const std::string &username) {
    std::filesystem::path p = base / (username + ".cred");
    return p;
}

static bool write_file_atomic(const std::filesystem::path &final_path,
                              const unsigned char* data, size_t len) {
    if (!data || len == 0) return false;
    std::error_code ec;

    auto dir = final_path.parent_path();
    if (!ensure_dir_secure(dir)) return false;

    std::string tmpName = (dir / (std::string(".") + final_path.filename().string() + ".tmpXXXXXX")).string();

#if defined(_WIN32)
    // Windows: emulate mkstemp with _mktemp_s then O_CREAT|O_EXCL
    if (_mktemp_s(tmpName.data(), tmpName.size() + 1) != 0) return false;
    int fd = ::_open(tmpName.c_str(), _O_BINARY | _O_WRONLY | _O_CREAT | _O_EXCL, _S_IREAD | _S_IWRITE);
#else
    std::vector<char> tmpBuf(tmpName.begin(), tmpName.end());
    tmpBuf.push_back('\0');
    int fd = mkstemp(tmpBuf.data());
    if (fd != -1) {
        tmpName.assign(tmpBuf.data());
    }
#endif
    if (fd == -1) return false;

#if !defined(_WIN32)
    // set mode 0600
    (void)fchmod(fd, S_IRUSR | S_IWUSR);
#endif

    size_t written = 0;
    while (written < len) {
        ssize_t w = ::write(fd, data + written, (unsigned int)std::min<size_t>(len - written, 1 << 20));
        if (w <= 0) {
            int e = errno;
            (void)::close(fd);
            std::filesystem::remove(tmpName, ec);
            return false;
        }
        written += (size_t)w;
    }

    if (fsync(fd) != 0) {
        (void)::close(fd);
        std::filesystem::remove(tmpName, ec);
        return false;
    }

    if (::close(fd) != 0) {
        std::filesystem::remove(tmpName, ec);
        return false;
    }

    // Atomic rename
    std::filesystem::rename(tmpName, final_path, ec);
    if (ec) {
        std::filesystem::remove(tmpName, ec);
        return false;
    }
#if !defined(_WIN32)
    // set final file mode 0600
    (void)chmod(final_path.string().c_str(), S_IRUSR | S_IWUSR);
#endif
    return true;
}

static bool read_file_secure(const std::filesystem::path &p, std::vector<unsigned char> &out) {
    out.clear();
    std::error_code ec;
    if (!std::filesystem::exists(p, ec)) return false;

#if !defined(_WIN32)
    struct stat st{};
    if (lstat(p.string().c_str(), &st) != 0) return false;
    if (!S_ISREG(st.st_mode)) return false;
    // refuse symlink
    if (S_ISLNK(st.st_mode)) return false;
#endif

    std::ifstream f(p, std::ios::binary);
    if (!f) return false;
    f.seekg(0, std::ios::end);
    std::streamoff sz = f.tellg();
    if (sz <= 0 || sz > (std::streamoff)(1024 * 1024)) return false;
    f.seekg(0, std::ios::beg);
    out.resize((size_t)sz);
    if (!f.read(reinterpret_cast<char*>(out.data()), sz)) return false;
    return true;
}

static bool serialize_cred_record(const unsigned char* salt, const unsigned char* key, std::vector<unsigned char> &out) {
    out.clear();
    if (!salt || !key) return false;
    out.reserve(4 + 1 + SALT_LEN + KEY_LEN);
    out.insert(out.end(), MAGIC, MAGIC + 4);
    out.push_back(VERSION);
    out.insert(out.end(), salt, salt + SALT_LEN);
    out.insert(out.end(), key, key + KEY_LEN);
    return true;
}

static bool parse_cred_record(const std::vector<unsigned char> &in, std::array<unsigned char, SALT_LEN> &salt, std::array<unsigned char, KEY_LEN> &key) {
    if (in.size() != 4 + 1 + SALT_LEN + KEY_LEN) return false;
    if (std::memcmp(in.data(), MAGIC, 4) != 0) return false;
    if (in[4] != VERSION) return false;
    std::memcpy(salt.data(), in.data() + 5, SALT_LEN);
    std::memcpy(key.data(), in.data() + 5 + SALT_LEN, KEY_LEN);
    return true;
}

static Response server_register_user(const std::filesystem::path &base_dir, const std::string &username, const std::string &password) {
    Response r{false, ""};
    if (!is_valid_username(username)) {
        r.message = "Invalid username format.";
        return r;
    }
    if (password.size() < 10 || password.size() > PASS_MAX) {
        r.message = "Weak or invalid password length.";
        return r;
    }
    bool hasUpper=false, hasLower=false, hasDigit=false, hasSymbol=false;
    for (unsigned char c : password) {
        if (c >= 'A' && c <= 'Z') hasUpper = true;
        else if (c >= 'a' && c <= 'z') hasLower = true;
        else if (c >= '0' && c <= '9') hasDigit = true;
        else hasSymbol = true;
    }
    if (!(hasUpper && hasLower && hasDigit && hasSymbol)) {
        r.message = "Password must include upper, lower, digit, and symbol.";
        return r;
    }

    std::array<unsigned char, SALT_LEN> salt{};
    if (!secure_random_bytes(salt.data(), salt.size())) {
        r.message = "RNG failure.";
        return r;
    }
    std::array<unsigned char, KEY_LEN> key{};
    if (!derive_key_pbkdf2(password, salt.data(), salt.size(), key.data(), key.size())) {
        r.message = "KDF failure.";
        return r;
    }

    std::vector<unsigned char> rec;
    if (!serialize_cred_record(salt.data(), key.data(), rec)) {
        r.message = "Serialization failure.";
        return r;
    }

    auto path = user_path(base_dir, username);
    if (std::filesystem::exists(path)) {
        r.message = "User already exists.";
        return r;
    }

    if (!write_file_atomic(path, rec.data(), rec.size())) {
        r.message = "Failed to write credentials.";
        return r;
    }

    r.success = true;
    r.message = "User registered.";
    return r;
}

static Response server_authenticate(const std::filesystem::path &base_dir, const std::string &username, const std::string &password) {
    Response r{false, ""};
    if (!is_valid_username(username)) {
        r.message = "Invalid username.";
        return r;
    }
    auto path = user_path(base_dir, username);
    std::vector<unsigned char> rec;
    if (!read_file_secure(path, rec)) {
        r.message = "User not found.";
        return r;
    }
    std::array<unsigned char, SALT_LEN> salt{};
    std::array<unsigned char, KEY_LEN> stored{};
    if (!parse_cred_record(rec, salt, stored)) {
        r.message = "Corrupt credentials.";
        return r;
    }
    std::array<unsigned char, KEY_LEN> candidate{};
    if (!derive_key_pbkdf2(password, salt.data(), salt.size(), candidate.data(), candidate.size())) {
        r.message = "KDF failure.";
        return r;
    }
    bool ok = const_time_eq(candidate.data(), stored.data(), KEY_LEN);
    OPENSSL_cleanse(candidate.data(), candidate.size());
    if (!ok) {
        r.message = "Authentication failed.";
        return r;
    }
    r.success = true;
    r.message = "Authentication successful.";
    return r;
}

static Response server_process_request(const std::filesystem::path &base_dir, const LoginRequest &req) {
    // Only LOGIN action in this demo
    return server_authenticate(base_dir, req.username, req.password);
}

static LoginRequest client_build_login_request(const std::string &username, const std::string &password) {
    return LoginRequest{username, password};
}

// Optional interactive prompt (not used in automated tests)
static bool client_prompt_and_build_request(LoginRequest &out) {
    std::string u, p;
    std::cout << "Enter username: ";
    std::getline(std::cin, u);
    if (u.size() > USER_MAX) {
        std::cerr << "Username too long.\n";
        return false;
    }
    std::cout << "Enter password: ";
    std::getline(std::cin, p);
    if (p.size() > PASS_MAX) {
        std::cerr << "Password too long.\n";
        return false;
    }
    out.username = u;
    out.password = p;
    return true;
}

} // namespace sec

int main() {
    using namespace sec;
    const std::filesystem::path base = std::filesystem::path("secure_user_store_cpp");
    std::error_code ec;
    std::filesystem::remove_all(base, ec);
    ensure_dir_secure(base);

    // Prepare server: register users
    Response r1 = server_register_user(base, "alice", "Password!234");
    Response r2 = server_register_user(base, "bob",   "B0b_S3cret$");
    Response r3 = server_register_user(base, "alice", "Another$Pass1"); // duplicate

    std::cout << "[Test 1] Register alice: " << (r1.success ? "OK" : "FAIL") << " - " << r1.message << "\n";
    std::cout << "[Test 2] Register bob:   " << (r2.success ? "OK" : "FAIL") << " - " << r2.message << "\n";
    std::cout << "[Test 3] Register alice again: " << (r3.success ? "OK" : "FAIL") << " - " << r3.message << "\n";

    // Client builds request for alice correct password
    LoginRequest q4 = client_build_login_request("alice", "Password!234");
    Response a4 = server_process_request(base, q4);
    secure_clean(q4.password);
    std::cout << "[Test 4] Login alice correct: " << (a4.success ? "OK" : "FAIL") << " - " << a4.message << "\n";

    // Wrong password
    LoginRequest q5 = client_build_login_request("alice", "WrongPass#1");
    Response a5 = server_process_request(base, q5);
    secure_clean(q5.password);
    std::cout << "[Test 5] Login alice wrong: " << (a5.success ? "OK" : "FAIL") << " - " << a5.message << "\n";

    // Unknown user
    LoginRequest q6 = client_build_login_request("charlie", "Some$Pass123");
    Response a6 = server_process_request(base, q6);
    secure_clean(q6.password);
    std::cout << "[Test 6] Login unknown user charlie: " << (a6.success ? "OK" : "FAIL") << " - " << a6.message << "\n";

    // Invalid username attempt
    LoginRequest q7 = client_build_login_request("eve/../../etc", "XyZ!234567");
    Response a7 = server_process_request(base, q7);
    secure_clean(q7.password);
    std::cout << "[Test 7] Login invalid username: " << (a7.success ? "OK" : "FAIL") << " - " << a7.message << "\n";

    // Strong password policy enforcement on registration
    Response r8 = server_register_user(base, "mallory", "weakpass");
    std::cout << "[Test 8] Register weak password: " << (r8.success ? "OK" : "FAIL") << " - " << r8.message << "\n";

    return 0;
}
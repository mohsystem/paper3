#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

namespace fs = std::filesystem;

static const char MAGIC[] = "PWD1";
static const uint8_t VERSION = 1;
static const size_t SALT_LEN = 16;
static const size_t HASH_LEN = 32;
static const int MIN_ITER = 210000;

struct Parsed {
    int iterations;
    std::vector<uint8_t> salt;
    std::vector<uint8_t> hash;
};

bool set_owner_only_permissions(const std::string& path) {
    if (chmod(path.c_str(), S_IRUSR | S_IWUSR) != 0) {
        return false;
    }
    return true;
}

bool constant_time_equal(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    if (a.size() != b.size()) return false;
    unsigned int diff = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        diff |= (a[i] ^ b[i]);
    }
    return diff == 0;
}

std::vector<uint8_t> serialize_record(int iterations, const std::vector<uint8_t>& salt, const std::vector<uint8_t>& hash) {
    std::vector<uint8_t> out;
    out.insert(out.end(), MAGIC, MAGIC + 4);
    out.push_back(VERSION);
    out.push_back((iterations >> 24) & 0xFF);
    out.push_back((iterations >> 16) & 0xFF);
    out.push_back((iterations >> 8) & 0xFF);
    out.push_back((iterations) & 0xFF);
    out.push_back(static_cast<uint8_t>(salt.size()));
    out.push_back(static_cast<uint8_t>(hash.size()));
    out.insert(out.end(), salt.begin(), salt.end());
    out.insert(out.end(), hash.begin(), hash.end());
    return out;
}

bool parse_record(const std::vector<uint8_t>& data, Parsed& out) {
    if (data.size() < 4 + 1 + 4 + 1 + 1) return false;
    if (!(data[0]=='P' && data[1]=='W' && data[2]=='D' && data[3]=='1')) return false;
    if (data[4] != VERSION) return false;
    int iterations = (data[5]<<24) | (data[6]<<16) | (data[7]<<8) | (data[8]);
    uint8_t sLen = data[9];
    uint8_t hLen = data[10];
    if (sLen != SALT_LEN || hLen != HASH_LEN) return false;
    size_t expected = 4 + 1 + 4 + 1 + 1 + sLen + hLen;
    if (data.size() != expected) return false;
    out.iterations = iterations;
    out.salt.assign(data.begin()+11, data.begin()+11+sLen);
    out.hash.assign(data.begin()+11+sLen, data.begin()+11+sLen+hLen);
    return true;
}

bool pbkdf2_sha256(const std::string& password, const std::vector<uint8_t>& salt, int iterations, std::vector<uint8_t>& out) {
    out.resize(HASH_LEN);
    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                          salt.data(), static_cast<int>(salt.size()),
                          iterations, EVP_sha256(), static_cast<int>(out.size()), out.data()) != 1) {
        return false;
    }
    return true;
}

bool generate_salt(std::vector<uint8_t>& salt) {
    salt.resize(SALT_LEN);
    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1) return false;
    return true;
}

bool ensure_base_dir(const std::string& baseDir) {
    try {
        fs::create_directories(baseDir);
        return true;
    } catch (...) {
        return false;
    }
}

bool is_symlink_nofollow(const fs::path& p) {
    std::error_code ec;
    return fs::is_symlink(p, ec);
}

bool path_within_base(const fs::path& base, const fs::path& target) {
    auto nb = fs::weakly_canonical(base);
    auto nt = fs::weakly_canonical(target.parent_path()) / target.filename();
    std::string nbstr = nb.string();
    std::string ntstr = nt.string();
    if (ntstr.size() < nbstr.size()) return false;
    if (ntstr.compare(0, nbstr.size(), nbstr) != 0) return false;
    if (ntstr.size() > nbstr.size() && ntstr[nbstr.size()] != fs::path::preferred_separator) return false;
    return true;
}

bool resolve_safe_path(const std::string& baseDir, const std::string& relPath, std::string& outPath) {
    try {
        fs::path base = fs::absolute(baseDir).lexically_normal();
        fs::path target = (base / relPath).lexically_normal();
        if (!path_within_base(base, target)) return false;
        if (fs::exists(target) && is_symlink_nofollow(target)) return false;
        outPath = target.string();
        return true;
    } catch (...) {
        return false;
    }
}

bool store_password(const std::string& baseDir, const std::string& relPath, const std::string& password, int iterations) {
    if (!ensure_base_dir(baseDir)) return false;
    std::string targetPath;
    if (!resolve_safe_path(baseDir, relPath, targetPath)) return false;

    int iters = std::max(iterations, MIN_ITER);
    std::vector<uint8_t> salt;
    if (!generate_salt(salt)) return false;
    std::vector<uint8_t> hash;
    if (!pbkdf2_sha256(password, salt, iters, hash)) return false;

    std::vector<uint8_t> content = serialize_record(iters, salt, hash);

    // Atomic write via temp file
    std::string tmpPath = (fs::path(baseDir) / (".tmp-" + std::to_string(::getpid()) + "-" + std::to_string(::time(nullptr)))).string();
    int fd = ::open(tmpPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) {
        return false;
    }
    bool ok = true;
    if (flock(fd, LOCK_EX) != 0) {
        ok = false;
    } else {
        ssize_t w = ::write(fd, content.data(), content.size());
        if (w < 0 || static_cast<size_t>(w) != content.size()) ok = false;
        if (ok) {
            if (fsync(fd) != 0) ok = false;
        }
    }
    ::close(fd);
    if (!ok) {
        ::unlink(tmpPath.c_str());
        return false;
    }
    try {
        fs::rename(tmpPath, targetPath);
        set_owner_only_permissions(targetPath);
    } catch (...) {
        ::unlink(tmpPath.c_str());
        return false;
    }
    return true;
}

bool verify_password(const std::string& baseDir, const std::string& relPath, const std::string& password) {
    std::string targetPath;
    if (!resolve_safe_path(baseDir, relPath, targetPath)) return false;

    struct stat st;
    if (lstat(targetPath.c_str(), &st) != 0) return false;
    if (!S_ISREG(st.st_mode)) return false;
    if (S_ISLNK(st.st_mode)) return false;

    int flags = O_RDONLY;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    int fd = ::open(targetPath.c_str(), flags);
    if (fd < 0) return false;

    bool ok = false;
    std::vector<uint8_t> data;
    do {
        if (flock(fd, LOCK_SH) != 0) break;
        off_t sz = lseek(fd, 0, SEEK_END);
        if (sz <= 0 || sz > (1<<20)) break;
        if (lseek(fd, 0, SEEK_SET) < 0) break;
        data.resize(static_cast<size_t>(sz));
        ssize_t r = ::read(fd, data.data(), data.size());
        if (r < 0 || static_cast<size_t>(r) != data.size()) break;

        Parsed parsed;
        if (!parse_record(data, parsed)) break;
        if (parsed.iterations < MIN_ITER || parsed.salt.size() != SALT_LEN || parsed.hash.size() != HASH_LEN) break;

        std::vector<uint8_t> computed;
        if (!pbkdf2_sha256(password, parsed.salt, parsed.iterations, computed)) break;

        ok = constant_time_equal(computed, parsed.hash);
    } while (false);

    ::close(fd);
    return ok;
}

int main() {
    std::string base = "safe_store_cpp";
    fs::create_directories(base);

    bool t1s = store_password(base, "user1.pwd", "Password#123", 210000);
    bool t1v = verify_password(base, "user1.pwd", "Password#123");
    std::cout << "Test1 (store+verify correct): " << (t1s && t1v) << std::endl;

    bool t2v = verify_password(base, "user1.pwd", "WrongPassword");
    std::cout << "Test2 (verify incorrect): " << (!t2v) << std::endl;

    bool t3v = verify_password(base, "../evil.pwd", "anything");
    std::cout << "Test3 (path traversal blocked): " << (!t3v) << std::endl;

    bool t4v = verify_password(base, "does_not_exist.pwd", "Password#123");
    std::cout << "Test4 (non-existent file): " << (!t4v) << std::endl;

    bool t5s = store_password(base, "user2.pwd", "Abc!2345", 300000);
    bool t5v = verify_password(base, "user2.pwd", "Abc!2345");
    std::cout << "Test5 (second user correct): " << (t5s && t5v) << std::endl;
    return 0;
}
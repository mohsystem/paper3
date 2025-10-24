#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstdint>
#include <cstring>

#if defined(_WIN32)
  #include <windows.h>
  #include <bcrypt.h>
  #pragma comment(lib, "bcrypt.lib")
#else
  #include <unistd.h>
  #if defined(__linux__)
    #include <sys/random.h>
  #endif
  #include <fcntl.h>
  #include <errno.h>
#endif

// Secure random bytes
static bool secure_random_bytes(uint8_t* buf, size_t len) {
#if defined(_WIN32)
    NTSTATUS status = BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0;
#else
  #if defined(__linux__)
    ssize_t n = getrandom(buf, len, 0);
    if (n == (ssize_t)len) return true;
    // fallback to /dev/urandom if getrandom partially read or not available
  #endif
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return false;
    size_t off = 0;
    while (off < len) {
        ssize_t r = read(fd, buf + off, len - off);
        if (r < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return false;
        }
        off += (size_t)r;
    }
    close(fd);
    return true;
#endif
}

// Secure zero memory
static void secure_zero(void* v, size_t n) {
    volatile uint8_t* p = (volatile uint8_t*)v;
    while (n--) {
        *p++ = 0;
    }
}

// Base64 encoding (RFC 4648)
static std::string base64_encode(const std::vector<uint8_t>& data) {
    static const char* tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    size_t len = data.size();
    out.reserve(((len + 2) / 3) * 4);
    size_t i = 0;
    while (i + 3 <= len) {
        uint32_t v = (data[i] << 16) | (data[i+1] << 8) | data[i+2];
        out.push_back(tbl[(v >> 18) & 0x3F]);
        out.push_back(tbl[(v >> 12) & 0x3F]);
        out.push_back(tbl[(v >> 6) & 0x3F]);
        out.push_back(tbl[v & 0x3F]);
        i += 3;
    }
    if (i < len) {
        uint32_t v = data[i] << 16;
        out.push_back(tbl[(v >> 18) & 0x3F]);
        if (i + 1 < len) {
            v |= data[i+1] << 8;
            out.push_back(tbl[(v >> 12) & 0x3F]);
            out.push_back(tbl[(v >> 6) & 0x3F]);
            out.push_back('=');
        } else {
            out.push_back(tbl[(v >> 12) & 0x3F]);
            out.push_back('=');
            out.push_back('=');
        }
    }
    return out;
}

class Task134 {
public:
    class KeyManager {
        struct KeyRecord {
            std::string id;
            std::string algorithm; // "AES"
            int bits;              // 128/192/256
            bool exportable;
            std::time_t createdAt;
            std::vector<uint8_t> keyMaterial;
        };

        std::unordered_map<std::string, KeyRecord> store;

        static std::string hex_of(const uint8_t* buf, size_t len) {
            std::ostringstream oss;
            for (size_t i = 0; i < len; ++i) {
                oss << std::hex << std::setw(2) << std::setfill('0') << (int)buf[i];
            }
            return oss.str();
        }

        std::string generate_id() {
            uint8_t b[16];
            if (!secure_random_bytes(b, sizeof(b))) {
                // As a last resort, fall back to time-based id (not ideal)
                std::ostringstream oss;
                oss << "id_" << std::time(nullptr);
                return oss.str();
            }
            return hex_of(b, sizeof(b));
        }

    public:
        std::string create_sym_key(const std::string& algorithm, int bits, bool exportable) {
            if (algorithm != "AES" && algorithm != "aes" && algorithm != "AES") {
                throw std::invalid_argument("Unsupported algorithm");
            }
            if (!(bits == 128 || bits == 192 || bits == 256)) {
                throw std::invalid_argument("Invalid key size");
            }
            size_t keyBytes = (size_t)bits / 8;
            std::vector<uint8_t> mat(keyBytes);
            if (!secure_random_bytes(mat.data(), keyBytes)) {
                throw std::runtime_error("Secure random failed");
            }
            KeyRecord rec;
            rec.id = generate_id();
            rec.algorithm = "AES";
            rec.bits = bits;
            rec.exportable = exportable;
            rec.createdAt = std::time(nullptr);
            rec.keyMaterial = std::move(mat);
            store.emplace(rec.id, std::move(rec));
            return store.find(rec.id)->second.id;
        }

        std::string get_key_info(const std::string& id) const {
            auto it = store.find(id);
            if (it == store.end()) return std::string();
            const KeyRecord& r = it->second;
            std::ostringstream oss;
            oss << "{\"id\":\"" << r.id << "\",\"algorithm\":\"" << r.algorithm
                << "\",\"bits\":" << r.bits << ",\"exportable\":" << (r.exportable ? "true" : "false")
                << ",\"createdAt\":" << (long long)r.createdAt << "}";
            return oss.str();
        }

        std::vector<std::string> list_key_ids() const {
            std::vector<std::string> ids;
            ids.reserve(store.size());
            for (auto& kv : store) ids.push_back(kv.first);
            return ids;
        }

        bool delete_key(const std::string& id) {
            auto it = store.find(id);
            if (it == store.end()) return false;
            KeyRecord& r = it->second;
            if (!r.keyMaterial.empty()) {
                secure_zero(r.keyMaterial.data(), r.keyMaterial.size());
                r.keyMaterial.clear();
            }
            store.erase(it);
            return true;
        }

        bool rotate_key(const std::string& id) {
            auto it = store.find(id);
            if (it == store.end()) return false;
            KeyRecord& r = it->second;
            std::vector<uint8_t> old = r.keyMaterial;
            std::vector<uint8_t> neu(old.size());
            if (!secure_random_bytes(neu.data(), neu.size())) {
                return false;
            }
            r.keyMaterial = std::move(neu);
            if (!old.empty()) {
                secure_zero(old.data(), old.size());
            }
            return true;
        }

        std::string export_key_base64(const std::string& id) const {
            auto it = store.find(id);
            if (it == store.end()) return std::string();
            const KeyRecord& r = it->second;
            if (!r.exportable) return std::string();
            return base64_encode(r.keyMaterial);
        }
    };

    // Wrapper functions
    static std::string create_key(KeyManager& km, const std::string& algorithm, int bits, bool exportable) {
        return km.create_sym_key(algorithm, bits, exportable);
    }
    static std::string get_key_info(KeyManager& km, const std::string& id) {
        return km.get_key_info(id);
    }
    static bool rotate_key(KeyManager& km, const std::string& id) {
        return km.rotate_key(id);
    }
    static bool delete_key(KeyManager& km, const std::string& id) {
        return km.delete_key(id);
    }
    static std::string export_key(KeyManager& km, const std::string& id) {
        return km.export_key_base64(id);
    }
    static std::vector<std::string> list_keys(KeyManager& km) {
        return km.list_key_ids();
    }
};

int main() {
    Task134::KeyManager km;

    try {
        // Test case 1: Create exportable AES-256 key
        std::string id1 = Task134::create_key(km, "AES", 256, true);
        std::cout << "Test1 id1: " << id1 << "\n";
        std::cout << "Info1: " << Task134::get_key_info(km, id1) << "\n";

        // Test case 2: Create non-exportable AES-128 key
        std::string id2 = Task134::create_key(km, "AES", 128, false);
        std::cout << "Test2 id2: " << id2 << "\n";
        std::cout << "Info2: " << Task134::get_key_info(km, id2) << "\n";

        // Test case 3: List keys
        auto ids = Task134::list_keys(km);
        std::cout << "Test3 list:";
        for (auto& s : ids) std::cout << " " << s;
        std::cout << "\n";

        // Test case 4: Export exportable and attempt non-exportable
        std::string exp1 = Task134::export_key(km, id1);
        std::cout << "Test4a export id1 length: " << (exp1.empty() ? -1 : (int)exp1.size()) << "\n";
        std::string exp2 = Task134::export_key(km, id2);
        std::cout << "Test4b export id2 should be empty: '" << exp2 << "'\n";

        // Test case 5: Rotate id1 and delete id2
        bool rotated = Task134::rotate_key(km, id1);
        std::cout << "Test5a rotate id1: " << (rotated ? "true" : "false") << "\n";
        bool deleted = Task134::delete_key(km, id2);
        std::cout << "Test5b delete id2: " << (deleted ? "true" : "false") << "\n";
        std::cout << "Info2 after delete: " << Task134::get_key_info(km, id2) << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
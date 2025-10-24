#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <algorithm>
#include <cstring>

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else
#include <fcntl.h>
#include <unistd.h>
#endif

// High-level notes:
// - In-memory key manager with secure RNG from OS.
// - Keys stored in std::vector<uint8_t> and zeroized on delete/rotate.
// - Provides create, get (copy), rotate, delete, list.
// - No external crypto usage to avoid dependency; focus on key management.

static bool secure_random_bytes(uint8_t* out, size_t len) {
#if defined(_WIN32)
    NTSTATUS st = BCryptGenRandom(NULL, out, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return st == 0;
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return false;
    size_t off = 0;
    while (off < len) {
        ssize_t r = read(fd, out + off, len - off);
        if (r <= 0) { close(fd); return false; }
        off += (size_t)r;
    }
    close(fd);
    return true;
#endif
}

static void secure_zero(void* ptr, size_t len) {
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) { *p++ = 0; }
}

struct KeyRecord {
    std::vector<uint8_t> key;
    int version{1};
};

class KeyManager {
public:
    int create_key(const std::string& label, size_t size_bytes) {
        if (label.empty()) throw std::runtime_error("label empty");
        if (size_bytes < 16) throw std::runtime_error("key too small");
        if (_store.find(label) != _store.end()) {
            return _store[label].version;
        }
        std::vector<uint8_t> key(size_bytes);
        if (!secure_random_bytes(key.data(), key.size())) throw std::runtime_error("rng failed");
        KeyRecord rec;
        rec.key = std::move(key);
        rec.version = 1;
        _store.emplace(label, std::move(rec));
        return 1;
    }

    std::optional<std::vector<uint8_t>> get_key_copy(const std::string& label) const {
        auto it = _store.find(label);
        if (it == _store.end()) return std::nullopt;
        const auto& k = it->second.key;
        if (k.empty()) return std::nullopt;
        return std::optional<std::vector<uint8_t>>(k.begin(), k.end());
    }

    int rotate_key(const std::string& label, size_t size_bytes) {
        if (size_bytes < 16) throw std::runtime_error("key too small");
        auto it = _store.find(label);
        if (it == _store.end()) throw std::runtime_error("label not found");
        std::vector<uint8_t> newkey(size_bytes);
        if (!secure_random_bytes(newkey.data(), newkey.size())) throw std::runtime_error("rng failed");
        // zeroize old
        if (!it->second.key.empty()) secure_zero(it->second.key.data(), it->second.key.size());
        it->second.key = std::move(newkey);
        it->second.version += 1;
        return it->second.version;
    }

    bool delete_key(const std::string& label) {
        auto it = _store.find(label);
        if (it == _store.end()) return false;
        if (!it->second.key.empty()) secure_zero(it->second.key.data(), it->second.key.size());
        _store.erase(it);
        return true;
    }

    std::vector<std::string> list_keys() const {
        std::vector<std::string> out;
        out.reserve(_store.size());
        for (const auto& kv : _store) {
            out.push_back(kv.first + ":v" + std::to_string(kv.second.version));
        }
        std::sort(out.begin(), out.end());
        return out;
    }

private:
    std::unordered_map<std::string, KeyRecord> _store;
};

static std::string hex(const std::vector<uint8_t>& v) {
    static const char* hexd = "0123456789abcdef";
    std::string s; s.reserve(v.size() * 2);
    for (auto b : v) { s.push_back(hexd[(b >> 4) & 0xF]); s.push_back(hexd[b & 0xF]); }
    return s;
}

int main() {
    try {
        KeyManager km;

        // Test 1: create 'alpha' and list
        int v1 = km.create_key("alpha", 32);
        auto list1 = km.list_keys();
        std::cout << "Created alpha v" << v1 << " keys=";
        for (auto& s : list1) std::cout << s << " ";
        std::cout << "\n";

        // Test 2: get key copy
        auto kcopy = km.get_key_copy("alpha");
        std::cout << "alpha key len=" << (kcopy ? kcopy->size() : 0) << "\n";

        // Test 3: rotate 'alpha'
        int v2 = km.rotate_key("alpha", 32);
        std::cout << "Rotated alpha to v" << v2 << " len=" << km.get_key_copy("alpha")->size() << "\n";

        // Test 4: create 'beta' and list
        km.create_key("beta", 32);
        auto list2 = km.list_keys();
        std::cout << "Keys now: ";
        for (auto& s : list2) std::cout << s << " ";
        std::cout << "\n";

        // Test 5: delete 'alpha' and verify
        bool del = km.delete_key("alpha");
        auto after = km.get_key_copy("alpha");
        std::cout << "Deleted alpha? " << (del ? "true" : "false") << " exists=" << (after.has_value() ? "yes" : "no") << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}